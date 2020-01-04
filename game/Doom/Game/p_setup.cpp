#include "p_setup.h"

#include "Doom/Base/i_file.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/m_bbox.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/cdmaptbl.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "g_game.h"
#include "p_firesky.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "p_switch.h"
#include "PsxVm/PsxVm.h"
#include "PsxVm/VmSVal.h"

// How much heap space is required after loading the map in order to run the game (48 KiB).
// If we don't have this much then the game craps out with an error.
// Need to be able to support various small allocs throughout gameplay for particles and so forth.
static constexpr int32_t MIN_REQ_HEAP_SPACE_FOR_GAMEPLAY = 0xC000;

// How many maps are in a map folder and the number of files per maps folder etc.
static constexpr int32_t LEVELS_PER_MAP_FOLDER = (uint32_t) CdMapTbl_File::MAPSPR01_IMG - (uint32_t) CdMapTbl_File::MAP01_WAD;
static constexpr int32_t NUM_FILES_PER_LEVEL = 3;
static constexpr int32_t FILES_PER_MAP_FOLDER = LEVELS_PER_MAP_FOLDER * NUM_FILES_PER_LEVEL;

// Sky stuff
static constexpr const char* SKY_LUMP_NAME = "F_SKY";
static constexpr int32_t SKY_LUMP_NAME_LEN = sizeof("F_SKY") - 1;   // -1 to discount null terminator

// Map data
const VmPtr<VmPtr<uint16_t>>        gpBlockmapLump(0x800780C4);
const VmPtr<VmPtr<uint16_t>>        gpBlockmap(0x80078140);
const VmPtr<int32_t>                gBlockmapWidth(0x80078284);
const VmPtr<int32_t>                gBlockmapHeight(0x80077EB8);
const VmPtr<fixed_t>                gBlockmapOriginX(0x8007818C);
const VmPtr<fixed_t>                gBlockmapOriginY(0x80078194);
const VmPtr<VmPtr<VmPtr<mobj_t>>>   gppBlockLinks(0x80077EDC);
const VmPtr<int32_t>                gNumVertexes(0x80078018);
const VmPtr<VmPtr<vertex_t>>        gpVertexes(0x800781E4);
const VmPtr<int32_t>                gNumSectors(0x80077F54);
const VmPtr<VmPtr<sector_t>>        gpSectors(0x800780A8);
const VmPtr<int32_t>                gNumSides(0x800781B4);
const VmPtr<VmPtr<side_t>>          gpSides(0x80077EA0);
const VmPtr<int32_t>                gNumLines(0x800781C8);
const VmPtr<VmPtr<line_t>>          gpLines(0x80077EB0);

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map vertex data from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadVertexes(const int32_t lumpNum) noexcept {
    // Read the WAD vertexes into the temp buffer from the map WAD
    const int32_t lumpSize = W_MapLumpLength(lumpNum);
    
    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadVertexes: lump > 64K");
    }

    *gNumVertexes = lumpSize / sizeof(mapvertex_t);
    *gpVertexes = (vertex_t*) Z_Malloc(**gpMainMemZone, *gNumVertexes * sizeof(vertex_t), PU_LEVEL, nullptr);
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Convert the vertexes to the renderer runtime format
    const mapvertex_t* pSrcVertex = (const mapvertex_t*) gTmpBuffer.get();
    vertex_t* pDstVertex = gpVertexes->get();

    for (int32_t vertexIdx = 0; vertexIdx < *gNumVertexes; ++vertexIdx) {
        pDstVertex->x = pSrcVertex->x;
        pDstVertex->y = pSrcVertex->y;
        pDstVertex->index = 0;
        ++pSrcVertex;
        ++pDstVertex;
    }
}

