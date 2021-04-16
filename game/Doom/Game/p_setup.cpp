#include "p_setup.h"

#include "Doom/Base/i_file.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/m_bbox.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/Renderer/r_sky.h"
#include "doomdata.h"
#include "g_game.h"
#include "info.h"
#include "p_firesky.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "p_switch.h"
#include "p_tick.h"
#include "PcPsx/MobjSpritePrecacher.h"

#include <cstring>

// How much heap space is required after loading the map in order to run the game (48 KiB in Doom, 32 KiB in Final Doom).
// If we don't have this much then the game dies with an error; I'm adopting the Final Doom requirement here since it is the lowest.
// Need to be able to support various small allocs throughout gameplay for particles and so forth.
static constexpr int32_t MIN_REQ_HEAP_SPACE_FOR_GAMEPLAY = 1024 * 32;

// How many maps are in a map folder and the number of files per maps folder etc.
static constexpr int32_t LEVELS_PER_MAP_FOLDER = (uint32_t) CdFileId::MAPSPR01_IMG - (uint32_t) CdFileId::MAP01_WAD;
static constexpr int32_t NUM_FILES_PER_LEVEL = 3;

// Sky stuff
static constexpr const char* SKY_LUMP_NAME = "F_SKY";
static constexpr int32_t SKY_LUMP_NAME_LEN = sizeof("F_SKY") - 1;   // -1 to discount null terminator

// Map data
uint16_t*       gpBlockmapLump;
uint16_t*       gpBlockmap;
int32_t         gBlockmapWidth;
int32_t         gBlockmapHeight;
fixed_t         gBlockmapOriginX;
fixed_t         gBlockmapOriginY;
mobj_t**        gppBlockLinks;
int32_t         gNumVertexes;
vertex_t*       gpVertexes;
int32_t         gNumSectors;
sector_t*       gpSectors;
int32_t         gNumSides;
side_t*         gpSides;
int32_t         gNumLines;
line_t*         gpLines;
int32_t         gNumSubsectors;
subsector_t*    gpSubsectors;
int32_t         gNumBspNodes;
node_t*         gpBspNodes;
int32_t         gNumSegs;
seg_t*          gpSegs;
int32_t         gTotalNumLeafEdges;
leafedge_t*     gpLeafEdges;
uint8_t*        gpRejectMatrix;
mapthing_t      gPlayerStarts[MAXPLAYERS];
mapthing_t      gDeathmatchStarts[MAX_DEATHMATCH_STARTS];
mapthing_t*     gpDeathmatchP;                                  // Points past the end of the deathmatch starts list

// PsyDoom: if not null then issue this warning after the level has started.
// Can be used to issue non-fatal warnings about bad map conditions to WAD authors.
#if PSYDOOM_MODS
    const char* gLevelStartupWarning;
#endif

// Is the map being loaded a Final Doom format map?
static bool gbLoadingFinalDoomMap;

// Function to update the fire sky.
// Set when the map has a fire sky, otherwise null.
void (*gUpdateFireSkyFunc)(texture_t& skyTex) = nullptr;

// Functions iternal to this module:
#if !PSYDOOM_LIMIT_REMOVING
    static void P_LoadBlocks(const CdFileId file) noexcept;
#endif

static void P_CacheMapTexturesWithWidth(const int32_t width) noexcept;

