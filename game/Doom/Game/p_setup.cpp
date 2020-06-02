#include "p_setup.h"

#include "Doom/Base/i_file.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/m_bbox.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/w_wad.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/Renderer/r_sky.h"
#include "doomdata.h"
#include "g_game.h"
#include "p_firesky.h"
#include "p_local.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "p_switch.h"
#include "p_tick.h"

// How much heap space is required after loading the map in order to run the game (48 KiB).
// If we don't have this much then the game craps out with an error.
// Need to be able to support various small allocs throughout gameplay for particles and so forth.
static constexpr int32_t MIN_REQ_HEAP_SPACE_FOR_GAMEPLAY = 1024 * 48;

// How many maps are in a map folder and the number of files per maps folder etc.
static constexpr int32_t LEVELS_PER_MAP_FOLDER = (uint32_t) CdMapTbl_File::MAPSPR01_IMG - (uint32_t) CdMapTbl_File::MAP01_WAD;
static constexpr int32_t NUM_FILES_PER_LEVEL = 3;
static constexpr int32_t FILES_PER_MAP_FOLDER = LEVELS_PER_MAP_FOLDER * NUM_FILES_PER_LEVEL;

// Sky stuff
static constexpr const char* SKY_LUMP_NAME = "F_SKY";
static constexpr int32_t SKY_LUMP_NAME_LEN = sizeof("F_SKY") - 1;   // -1 to discount null terminator

// Map data
const VmPtr<VmPtr<uint16_t>>                        gpBlockmapLump(0x800780C4);
const VmPtr<VmPtr<uint16_t>>                        gpBlockmap(0x80078140);
const VmPtr<int32_t>                                gBlockmapWidth(0x80078284);
const VmPtr<int32_t>                                gBlockmapHeight(0x80077EB8);
const VmPtr<fixed_t>                                gBlockmapOriginX(0x8007818C);
const VmPtr<fixed_t>                                gBlockmapOriginY(0x80078194);
const VmPtr<VmPtr<VmPtr<mobj_t>>>                   gppBlockLinks(0x80077EDC);
const VmPtr<int32_t>                                gNumVertexes(0x80078018);
const VmPtr<VmPtr<vertex_t>>                        gpVertexes(0x800781E4);
const VmPtr<int32_t>                                gNumSectors(0x80077F54);
const VmPtr<VmPtr<sector_t>>                        gpSectors(0x800780A8);
const VmPtr<int32_t>                                gNumSides(0x800781B4);
const VmPtr<VmPtr<side_t>>                          gpSides(0x80077EA0);
const VmPtr<int32_t>                                gNumLines(0x800781C8);
const VmPtr<VmPtr<line_t>>                          gpLines(0x80077EB0);
const VmPtr<int32_t>                                gNumSubsectors(0x80078224);
const VmPtr<VmPtr<subsector_t>>                     gpSubsectors(0x80077F40);
const VmPtr<int32_t>                                gNumBspNodes(0x800781B8);
const VmPtr<VmPtr<node_t>>                          gpBspNodes(0x80077EA4);
const VmPtr<int32_t>                                gNumSegs(0x800780A4);
const VmPtr<VmPtr<seg_t>>                           gpSegs(0x80078238);
const VmPtr<int32_t>                                gTotalNumLeafEdges(0x80077F64);
const VmPtr<VmPtr<leafedge_t>>                      gpLeafEdges(0x8007810C);
const VmPtr<VmPtr<uint8_t>>                         gpRejectMatrix(0x800780E4);
const VmPtr<mapthing_t[MAXPLAYERS]>                 gPlayerStarts(0x800A8E7C);
const VmPtr<mapthing_t[MAX_DEATHMATCH_STARTS]>      gDeathmatchStarts(0x8009806C);
const VmPtr<VmPtr<mapthing_t>>                      gpDeathmatchP(0x80078060);          // Points past the end of the deathmatch starts list