void P_LoadSegs() noexcept {
loc_80021BA0:
    sp -= 0x20;
    sw(s0, sp + 0x18);
    sw(ra, sp + 0x1C);
    s0 = a0;
    _thunk_W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80021BD4;
    I_Error("P_LoadSegs: lump > 64K");
loc_80021BD4:
    a0 = s0;
    _thunk_W_MapLumpLength();
    v1 = 0xAAAA0000;                                    // Result = AAAA0000
    v1 |= 0xAAAB;                                       // Result = AAAAAAAB
    multu(v0, v1);
    a2 = 2;                                             // Result = 00000002
    a3 = 0;                                             // Result = 00000000
    a0 = *gpMainMemZone;
    v0 = hi;
    v0 >>= 3;
    a1 = v0 << 2;
    a1 += v0;
    sw(v0, gp + 0xAC4);                                 // Store to: gNumSegs (800780A4)
    a1 <<= 3;
    _thunk_Z_Malloc();
    a0 = v0;
    v0 = lw(gp + 0xAC4);                                // Load from: gNumSegs (800780A4)
    a1 = 0;                                             // Result = 00000000
    sw(a0, gp + 0xC58);                                 // Store to: gpSegs (80078238)
    a2 = v0 << 2;
    a2 += v0;
    a2 <<= 3;
    _thunk_D_memset();
    a0 = s0;
    s0 = gTmpBuffer;
    a1 = gTmpBuffer;
    a2 = 1;                                             // Result = 00000001
    _thunk_W_ReadMapLump();
    v0 = lw(gp + 0xAC4);                                // Load from: gNumSegs (800780A4)
    t1 = lw(gp + 0xC58);                                // Load from: gpSegs (80078238)
    t2 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80021DC4;
    t0 = s0 + 8;                                        // Result = gTmpWadLumpBuffer[2] (80098750)
    a1 = t1 + 0xC;
    t4 = *gpVertexes;
    t3 = *gpSides;
loc_80021C6C:
    v1 = lh(s0);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v0 += t4;
    sw(v0, t1);
    v1 = lh(t0 - 0x6);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v0 += t4;
    sw(v0, a1 - 0x8);
    v0 = lh(t0 - 0x4);
    v0 <<= 16;
    sw(v0, a1);
    v0 = lh(t0 + 0x2);
    v0 <<= 16;
    sw(v0, a1 - 0x4);
    v1 = lh(t0 - 0x2);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = *gpLines;
    v0 <<= 2;
    a2 = v0 + v1;
    sw(a2, a1 + 0x8);
    a3 = lh(t0);
    a0 = a3 << 2;
    a0 += a2;
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += t3;
    sw(v0, a1 + 0x4);
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += t3;
    v0 = lw(v0 + 0x14);
    sw(v0, a1 + 0xC);
    v0 = lw(a2 + 0x10);
    v0 &= 4;
    {
        const bool bJump = (v0 == 0);
        v0 = a3 ^ 1;
        if (bJump) goto loc_80021D7C;
    }
    v0 <<= 2;
    v0 += a2;
    v1 = lw(v0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += t3;
    v0 = lw(v0 + 0x14);
    sw(v0, a1 + 0x10);
    goto loc_80021D80;
loc_80021D7C:
    sw(0, a1 + 0x10);
loc_80021D80:
    v1 = lw(a2);
    v0 = lw(t1);
    t2++;
    if (v1 != v0) goto loc_80021DA4;
    v0 = lw(a1);
    v0 >>= 19;
    sw(v0, a2 + 0x48);
loc_80021DA4:
    a1 += 0x28;
    t1 += 0x28;
    t0 += 0xC;
    v0 = lw(gp + 0xAC4);                                // Load from: gNumSegs (800780A4)
    v0 = (i32(t2) < i32(v0));
    s0 += 0xC;
    if (v0 != 0) goto loc_80021C6C;
loc_80021DC4:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map subsectors using data from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadSubSectors(const int32_t lumpNum) noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x18);

    a0 = lumpNum;
    _thunk_W_MapLumpLength();

    if (i32(v0) > TMP_BUFFER_SIZE) {
        I_Error("P_LoadSubsectors: lump > 64K");
    }

    a0 = lumpNum;
    _thunk_W_MapLumpLength();
    v0 >>= 2;
    sw(v0, gp + 0xC44);         // Store to: gNumSubsectors (80078224)

    a0 = *gpMainMemZone;
    a1 = v0 << 4;
    a2 = 2;
    a3 = 0;
    _thunk_Z_Malloc();
    sw(v0, gp + 0x960);         // Store to: gpSubsectors (80077F40)

    a0 = lumpNum;
    a1 = gTmpBuffer;
    a2 = 1;
    _thunk_W_ReadMapLump();

    a0 = lw(gp + 0x960);        // Load from: gpSubsectors (80077F40)
    a1 = 0;
    a2 = lw(gp + 0xC44);        // Load from: gNumSubsectors (80078224)    
    a2 <<= 4;
    _thunk_D_memset();

    a0 = 0;
    a1 = lw(gp + 0xC44);        // Load from: gNumSubsectors (80078224)
    v0 = lw(gp + 0x960);        // Load from: gpSubsectors (80077F40)
    s0 = gTmpBuffer;

    if (i32(a1) > 0) {
        v1 = v0 + 0xA;

        do {
            v0 = lhu(s0);
            a0++;
            sh(v0, v1 - 0x6);
            v0 = lhu(s0 + 0x2);
            s0 += 4;
            sh(0, v1 - 0x2);
            sh(0, v1);
            sh(v0, v1 - 0x4);
            v0 = (i32(a0) < i32(a1));
            v1 += 0x10;
        } while (v0 != 0);
    }

    s0 = lw(sp + 0x18);
    sp += 0x28;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map sector data from the specified map lump number.
// Also sets up the sky texture pointer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadSectors(const int32_t lumpNum) noexcept {
    // Store the name of the sky lump here
    VmSVal<lumpname_t> skyLumpName = {};
    skyLumpName->chars[0] = 'S';
    skyLumpName->chars[1] = 'K';
    skyLumpName->chars[2] = 'Y';

    // Sanity check the sectors lump is not too big
    const int32_t lumpSize = W_MapLumpLength(lumpNum);

    if (lumpSize > TMP_BUFFER_SIZE) {
        I_Error("P_LoadSectors: lump > 64K");
    }

    // Alloc ram for the runtime sectors and zero initialize
    *gNumSectors = lumpSize / sizeof(mapsector_t);
    *gpSectors = (sector_t*) Z_Malloc(**gpMainMemZone, *gNumSectors * sizeof(sector_t), PU_LEVEL, nullptr);
    D_memset(gpSectors->get(), (std::byte) 0, *gNumSectors * sizeof(sector_t));

    // Read the map lump containing the sectors
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Process the WAD sectors and convert them into runtime sectors
    {
        const mapsector_t* pSrcSec = (const mapsector_t*) gTmpBuffer.get();
        sector_t* pDstSec = gpSectors->get();

        for (int32_t secIdx = 0; secIdx < *gNumSectors; ++secIdx) {
            // Save basic properties
            pDstSec->floorheight = (fixed_t) pSrcSec->floorheight << FRACBITS;
            pDstSec->ceilingheight = (fixed_t) pSrcSec->ceilingheight << FRACBITS;
            pDstSec->colorid = pSrcSec->colorid;
            pDstSec->lightlevel = pSrcSec->lightlevel;
            pDstSec->special = pSrcSec->special;
            pDstSec->thinglist = nullptr;
            pDstSec->tag = pSrcSec->tag;
            pDstSec->flags = pSrcSec->flags;

            // Figure out floor texture number
            a0 = ptrToVmAddr(pSrcSec->floorpic);            
            R_FlatNumForName();
            pDstSec->floorpic = v0;

            // Figure out ceiling texture numebr.
            // Note: if the ceiling has a sky then figure out the sky lump name for it instead - will load the sky lump later on.
            const bool bCeilHasSky = (D_strncasecmp(pSrcSec->ceilingpic, SKY_LUMP_NAME, SKY_LUMP_NAME_LEN) == 0);

            if (bCeilHasSky) {
                // No ceiling texture: extract and save the 2 digits for the sky number ('01', '02' etc.)
                pDstSec->ceilingpic = -1;
                skyLumpName->chars[3] = pSrcSec->ceilingpic[5];
                skyLumpName->chars[4] = pSrcSec->ceilingpic[6];
            } else {
                // Normal case: ceiling has a texture, save it's number
                a0 = ptrToVmAddr(pSrcSec->ceilingpic);
                R_FlatNumForName();
                pDstSec->ceilingpic = v0;
            }

            ++pSrcSec;
            ++pDstSec;
        }
    }

    // Set the sky texture pointer
    if (skyLumpName->chars[3] != 0) {
        a0 = skyLumpName.addr();
        R_TextureNumForName();
        *gpSkyTexture = &(*gpTextures)[v0];
    } else {
        *gpSkyTexture = nullptr;
    }
}

