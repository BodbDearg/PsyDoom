#include "p_setup.h"

#include "Doom/Base/i_file.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/m_bbox.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/cdmaptbl.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_doors.h"
#include "Doom/Game/p_floor.h"
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
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "p_switch.h"
#include "p_tick.h"
#include "p_weak.h"
#include "PsyDoom/DevMapAutoReloader.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/MapHash.h"
#include "PsyDoom/MapInfo/MapInfo.h"
#include "PsyDoom/MapPatcher/MapPatcher.h"
#include "PsyDoom/MobjSpritePrecacher.h"
#include "PsyDoom/ModMgr.h"
#include "PsyDoom/ScriptingEngine.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

// How much heap space is required after loading the map in order to run the game (48 KiB in Doom, 32 KiB in Final Doom).
// If we don't have this much then the game dies with an error; I'm adopting the Final Doom requirement here since it is the lowest.
// Need to be able to support various small allocs throughout gameplay for particles and so forth.
static constexpr int32_t MIN_REQ_HEAP_SPACE_FOR_GAMEPLAY = 1024 * 32;

// How many maps are in a map folder and the number of files per maps folder.
// PsyDoom: file ids are no longer simple integers, so these numbers are now no longer applicable.
#if !PSYDOOM_MODS
    static constexpr int32_t LEVELS_PER_MAP_FOLDER = (uint32_t) CdFileId::MAPSPR01_IMG - (uint32_t) CdFileId::MAP01_WAD;
    static constexpr int32_t NUM_FILES_PER_LEVEL = 3;
#endif

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

// PsyDoom: a list of all player starts for players 1 & 2, including duplicates that spawn so called 'Voodoo dolls'.
// This list is used to add support for 'Voodoo dolls' to the PSX engine.
#if PSYDOOM_MODS
    static std::vector<mapthing_t> gAllPlayerStarts;
#endif

// PsyDoom: if not null then issue this warning after the level has started.
// Can be used to issue non-fatal warnings about bad map conditions to WAD authors.
#if PSYDOOM_MODS
    char gLevelStartupWarning[64];
#endif

// PsyDoom: sets of texture and flat texture indexes to indicate what walls and flats are to be loaded & cached during level setup.
// The new flexible texture mangement code first flags all the resources needed using these sets before actually sorting and loading the resources.
#if PSYDOOM_LIMIT_REMOVING
    FixedIndexSet gCacheTextureSet;
    FixedIndexSet gCacheFlatTextureSet;

    // A sorted list of textures to be loaded, produced from the indexes: re-use to avoid reallocations
    static std::vector<texture_t*> gLoadTextureList;
#endif

// Is the map being loaded a Final Doom format map?
static bool gbLoadingFinalDoomMap;

// Function to update the fire sky.
// Set when the map has a fire sky, otherwise null.
void (*gUpdateFireSkyFunc)(texture_t& skyTex) = nullptr;

// Functions internal to this module
#if PSYDOOM_LIMIT_REMOVING
    static void P_FlagMapTexturesForLoading() noexcept;
    static void P_LoadMapTextures() noexcept;
#else
    static void P_CacheMapTexturesWithWidth(const int32_t width) noexcept;
#endif

#if !PSYDOOM_MODS
    static void P_CacheSprite(const spritedef_t& sprdef) noexcept;  // PsyDoom: not used, so compiling out
