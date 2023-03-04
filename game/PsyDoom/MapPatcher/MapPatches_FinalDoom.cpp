//------------------------------------------------------------------------------------------------------------------------------------------
// A module containing map patches to apply to PlayStation Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatches.h"

#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Renderer/r_data.h"
#include "MapPatcherUtils.h"

using namespace MapPatcherUtils;

BEGIN_NAMESPACE(MapPatches)

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP07: Geryon
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Geryon() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix a door leading to the secret area with the super shotgun not having it's texture move as it lowers
        removeFlagsFromLines(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 966);
        gpSides[gpLines[966].sidenum[0]].rowoffset = 64 * FRACUNIT;

        // In the room immediately after the blue door, fix the wall textures so that they move when the wall does.
        // Also need to re-align the textures after making this change.
        removeFlagsFromLines(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 710, 712, 723, 726);
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
        addFlagsToLines(ML_DONTPEGBOTTOM,
            // Fix door tracks that shouldn't move for the small room with the Chainsaw
            727, 729,
            // Fix door tracks for the room containing the red key (beside the SSG) that move when they shouldn't
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
        // Fix the texture on the hidden elevator inside the castle walls not moving with the platform.
        // Also need to adjust the texture coordinates when using this fix.
        removeFlagsFromLines(ML_DONTPEGBOTTOM, 325);
        gpSides[gpLines[325].sidenum[0]].rowoffset = 65 * FRACUNIT;

        // Hide the opening to a monster closet at the northeast of the central area and adjust the texturing accordingly (to blend in)
        addFlagsToLines(ML_DONTDRAW, 552, 554, 555);
        addFlagsToLines(ML_SECRET | ML_MIDMASKED, 553);
        gpSides[gpLines[553].sidenum[0]].midtexture = R_TextureNumForName("ROCK03");
        gpSides[gpLines[553].sidenum[0]].textureoffset = -16 * FRACUNIT;

        // Fix the texture alignment on a deathmatch only hidden door (on the east side of the map)
        gpSides[gpLines[1251].sidenum[0]].textureoffset = 8 * FRACUNIT;
        gpSides[gpLines[1251].sidenum[0]].rowoffset = -29 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP11: Subspace
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Subspace() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Remove unintended actions from lines neighboring a switch which lowers the blue key platform.
        // These duplicate the same action of the actual switch line.
        removeLineActions(852, 892, 893);
    }

    if (shouldApplyMapPatches_Visual()) {
        // Shift textures on secret doors to make them possible to discover by observation instead of by chance only
        gpSides[gpLines[196].sidenum[0]].rowoffset += 8 * FRACUNIT;
        gpSides[gpLines[198].sidenum[0]].rowoffset += 8 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP13: Vesperas
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Vesperas() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix door tracks which shouldn't move for the north door in the starting room
        addFlagsToLines(ML_DONTPEGBOTTOM, 16, 18);

        // Fix the wall texture not moving when the wall lowers to reveal the closet with the red key (in the NW room with the Hell Knights)
        removeFlagsFromLines(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 696);
        gpSides[gpLines[696].sidenum[0]].rowoffset = 31 * FRACUNIT;

        // Hide various map lines which should not show
        hideLines(
            // Hell Knight monster closet in the blue key room
            1165, 1522, 1523, 1524,
            // Dummy sector in the northeast blood room (with the SSG)
            1493, 1494, 1495,
            // Dummy sector in the west hallway leading to the red key room
            659, 660, 662,
            // Cacodemon monster closet in the SW hallway (beside the Chainsaw)
            1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089, 1245, 1246
        );

        // Mark various lines as secret on automap so they render as solid lines on the map
        addFlagsToLines(ML_SECRET,
            // South monster closet in the courtyard
            571,
            // Dummy sector in the west hallway leading to the red key room
            663,
            // Dummy sectors in the NW hallway (next to the monster platforms that lower, which have health bonuses on them)
            1155, 1159,
            // Dummy sector in the west room with the Cacodemon (beside the Computer area map)
            1240,
            // Monster closet in the NW room with the red key
            1269,
            // Dummy sector in a SW hallway (near the Chainsaw)
            1387,
            // Dummy sectors in the NW room with the blood and SSG
            1444, 1462, 1492,
            // Cacodemon monster closet in SW hallway (near the Chainsaw)
            280, 300, 452,
            // Dummy sector in the west alcove of the slime room (near the Plasma Rifle)
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
        addFlagsToLines(ML_DONTPEGBOTTOM,
            // Door to outside
            346, 348,
            // Blue key door
            397, 398
        );

        // Align the texture above the outer door of the north outside area
        gpSides[gpLines[344].sidenum[0]].rowoffset = -16 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP15: Human Barbeque
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HumanBarbeque() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix the track on a door inside the secret room near the exit moving when it shouldn't
        addFlagsToLines(ML_DONTPEGBOTTOM, 315, 317);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP16: Wormhole
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Wormhole() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide 2 linedefs from the automap in the starting room to match those in the alternate version
        hideLines(1425, 1426);

        // Fix the brightness for half of the central weapon pedestal in the starting room and alternate version.
        // Should be the same brightness as the surrounding areas and also the other half of the pedestal.
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
        hideLines(
            // Closets attached to the blue Armor room
            1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079,
            // A nukage tank in the north area (near the exit)
            1158, 1162, 1166, 1163, 1161, 1231, 1233, 1232, 1230, 1160, 1167, 1165, 1164
        );
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix the nukage tank (containing the red key) in the east area of the map so that it damages the player
        gpSectors[174].special = 7;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP19: Deepest Reaches
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DeepestReaches() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Change the texture for the secret door leading to the BFG which appears only on Ultra Violence (in the area past the blue door).
        // Make it stand out more, similar to PC Doom.
        gpSides[gpLines[533].sidenum[0]].toptexture = R_TextureNumForName("ROCK16");
        gpSides[gpLines[538].sidenum[0]].toptexture = R_TextureNumForName("ROCK16");

        // Fix the inside texture for the secret door above not moving as the door opens
        removeFlagsFromLines(ML_DONTPEGTOP, 1380, 1381);

        // Fix the texture for the secret door leading to the Megasphere not moving when it is opened
        removeFlagsFromLines(ML_DONTPEGTOP | ML_DONTPEGBOTTOM, 773, 776);
        gpSides[gpLines[776].sidenum[0]].rowoffset = 63 * FRACUNIT;

        // Unhide map lines that shouldn't be hidden
        unhideLines(
            // Near the entrance to the secret area with the Megasphere
            706, 707,
            // In the cave/rock area with the yellow key
            1438, 1439, 1440, 1441
        );
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Move stuck imps in order to fix a platform which won't lower in the west-most (wooden) room.
        // The platform should lower when picking up the rocket launcher or BFG9000 in the nearby raised area, but does not because of the stuck imps.
        moveMobj(143, MT_TROOP, -1704, {}, -1712, {});
        moveMobj(145, MT_TROOP, -1776, {}, {}, -1048);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP20: Processing Area
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ProcessingArea() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide a map line on the east wall of the south courtyard which should not show
        hideLines(953);

        // Fix texture alignment on the door leading from the top-of-wall secret area in the south courtyard
        removeFlagsFromLines(ML_DONTPEGTOP, 202);

        // Prevent textures in a tunnel to the north (leading to the outdoor area with the rocket launcher) from moving when the passage opens.
        // Also realign the textures to correspond with this fix.
        addFlagsToLines(ML_DONTPEGBOTTOM, 572, 664);
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

        // South hallway monster closet: show a map line for the sound channel entrance as solid and fix the texture alignment
        addFlagsToLines(ML_SECRET, 1);
        gpSides[gpLines[1].sidenum[0]].rowoffset = 0 * FRACUNIT;
        gpSectors[162].floorheight = 0 * FRACUNIT;

        // Hide teleport destination sectors in the central courtyard
        hideLines(972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983);

        // Fix the texture of a co-op only closet door to make it blend in better with the surrounding environment.
        // This closet is in the room with the red key (north west side).
        gpSides[gpLines[1217].sidenum[0]].toptexture = R_TextureNumForName("BRONZE05");

        // Hide a monster closet on the north side of the map (beside the blue key)
        addFlagsToLines(ML_SECRET | ML_MIDMASKED, 919);
        hideLines(1051, 1052);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Two monsters are in a closet that is only accessible in co-op: remove them in single player and subtract from the kill count.
        // This closet is in the room with the red key, on the north west side of the room.
        if (gStartGameType == gt_single) {
            forAllMobjInSectors(
                [](mobj_t& mobj) noexcept {
                    if (mobj.flags & MF_COUNTKILL) {
                        P_RemoveMobj(mobj);
                        gTotalKills--;
                    }
                },
                237
            );
        }

        // Move one of the central courtyard teleport destinations (south west corner).
        // The destination marker is not on the teleport destination sector, so the teleport is broken.
        moveMobj(75, MT_TELEPORTMAN, -536, -1032, -592, -1040);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP22: Quarry
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Quarry() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix shootable lines which should render on the map as solid.
        // These lines are beside the SSG secret, at the south west corner of the map.
        addFlagsToLines(ML_SECRET, 167, 191);

        // Unhide a map line which should be shown in the SE tunnel (near the Chaingun)
        unhideLines(276);

        // Hide zero height sectors in the eastern lava cavern
        hideLines(846, 847, 849, 850, 851, 852);
        addFlagsToLines(ML_SECRET, 716, 720, 726, 799);

        // Hide west (outside) elevator walk-over lines
        hideLines(111, 112);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP23: Ballistyx
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Ballistyx() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Make various lines not render sky walls or be see through for occlusion purposes
        addVoidFlagToLines(
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
        forAllMobj(
            [](mobj_t& mobj) noexcept {
                const uint32_t sectorIdx = (uint32_t)(mobj.subsector->sector - gpSectors);

                if (sectorIdx == 167) {
                    mobj.y -= 24 * FRACUNIT;
                } else if ((sectorIdx == 150) || (sectorIdx == 164)) {
                    mobj.y += 24 * FRACUNIT;
                } else if (sectorIdx == 163) {
                    mobj.x -= 16 * FRACUNIT;
                }
            }
        );

        // Fix a door which is supposed to open when going up the first set of steps at the start of the map.
        // Add back the missing 'W1 Door Open Stay (fast)' action.
        gpLines[637].special = 109;
        gpLines[637].tag = 18;

        // Remove unintended line actions from door tracks for a monster closet in the south cave (near the blood pool and blue key).
        // This closet is in the south east corner of the cavern containing the blue key.
        removeLineActions(862, 864);
    }

    if (shouldApplyMapPatches_Visual()) {
        // Hide a monster closet (containing imps) beside the first set of stairs at the start of the map
        hideLines(1098, 1101, 1102);
        addFlagsToLines(ML_SECRET | ML_MIDMASKED | ML_DONTPEGTOP, 1100);
        gpSides[gpLines[1100].sidenum[0]].midtexture = R_TextureNumForName("SUPPORT3");
        gpSides[gpLines[1100].sidenum[0]].textureoffset = 8 * FRACUNIT;

        // Fix door tracks moving on the door for the secret closet containing a Soulsphere and Light amplification powerup.
        addFlagsToLines(ML_DONTPEGBOTTOM, 1479, 1480);

        // Fix door textures not moving upon opening for 7 doors in the circular brown sludge room.
        // Also adjust the texture coordinates accordingly for this fix.
        removeFlagsFromLines(ML_DONTPEGTOP, 305, 306, 312, 316, 320, 324, 336);
        gpSides[gpLines[305].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[306].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[312].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[316].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[320].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[324].sidenum[0]].rowoffset = 16 * FRACUNIT;
        gpSides[gpLines[336].sidenum[0]].rowoffset = 32 * FRACUNIT;

        // Fix the textures not moving on doors for 2 monster closets in the south cave (near the blood pool and blue key).
        // Also fix their door tracks moving when opening.
        removeFlagsFromLines(ML_DONTPEGTOP, 863, 865, 873, 875);
        gpSides[gpLines[865].sidenum[1]].rowoffset = -48 * FRACUNIT;
        gpSides[gpLines[873].sidenum[1]].rowoffset = -48 * FRACUNIT;
        addFlagsToLines(ML_DONTPEGBOTTOM, 862, 864, 872, 874);

        // Fix textures on the red key platform not moving as it lowers
        removeFlagsFromLines(ML_DONTPEGBOTTOM, 841, 842, 843, 844);

        // Fix door tracks moving for a trap door in the short hallway with the cages on both sides.
        // Also fix up the texture alignment for some neighboring lines.
        addFlagsToLines(ML_DONTPEGBOTTOM, 216, 219);
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
        // Fix the texture on the elevator in the southwest room not moving as it goes up and down.
        // Also adjust the texture alignment for this fix.
        removeFlagsFromLines(ML_DONTPEGBOTTOM, 550);
        gpSides[gpLines[550].sidenum[0]].textureoffset = 0 * FRACUNIT;
        gpSides[gpLines[550].sidenum[0]].rowoffset = 64 * FRACUNIT;

        // Hide lines in front of teleporters in the starting area
        hideLines(1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135);

        // Remove all chain hook with blood decorations.
        // These were Arch-viles carried over from PC Doom and all (except one) only appear in deathmatch.
        forAllMobj(
            [](mobj_t& mobj) noexcept {
                if (mobj.type == MT_MISC_BLOODHOOK) {
                    P_RemoveMobj(mobj);
                }
            }
        );

        // Fix textures not moving on monster closet doors which open after picking up the yellow key.
        // These doors are in the cavern directly to the east of the starting area.
        removeFlagsFromLines(ML_DONTPEGTOP, 657, 658, 758, 760, 762, 764, 766);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Restore false walls in the marble maze
        addFlagsToLines(ML_MIDMASKED, 419, 425, 429, 481);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP26: Aztec
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Aztec() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Close up dummy sectors in the glowing floor room and add the required upper textures for this change
        modifySectors(
            [](sector_t& sector) noexcept { sector.ceilingheight = sector.floorheight; },
            55, 60
        );

        modifyLines(
            [](line_t& line) noexcept { gpSides[line.sidenum[0]].toptexture = R_TextureNumForName("ROCK09"); },
            199, 1203, 194, 1206
        );

        // Close up dummy sectors in the large circular southeast room (with the Arachnotrons)
        modifySectors(
            [](sector_t& sector) noexcept { sector.ceilingheight = sector.floorheight; },
            119, 120, 131, 132, 140, 149, 153, 154
        );
    }

    if (shouldApplyMapPatches_Visual()) {
        // Fix textures not moving on the monster closet door in the glowing floor room
        removeFlagsFromLines(ML_DONTPEGTOP, 196, 197, 225, 226);

        // Fix the texture not moving when a teleport lowers on entering the large southeast room (the room with the Arachnotron bridge)
        removeFlagsFromLines(ML_DONTPEGBOTTOM, 1171);

        // Fix a door texture not moving (on open) for the southernmost trap door (beside the SSG, containing a Chaingunner)
        removeFlagsFromLines(ML_DONTPEGTOP, 478);
        gpSides[gpLines[478].sidenum[0]].rowoffset = -1 * FRACUNIT;

        // Fix door textures not moving (on open) for the northernmost trap doors (containing Hell Knights)
        removeFlagsFromLines(ML_DONTPEGTOP, 715, 922);
        gpSides[gpLines[715].sidenum[0]].rowoffset = 20 * FRACUNIT;
        gpSides[gpLines[922].sidenum[0]].rowoffset = 20 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP27: Ghost Town
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_GhostTown() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix door textures not moving on various doors as they open
        removeFlagsFromLines(ML_DONTPEGTOP,
            // The blue key room monster closet door
            595,
            // Southeast arena monster closet doors (on the way towards the blue key)
            502, 503, 504, 505, 506, 507
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP28: Baron's Lair
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BaronsLair() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Shift floor and ceiling flats for the exit portal; they are misaligned.
        gpSectors[124].ceilTexOffsetX = 32 * FRACUNIT;
        gpSectors[124].floorTexOffsetX = 32 * FRACUNIT;
        
        // Shift ceiling flats for lights in the northmost room; they are misaligned.
        gpSectors[273].ceilTexOffsetY = 32 * FRACUNIT;
        gpSectors[278].ceilTexOffsetY = 32 * FRACUNIT;

        // Increase the light level for deathmatch start closets
        modifySectors(
            [](sector_t& sector) noexcept { sector.lightlevel = 32; },
            13, 14, 42, 103, 220, 222, 251, 254
        );

        // Align switch textures in the deathmatch start closets
        modifyLines(
            [](line_t& line) noexcept {
                gpSides[line.sidenum[0]].rowoffset = 0;
            },
            579, 591, 585, 619, 622, 624, 1214, 1226
        );

        // Hide deathmatch start closets from the map
        addFlagsToLines(ML_SECRET,
            1225,       // North east closet
            1215,       // South east closet
            620,        // South west closet (eastmost)
            621,        // South west closet (beside eastmost)
            623,        // South west closet (northmost)
            562,        // North west closet (eastmost)
            569,        // North west closet (beside eastmost)
            572         // North west closet (eastmost)
        );

        hideLines(
            // North east closet
            1219, 1220, 1221, 1222, 1223, 1226,
            // South east closet
            1208, 1209, 1210, 1211, 1212, 1214,
            // South west closet (eastmost)
            604, 605, 616, 617, 618, 619,
            // South west closet (beside eastmost)
            601, 602, 612, 613, 614, 622,
            // South west closet (northmost)
            598, 599, 608, 609, 610, 624,
            // North west closet (eastmost)
            586, 587, 588, 589, 590, 591,
            // North west closet (beside eastmost)
            574, 575, 576, 577, 578, 579,
            // North west closet (eastmost)
            580, 581, 582, 583, 584, 585
        );

        // From the map, hide pillars in the starting area which lower in deathmatch.
        // Also fix their textures not moving as they lower and adjust coordinates accordingly.
        hideLines(
            704,    // South (west side)
            705,    // South (east side)
            706,    // West (south side)
            707,    // West (north side)
            708,    // North (west side)
            709     // North (east side)
        );

        modifyLines(
            [](line_t& line) noexcept {
                line.flags &= ~ML_DONTPEGTOP;
                line.flags |= ML_SECRET;
            },
            0, 6,       // North (west side)
            9, 15,      // North (east side)
            20, 26,     // West (north side)
            29, 35,     // West (south side)
            74, 76,     // South (west side)
            77, 79      // South (east side)
        );

        modifyLines(
            [](line_t& line) noexcept {
                gpSides[line.sidenum[0]].rowoffset = -24 * FRACUNIT;
                line.flags &= ~ML_DONTPEGTOP;
                line.flags |= ML_SECRET;
            },
            7, 483,     // North (west side)
            16, 482,    // North (east side)
            27, 489,    // West (north side)
            38, 488,    // West (south side)
            75, 487,    // South (west side)
            78, 486     // South (east side)
        );

        // Fix door tracks on some of the deathmatch only doors moving when they open
        addFlagsToLines(ML_DONTPEGBOTTOM, 580, 584, 574, 578, 586, 590);

        // Hide monster closet tunnels connected to Imp pedestals beside the northmost room
        addFlagsToLines(ML_SECRET, 295, 1097);
        hideLines(1089, 1090, 1091, 1098, 1099, 1100);
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
        // Provide a new way to trigger it by running over a line, and also provide a way to get back out of it.
        gpLines[333].tag = 7;
        gpLines[333].special = 105;     // WR Door Open Wait Close (fast)

        modifyLines(
            [](line_t& line) noexcept {
                line.special = 117;     // DR Door Open Wait Close (fast)
            },
            603, 604, 606
        );

        // Fix the railing next to the stairs in the northeast room (with invisibility power up) being walk-through
        addFlagsToLines(ML_BLOCKING, 691, 692, 693, 694, 695);

        // The blue key (in the sludge pit near the start) is virutally impossible to get to without strafe jumping from the platform above.
        // To fix, make the barrier to it's trench lower along with the barrier on the platform above.
        // Also add an additional texture to cover the new wall which will be exposed, and adjust the barrier texture so it moves as the barrier lowers.
        gpSectors[39].tag = 27;
        gpSides[gpLines[80].sidenum[1]].bottomtexture = R_TextureNumForName("BRICK12");
        removeFlagsFromLines(ML_DONTPEGBOTTOM, 80);
    }

    if (shouldApplyMapPatches_Visual()) {
        // Prevent the railing texture around the invisibility sphere from repeating vertically in the northeast room
        addFlagsToLines(ML_MID_FIXED_HEIGHT, 916, 917, 918);

        // Hide a map line which should be hidden at the entrance to the small (caged) square containing the invisiblity sphere
        hideLines(935);

        // Hide map lines which shouldn't show for the east room window skybox (beside a row of Lost Souls)
        hideLines(833, 834, 835, 836, 837, 838);

        // Fix the alignment for the switch texture on the south hallway barrier (on the way to the platform with the blue key jump)
        gpSides[gpLines[911].sidenum[0]].textureoffset = -16 * FRACUNIT;
        gpSides[gpLines[911].sidenum[0]].rowoffset = 0 * FRACUNIT;

        // Realign the texture above and below the inside of the northeast courtyard window
        addFlagsToLines(ML_DONTPEGTOP, 456);
        gpSides[gpLines[456].sidenum[0]].rowoffset = 57 * FRACUNIT;

        // Fix the texture not moving on a hidden elevator close to exit (closet containing rockets, backpack and clips).
        // Also adjust the texture alignment for this fix.
        removeFlagsFromLines(ML_DONTPEGBOTTOM, 805);
        gpSides[gpLines[805].sidenum[0]].rowoffset = 64 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP30: Onslaught
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Onslaught() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix door tracks moving on the northeast monster closet containing Cacodemons (in the start room)
        addFlagsToLines(ML_DONTPEGBOTTOM, 535, 539);
        
        // Fix door textures not moving on various monster closets in the northeast/starting room
        removeFlagsFromLines(ML_DONTPEGTOP, 796);
        
        // Fix hidden platform textures not moving in a small lowered area in the central outdoor room
        removeFlagsFromLines(ML_DONTPEGBOTTOM, 108, 121);

        // Fix textures not moving on doors which open when walking into the Cyberdemon room
        modifyLines(
            [](line_t& line) noexcept {
                gpSides[line.sidenum[0]].rowoffset = -8 * FRACUNIT;
                line.flags &= ~(ML_DONTPEGTOP | ML_DONTPEGBOTTOM);
            },
            0, 1, 2, 3, 4, 5
        );

        // Fix door tracks moving for the doors which open when walking into the Cyberdemon room
        addFlagsToLines(ML_DONTPEGBOTTOM, 32, 40, 42, 48);

        // Hide unused areas in the Cyberdemon room
        addFlagsToLines(ML_MIDMASKED | ML_SECRET, 29, 41);
        addFlagsToLines(ML_DONTDRAW, 30, 33, 36, 37, 38, 79, 80, 81, 183, 544, 545, 546);

        // Fix door textures not moving on the south monster closet in the final area.
        // Also fix the door tracks moving when the door opens.
        removeFlagsFromLines(ML_DONTPEGTOP, 686, 687);
        addFlagsToLines(ML_DONTPEGBOTTOM, 753, 757);

        // Fix door textures not moving on doors inside the red key room (never seen moving on single player).
        // Also fix some door tracks moving in some cases.
        removeFlagsFromLines(ML_DONTPEGTOP, 707, 786, 787);
        removeFlagsFromLines(ML_DONTPEGTOP | ML_DONTPEGBOTTOM, 668, 696);
        gpSides[gpLines[668].sidenum[0]].rowoffset = -58 * FRACUNIT;
        gpSides[gpLines[696].sidenum[0]].rowoffset = 45 * FRACUNIT;
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Restore a secret on the sector containing the Mega-armor (to match PC behavior)
        gpSectors[41].special = 9;
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
    { 127601, 0x1008C54A53E8B33E, 0x8E35C49173174DCD, patchMap_Aztec                },      // MAP26
    { 113829, 0x25A6925BB713C346, 0x7AF7C07603DEA325, patchMap_GhostTown            },      // MAP27
    { 141807, 0x3461BD1E919965AB, 0x07C36C7B648205F6, patchMap_BaronsLair           },      // MAP28
    { 107736, 0xD9789CCEA024CCCC, 0x61CCB6C421B65C47, patchMap_TheDeathDomain       },      // MAP29 (NTSC)
    { 107736, 0x0599BE06504C6FAD, 0x1DCB1C8AD6410764, patchMap_TheDeathDomain       },      // MAP29 (PAL, why different?)
    { 110131, 0x2C157281E504283E, 0x914845A33B9F0503, patchMap_Onslaught            },      // MAP30
};

const PatchList gPatches_FinalDoom = { gPatchArray_FinalDoom, C_ARRAY_SIZE(gPatchArray_FinalDoom) };

END_NAMESPACE(MapPatches)