void P_LoadNodes() noexcept {
loc_80022104:
    sp -= 0x20;
    sw(s0, sp + 0x18);
    sw(ra, sp + 0x1C);
    s0 = a0;
    _thunk_W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80022138;
    I_Error("P_LoadNodes: lump > 64K");
loc_80022138:
    a0 = s0;
    _thunk_W_MapLumpLength();
    v1 = 0x24920000;                                    // Result = 24920000
    v1 |= 0x4925;                                       // Result = 24924925
    v0 >>= 2;
    multu(v0, v1);
    a2 = 2;                                             // Result = 00000002
    a3 = 0;                                             // Result = 00000000
    a0 = *gpMainMemZone;
    v0 = hi;
    a1 = v0 << 3;
    a1 -= v0;
    sw(v0, gp + 0xBD8);                                 // Store to: gNumBspNodes (800781B8)
    a1 <<= 3;
    _thunk_Z_Malloc();
    a0 = s0;
    s0 = gTmpBuffer;
    a1 = gTmpBuffer;
    sw(v0, gp + 0x8C4);                                 // Store to: gpBspNodes (80077EA4)
    a2 = 1;                                             // Result = 00000001
    _thunk_W_ReadMapLump();
    v0 = lw(gp + 0xBD8);                                // Load from: gNumBspNodes (800781B8)
    t3 = lw(gp + 0x8C4);                                // Load from: gpBspNodes (80077EA4)
    t6 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80022264;
    t5 = s0 + 6;                                        // Result = gTmpWadLumpBuffer[1] (8009874E)
    t4 = t3 + 0xC;
loc_800221AC:
    t2 = 0;                                             // Result = 00000000
    v0 = lh(s0);
    t1 = s0;
    v0 <<= 16;
    sw(v0, t3);
    v0 = lh(t5 - 0x4);
    t0 = t3;
    v0 <<= 16;
    sw(v0, t4 - 0x8);
    v0 = lh(t5 - 0x2);
    a3 = s0;
    v0 <<= 16;
    sw(v0, t4 - 0x4);
    v0 = lh(t5);
    a2 = t3;
    v0 <<= 16;
    sw(v0, t4);
loc_800221F0:
    a1 = 0;                                             // Result = 00000000
    a0 = t0 + 0x10;
    v0 = lhu(a3 + 0x18);
    v1 = t1 + 8;
    sw(v0, a2 + 0x30);
loc_80022204:
    v0 = lh(v1);
    v1 += 2;
    a1++;
    v0 <<= 16;
    sw(v0, a0);
    v0 = (i32(a1) < 4);
    a0 += 4;
    if (v0 != 0) goto loc_80022204;
    t1 += 8;                                            // Result = gTmpWadLumpBuffer[2] (80098750)
    t0 += 0x10;
    a3 += 2;                                            // Result = gTmpWadLumpBuffer[0] (8009874A)
    t2++;                                               // Result = 00000001
    v0 = (i32(t2) < 2);                                 // Result = 00000001
    a2 += 4;
    if (v0 != 0) goto loc_800221F0;
    t6++;                                               // Result = 00000001
    t4 += 0x38;
    t3 += 0x38;
    t5 += 0x1C;                                         // Result = gTmpWadLumpBuffer[8] (8009876A)
    v0 = lw(gp + 0xBD8);                                // Load from: gNumBspNodes (800781B8)
    v0 = (i32(t6) < i32(v0));
    s0 += 0x1C;                                         // Result = gTmpWadLumpBuffer[7] (80098764)
    if (v0 != 0) goto loc_800221AC;
loc_80022264:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load map things and spawn them using data from the specified map lump number
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_LoadThings(const int32_t lumpNum) noexcept {
    sp -= 0x30;
    sw(s0, sp + 0x18);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);

    a0 = lumpNum;
    _thunk_W_MapLumpLength();

    if (i32(v0) > TMP_BUFFER_SIZE) {
        I_Error("P_LoadThings: lump > 64K");
    }

    a0 = lumpNum;
    _thunk_W_MapLumpLength();
    v1 = 0xCCCCCCCD;        // Result = CCCCCCCD
    multu(v0, v1);
    v0 = hi;
    s3 = v0 >> 3;

    a0 = lumpNum;
    a1 = gTmpBuffer;
    a2 = 1;
    _thunk_W_ReadMapLump();

    if (s3 != 0) {
        s1 = gTmpBuffer;
        s0 = s1 + 6;        // Result = gTmpWadLumpBuffer[1] (8009874E)
        s2 = 0;

        do {
            v0 = lhu(s1);
            sh(v0, s1);
            v0 = lhu(s0 - 0x4);
            v1 = lhu(s0 - 0x2);
            a1 = lhu(s0);
            a2 = lhu(s0 + 0x2);
            a0 = s1;
            sh(v0, s0 - 0x4);
            sh(v1, s0 - 0x2);
            sh(a1, s0);
            sh(a2, s0 + 0x2);
            P_SpawnMapThing();
            a1 = lh(s0);
            v0 = (i32(a1) < 0x1000);
            s2++;

            if (v0 == 0) {
                I_Error("P_LoadThings: doomednum:%d >= 4096", (int32_t) a1);
            }
            
            s0 += 0xA;
            s1 += 0xA;
        } while (i32(s2) < i32(s3));
    }

    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
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
    D_memset(gpLines->get(), (std::byte) 0, *gNumLines * sizeof(line_t));

    // Read the map lump containing the sidedefs
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Process the WAD linedefs and convert them into runtime linedefs
    const maplinedef_t* pSrcLine = (maplinedef_t*) gTmpBuffer.get();
    line_t* pDstLine = gpLines->get();

    for (int32_t lineIdx = 0; lineIdx < *gNumLines; ++lineIdx) {
        // Save some basic line properties
        pDstLine->flags = pSrcLine->flags;
        pDstLine->special = pSrcLine->special;
        pDstLine->tag = pSrcLine->tag;

        // Save line vertices, delta coordinates and slope type
        vertex_t& vertex1 = (*gpVertexes)[pSrcLine->vertex1];
        vertex_t& vertex2 = (*gpVertexes)[pSrcLine->vertex2];

        pDstLine->vertex1 = &vertex1;
        pDstLine->vertex2 = &vertex2;
        pDstLine->dx = vertex2.x - vertex1.x;
        pDstLine->dy = vertex2.y - vertex1.y;

        if (pDstLine->dx == 0) {
            pDstLine->slopetype = slopetype_t::ST_VERTICAL;
        } else if (pDstLine->dy == 0) {
            pDstLine->slopetype = slopetype_t::ST_HORIZONTAL;
        } else {
            a0 = pDstLine->dy;
            a1 = pDstLine->dx;
            FixedDiv();

            if (int32_t(v0) > 0) {
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
        const int32_t sidenum1 = pSrcLine->sidenum[0];
        const int32_t sidenum2 = pSrcLine->sidenum[1];

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
    D_memset(gpSides->get(), (std::byte) 0, *gNumSides  * sizeof(side_t));

    // Read the map lump containing the sidedefs
    W_ReadMapLump(lumpNum, gTmpBuffer.get(), true);

    // Process the WAD sidedefs and convert them into runtime sidedefs
    const mapsidedef_t* pSrcSide = (const mapsidedef_t*) gTmpBuffer.get();
    side_t* pDstSide = gpSides->get();

    for (int32_t sideIdx = 0; sideIdx < *gNumSides; ++sideIdx) {
        pDstSide->textureoffset = (fixed_t) pSrcSide->textureoffset << FRACBITS;
        pDstSide->rowoffset = (fixed_t) pSrcSide->rowoffset << FRACBITS;
        pDstSide->sector = &(*gpSectors)[pSrcSide->sector];

        a0 = ptrToVmAddr(pSrcSide->toptexture);
        R_TextureNumForName();
        pDstSide->toptexture = v0;

        a0 = ptrToVmAddr(pSrcSide->midtexture);
        R_TextureNumForName();
        pDstSide->midtexture = v0;

        a0 = ptrToVmAddr(pSrcSide->bottomtexture);
        R_TextureNumForName();
        pDstSide->bottomtexture = v0;

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

    // The first 8 bytes of the blockmap are it's header.    
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

    // I'm not sure why this code was doing this pointless copy - disable as it is useless.
    // Maybe an accidental leftover from when the code was doing something else?
    #if !PC_PSX_DOOM_MODS
    {
        const int32_t count = (lumpSize / 2) + (lumpSize & 1);

        for (int32_t i = 0; i < count; ++i) {
            (*gpBlockmapLump)[i] = (*gpBlockmapLump)[i];
        }
    }
    #endif

    // Save blockmap dimensions
    *gBlockmapWidth = blockmapHeader.width;
    *gBlockmapHeight = blockmapHeader.height;
    *gBlockmapOriginX = (fixed_t) blockmapHeader.originx << FRACBITS;
    *gBlockmapOriginY = (fixed_t) blockmapHeader.originy << FRACBITS;
    
    // Alloc and null initialize the list of map objects for each block
    const int32_t blockLinksSize = (int32_t) blockmapHeader.width * (int32_t) blockmapHeader.height * sizeof(VmPtr<mobj_t>);
    *gppBlockLinks = (VmPtr<mobj_t>*) Z_Malloc(**gpMainMemZone, blockLinksSize, PU_LEVEL, nullptr);
    D_memset(gppBlockLinks->get(), (std::byte) 0, blockLinksSize);
}

void P_LoadMapLump() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    _thunk_W_MapLumpLength();
    a1 = v0;
    a2 = 2;
    a0 = *gpMainMemZone;
    a3 = 0;
    _thunk_Z_Malloc();
    a0 = s0;
    a1 = v0;
    sw(a1, gp + 0xB04);                                 // Store to: gpRejectMatrix (800780E4)
    a2 = 1;                                             // Result = 00000001
    _thunk_W_ReadMapLump();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_LoadLeafs() noexcept {
loc_80022920:
    sp -= 0x48;
    sw(s2, sp + 0x28);
    s2 = a0;
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    _thunk_W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    fp = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80022974;
    I_Error("P_LoadLeafs: lump > 64K");
loc_80022974:
    a0 = s2;
    s0 = gTmpBuffer;
    a1 = gTmpBuffer;
    a2 = 1;
    _thunk_W_ReadMapLump();
    s1 = 0;
    s4 = gTmpBuffer;
loc_80022994:
    a0 = s2;
    _thunk_W_MapLumpLength();
    v0 += s0;
    v0 = (s4 < v0);
    if (v0 == 0) goto loc_800229C8;
    v0 = lh(s4);
    fp++;
    s1 += v0;
    v0 <<= 2;
    v0 += 2;
    s4 += v0;
    goto loc_80022994;
loc_800229C8:
    v0 = lw(gp + 0xC44);                                // Load from: gNumSubsectors (80078224)
    s4 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    if (fp == v0) goto loc_800229E8;
    I_Error("P_LoadLeafs: leaf/subsector inconsistancy");
loc_800229E8:
    a1 = s1 << 3;
    a2 = 2;
    a0 = *gpMainMemZone;
    a3 = 0;
    _thunk_Z_Malloc();
    s6 = v0;
    v0 = lw(gp + 0x960);                                // Load from: gpSubsectors (80077F40)
    sw(s6, gp + 0xB2C);                                 // Store to: gpLeafEdges (8007810C)
    sw(0, gp + 0x984);                                  // Store to: gTotalNumLeafEdges (80077F64)
    s7 = 0;                                             // Result = 00000000
    if (i32(fp) <= 0) goto loc_80022B24;
    s2 = v0 + 8;
loc_80022A1C:
    v0 = lhu(s4);
    sh(v0, s2);
    v0 = lhu(gp + 0x984);                               // Load from: gTotalNumLeafEdges (80077F64)
    v1 = lh(s2);
    s5 = 0;                                             // Result = 00000000
    sh(v0, s2 + 0x2);
    if (i32(v1) <= 0) goto loc_80022AF8;
    s3 = s6 + 4;
    s1 = s4;
loc_80022A44:
    s0 = lh(s1 + 0x2);
    v0 = *gNumVertexes;
    v0 = (i32(s0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 3;
        if (bJump) goto loc_80022A70;
    }
    I_Error("P_LoadLeafs: vertex out of range\n");
    v0 = s0 << 3;
loc_80022A70:
    v0 -= s0;
    v1 = *gpVertexes;
    v0 <<= 2;
    v0 += v1;
    sw(v0, s6);
    s0 = lh(s1 + 0x4);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s0 != v0) goto loc_80022A9C;
    sw(0, s3);
    goto loc_80022AD8;
loc_80022A9C:
    v0 = lw(gp + 0xAC4);                                // Load from: gNumSegs (800780A4)
    v0 = (i32(s0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_80022AC4;
    }
    I_Error("P_LoadLeafs: seg out of range\n");
    v0 = s0 << 2;
loc_80022AC4:
    v0 += s0;
    v1 = lw(gp + 0xC58);                                // Load from: gpSegs (80078238)
    v0 <<= 3;
    v0 += v1;
    sw(v0, s3);
loc_80022AD8:
    s1 += 4;
    s5++;
    s3 += 8;
    v0 = lh(s2);
    v0 = (i32(s5) < i32(v0));
    s6 += 8;
    if (v0 != 0) goto loc_80022A44;
loc_80022AF8:
    s7++;
    v0 = lh(s2);
    v1 = lw(gp + 0x984);                                // Load from: gTotalNumLeafEdges (80077F64)
    v0 <<= 2;
    v0 += 2;
    s4 += v0;
    v1 += s5;
    v0 = (i32(s7) < i32(fp));
    sw(v1, gp + 0x984);                                 // Store to: gTotalNumLeafEdges (80077F64)
    s2 += 0x10;
    if (v0 != 0) goto loc_80022A1C;
loc_80022B24:
    ra = lw(sp + 0x44);
    fp = lw(sp + 0x40);
    s7 = lw(sp + 0x3C);
    s6 = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x48;
    return;
}

void P_GroupLines() noexcept {
loc_80022B58:
    a1 = lw(gp + 0xC44);                                // Load from: gNumSubsectors (80078224)
    a0 = lw(gp + 0x960);                                // Load from: gpSubsectors (80077F40)
    sp -= 0x60;
    sw(s4, sp + 0x50);
    s4 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x5C);
    sw(s6, sp + 0x58);
    sw(s5, sp + 0x54);
    sw(s3, sp + 0x4C);
    sw(s2, sp + 0x48);
    sw(s1, sp + 0x44);
    sw(s0, sp + 0x40);
    if (i32(a1) <= 0) goto loc_80022BC8;
    a2 = lw(gp + 0xC58);                                // Load from: gpSegs (80078238)
loc_80022B90:
    v1 = lh(a0 + 0x6);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 3;
    v0 += a2;
    v0 = lw(v0 + 0x10);
    v0 = lw(v0 + 0x14);
    s4++;
    sw(v0, a0);
    v0 = (i32(s4) < i32(a1));
    a0 += 0x10;
    if (v0 != 0) goto loc_80022B90;
loc_80022BC8:
    a1 = 0;
    v0 = *gNumLines;
    s1 = *gpLines;
    s4 = 0;
    if (i32(v0) <= 0) goto loc_80022C3C;
    a2 = v0;
    a0 = s1 + 0x38;
loc_80022BE4:
    v1 = lw(a0);
    v0 = lw(v1 + 0x54);
    v0++;
    sw(v0, v1 + 0x54);
    v1 = lw(a0 + 0x4);
    a1++;
    if (v1 == 0) goto loc_80022C2C;
    v0 = lw(a0);
    if (v1 == v0) goto loc_80022C2C;
    v0 = lw(v1 + 0x54);
    a1++;
    v0++;
    sw(v0, v1 + 0x54);
loc_80022C2C:
    s4++;
    v0 = (i32(s4) < i32(a2));
    a0 += 0x4C;
    if (v0 != 0) goto loc_80022BE4;
loc_80022C3C:
    a1 <<= 2;
    a2 = 2;
    s4 = 0;
    a0 = *gpMainMemZone;
    a3 = 0;
    _thunk_Z_Malloc();
    s5 = v0;
    v0 = *gNumSectors;
    s6 = *gpSectors;
    s3 = s6 + 0x30;
    if (i32(v0) <= 0) goto loc_80022E3C;
loc_80022C6C:
    a0 = sp + 0x10;
    M_ClearBox();
    s1 = *gpLines;
    v0 = *gNumLines;
    s2 = 0;                                             // Result = 00000000
    sw(s5, s3 + 0x28);
    if (i32(v0) <= 0) goto loc_80022CFC;
    s0 = s1 + 4;
loc_80022C8C:
    v0 = lw(s0 + 0x34);
    if (v0 == s6) goto loc_80022CAC;
    v0 = lw(s0 + 0x38);
    if (v0 != s6) goto loc_80022CE0;
loc_80022CAC:
    sw(s1, s5);
    v0 = lw(s1);
    a1 = lw(v0);
    a2 = lw(v0 + 0x4);
    a0 = sp + 0x10;
    M_AddToBox();
    v0 = lw(s0);
    s5 += 4;
    a1 = lw(v0);
    a2 = lw(v0 + 0x4);
    a0 = sp + 0x10;
    M_AddToBox();
loc_80022CE0:
    s2++;
    s0 += 0x4C;
    v0 = *gNumLines;
    v0 = (i32(s2) < i32(v0));
    s1 += 0x4C;
    if (v0 != 0) goto loc_80022C8C;
loc_80022CFC:
    v0 = lw(s3 + 0x28);
    v1 = lw(s3 + 0x24);
    v0 = s5 - v0;
    v0 = u32(i32(v0) >> 2);
    if (v0 == v1) goto loc_80022D24;
    I_Error("P_GroupLines: miscounted");
loc_80022D24:
    v0 = lw(sp + 0x1C);
    v1 = lw(sp + 0x18);
    v0 += v1;
    v1 = v0 >> 31;
    v0 += v1;
    v0 = u32(i32(v0) >> 1);
    sw(v0, s3 + 0x8);
    a1 = lw(sp + 0x10);
    v0 = lw(sp + 0x14);
    a0 = lw(s3 + 0x8);
    a1 += v0;
    v0 = a1 >> 31;
    a1 += v0;
    a1 = u32(i32(a1) >> 1);
    sw(a1, s3 + 0xC);
    R_PointInSubsector();
    sw(v0, s3 + 0x14);
    v1 = lw(sp + 0x10);
    v0 = *gBlockmapOriginY;
    a0 = *gBlockmapHeight;
    v1 -= v0;
    v0 = 0x200000;                                      // Result = 00200000
    v1 += v0;
    v1 = u32(i32(v1) >> 23);
    v0 = (i32(v1) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = v1;
        if (bJump) goto loc_80022D98;
    }
    v0 = a0 - 1;
loc_80022D98:
    sw(v0, s3 - 0x8);
    v0 = lw(sp + 0x14);
    v1 = *gBlockmapOriginY;
    v0 -= v1;
    v1 = 0xFFE00000;                                    // Result = FFE00000
    v0 += v1;
    v1 = u32(i32(v0) >> 23);
    if (i32(v1) >= 0) goto loc_80022DC4;
    v1 = 0;                                             // Result = 00000000
loc_80022DC4:
    sw(v1, s3 - 0x4);
    v1 = lw(sp + 0x1C);
    v0 = *gBlockmapOriginX;
    a0 = *gBlockmapWidth;
    v1 -= v0;
    v0 = 0x200000;                                      // Result = 00200000
    v1 += v0;
    v1 = u32(i32(v1) >> 23);
    v0 = (i32(v1) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = v1;
        if (bJump) goto loc_80022DF4;
    }
    v0 = a0 - 1;
loc_80022DF4:
    sw(v0, s3 + 0x4);
    v0 = lw(sp + 0x18);
    v1 = *gBlockmapOriginX;
    v0 -= v1;
    v1 = 0xFFE00000;                                    // Result = FFE00000
    v0 += v1;
    v1 = u32(i32(v0) >> 23);
    s4++;
    if (i32(v1) >= 0) goto loc_80022E20;
    v1 = 0;                                             // Result = 00000000
loc_80022E20:
    sw(v1, s3);
    s3 += 0x5C;
    v0 = *gNumSectors;
    v0 = (i32(s4) < i32(v0));
    s6 += 0x5C;
    if (v0 != 0) goto loc_80022C6C;
loc_80022E3C:
    ra = lw(sp + 0x5C);
    s6 = lw(sp + 0x58);
    s5 = lw(sp + 0x54);
    s4 = lw(sp + 0x50);
    s3 = lw(sp + 0x4C);
    s2 = lw(sp + 0x48);
    s1 = lw(sp + 0x44);
    s0 = lw(sp + 0x40);
    sp += 0x60;
    return;
}

void P_InitMapTextures() noexcept {
loc_80022E68:
    v0 = *gNumSectors;
    v1 = *gpSectors;
    sp -= 0x30;
    sw(s0, sp + 0x20);
    s0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    if (i32(v0) <= 0) goto loc_80022F0C;
    s2 = -1;                                            // Result = FFFFFFFF
    s1 = v1 + 8;
loc_80022E94:
    v1 = lw(s1 + 0x4);
    {
        const bool bJump = (v1 == s2);
        v1 <<= 5;
        if (bJump) goto loc_80022ECC;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EDC);                               // Load from: gpFlatTextures (80078124)
    a0 = v1 + v0;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80022ECC;
    I_CacheTex();
loc_80022ECC:
    v0 = lw(s1);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7EDC);                               // Load from: gpFlatTextures (80078124)
    v0 <<= 5;
    a0 = v0 + v1;
    v0 = lhu(a0 + 0xA);
    s0++;
    if (v0 != 0) goto loc_80022EF8;
    I_CacheTex();
loc_80022EF8:
    v0 = *gNumSectors;
    v0 = (i32(s0) < i32(v0));
    s1 += 0x5C;
    if (v0 != 0) goto loc_80022E94;
loc_80022F0C:
    v0 = 2;                                             // Result = 00000002
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FD8);                                // Store to: gTexCacheFillPage (80078028)
    v0 = *gLockedTexPagesMask;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F7C);                              // Load from: gPaletteClutId_Main (800A9084)
    a0 = *gpSkyTexture;
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D1C);                                 // Store to: gTexCacheFillBlockX (800782E4)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D18);                                 // Store to: gTexCacheFillBlockY (800782E8)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D88);                                 // Store to: gTexCacheRowBlockH (80078278)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7B34);                                 // Store to: gpUpdateFireSkyFunc (80077B34)
    v0 |= 2;
    *gLockedTexPagesMask = v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0x7D34);                                // Store to: gPaletteClutId_CurMapSky (800782CC)
    if (a0 == 0) goto loc_80022FE8;
    a0 = lh(a0 + 0x10);
    v1 = *gpLumpInfo;
    v0 = a0 << 4;
    v0 += v1;
    v1 = lbu(v0 + 0xC);
    v0 = 0x39;                                          // Result = 00000039
    a1 = 8;                                             // Result = 00000008
    if (v1 != v0) goto loc_80022FD8;
    a2 = 1;                                             // Result = 00000001
    _thunk_W_CacheLumpNum();
    s0 = 0;                                             // Result = 00000000
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F5E);                              // Load from: gPaletteClutId_Sky (800A90A2)
    v0 = 0x80020000;                                    // Result = 80020000
    v0 += 0x7CB0;                                       // Result = P_UpdateFireSky (80027CB0)
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7B34);                                // Store to: gpUpdateFireSkyFunc (80077B34)
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0x7D34);                                // Store to: gPaletteClutId_CurMapSky (800782CC)
loc_80022FBC:
    a0 = *gpSkyTexture;
    s0++;
    P_UpdateFireSky();
    v0 = (i32(s0) < 0x40);
    if (v0 != 0) goto loc_80022FBC;