#if !PSYDOOM_MODS
    static void P_CacheSprite(const spritedef_t& sprdef) noexcept;  // PsyDoom: not used, so compiling out
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map vertex data from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadVertexes(const int32_t lumpNum) noexcept {
    // Sanity check the vertices lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    
    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadVertexes: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Alloc the runtime vertex array
    gNumVertexes = lumpSize / sizeof(mapvertex_t);
    gpVertexes = (vertex_t*) Z_Malloc(*gpMainMemZone, gNumVertexes * sizeof(vertex_t), PU_LEVEL, nullptr);
    
    // Read the WAD vertexes into the temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);

    // Convert the vertexes to the renderer runtime format
    const mapvertex_t* pSrcVertex = (const mapvertex_t*) pTmpBufferBytes;
    vertex_t* pDstVertex = gpVertexes;

    for (int32_t vertexIdx = 0; vertexIdx < gNumVertexes; ++vertexIdx) {
        pDstVertex->x = Endian::littleToHost(pSrcVertex->x);
        pDstVertex->y = Endian::littleToHost(pSrcVertex->y);
        pDstVertex->frameUpdated = 0;
        ++pSrcVertex;
        ++pDstVertex;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load line segments from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadSegs(const int32_t lumpNum) noexcept {
    // Sanity check the segs lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadSegs: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Alloc ram for the runtime segs and zero initialize
    gNumSegs = lumpSize / sizeof(mapseg_t);
    gpSegs = (seg_t*) Z_Malloc(*gpMainMemZone, gNumSegs * sizeof(seg_t), PU_LEVEL, nullptr);
    D_memset(gpSegs, std::byte(0), gNumSegs * sizeof(seg_t));

    // Read the map lump containing the segs into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);

    // Process the WAD segs and convert them into runtime segs
    const mapseg_t* pSrcSeg = (const mapseg_t*) pTmpBufferBytes;
    seg_t* pDstSeg = gpSegs;

    for (int32_t segIdx = 0; segIdx < gNumSegs; ++segIdx) {
        // Store basic seg properties
        pDstSeg->vertex1 = &gpVertexes[Endian::littleToHost(pSrcSeg->vertex1)];
        pDstSeg->vertex2 = &gpVertexes[Endian::littleToHost(pSrcSeg->vertex2)];
        pDstSeg->angle = (angle_t) d_int_to_fixed(pSrcSeg->angle);                  // Weird, but it is what it is...
        pDstSeg->offset = d_int_to_fixed(Endian::littleToHost(pSrcSeg->offset));

        // Figure out seg line and side
        line_t& linedef = gpLines[Endian::littleToHost(pSrcSeg->linedef)];
        pDstSeg->linedef = &linedef;

        const int32_t sideNum = linedef.sidenum[Endian::littleToHost(pSrcSeg->side)];
        side_t& side = gpSides[sideNum];
        pDstSeg->sidedef = &side;

        // Set front and backsector reference
        pDstSeg->frontsector = side.sector;

        if (linedef.flags & ML_TWOSIDED) {
            const int32_t backSideNum = linedef.sidenum[Endian::littleToHost(pSrcSeg->side) ^ 1];
            side_t& backSide = gpSides[backSideNum];
            pDstSeg->backsector = backSide.sector;
        } else {
            pDstSeg->backsector = nullptr;
        }
        
        // Take this opportunity to compute line fineangle if the seg is pointing in the same direction
        if (linedef.vertex1 == pDstSeg->vertex1) {
            linedef.fineangle = pDstSeg->angle >> ANGLETOFINESHIFT;
        }

        ++pSrcSeg;
        ++pDstSeg;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map subsectors using data from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadSubSectors(const int32_t lumpNum) noexcept {
    // Sanity check the subsectors lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadSubsectors: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Alloc ram for the runtime subsectors and zero initialize
    gNumSubsectors = lumpSize / sizeof(mapsubsector_t);
    gpSubsectors = (subsector_t*) Z_Malloc(*gpMainMemZone, gNumSubsectors * sizeof(subsector_t), PU_LEVEL, nullptr);
    D_memset(gpSubsectors, std::byte(0), gNumSubsectors * sizeof(subsector_t));

    // Read the map lump containing the subsectors into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);
    
    // Process the WAD subsectors and convert them into runtime subsectors
    const mapsubsector_t* pSrcSubsec = (const mapsubsector_t*) pTmpBufferBytes;
    subsector_t* pDstSubsec = gpSubsectors;

    for (int32_t subsectorIdx = 0; subsectorIdx < gNumSubsectors; ++subsectorIdx) {
        pDstSubsec->numsegs = Endian::littleToHost(pSrcSubsec->numsegs);
        pDstSubsec->firstseg = Endian::littleToHost(pSrcSubsec->firstseg);
        pDstSubsec->numLeafEdges = 0;
        pDstSubsec->firstLeafEdge = 0;

        #if PSYDOOM_MODS
            pDstSubsec->vkDrawSubsecIdx = -1;   // Initialize as 'not drawn' for new the Vulkan renderer
        #endif

        ++pSrcSubsec;
        ++pDstSubsec;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map sector data from the specified map lump number.
// Also sets up the sky texture pointer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadSectors(const int32_t lumpNum) noexcept {
    // Store the name of the sky lump here
    char skyLumpName[8] = {};
    skyLumpName[0] = 'S';
    skyLumpName[1] = 'K';
    skyLumpName[2] = 'Y';

    // Sanity check the sectors lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadSectors: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Alloc ram for the runtime sectors and zero initialize
    if (gbLoadingFinalDoomMap) {
        gNumSectors = lumpSize / sizeof(mapsector_final_t);
    } else {
        gNumSectors = lumpSize / sizeof(mapsector_t);
    }

    gpSectors = (sector_t*) Z_Malloc(*gpMainMemZone, gNumSectors * sizeof(sector_t), PU_LEVEL, nullptr);
    D_memset(gpSectors, std::byte(0), gNumSectors * sizeof(sector_t));

    // Read the map lump containing the sectors into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);

    // Process the WAD sectors and convert them into runtime sectors
    auto processWadSectors = [&](auto pWadSectors) noexcept {
        // Which sector type are we dealing with and is it Final Doom?
        typedef std::remove_reference_t<decltype(*pWadSectors)> wadsector_t;
        constexpr bool bFinalDoom = std::is_same_v<wadsector_t, mapsector_final_t>;

        // This is required for the Final Doom case
        const int32_t firstSkyTexPic = W_GetNumForName("F_SKY01") - gFirstFlatLumpNum;

        // Process the sectors
        const wadsector_t* pSrcSec = pWadSectors;
        sector_t* pDstSec = gpSectors;

        for (int32_t secIdx = 0; secIdx < gNumSectors; ++secIdx) {
            // Save basic properties
            pDstSec->floorheight = d_int_to_fixed(Endian::littleToHost(pSrcSec->floorheight));
            pDstSec->ceilingheight = d_int_to_fixed(Endian::littleToHost(pSrcSec->ceilingheight));
            pDstSec->colorid = Endian::littleToHost(pSrcSec->colorid);
            pDstSec->lightlevel = Endian::littleToHost(pSrcSec->lightlevel);
            pDstSec->special = Endian::littleToHost(pSrcSec->special);
            pDstSec->thinglist = nullptr;
            pDstSec->tag = Endian::littleToHost(pSrcSec->tag);
            pDstSec->flags = Endian::littleToHost(pSrcSec->flags);

            if constexpr (bFinalDoom) {
                // Final Doom specific stuff: we have the actual floor and ceiling texture indexes in this case: no lookup needed!
                const int32_t ceilingPic = Endian::littleToHost(pSrcSec->ceilingpic);

                pDstSec->floorpic = Endian::littleToHost(pSrcSec->floorpic);
                pDstSec->ceilingpic = ceilingPic;

                // Note: if the ceiling has a sky then remove it's texture figure out the sky lump name for it - will load the sky lump later on
                if (ceilingPic >= firstSkyTexPic) {
                    pDstSec->ceilingpic = -1;

                    // PsyDoom: add support for more than 9 sky variants (up to 99)
                    #if PSYDOOM_MODS
                        skyLumpName[3] = '0' + (char)((ceilingPic - firstSkyTexPic) / 10);
                        skyLumpName[4] = '1' + (char)((ceilingPic - firstSkyTexPic) % 10);
                    #else
                        skyLumpName[3] = '0';
                        skyLumpName[4] = '1' + (char)(ceilingPic - firstSkyTexPic);     // Note: only supports up to 9 sky variants!
                    #endif
                }
            } else {
                // Original PSX Doom specific stuff, have to lookup flat numbers from names.
                // First, figure out the floor texture number:
                pDstSec->floorpic = R_FlatNumForName(pSrcSec->floorpic);

                #if PSYDOOM_MODS
                    // PsyDoom: if the floor texture is not found issue a descriptive error rather than crashing later
                    if (pDstSec->floorpic == -1) {
                        char picname[wadsector_t::MAXNAME + 1];
                        std::memcpy(picname, pSrcSec->floorpic, wadsector_t::MAXNAME);
                        picname[wadsector_t::MAXNAME] = 0;
                        I_Error("R_FlatNumForName: can't find '%s'!", picname);
                    }
                #endif

                // Figure out ceiling texture numebr.
                // Note: if the ceiling has a sky then figure out the sky lump name for it instead - will load the sky lump later on.
                const bool bCeilHasSky = (D_strncasecmp(pSrcSec->ceilingpic, SKY_LUMP_NAME, SKY_LUMP_NAME_LEN) == 0);

                if (bCeilHasSky) {
                    // No ceiling texture: extract and save the 2 digits for the sky number ('01', '02' etc.)
                    pDstSec->ceilingpic = -1;
                    skyLumpName[3] = pSrcSec->ceilingpic[5];
                    skyLumpName[4] = pSrcSec->ceilingpic[6];
                } else {
                    // Normal case: ceiling has a texture, save it's number
                    pDstSec->ceilingpic = R_FlatNumForName(pSrcSec->ceilingpic);
                }
            }

            ++pSrcSec;
            ++pDstSec;
        }
    };

    // Process the sectors found in the WAD file as Final Doom or original Doom format
    if (gbLoadingFinalDoomMap) {
        processWadSectors((mapsector_final_t*) pTmpBufferBytes);
    } else {
        processWadSectors((mapsector_t*) pTmpBufferBytes);
    }

    // Set the sky texture pointer
    if (skyLumpName[3] != 0) {
        const int32_t skyTexIdx = R_TextureNumForName(skyLumpName);
        gpSkyTexture = &gpTextures[skyTexIdx];
    } else {
        gpSkyTexture = nullptr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map bsp nodes from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadNodes(const int32_t lumpNum) noexcept {
    // Sanity check the nodes lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    
    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadNodes: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Alloc ram for the runtime nodes
    gNumBspNodes = lumpSize / sizeof(mapnode_t);
    gpBspNodes = (node_t*) Z_Malloc(*gpMainMemZone, gNumBspNodes * sizeof(node_t), PU_LEVEL, nullptr);

    // Read the map lump containing the nodes into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);

    // Process the WAD nodes and convert them into runtime nodes.
    // The format for nodes on the PSX appears identical to PC.
    const mapnode_t* pSrcNode = (const mapnode_t*) pTmpBufferBytes;
    node_t* pDstNode = gpBspNodes;

    for (int32_t nodeIdx = 0; nodeIdx < gNumBspNodes; ++nodeIdx) {
        pDstNode->line.x = d_int_to_fixed(Endian::littleToHost(pSrcNode->x));
        pDstNode->line.y = d_int_to_fixed(Endian::littleToHost(pSrcNode->y));
        pDstNode->line.dx = d_int_to_fixed(Endian::littleToHost(pSrcNode->dx));
        pDstNode->line.dy = d_int_to_fixed(Endian::littleToHost(pSrcNode->dy));

        for (int32_t childIdx = 0; childIdx < 2; ++childIdx) {
            pDstNode->children[childIdx] = Endian::littleToHost(pSrcNode->children[childIdx]);

            for (int32_t coordIdx = 0; coordIdx < 4; ++coordIdx) {
                const fixed_t coord = d_int_to_fixed(Endian::littleToHost(pSrcNode->bbox[childIdx][coordIdx]));
                pDstNode->bbox[childIdx][coordIdx] = coord;
            }
        }

        ++pSrcNode;
        ++pDstNode;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map things and spawn them using data from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadThings(const int32_t lumpNum) noexcept {
    // Sanity check the things lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadThings: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Determine how many things there are to spawn and read the lump from the WAD
    const int32_t numThings = lumpSize / sizeof(mapthing_t);
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);

    // Spawn the map things
    mapthing_t* pSrcThing = (mapthing_t*) pTmpBufferBytes;

    for (int32_t thingIdx = 0; thingIdx < numThings; ++thingIdx) {
        // Endian correct the map thing and then spawn it
        pSrcThing->x = Endian::littleToHost(pSrcThing->x);
        pSrcThing->y = Endian::littleToHost(pSrcThing->y);
        pSrcThing->angle = Endian::littleToHost(pSrcThing->angle);
        pSrcThing->type = Endian::littleToHost(pSrcThing->type);
        pSrcThing->options = Endian::littleToHost(pSrcThing->options);

        P_SpawnMapThing(*pSrcThing);

        // Not sure why this check is being done AFTER we try and spawn the thing, surely it would make more sense to do before?
        // Regardless if the 'DoomEd' number is too big then throw an error:
        if (pSrcThing->type >= 4096) {
            I_Error("P_LoadThings: doomednum:%d >= 4096", (int32_t) pSrcThing->type);
        }

        ++pSrcThing;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load linedefs from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadLineDefs(const int32_t lumpNum) noexcept {
    // Sanity check the linedefs lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadLineDefs: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Alloc ram for the runtime linedefs and zero initialize
    gNumLines = lumpSize / sizeof(maplinedef_t);
    gpLines = (line_t*) Z_Malloc(*gpMainMemZone, gNumLines * sizeof(line_t), PU_LEVEL, nullptr);
    D_memset(gpLines, std::byte(0), gNumLines * sizeof(line_t));

    // Read the map lump containing the sidedefs into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);

    // Process the WAD linedefs and convert them into runtime linedefs
    const maplinedef_t* pSrcLine = (maplinedef_t*) pTmpBufferBytes;
    line_t* pDstLine = gpLines;

    for (int32_t lineIdx = 0; lineIdx < gNumLines; ++lineIdx) {
        // Save some basic line properties
        pDstLine->flags = Endian::littleToHost(pSrcLine->flags);
        pDstLine->special = Endian::littleToHost(pSrcLine->special);
        pDstLine->tag = Endian::littleToHost(pSrcLine->tag);

        // Save line vertices, delta coordinates and slope type
        vertex_t& vertex1 = gpVertexes[Endian::littleToHost(pSrcLine->vertex1)];
        vertex_t& vertex2 = gpVertexes[Endian::littleToHost(pSrcLine->vertex2)];

        pDstLine->vertex1 = &vertex1;
        pDstLine->vertex2 = &vertex2;
        pDstLine->dx = vertex2.x - vertex1.x;
        pDstLine->dy = vertex2.y - vertex1.y;

        if (pDstLine->dx == 0) {
            pDstLine->slopetype = slopetype_t::ST_VERTICAL;
        } else if (pDstLine->dy == 0) {
            pDstLine->slopetype = slopetype_t::ST_HORIZONTAL;
        } else {
            if (FixedDiv(pDstLine->dy, pDstLine->dx) > 0) {
                pDstLine->slopetype = slopetype_t::ST_POSITIVE;
            } else {
                pDstLine->slopetype = slopetype_t::ST_NEGATIVE;
            }
        }

        // Save line bounding box
        if (vertex1.x < vertex2.x) {
            pDstLine->bbox[BOXLEFT] = vertex1.x;
            pDstLine->bbox[BOXRIGHT] = vertex2.x;
        } else {
            pDstLine->bbox[BOXLEFT] = vertex2.x;
            pDstLine->bbox[BOXRIGHT] = vertex1.x;
        }

        if (vertex1.y < vertex2.y) {
            pDstLine->bbox[BOXBOTTOM] = vertex1.y;
            pDstLine->bbox[BOXTOP] = vertex2.y;
        } else {
            pDstLine->bbox[BOXBOTTOM] = vertex2.y;
            pDstLine->bbox[BOXTOP] = vertex1.y;
        }

        // Save side numbers and sector references
        const int32_t sidenum1 = Endian::littleToHost(pSrcLine->sidenum[0]);
        const int32_t sidenum2 = Endian::littleToHost(pSrcLine->sidenum[1]);

        pDstLine->sidenum[0] = sidenum1;
        pDstLine->sidenum[1] = sidenum2;

        if (sidenum1 != -1) {
            side_t& side = gpSides[sidenum1];
            pDstLine->frontsector = side.sector;
        } else {
            pDstLine->frontsector = nullptr;
        }

        if (sidenum2 != -1) {
            side_t& side = gpSides[sidenum2];
            pDstLine->backsector = side.sector;
        } else {
            pDstLine->backsector = nullptr;
        }

        ++pSrcLine;
        ++pDstLine;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load side definitions from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadSideDefs(const int32_t lumpNum) noexcept {
    // Sanity check the sidedefs lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadSideDefs: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Alloc ram for the runtime sidedefs and zero initialize
    if (gbLoadingFinalDoomMap) {
        gNumSides = lumpSize / sizeof(mapsidedef_final_t);
    } else {
        gNumSides = lumpSize / sizeof(mapsidedef_t);
    }

    gpSides = (side_t*) Z_Malloc(*gpMainMemZone, gNumSides * sizeof(side_t), PU_LEVEL, nullptr);
    D_memset(gpSides, std::byte(0), gNumSides  * sizeof(side_t));

    // Read the map lump containing the sidedefs into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);

    // Process the WAD sidedefs and convert them into runtime sidedefs
    auto processWadSidedefs = [&](auto pWadSidedefs) noexcept {
        // Which sidedef type are we dealing with and is it Final Doom?
        typedef std::remove_reference_t<decltype(*pWadSidedefs)> wadsidedef_t;
        constexpr bool bFinalDoom = std::is_same_v<wadsidedef_t, mapsidedef_final_t>;

        const wadsidedef_t* pSrcSide = pWadSidedefs;
        side_t* pDstSide = gpSides;

        for (int32_t sideIdx = 0; sideIdx < gNumSides; ++sideIdx) {
            pDstSide->textureoffset = d_int_to_fixed(Endian::littleToHost(pSrcSide->textureoffset));
            pDstSide->rowoffset = d_int_to_fixed(Endian::littleToHost(pSrcSide->rowoffset));
            pDstSide->sector = &gpSectors[Endian::littleToHost(pSrcSide->sector)];

            // For Final Doom we don't need to do any lookup, we have the numbers already...
            if constexpr (bFinalDoom) {
                pDstSide->toptexture = Endian::littleToHost(pSrcSide->toptexture);
                pDstSide->midtexture = Endian::littleToHost(pSrcSide->midtexture);
                pDstSide->bottomtexture = Endian::littleToHost(pSrcSide->bottomtexture);
            } else {
                pDstSide->toptexture = R_TextureNumForName(pSrcSide->toptexture);
                pDstSide->midtexture = R_TextureNumForName(pSrcSide->midtexture);
                pDstSide->bottomtexture = R_TextureNumForName(pSrcSide->bottomtexture);
            }

            ++pSrcSide;
            ++pDstSide;
        }
    };

    // Process the sides found in the WAD file as Final Doom or original Doom format
    if (gbLoadingFinalDoomMap) {
        processWadSidedefs((mapsidedef_final_t*) pTmpBufferBytes);
    } else {
        processWadSidedefs((mapsidedef_t*) pTmpBufferBytes);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the blockmap from the specified map lump number.
// For more details about the blockmap, see: https://doomwiki.org/wiki/Blockmap
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadBlockMap(const int32_t lumpNum) noexcept {
    // Read the blockmap lump into RAM
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    gpBlockmapLump = (uint16_t*) Z_Malloc(*gpMainMemZone, lumpSize, PU_LEVEL, nullptr);
    W_ReadMapLump(lumpNum, gpBlockmapLump, true);

    // The first 8 bytes of the blockmap are it's header
    struct blockmap_hdr_t {
        int16_t     originx;
        int16_t     originy;
        int16_t     width;
        int16_t     height;
    };

    static_assert(sizeof(blockmap_hdr_t) == 8);
    blockmap_hdr_t& blockmapHeader = *(blockmap_hdr_t*) gpBlockmapLump;

    // The offsets to each blocklist (list of block lines per block) start after the header
    gpBlockmap = (uint16_t*)(&(&blockmapHeader)[1]);

    // Endian correction for the entire blockmap lump.
    //
    // PsyDoom: skip doing this on little endian architectures to save a few cycles.
    // The original code did this transform from little endian to little endian even though it had no effect...
    #if PSYDOOM_MODS
        constexpr bool bEndianCorrect = (!Endian::isLittle());
    #else
        constexpr bool bEndianCorrect = true;
    #endif

    if constexpr (bEndianCorrect) {
        static_assert(sizeof(gpBlockmapLump[0]) == sizeof(int16_t));
        const int32_t count = (lumpSize / 2) + (lumpSize & 1);

        for (int32_t i = 0; i < count; ++i) {
            gpBlockmapLump[i] = Endian::littleToHost(gpBlockmapLump[i]);
        }
    }

    // Save blockmap dimensions
    gBlockmapWidth = blockmapHeader.width;
    gBlockmapHeight = blockmapHeader.height;
    gBlockmapOriginX = d_int_to_fixed(blockmapHeader.originx);
    gBlockmapOriginY = d_int_to_fixed(blockmapHeader.originy);
    
    // Alloc and null initialize the list of map objects for each block
    const int32_t blockLinksSize = (int32_t) blockmapHeader.width * (int32_t) blockmapHeader.height * sizeof(gppBlockLinks[0]);
    gppBlockLinks = (mobj_t**) Z_Malloc(*gpMainMemZone, blockLinksSize, PU_LEVEL, nullptr);
    D_memset(gppBlockLinks, std::byte(0), blockLinksSize);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the reject map from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadRejectMap(const int32_t lumpNum) noexcept {
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    gpRejectMatrix = (uint8_t*) Z_Malloc(*gpMainMemZone, lumpSize, PU_LEVEL, nullptr);
    W_ReadMapLump(lumpNum, gpRejectMatrix, true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load leafs from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadLeafs(const int32_t lumpNum) noexcept {
    // Sanity check the leafs lump is not too big.
    // PsyDoom: if limit removing, just ensure the buffer is big enough instead - any size of data is allowed.
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    
    #if PSYDOOM_LIMIT_REMOVING
        gTmpBuffer.ensureSize(lumpSize);
        std::byte* const pTmpBufferBytes = gTmpBuffer.bytes();
    #else
        if (lumpSize > TMP_BUFFER_SIZE) {
            I_Error("P_LoadLeafs: lump > 64K");
        }

        std::byte* const pTmpBufferBytes = gTmpBuffer;
    #endif

    // Read the map lump containing the leaf edges into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);
    const std::byte* const pLumpBeg = pTmpBufferBytes;
    const std::byte* const pLumpEnd = pTmpBufferBytes + lumpSize;

    // Determine the number of leafs in the lump.
    // The number of leafs MUST equal the number of subsectors, and they must be in the same order as their subsectors.
    int32_t numLeafs = 0;
    int32_t totalLeafEdges = 0;

    for (const std::byte* pLumpByte = pLumpBeg; pLumpByte < pLumpEnd;) {
        // Increment the leaf count, convert to host endian and skip past it
        mapleaf_t leaf = *(mapleaf_t*) pLumpByte;
        leaf.numedges = Endian::littleToHost(leaf.numedges);
        pLumpByte += sizeof(mapleaf_t);
        ++numLeafs;
        
        // Skip past the leaf edges and include them in the leaf edge count
        pLumpByte += leaf.numedges * sizeof(mapleafedge_t);
        totalLeafEdges += leaf.numedges;
    }

    if (numLeafs != gNumSubsectors) {
        I_Error("P_LoadLeafs: leaf/subsector inconsistancy");
    }

    // Allocate room for all the leaf edges
    gpLeafEdges = (leafedge_t*) Z_Malloc(*gpMainMemZone, totalLeafEdges * sizeof(leafedge_t), PU_LEVEL, nullptr);
    
    // Convert WAD leaf edges to runtime leaf edges and link them in with other map data structures
    gTotalNumLeafEdges = 0;

    const std::byte* pLumpByte = pTmpBufferBytes;
    subsector_t* pSubsec = gpSubsectors;
    leafedge_t* pDstEdge = gpLeafEdges;

    for (int32_t leafIdx = 0; leafIdx < numLeafs; ++leafIdx) {
        // Convert leaf data to host endian and move past it
        mapleaf_t leaf = *(mapleaf_t*) pLumpByte;
        leaf.numedges = Endian::littleToHost(leaf.numedges);
        pLumpByte += sizeof(mapleaf_t);

        // Save leaf info on the subsector
        pSubsec->numLeafEdges = leaf.numedges;
        pSubsec->firstLeafEdge = (int16_t) gTotalNumLeafEdges;

        // Process the edges in the leaf
        for (int32_t edgeIdx = 0; edgeIdx < pSubsec->numLeafEdges; ++edgeIdx) {
            // Convert edge data to host endian and move past it
            mapleafedge_t srcEdge = *(const mapleafedge_t*) pLumpByte;
            srcEdge.segnum = Endian::littleToHost(srcEdge.segnum);
            srcEdge.vertexnum = Endian::littleToHost(srcEdge.vertexnum);

            pLumpByte += sizeof(mapleafedge_t);

            // Set leaf vertex reference
            if (srcEdge.vertexnum >= gNumVertexes) {
                I_Error("P_LoadLeafs: vertex out of range\n");
            }

            pDstEdge->vertex = &gpVertexes[srcEdge.vertexnum];

            // Set leaf seg reference
            if (srcEdge.segnum == -1) {
                pDstEdge->seg = nullptr;
            } else {
                if (srcEdge.segnum >= gNumSegs) {
                    I_Error("P_LoadLeafs: seg out of range\n");
                }

                pDstEdge->seg = &gpSegs[srcEdge.segnum];
            }

            ++pDstEdge;
        }

        // Move along to the next leaf
        gTotalNumLeafEdges += pSubsec->numLeafEdges;
        ++pSubsec;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Builds the line lists for each sector, bounding boxes as well as sound origin points
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_GroupLines() noexcept {
    // Associate subsectors with their sectors
    {
        subsector_t* pSubsec = gpSubsectors;
        
        for (int32_t subsecIdx = 0; subsecIdx < gNumSubsectors; ++subsecIdx) {
            const seg_t& seg = gpSegs[pSubsec->firstseg];
            pSubsec->sector = seg.sidedef->sector;
            ++pSubsec;
        }
    }

    // Count the number of lines in each sector and how many line references there are among all sectors
    int32_t totalLineRefs = 0;

    {
        line_t* pLine = gpLines;

        for (int32_t lineIdx = 0; lineIdx < gNumLines; ++lineIdx) {
            ++pLine->frontsector->linecount;
            ++totalLineRefs;

            if (pLine->backsector && pLine->backsector != pLine->frontsector) {
                ++pLine->backsector->linecount;
                ++totalLineRefs;
            }
            
            ++pLine;
        }
    }

    // Alloc the array of line refs that will be shared by all sectors
    line_t** const pLineRefBuffer = (line_t**) Z_Malloc(*gpMainMemZone, totalLineRefs * sizeof(line_t*), PU_LEVEL, nullptr);
    line_t** pLineRef = pLineRefBuffer;

    // Build the list of lines for each sector, also bounding boxes and the 'sound origin' point
    sector_t* pSec = gpSectors;

    for (int32_t secIdx = 0; secIdx < gNumSectors; ++secIdx) {
        // Clear the bounding box for the sector and set the line list start
        fixed_t bbox[4];
        M_ClearBox(bbox);
        pSec->lines = pLineRef;

        // Build up the bounding box and line list for the sector by examining each line in the level against this sector.
        // Not an efficient algorithm, since it is O(N^2) but works OK given the size of the datasets in DOOM.
        // This might be a problem if you are planning on making a DOOM open world game however... :P
        {
            line_t* pLine = gpLines;

            for (int32_t lineIdx = 0; lineIdx < gNumLines; ++lineIdx) {
                // Does this line belong to this sector?
                // If so save the line reference in the sector line list and add to the sector bounding box.
                if (pLine->frontsector == pSec || pLine->backsector == pSec) {
                    *pLineRef = pLine;
                    ++pLineRef;

                    M_AddToBox(bbox, pLine->vertex1->x, pLine->vertex1->y);
                    M_AddToBox(bbox, pLine->vertex2->x, pLine->vertex2->y);
                }

                ++pLine;
            }
        }

        // Sanity check the size of the line list we built is what we would expect.
        // It should not contradict the line count for the sector.
        const int32_t actualLineCount = (int32_t)(pLineRef - pSec->lines);

        if (actualLineCount != pSec->linecount) {
            I_Error("P_GroupLines: miscounted");
        }

        // Set the sound origin location for sector sounds and also the subsector.
        // Use the bounding box center for this.
        {
            degenmobj_t& soundorg = pSec->soundorg;
            soundorg.x = (bbox[BOXLEFT] + bbox[BOXRIGHT]) / 2;
            soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;

            #if PSYDOOM_MODS && PSYDOOM_FIX_UB
                // The original code did not appear to initialize the 'z' field!
                // I'm not sure it's used for sound code but give it a defined value of midway up in the air for good measure.
                soundorg.z = (pSec->floorheight + pSec->ceilingheight) / 2;
            #endif

            pSec->soundorg.subsector = R_PointInSubsector(soundorg.x, soundorg.y);
        }

        // Compute the bounding box for the sector in blockmap units.
        // Note that if the sector extends the beyond the blockmap then we constrain it's coordinate.
        {
            int32_t bmcoord = d_rshift<MAPBLOCKSHIFT>(bbox[BOXTOP] - gBlockmapOriginY + MAXRADIUS);
            bmcoord = (bmcoord >= gBlockmapHeight) ? gBlockmapHeight - 1 : bmcoord;
            pSec->blockbox[BOXTOP] = bmcoord;
        }
        {
            int32_t bmcoord = d_rshift<MAPBLOCKSHIFT>(bbox[BOXBOTTOM] - gBlockmapOriginY - MAXRADIUS);
            bmcoord = (bmcoord < 0) ? 0 : bmcoord;
            pSec->blockbox[BOXBOTTOM] = bmcoord;
        }
        {
            int32_t bmcoord = d_rshift<MAPBLOCKSHIFT>(bbox[BOXRIGHT] - gBlockmapOriginX + MAXRADIUS);
            bmcoord = (bmcoord >= gBlockmapWidth) ? gBlockmapWidth - 1 : bmcoord;
            pSec->blockbox[BOXRIGHT] = bmcoord;
        }
        {
            int32_t bmcoord = d_rshift<MAPBLOCKSHIFT>(bbox[BOXLEFT] - gBlockmapOriginX - MAXRADIUS);
            bmcoord = (bmcoord < 0) ? 0 : bmcoord;
            pSec->blockbox[BOXLEFT] = bmcoord;
        }

        ++pSec;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads wall, floor and switch textures into VRAM.
// For animated textures the first frame will be put into VRAM and the rest of the animation cached in main RAM.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_Init() noexcept {
    // PsyDoom: make sure all animated textures and flats use the base picture - the engine expects this:
    #if PSYDOOM_MODS
        P_SetAnimsToBasePic();
    #endif

    // Load sector flats into VRAM if not already there
    {
        const sector_t* pSec = gpSectors;

        for (int32_t secIdx = 0; secIdx < gNumSectors; ++secIdx, ++pSec) {
            // Note: ceiling might not have a texture (sky)
            if (pSec->ceilingpic != -1) {
                texture_t& ceilTex = gpFlatTextures[pSec->ceilingpic];

                if (ceilTex.texPageId == 0) {
                    I_CacheTex(ceilTex);
                }
            }

            texture_t& floorTex = gpFlatTextures[pSec->floorpic];

            if (floorTex.texPageId == 0) {
                I_CacheTex(floorTex);
            }
        }
    }
    
    // Lock the texture page (2nd page) used exclusively by flat textures.
    // Also force the fill location in VRAM to the 3rd page after we add flats.
    //
    // This code makes the following assumptions:
    //  (1) The 2nd page can ONLY contain flat textures.
    //      Even if not completely full it's put off limits and 'locked' before we can use it for other stuff.
    //  (2) Flats will NEVER use more than one texture page.
    //      This therefore restricts the maximum number of flats to 16 because even if we add more, they will
    //      be evicted by any wall texture loads below - since we forced the fill location to the start of the 3rd page.
    //
    // So if you want to support more than 16 flats or do a more flexible VRAM arrangement, this is the place to look..
    gLockedTexPagesMask |= 0b0000'0000'0010;
    gTCacheFillPage = 2;
    gTCacheFillCellX = 0;
    gTCacheFillCellY = 0;
    gTCacheFillRowCellH = 0;

    // Initialize the sky texture, palette and do some more specialized setup if it's the fire sky
    gUpdateFireSkyFunc = nullptr;
    gPaletteClutId_CurMapSky = gPaletteClutIds[MAINPAL];

    if (gpSkyTexture) {
        // If the lump name for the sky follows the format 'xxxx9' then assume it is a fire sky.
        // That needs to have it's lump cached, palette & update function set and initial few updates done...
        texture_t& skyTex = *gpSkyTexture;
        const lumpinfo_t& skyTexLump = gpLumpInfo[skyTex.lumpNum];

        if (skyTexLump.name.chars[4] == '9') {
            W_CacheLumpNum(skyTex.lumpNum, PU_ANIMATION, true);
            gPaletteClutId_CurMapSky = gPaletteClutIds[FIRESKYPAL];
            gUpdateFireSkyFunc = P_UpdateFireSky;
            
            // This gets the fire going, so it doesn't take a while to creep up when the map is started.
            // Do a number of fire update rounds before the player even enters the map:
            for (int32_t i = 0; i < 64; ++i) {
                P_UpdateFireSky(skyTex);
            }
        }

        // Final Doom: if the last digit in 'SKYXX' matches one of these digits, then use whatever palette is for that sky:
        if (gbLoadingFinalDoomMap) {
            switch (skyTexLump.name.chars[4]) {
                case '2':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL1];    break;
                case '3':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL2];    break;
                case '4':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL3];    break;
                case '5':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL4];    break;
                case '6':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL5];    break;
            }
        }

        // Ensure the sky texture is in VRAM
        I_CacheTex(skyTex);
    }

    // Load wall and switch textures into VRAM.
    // Note that switches are assumed to be 64 pixels wide, hence cached just before the 128 pixel textures.
    P_CacheMapTexturesWithWidth(16);
    P_CacheMapTexturesWithWidth(64);
    P_InitSwitchList();
    P_CacheMapTexturesWithWidth(128);

    // Give all sides without textures a default one.
    // Maybe done so constant validity checks don't have to be done elsewhere in the game to avoid crashing?
    {
        side_t* pSide = gpSides;

        for (int32_t sideIdx = 0; sideIdx < gNumSides; ++sideIdx, ++pSide) {
            if (pSide->toptexture == -1) {
                pSide->toptexture = 0;
            }
            
            if (pSide->midtexture == -1) {
                pSide->midtexture = 0;
            }

            if (pSide->bottomtexture == -1) {
                pSide->bottomtexture = 0;
            }
        }
    }
    
    // Some more hardcoded limits and memory arrangement for textures, this time for wall textures.
    // Lock down the texture pages used by wall textures and set the cache to start filling immediately after that (on the 6th page).
    //
    // Basically only the 3rd, 4th and 5th texture pages can be used for wall textures.
    // This gives a maximum wall texture area of 768x256 - everything else after that is reserved for sprites.
    // Even if the code above fills in more textures, they will eventually be evicted from the cache in favor of sprites.
    gLockedTexPagesMask |= 0b0000'0001'1100;
    gTCacheFillPage = 5;
    gTCacheFillCellX = 0;
    gTCacheFillCellY = 0;
    gTCacheFillRowCellH = 0;
    
    // Clear out any floor or wall textures we had temporarily in RAM from the above caching.
    //
    // Small optimization opportunity: this is slightly wasteful in that if we have animated walls/floors, then they will be evicted by this
    // call here and then reloaded immediately again in the call to 'P_InitPicAnims', but under a different zone memory manager tag.
    // We could perhaps change the memory tag and retain them in RAM if still needed for animation?
    Z_FreeTags(*gpMainMemZone, PU_CACHE);

    // Load animated wall and flat textures into RAM so they are ready when needed.
    // These will be uploaded dynamically into VRAM at runtime, as the engine animates the flat or wall texture.
    P_InitPicAnims();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads the specified level, textures and sets it up for gameplay by spawning things, players etc.
// Note: while most of the loading and setup is done here for the level, sound and music are handled eleswhere.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SetupLevel(const int32_t mapNum, [[maybe_unused]] const skill_t skill) noexcept {
    // Cleanup of memory and resetting the RNG before we start
    Z_FreeTags(*gpMainMemZone, PU_CACHE | PU_LEVSPEC| PU_LEVEL);

    if (!gbIsLevelBeingRestarted) {
        gLockedTexPagesMask &= 1;
        Z_FreeTags(*gpMainMemZone, PU_ANIMATION);
    }
    
    I_PurgeTexCache();
    Z_CheckHeap(*gpMainMemZone);
    M_ClearRandom();

    // Init player stats for the map
    gTotalKills = 0;
    gTotalItems = 0;
    gTotalSecret = 0;

    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        player_t& player = gPlayers[playerIdx];

        player.killcount = 0;
        player.secretcount = 0;
        player.itemcount = 0;
        player.frags = 0;
    }

    // Setup the thinkers and map objects lists
    gThinkerCap.prev = &gThinkerCap;
    gThinkerCap.next = &gThinkerCap;

    gMObjHead.next = &gMObjHead;
    gMObjHead.prev = &gMObjHead;

    // Setup the item respawn queue and dead player removal queue index
    gItemRespawnQueueHead = 0;
    gItemRespawnQueueTail = 0;
    gDeadPlayerRemovalQueueIdx = 0;

    // Figure out which file to open for the map WAD.
    //
    // Note: for Final Doom I've added logic to allow for MAPXX.WAD or MAPXX.ROM, with a preference for the .WAD file.
    // Also, if the file for the map has the .ROM extension then it is assumed the map data is in Final Doom format.
    const int32_t mapIndex = mapNum - 1;
    const int32_t mapFolderIdx = mapIndex / LEVELS_PER_MAP_FOLDER;
    const int32_t mapIdxInFolder = mapIndex - mapFolderIdx * LEVELS_PER_MAP_FOLDER;
    const int32_t mapFolderOffset = mapFolderIdx * NUM_FILES_PER_LEVEL * LEVELS_PER_MAP_FOLDER;

    const CdFileId mapWadFile_doom = (CdFileId)((int32_t) CdFileId::MAP01_WAD + mapIdxInFolder + mapFolderOffset);
    const CdFileId mapWadFile_finalDoom = (CdFileId)((int32_t) CdFileId::MAP01_ROM + mapIndex);
    gbLoadingFinalDoomMap = (!gCdMapTbl[(int32_t) mapWadFile_doom].startSector);

    const CdFileId mapWadFile = (gbLoadingFinalDoomMap) ? mapWadFile_finalDoom : mapWadFile_doom;
    
    // Open the map wad
    void* const pMapWadFileData = W_OpenMapWad(mapWadFile);

    // Figure out the name of the map start lump marker
    char mapLumpName[8] = {};
    mapLumpName[0] = 'M';
    mapLumpName[1] = 'A';
    mapLumpName[2] = 'P';

    {
        uint8_t digit1 = (uint8_t) mapNum / 10;
        uint8_t digit2 = (uint8_t) mapNum - digit1 * 10;
        mapLumpName[3] = '0' + digit1;
        mapLumpName[4] = '0' + digit2;
    }
    
    // Get the lump index for the map start lump
    const uint32_t mapStartLump = W_MapCheckNumForName(mapLumpName);
    
    if (mapStartLump == -1) {
        I_Error("P_SetupLevel: %s not found", mapLumpName);
    }

    // Loading various map lumps
    P_LoadBlockMap(mapStartLump + ML_BLOCKMAP);
    P_LoadVertexes(mapStartLump + ML_VERTEXES);
    P_LoadSectors(mapStartLump + ML_SECTORS);
    P_LoadSideDefs(mapStartLump + ML_SIDEDEFS);
    P_LoadLineDefs(mapStartLump + ML_LINEDEFS);
    P_LoadSubSectors(mapStartLump + ML_SSECTORS);
    P_LoadNodes(mapStartLump + ML_NODES);
    P_LoadSegs(mapStartLump + ML_SEGS);
    P_LoadLeafs(mapStartLump + ML_LEAFS);
    P_LoadRejectMap(mapStartLump + ML_REJECT);

    // Build sector line lists etc.
    P_GroupLines();

    // Load and spawn map things. Also initialize the next deathmatch start.
    gpDeathmatchP = &gDeathmatchStarts[0];
    P_LoadThings(mapStartLump + ML_THINGS);
    
    // Spawn special thinkers such as light flashes etc. and free up the loaded WAD data
    P_SpawnSpecials();
    Z_Free2(*gpMainMemZone, pMapWadFileData);

    // Loading map textures and sprites.
    //
    // PsyDoom limit removing: loading .IMG files for maps (e.g  MAPSPR01.IMG and MAPTEX01.IMG) is no longer done to make modding easier.
    // Instead, all graphical resources will be loaded from WAD files. The .IMG files no longer need to be produced for PsyDoom and we can
    // add new texture etc. lumps to the main IWAD without worrying about invalidating lump number references in existing .IMG files.
    if (!gbIsLevelBeingRestarted) {
        #if !PSYDOOM_LIMIT_REMOVING
            const CdFileId mapTexFile = (CdFileId)((int32_t) CdFileId::MAPTEX01_IMG + mapIdxInFolder + mapFolderOffset);
            const CdFileId mapSprFile = (CdFileId)((int32_t) CdFileId::MAPSPR01_IMG + mapIdxInFolder + mapFolderOffset);
            P_LoadBlocks(mapTexFile);
        #endif

        P_Init();

        #if !PSYDOOM_LIMIT_REMOVING
            P_LoadBlocks(mapSprFile);
        #endif
    }

    // PsyDoom: precache all sprites needed for the level
    #if PSYDOOM_MODS
        MobjSpritePrecacher::doPrecaching();
    #endif

    // Check there is enough heap space left in order to run the level
    const int32_t freeMemForGameplay = Z_FreeMemory(*gpMainMemZone);

    if (freeMemForGameplay < MIN_REQ_HEAP_SPACE_FOR_GAMEPLAY) {
        Z_DumpHeap();
        I_Error("P_SetupLevel: not enough free memory %d", freeMemForGameplay);
    }

    // Spawn the player(s)
    if (gNetGame != gt_single) {
        I_NetHandshake();
        
        // Randomly spawn players in different locations - this logic is a little strange.
        // We spawn all players in the same location but immediately respawn and remove the old 'mobj_t' to get the random starts.
        for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
            mobj_t* const pPlayerThing = P_SpawnMobj(
                d_int_to_fixed(gPlayerStarts[0].x),
                d_int_to_fixed(gPlayerStarts[0].y),
                0,
                MT_PLAYER
            );

            gPlayers[playerIdx].mo = pPlayerThing;
            G_DoReborn(playerIdx);
            P_RemoveMobj(*pPlayerThing);
        }
    }
    else {
        P_SpawnPlayer(gPlayerStarts[0]);
    }
}

// PsyDoom limit removing: loading .IMG files for maps (e.g  MAPSPR01.IMG and MAPTEX01.IMG) is no longer done to make modding easier.
// Instead, all graphical resources will be loaded from WAD files. The .IMG files no longer need to be produced for PsyDoom and we can
// add new texture etc. lumps to the main IWAD without worrying about invalidating lump number references in existing .IMG files.
#if !PSYDOOM_LIMIT_REMOVING
//------------------------------------------------------------------------------------------------------------------------------------------
// Loads a list of memory blocks containing WAD lumps from the given file.
//
// PsyDoom: this function had to be completely rewritten since 'sizeof(memblock_t)' is no longer equal to 'sizeof(fileblock_t)'.
// This is due to pointers being larger in 64-bit mode. See the 'Old' folder for the original version of this function.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadBlocks(const CdFileId file) noexcept {
    // Open the blocks file and get it's size
    const int32_t openFileIdx = OpenFile(file);
    const int32_t fileSize = SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::END);
    SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::SET);

    // Keep reading the memory blocks in the file until we are done
    int32_t bytesLeft = fileSize;

    while (bytesLeft > 0) {
        // Read the header for this block and verify all of it's properties.
        // Note that the size of the block also includes the block header.
        fileblock_t blockHeader = {};
        ReadFile(openFileIdx, &blockHeader, sizeof(fileblock_t));
        bytesLeft -= sizeof(fileblock_t);

        if (blockHeader.id != ZONEID) {
            I_Error("P_LoadBlocks: bad zoneid!");
        }
            
        if (blockHeader.lumpNum >= gNumLumps) {
            I_Error("P_LoadBlocks: bad lumpnum!");
        }

        if (blockHeader.isUncompressed >= 2) {
            I_Error("P_LoadBlocks: bad c-mode!");
        }

        const int32_t blockDataSize = blockHeader.size - sizeof(fileblock_t);

        if ((blockDataSize > bytesLeft) || (blockDataSize <= 0)) {
            I_Error("P_LoadBlocks: bad size!");
        }

        // If this lump is already loaded then we can skip past reading the data for this block
        void*& lumpCacheEntry = gpLumpCache[blockHeader.lumpNum];

        if (lumpCacheEntry) {
            SeekAndTellFile(openFileIdx, blockDataSize, PsxCd_SeekMode::CUR);
            bytesLeft -= blockDataSize;
            continue;
        }

        // Alloc a memory block for this memory block on disk and read the data.
        // Set the user of the memory block to the lump cache entry for this lump (this also populates the lump cache entry):
        uint8_t* const pBlockData = (uint8_t*) Z_Malloc(*gpMainMemZone, blockDataSize, blockHeader.tag, &lumpCacheEntry);
        ReadFile(openFileIdx, pBlockData, blockDataSize);
        bytesLeft -= blockDataSize;

        // Verify the decompressed size is valid if the block was compressed
        if (blockHeader.isUncompressed == 0) {
            // Get the decompressed size of the data following the file block header and make sure it is what we expect
            const uint32_t inflatedSize = getDecodedSize(pBlockData);
            const lumpinfo_t& lump = gpLumpInfo[blockHeader.lumpNum];

            if (inflatedSize != lump.size) {
                // Uh-oh, the data in the lumps file is not what we expect! Get the null terminated lump name and print out the problem:
                char lumpName[MAXLUMPNAME + 1];
                std::memcpy(lumpName, lump.name.chars, MAXLUMPNAME);
                lumpName[MAXLUMPNAME] = 0;

                I_Error(
                    "P_LoadBlocks: bad decompressed size for lump %d (%s)!\n"
                    "The master WAD says it should be %d bytes.\n"
                    "In lump file '%s' it decompresses to %d bytes.",
                    (int) blockHeader.lumpNum,
                    lumpName,
                    (int) lump.size,
                    gCdMapTblFileNames[(size_t) file],
                    (int) inflatedSize
                );
            }
        }

        // Save whether the lump is compressed or not
        gpbIsUncompressedLump[blockHeader.lumpNum] = blockHeader.isUncompressed;
    }
    
    // After all that is done close up the file and check the heap is still in a good state
    CloseFile(openFileIdx);
    Z_CheckHeap(*gpMainMemZone);
}
#endif  // #if !PSYDOOM_LIMIT_REMOVING

#if !PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Caches into RAM all frames for a sprite.
// This function appears to be unused in the retail version of the game.
// PsyDoom: don't bother compiling this function in, since it is unused.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_CacheSprite(const spritedef_t& sprdef) noexcept {
    // Cache all frames in the sprite
    for (int32_t frameIdx = 0; frameIdx < sprdef.numframes; ++frameIdx) {
        const spriteframe_t& spriteFrame = sprdef.spriteframes[frameIdx];

        // Cache all directions for the frame
        for (int32_t dirIdx = 0; dirIdx < 8; ++dirIdx) {
            const int32_t lumpNum = spriteFrame.lump[dirIdx];
            
            if ((lumpNum < gFirstSpriteLumpNum) || (lumpNum > gLastSpriteLumpNum)) {
                I_Error("CacheSprite: invalid sprite lump %d", lumpNum);
            }

            W_CacheLumpNum(lumpNum, PU_ANIMATION, false);

            // If there are no more rotations for this sprite then stop here
            if (!spriteFrame.rotate)
                break;
        }
    }
}
#endif  // #if !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Cache map textures with the given width into VRAM.
// Textures are cached in groups according to their width in order to try and make more efficient use of VRAM space.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_CacheMapTexturesWithWidth(const int32_t width) noexcept {
    // Round the current fill position in the texture cache up to the nearest multiple of the given texture width.
    // This ensures that for instance 64 pixel textures are on 64 pixel boundaries on the x dimension.
    {
        const int32_t cellX = gTCacheFillCellX;
        const int32_t cellW = ((width >= 0) ? width : width + 15) / TCACHE_CELL_SIZE;   // This is a signed floor() operation
        const int32_t cellXRnd = (cellX + (cellW - 1)) & (-cellW);                      // Note: '&(-cellW)' chops off the unwanted lower bits from the result
        gTCacheFillCellX = cellXRnd;
    }

    // Run through all the sides in the map and cache textures with the specified width
    side_t* pSide = gpSides;

    for (int32_t sideIdx = 0; sideIdx < gNumSides; ++sideIdx, ++pSide) {
        if (pSide->toptexture != -1) {
            texture_t& tex = gpTextures[pSide->toptexture];

            if ((tex.width == width) && (tex.texPageId == 0)) {
                I_CacheTex(tex);
            }
        }
        
        if (pSide->midtexture != -1) {
            texture_t& tex = gpTextures[pSide->midtexture];

            if ((tex.width == width) && (tex.texPageId == 0)) {
                I_CacheTex(tex);
            }
        }
        
        if (pSide->bottomtexture != -1) {
            texture_t& tex = gpTextures[pSide->bottomtexture];

            if ((tex.width == width) && (tex.texPageId == 0)) {
                I_CacheTex(tex);
            }
        }
    }
}
