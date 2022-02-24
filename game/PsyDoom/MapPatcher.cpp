//------------------------------------------------------------------------------------------------------------------------------------------
// A module that allows patching original game maps to fix a few select issues.
// Provides mechanisms to check when patches should be applied and calls patch functions to do the modifications.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatcher.h"

#include "Asserts.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Game.h"
#include "MapHash.h"

BEGIN_NAMESPACE(MapPatcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Format for a function that patches map data
//------------------------------------------------------------------------------------------------------------------------------------------
typedef void (*PatcherFunc)();

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a patch to be applied to map data.
// Defines when the patch is applied and the function that will apply the patch.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PatchDef {
    int32_t         mapSize;        // Filter: combined size (in bytes) of all the lumps for the map
    uint64_t        md5Word1;       // Filter: The MD5 for the combined map lump data (bytes 0-7)
    uint64_t        md5Word2;       // Filter: The MD5 for the combined map lump data (bytes 8-15)
    PatcherFunc     patcherFunc;    // The function that does the work of patching
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: apply a transformation to a number of linedefs
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ModFuncT>
static void modifyLinedefs([[maybe_unused]] const ModFuncT & func) noexcept {}

template <class ModFuncT, class ...Int32List>
static void modifyLinedefs(const ModFuncT & func, const int32_t linedefIdx, Int32List... linedefIndexes) noexcept {
    ASSERT(linedefIdx < gNumLines);
    func(gpLines[linedefIdx]);
    modifyLinedefs(func, linedefIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: apply a transformation to a number of sectors
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ModFuncT>
static void modifySectors([[maybe_unused]] const ModFuncT & func) noexcept {}

template <class ModFuncT, class ...Int32List>
static void modifySectors(const ModFuncT & func, const int32_t sectorIdx, Int32List... sectorIndexes) noexcept {
    ASSERT(sectorIdx < gNumSectors);
    func(gpSectors[sectorIdx]);
    modifySectors(func, sectorIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears mysterious sector flags (of unknown purpose) other than 'SF_NO_REVERB' found in original game maps.
// 
// The 'SF_NO_REVERB' flag was the only one originally used by the retail version of the game but for some reason various sectors in the
// game can use any of the 16 available bits as sector flags. This would not be a problem except that PsyDoom now assigns meaning to the
// most of the flag bits (for 2-colored lighting and other features) and this can cause issues for original game maps.
// Hence we must clear all sector flag bits (other than the 1st) for original game maps.
//------------------------------------------------------------------------------------------------------------------------------------------
static void clearMysterySectorFlags() noexcept {
    const int32_t numSectors = gNumSectors;
    sector_t* const pSectors = gpSectors;

    for (int32_t i = 0; i < numSectors; ++i) {
        // Note: flags is now only interpreted as 8-bits in the level data, the other 8-bits are used for 'ceilColorid'
        pSectors[i].flags &= (~SF_GHOSTPLAT);

        // Originally this was the high 8-bits of the 'flags' field.
        // Original maps never use 2 colored lighting so just set it to the floor color.
        pSectors[i].ceilColorid = pSectors[i].colorid;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Common patches applied to ALL original game maps
//------------------------------------------------------------------------------------------------------------------------------------------
static void applyCommonMapPatches() noexcept {
    clearMysterySectorFlags();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// A helper for 'GEC Master Edition (Beta 3)': fixes the wrong version of the 'REDROK01' texture being used for the 'Doom' episode maps.
// Switches the 'Final Doom' style of this texture with the original 'Doom' version.
//------------------------------------------------------------------------------------------------------------------------------------------
static void gecMEBeta3_fixWrongREDROK01() noexcept {
    const int32_t numSides = gNumSides;
    const int32_t oldTexNum = R_TextureNumForName("REDROK01");
    const int32_t newTexNum = R_TextureNumForName("REDROKX1");

    if ((oldTexNum >= 0) && (newTexNum >= 0)) {
        for (int32_t sideIdx = 0; sideIdx < numSides; ++sideIdx) {
            side_t& side = gpSides[sideIdx];

            if (side.bottomtexture == oldTexNum) {
                side.bottomtexture = newTexNum;
            }
            
            if (side.midtexture == oldTexNum) {
                side.midtexture = newTexNum;
            }

            if (side.toptexture == oldTexNum) {
                side.toptexture = newTexNum;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix bugs in Doom MAP04, 'Command Control' where step textures appear black
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_CommandControl() noexcept {
    applyCommonMapPatches();

    // These steps need to have their 'lower unpegged' flag cleared
    modifyLinedefs(
        [](line_t& line) { line.flags &= ~ML_DONTPEGBOTTOM; },
        337, 746, 747, 748
    );

    // This step needs a texture coordinate adjustment
    gpSides[253].rowoffset = 0;
    gpSides[253].rowoffset.snap();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix a bug in Doom with MAP19 ('House Of Pain') where an unintended door linedef causes one of the ceilings to collapse permanently.
// Removes the line action which causes the bug.
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HouseOfPain() noexcept {
    applyCommonMapPatches();
    gpLines[435].special = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix a bug in Doom with MAP22 ('Limbo') where a single step/lower-wall is not rendered
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Limbo() noexcept {
    applyCommonMapPatches();

    // This texture was not assigned in the original map, but was defaulted to texture index '0' by the original PSX Doom code in 'P_Init'.
    // PsyDoom now allows walls to deliberately have no texture assigned so they can be invisible, but that change causes a bug with this
    // step (beside the lift) not appearing in the room with the lift and the Baron. Fix by replicating the original PSX behavior.
    gpSides[967].bottomtexture = 0;
}


//------------------------------------------------------------------------------------------------------------------------------------------
// Fixes to 'Entryway' for Doom: fixes grass beyond the sky wall sometimes appearing
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Entryway() noexcept {
    applyCommonMapPatches();
    gpSectors[25].floorpic = -1;    // Sky, don't render any grass!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fixes the starting hut for MAP47, 'The Citadel' for Doom.
// Makes it so you can see past it, since it is shorter than buildings around it.
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCitadel() noexcept {
    applyCommonMapPatches();
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        170, 171, 172, 173, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix a wall top/floor sometimes not appearing for MAP46, 'The Courtyard' for Doom.
// This wall is at the starting area and borders the sky.
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCourtyard() noexcept {
    applyCommonMapPatches();
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        45, 17, 18, 51
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix various issues in MAP23, 'Ballistyx' for Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Ballistyx() noexcept {
    applyCommonMapPatches();

    // Make various linedefs not render sky walls or be see through for occlusion purposes
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        // Altar hole: don't draw sky walls
        1212, 1211, 1215, 1210, 1214, 1213,
        // Altar pillars: don't occlude and prevent geometry behind from rendering (needed for floating platform hack)
        1216, 1291, 1290, 1207, 1209, 1292, 1293, 1199,
        // Yellow key cage area: the outer wall floors sometimes don't render due to occluding sky walls; make them not occlude:
        1525, 1064, 1526, 1052, 371, 1053, 374, 1509, 373, 1054, 1524, 1055
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix various issues in MAP07, 'Against Thee Wickedly' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_AgainstTheeWickedly() noexcept {
    // Fix a sector which is marked as damaging that shouldn't be
    gpSectors[95].special = 0;
}


//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP11 'Betray' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Betray() noexcept {
    // Fix sky walls rendering where they should not
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        495, 496, 497, 498, 499, 500, 501, 502, 673, 674, 675, 676, 677, 678, 679, 680,     // Two teleporter pillars
        356, 357, 358, 359, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606                // Cross
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP24 'Tomb Of Malevolence' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TombOfMalevolence() noexcept {
    // Set the next level to the start map of the next episode instead of '99', so the next episode is correctly selected on the main menu
    modifyLinedefs(
        [](line_t& line) { line.tag = 31; },
        661, 662, 663, 664
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP30 'Baphomet Demense' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BaphometDemense() noexcept {
    gecMEBeta3_fixWrongREDROK01();

    // Fix glitches in the caged area where the soulsphere is.
    // Create walls to cover the areas where the sky doesn't render; also fixes the sky flickering on and off!
    gpSectors[174].ceilingheight = 136 * FRACUNIT;

    // Fix glitches in the caged area where the chaingun is: get rid of the sky and brighten up the newly showing skin wall a little
    gpSectors[162].lightlevel = 104;
    gpSectors[177].ceilingpic = gpSectors[178].ceilingpic;
    gpSectors[178].lightlevel = 128;

    // Set the next level to the start map of the next episode instead of '99', so the next episode is correctly selected on the main menu.
    modifyLinedefs(
        [](line_t& line) { line.tag = 31; },
        683
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP32 'Trapped On Titan' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TrappedOnTitan() noexcept {
    // Fix the player being unable to escape the area with the rising stairs and overhead sky.
    // Raise the ceiling by 64 for all sectors in that square, so the player can reach the top of the stairs.
    modifySectors(
        [](sector_t& sector) { sector.ceilingheight += 64 * FRACUNIT; },
        25, 37, 28, 35, 27, 39, 30, 41, 32, 38, 29, 36, 33, 40, 31, 42, 34
    );

    // Fix some sides by the sky squares in the above area being cut off
    gpSectors[25].ceilingheight -= 24 * FRACUNIT;

    // Fix some missing textures with a lift in the room with the blue door
    gpSides[gpLines[212].sidenum[0]].bottomtexture = R_GetOverrideTexNum(R_TextureNumForName("SUPPORT5"));
    gpSides[gpLines[212].sidenum[1]].bottomtexture = R_GetOverrideTexNum(R_TextureNumForName("DOORTRAK"));

    // Fix some steps being too high to climb up in the wooden building
    gpSectors[194].floorheight += 2 * FRACUNIT;
    gpSectors[195].floorheight += 4 * FRACUNIT;

    // Fix glitches when looking over one of the buildings outside: raise its sky level to be the same as the other buildings.
    // This also makes the building more faithful to the PC original.
    gpSectors[107].ceilingheight = 0;

    // Fix the area beyond the lift not lowering after the switch outside is pressed.
    // An Imp stuck in the ceiling prevents this. Fix by raising the ceiling so it is not stuck.
    gpSectors[270].ceilingheight += 64 * FRACUNIT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP34 'Black Tower' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BlackTower() noexcept {
    // Fix sky walls displaying over the pillars supporting a covering by the brown sludge
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        629, 627, 939, 624, 626, 704, 623, 622, 693, 632, 633, 680, 634, 1081, 834, 636, 637, 985
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP36 'TEETH' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TEETH() noexcept {
    // Correct the target secret level number from '20' (map number within the episode) to '50' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 50; },
        304
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP49 'The Image of Evil' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheImageOfEvil() noexcept {
    // Set the next level to the start map of the next episode instead of '99', so the next episode is correctly selected on the main menu.
    modifyLinedefs(
        [](line_t& line) { line.tag = 51; },
        929, 936, 937, 938
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP50 'Bad Dream' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BadDream() noexcept {
    // Correct the level to return to from '7' (map number within the episode) to '37' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 37; },
        83
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP57 'Redemption' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Redemption() noexcept {
    // Fix a missing texture on a step side after it lowers
    gpSides[gpLines[597].sidenum[1]].bottomtexture = R_GetOverrideTexNum(R_TextureNumForName("METAL03"));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP58 'Storage Facility' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_StorageFacility() noexcept {
    // Hide a teleporter monster closet that sometimes shows outside: make its walls and ceilings invisible
    modifySectors(
        [](sector_t& sector) { sector.ceilingpic = -1; },
        183, 184
    );

    modifyLinedefs(
        [](line_t& line) { gpSides[line.sidenum[0]].midtexture = -1; },
        1075, 1076, 1077, 1078, 1079, 1080, 1081, 1082, 1083
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP60 'Dead Zone' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_DeadZone() noexcept {
    // Correct the target secret level number from '19' (map number within the episode) to '69' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 69; },
        672
    );

    // Fix some outer sky wall floors that should not render (make them sky too)
    modifySectors(
        [](sector_t& sector) { sector.floorpic = -1; },
        220, 91, 219, 80
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP62 'Shipping Respawning' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ShippingRespawning() noexcept {
    // Fix some windows not showing any geometry past them due to a small completely closed sector in the middle - treat it as a void:
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        5, 8, 1189, 1568
    );

    // Fix sky walls being rendered for some pillars outside that shouldn't have them
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        20, 21, 22, 23, 14, 24, 25, 26, 16, 27, 28, 29, 18, 32, 31, 30
    );

    // Fix being able to get stuck outside of the level bounds if rushing forward to collect the soulsphere.
    // Don't allow the player to pass certain lines.
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_BLOCKING; },
        60, 817, 186
    );

    // Fix being able to see past the end of the sky texture in the soulsphere area - raise the walls to cover it
    modifySectors(
        [](sector_t& sector) { sector.floorheight += 12 * FRACUNIT; },
        12, 168
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP67 'Mount Pain' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_MountPain() noexcept {
    // Fix being able to see bits of geometry that shouldn't be seen through some outside windows.
    // Force the fully closed sector containing the sky to render, hence forcing a skywall to render too. Also remove its floor.
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        1360, 1412
    );

    gpSectors[36].floorpic = -1;

    // Remove a hook that floats in the sky outside
    for (mobj_t* pMobj = gpSectors[81].thinglist; pMobj != nullptr;) {
        if (pMobj->type == MT_MISC_BLOODHOOK) {
            mobj_t* const pNextMobj = pMobj->snext;
            P_RemoveMobj(*pMobj);
            pMobj = pNextMobj;
        } else {
            pMobj = pMobj->snext;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP68 'River Styx' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_RiverStyx() noexcept {
    // Set the next level to the start map of the next episode instead of '99', so the next episode is correctly selected on the main menu.
    modifyLinedefs(
        [](line_t& line) { line.tag = 71; },
        1840
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP69 'Pharaoh' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Pharaoh() noexcept {
    // Correct the level to return to from '11' (map number within the episode) to '61' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 61; },
        162, 258, 259, 429, 573, 734, 929, 964
    );

    // Correct the target secret level number from '20' (map number within the episode) to '70' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 70; },
        1323
    );

    // Remove some skywalls that shouldn't be there for an outside box area
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        890, 910, 911, 912, 906, 907, 908, 909
    );

    // Remove some skywalls around the ship near the exit area that shouldn't be there
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        1103, 293, 1091, 746, 1393
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP70 'Caribbean' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Caribbean() noexcept {
    // Correct the level to return to from '11' (map number within the episode) to '61' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 61; },
        662
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP79 'The Twilight' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheTwilight() noexcept {
    // Correct the target secret level number from '23' (map number within the episode) to '93' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 93; },
        120, 309, 911, 919
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP80 'The Omen' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheOmen() noexcept {
    // Fix a chunk of wall missing and replaced with sky near the gazebo
    gpSectors[29].ceilingpic = R_GetOverrideFlatNum(R_FlatNumForName("GRASS03"));

    // Fix not being able to get into a switch area because the bars don't allow enough clearance once opened
    gpSectors[93].ceilingheight += 24 * FRACUNIT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP83 'N-M-E' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_NME() noexcept {
    // Remove sky walls around some pillars that shouldn't have them
    for (int32_t lineIdx = 890; lineIdx <= 929; ++lineIdx) {
        gpLines[lineIdx].flags |= ML_VOID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP89 'Bunker' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Bunker() noexcept {
    // Fix a 'P_PlayerInSpecialSector: unknown special 235' error when stepping onto a teleporter pad
    gpSectors[176].special = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP91 'The Sewers' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheSewers() noexcept {
    // Fix sky walls that shouldn't be around the floating lights
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308,     // Outer edges
        1434, 1435, 1436, 1437,                             // Top left light
        1438, 1439, 1440, 1441,                             // Top right light
        1426, 1427, 1428, 1429,                             // Center light
        1430, 1431, 1432, 1433,                             // Bottom left light
        1422, 1423, 1424, 1425                              // Bottom right light
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP93 'Cyberden' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Cyberden() noexcept {
    // Correct the target secret level number from '24' (map number within the episode) to '94' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 94; },
        729, 732, 734, 735
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues in MAP94 'Go 2 It' for the GEC Master Edition (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Go2It() noexcept {
    // Correct the level to return to from '10' (map number within the episode) to '80' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 80; },
        880, 881, 882, 883
    );

    // Fix a ceiling with the same height as a nearby sky - give it a small lip
    gpSectors[69].ceilingheight -= 2 * FRACUNIT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the map fixes to apply
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatches[] = {
    // Doom
    {  56435, 0x7921ADB466CEE45E, 0x4476F208866BF8A7, applyCommonMapPatches },          // MAP01
    { 119369, 0x4ED7CD6367900B52, 0x9609E85DB101DC09, applyCommonMapPatches },          // MAP02
    { 110284, 0x9BFF3A037128D1CA, 0x12F445D3F9B8BAC6, applyCommonMapPatches },          // MAP03
    {  92341, 0x1D79B5BDE5426081, 0x4E9413A01EAF4B4A, patchMap_CommandControl },        // MAP04
    {  89865, 0x0A8ACFFC833D6E36, 0x070A7A5CDDEE1CE0, applyCommonMapPatches },          // MAP05
    { 124094, 0x2097E86807523FF3, 0xA2F0C52632B12372, applyCommonMapPatches },          // MAP06
    { 108814, 0xD89ECAA4823454FD, 0xC7C178FA280CA569, applyCommonMapPatches },          // MAP07
    {  51882, 0x94BC7E609E1AC29A, 0xC1B6D482725C2C34, applyCommonMapPatches },          // MAP08
    {  47025, 0x492736BF0840ED38, 0x92A3AA841280B742, applyCommonMapPatches },          // MAP09
    {  97045, 0x48FFA0D005CB2DDA, 0x2631E9D5AB867200, applyCommonMapPatches },          // MAP10
    {  75368, 0x6D99C761DE799820, 0xAEDB0E4CA9441431, applyCommonMapPatches },          // MAP11
    { 119221, 0xB0E9622905A41337, 0xED94BA27D70017BF, applyCommonMapPatches },          // MAP12
    {  83505, 0x8635E6DB6360B27C, 0xD5835A25E276A0C4, applyCommonMapPatches },          // MAP13
    {  85802, 0x556287C93A6396F9, 0xC019D5F66797A596, applyCommonMapPatches },          // MAP14
    {  83539, 0xFDA28FD54C7E9A92, 0xE7F93F0E3C5C1D7F, applyCommonMapPatches },          // MAP15
    {  27956, 0x39B94C1CF5E19EB0, 0xE0A691816A8C166A, applyCommonMapPatches },          // MAP16
    {  56466, 0x4F240435B71CA6CA, 0xFA106C3EC5548BF0, applyCommonMapPatches },          // MAP17
    {  71253, 0x0541C17B11B2DC05, 0x577D152A01E48073, applyCommonMapPatches },          // MAP18
    {  75515, 0xFE716B01FE414A2A, 0xA3A7AFA1956DF697, patchMap_HouseOfPain },           // MAP19
    { 143483, 0x36A01960BAD36249, 0x2BC3BF03E0ED6D64, applyCommonMapPatches },          // MAP20
    {  86538, 0x403A02FD929949E5, 0xB4185CB43CEA9B46, applyCommonMapPatches },          // MAP21
    { 109754, 0x1E3E66448FE6645C, 0x3DCA2CA78FC862F3, patchMap_Limbo },                 // MAP22
    {  32935, 0x55A24A4ED4053AC3, 0x636CDB24CE519EF8, applyCommonMapPatches },          // MAP23
    {  52915, 0xA8CCE876F52671B2, 0xDA2BB82C5D03383C, applyCommonMapPatches },          // MAP24
    {  72352, 0x255311EE3A46B4F4, 0x30E325760C3C0D55, applyCommonMapPatches },          // MAP25
    { 111520, 0x85B038429CCD933B, 0x8488BBE9B15A5F8C, applyCommonMapPatches },          // MAP26
    {  82104, 0x52B9EDF6AA65FD8C, 0x3D965AFD07455BA6, applyCommonMapPatches },          // MAP27
    { 146652, 0x1C5AD3B2CC520748, 0x79223365451D6965, applyCommonMapPatches },          // MAP28
    { 163970, 0x85E5F59863FC567A, 0x825E1D627586324B, applyCommonMapPatches },          // MAP29
    { 146600, 0x0776A66BD2962C70, 0xEA25B44BFB2863F0, applyCommonMapPatches },          // MAP30
    {  46210, 0x41EA6956972B2510, 0xE4760C46A4BBD40D, patchMap_Entryway     },          // MAP31
    {  63255, 0x787980722B2A3ABF, 0xDA758F7A7236BAD9, applyCommonMapPatches },          // MAP32
    {  71907, 0x9354072B9094E9BE, 0xFDA856CDE67680DC, applyCommonMapPatches },          // MAP33
    {  67614, 0xE36C70A633E0AE7D, 0x9223DF3ADFDF8808, applyCommonMapPatches },          // MAP34
    { 114123, 0x52229ABCD304D8BA, 0x6EAEA8DB75133B5A, applyCommonMapPatches },          // MAP35
    { 129248, 0xE2245D687CCABC7C, 0x01497DF00B763463, applyCommonMapPatches },          // MAP36
    {  26682, 0x2B0A8D80B5411593, 0x3A427EE05B7353F6, applyCommonMapPatches },          // MAP37
    {  82063, 0xBFEDBDE9F8B8CCE2, 0x78D6E2C3A9AB74AB, applyCommonMapPatches },          // MAP38
    {  91388, 0x22B7D106F531FB4E, 0xFE3FAB276C892BD4, applyCommonMapPatches },          // MAP39
    { 130676, 0xD84B13024E326B64, 0x548472C7F8B24A27, applyCommonMapPatches },          // MAP40
    { 116024, 0x59800E5259D02FD8, 0x28EB273CFC8E41CC, applyCommonMapPatches },          // MAP41
    { 109934, 0x7E22F4311F3955D5, 0x16E918F5C11AD780, applyCommonMapPatches },          // MAP42
    { 192997, 0x7B86B9C35B754883, 0xD5F5CE44AB12898D, applyCommonMapPatches },          // MAP43
    { 110145, 0xE296122ADE38AB74, 0x13505BF841234D4C, applyCommonMapPatches },          // MAP44
    { 158462, 0x37D6A1335F058A41, 0xA82656A6FDEB132B, applyCommonMapPatches },          // MAP45
    { 105883, 0x0CA0922874005BC1, 0x37173A0C68F8FA6A, patchMap_TheCourtyard },          // MAP46
    { 186755, 0x73E10EF08AE21FD5, 0x8115F467FE2CD3CA, patchMap_TheCitadel   },          // MAP47
    {  54866, 0xF41440631C2B6FB2, 0x728E55510D5AE858, applyCommonMapPatches },          // MAP48
    {  74303, 0x522256004AD8E073, 0x2C190C108C98B31D, applyCommonMapPatches },          // MAP49
    {  64540, 0x47EA67DBBA5F33DC, 0x2280784D842FECC1, applyCommonMapPatches },          // MAP50
    { 106555, 0x9FCBB09C2A8C8B67, 0xF00080B9655646C8, applyCommonMapPatches },          // MAP51
    { 117839, 0x67138B444A196EC4, 0x229285E95F31ADE4, applyCommonMapPatches },          // MAP52
    { 131947, 0xC966739D25AC3FFD, 0xB7CDA8E3CF9A5186, applyCommonMapPatches },          // MAP53
    {  45962, 0x27D515F2A59962E3, 0x3E35ABAD09E87EA1, applyCommonMapPatches },          // MAP54
    {  19237, 0xB5116FF7C0CBCF38, 0x7C6C9E29F2EA963B, applyCommonMapPatches },          // MAP55
    {  85042, 0x98C00035EA735BF3, 0x36C5C0BA592334C9, applyCommonMapPatches },          // MAP56
    {  58333, 0xBBE3159AAEE4F03D, 0x086F778E5A08DBAD, applyCommonMapPatches },          // MAP57
    { 194653, 0x158C832CA1D1C539, 0x9C4ED57B29C13E66, applyCommonMapPatches },          // MAP58
    {  79441, 0x0ECE269F1AA74445, 0x0B254FDF53895D4F, applyCommonMapPatches },          // MAP59
    // Final Doom
    { 149065, 0xEB9558B4F55AB3B1, 0xEFF704814B4411D6, applyCommonMapPatches },          // MAP01
    { 117111, 0xADAA213B6086EE11, 0x978D2CC01B168F5D, applyCommonMapPatches },          // MAP02 (NTSC)
    { 117187, 0xAA3783F676D9B0CE, 0x9DE393E81F9FF509, applyCommonMapPatches },          // MAP02 (PAL, why different?)
    { 135628, 0xC80C9E89D5679E63, 0x7AD86A771493F84E, applyCommonMapPatches },          // MAP03
    {  88648, 0xCA130E217D79594D, 0x1241B18E3E7D8110, applyCommonMapPatches },          // MAP04
    { 132897, 0x2E8E3492E704154D, 0x0DE2F0470D62DDB8, applyCommonMapPatches },          // MAP05
    {  88004, 0xC12D7BD6D414250C, 0xF981017C0C8ADF20, applyCommonMapPatches },          // MAP06
    { 165920, 0x903B721BA84B1FFD, 0xCED86BF62E5CE0BE, applyCommonMapPatches },          // MAP07
    { 151747, 0x93EA3A4DE9DA978B, 0x3D27F6255CA0B9CC, applyCommonMapPatches },          // MAP08
    { 102104, 0x1504DC20E04BE8F1, 0x3A63FD22BC9C0D8C, applyCommonMapPatches },          // MAP09
    { 139820, 0x5EDEF8B2A51779E8, 0x8D1314A4F889EFCC, applyCommonMapPatches },          // MAP10
    {  96211, 0x42B2A3CE9B37CA2A, 0x72D00C8E1681AEB4, applyCommonMapPatches },          // MAP11
    { 106776, 0xAD3AADE890018818, 0x7D70AC984E7211CC, applyCommonMapPatches },          // MAP12
    { 152855, 0xD4905C759C1713E1, 0x1B78CCD5275A40EB, applyCommonMapPatches },          // MAP13
    {  54706, 0x979F686C4297312E, 0xB9EA33C07E20F4E3, applyCommonMapPatches },          // MAP14
    {  77891, 0x20F93855131B2C1A, 0xD98E0D6C4EAEC765, applyCommonMapPatches },          // MAP15
    { 156972, 0xC4DF66BEDEE0E1C4, 0xFB56E82FA017FD9D, applyCommonMapPatches },          // MAP16
    { 179622, 0x97DFE2C07BE92D3C, 0xEC29BA71305623B3, applyCommonMapPatches },          // MAP17
    { 131823, 0xADD51543E9578AB7, 0xA3E479551A015464, applyCommonMapPatches },          // MAP18
    { 177868, 0x5BDC5BC7E62822C1, 0x3F374AD0091C79F1, applyCommonMapPatches },          // MAP19
    { 105404, 0x5849A9F98647AF13, 0x59C891E67F19FC69, applyCommonMapPatches },          // MAP20
    { 162561, 0x5BA4490CA5C13E9A, 0x23D505C31AF4CADF, applyCommonMapPatches },          // MAP21
    {  96826, 0x9B6446A94907229A, 0x6DC9F5EDDB9D4F2D, applyCommonMapPatches },          // MAP22
    { 167847, 0x3BC3E6570C2D06B3, 0x18756B0D2C98BE86, patchMap_Ballistyx },             // MAP23
    { 121920, 0x445D7FDA25066B71, 0xAC3893B22E188D4D, applyCommonMapPatches },          // MAP24
    { 113719, 0xFBA63EF7487AB574, 0xE21B77623A0DE2AA, applyCommonMapPatches },          // MAP25
    { 127601, 0x1008C54A53E8B33E, 0x8E35C49173174DCD, applyCommonMapPatches },          // MAP26
    { 113829, 0x25A6925BB713C346, 0x7AF7C07603DEA325, applyCommonMapPatches },          // MAP27
    { 141807, 0x3461BD1E919965AB, 0x07C36C7B648205F6, applyCommonMapPatches },          // MAP28
    { 107736, 0xD9789CCEA024CCCC, 0x61CCB6C421B65C47, applyCommonMapPatches },          // MAP29 (NTSC)
    { 107736, 0x0599BE06504C6FAD, 0x1DCB1C8AD6410764, applyCommonMapPatches },          // MAP29 (PAL, why different?)
    { 110131, 0x2C157281E504283E, 0x914845A33B9F0503, applyCommonMapPatches },          // MAP30
    // GEC Master Edition (Beta 3)
    {  62640, 0xA68435B9FEA82A74, 0xB539F9DEF881149D, gecMEBeta3_fixWrongREDROK01 },    // MAP05
    { 129546, 0x3B2546BA128349AE, 0xC4D9982A6D4C27DD, patchMap_AgainstTheeWickedly },   // MAP07
    {  96628, 0x808AD04C8D43EEC8, 0x73B0859F1115F292, patchMap_Betray },                // MAP11
    {  87975, 0x7CF1F4CF0C3427C5, 0x50A8701B4A994752, gecMEBeta3_fixWrongREDROK01 },    // MAP14
    {  87074, 0x8CE5FFE1D040C140, 0x4E89A7383999004F, patchMap_TombOfMalevolence  },    // MAP24
    { 166962, 0x9F83C36FCCE657BD, 0x8E17C9FE4D19BFED, patchMap_BaphometDemense },       // MAP30
    { 172689, 0xB8354D5A39E9F37A, 0x013E5A66B42A71F9, patchMap_TrappedOnTitan },        // MAP32
    { 152959, 0xBD44DD87CE623522, 0x57AEC452C8FB2CF7, patchMap_BlackTower },            // MAP34
    { 123320, 0x87ED0BB125C317ED, 0x47E362A5D5258526, patchMap_TEETH },                 // MAP36
    { 121126, 0x364CF475759D4689, 0x905D77EC9FE7AAD1, patchMap_TheImageOfEvil },        // MAP49
    {  11033, 0x630B968605A4F759, 0x8A9D02099C77ECD1, patchMap_BadDream },              // MAP50
    {  78120, 0x150CD634F190B42E, 0x5535698EAC736972, patchMap_Redemption },            // MAP57
    { 157892, 0x4A5802B317C04C27, 0xB7E3604C08F07666, patchMap_StorageFacility },       // MAP58
    { 123947, 0x3805BA7B953B38B3, 0x2F9DCB10186E7DB3, patchMap_DeadZone },              // MAP60
    { 179279, 0x8C446FE7E1528EBE, 0x34B40119DCDABD64, patchMap_ShippingRespawning },    // MAP62
    { 186321, 0xDDAABED7BB4FF8B9, 0xEF0F5277E3C96A4C, patchMap_MountPain },             // MAP67
    { 164989, 0x413FE3E56F2C2453, 0x9E047C4ECCB4FEA7, patchMap_RiverStyx },             // MAP68
    { 167156, 0xE21C586BF2242082, 0xFA5D9C91DB288B5E, patchMap_Pharaoh },               // MAP69
    { 120117, 0x475B5271367FB4C1, 0xF1D3C564A0145DA6, patchMap_Caribbean },             // MAP70
    {  95176, 0xA5DA6BC16E6BF2C2, 0xD8A4986E39B4EB00, patchMap_TheTwilight },           // MAP79
    {  99229, 0xE3B921D12019C298, 0x93AD211E85EC33D7, patchMap_TheOmen },               // MAP80
    {  98162, 0xC2584F9BA85B42A5, 0xAE958A8D617754FC, patchMap_NME },                   // MAP83
    { 138392, 0x9DA7F8CC0F0CB11F, 0xE3383B410F789D07, patchMap_Bunker },                // MAP89
    { 144965, 0x11F026E6D12E8EC4, 0x045807938CC41667, patchMap_TheSewers },             // MAP91
    { 106517, 0x2EA30BD21131045A, 0x524AD93A1261BC8F, patchMap_Cyberden },              // MAP93
    {  97336, 0x7DBB35F7DE4902C0, 0x93F71E0D6A338D71, patchMap_Go2It },                 // MAP94
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to apply patches for the current map
//------------------------------------------------------------------------------------------------------------------------------------------
void applyPatches() noexcept {
    // Cache these globals locally since they will be compared a lot
    const int32_t mapSize = MapHash::gDataSize;
    const uint64_t md5Word1 = MapHash::gWord1;
    const uint64_t md5Word2 = MapHash::gWord2;

    // Check to see which patch applies (if any)
    for (const PatchDef& patch : gPatches) {
        ASSERT(patch.patcherFunc);

        // Wrong map data size?
        if (mapSize != patch.mapSize)
            continue;

        // Wrong MD5 hash for all the map data?
        if ((md5Word1 != patch.md5Word1) || (md5Word2 != patch.md5Word2))
            continue;

        // Match, apply the patch and abort the search
        patch.patcherFunc();
        break;
    }
}

END_NAMESPACE(MapPatcher)