loc_80022FD8:
    a0 = *gpSkyTexture;
    I_CacheTex();
loc_80022FE8:
    a0 = 0x10;                                          // Result = 00000010
    P_CacheMapTexturesWithWidth();
    a0 = 0x40;                                          // Result = 00000040
    P_CacheMapTexturesWithWidth();
    s0 = 0;                                             // Result = 00000000
    P_InitSwitchList();
    a0 = 0x80;                                          // Result = 00000080
    P_CacheMapTexturesWithWidth();
    v1 = *gNumSides;
    v0 = *gpSides;
    a0 = -1;                                            // Result = FFFFFFFF
    if (i32(v1) <= 0) goto loc_80023068;
    a1 = v1;
    v1 = v0 + 0xC;
loc_80023020:
    v0 = lw(v1 - 0x4);
    if (v0 != a0) goto loc_80023034;
    sw(0, v1 - 0x4);
loc_80023034:
    v0 = lw(v1 + 0x4);
    if (v0 != a0) goto loc_80023048;
    sw(0, v1 + 0x4);
loc_80023048:
    v0 = lw(v1);
    s0++;
    if (v0 != a0) goto loc_8002305C;
    sw(0, v1);
loc_8002305C:
    v0 = (i32(s0) < i32(a1));
    v1 += 0x18;
    if (v0 != 0) goto loc_80023020;