// Function to update the fire sky.
// Set when the map has a fire sky, otherwise null.
void (*gUpdateFireSkyFunc)(texture_t& skyTex) = nullptr;

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map vertex data from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadVertexes(const int32_t lumpNum) noexcept {
    // Sanity check the vertices lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    
    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadVertexes: lump > 64K");
    }

    // Alloc the runtime vertex array
    *gNumVertexes = lumpSize / sizeof(mapvertex_t);
    *gpVertexes = (vertex_t*) Z_Malloc(**gpMainMemZone, *gNumVertexes * sizeof(vertex_t), PU_LEVEL, nullptr);
    
    // Read the WAD vertexes into the temp buffer from the map WAD
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Convert the vertexes to the renderer runtime format
    const mapvertex_t* pSrcVertex = (const mapvertex_t*) gTmpBuffer.get();
    vertex_t* pDstVertex = gpVertexes->get();

    for (int32_t vertexIdx = 0; vertexIdx < *gNumVertexes; ++vertexIdx) {
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
    // Sanity check the segs lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadSegs: lump > 64K");
    }
    
    // Alloc ram for the runtime segs and zero initialize
    *gNumSegs = lumpSize / sizeof(mapseg_t);
    *gpSegs = (seg_t*) Z_Malloc(**gpMainMemZone, *gNumSegs * sizeof(seg_t), PU_LEVEL, nullptr);
    D_memset(gpSegs->get(), std::byte(0), *gNumSegs * sizeof(seg_t));

    // Read the map lump containing the segs into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Process the WAD segs and convert them into runtime segs
    const mapseg_t* pSrcSeg = (const mapseg_t*) gTmpBuffer.get();
    seg_t* pDstSeg = gpSegs->get();

    for (int32_t segIdx = 0; segIdx < *gNumSegs; ++segIdx) {
        // Store basic seg properties
        pDstSeg->vertex1 = &(*gpVertexes)[Endian::littleToHost(pSrcSeg->vertex1)];
        pDstSeg->vertex2 = &(*gpVertexes)[Endian::littleToHost(pSrcSeg->vertex2)];
        pDstSeg->angle = (fixed_t) Endian::littleToHost(pSrcSeg->angle) << FRACBITS;
        pDstSeg->offset = (fixed_t) Endian::littleToHost(pSrcSeg->offset) << FRACBITS;

        // Figure out seg line and side
        line_t& linedef = (*gpLines)[Endian::littleToHost(pSrcSeg->linedef)];
        pDstSeg->linedef = &linedef;

        const int32_t sideNum = linedef.sidenum[Endian::littleToHost(pSrcSeg->side)];
        side_t& side = (*gpSides)[sideNum];
        pDstSeg->sidedef = &side;

        // Set front and backsector reference
        pDstSeg->frontsector = side.sector;

        if (linedef.flags & ML_TWOSIDED) {
            const int32_t backSideNum = linedef.sidenum[Endian::littleToHost(pSrcSeg->side) ^ 1];
            side_t& backSide = (*gpSides)[backSideNum];
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
    // Sanity check the subsectors lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadSubsectors: lump > 64K");
    }

    // Alloc ram for the runtime subsectors and zero initialize
    *gNumSubsectors = lumpSize / sizeof(mapsubsector_t);
    *gpSubsectors = (subsector_t*) Z_Malloc(**gpMainMemZone, *gNumSubsectors * sizeof(subsector_t), PU_LEVEL, nullptr);
    D_memset(gpSubsectors->get(), std::byte(0), *gNumSubsectors * sizeof(subsector_t));

    // Read the map lump containing the subsectors into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);
    
    // Process the WAD subsectors and convert them into runtime subsectors
    const mapsubsector_t* pSrcSubsec = (const mapsubsector_t*) gTmpBuffer.get();
    subsector_t* pDstSubsec = gpSubsectors->get();

    for (int32_t subsectorIdx = 0; subsectorIdx < *gNumSubsectors; ++subsectorIdx) {
        pDstSubsec->numsegs = Endian::littleToHost(pSrcSubsec->numsegs);
        pDstSubsec->firstseg = Endian::littleToHost(pSrcSubsec->firstseg);
        pDstSubsec->numLeafEdges = 0;
        pDstSubsec->firstLeafEdge = 0;

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

    // Sanity check the sectors lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadSectors: lump > 64K");
    }

    // Alloc ram for the runtime sectors and zero initialize
    *gNumSectors = lumpSize / sizeof(mapsector_t);
    *gpSectors = (sector_t*) Z_Malloc(**gpMainMemZone, *gNumSectors * sizeof(sector_t), PU_LEVEL, nullptr);
    D_memset(gpSectors->get(), std::byte(0), *gNumSectors * sizeof(sector_t));

    // Read the map lump containing the sectors into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Process the WAD sectors and convert them into runtime sectors
    {
        const mapsector_t* pSrcSec = (const mapsector_t*) gTmpBuffer.get();
        sector_t* pDstSec = gpSectors->get();

        for (int32_t secIdx = 0; secIdx < *gNumSectors; ++secIdx) {
            // Save basic properties
            pDstSec->floorheight = (fixed_t) Endian::littleToHost(pSrcSec->floorheight) << FRACBITS;
            pDstSec->ceilingheight = (fixed_t) Endian::littleToHost(pSrcSec->ceilingheight) << FRACBITS;
            pDstSec->colorid = Endian::littleToHost(pSrcSec->colorid);
            pDstSec->lightlevel = Endian::littleToHost(pSrcSec->lightlevel);
            pDstSec->special = Endian::littleToHost(pSrcSec->special);
            pDstSec->thinglist = nullptr;
            pDstSec->tag = Endian::littleToHost(pSrcSec->tag);
            pDstSec->flags = Endian::littleToHost(pSrcSec->flags);

            // Figure out floor texture number
            pDstSec->floorpic = R_FlatNumForName(pSrcSec->floorpic);

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

            ++pSrcSec;
            ++pDstSec;
        }
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
    // Sanity check the nodes lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    
    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadNodes: lump > 64K");
    }

    // Alloc ram for the runtime nodes
    *gNumBspNodes = lumpSize / sizeof(mapnode_t);
    *gpBspNodes = (node_t*) Z_Malloc(**gpMainMemZone, *gNumBspNodes * sizeof(node_t), PU_LEVEL, nullptr);

    // Read the map lump containing the nodes into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Process the WAD nodes and convert them into runtime nodes.
    // The format for nodes on the PSX appears identical to PC.
    const mapnode_t* pSrcNode = (const mapnode_t*) gTmpBuffer.get();
    node_t* pDstNode = gpBspNodes->get();

    for (int32_t nodeIdx = 0; nodeIdx < *gNumBspNodes; ++nodeIdx) {
        pDstNode->line.x = (fixed_t) Endian::littleToHost(pSrcNode->x) << FRACBITS;
        pDstNode->line.y = (fixed_t) Endian::littleToHost(pSrcNode->y) << FRACBITS;
        pDstNode->line.dx = (fixed_t) Endian::littleToHost(pSrcNode->dx) << FRACBITS;
        pDstNode->line.dy = (fixed_t) Endian::littleToHost(pSrcNode->dy) << FRACBITS;

        for (int32_t childIdx = 0; childIdx < 2; ++childIdx) {
            pDstNode->children[childIdx] = Endian::littleToHost(pSrcNode->children[childIdx]);

            for (int32_t coordIdx = 0; coordIdx < 4; ++coordIdx) {
                const fixed_t coord = (fixed_t) Endian::littleToHost(pSrcNode->bbox[childIdx][coordIdx]) << FRACBITS;
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
    // Sanity check the things lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadThings: lump > 64K");
    }

    // Determine how many things there are to spawn and read the lump from the WAD
    const int32_t numThings = lumpSize / sizeof(mapthing_t);
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Spawn the map things
    mapthing_t* pSrcThing = (mapthing_t*) gTmpBuffer.get();

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
    // Sanity check the linedefs lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadLineDefs: lump > 64K");
    }

    // Alloc ram for the runtime linedefs and zero initialize
    *gNumLines = lumpSize / sizeof(maplinedef_t);
    *gpLines = (line_t*) Z_Malloc(**gpMainMemZone, *gNumLines * sizeof(line_t), PU_LEVEL, nullptr);
    D_memset(gpLines->get(), std::byte(0), *gNumLines * sizeof(line_t));

    // Read the map lump containing the sidedefs into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Process the WAD linedefs and convert them into runtime linedefs
    const maplinedef_t* pSrcLine = (maplinedef_t*) gTmpBuffer.get();
    line_t* pDstLine = gpLines->get();

    for (int32_t lineIdx = 0; lineIdx < *gNumLines; ++lineIdx) {
        // Save some basic line properties
        pDstLine->flags = Endian::littleToHost(pSrcLine->flags);
        pDstLine->special = Endian::littleToHost(pSrcLine->special);
        pDstLine->tag = Endian::littleToHost(pSrcLine->tag);

        // Save line vertices, delta coordinates and slope type
        vertex_t& vertex1 = (*gpVertexes)[Endian::littleToHost(pSrcLine->vertex1)];
        vertex_t& vertex2 = (*gpVertexes)[Endian::littleToHost(pSrcLine->vertex2)];

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
            side_t& side = (*gpSides)[sidenum1];
            pDstLine->frontsector = side.sector;
        } else {
            pDstLine->frontsector = nullptr;
        }

        if (sidenum2 != -1) {
            side_t& side = (*gpSides)[sidenum2];
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
    // Sanity check the sidedefs lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadSideDefs: lump > 64K");
    }

    // Alloc ram for the runtime sidedefs and zero initialize
    *gNumSides = lumpSize / sizeof(mapsidedef_t);
    *gpSides = (side_t*) Z_Malloc(**gpMainMemZone, *gNumSides * sizeof(side_t), PU_LEVEL, nullptr);
    D_memset(gpSides->get(), std::byte(0), *gNumSides  * sizeof(side_t));

    // Read the map lump containing the sidedefs into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Process the WAD sidedefs and convert them into runtime sidedefs
    const mapsidedef_t* pSrcSide = (const mapsidedef_t*) gTmpBuffer.get();
    side_t* pDstSide = gpSides->get();

    for (int32_t sideIdx = 0; sideIdx < *gNumSides; ++sideIdx) {
        pDstSide->textureoffset = (fixed_t) Endian::littleToHost(pSrcSide->textureoffset) << FRACBITS;
        pDstSide->rowoffset = (fixed_t) Endian::littleToHost(pSrcSide->rowoffset) << FRACBITS;
        pDstSide->sector = &(*gpSectors)[Endian::littleToHost(pSrcSide->sector)];
        pDstSide->toptexture = R_TextureNumForName(pSrcSide->toptexture);
        pDstSide->midtexture = R_TextureNumForName(pSrcSide->midtexture);
        pDstSide->bottomtexture = R_TextureNumForName(pSrcSide->bottomtexture);

        ++pSrcSide;
        ++pDstSide;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the blockmap from the specified map lump number.
// For more details about the blockmap, see: https://doomwiki.org/wiki/Blockmap
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadBlockMap(const int32_t lumpNum) noexcept {
    // Read the blockmap lump into RAM
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    *gpBlockmapLump = (uint16_t*) Z_Malloc(**gpMainMemZone, lumpSize, PU_LEVEL, nullptr);
    W_ReadMapLump(lumpNum, gpBlockmapLump->get(), true);

    // The first 8 bytes of the blockmap are it's header
    struct blockmap_hdr_t {
        int16_t     originx;
        int16_t     originy;
        int16_t     width;
        int16_t     height;
    };

    static_assert(sizeof(blockmap_hdr_t) == 8);
    blockmap_hdr_t& blockmapHeader = *(blockmap_hdr_t*) gpBlockmapLump->get();

    // The offsets to each blocklist (list of block lines per block) start after the header
    *gpBlockmap = (uint16_t*)(&(&blockmapHeader)[1]);

    // Endian correction for the entire blockmap lump.
    //
    // PC-PSX: skip doing this on little endian architectures to save a few cycles.
    // The original code did this transform from little endian to little endian even though it had no effect...
    #if PC_PSX_DOOM_MODS
        constexpr bool bEndianCorrect = (!Endian::isLittle());
    #else
        constexpr bool bEndianCorrect = true;
    #endif

    if constexpr (bEndianCorrect) {
        static_assert(sizeof((*gpBlockmapLump)[0]) == sizeof(int16_t));
        const int32_t count = (lumpSize / 2) + (lumpSize & 1);

        for (int32_t i = 0; i < count; ++i) {
            (*gpBlockmapLump)[i] = Endian::littleToHost((*gpBlockmapLump)[i]);
        }
    }

    // Save blockmap dimensions
    *gBlockmapWidth = blockmapHeader.width;
    *gBlockmapHeight = blockmapHeader.height;
    *gBlockmapOriginX = (fixed_t) blockmapHeader.originx << FRACBITS;
    *gBlockmapOriginY = (fixed_t) blockmapHeader.originy << FRACBITS;
    
    // Alloc and null initialize the list of map objects for each block
    const int32_t blockLinksSize = (int32_t) blockmapHeader.width * (int32_t) blockmapHeader.height * sizeof(VmPtr<mobj_t>);
    *gppBlockLinks = (VmPtr<mobj_t>*) Z_Malloc(**gpMainMemZone, blockLinksSize, PU_LEVEL, nullptr);
    D_memset(gppBlockLinks->get(), std::byte(0), blockLinksSize);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the reject map from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadRejectMap(const int32_t lumpNum) noexcept {
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    *gpRejectMatrix = (uint8_t*) Z_Malloc(**gpMainMemZone, lumpSize, PU_LEVEL, nullptr);
    W_ReadMapLump(lumpNum, gpRejectMatrix->get(), true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load leafs from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadLeafs(const int32_t lumpNum) noexcept {
    // Sanity check the leafs lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    
    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadLeafs: lump > 64K");
    }

    // Read the map lump containing the leaf edges into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);
    const std::byte* const pLumpBeg = gTmpBuffer.get();
    const std::byte* const pLumpEnd = gTmpBuffer.get() + lumpSize;

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

    if (numLeafs != *gNumSubsectors) {
        I_Error("P_LoadLeafs: leaf/subsector inconsistancy");
    }

    // Allocate room for all the leaf edges
    *gpLeafEdges = (leafedge_t*) Z_Malloc(**gpMainMemZone, totalLeafEdges * sizeof(leafedge_t), PU_LEVEL, nullptr);
    
    // Convert WAD leaf edges to runtime leaf edges and link them in with other map data structures
    *gTotalNumLeafEdges = 0;

    const std::byte* pLumpByte = gTmpBuffer.get();
    subsector_t* pSubsec = gpSubsectors->get();
    leafedge_t* pDstEdge = gpLeafEdges->get();

    for (int32_t leafIdx = 0; leafIdx < numLeafs; ++leafIdx) {
        // Convert leaf data to host endian and move past it
        mapleaf_t leaf = *(mapleaf_t*) pLumpByte;
        leaf.numedges = Endian::littleToHost(leaf.numedges);
        pLumpByte += sizeof(mapleaf_t);

        // Save leaf info on the subsector
        pSubsec->numLeafEdges = leaf.numedges;
        pSubsec->firstLeafEdge = (int16_t) *gTotalNumLeafEdges;

        // Process the edges in the leaf
        for (int32_t edgeIdx = 0; edgeIdx < pSubsec->numLeafEdges; ++edgeIdx) {
            // Convert edge data to host endian and move past it
            mapleafedge_t srcEdge = *(const mapleafedge_t*) pLumpByte;
            srcEdge.segnum = Endian::littleToHost(srcEdge.segnum);
            srcEdge.vertexnum = Endian::littleToHost(srcEdge.vertexnum);

            pLumpByte += sizeof(mapleafedge_t);

            // Set leaf vertex reference
            if (srcEdge.vertexnum >= *gNumVertexes) {
                I_Error("P_LoadLeafs: vertex out of range\n");
            }

            pDstEdge->vertex = &(*gpVertexes)[srcEdge.vertexnum];

            // Set leaf seg reference
            if (srcEdge.segnum == -1) {
                pDstEdge->seg = nullptr;
            } else {
                if (srcEdge.segnum >= *gNumSegs) {
                    I_Error("P_LoadLeafs: seg out of range\n");
                }

                pDstEdge->seg = &(*gpSegs)[srcEdge.segnum];
            }

            ++pDstEdge;
        }

        // Move along to the next leaf
        *gTotalNumLeafEdges += pSubsec->numLeafEdges;
        ++pSubsec;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Builds the line lists for each sector, bounding boxes as well as sound origin points
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_GroupLines() noexcept {
    // Associate subsectors with their sectors
    {
        subsector_t* pSubsec = gpSubsectors->get();
        
        for (int32_t subsecIdx = 0; subsecIdx < *gNumSubsectors; ++subsecIdx) {
            const seg_t& seg = (*gpSegs)[pSubsec->firstseg];
            pSubsec->sector = seg.sidedef->sector;
            ++pSubsec;
        }
    }

    // Count the number of lines in each sector and how many line references there are among all sectors
    int32_t totalLineRefs = 0;

    {
        line_t* pLine = gpLines->get();

        for (int32_t lineIdx = 0; lineIdx < *gNumLines; ++lineIdx) {
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
    VmPtr<line_t>* const pLineRefBuffer = (VmPtr<line_t>*) Z_Malloc(**gpMainMemZone, totalLineRefs * sizeof(VmPtr<line_t>), PU_LEVEL, nullptr);
    VmPtr<line_t>* pLineRef = pLineRefBuffer;

    // Build the list of lines for each sector, also bounding boxes and the 'sound origin' point
    sector_t* pSec = gpSectors->get();

    for (int32_t secIdx = 0; secIdx < *gNumSectors; ++secIdx) {
        // Clear the bounding box for the sector and set the line list start
        fixed_t bbox[4];
        M_ClearBox(bbox);
        pSec->lines = pLineRef;

        // Build up the bounding box and line list for the sector by examining each line in the level against this sector.
        // Not an efficient algorithm, since it is O(N^2) but works OK given the size of the datasets in DOOM.
        // This might be a problem if you are planning on making a DOOM open world game however... :P
        {
            line_t* pLine = gpLines->get();

            for (int32_t lineIdx = 0; lineIdx < *gNumLines; ++lineIdx) {
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
        const int32_t actualLineCount = (int32_t)(pLineRef - pSec->lines.get());

        if (actualLineCount != pSec->linecount) {
            I_Error("P_GroupLines: miscounted");
        }

        // Set the sound origin location for sector sounds and also the subsector.
        // Use the bounding box center for this.
        {
            degenmobj_t& soundorg = pSec->soundorg;
            soundorg.x = (bbox[BOXLEFT] + bbox[BOXRIGHT]) >> 1;
            soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) >> 1;

            #if PC_PSX_DOOM_MODS
                // The original code did not appear to initialize the 'z' field!
                // I'm not sure it's used for sound code but give it a defined value of midway up in the air for good measure.
                soundorg.z = (pSec->floorheight + pSec->ceilingheight) >> 1;
            #endif

            pSec->soundorg.subsector = R_PointInSubsector(soundorg.x, soundorg.y);
        }

        // Compute the bounding box for the sector in blockmap units.
        // Note that if the sector extends the beyond the blockmap then we constrain it's coordinate.
        {
            int32_t bmcoord = (bbox[BOXTOP] - *gBlockmapOriginY + MAXRADIUS) >> MAPBLOCKSHIFT;
            bmcoord = (bmcoord >= *gBlockmapHeight) ? *gBlockmapHeight - 1 : bmcoord;
            pSec->blockbox[BOXTOP] = bmcoord;
        }
        {
            int32_t bmcoord = (bbox[BOXBOTTOM] - *gBlockmapOriginY - MAXRADIUS) >> MAPBLOCKSHIFT;
            bmcoord = (bmcoord < 0) ? 0 : bmcoord;
            pSec->blockbox[BOXBOTTOM] = bmcoord;
        }
        {
            int32_t bmcoord = (bbox[BOXRIGHT] - *gBlockmapOriginX + MAXRADIUS) >> MAPBLOCKSHIFT;
            bmcoord = (bmcoord >= *gBlockmapWidth) ? *gBlockmapWidth - 1 : bmcoord;
            pSec->blockbox[BOXRIGHT] = bmcoord;
        }
        {
            int32_t bmcoord = (bbox[BOXLEFT] - *gBlockmapOriginX - MAXRADIUS) >> MAPBLOCKSHIFT;
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
void P_Init() noexcept {
    // Load sector flats into VRAM if not already there
    {
        const sector_t* pSec = gpSectors->get();

        for (int32_t secIdx = 0; secIdx < *gNumSectors; ++secIdx, ++pSec) {
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
    *gLockedTexPagesMask |= 0b0000'0000'0010;
    *gTCacheFillPage = 2;
    *gTCacheFillCellX = 0;
    *gTCacheFillCellY = 0;
    *gTCacheFillRowCellH = 0;

    // Initialize the sky texture, palette and do some more specialized setup if it's the fire sky
    gUpdateFireSkyFunc = nullptr;
    gPaletteClutId_CurMapSky = gPaletteClutIds[MAINPAL];

    if (gpSkyTexture) {
        // If the lump name for the sky follows the format 'xxxx9' then assume it is a fire sky.
        // That needs to have it's lump cached, palette & update function set and initial few updates done...
        texture_t& skyTex = *gpSkyTexture;
        const lumpinfo_t& skyTexLump = (*gpLumpInfo)[skyTex.lumpNum];

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
        side_t* pSide = gpSides->get();

        for (int32_t sideIdx = 0; sideIdx < *gNumSides; ++sideIdx, ++pSide) {
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
    *gLockedTexPagesMask |= 0b0000'0001'1100;
    *gTCacheFillPage = 5;
    *gTCacheFillCellX = 0;
    *gTCacheFillCellY = 0;
    *gTCacheFillRowCellH = 0;
    
    // Clear out any floor or wall textures we had temporarily in RAM from the above caching.
    //
    // Small optimization opportunity: this is slightly wasteful in that if we have animated walls/floors, then they will be evicted by this
    // call here and then reloaded immediately again in the call to 'P_InitPicAnims', but under a different zone memory manager tag.
    // We could perhaps change the memory tag and retain them in RAM if still needed for animation?
    Z_FreeTags(**gpMainMemZone, PU_CACHE);

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
    Z_FreeTags(**gpMainMemZone, PU_CACHE|PU_LEVSPEC|PU_LEVEL);

    if (!*gbIsLevelBeingRestarted) {
        *gLockedTexPagesMask &= 1;
        Z_FreeTags(**gpMainMemZone, PU_ANIMATION);
    }
    
    I_PurgeTexCache();
    Z_CheckHeap(**gpMainMemZone);
    M_ClearRandom();

    // Init player stats for the map
    *gTotalKills = 0;
    *gTotalItems = 0;
    *gTotalSecret = 0;

    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        player_t& player = gPlayers[playerIdx];

        player.killcount = 0;
        player.secretcount = 0;
        player.itemcount = 0;
        player.frags = 0;
    }

    // Setup the thinkers and map objects lists
    gThinkerCap->prev = gThinkerCap.get();
    gThinkerCap->next = gThinkerCap.get();

    gMObjHead->next = gMObjHead.get();
    gMObjHead->prev = gMObjHead.get();

    // Setup the item respawn queue and map object kill count
    *gItemRespawnQueueHead = 0;
    *gItemRespawnQueueTail = 0;
    *gNumMObjKilled = 0;

    // Figure out which file to open for the map WAD.
    // Not sure why the PSX code was checking for a negative map index here, maybe a special dev/test thing?
    const int32_t mapIndex = mapNum - 1;
    const int32_t mapFolderIdx = (mapIndex >= 0) ?
        (mapIndex / LEVELS_PER_MAP_FOLDER) :
        (mapIndex + LEVELS_PER_MAP_FOLDER - 1) / LEVELS_PER_MAP_FOLDER;

    const int32_t mapIdxInFolder = mapIndex - mapFolderIdx * LEVELS_PER_MAP_FOLDER;
    const int32_t mapFolderOffset = mapFolderIdx * NUM_FILES_PER_LEVEL * LEVELS_PER_MAP_FOLDER;
    const CdMapTbl_File mapWadFile = (CdMapTbl_File)((int32_t) CdMapTbl_File::MAP01_WAD + mapIdxInFolder + mapFolderOffset);
    
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
    *gpDeathmatchP = &gDeathmatchStarts[0];
    P_LoadThings(mapStartLump + ML_THINGS);
    
    // Spawn special thinkers such as light flashes etc. and free up the loaded WAD data
    P_SpawnSpecials();
    Z_Free2(**gpMainMemZone, pMapWadFileData);

    // Loading map textures and sprites
    if (!*gbIsLevelBeingRestarted) {
        const CdMapTbl_File mapTexFile = (CdMapTbl_File)((int32_t) CdMapTbl_File::MAPTEX01_IMG + mapIdxInFolder + mapFolderOffset);
        const CdMapTbl_File mapSprFile = (CdMapTbl_File)((int32_t) CdMapTbl_File::MAPSPR01_IMG + mapIdxInFolder + mapFolderOffset);
        
        P_LoadBlocks(mapTexFile);
        P_Init();
        P_LoadBlocks(mapSprFile);
    }

    // Check there is enough heap space left in order to run the level
    const int32_t freeMemForGameplay = Z_FreeMemory(**gpMainMemZone);

    if (freeMemForGameplay < MIN_REQ_HEAP_SPACE_FOR_GAMEPLAY) {
        Z_DumpHeap();
        I_Error("P_SetupLevel: not enough free memory %d", freeMemForGameplay);
    }

    // Spawn the player(s)
    if (*gNetGame != gt_single) {
        I_NetHandshake();
        
        // Randomly spawn players in different locations - this logic is a little strange.
        // We spawn all players in the same location but immediately respawn and remove the old 'mobj_t' to get the random starts.
        for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
            mobj_t* const pPlayerThing = P_SpawnMobj(
                (int32_t) gPlayerStarts[0].x << FRACBITS,
                (int32_t) gPlayerStarts[0].y << FRACBITS,
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads a list of memory blocks containing WAD lumps from the given file.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_LoadBlocks(const CdMapTbl_File file) noexcept {
    // Try and load the memory blocks containing lumps from the given file.
    // Retry this a number of times before giving up, if the initial load attempt fails.
    // Presumably this was to try and recover from a bad CD...
    int32_t numLoadAttempts = 0;
    int32_t fileSize = -1;
    std::byte* pBlockData = nullptr;
    memblock_t initialAllocHeader = {};

    while (true) {
        // If there have been too many failed load attempts then issue an error
        if (numLoadAttempts >= 4) {
            I_Error("P_LoadBlocks: Data Failure");
        }

        ++numLoadAttempts;
        
        // Open the blocks file and get it's size
        const int32_t openFileIdx = OpenFile(file);
        fileSize = SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::END);

        // Alloc room to hold the file: note that we reduce the alloc size by 'sizeof(memblock_t)' since the blocks
        // file already includes space for a 'memblock_t' header for each lump. We also save the current memblock
        // header just in case loading fails, so we can restore it prior to deallocation...
        std::byte* const pAlloc = (std::byte*) Z_Malloc(**gpMainMemZone, fileSize - sizeof(memblock_t), PU_STATIC, nullptr);
        initialAllocHeader = ((memblock_t*) pAlloc)[-1];
        pBlockData = (std::byte*) &((fileblock_t*) pAlloc)[-1];
        
        // Read the file contents
        SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::SET);
        ReadFile(openFileIdx, pBlockData, fileSize);
        CloseFile(openFileIdx);

        // Process all of the memory blocks in the file and make sure they are ok.
        // Once they are verified then we can start linking them in with other memory blocks in the heap:
        bool bLoadedOk = true;
        int32_t bytesLeft = fileSize;
        std::byte* pCurBlockData = pBlockData;

        do {
            // Verify the block has a valid zoneid
            fileblock_t& fileBlock = *(fileblock_t*) pCurBlockData;
            
            if (fileBlock.id != ZONEID) {
                bLoadedOk = false;
                break;
            }
            
            // Verify the lump number is valid
            if (fileBlock.lumpNum >= *gNumLumps) {
                bLoadedOk = false;
                break;
            }

            // Verify the compression mode is valid
            if (fileBlock.isUncompressed >= 2) {
                bLoadedOk = false;
                break;
            }

            // Verify the decompressed size is valid
            if (fileBlock.isUncompressed == 0) {
                // Get the decompressed size of the data following the file block header and make sure it is what we expect
                const uint32_t inflatedSize = getDecodedSize(&(&fileBlock)[1]);
                const lumpinfo_t& lump = (*gpLumpInfo)[fileBlock.lumpNum];
                
                if (inflatedSize != lump.size) {
                    bLoadedOk = false;
                    break;
                }
            }

            // Advance onto the next block and make sure we haven't gone past the end of the data
            const int32_t blockSize = fileBlock.size;
            bytesLeft -= blockSize;
            
            if (bytesLeft < 0) {
                bLoadedOk = false;
                break;
            }
            
            pCurBlockData += blockSize;
        } while (bytesLeft != 0);

        // If everything was loaded ok then link the first block into the heap block list and finish up.
        // Will do the rest of the linking in the loop below:
        if (bLoadedOk) {
            memblock_t& memblock = *(memblock_t*) pBlockData;
            memblock.prev = initialAllocHeader.prev;
            break;
        }

        // Load failed: restore the old alloc header and free the memory block.
        // Will try again a certain number of times to try and counteract unreliable CDs.
        ((memblock_t*) pBlockData)[0] = initialAllocHeader;
        Z_Free2(**gpMainMemZone, pAlloc);
    }
    
    // Once all the blocks are loaded and verified then setup all of the block links.
    // Also mark blocks for lumps that are already loaded as freeable.
    std::byte* pCurBlockData = pBlockData;
    int32_t bytesLeft = fileSize;

    do {
        // Note: making a copy of the fileblock header to avoid possible strict aliasing weirdness reading and writing to the
        // same memory using different struct types. The original code did not do that but this should be functionally the same.
        fileblock_t fileblock = *(fileblock_t*) pCurBlockData;
        memblock_t& memblock = *(memblock_t*) pCurBlockData;
    
        // Check if this lump is already loaded
        VmPtr<void>& lumpCacheEntry = (*gpLumpCache)[fileblock.lumpNum];
        
        if (lumpCacheEntry) {
            // If the lump is already loaded then mark this memory block as freeable
            memblock.user = nullptr;
            memblock.tag = 0;
            memblock.id = 0;
        } else {
            // Lump not loaded, set the lump cache entry to point to the newly loaded data.
            // Also save whether the lump is compressed or not:
            memblock.user = &lumpCacheEntry;
            lumpCacheEntry = &memblock + 1;
            (*gpbIsUncompressedLump)[fileblock.lumpNum] = fileblock.isUncompressed;
        }
        
        // Is this the last loade block in the file?
        // If it is then set the size based on where the next block in the heap starts, otherwise just use the size defined in the file.
        bytesLeft -= fileblock.size;
        
        if (bytesLeft != 0) {
            memblock_t* const pNextMemblock = (memblock_t*)(pCurBlockData + fileblock.size);
            memblock.next = pNextMemblock;
        } else {
            const uint32_t blockSize = (uint32_t)((std::byte*) initialAllocHeader.next.get() - pCurBlockData);
            
            if (initialAllocHeader.next) {
                memblock.size = blockSize;
            }
            
            memblock.next = initialAllocHeader.next;
        }
        
        // Set backlinks for the next block
        if (memblock.next) {
            memblock.next->prev = &memblock;
        }
        
        // Move onto the next block loaded
        pCurBlockData = (std::byte*) memblock.next.get();
        
    } while (bytesLeft != 0);
    
    // After all that is done, make sure the heap is valid
    Z_CheckHeap(**gpMainMemZone);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Caches into RAM all frames for a sprite.
// This function appears to be unused in the retail version of the game.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_CacheSprite(const spritedef_t& sprdef) noexcept {
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Cache map textures with the given width into VRAM.
// Textures are cached in groups according to their width in order to try and make more efficient use of VRAM space.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_CacheMapTexturesWithWidth(const int32_t width) noexcept {
    // Round the current fill position in the texture cache up to the nearest multiple of the given texture width.
    // This ensures that for instance 64 pixel textures are on 64 pixel boundaries on the x dimension.
    {
        const int32_t cellX = *gTCacheFillCellX;
        const int32_t cellW = ((width >= 0) ? width : width + 15) / TCACHE_CELL_SIZE;   // This is a signed floor() operation
        const int32_t cellXRnd = (cellX + (cellW - 1)) & (-cellW);                      // Note: '&(-cellW)' chops off the unwanted lower bits from the result
        *gTCacheFillCellX = cellXRnd;
    }

    // Run through all the sides in the map and cache textures with the specified width
    side_t* pSide = gpSides->get();

    for (int32_t sideIdx = 0; sideIdx < *gNumSides; ++sideIdx, ++pSide) {
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
