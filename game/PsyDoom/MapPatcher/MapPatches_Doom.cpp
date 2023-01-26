//------------------------------------------------------------------------------------------------------------------------------------------
// A module containing map patches to apply to the original PlayStation Doom
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatches.h"

#include "Doom/Base/sounds.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_telept.h"
#include "Doom/Renderer/r_data.h"

BEGIN_NAMESPACE(MapPatches)

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP01: Hangar
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Hangar() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide linedefs in central courtyard
        addFlagsToLinedefs(ML_DONTDRAW, 234, 236, 237);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP02: Plant
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Plant() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide linedefs in east courtyard
        addFlagsToLinedefs(ML_DONTDRAW, 935, 936, 937);

        // Replace texture in opening behind red key
        gpSides[gpLines[943].sidenum[0]].midtexture = R_TextureNumForName("LITE01");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP03: Toxin Refinery
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ToxinRefinery() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide linedef in secret exit hallway
        addFlagsToLinedefs(ML_DONTDRAW, 646);

        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            // Blue key room
            895, 896, 897
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP04: Command Control
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_CommandControl() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        removeFlagsFromLinedefs(
            ML_DONTPEGBOTTOM,
            // Fix bugs where step textures appear black
            337, 746, 747, 748,
            // Fix the texture on the bridge near the exit not moving as it is raised:
            433, 434
        );

        // This step needs a texture coordinate adjustment
        gpSides[253].rowoffset = 0;
        gpSides[253].rowoffset.snap();

        // Hide linedefs
        addFlagsToLinedefs(ML_DONTDRAW,
            // East central hallway
            692,
            // Maze
            228
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP05: Phobos Lab
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_PhobosLab() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide linedefs in east window
        addFlagsToLinedefs(ML_DONTDRAW, 503, 504);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP06: Central Processing
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_CentralProcessing() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix a door track for the southern red door unintentionally moving with the door
        gpLines[964].flags |= ML_DONTPEGBOTTOM;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP09: Deimos Anomaly
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DeimosAnomaly() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Add missing textures to back side of wall that lowers around final teleport
        modifyLinedefs(
            [](line_t& line) { gpSides[line.sidenum[1]].bottomtexture = R_TextureNumForName("BRONZE01"); },
            243, 244
        );

        // Move secret flags to outer wall of final teleport to better match the PC version
        removeFlagsFromLinedefs(ML_SECRET, 243, 244);
        addFlagsToLinedefs(ML_SECRET, 356, 357);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP10: Containment Area
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ContainmentArea() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            // Blue doors hallway
            357, 504,
            // South central hallway
            683, 684, 703, 704,
            // Exit room
            685
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP11: Refinery
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Refinery() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            69, 70, 217, 366, 367, 451, 495, 496
        );
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Restored secret to stairs leading from southwest lava room; same as PC Doom.
        gpSectors[88].special = 9;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP12: Deimos Lab
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DeimosLab() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            187, 200, 267, 641, 554, 555, 556, 622, 100, 130
        );

        // Add missing texture to back side of linedef on platform that raises up when you step off of it
        gpSides[gpLines[297].sidenum[1]].bottomtexture = R_TextureNumForName("METAL01");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP13: Command Center
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_CommandCenter() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a trap pillar containing a Baron not lowering.
        // The pillar doesn't lower because the Baron is stuck in the ceiling - fix by raising the ceiling.
        gpSectors[71].ceilingheight = 232 * FRACUNIT;
    }

    if (shouldApplyMapPatches_Visual()) {
        // Remove unused action from linedef. This was left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        gpLines[383].special = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP14: Halls of the Damned
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HallsOfTheDamned() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix stuck demons and baron of hell in south monster closet in central hallway on harder skills
        if (gGameSkill >= sk_hard) {
            // Monster closet needs to be slightly lengthened to make room for all monsters
            gpVertexes[352].y -= 16 * FRACUNIT;
            gpVertexes[353].y -= 16 * FRACUNIT;
            
            modifySectors(
                [](sector_t& sector) noexcept {
                    for (mobj_t* pMobj = sector.thinglist; pMobj != nullptr;) {
                        mobj_t* const pNextMobj = pMobj->snext;

                        if (pMobj->type == MT_BRUISER) {
                            EV_TeleportTo(*pMobj, pMobj->x, -248 * FRACUNIT, pMobj->angle, false, false, (mobjtype_t) 0, sfx_None);
                        }

                        if ((pMobj->type == MT_SERGEANT) && (pMobj->x == 992 * FRACUNIT) && (pMobj->y == -128 * FRACUNIT)) {
                            // This will only move one of the two demons since the other one will fail to teleport to the same spot
                            EV_TeleportTo(*pMobj, pMobj->x, -192 * FRACUNIT, pMobj->angle, false, false, (mobjtype_t)0, sfx_None);
                        }

                        if ((pMobj->type == MT_SERGEANT) && (pMobj->x == 1056 * FRACUNIT) && (pMobj->y == -128 * FRACUNIT)) {
                            // This will only move one of the two demons since the other one will fail to teleport to the same spot
                            EV_TeleportTo(*pMobj, pMobj->x, -192 * FRACUNIT, pMobj->angle, false, false, (mobjtype_t)0, sfx_None);
                        }

                        pMobj = pNextMobj;
                    }
                }, 88
            );
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP15: Spawning Vats
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_SpawningVats() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide skyboxes for northwest hallway windows
        addFlagsToLinedefs(ML_DONTDRAW,
            624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 640, 641, 642, 643, 644, 645, 646, 647, 648, 652
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP16: Hell Gate
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HellGate() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide skybox for window
        addFlagsToLinedefs(ML_DONTDRAW, 27, 28, 29, 30, 31, 32, 33, 34, 35, 42);

        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            72, 87
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP17: Hell Keep
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HellKeep() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Unhide linedefs that shouldn't be hidden
        removeFlagsFromLinedefs(ML_DONTDRAW, 41, 42, 77, 78, 83, 84);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP18: Pandemonium
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Pandemonium() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix the texture of pillars to the north of the start point not moving as they are lowered
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 529, 531, 533, 534, 540, 541);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP19: House Of Pain
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HouseOfPain() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a bug where an unintended door linedef causes one of the ceilings to collapse permanently.
        // Remove the line action that causes the bug.
        gpLines[435].special = 0;
    }

    if (shouldApplyMapPatches_Visual()) {
        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        gpLines[88].special = 0;
        gpLines[450].special = 0;

        // Hide linedefs
        addFlagsToLinedefs(ML_DONTDRAW, 90, 91, 518, 519, 520, 524, 525, 526, 527);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP20: Unholy Cathedral
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_UnholyCathedral() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a secret door not opening because it requires the player to walk across it's line.
        // The door should be the yellow key also. Only apply this patch if not playing a classic demo because it affects gameplay!
        gpLines[505].special = 34;

        // Fix various one-sided lines being incorrectly tagged as 'door' lines.
        // In the original game activating these lines could cause a crash.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            // Fix 2 one-sided lines by the north-east teleporter being incorrectly tagged as 'door' lines
            499, 501,
            // Fix another instance of one-sided lines being marked as 'door' lines (the track of the door leading to westernmost room)
            217, 220
        );
    }

    if (shouldApplyMapPatches_Visual()) {
        // For the secret door bug above make sure the floor texture matches the outside floor texture (like on PC)
        modifySectors(
            [](sector_t& sector){ sector.floorpic = gpSectors[30].floorpic; },
            164, 165
        );

        // Fix a missing upper wall texture when the ceiling closes in the trap room with the BFG
        modifyLinedefs(
            [](line_t& line) {
                gpSides[line.sidenum[0]].toptexture = R_TextureNumForName("MARBLE04");
                gpSides[line.sidenum[1]].toptexture = R_TextureNumForName("MARBLE04");
            },
            950, 1078
        );

        // Fix the alignment of the scrolling skull wall textures
        auto setLineFrontTexOffset = [](const int32_t lineNum, const int32_t offsetX, const int32_t offsetY) noexcept {
            side_t& side = gpSides[gpLines[lineNum].sidenum[0]];
            side.textureoffset.snapToValue(offsetX * FRACUNIT);
            side.rowoffset.snapToValue(offsetY * FRACUNIT);
        };

        setLineFrontTexOffset(1113, 32, -48);   // Top skulls
        setLineFrontTexOffset(1114,  0, -48);
        setLineFrontTexOffset(1115, 32, -48);

        setLineFrontTexOffset(1118, 32,   0);   // Middle skulls
        setLineFrontTexOffset(1119,  0,  48);
        setLineFrontTexOffset(1120, 32,   0);

        setLineFrontTexOffset(1123, 32, -48);   // Bottom Skulls
        setLineFrontTexOffset(1125, 32, -48);

        // Fix the track of some doorways moving when it should not
        removeFlagsFromLinedefs(
            ML_DONTPEGBOTTOM,
            // Doorway to west green slime area
            733, 736,
            // Doorway to northmost room
            815, 818
        );

        // Tweak the vertical alignment of the doorway track to the northmost room (adjustments following the 'lower unpegged' fix)
        modifyLinedefs(
            [](line_t& line) { gpSides[line.sidenum[0]].rowoffset = 16 * FRACUNIT; },
            815, 818
        );

        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            426, 431, 538, 550, 819, 820, 1068
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP21: Mt. Erebus
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_MtErebus() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            48, 66, 246, 247, 248, 293, 325, 333
        );

        // Unhide linedefs that shouldn't be hidden
        removeFlagsFromLinedefs(ML_DONTDRAW, 385, 396, 407, 410, 443, 516);

        // Fix texture alignment for doors in Y-shaped building
        gpSides[gpLines[403].sidenum[0]].textureoffset = -16 * FRACUNIT;
        gpSides[gpLines[404].sidenum[0]].textureoffset = -16 * FRACUNIT;
        gpSides[gpLines[405].sidenum[0]].textureoffset = -7 * FRACUNIT;
        gpSides[gpLines[406].sidenum[0]].textureoffset = -11 * FRACUNIT;

        // Fix Texture alignment for platform that lowers in Y-shaped building
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 408, 409);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP22: Limbo
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Limbo() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Fix a bug where a single step/lower-wall is not rendered.
        // This texture was not assigned in the original map, but was defaulted to texture index '0' by the original PSX Doom code in 'P_Init'.
        // PsyDoom now allows walls to deliberately have no texture assigned so they can be invisible, but that change causes a bug with this
        // step (beside the lift) not appearing in the room with the lift and the Baron. Fix by replicating the original PSX behavior.
        gpSides[967].bottomtexture = 0;
    }

    if (shouldApplyMapPatches_Visual()) {
        // Remove unused actions from linedefs. These were left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            235, 236, 237
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP24: Hell Beneath
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HellBeneath() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a secret in the room with the red door being almost impossible to trigger.
        // Shift the secret onto a neighboring blood sector which is much bigger.
        gpSectors[26].special = 9;
        gpSectors[27].special = 0;

        // Add hidden switch to escape blue key alcove if you get trapped
        gpLines[294].special = 60;
        gpLines[294].tag = 8;
    }

    if (shouldApplyMapPatches_Visual()) {
        // Flag linedefs as secret in blue key room
        addFlagsToLinedefs(ML_SECRET, 263, 266);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP25: Perfect Hatred
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_PerfectHatred() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix unpegged textures on hidden teleport
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 174);

        // Remove unused action from linedef. This was left over from PC Doom, but no longer used.
        // Note: This does not affect gameplay. This is only used to change how linedefs are displayed on the automap.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            117
        );
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Move tall green firestick that's stuck in the wall
        modifySectors(
            [](sector_t& sector) noexcept {
                for (mobj_t* pMobj = sector.thinglist; pMobj != nullptr;) {
                    mobj_t* const pNextMobj = pMobj->snext;

                    if ((pMobj->type == MT_MISC42) && (pMobj->y == 1712 * FRACUNIT)) {
                        EV_TeleportTo(*pMobj, -304 * FRACUNIT, pMobj->y, pMobj->angle, false, false, (mobjtype_t)0, sfx_None);
                    }

                    pMobj = pNextMobj;
                }
            }, 98
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP27: Unruly Evil
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_UnrulyEvil() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Move vertexes temporarily to allow moving cacodemons
        gpVertexes[479].y += 8 * FRACUNIT;
        gpVertexes[480].y += 8 * FRACUNIT;
        gpVertexes[472].y += 8 * FRACUNIT;
        gpVertexes[473].y += 8 * FRACUNIT;

        // Fix monsters that are stuck in north-most monster closets
        modifySectors(
            [](sector_t& sector) noexcept {
                for (mobj_t* pMobj = sector.thinglist; pMobj != nullptr;) {
                    mobj_t* const pNextMobj = pMobj->snext;

                    // Move cacodemons on hard
                    if ((pMobj->type == MT_HEAD) && (pMobj->y == -96 * FRACUNIT)) {
                        EV_TeleportTo(*pMobj, pMobj->x, -80 * FRACUNIT, pMobj->angle, false, false, (mobjtype_t)0, sfx_None);
                    }

                    // Move spectres on easy
                    if (pMobj->type == MT_SERGEANT) {
                        EV_TeleportTo(*pMobj, pMobj->x, -144 * FRACUNIT, pMobj->angle, false, false, (mobjtype_t)0, sfx_None);
                    }

                    pMobj = pNextMobj;
                }
            }, 89, 95
        );

        // Move vertexes back to original locations
        gpVertexes[479].y -= 8 * FRACUNIT;
        gpVertexes[480].y -= 8 * FRACUNIT;
        gpVertexes[472].y -= 8 * FRACUNIT;
        gpVertexes[473].y -= 8 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP28: Unto the Cruel
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_UntoTheCruel() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Unhide linedefs that don't need to be hidden
        removeFlagsFromLinedefs(ML_DONTDRAW, 
            // Bridge area (except bridge itself)
            581, 582, 583, 585, 601, 602,
            // Final area stairs
            632, 633, 634, 640, 641, 642, 653, 654, 678, 687, 688, 695, 696, 697, 707, 706,
            // Final area walkways
            643, 644, 676, 677,
            // Final area windows
            709, 710, 711, 712,
            // Exit teleporter
            726, 727, 728, 729
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP29: Twilight Descends
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TwilightDescends() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Unhide linedefs that don't need to be hidden
        removeFlagsFromLinedefs(ML_DONTDRAW,
            // Secret doorway in north hallway
            853, 871,
            // South tunnels
            297, 304, 302, 316,
            // West stairs
            69, 71, 73, 75, 77, 79, 81,
            // West building doorway
            98, 106
        );

        // Hide linedefs that should be hidden
        addFlagsToLinedefs(ML_DONTDRAW,
            // North tunnel (hide remaining structures to match the rest that are already hidden)
            502, 503, 504, 505, 506, 507, 508, 511, 515, 516, 520, 523, 634, 644, 645, 813, 814,
            815, 816, 817, 818, 819, 820, 823, 824, 825, 826, 828, 829, 830, 1237, 1239
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP31: Entryway
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Entryway() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Fix grass beyond the sky wall sometimes appearing - don't render it!
        modifySectors(
            [](sector_t& sector) { sector.floorpic = -1; },
            3, 25
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP32: Underhalls
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Underhalls() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix red door bars; all sides should be the same special and tag
        modifyLinedefs(
            [](line_t& line) {
                line.special = 135;
                line.tag = 7;
            },
            111, 112, 113, 511, 513, 514, 516, 517, 518, 520, 521, 522
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP34: The Focus
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheFocus() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        removeFlagsFromLinedefs(
            ML_DONTPEGBOTTOM,
            // Fix the texture on the bridge near the exit not moving as it is raised:
            511, 515
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP35: The Waste Tunnels
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheWasteTunnels() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix the texture on the secret elevator in the first area
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 329);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Move one of two lost souls that are stuck togther on the hard skill level in the eastmost passage
        modifySectors(
            [](sector_t& sector) noexcept {
                for (mobj_t* pMobj = sector.thinglist; pMobj != nullptr;) {
                    mobj_t* const pNextMobj = pMobj->snext;

                    if ((pMobj->type == MT_SKULL) && (pMobj->x == 2848 * FRACUNIT) && (pMobj->y == -400 * FRACUNIT)) {
                        // This will only move one of the two lost souls since the other one will fail to teleport to the same spot
                        EV_TeleportTo(*pMobj, pMobj->x, -320 * FRACUNIT, pMobj->angle, false, false, (mobjtype_t)0, sfx_None);
                    }

                    pMobj = pNextMobj;
                }
            }, 65
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP36: The Crusher
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCrusher() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix one-sided lines being incorrectly tagged as 'door' lines.
        // In the original game activating these lines could cause a crash.
        modifyLinedefs(
            [](line_t& line) { line.special = 0; },
            // Fix 2 one-sided lines on the red door being incorrectly tagged as 'door' lines
            24, 25
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP37: Dead Simple
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DeadSimple() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide deathmatch only door
        addFlagsToLinedefs(ML_SECRET, 168);

        // Fix texture on deathmatch only door
        removeFlagsFromLinedefs(ML_DONTPEGTOP, 168);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP38: Tricks And Traps
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TricksAndTraps() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix the texture on the two gun activated doors
        removeFlagsFromLinedefs(ML_DONTPEGTOP, 281, 282, 286, 287);

        // Flag corner wall in southeast room as secret
        addFlagsToLinedefs(ML_SECRET, 263, 645, 646);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP39: The Pit
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ThePit() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide dummy sector on east elevator
        addFlagsToLinedefs(ML_SECRET, 9, 71);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP40: Refueling Base
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_RefuelingBase() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix the texture on the hidden door on the northeast room
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 656);
        modifyLinedefs(
            [](line_t& line) { gpSides[line.sidenum[0]].rowoffset = -16 * FRACUNIT; },
            656
        );

        // Hide monsters only teleport linedefs in exit area
        addFlagsToLinedefs(ML_DONTDRAW, 894, 895, 897, 1060);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP41: O of Destruction!
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_OofDestruction() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix dummy sector on south wall
        addFlagsToLinedefs(ML_SECRET, 374, 375, 376, 377, 378, 793);
        addFlagsToLinedefs(ML_DONTDRAW, 788, 789, 790, 791, 792, 794);
        gpSectors[83].ceilingheight = 136 * FRACUNIT;
        gpSectors[83].ceilingpic = R_FlatNumForName("BRN14");
        modifyLinedefs(
            [](line_t& line) { gpSides[line.sidenum[0]].toptexture = R_TextureNumForName("ROCK06"); },
            374, 375, 376, 377, 378
        );
        gpSides[gpLines[793].sidenum[0]].toptexture = R_TextureNumForName("ROCK10");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP42: The Factory
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheFactory() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Remove the 'hidden' flag from linedefs that shouldn't be hidden on the automap
        removeFlagsFromLinedefs(ML_DONTDRAW, 260, 540, 547, 548, 549);

        // Fix door tracks on imp building door
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 113, 114);

        // Hide linedefs around perimeter of map
        addFlagsToLinedefs(ML_DONTDRAW, 63, 64, 65, 66);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Remove tag from teleporter sector in the south building that causing the ceiling to
        // raise when activating a switch intended for the door in the super shotgun room
        gpSectors[63].tag = 0;

        // Remove unneeded actions from walls next to switch in slime floor room
        gpLines[520].special = 0;
        gpLines[775].special = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP44: Suburbs
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Suburbs() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix some textures by the secret exit door moving when they should not
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 573, 781, 782);

        // Fix the texture of the elevator inside the secret area with the BFG not moving properly
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 598);

        // Hide linedefs in west and southwest perimeter
        addFlagsToLinedefs(ML_DONTDRAW, 537, 538, 539);

        // Change floor texture in southwest monster closet; you can see through the opening in the wall
        gpSectors[14].floorpic = R_FlatNumForName("SLIME01");

        // Change ceiling texture in southwest slime pit inlets; you can see when you die
        gpSectors[12].ceilingpic = R_FlatNumForName("ROK02");
        gpSectors[17].ceilingpic = R_FlatNumForName("ROK02");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP45: Tenements
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Tenements() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a secret near the end being very easy to skip over.
        // Shift the secret to a neighboring sector which the player is almost guaranteed to go through to reach the Megasphere.
        gpSectors[110].special = 9;
        gpSectors[112].special = 0;

        // Fix some Imps on 'Ultra Violence' (and also 'Nightmare') being stuck in a wall and un-killable.
        // Move them to the right so they are not stuck.
        modifySectors(
            [](sector_t& sector) noexcept {
                for (mobj_t* pMobj = sector.thinglist; pMobj != nullptr;) {
                    mobj_t* const pNextMobj = pMobj->snext;

                    if ((pMobj->type == MT_TROOP) && (pMobj->x == -2272 * FRACUNIT)) {
                        EV_TeleportTo(*pMobj, -2160 * FRACUNIT, pMobj->y, pMobj->angle, false, false, (mobjtype_t) 0, sfx_None);
                    }

                    pMobj = pNextMobj;
                }
            },
            179, 182, 183, 184
        );
    }

    if (shouldApplyMapPatches_Visual()) {
        // Fix the track of some doorways moving when it should not
        addFlagsToLinedefs(ML_DONTPEGBOTTOM,
            // First room, door on the left
            128, 130,
            // Blood floor room, door on the right
            275, 276
        );

        // Unhide linedefs that shouldn't be hidden
        removeFlagsFromLinedefs(ML_DONTDRAW, 1128, 1129);

        // Fix door textures
        removeFlagsFromLinedefs(ML_DONTPEGTOP,
            // Top of the first stairs on the right
            159, 160,
            // Inside of right shutter at start
            120,
            // Shutters in east room
            391, 441, 1109
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP46: The Courtyard
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCourtyard() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Fix a wall top/floor sometimes not appearing: this wall is at the starting area and borders the sky
        addVoidFlagToLinedefs(45, 17, 18, 51);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Move stuck spectre in west passage on easier skills
        modifySectors(
            [](sector_t& sector) noexcept {
                for (mobj_t* pMobj = sector.thinglist; pMobj != nullptr;) {
                    mobj_t* const pNextMobj = pMobj->snext;

                    if ((pMobj->type == MT_SERGEANT) && (pMobj->y == 992 * FRACUNIT)) {
                        EV_TeleportTo(*pMobj, -2050 * FRACUNIT, pMobj->y, pMobj->angle, false, false, (mobjtype_t)0, sfx_None);
                    }

                    pMobj = pNextMobj;
                }
            }, 16
        );
    }

    if (shouldApplyMapPatches_Visual()) {
        // Fix door/platform textures
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 
            // North building
            622, 623, 625, 627,
            // Northeast courtyard door
            535, 536,
            // West courtyard door
            552, 573,
            // Southwest courtyard door
            451, 459,
            // Northwest buildings
            480, 486, 497, 502,
            // Southwest monster closet
            543, 547,
            // Southwest blue armor area
            742, 743
        );

        // Hide linedefs
        addFlagsToLinedefs(ML_DONTDRAW, 413, 647, 447);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP47: The Citadel
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCitadel() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Fix the starting hut: make it so you can see past it, since it is shorter than buildings around it
        addVoidFlagToLinedefs(170, 171, 172, 173, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Raise elevator to same floor height as neighboring sector in NE corner so it functions like it should
        gpSectors[8].floorheight = 176 * FRACUNIT;

        // Fix SW teleporter in teleporter room not working on easier skills
        if (gGameSkill <= sk_easy) {
            mobj_t* const pTeleDest = P_SpawnMobj(-912 * FRACUNIT, -880 * FRACUNIT, INT32_MIN, MT_TELEPORTMAN);
            pTeleDest->angle = ANG270;
        }
    }

    if (shouldApplyMapPatches_Visual()) {
        // Fix wall texture moving that shouldn't on barrier in yellow key room
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 509);

        // Fix door tracks in "imp room"
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 922, 923);

        // Hide linedefs
        addFlagsToLinedefs(ML_DONTDRAW, 1130, 1218, 1219, 1220);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // The cacodemon (hard skill) is stuck in the ceiling in the northwest building; raising the ceiling
        gpSectors[91].ceilingheight = 328 * FRACUNIT;
        gpSectors[92].ceilingheight = 328 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP49: The Catacombs
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCatacombs() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Add missing texture to back sidedef on bridge that raises when you use the switch two times
        gpSides[gpLines[54].sidenum[1]].bottomtexture = R_TextureNumForName("METAL03");
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM, 54);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP50: Barrels of Fun
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BarrelsOfFun() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Hide linedefs
        addFlagsToLinedefs(ML_DONTDRAW, 394, 395, 396, 397, 400, 403, 404, 405, 408);

        // Fix textures on large door in east outdoor area
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 144, 161);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP52: The Abandoned Mines
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheAbandonedMines() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix textures on lava room monster closet door
        addFlagsToLinedefs(ML_DONTPEGBOTTOM, 805, 808);
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 803);

        // Fix textures on secret doors in central area
        removeFlagsFromLinedefs(ML_DONTPEGBOTTOM | ML_DONTPEGTOP, 91, 337);
        gpSides[gpLines[91].sidenum[0]].rowoffset = -8 * FRACUNIT;
        gpSides[gpLines[337].sidenum[0]].rowoffset = 24 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP53: Monster Condo
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_MonsterCondo() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix wall textures moving that shouldn't move
        addFlagsToLinedefs(ML_DONTPEGBOTTOM,
            // Hidden passage in SE corner of map
            226, 230,
            // Wall opening in NE corner of map
            567, 568,
            // West doors in starting room
            172, 176, 177, 179, 182, 184, 188, 190
        );
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Remove a redundant switch action from linedef that doesn't have a switch texture in the SE corner of map
        gpLines[201].special = 0;

        // Linedef is flagged as impassable in NE room when it should not be
        removeFlagsFromLinedefs(ML_BLOCKING, 342);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP58: The Mansion
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheMansion() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix the secret for the Club Doom exit often not registering.
        // Shift the secret to a sector where it should always be counted.
        gpSectors[63].special = 0;
        gpSectors[84].special = 9;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the map patches for this game type
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatchArray_Doom[] = {
    {  56435, 0x7921ADB466CEE45E, 0x4476F208866BF8A7, patchMap_Hangar               },      // MAP01
    { 119369, 0x4ED7CD6367900B52, 0x9609E85DB101DC09, patchMap_Plant                },      // MAP02
    { 110284, 0x9BFF3A037128D1CA, 0x12F445D3F9B8BAC6, patchMap_ToxinRefinery        },      // MAP03
    {  92341, 0x1D79B5BDE5426081, 0x4E9413A01EAF4B4A, patchMap_CommandControl       },      // MAP04
    {  89865, 0x0A8ACFFC833D6E36, 0x070A7A5CDDEE1CE0, patchMap_PhobosLab            },      // MAP05
    { 124094, 0x2097E86807523FF3, 0xA2F0C52632B12372, patchMap_CentralProcessing    },      // MAP06
    { 108814, 0xD89ECAA4823454FD, 0xC7C178FA280CA569, applyOriginalMapCommonPatches },      // MAP07
    {  51882, 0x94BC7E609E1AC29A, 0xC1B6D482725C2C34, applyOriginalMapCommonPatches },      // MAP08
    {  47025, 0x492736BF0840ED38, 0x92A3AA841280B742, patchMap_DeimosAnomaly        },      // MAP09
    {  97045, 0x48FFA0D005CB2DDA, 0x2631E9D5AB867200, patchMap_ContainmentArea      },      // MAP10
    {  75368, 0x6D99C761DE799820, 0xAEDB0E4CA9441431, patchMap_Refinery             },      // MAP11
    { 119221, 0xB0E9622905A41337, 0xED94BA27D70017BF, patchMap_DeimosLab            },      // MAP12
    {  83505, 0x8635E6DB6360B27C, 0xD5835A25E276A0C4, patchMap_CommandCenter        },      // MAP13
    {  85802, 0x556287C93A6396F9, 0xC019D5F66797A596, patchMap_HallsOfTheDamned     },      // MAP14
    {  83539, 0xFDA28FD54C7E9A92, 0xE7F93F0E3C5C1D7F, patchMap_SpawningVats         },      // MAP15
    {  27956, 0x39B94C1CF5E19EB0, 0xE0A691816A8C166A, patchMap_HellGate             },      // MAP16
    {  56466, 0x4F240435B71CA6CA, 0xFA106C3EC5548BF0, patchMap_HellKeep             },      // MAP17
    {  71253, 0x0541C17B11B2DC05, 0x577D152A01E48073, patchMap_Pandemonium          },      // MAP18
    {  75515, 0xFE716B01FE414A2A, 0xA3A7AFA1956DF697, patchMap_HouseOfPain          },      // MAP19
    { 143483, 0x36A01960BAD36249, 0x2BC3BF03E0ED6D64, patchMap_UnholyCathedral      },      // MAP20
    {  86538, 0x403A02FD929949E5, 0xB4185CB43CEA9B46, patchMap_MtErebus             },      // MAP21
    { 109754, 0x1E3E66448FE6645C, 0x3DCA2CA78FC862F3, patchMap_Limbo                },      // MAP22
    {  32935, 0x55A24A4ED4053AC3, 0x636CDB24CE519EF8, applyOriginalMapCommonPatches },      // MAP23
    {  52915, 0xA8CCE876F52671B2, 0xDA2BB82C5D03383C, patchMap_HellBeneath          },      // MAP24
    {  72352, 0x255311EE3A46B4F4, 0x30E325760C3C0D55, patchMap_PerfectHatred        },      // MAP25
    { 111520, 0x85B038429CCD933B, 0x8488BBE9B15A5F8C, applyOriginalMapCommonPatches },      // MAP26
    {  82104, 0x52B9EDF6AA65FD8C, 0x3D965AFD07455BA6, patchMap_UnrulyEvil           },      // MAP27
    { 146652, 0x1C5AD3B2CC520748, 0x79223365451D6965, patchMap_UntoTheCruel         },      // MAP28
    { 163970, 0x85E5F59863FC567A, 0x825E1D627586324B, patchMap_TwilightDescends     },      // MAP29
    { 146600, 0x0776A66BD2962C70, 0xEA25B44BFB2863F0, applyOriginalMapCommonPatches },      // MAP30
    {  46210, 0x41EA6956972B2510, 0xE4760C46A4BBD40D, patchMap_Entryway             },      // MAP31
    {  63255, 0x787980722B2A3ABF, 0xDA758F7A7236BAD9, patchMap_Underhalls           },      // MAP32
    {  71907, 0x9354072B9094E9BE, 0xFDA856CDE67680DC, applyOriginalMapCommonPatches },      // MAP33
    {  67614, 0xE36C70A633E0AE7D, 0x9223DF3ADFDF8808, patchMap_TheFocus             },      // MAP34
    { 114123, 0x52229ABCD304D8BA, 0x6EAEA8DB75133B5A, patchMap_TheWasteTunnels      },      // MAP35
    { 129248, 0xE2245D687CCABC7C, 0x01497DF00B763463, patchMap_TheCrusher           },      // MAP36
    {  26682, 0x2B0A8D80B5411593, 0x3A427EE05B7353F6, patchMap_DeadSimple           },      // MAP37
    {  82063, 0xBFEDBDE9F8B8CCE2, 0x78D6E2C3A9AB74AB, patchMap_TricksAndTraps       },      // MAP38
    {  91388, 0x22B7D106F531FB4E, 0xFE3FAB276C892BD4, patchMap_ThePit               },      // MAP39
    { 130676, 0xD84B13024E326B64, 0x548472C7F8B24A27, patchMap_RefuelingBase        },      // MAP40
    { 116024, 0x59800E5259D02FD8, 0x28EB273CFC8E41CC, patchMap_OofDestruction       },      // MAP41
    { 109934, 0x7E22F4311F3955D5, 0x16E918F5C11AD780, patchMap_TheFactory           },      // MAP42
    { 192997, 0x7B86B9C35B754883, 0xD5F5CE44AB12898D, applyOriginalMapCommonPatches },      // MAP43
    { 110145, 0xE296122ADE38AB74, 0x13505BF841234D4C, patchMap_Suburbs              },      // MAP44
    { 158462, 0x37D6A1335F058A41, 0xA82656A6FDEB132B, patchMap_Tenements            },      // MAP45
    { 105883, 0x0CA0922874005BC1, 0x37173A0C68F8FA6A, patchMap_TheCourtyard         },      // MAP46
    { 186755, 0x73E10EF08AE21FD5, 0x8115F467FE2CD3CA, patchMap_TheCitadel           },      // MAP47
    {  54866, 0xF41440631C2B6FB2, 0x728E55510D5AE858, applyOriginalMapCommonPatches },      // MAP48
    {  74303, 0x522256004AD8E073, 0x2C190C108C98B31D, patchMap_TheCatacombs         },      // MAP49
    {  64540, 0x47EA67DBBA5F33DC, 0x2280784D842FECC1, patchMap_BarrelsOfFun         },      // MAP50
    { 106555, 0x9FCBB09C2A8C8B67, 0xF00080B9655646C8, applyOriginalMapCommonPatches },      // MAP51
    { 117839, 0x67138B444A196EC4, 0x229285E95F31ADE4, patchMap_TheAbandonedMines    },      // MAP52
    { 131947, 0xC966739D25AC3FFD, 0xB7CDA8E3CF9A5186, patchMap_MonsterCondo         },      // MAP53
    {  45962, 0x27D515F2A59962E3, 0x3E35ABAD09E87EA1, applyOriginalMapCommonPatches },      // MAP54
    {  19237, 0xB5116FF7C0CBCF38, 0x7C6C9E29F2EA963B, applyOriginalMapCommonPatches },      // MAP55
    {  85042, 0x98C00035EA735BF3, 0x36C5C0BA592334C9, applyOriginalMapCommonPatches },      // MAP56
    {  58333, 0xBBE3159AAEE4F03D, 0x086F778E5A08DBAD, applyOriginalMapCommonPatches },      // MAP57
    { 194653, 0x158C832CA1D1C539, 0x9C4ED57B29C13E66, patchMap_TheMansion           },      // MAP58
    {  79441, 0x0ECE269F1AA74445, 0x0B254FDF53895D4F, applyOriginalMapCommonPatches },      // MAP59
};

const PatchList gPatches_Doom = { gPatchArray_Doom, C_ARRAY_SIZE(gPatchArray_Doom) };

END_NAMESPACE(MapPatches)