loc_80023068:
    v1 = *gLockedTexPagesMask;
    v0 = 5;                                             // Result = 00000005
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D1C);                                 // Store to: gTexCacheFillBlockX (800782E4)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D18);                                 // Store to: gTexCacheFillBlockY (800782E8)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FD8);                                // Store to: gTexCacheFillPage (80078028)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D88);                                 // Store to: gTexCacheRowBlockH (80078278)
    v1 |= 0x1C;
    *gLockedTexPagesMask = v1;
    
    Z_FreeTags(**gpMainMemZone, PU_CACHE);

    P_InitPicAnims();
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void P_SetupLevel(const int32_t mapNum, [[maybe_unused]] const skill_t skill) noexcept {
loc_800230D4:
    VmSVal<lumpname_t> mapLumpName;

    sp -= 0x98;    
    sw(s0, sp + 0x78);
    sw(s1, sp + 0x7C);
    sw(s2, sp + 0x80);
    sw(s3, sp + 0x84);

    Z_FreeTags(**gpMainMemZone, PU_CACHE|PU_LEVSPEC|PU_LEVEL);

    if (!*gbIsLevelBeingRestarted) {
        *gLockedTexPagesMask &= 1;
        Z_FreeTags(**gpMainMemZone, PU_ANIMATION);
    }
    
    I_ResetTexCache();
    Z_CheckHeap(**gpMainMemZone);
    M_ClearRandom();

    // Init player stats for the map
    v1 = 0;                                             // Result = 00000000
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7F20);                                 // Store to: gTotalKills (80077F20)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7F2C);                                 // Store to: gTotalItems (80077F2C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7FEC);                                 // Store to: gTotalSecret (80077FEC)

    s1 = 0;

    do {
        at = 0x800B0000;                                    // Result = 800B0000
        at -= 0x774C;                                       // Result = gPlayer1[32] (800A88B4)
        at += v1;
        sw(0, at);
        at = 0x800B0000;                                    // Result = 800B0000
        at -= 0x7744;                                       // Result = gPlayer1[34] (800A88BC)
        at += v1;
        sw(0, at);
        at = 0x800B0000;                                    // Result = 800B0000
        at -= 0x7748;                                       // Result = gPlayer1[33] (800A88B8)
        at += v1;
        sw(0, at);
        at = 0x800B0000;                                    // Result = 800B0000
        at -= 0x77B0;                                       // Result = gPlayer1[19] (800A8850)
        at += v1;
        sw(0, at);
        s1++;
        v0 = (i32(s1) < 2);
        v1 += 0x12C;
    } while (v0 != 0);

    // Setup the thinkers and map objects lists
    v0 = 0x80096550;                                    // Result = gThinkerCap[0] (80096550)
    sw(v0, v0);                                         // Store to: gThinkerCap[0] (80096550)
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x6554);                                // Store to: gThinkerCap[1] (80096554)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    v1 = v0 - 0x14;                                     // Result = gMObjHead[0] (800A8E90)
    sw(v1, v0);                                         // Store to: gMObjHead[5] (800A8EA4)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v1, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)

    // Setup the item respawn queue and map object kill count
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7EC8);                                 // Store to: gItemRespawnQueueHead (80078138)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7E80);                                 // Store to: gItemRespawnQueueTail (80078180)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7FF0);                                 // Store to: gNumMObjKilled (80078010)

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
    mapLumpName->chars[0] = 'M';
    mapLumpName->chars[1] = 'A';
    mapLumpName->chars[2] = 'P';

    {
        uint8_t digit1 = (uint8_t) mapNum / 10;
        uint8_t digit2 = (uint8_t) mapNum - digit1 * 10;
        mapLumpName->chars[3] = '0' + digit1;
        mapLumpName->chars[4] = '0' + digit2;
    }
    
    // Get the lump index for the map start lump
    const uint32_t mapStartLump = W_MapCheckNumForName(mapLumpName->chars);
    
    if (mapStartLump == -1) {
        I_Error("P_SetupLevel: %s not found", mapLumpName->chars);
    }

    // Loading various map lumps
    P_LoadBlockMap(mapStartLump + ML_BLOCKMAP);
    P_LoadVertexes(mapStartLump + ML_VERTEXES);
    P_LoadSectors(mapStartLump + ML_SECTORS);
    P_LoadSideDefs(mapStartLump + ML_SIDEDEFS);
    P_LoadLineDefs(mapStartLump + ML_LINEDEFS);
    P_LoadSubSectors(mapStartLump + ML_SSECTORS);

    a0 = mapStartLump + ML_NODES;
    P_LoadNodes();

    a0 = mapStartLump + ML_SEGS;
    P_LoadSegs();
    
    a0 = mapStartLump + ML_LEAFS;
    P_LoadLeafs();

    // Load the reject map
    a0 = mapStartLump + ML_REJECT;
    _thunk_W_MapLumpLength();

    a1 = v0;
    a2 = 2;
    a0 = *gpMainMemZone;
    a3 = 0;
    _thunk_Z_Malloc();
    sw(v0, gp + 0xB04);     // Store to: gpRejectMatrix (800780E4)

    a0 = mapStartLump + ML_REJECT;
    a1 = v0;    
    a2 = 1;
    _thunk_W_ReadMapLump();

    // Build sector line lists etc.
    P_GroupLines();

    // Deathmatch starts
    v0 = 0x8009806C;        // Result = gDeathmatchStarts[0] (8009806C)              
    sw(v0, gp + 0xA80);     // Store to: gpDeathmatchP (80078060)

    // Load and spawn things
    P_LoadThings(mapStartLump + ML_THINGS);
    
    // Spawn special thinkers such as light flashes etc. and free up the loaded WAD data
    P_SpawnSpecials();
    Z_Free2(**gpMainMemZone, pMapWadFileData);

    // Loading map textures and sprites
    if (!*gbIsLevelBeingRestarted) {
        const CdMapTbl_File mapTexFile = (CdMapTbl_File)((int32_t) CdMapTbl_File::MAPTEX01_IMG + mapIdxInFolder + mapFolderOffset);
        const CdMapTbl_File mapSprFile = (CdMapTbl_File)((int32_t) CdMapTbl_File::MAPSPR01_IMG + mapIdxInFolder + mapFolderOffset);
        
        P_LoadBlocks(mapTexFile);
        P_InitMapTextures();
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
        s1 = 0;
        I_NetHandshake();
        s3 = 0x800A8E7C;            // Result = gPlayer1MapThing[0] (800A8E7C)
        s2 = 0;

        do {
            a2 = 0;
            a3 = 0;
            a0 = lh(s3);                // Load from: gPlayer1MapThing[0] (800A8E7C)
            a1 = lh(s3 + 0x2);          // Load from: gPlayer1MapThing[1] (800A8E7E)
            a0 <<= 16;
            a1 <<= 16;
            P_SpawnMObj();
            s0 = v0;
            at = 0x800A87EC;            // Result = gPlayer1[0] (800A87EC)
            at += s2;
            sw(s0, at);
            a0 = s1;
            G_DoReborn();
            a0 = s0;
            P_RemoveMObj();
            s1++;
            v0 = (i32(s1) < 2);
            s2 += 0x12C;
        } while (v0 != 0);
    }
    else {
        a0 = 0x800A8E7C;    // Result = gPlayer1MapThing[0] (800A8E7C)
        P_SpawnPlayer();
    }
    
    s3 = lw(sp + 0x84);
    s2 = lw(sp + 0x80);
    s1 = lw(sp + 0x7C);
    s0 = lw(sp + 0x78);
    sp += 0x98;
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