#endif

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom helper: tell if a flat texture index is a sky texture
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isSkyFlatPic(const int32_t picNum) noexcept {
    if ((picNum >= 0) && (picNum < gNumFlatLumps)) {
        const texture_t& tex = gpFlatTextures[picNum];
        const WadLumpName lumpName = W_GetLumpName(tex.lumpNum);
        return (
            ((lumpName.chars[0] & 0x7F) == 'F') &&  // N.B: must mask to remove the 'compressed' flag from the character
            (lumpName.chars[1] == '_') &&
            (lumpName.chars[2] == 'S') &&
            (lumpName.chars[3] == 'K') &&
            (lumpName.chars[4] == 'Y')
        );
    }

    return false;
}
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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
    #endif

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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
    #endif

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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
    #endif

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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
    #endif

    // Process the WAD sectors and convert them into runtime sectors
    auto processWadSectors = [&](auto pWadSectors) noexcept {
        // Which sector type are we dealing with and is it Final Doom?
        typedef std::remove_reference_t<decltype(*pWadSectors)> wadsector_t;
        constexpr bool bFinalDoom = std::is_same_v<wadsector_t, mapsector_final_t>;

        // This is required for the Final Doom case.
        // PsyDoom: this won't work anymore because texture lumps are not guaranteed to be contiguous if we are loading WADs other than the original IWAD.
        // Instead, an alternative method will be used below to determine when a flat is using a sky.
        #if !PSYDOOM_MODS
            const int32_t firstSkyTexPic = W_GetNumForName("F_SKY01") - gFirstFlatLumpNum;
        #endif

        // Process the sectors
        const wadsector_t* pSrcSec = pWadSectors;
        sector_t* pDstSec = gpSectors;

        for (int32_t secIdx = 0; secIdx < gNumSectors; ++secIdx) {
            // PsyDoom helper: ensures a floor or ceiling texture is in range and issues a warning if not (if warnings are allowed).
            // These are new safety checks which were originally not run.
            const auto ensureValidFlatPic = [&](int32_t& flatPicNum) noexcept {
                #if PSYDOOM_MODS
                    if ((flatPicNum < 0) || (flatPicNum >= gNumFlatLumps)) {
                        flatPicNum = 0;

                        #if PSYDOOM_MISSING_TEX_WARNINGS
                            std::snprintf(
                                gLevelStartupWarning,
                                C_ARRAY_SIZE(gLevelStartupWarning),
                                (&flatPicNum == &pDstSec->floorpic) ? "W:bad f-tex for sector %d!" : "W:bad c-tex for sector %d!",
                                secIdx
                            );
                        #endif
                    }
                #endif
            };

            // Save basic properties
            pDstSec->floorheight = d_int_to_fixed(Endian::littleToHost(pSrcSec->floorheight));
            pDstSec->ceilingheight = d_int_to_fixed(Endian::littleToHost(pSrcSec->ceilingheight));
            pDstSec->colorid = Endian::littleToHost(pSrcSec->colorid);
            pDstSec->lightlevel = Endian::littleToHost(pSrcSec->lightlevel);
            pDstSec->special = Endian::littleToHost(pSrcSec->special);
            pDstSec->thinglist = nullptr;
            pDstSec->tag = Endian::littleToHost(pSrcSec->tag);
            pDstSec->flags = Endian::littleToHost(pSrcSec->flags);

            #if PSYDOOM_MODS
                pDstSec->ceilColorid = Endian::littleToHost(pSrcSec->ceilColorid);
                pDstSec->ceilColorid = (pDstSec->ceilColorid == 0) ? pDstSec->colorid : pDstSec->ceilColorid;   // N.B: use the floor color if ceiling color is '0' (undefined)
            #endif

            if constexpr (bFinalDoom) {
                // Final Doom specific stuff: we have the actual floor and ceiling texture indexes in this case: no lookup needed!
                // PsyDoom: use the overriden version of the flats if there are multiple versions of the same flat.
                #if PSYDOOM_MODS
                    const int32_t ceilingPic = R_GetOverrideFlatNum(Endian::littleToHost(pSrcSec->ceilingpic));
                    const int32_t floorPic = R_GetOverrideFlatNum(Endian::littleToHost(pSrcSec->floorpic));
                #else
                    const int32_t ceilingPic = Endian::littleToHost(pSrcSec->ceilingpic);
                    const int32_t floorPic = Endian::littleToHost(pSrcSec->floorpic);
                #endif

                pDstSec->floorpic = floorPic;
                pDstSec->ceilingpic = ceilingPic;

                // Note: if the ceiling has a sky then remove its texture and figure out the sky lump name for it - will load the sky lump later on.
                // PsyDoom: use an alternative method to detect skies since texture lump numbers are no longer guaranteed to be contiguous if user mods are used.
                #if PSYDOOM_MODS
                    const bool bIsSkyCeilingPic = isSkyFlatPic(ceilingPic);
                #else
                    const bool bIsSkyCeilingPic = (ceilingPic >= firstSkyTexPic);
                #endif

                if (bIsSkyCeilingPic) {
                    pDstSec->ceilingpic = -1;

                    // PsyDoom: add support for more than 9 sky variants (up to 99) and adjust for texture management differences
                    #if PSYDOOM_MODS
                        const WadLumpName lumpName = W_GetLumpName(gpFlatTextures[ceilingPic].lumpNum);
                        skyLumpName[3] = lumpName.chars[5];
                        skyLumpName[4] = lumpName.chars[6];
                    #else
                        skyLumpName[3] = '0';
                        skyLumpName[4] = '1' + (char)(ceilingPic - firstSkyTexPic);     // Note: only supports up to 9 sky variants!
                    #endif
                } else {
                    ensureValidFlatPic(pDstSec->ceilingpic);
                }

                // PsyDoom: add support for floor skies too
                #if PSYDOOM_MODS
                    if (isSkyFlatPic(floorPic)) {
                        pDstSec->floorpic = -1;

                        const WadLumpName lumpName = W_GetLumpName(gpFlatTextures[floorPic].lumpNum);
                        skyLumpName[3] = lumpName.chars[5];
                        skyLumpName[4] = lumpName.chars[6];
                    } else {
                        ensureValidFlatPic(pDstSec->floorpic);
                    }
                #else
                    ensureValidFlatPic(pDstSec->floorpic);
                #endif
            } else {
                // Original PSX Doom specific stuff: have to lookup flat numbers from names. First, figure out the floor texture number.
                // PsyDoom: add support for floor skies too.
                #if PSYDOOM_MODS
                    const bool bFloorHasSky = (D_strncasecmp(pSrcSec->floorpic, SKY_LUMP_NAME, SKY_LUMP_NAME_LEN) == 0);

                    if (bFloorHasSky) {
                        // No floor texture: extract and save the 2 digits for the sky number ('01', '02' etc.)
                        pDstSec->floorpic = -1;
                        skyLumpName[3] = pSrcSec->floorpic[5];
                        skyLumpName[4] = pSrcSec->floorpic[6];
                    } else {
                        // Normal case: floor has a texture, save it's number
                        pDstSec->floorpic = R_FlatNumForName(pSrcSec->floorpic);
                        ensureValidFlatPic(pDstSec->floorpic);
                    }
                #else
                    pDstSec->floorpic = R_FlatNumForName(pSrcSec->floorpic);
                    ensureValidFlatPic(pDstSec->floorpic);
                #endif

                // Figure out ceiling texture number.
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
                    ensureValidFlatPic(pDstSec->ceilingpic);
                }
            }
            
            // PsyDoom: sector is not interpolated for the first frame!
            #if PSYDOOM_MODS
                R_SnapSectorInterpolation(*pDstSec);
            #endif

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
        // PsyDoom: the sky texture MUST exist, otherwise issue a fatal error
        #if PSYDOOM_MODS
            const int32_t skyTexIdx = R_TextureNumForName(skyLumpName, true);
        #else
            const int32_t skyTexIdx = R_TextureNumForName(skyLumpName);
        #endif

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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
    #endif

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

    // PsyDoom: add to the hash for the map and reset the list containing all player starts (including duplicates)
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
        gAllPlayerStarts.clear();
        gAllPlayerStarts.reserve(12);
    #endif

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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
    #endif

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
    D_memset(gpSides, std::byte(0), gNumSides * sizeof(side_t));

    // Read the map lump containing the sidedefs into a temp buffer from the map WAD
    W_ReadMapLump(lumpNum, pTmpBufferBytes, true);

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
    #endif

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
                // PsyDoom: use the overriden version of the textures if there are multiple versions of the same texture
                #if PSYDOOM_MODS
                    pDstSide->toptexture = R_GetOverrideTexNum(Endian::littleToHost(pSrcSide->toptexture));
                    pDstSide->midtexture = R_GetOverrideTexNum(Endian::littleToHost(pSrcSide->midtexture));
                    pDstSide->bottomtexture = R_GetOverrideTexNum(Endian::littleToHost(pSrcSide->bottomtexture));
                #else
                    pDstSide->toptexture = Endian::littleToHost(pSrcSide->toptexture);
                    pDstSide->midtexture = Endian::littleToHost(pSrcSide->midtexture);
                    pDstSide->bottomtexture = Endian::littleToHost(pSrcSide->bottomtexture);
                #endif
            } else {
                pDstSide->toptexture = R_TextureNumForName(pSrcSide->toptexture);
                pDstSide->midtexture = R_TextureNumForName(pSrcSide->midtexture);
                pDstSide->bottomtexture = R_TextureNumForName(pSrcSide->bottomtexture);

                // PsyDoom: level startup warnings if textures are defined but not found.
                // Note: these warnings may trigger on some original maps! For creating new maps however this can be a useful tool.
                #if PSYDOOM_MODS && PSYDOOM_MISSING_TEX_WARNINGS
                    constexpr auto isTexNameDefined = [](const char name[8]) noexcept -> bool {
                        // Valid name if not empty string or '-' string
                        return (name[0] && ((name[0] != '-') || name[1]));
                    };

                    if (isTexNameDefined(pSrcSide->toptexture) && (pDstSide->toptexture < 0) || (pDstSide->toptexture >= gNumTexLumps)) {
                        std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "W:bad u-tex for side %d!", sideIdx);
                    }

                    if (isTexNameDefined(pSrcSide->midtexture) && (pDstSide->midtexture < 0) || (pDstSide->midtexture >= gNumTexLumps)) {
                        std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "W:bad m-tex for side %d!", sideIdx);
                    }

                    if (isTexNameDefined(pSrcSide->bottomtexture) && (pDstSide->bottomtexture < 0) || (pDstSide->bottomtexture >= gNumTexLumps)) {
                        std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "W:bad l-tex for side %d!", sideIdx);
                    }
                #endif
            }

            // PsyDoom: side is not interpolated for the first frame!
            #if PSYDOOM_MODS
                R_SnapSideInterpolation(*pDstSide);
            #endif

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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(gpBlockmapLump, lumpSize);
    #endif

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
    const int32_t blockLinksSize = blockmapHeader.width * blockmapHeader.height * (int32_t) sizeof(gppBlockLinks[0]);
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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(gpRejectMatrix, lumpSize);
    #endif
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

    // PsyDoom: add to the hash for the map
    #if PSYDOOM_MODS
        MapHash::addData(pTmpBufferBytes, lumpSize);
    #endif

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

        for (int32_t lineIdx = 0; lineIdx < gNumLines; ++lineIdx, ++pLine) {
            sector_t* const pLineFrontSec = pLine->frontsector;
            sector_t* const pLineBackSec = pLine->backsector;

            // PsyDoom: rather than crashing gracefully handle orphaned lines with no sectors in the map data - just ignore them...
            #if PSYDOOM_MODS
                if (!pLineFrontSec)
                    continue;
            #endif

            ++pLineFrontSec->linecount;
            ++totalLineRefs;

            if (pLineBackSec && (pLineBackSec != pLineFrontSec)) {
                ++pLineBackSec->linecount;
                ++totalLineRefs;
            }
        }
    }

    // Alloc the array of line refs that will be shared by all sectors
    line_t** const pLineRefBuffer = (line_t**) Z_Malloc(*gpMainMemZone, totalLineRefs * sizeof(line_t*), PU_LEVEL, nullptr);
    line_t** pLineRef = pLineRefBuffer;

    // Build the list of lines for each sector, also bounding boxes and the 'sound origin' point
    ASSERT(gpSectors);
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

            for (int32_t lineIdx = 0; lineIdx < gNumLines; ++lineIdx, ++pLine) {
                sector_t* const pLineFrontSec = pLine->frontsector;
                sector_t* const pLineBackSec = pLine->backsector;

                // PsyDoom: rather than crashing gracefully handle orphaned lines with no sectors in the map data - just ignore them...
                #if PSYDOOM_MODS
                    if (!pLineFrontSec)
                        continue;
                #endif

                // Does this line belong to this sector?
                // If so save the line reference in the sector line list and add to the sector bounding box.
                if ((pLineFrontSec == pSec) || (pLineBackSec == pSec)) {
                    *pLineRef = pLine;
                    ++pLineRef;

                    M_AddToBox(bbox, pLine->vertex1->x, pLine->vertex1->y);
                    M_AddToBox(bbox, pLine->vertex2->x, pLine->vertex2->y);
                }
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
    // PsyDoom: make sure all animated textures and flats use the base picture - the engine expects this
    #if PSYDOOM_MODS
        P_SetAnimsToBasePic();
    #endif

    // PsyDoom limit removing: start filling from the beginning of VRAM
    #if PSYDOOM_LIMIT_REMOVING
        I_SetTexCacheFillPage(0);
    #endif

    // Load sector flats into VRAM if not already there.
    // PsyDoom limit removing: this code is now just flagging resources for later loading.
    {
        const sector_t* pSec = gpSectors;

        for (int32_t secIdx = 0; secIdx < gNumSectors; ++secIdx, ++pSec) {
            // Note: ceiling might not have a texture (sky)
            if (pSec->ceilingpic != -1) {
                #if PSYDOOM_MODS
                    const bool bValidCeilingPic = ((pSec->ceilingpic >= 0) && (pSec->ceilingpic < gNumFlatLumps));      // PsyDoom: added safety checks
                #else
                    const bool bValidCeilingPic = true;
                #endif

                if (bValidCeilingPic) {
                    #if PSYDOOM_LIMIT_REMOVING
                        gCacheFlatTextureSet.add(pSec->ceilingpic);
                    #else
                        texture_t& ceilTex = gpFlatTextures[pSec->ceilingpic];

                        if (!ceilTex.isCached()) {
                            I_CacheTex(ceilTex);
                        }
                    #endif
                }
            }

            #if PSYDOOM_MODS
                const bool bValidFloorPic = ((pSec->floorpic >= 0) && (pSec->floorpic < gNumFlatLumps));    // PsyDoom: added safety checks
            #else
                const bool bValidFloorPic = true;
            #endif

            if (bValidFloorPic) {
                #if PSYDOOM_LIMIT_REMOVING
                    gCacheFlatTextureSet.add(pSec->floorpic);
                #else
                    texture_t& floorTex = gpFlatTextures[pSec->floorpic];

                    if (!floorTex.isCached()) {
                        I_CacheTex(floorTex);
                    }
                #endif
            }
        }
    }

    #if PSYDOOM_MODS
        // PsyDoom: if not limit removing restrict flats to 1 texture page (flats should be on page '1') and start filling from page 2 onwards.
        // Also lock the texture page used by flats.
        #if !PSYDOOM_LIMIT_REMOVING
            for (uint32_t pageIdx = 2; pageIdx < I_GetNumTexCachePages(); ++pageIdx) {
                I_PurgeTexCachePage(pageIdx);
            }

            I_LockTexCachePage(1);
            I_SetTexCacheFillPage(2);
        #endif
    #else
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
        // So if you want to support more than 16 flats or do a more flexible VRAM arrangement, this is the place to look.
        gLockedTexPagesMask |= 0b0000'0000'0010;
        gTCacheFillPage = 2;
        gTCacheFillCellX = 0;
        gTCacheFillCellY = 0;
        gTCacheFillRowCellH = 0;
    #endif

    // Initialize the sky texture, palette and do some more specialized setup if it's the fire sky
    gUpdateFireSkyFunc = nullptr;
    gPaletteClutId_CurMapSky = gPaletteClutIds[MAINPAL];

    if (gpSkyTexture) {
        // If the lump name for the sky follows the format 'xxxx9' then assume it is a fire sky.
        // That needs to have it's lump cached, palette & update function set and initial few updates done...
        texture_t& skyTex = *gpSkyTexture;

        // PsyDoom: updates to work with the new WAD management code
        #if PSYDOOM_MODS
            const WadLumpName skyTexName = W_GetLumpName(skyTex.lumpNum);
        #else
            const lumpinfo_t& skyTexLump = gpLumpInfo[skyTex.lumpNum];
            const lumpname_t& skyTexName = skyTexLump.name;
        #endif

        if (skyTexName.chars[4] == '9') {
            W_CacheLumpNum(skyTex.lumpNum, PU_ANIMATION, true);
            gPaletteClutId_CurMapSky = gPaletteClutIds[FIRESKYPAL];
            gUpdateFireSkyFunc = P_UpdateFireSky;

            // PsyDoom: updates to work with the new WAD management code - ensure texture metrics are up-to-date!
            #if PSYDOOM_MODS
            {
                const WadLump& skyTexWadLump = W_GetLump(skyTex.lumpNum);
                const std::byte* const pLumpData = (const std::byte*) skyTexWadLump.pCachedData;
                R_UpdateTexMetricsFromData(skyTex, pLumpData, skyTexWadLump.uncompressedSize);
            }
            #endif

            // This gets the fire going, so it doesn't take a while to creep up when the map is started.
            // Do a number of fire update rounds before the player even enters the map:
            for (int32_t i = 0; i < 64; ++i) {
                P_UpdateFireSky(skyTex);
            }
        }

        // Final Doom: if the last digit in 'SKYXX' matches one of these digits, then use whatever palette is for that sky:
        if (Game::gConstants.bUseFinalDoomSkyPalettes) {
            switch (skyTexName.chars[4]) {
                case '2':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL1];    break;
                case '3':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL2];    break;
                case '4':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL3];    break;
                case '5':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL4];    break;
                case '6':   gPaletteClutId_CurMapSky = gPaletteClutIds[SKYPAL5];    break;
            }
        }

        // PsyDoom: MAPINFO allows the sky palette to be overriden
        #if PSYDOOM_MODS
            const MapInfo::Map* const pMap = MapInfo::getMap(gGameMap);

            if (pMap) {
                const int32_t skyPalOverride = pMap->skyPaletteOverride;

                if ((skyPalOverride >= 0) && ((uint32_t) skyPalOverride < MAXPALETTES)) {
                    gPaletteClutId_CurMapSky = gPaletteClutIds[skyPalOverride];
                }
            }
        #endif

        // Ensure the sky texture is in VRAM.
        // PsyDoom limit removing: this code is now just flagging the texture for later loading rather than loading directly.
        #if PSYDOOM_LIMIT_REMOVING
            const uint32_t skyTextureIdx = (uint32_t)(gpSkyTexture - gpTextures);
            gCacheTextureSet.add(skyTextureIdx);
        #else
            I_CacheTex(skyTex);
        #endif
    }

    // Load wall and switch textures into VRAM.
    // Note that switches are assumed to be 64 pixels wide, hence cached just before the 128 pixel textures.
    // PsyDoom limit removing: just flagging required textures here now, will sort and load later.
    #if PSYDOOM_LIMIT_REMOVING
        P_FlagMapTexturesForLoading();
    #else
        P_CacheMapTexturesWithWidth(16);
        P_CacheMapTexturesWithWidth(64);
    #endif

    P_InitSwitchList();

    #if !PSYDOOM_LIMIT_REMOVING
        P_CacheMapTexturesWithWidth(128);
    #endif

    // Give all sides without textures a default one.
    // Maybe done so constant validity checks don't have to be done elsewhere in the game to avoid crashing?
    //
    // PsyDoom: remove this code, so that we are preserving the concept of 'no texture' being defined for a side.
    // Otherwise this may have unintended consequences for some gameplay code, for example floor movers of the 'raiseToTexture' type.
    // We don't want some dummy texture or the WAD layout (whatever is the 1st texture) affecting the behavior of that mover type.
    // When a texture is undefined, it should be ignored by special actions that vary their behavior depending on texture.
    #if !PSYDOOM_MODS
    {
        side_t* pSide = gpSides;

        for (int32_t sideIdx = 0; sideIdx < gNumSides; ++sideIdx, ++pSide) {
            // These are the original checks: they just see if a side has an intentional '-1' texture (no-texture or 'don't care') and
            // assign the side a default texture to avoid the engine crashing when we try to access it.
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
    #endif

    // PsyDoom: check for sides that have invalid textures assigned other than '-1' (intentional no texture).
    // These missing textures are likely NOT intentional, so issue a warning when we find them - if that feature is enabled.
    // Also remap the bad texture indexes to 'no texture' or texture index '-1' so that the engine ignores them.
    #if PSYDOOM_MODS
        // Check against all lines
        for (int32_t lineIdx = 0; lineIdx < gNumLines; ++lineIdx) {
            const line_t& line = gpLines[lineIdx];
            const int32_t numSides = ((line.sidenum[1] >= 0) && (line.flags & ML_TWOSIDED)) ? 2 : 1;

            // Check against all sides of this line
            for (int32_t lineSide = 0; lineSide < numSides; ++lineSide) {
                // Get the side and sanity check the line data
                const int32_t sideIdx = line.sidenum[lineSide];

                if ((sideIdx < 0) || (sideIdx >= gNumSides)) {
                    I_Error("P_Init(): line %d with invalid side idx %d!", lineIdx, sideIdx);
                }

                side_t& side = gpSides[sideIdx];

                // Verify the side textures are OK and use placeholder 'no texture' (-1) if not:
                if ((side.toptexture < -1) || (side.toptexture >= gNumTexLumps)) {
                    #if PSYDOOM_MISSING_TEX_WARNINGS
                        // Only warn if the line is two sided, otherwise it cannot be a problem!
                        if (numSides == 2) {
                            std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "W:bad u-tex for side %d!", sideIdx);
                        }
                    #endif

                    side.toptexture = -1;
                }

                if ((side.midtexture < -1) || (side.midtexture >= gNumTexLumps)) {
                    #if PSYDOOM_MISSING_TEX_WARNINGS
                        std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "W:bad m-tex for side %d!", sideIdx);
                    #endif

                    side.midtexture = -1;
                }

                if ((side.bottomtexture < -1) || (side.bottomtexture >= gNumTexLumps)) {
                    #if PSYDOOM_MISSING_TEX_WARNINGS
                        // Only warn if the line is two sided, otherwise it cannot be a problem!
                        if (numSides == 2) {
                            std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "W:bad l-tex for side %d!", sideIdx);
                        }
                    #endif

                    side.bottomtexture = -1;
                }
            }
        }
    #endif

    // Some more hardcoded limits and memory arrangement for textures, this time for wall textures.
    // Lock down the texture pages used by wall textures and set the cache to start filling immediately after that (on the 6th page).
    //
    // Basically only the 3rd, 4th and 5th texture pages can be used for wall textures.
    // This gives a maximum wall texture area of 768x256 - everything else after that is reserved for sprites.
    // Even if the code above fills in more textures, they will eventually be evicted from the cache in favor of sprites.
    #if PSYDOOM_MODS
        // PsyDoom, non limit removing: lock the pages reserved for wall textures and purge all other cache pages (those are for sprites)
        #if !PSYDOOM_LIMIT_REMOVING
            I_LockTexCachePage(2);
            I_LockTexCachePage(3);
            I_LockTexCachePage(4);
            I_SetTexCacheFillPage(5);

            for (uint32_t pageIdx = 5; pageIdx < I_GetNumTexCachePages(); ++pageIdx) {
                I_PurgeTexCachePage(pageIdx);
            }
        #endif
    #else
        gLockedTexPagesMask |= 0b0000'0001'1100;
        gTCacheFillPage = 5;
        gTCacheFillCellX = 0;
        gTCacheFillCellY = 0;
        gTCacheFillRowCellH = 0;
    #endif

    // PsyDoom: new limit removing texture management code.
    // Load all of the wall and flat textures that were previously flagged to be loaded.
    // Switch to using loose packing for in-game sprites also after all textures and flats are loaded.
    #if PSYDOOM_LIMIT_REMOVING
        P_LoadMapTextures();
        I_TexCacheUseLoosePacking(true);
    #endif

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
        // Texture cache: unlock everything except UI assets and other reserved areas of VRAM.
        // In limit removing mode also ensure we are using tight packing of VRAM.
        #if PSYDOOM_MODS
            #if PSYDOOM_LIMIT_REMOVING
                I_LockAllWallAndFloorTextures(false);
                I_TexCacheUseLoosePacking(false);
            #else
                I_UnlockAllTexCachePages();
                I_LockTexCachePage(0);
            #endif
        #else
            gLockedTexPagesMask &= 1;
        #endif

        Z_FreeTags(*gpMainMemZone, PU_ANIMATION);
    }

    I_PurgeTexCache();
    Z_CheckHeap(*gpMainMemZone);
    M_ClearRandom();

    // PsyDoom: initialize the map object weak referencing system
    #if PSYDOOM_MODS
        P_InitWeakRefs();
    #endif

    // PsyDoom limit removing: init the sets of wall and flat textures to be loaded
    #if PSYDOOM_LIMIT_REMOVING
        gCacheTextureSet.init(gNumTexLumps);
        gCacheFlatTextureSet.init(gNumFlatLumps);
    #endif

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

    gMobjHead.next = &gMobjHead;
    gMobjHead.prev = &gMobjHead;

    // Setup the item respawn queue and dead player removal queue index
    gItemRespawnQueueHead = 0;
    gItemRespawnQueueTail = 0;
    gDeadPlayerRemovalQueueIdx = 0;

    // Figure out which file to open for the map WAD.
    //
    // Note: for Final Doom I've added logic to allow for MAPXX.WAD or MAPXX.ROM, with a preference for the .WAD file format.
    // Also, if the file for the map has the .ROM extension then it is assumed the map data is in Final Doom format.
    // 
    // PsyDoom: this logic is now changed to account for the fact that file ids are no longer simple integers.
    // I'm also allowing a .WAD file override to be used even if the there is a .ROM on disk (Final Doom).
    #if PSYDOOM_MODS
        constexpr auto makeMapFileId = [](const int32_t mapNum, const char* const extension) noexcept {
            char name[64];
            std::sprintf(name, "MAP%02d.%s", mapNum, extension);
            return CdFileId(name);
        };

        const CdFileId mapWadFile_doom = makeMapFileId(mapNum, "WAD");
        const CdFileId mapWadFile_finalDoom = makeMapFileId(mapNum, "ROM");
        gbLoadingFinalDoomMap = ((CdMapTbl_GetEntry(mapWadFile_doom) == PsxCd_MapTblEntry{}) && (!ModMgr::areOverridesAvailableForFile(mapWadFile_doom)));
    #else
        const int32_t mapIndex = mapNum - 1;
        const int32_t mapFolderIdx = mapIndex / LEVELS_PER_MAP_FOLDER;
        const int32_t mapIdxInFolder = mapIndex - mapFolderIdx * LEVELS_PER_MAP_FOLDER;
        const int32_t mapFolderOffset = mapFolderIdx * NUM_FILES_PER_LEVEL * LEVELS_PER_MAP_FOLDER;

        const CdFileId mapWadFile_doom = (CdFileId)((int32_t) CdFileId::MAP01_WAD + mapIdxInFolder + mapFolderOffset);
        const CdFileId mapWadFile_finalDoom = (CdFileId)((int32_t) CdFileId::MAP01_ROM + mapIndex);

        gbLoadingFinalDoomMap = (!gCdMapTbl[(int32_t) mapWadFile_doom].startSector);
    #endif

    const CdFileId mapWadFile = (gbLoadingFinalDoomMap) ? mapWadFile_finalDoom : mapWadFile_doom;

    // Open the map wad.
    // PsyDoom: this no longer returns a data pointer with the new WAD code.
    #if PSYDOOM_MODS
        W_OpenMapWad(mapWadFile);
    #else
        void* const pMapWadFileData = W_OpenMapWad(mapWadFile);
    #endif

    // PsyDoom: don't use this lump relative indexing anymore, just search for the lump names we want.
    // This also allows more flexibility where the 'MAP01' etc. markers in the map WAD can just be ignored - map files can be renamed more easily.
    #if !PSYDOOM_MODS
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
    #endif

    // Loading various map lumps.
    // PsyDoom: not using relative indexing anymore to load map lumps, search for the lump names instead.
    // PsyDoom: clear the map hash before starting to load level lumps that will add to the hash.
    #if PSYDOOM_MODS
        MapHash::clear();
        P_LoadBlockMap(W_MapGetNumForName("BLOCKMAP"));
        P_LoadVertexes(W_MapGetNumForName("VERTEXES"));
        P_LoadSectors(W_MapGetNumForName("SECTORS"));
        P_LoadSideDefs(W_MapGetNumForName("SIDEDEFS"));
        P_LoadLineDefs(W_MapGetNumForName("LINEDEFS"));
        P_LoadSubSectors(W_MapGetNumForName("SSECTORS"));
        P_LoadNodes(W_MapGetNumForName("NODES"));
        P_LoadSegs(W_MapGetNumForName("SEGS"));
        P_LoadLeafs(W_MapGetNumForName("LEAFS"));
        P_LoadRejectMap(W_MapGetNumForName("REJECT"));
    #else
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
    #endif

    // Build sector line lists etc.
    P_GroupLines();

    // Load and spawn map things; also initialize the next deathmatch start
    gpDeathmatchP = &gDeathmatchStarts[0];

    #if PSYDOOM_MODS
        P_LoadThings(W_MapGetNumForName("THINGS"));     // PsyDoom: not using relative indexing anymore to load map lumps, search for the lump names instead
        ScriptingEngine::init();                        // PsyDoom: initialize the scripting engine if the map has Lua scripted actions
        MapHash::finalize();                            // PsyDoom: compute the final map hash
        MapPatcher::applyPatches();                     // PsyDoom: apply any patches to original map data that are relevant at this point, once all things have been loaded
        // PsyDoom: if playing deathmatch or 'no monsters' setting is set, activate all special tagged boss sectors
        if (gNetGame == gt_deathmatch && (Game::gSettings.bDmActivateSpecialSectors)) {
            for (int32_t i = 666; i <= 672; ++i) {
                line_t dummyLine = {};
                dummyLine.tag = i;
                switch (dummyLine.tag) {
                case 666:
                    EV_DoFloor(dummyLine, lowerFloorToLowest);
                    break;
                case 667:
                    EV_DoFloor(dummyLine, raiseFloor24);
                    break;
                case 668:
                    EV_DoFloor(dummyLine, lowerFloorToLowest);
                    break;
                case 669:
                    EV_DoFloor(dummyLine, lowerFloorToLowest);
                    break;
                case 670:
                    EV_DoDoor(dummyLine, Open);
                    break;
                case 671:
                    EV_DoFloor(dummyLine, lowerFloorToLowest);
                    break;
                case 672:
                    EV_DoDoor(dummyLine, Open);
                    break;
                }
            }
        }
    #else
        P_LoadThings(mapStartLump + ML_THINGS);
    #endif

    // Spawn special thinkers such as light flashes etc. and free up the loaded WAD data.
    // PsyDoom: the WAD manager is now responsible for freeing up resources used by the map WAD.
    P_SpawnSpecials();

    #if PSYDOOM_MODS
        W_CloseMapWad();
    #else
        Z_Free2(*gpMainMemZone, pMapWadFileData);
    #endif

    // Loading map textures and sprites.
    //
    // PsyDoom: loading .IMG files for maps (e.g  MAPSPR01.IMG and MAPTEX01.IMG) is no longer done to make modding easier.
    // Instead, all graphical resources will be loaded from WAD files. The .IMG files no longer need to be produced for PsyDoom and we can
    // add new texture etc. lumps to the main IWAD without worrying about invalidating lump number references in existing .IMG files.
    // Note also that 'P_LoadBlocks' was removed from this module as part of this change. It can be found in the 'Old' code folder if required.
    if (!gbIsLevelBeingRestarted) {
        #if !PSYDOOM_MODS
            const CdFileId mapTexFile = (CdFileId)((int32_t) CdFileId::MAPTEX01_IMG + mapIdxInFolder + mapFolderOffset);
            const CdFileId mapSprFile = (CdFileId)((int32_t) CdFileId::MAPSPR01_IMG + mapIdxInFolder + mapFolderOffset);
            P_LoadBlocks(mapTexFile);
        #endif

        P_Init();

        #if !PSYDOOM_MODS
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

    // PsyDoom: in single player and co-op spawn 'Voodoo dolls' if there are duplicate player starts.
    // For each player start that is not the actual one used, spawn a 'Voodoo doll'.
    #if PSYDOOM_MODS
        const auto spawnVoodooDollsForPlayer = [](const int32_t playerIdx) noexcept {
            for (const mapthing_t& playerStart : gAllPlayerStarts) {
                // Is this start for this player?
                if (playerStart.type == playerIdx + 1) {
                    // Is this start different to the actual one used?
                    if (std::memcmp(&gPlayerStarts[playerIdx], &playerStart, sizeof(mapthing_t)) != 0) {
                        // This is a 'Voodoo doll' player start, spawn the doll and remove any 'noclip' cheats it might have inherited from the player:
                        P_SpawnPlayer(playerStart);
                        gPlayers[playerIdx].mo->flags &= ~(MF_NOCLIP);
                    }
                }
            }
        };

        spawnVoodooDollsForPlayer(0);

        if (gNetGame == gt_coop) {
            spawnVoodooDollsForPlayer(1);
        }
    #endif

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

        // PsyDoom: if the 'in-place' level reload cheat is active then position the player to where we were before the reload
        #if PSYDOOM_MODS
            if (gbDoInPlaceLevelReload) {
                gbDoInPlaceLevelReload = false;

                // Move the player on the xy axes
                player_t& player = gPlayers[gCurPlayerIndex];
                mobj_t& playerMobj = *player.mo;
                P_UnsetThingPosition(playerMobj);
                playerMobj.x = gInPlaceReloadPlayerX;
                playerMobj.y = gInPlaceReloadPlayerY;
                P_SetThingPosition(playerMobj);

                // Set player z position, view height and angle
                const sector_t& newSector = *playerMobj.subsector->sector;
                playerMobj.angle = gInPlaceReloadPlayerAng;
                playerMobj.floorz = newSector.floorheight;
                playerMobj.ceilingz = newSector.ceilingheight;
                playerMobj.z = std::max(gInPlaceReloadPlayerZ, (fixed_t) newSector.floorheight);
                player.viewz = playerMobj.z + VIEWHEIGHT;
                player.viewheight = VIEWHEIGHT;
                player.deltaviewheight = 0;

                // Snap all interpolations
                R_SnapMobjInterpolation(playerMobj);
            }
        #endif
    }

    // PsyDoom: monitor the current map file for changes if appropriate
    #if PSYDOOM_MODS
        DevMapAutoReloader::init(mapWadFile);
    #endif
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: adds the specified map thing to the list of all player starts (including duplicates) for the level.
// Used to add support for so called 'Voodoo dolls'.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_AddPlayerStart(const mapthing_t& mapThing) noexcept {
    gAllPlayerStarts.push_back(mapThing);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: caches and decompresses the lump for the specified texture.
