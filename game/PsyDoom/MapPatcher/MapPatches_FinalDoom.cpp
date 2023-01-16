//------------------------------------------------------------------------------------------------------------------------------------------
// A module containing map patches to apply to PlayStation Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatches.h"

#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_telept.h"
#include "Doom/Renderer/r_data.h"

BEGIN_NAMESPACE(MapPatches)

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP07: Geryon
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Geryon() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix a door leading to the secret area with the super shotgun not having it's texture move as it lowers
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 966);
        gpSides[gpLines[966].sidenum[0]].rowoffset = 64 * FRACUNIT;

        // In the room immediately after the blue door, fix the wall textures so they move when the wall does and re-align
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 710, 712, 723, 726);
        gpSides[gpLines[710].sidenum[0]].rowoffset = -50 * FRACUNIT;
        gpSides[gpLines[712].sidenum[0]].rowoffset = -50 * FRACUNIT;
        gpSides[gpLines[723].sidenum[0]].rowoffset = -34 * FRACUNIT;
        gpSides[gpLines[726].sidenum[0]].rowoffset = -34 * FRACUNIT;
        gpSides[gpLines[733].sidenum[0]].rowoffset = 0 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP08: Minos
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Minos() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        addFlagsToLinedefs(ML_DONTPEGBOTTOM,
            // Fix hidden door tracks in north central hallway that move when they shouldn't
            727, 729,
            // Fix door tracks in NE corner of blue door room that move when they shouldn't
            639, 641, 642, 644
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP09: Nessus
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Nessus() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix the BFG secret being inaccessible - transfer it to a neighboring sector:
        gpSectors[57].special = 0;
        gpSectors[60].special = 9;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP10: Paradox
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Paradox() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix texture on hidden elevator in castle so it moved with the platform and shift texture
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 325);
        gpSides[gpLines[325].sidenum[0]].rowoffset = 65 * FRACUNIT;

        // Hide opening to monster closet and fix texture alignment
        addFlagsToLinedefs(ML_DONTDRAW, 552, 554, 555);
        addFlagsToLinedefs(ML_SECRET | ML_MIDMASKED, 553);
        gpSides[gpLines[553].sidenum[0]].midtexture = R_TextureNumForName("ROCK03");
        gpSides[gpLines[553].sidenum[0]].textureoffset = -16 * FRACUNIT;

        // Fix texture alignment on deathmatch only hidden door
        gpSides[1743].textureoffset = 8 * FRACUNIT;
        gpSides[1743].rowoffset = -29 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP11: Subspace
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Subspace() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Remove actions from neighboring lindefs that lower blue key platform that shouldn't have actions assigned to them
        gpLines[852].special = 0;
        gpLines[892].special = 0;
        gpLines[893].special = 0;
    }

    if (shouldApplyMapPatches_Visual()) {
        // Shift textures on secret doors to make them possible to discover by observation instead of by chance only
        gpSides[236].rowoffset += 16 * FRACUNIT;
        gpSides[240].rowoffset += 16 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP13: Vesperas
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Vesperas() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix door tracks on north door in starting room
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 16, 18);

        // Fix wall texture on hidden wall in NW room with hell knights
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 696);
        gpSides[gpLines[696].sidenum[0]].rowoffset = 31 * FRACUNIT;

        // Hide various linedefs from automap
        addFlagsToLinedefs(ML_DONTDRAW,
            // Hell Knight monster closet in blue key room
            1165, 1522, 1523, 1524,
            // Dummy sector in NE blood room
            1493, 1494, 1495,
            // Dummy sector in west hallway
            659, 660, 662,
            // Cacodemon monster closet in SW hallway
            1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1245, 1246
        );

        // Mark various linedefs as secret on automap
        addFlagsToLinedefs(ML_SECRET,
            // South monster closet in courtyard
            571,
            // Dummy sector in west hallway
            663,
            // Dummy sectors in NW hallway
            1155, 1159,
            // Dummy sector in west caco room
            1240,
            // Monster closet in NW room
            1269,
            // Dummy sector in SW hallway
            1387,
            // Dummy sectors in NW room
            1444, 1462, 1492,
            // Cacodemon monster closet in SW hallway
            280, 300, 452,
            // Dummy sector in west alcove of slime room
            596
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP14: System Control
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_SystemControl() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix door tracks
        addFlagsToLinedefs(ML_DONTPEGBOTTOM,
            // Door to outside
            346, 348,
            // Blue key door
            397, 398
        );

        // Align texture above outside door
        gpSides[483].rowoffset = -16 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP15: Human Barbeque
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HumanBarbeque() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix the track on a door inside the secret room near the exit moving when it shouldn't
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 315, 317);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP16: Wormhole
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Wormhole() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide 2 linedefs from automap in starting room to match those in the alternate version
        addFlagsToLinedefs(ML_DONTDRAW, 1425, 1426);

        // Change brightness of half of central elevator in starting room and alternate version to be the same on both halves
        gpSectors[185].lightlevel = 95;
        gpSectors[243].lightlevel = 95;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP18: Nukage Processing
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_NukageProcessing() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide multiplayer-only areas from computer map to match others of the same type
        addFlagsToLinedefs(ML_DONTDRAW, 
            // Blue Armor room
            1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079,
            // Nukage tank in north area
            1158, 1162, 1166, 1163, 1161, 1231, 1233, 1232, 1230, 1160, 1167, 1165, 1164);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix nukage tank in the east area of the map so it damages the player
        gpSectors[174].special = 7;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP19: Deepest Reaches
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DeepestReaches() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Change texture for secret door in blue door area to make it stand out (same as PC Doom)
        gpSides[786].toptexture = R_TextureNumForName("ROCK16");
        gpSides[792].toptexture = R_TextureNumForName("ROCK16");

        // Fix inside texture for secret door in blue door area
        removeFlagsFromLinedefs(ML_DONTPEGTOP, 1380, 1381);

        // Fix texture for megasphere door in secret area
        removeFlagsFromLinedefs(ML_DONTPEGTOP | ML_DONTPEGBOTTOM, 773, 776);
        gpSides[gpLines[776].sidenum[0]].rowoffset = 63 * FRACUNIT;

        // Unhide linedefs that shouldn't be hidden from automap
        removeFlagsFromLinedefs(ML_DONTDRAW, 706, 707, 1438, 1439, 1440, 1441);
    }
    if (shouldApplyMapPatches_GamePlay()) {
        // Move stuck imps in order to fix platform in the west-most room that lowers when picking up rocket launcher or BFG9000
        modifySectors(
            [](sector_t& sector) noexcept {
                for (mobj_t* pMobj = sector.thinglist; pMobj != nullptr;) {
                    mobj_t* const pNextMobj = pMobj->snext;

                    if ((pMobj->type == MT_TROOP) && (pMobj->x == -1704 * FRACUNIT)) {
                        pMobj->x -= 8 * FRACUNIT;
                    }
                    if ((pMobj->type == MT_TROOP) && (pMobj->x == -1776 * FRACUNIT)) {
                        pMobj->y += 16 * FRACUNIT;
                    }

                    pMobj = pNextMobj;
                }
            }, 143, 145
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP20: Processing Area
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ProcessingArea() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide linedef on east wall of south courtyard
        addFlagsToLinedefs(ML_DONTDRAW, 953);

        // Fix texture alignment on secret door of south courtyard
        removeFlagsFromLinedefs(ML_DONTPEGTOP, 202);

        // Prevent textures in north tunnel from moving when passage opens and realign
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 572, 664);
        gpSides[gpLines[572].sidenum[0]].rowoffset = -8 * FRACUNIT;
        gpSides[gpLines[664].sidenum[0]].textureoffset = -16 * FRACUNIT;
        gpSides[gpLines[664].sidenum[0]].rowoffset = -8 * FRACUNIT;  
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP21: Lunar Mining Project
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_LunarMiningProject() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix a missing texture on a small lip in the mines
        gpSides[gpLines[718].sidenum[1]].bottomtexture = R_TextureNumForName("ROCK06");

        // Mark linedef for south monster closet as secret and fix texture alignment
        addFlagsToLinedefs(ML_SECRET, 1);
        gpSides[gpLines[1].sidenum[0]].rowoffset = 0 * FRACUNIT;
        gpSectors[162].floorheight = 0 * FRACUNIT;

        // Hide courtyard teleport destination sectors
        addFlagsToLinedefs(ML_DONTDRAW, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983);

        // Change texture of co-op only closet door
        gpSides[1735].toptexture = R_TextureNumForName("BRONZE05");

        // Hide north monster closet
        addFlagsToLinedefs(ML_SECRET | ML_MIDMASKED, 919);
        addFlagsToLinedefs(ML_DONTDRAW, 1051, 1052);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Two monsters are in a closet that is only accessible in co-op; remove and subtract from total monsters count in single player
        if (gStartGameType == gt_single) {
            for (mobj_t* pMobj = gpSectors[237].thinglist; pMobj != nullptr; pMobj = pMobj->snext) {
                P_RemoveMobj(*pMobj);
                gTotalKills--;
            }
        }

        // Move one of the courtyard teleport destinations; it is in the wrong spot
        forAllThings(
            [](mobj_t& mobj) noexcept {
                const uint32_t sectorIdx = (uint32_t)(mobj.subsector->sector - gpSectors);

                if ((sectorIdx == 75) &&  (mobj.x == -536 * FRACUNIT)) {
                    P_RemoveMobj(mobj);
                }
            }
        );
        mobj_t* const pTeleDest = P_SpawnMobj(-592 * FRACUNIT, -1040 * FRACUNIT, INT32_MIN, MT_TELEPORTMAN);
        pTeleDest->angle = ANG45;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP22: Quarry
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Quarry() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Mark linedefs as secret for SW gun triggered secret
        addFlagsToLinedefs(ML_SECRET, 167, 191);

        // Unhide linedef in SE tunnel
        removeFlagsFromLinedefs(ML_DONTDRAW, 276);

        // Hide zero height sectors in eastern cavern
        addFlagsToLinedefs(ML_DONTDRAW, 846, 847, 849, 850, 851, 852);
        addFlagsToLinedefs(ML_SECRET, 716, 720, 726, 799);

        // Hide west elevator walk-over linedefs
        addFlagsToLinedefs(ML_DONTDRAW, 111, 112);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP23: Ballistyx
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Ballistyx() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Make various linedefs not render sky walls or be see through for occlusion purposes
        addVoidFlagToLinedefs(
            // Altar hole: don't draw sky walls
            1212, 1211, 1215, 1210, 1214, 1213,
            // Altar pillars: don't occlude and prevent geometry behind from rendering (needed for floating platform hack)
            1216, 1291, 1290, 1207, 1209, 1292, 1293, 1199,
            // Yellow key cage area: the outer wall floors sometimes don't render due to occluding sky walls; make them not occlude:
            1525, 1064, 1526, 1052, 371, 1053, 374, 1509, 373, 1054, 1524, 1055
        );
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix some monsters not teleporting in at the start: nudge the teleport destinations a little
        forAllThings(
            [](mobj_t& mobj) noexcept {
                const uint32_t sectorIdx = (uint32_t)(mobj.subsector->sector - gpSectors);

                if (sectorIdx == 167) {
                    mobj.y -= 24 * FRACUNIT;
                } else if ((sectorIdx == 150) || (sectorIdx == 163) || (sectorIdx == 164)) {
                    mobj.y += 24 * FRACUNIT;
                }
            }
        );

        // Fix door that is supposed to open when going up the first steps
        gpLines[637].special = 109;
        gpLines[637].tag = 18;
    }

    if (shouldApplyMapPatches_Visual()) {
        // Hide monster closet
        addFlagsToLinedefs(ML_DONTDRAW, 1098, 1101, 1102);
        addFlagsToLinedefs(ML_SECRET | ML_MIDMASKED | ML_DONTPEGTOP, 1100);
        gpSides[gpLines[1100].sidenum[0]].midtexture = R_TextureNumForName("SUPPORT3");
        gpSides[gpLines[1100].sidenum[0]].textureoffset = 8 * FRACUNIT;

        // Fix door tracks on soulsphere/lite amp goggles secret door
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 1479, 1480);

        // Remove upper unpegged flag and shift texture for 7 door in northmost room
        removeFlagsFromLinedefs(ML_DONTPEGTOP, 305, 306, 312, 316, 320, 324, 336);
        gpSides[gpLines[305].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[306].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[312].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[316].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[320].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[324].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[336].sidenum[0]].rowoffset = 32 * FRACUNIT;

        // Fix textures on doors for 2 south room closets
        removeFlagsFromLinedefs(ML_DONTPEGTOP, 863, 865, 873, 875);
        gpSides[gpLines[865].sidenum[1]].rowoffset = -48 * FRACUNIT;
        gpSides[gpLines[873].sidenum[1]].rowoffset = -48 * FRACUNIT;
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 862, 864, 872, 874);

        // Remove actions from door tracks for south room closet
        gpLines[862].special = 0;
        gpLines[864].special = 0;

        // Fix textures on red key platform
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 841, 842, 843, 844);

        // Fix door tracks on cage hallway
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 216, 219);
        gpSides[gpLines[215].sidenum[0]].textureoffset = 8 * FRACUNIT;
        gpSides[gpLines[216].sidenum[0]].textureoffset = 8 * FRACUNIT;
        gpSides[gpLines[216].sidenum[0]].rowoffset = -26 * FRACUNIT;
        gpSides[gpLines[219].sidenum[0]].rowoffset = -26 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP24: Heck
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Heck() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix texture on elevator in southwest room
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 550);
        gpSides[gpLines[550].sidenum[0]].textureoffset = 0 * FRACUNIT;
        gpSides[gpLines[550].sidenum[0]].rowoffset = 64 * FRACUNIT;

        // Hide linedefs in front of teleporters in starting area
        addFlagsToLinedefs(ML_DONTDRAW, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135);

        // Remove all chain hook with blood decorations; these were archviles carried over from PC Doom; most only appear in deathmatch
        forAllThings(
            [](mobj_t& mobj) noexcept {
                const uint32_t sectorIdx = (uint32_t)(mobj.subsector->sector - gpSectors);

                if (mobj.type == MT_MISC_BLOODHOOK) {
                    P_RemoveMobj(mobj);
                }
            }
        );

        // Restore false walls in marble maze
        addFlagsToLinedefs(ML_MIDMASKED, 419, 425, 429, 481);

        // Fix texture on doors that open when picking up the yellow key
        removeFlagsFromLinedefs(ML_DONTPEGTOP, 657, 658, 758, 760, 762, 764, 766);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP29: The Death Domain
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheDeathDomain() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix being able to close the blue door permanently from the opposite side of it.
        // Change the line special from 'DR Door Open Wait Close (fast)' to 'D1 Door (Blue) Open Stay'.
        gpLines[513].special = 32;

        // Fix a secret area containing Megaarmor armor being completely inaccessible.
        // Provide a new way to trigger it by running over a linedef, and also provide a way to get back out of it.
        gpLines[333].tag = 7;
        gpLines[333].special = 105;     // WR Door Open Wait Close (fast)

        modifyLinedefs(
            [](line_t& line) {
                line.special = 117;     // DR Door Open Wait Close (fast)
            },
            603, 604, 606
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the map patches for this game type
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatchArray_FinalDoom[] = {
    { 149065, 0xEB9558B4F55AB3B1, 0xEFF704814B4411D6, applyOriginalMapCommonPatches },      // MAP01
    { 117111, 0xADAA213B6086EE11, 0x978D2CC01B168F5D, applyOriginalMapCommonPatches },      // MAP02 (NTSC)
    { 117187, 0xAA3783F676D9B0CE, 0x9DE393E81F9FF509, applyOriginalMapCommonPatches },      // MAP02 (PAL, why different?)
    { 135628, 0xC80C9E89D5679E63, 0x7AD86A771493F84E, applyOriginalMapCommonPatches },      // MAP03
    {  88648, 0xCA130E217D79594D, 0x1241B18E3E7D8110, applyOriginalMapCommonPatches },      // MAP04
    { 132897, 0x2E8E3492E704154D, 0x0DE2F0470D62DDB8, applyOriginalMapCommonPatches },      // MAP05
    {  88004, 0xC12D7BD6D414250C, 0xF981017C0C8ADF20, applyOriginalMapCommonPatches },      // MAP06
    { 165920, 0x903B721BA84B1FFD, 0xCED86BF62E5CE0BE, patchMap_Geryon               },      // MAP07
    { 151747, 0x93EA3A4DE9DA978B, 0x3D27F6255CA0B9CC, patchMap_Minos                },      // MAP08
    { 102104, 0x1504DC20E04BE8F1, 0x3A63FD22BC9C0D8C, patchMap_Nessus               },      // MAP09
    { 139820, 0x5EDEF8B2A51779E8, 0x8D1314A4F889EFCC, patchMap_Paradox              },      // MAP10
    {  96211, 0x42B2A3CE9B37CA2A, 0x72D00C8E1681AEB4, patchMap_Subspace             },      // MAP11
    { 106776, 0xAD3AADE890018818, 0x7D70AC984E7211CC, applyOriginalMapCommonPatches },      // MAP12
    { 152855, 0xD4905C759C1713E1, 0x1B78CCD5275A40EB, patchMap_Vesperas             },      // MAP13
    {  54706, 0x979F686C4297312E, 0xB9EA33C07E20F4E3, patchMap_SystemControl        },      // MAP14
    {  77891, 0x20F93855131B2C1A, 0xD98E0D6C4EAEC765, patchMap_HumanBarbeque        },      // MAP15
    { 156972, 0xC4DF66BEDEE0E1C4, 0xFB56E82FA017FD9D, patchMap_Wormhole             },      // MAP16
    { 179622, 0x97DFE2C07BE92D3C, 0xEC29BA71305623B3, applyOriginalMapCommonPatches },      // MAP17
    { 131823, 0xADD51543E9578AB7, 0xA3E479551A015464, patchMap_NukageProcessing     },      // MAP18
    { 177868, 0x5BDC5BC7E62822C1, 0x3F374AD0091C79F1, patchMap_DeepestReaches       },      // MAP19
    { 105404, 0x5849A9F98647AF13, 0x59C891E67F19FC69, patchMap_ProcessingArea       },      // MAP20
    { 162561, 0x5BA4490CA5C13E9A, 0x23D505C31AF4CADF, patchMap_LunarMiningProject   },      // MAP21
    {  96826, 0x9B6446A94907229A, 0x6DC9F5EDDB9D4F2D, patchMap_Quarry               },      // MAP22
    { 167847, 0x3BC3E6570C2D06B3, 0x18756B0D2C98BE86, patchMap_Ballistyx            },      // MAP23
    { 121920, 0x445D7FDA25066B71, 0xAC3893B22E188D4D, patchMap_Heck                 },      // MAP24
    { 113719, 0xFBA63EF7487AB574, 0xE21B77623A0DE2AA, applyOriginalMapCommonPatches },      // MAP25
    { 127601, 0x1008C54A53E8B33E, 0x8E35C49173174DCD, applyOriginalMapCommonPatches },      // MAP26
    { 113829, 0x25A6925BB713C346, 0x7AF7C07603DEA325, applyOriginalMapCommonPatches },      // MAP27
    { 141807, 0x3461BD1E919965AB, 0x07C36C7B648205F6, applyOriginalMapCommonPatches },      // MAP28
    { 107736, 0xD9789CCEA024CCCC, 0x61CCB6C421B65C47, patchMap_TheDeathDomain       },      // MAP29 (NTSC)
    { 107736, 0x0599BE06504C6FAD, 0x1DCB1C8AD6410764, patchMap_TheDeathDomain       },      // MAP29 (PAL, why different?)
    { 110131, 0x2C157281E504283E, 0x914845A33B9F0503, applyOriginalMapCommonPatches },      // MAP30
};

const PatchList gPatches_FinalDoom = { gPatchArray_FinalDoom, C_ARRAY_SIZE(gPatchArray_FinalDoom) };

END_NAMESPACE(MapPatches)