void P_CacheSprite() noexcept {
    sp -= 0x38;
    sw(s5, sp + 0x2C);
    s5 = a0;
    sw(s4, sp + 0x28);
    sw(ra, sp + 0x30);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    v0 = lw(s5);
    s3 = lw(s5 + 0x4);
    s4 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80023AA0;
loc_80023A0C:
    s2 = 0;                                             // Result = 00000000
    s1 = s3;
loc_80023A14:
    s0 = lw(s1 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FEC);                               // Load from: gFirstSpriteLumpNum (80078014)
    v0 = (i32(s0) < i32(v0));
    if (v0 != 0) goto loc_80023A48;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F38);                               // Load from: gLastSpriteLumpNum (80077F38)
    v0 = (i32(v0) < i32(s0));
    a0 = s0;
    if (v0 == 0) goto loc_80023A5C;
loc_80023A48:
    I_Error("CacheSprite: invalid sprite lump %d", (int32_t) s0);
    a0 = s0;
loc_80023A5C:
    a1 = 8;                                             // Result = 00000008
    a2 = 0;                                             // Result = 00000000
    _thunk_W_CacheLumpNum();
    v0 = lw(s3);
    if (v0 == 0) goto loc_80023A88;
    s2++;                                               // Result = 00000001
    v0 = (i32(s2) < 8);                                 // Result = 00000001
    s1 += 4;
    if (v0 != 0) goto loc_80023A14;