// Once done, saves the size info for the texture to the texture object.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_CacheAndUpdateTexSizeInfo(texture_t& tex, const int32_t lumpNum) noexcept {
    // Sanity check
    if (W_LumpLength(lumpNum) < (int32_t) sizeof(texlump_header_t)) {
        I_Error("P_CacheAndUpdateTexSizeInfo: Bad tex lump %d! Not enough data!", lumpNum);
    }

    // Cache the texture data and get the header from the first bytes.
    // PsyDoom: updates to work with the new WAD management code.
    #if PSYDOOM_MODS
        const WadLump& wadLump = W_CacheLumpNum(lumpNum, PU_CACHE, true);
        const texlump_header_t texHdr = *(texlump_header_t*) wadLump.pCachedData;
    #else
        const texlump_header_t texHdr = *(texlump_header_t*) W_CacheLumpNum(lumpNum, PU_CACHE, true);
    #endif

    // Save the texture size info.
    // Note: the header 'offset' field is deliberately ignored for textures (only used for sprites).
    tex.offsetX = {};
    tex.offsetY = {};
    tex.width = Endian::littleToHost(texHdr.width);
    tex.height = Endian::littleToHost(texHdr.height);
    tex.width16 = (uint8_t)((tex.width + 15u) / 16u);
    tex.height16 = (uint8_t)((tex.height + 15u) / 16u);
}
#endif  // #if PSYDOOM_MODS

