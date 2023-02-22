//------------------------------------------------------------------------------------------------------------------------------------------
// A module containing map patches to apply to the 'GEC Master Edition' (Beta 4)
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatches.h"

#include "Doom/Game/p_change.h"
#include "Doom/Renderer/r_data.h"
#include "MapPatcherUtils.h"

using namespace MapPatcherUtils;

BEGIN_NAMESPACE(MapPatches)

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP11: Sheol
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Sheol() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix parts of the exit room which should not be mapped (like the original version)
        hideLines(181, 182, 183, 593, 830, 897, 981);

        // Fix a lift inside the skin building in the outdoor area looking weird when moving
        removeFlagsFromLines(ML_DONTPEGBOTTOM, 1030);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP13: Paths of Wretchedness
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_PathsOfWretchedness() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix lines which should be mapped in outside lava area past the red, blue and yellow key barrier
        unhideLines(65, 189, 198, 263, 879, 1371);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP14: Abaddons Void
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_AbaddonsVoid() noexcept {
    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a jump from one ledge to the other (near the start area) being almost impossible with 30 Hz physics.
        // Raise the ledge being jumped from to make it possible.
        gpSectors[51].floorheight += 16 * FRACUNIT;
        gpSides[gpLines[401].sidenum[1]].bottomtexture = R_TextureNumForName("WOOD07");

        for (mobj_t* pMobj = gpSectors[51].thinglist; pMobj; pMobj = pMobj->snext) {
            // Make sure the health bonuses in the sector we just raised get moved too...
            P_ThingHeightClip(*pMobj);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP15: Unspeakable Persecution
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_UnspeakablePersecution() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix missing textures for a floor which raises to block off a pathway (near the start area)
        modifyLines(
            [](line_t& line) noexcept {
                gpSides[line.sidenum[0]].bottomtexture = R_TextureNumForName("REDROKX1");
            },
            33, 34, 859
        );

        // Fix a map line which should not show near the start (hell symbol)
        hideLines(733);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a line which the player should be able to cross to use the teleport it bounds
        modifyLines(
            [](line_t& line) noexcept {
                line.special = 97;  // WR Teleport
                line.tag = 41;
            },
            385
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP16: Nightmare Underworld
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_NightmareUnderworld() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a map line in the secret area with the soulsphere (near a lava pit) from being mapped
        hideLines(165);

        // Fix a door which looks weird as it opens (should not be upper unpegged).
        // This door is near the big hellish crack in the wood + skin room.
        removeFlagsFromLines(ML_DONTPEGTOP, 1494);
        gpSides[gpLines[1494].sidenum[0]].rowoffset = 8 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP18: Downtown
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Downtown() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix lines which should be mapped in the building with the rocket launcher
        unhideLines(268, 320);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP19: Industrial Zone
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_IndustrialZone() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a line which should be mapped in the building with the red key
        unhideLines(110);

        // Fix lines which should be mapped outside, near the start area
        unhideLines(397, 422, 424, 832);
    }

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix the switch past the gate leading up a secret section of the castle wall being usable through the metal bars.
        // Seal up the bar gate completely (to make it block line activations) and adjust the texture coords to make it look prettier.
        // This is a bug introduced by PsyDoom's improved 'use line' logic; the switch should have been technically usable through the bars
        // previously (according to the game rules) but wasn't due to bugs in how the line activation logic worked (a happy coincidence).
        modifySectors(
            [](sector_t& sector) noexcept {
                sector.floorheight = sector.ceilingheight;
            },
            260
        );

        modifyLines(
            [](line_t& line) noexcept {
                gpSides[line.sidenum[0]].rowoffset = 0;
                line.flags |= ML_VOID;
            },
            1113, 737
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP20: Betray
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Betray() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a map line on the teleporter beside the exit room which should show
        unhideLines(559);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP30: Vivisection
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Vivisection() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix monster closet map lines which should be hidden after getting the map powerup
        hideLines(121, 163, 424, 443, 869, 897, 974);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP45: The Farside Of Titan
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheFarsideOfTitan() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a map line which should show near the Soulsphere on a pedestal
        unhideLines(562);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP46: Dantes Gate
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DantesGate() noexcept {
    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a line which should not be marked as a door line beside the room with the blue key.
        // Activating this door line messes up the the adjacent sector.
        removeLineActions(645);
    }

    if (shouldApplyMapPatches_Visual()) {
        // Fix 2 door lines in a monster closet (which is not intended to be mapped) from appearing.
        // This small room is joined to the large circular room, at the south west corner.
        hideLines(852, 853);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP48: Bloodflood
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Bloodflood() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a hidden map line in the monster closet in the sewers, beside the green armor
        unhideLines(400);

        // Fix a map line for the exit portal not showing
        addFlagsToLines(ML_SECRET, 527);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP49: Derelict Station
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DerelictStation() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix some map lines showing inside a hidden monster closet attached to the room with the blue key
        hideLines(38, 649, 717, 718);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP54: The Image of Evil
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheImageOfEvil() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a missing door side texture near the exit portal
        gpSides[gpLines[925].sidenum[0]].midtexture = R_TextureNumForName("BROWN12");

        // Fix a missing map line in the intenstine maze (attached to an unmapped room)
        addFlagsToLines(ML_SECRET, 920);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP55: Black Tower
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BlackTower() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a missing step texture in the room with the yellow key
        gpSides[gpLines[951].sidenum[1]].bottomtexture = R_TextureNumForName("MARBLE04");

        // Fix a map line which should not be hidden (in the small room attached to the Megasphere)
        unhideLines(760);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP57: The Express Elevator to Hell
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheExpressElevatorToHell() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Hide a monster closet automap line that should be hidden (in the south room with 'plus' shaped pillbox in the middle)
        hideLines(495);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP59: Hanger
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Hanger() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix map lines in the room with the Soulsphere and large pillars not showing
        unhideLines(215, 1101, 1504, 1507, 1508, 1510, 1532, 1536);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP65: Storage Facility
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_StorageFacility() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix map lines in a hidden monster closet room sometimes showing (the room is intended to be hidden)
        hideLines(539, 479);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP67: Dead Zone
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DeadZone() noexcept {
    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a sector containing deaf monsters (outside the outer wall) never raising, making 100% kills impossible.
        // Fix by making it raise with another similar sector containing monsters outside the wall.
        gpSectors[32].tag = 36;
    }

    if (shouldApplyMapPatches_Visual()) {
        // Fix a line inside the secret exit room which should not show
        hideLines(649);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP69: Shipping Respawning
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ShippingRespawning() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a map line in the area with the crates not showing
        unhideLines(1625);

        // Fix a map line inside a monster closet which should not show, near the teleporter pads by the crates
        hideLines(500);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP70: Central Processing
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_CentralProcessing() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a map line in a slime pit to the west of the map not showing
        unhideLines(1034);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP71: Administration Center
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_AdministrationCenter() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a map line showing at the east of the building which is part of a hidden room
        hideLines(1213);

        // Fix a map line not showing in the room with the teleport leading towards the outside
        unhideLines(834);

        // Fix a map line not showing in the outdoor area (inside the big building)
        unhideLines(600);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP74: Mount Pain
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_MountPain() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a map line which should show in the terraced room past the brimstone maze
        unhideLines(349);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP76: Well Of Souls
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_WellOfSouls() noexcept {
    if (shouldApplyMapPatches_GamePlay()) {
        // Fix not being able to reach the exit again if backtracking after raising the final lift to the exit.
        // This line special would lower the lift permanently, preventing the player from reaching the exit.
        // It's not needed for anything so just remove the special:
        removeLineActions(590);
    }

    if (shouldApplyMapPatches_Visual()) {
        // Fix a map line not showing by the yellow key pedestal
        unhideLines(196);

        // Fix map lines not showing for the cliff face by the bridge
        unhideLines(403, 405);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP82: Speed
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Speed() noexcept {
    if (shouldApplyMapPatches_GamePlay()) {
        // Fix the player being able to see past the end of the sky in various places: raise outer walls bordering the sky
        modifySectors(
            [](sector_t& sector) noexcept {
                sector.floorheight += 16 * FRACUNIT;
                sector.ceilingheight += 16 * FRACUNIT;
            },
            130
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP88: Neurosphere
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Neurosphere() noexcept {
    if (shouldApplyMapPatches_GamePlay()) {
        // Fix being able to get stuck in the blood in the southernmost room.
        // The player can bypass a line which raises elevators/platforms (containing chaingunners) which allow escape from the blood area.
        // This bypass can be achieved by hopping up onto a half wall and bypassing the trigger line near the Super Shotgun.
        modifyLines(
            [](line_t& line) noexcept {
                // Fix by adding an additional 'W1 Floor Raise To Next Higher Floor' trigger which cannot be bypassed!
                line.special = 119;
                line.tag = 7;
            }, 
            599
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP90: Slayer
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Slayer() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix map lines showing in a part of a monster closet which should be hidden (to the west of the map)
        hideLines(219, 220);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP99: Warrens
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Warrens() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix a missing map line near the Soulsphere by the red key
        unhideLines(456);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP109: Go 2 It
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Go2It() noexcept {
    if (shouldApplyMapPatches_Visual()) {
        // Fix missing map lines in the room with the grass and pillars
        unhideLines(248, 249, 250, 251);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the map patches for this game type
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatchArray_GEC_ME_Beta4[] = {
    { 160332, 0x013F3B27435F706B, 0xA99F122A12E2E3EE, patchMap_Sheol },                         // MAP11
    { 173712, 0x93D4D689C51DEF47, 0x9F26A30E8101CF37, patchMap_PathsOfWretchedness },           // MAP13
    { 105207, 0xA2B8A7F0CFA83D6B, 0xF88EAB0FDCE3667A, patchMap_AbaddonsVoid },                  // MAP14
    { 132114, 0x4DDD1E65175AF8F7, 0x01668444BCDDBBC1, patchMap_UnspeakablePersecution },        // MAP15
    { 171138, 0xC908305550FCEAF7, 0x640FD1210269AB9D, patchMap_NightmareUnderworld },           // MAP16
    { 103102, 0x9C898E35FBD2E7F7, 0x5069635F626857D8, patchMap_Downtown },                      // MAP18
    { 162499, 0x05C85D345D4523AE, 0x77C081B1536F0B82, patchMap_IndustrialZone },                // MAP19
    {  87382, 0x85CC150D7341AD22, 0x01555EF45756FF8E, patchMap_Betray },                        // MAP20
    { 122661, 0xD43A809445B5907E, 0x4AD280F7598BE3D5, patchMap_Vivisection },                   // MAP30
    { 151044, 0xE49FB277051FDB5B, 0xD95431833051308A, patchMap_TheFarsideOfTitan },             // MAP45
    { 122098, 0xDF2507C85472FB25, 0xEFAED249AFE4CCD2, patchMap_DantesGate },                    // MAP46
    {  59287, 0x29A485887DAE27C3, 0x534228B651D4DFDE, patchMap_Bloodflood },                    // MAP48
    {  68720, 0xAC099662761778F7, 0xD2F689E6EC430246, patchMap_DerelictStation },               // MAP49
    { 120364, 0xA17F95563CCAE891, 0xC7BC7B007C0E9C2E, patchMap_TheImageOfEvil },                // MAP54
    { 143954, 0x457AA5F01F5592E0, 0xAD3A9AC1BF42EF93, patchMap_BlackTower },                    // MAP55
    { 123320, 0xA97F2699096BE16E, 0x81447F745D75DD8B, patchMap_TheExpressElevatorToHell },      // MAP57
    { 140344, 0xF926DB363CE3A90A, 0x2944E39AE9A969A1, patchMap_Hanger },                        // MAP59
    { 157892, 0x007C1D52778AB863, 0x3AD9589FE0B4F2C9, patchMap_StorageFacility },               // MAP65
    { 123951, 0x441FA8942AE1EF39, 0xCB7E8B72E7B50B06, patchMap_DeadZone },                      // MAP67
    { 179291, 0x2F8C304B18C2E917, 0xCC356F976B15958B, patchMap_ShippingRespawning },            // MAP69
    { 149154, 0x8973A4B380C2AC19, 0x9CCE3CB032CF6186, patchMap_CentralProcessing },             // MAP70
    { 174087, 0xE298170F408CF590, 0xE04DC9FFE6162B53, patchMap_AdministrationCenter },          // MAP71
    { 186331, 0x23518DCF7456882D, 0xC47FB9150E9413A6, patchMap_MountPain },                     // MAP74
    {  91382, 0x9E5DDD89794B172C, 0x3A73D5AEBF5566A5, patchMap_WellOfSouls },                   // MAP76
    { 115075, 0x229AB3F282BAF6B0, 0x008C9CF373E2ECD4, patchMap_Speed },                         // MAP82
    { 142221, 0x93EB99DA2D4BF37E, 0xE2A66BA48B17B268, patchMap_Neurosphere },                   // MAP88
    {  46378, 0x90476B1E1C3EF4CA, 0x3AB1827AEEE4D881, patchMap_Slayer },                        // MAP90
    {  46366, 0x39399FE7670D2BDC, 0x988E14C60AA40392, patchMap_Warrens },                       // MAP99
    {  95297, 0x7933332EEC490012, 0x31A3577166916A83, patchMap_Go2It },                         // MAP109
};

const PatchList gPatches_GEC_ME_Beta4 = { gPatchArray_GEC_ME_Beta4, C_ARRAY_SIZE(gPatchArray_GEC_ME_Beta4) };

END_NAMESPACE(MapPatches)