loc_80023A88:
    s4++;
    v0 = lw(s5);
    v0 = (i32(s4) < i32(v0));
    s3 += 0x2C;
    if (v0 != 0) goto loc_80023A0C;
loc_80023AA0:
    ra = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x38;
    return;
}

void P_CacheMapTexturesWithWidth() noexcept {
loc_80023AC8:
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D1C);                               // Load from: gTexCacheFillBlockX (800782E4)
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    v1 = v0 - 1;
    if (i32(s1) >= 0) goto loc_80023AF8;
    a0 = s1 + 0xF;
loc_80023AF8:
    v0 = u32(i32(a0) >> 4);
    v1 += v0;
    v0 = -v0;
    v1 &= v0;
    v0 = *gNumSides;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7D1C);                                // Store to: gTexCacheFillBlockX (800782E4)
    v1 = *gpSides;
    s2 = 0;
    if (i32(v0) <= 0) goto loc_80023C14;
    s3 = -1;                                            // Result = FFFFFFFF
    s0 = v1 + 0xC;
loc_80023B28:
    v1 = lw(s0 - 0x4);
    {
        const bool bJump = (v1 == s3);
        v1 <<= 5;
        if (bJump) goto loc_80023B70;
    }
    v0 = *gpTextures;
    a0 = v1 + v0;
    v0 = lh(a0 + 0x4);
    if (v0 != s1) goto loc_80023B70;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80023B70;
    I_CacheTex();
loc_80023B70:
    v1 = lw(s0 + 0x4);
    {
        const bool bJump = (v1 == s3);
        v1 <<= 5;
        if (bJump) goto loc_80023BB8;
    }
    v0 = *gpTextures;
    a0 = v1 + v0;
    v0 = lh(a0 + 0x4);
    if (v0 != s1) goto loc_80023BB8;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80023BB8;
    I_CacheTex();
loc_80023BB8:
    v1 = lw(s0);
    s2++;
    if (v1 == s3) goto loc_80023C00;
    v0 = *gpTextures;
    v1 <<= 5;
    a0 = v1 + v0;
    v0 = lh(a0 + 0x4);
    if (v0 != s1) goto loc_80023C00;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80023C00;
    I_CacheTex();
loc_80023C00:
    v0 = *gNumSides;
    v0 = (i32(s2) < i32(v0));
    s0 += 0x18;
    if (v0 != 0) goto loc_80023B28;
loc_80023C14:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}