#if PSYDOOM_LIMIT_REMOVING
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom limit removing addition: flags all textures in the map for loading
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_FlagMapTexturesForLoading() noexcept {
    // Run through all the sides in the map and flag all their textures for loading
    side_t* pSide = gpSides;

    for (int32_t sideIdx = 0; sideIdx < gNumSides; ++sideIdx, ++pSide) {
        const int32_t topTexture = pSide->toptexture;
        const int32_t midTexture = pSide->midtexture;
        const int32_t botTexture = pSide->bottomtexture;

        if ((topTexture >= 0) && (topTexture < gNumTexLumps)) {
            gCacheTextureSet.add(topTexture);
        }

        if ((midTexture >= 0) && (midTexture < gNumTexLumps)) {
            gCacheTextureSet.add(midTexture);
        }

        if ((botTexture >= 0) && (botTexture < gNumTexLumps)) {
            gCacheTextureSet.add(botTexture);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom limit removing addition: loads previously all requested map textures.
// The textures are sorted and loaded based on their height, with the tallest textures going first.
// Any texture size for wall and flat textures up to 256x256 is allowed.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadMapTextures() noexcept {
    // Gather pointers to all the textures to be loaded, for both flats and walls.
    //
    // While doing this, also refresh the size info we have for each texture by loading the header bytes from it's corresponding lump.
    // PsyDoom doesn't require modders to add new textures to the 'TEXTURE1' info lump (for ease of modding) and flats are now no longer
    // assumed to be always 64x64. Because of this, retrieving (on-demand) the size info by caching the texture's lump is required.
    // Hopefully with PsyDoom's larger heap this data will not be evicted before we go to actually cache the texture into VRAM, so it won't
    // be a duplicated loading operation.
    gLoadTextureList.clear();
    gLoadTextureList.reserve((size_t)(gCacheFlatTextureSet.size() + gCacheTextureSet.size()));

    gCacheTextureSet.forEachIndex(
        [](const uint64_t idx) noexcept {
            texture_t& tex = gpTextures[idx];
            gLoadTextureList.push_back(&tex);
            P_CacheAndUpdateTexSizeInfo(tex, tex.lumpNum);
        }
    );

    gCacheFlatTextureSet.forEachIndex(
        [](const uint64_t idx) noexcept {
            texture_t& tex = gpFlatTextures[idx];
            gLoadTextureList.push_back(&tex);
            P_CacheAndUpdateTexSizeInfo(tex, tex.lumpNum);
        }
    );

    // Sort the textures in ascending order of height
    std::sort(
        gLoadTextureList.begin(),
        gLoadTextureList.end(),
        [](texture_t* pTex1, texture_t* pTex2) noexcept {
            // Sort based on texture cache cell height since this is what affects packing.
            // If the height is equal prefer fatter textures first.
            if (pTex1->height16 == pTex2->height16)
                return (pTex1->width16 > pTex2->width16);

            return (pTex1->height16 > pTex2->height16);
        }
    );

    // Cache all of the textures, starting at the beginning of available VRAM
    I_SetTexCacheFillPage(0);
    uint32_t maxTexturePage = 1;

    for (texture_t* pTex : gLoadTextureList) {
        I_CacheTex(*pTex);
        maxTexturePage = std::max(maxTexturePage, I_GetCurTexCacheFillPage());
    }

    // Ensure all wall and floor textures are locked.
    // Only sprites can be unloaded from VRAM during gameplay.
    I_LockAllWallAndFloorTextures(true);
}
#endif  //  #if PSYDOOM_LIMIT_REMOVING

#if !PSYDOOM_LIMIT_REMOVING
//------------------------------------------------------------------------------------------------------------------------------------------
// Cache map textures with the given width into VRAM.
// Textures are cached in groups according to their width in order to try and make more efficient use of VRAM space.
// PsyDoom limit removing: this function is no longer needed because of the new (more flexible) texture management logic.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_CacheMapTexturesWithWidth(const int32_t width) noexcept {
    // Round the current fill position in the texture cache up to the nearest multiple of the given texture width.
    // This ensures that for instance 64 pixel textures are on 64 pixel boundaries on the x dimension.
    // PsyDoom: don't see any reason to do this anymore with the new texture cache management code, leave it out.
    #if !PSYDOOM_MODS
    {
        const int32_t cellX = gTCacheFillCellX;
        const int32_t cellW = ((width >= 0) ? width : width + 15) / TCACHE_CELL_SIZE;   // This is a signed floor() operation
        const int32_t cellXRnd = (cellX + (cellW - 1)) & (-cellW);                      // Note: '&(-cellW)' chops off the unwanted lower bits from the result
        gTCacheFillCellX = cellXRnd;
    }
    #endif

    // PsyDoom: added safety to ensure the texture number is valid, before we cache it:
    #if PSYDOOM_MODS
        const auto isValidTexNum = [](const int32_t num) noexcept { return ((num >= 0) && (num < gNumTexLumps)); };
    #else
        const auto isValidTexNum = [](const int32_t num) noexcept { return (num != -1); };
    #endif

    // Run through all the sides in the map and cache textures with the specified width.
    //
    // PsyDoom: the 'TEXTURE1' lump is no longer used to know in advance the dimensions of all textures before they are loaded (to make modding easier).
    // Hence we need to ensure each texture is loaded into RAM first before doing the size comparisons below.
    side_t* pSide = gpSides;

    for (int32_t sideIdx = 0; sideIdx < gNumSides; ++sideIdx, ++pSide) {
        if (isValidTexNum(pSide->toptexture)) {
            texture_t& tex = gpTextures[pSide->toptexture];

            #if PSYDOOM_MODS
                P_CacheAndUpdateTexSizeInfo(tex, tex.lumpNum);  // PsyDoom: make sure we know the texture size before comparison!
            #endif

            if ((tex.width == width) && (!tex.isCached())) {
                I_CacheTex(tex);
            }
        }

        if (isValidTexNum(pSide->midtexture)) {
            texture_t& tex = gpTextures[pSide->midtexture];

            #if PSYDOOM_MODS
                P_CacheAndUpdateTexSizeInfo(tex, tex.lumpNum);  // PsyDoom: make sure we know the texture size before comparison!
            #endif

            if ((tex.width == width) && (!tex.isCached())) {
                I_CacheTex(tex);
            }
        }

        if (isValidTexNum(pSide->bottomtexture)) {
            texture_t& tex = gpTextures[pSide->bottomtexture];

            #if PSYDOOM_MODS
                P_CacheAndUpdateTexSizeInfo(tex, tex.lumpNum);  // PsyDoom: make sure we know the texture size before comparison!
            #endif

            if ((tex.width == width) && (!tex.isCached())) {
                I_CacheTex(tex);
            }
        }
    }
}
#endif  // #if !PSYDOOM_LIMIT_REMOVING
