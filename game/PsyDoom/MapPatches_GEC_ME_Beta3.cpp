//------------------------------------------------------------------------------------------------------------------------------------------
// A module containing map patches to apply to the 'GEC Master Edition' (Beta 3)
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatches.h"

#include "Doom/Game/info.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Renderer/r_data.h"

BEGIN_NAMESPACE(MapPatches)

//------------------------------------------------------------------------------------------------------------------------------------------
// Fixes the wrong version of the 'REDROK01' texture being used for the 'Doom' episode maps.
// Switches the 'Final Doom' style of this texture with the original 'Doom' version.
//------------------------------------------------------------------------------------------------------------------------------------------
static void fixWrongREDROK01() noexcept {
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
// Fix issues for MAP02: Forgotten Sewers
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ForgottenSewers() noexcept {
    // Fix automap lines that should be visible
    unhideLinedefs(969, 970, 971, 972, 973);

    // Fix a zero brightness sector
    gpSectors[153].lightlevel = 128;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP07: Against Thee Wickedly
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_AgainstTheeWickedly() noexcept {
    // Fix a sector which is marked as damaging that shouldn't be
    gpSectors[95].special = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP08: And Hell Followed
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_AndHellFollowed() noexcept {
    // Fix slime sectors that are supposed to be damaging but aren't
    modifySectors(
        [](sector_t& sector) { sector.special = 7; },   // Damage -2% or -5% health
        213, 224, 225, 226, 227, 228
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP11: Betray
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Betray() noexcept {
    // Fix sky walls rendering where they should not
    addVoidFlagToLinedefs(
        495, 496, 497, 498, 499, 500, 501, 502, 673, 674, 675, 676, 677, 678, 679, 680,     // Two teleporter pillars
        356, 357, 358, 359, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606                // Cross
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP13: The Chasm
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheChasm() noexcept {
    // Fix missing textures when a floor raises
    modifyLinedefs(
        [](line_t& line) { gpSides[line.sidenum[1]].bottomtexture = R_TextureNumForName("SKIN08"); },
        211, 212, 213, 214
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP16: Icon Of Sin
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_IconOfSin() noexcept {
    // Fix missing lower step textures in the exit tunnels
    modifyLinedefs(
        [](line_t& line) { gpSides[line.sidenum[1]].bottomtexture = R_TextureNumForName("BRICK09"); },
        261, 287, 293,      // Left tunnel
        366, 367, 368 
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP21: Vivisection
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Vivisection() noexcept {
    // Fix a missing step texture
    gpSides[gpLines[1134].sidenum[1]].bottomtexture = R_TextureNumForName("ROCK09");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP22: Inferno of Blood
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_InfernoOfBlood() noexcept {
    // Fix very mismatched textures used: red brick instead of brown rock
    modifyLinedefs(
        [](line_t& line) { gpSides[line.sidenum[0]].midtexture = R_TextureNumForName("ROCK03"); },
        759, 762
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP23: Barons Banquet
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BaronsBanquet() noexcept {
    // Fix a secret platform that won't lower: the special that triggers it is set to raise but it's already raised!
    // Fix by adding another trigger that will lower the platform instead.
    gpSectors[80].tag = 9;

    modifyLinedefs(
        [](line_t& line) {
            line.special = 36;  // W1 Floor Lower to Lowest Floor
            line.tag = 9;
        },
        565, 566, 567
    );

    // Fix an impaled corpse being translucent and rendered like a nightmare spectre
    for (mobj_t* pMobj = gpSectors[75].thinglist; pMobj != nullptr; pMobj = pMobj->snext) {
        if (pMobj->type == MT_MISC74) {
            pMobj->flags &= ~MF_ALL_BLEND_FLAGS;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP24: Tomb Of Malevolence
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TombOfMalevolence() noexcept {
    // Set the next level to the start map of the next episode instead of '99', so the next episode is correctly selected on the main menu
    modifyLinedefs(
        [](line_t& line) { line.tag = 31; },
        661, 662, 663, 664
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP26: Fear
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Fear() noexcept {
    // Fix a small crate having the wrong floor texture
    gpSectors[95].floorpic = R_FlatNumForName("GRAY01");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP30: Baphomet Demense
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BaphometDemense() noexcept {
    fixWrongREDROK01();

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
// Fix issues for MAP31: Titan Manor
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TitanManor() noexcept {
    // Make the main progression path more obvious and not like a well hidden secret.
    // Change the texture of the hidden door in the fireplace and unmark it as a secret to make it stand out.
    modifyLinedefs(
        [](line_t& line) {
            line.flags &= ~ML_SECRET;
            gpSides[line.sidenum[0]].toptexture = R_TextureNumForName("MARBLE05");
        },
        482
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP32: Trapped On Titan
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
    gpSides[gpLines[212].sidenum[0]].bottomtexture = R_TextureNumForName("SUPPORT5");
    gpSides[gpLines[212].sidenum[1]].bottomtexture = R_TextureNumForName("DOORTRAK");

    // Fix some steps being too high to climb up in the wooden building
    gpSectors[194].floorheight += 2 * FRACUNIT;
    gpSectors[195].floorheight += 4 * FRACUNIT;

    // Fix glitches when looking over one of the buildings outside: raise its sky level to be the same as the other buildings.
    // This also makes the building more faithful to the PC original.
    gpSectors[107].ceilingheight = 0;

    // Fix the area beyond the lift not lowering after the switch outside is pressed.
    // An Imp stuck in the ceiling prevents this. Fix by raising the ceiling so it is not stuck.
    gpSectors[270].ceilingheight += 64 * FRACUNIT;

    // Fix being able to get stuck in the area with the rocket launcher, in between its pedestal and some walls.
    // Lower the pedestal once it is reached.
    gpSectors[43].tag = 5;

    modifyLinedefs(
        [](line_t& line) {
            line.special = 36;  // W1 Floor Lower to 8 above Highest Floor
            line.tag = 5;
        },
        162, 163, 164
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP34: Black Tower
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BlackTower() noexcept {
    // Fix sky walls displaying over the pillars supporting a covering by the brown sludge
    addVoidFlagToLinedefs(629, 627, 939, 624, 626, 704, 623, 622, 693, 632, 633, 680, 634, 1081, 834, 636, 637, 985);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP36: TEETH
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TEETH() noexcept {
    // Correct the target secret level number from '20' (map number within the episode) to '50' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 50; },
        304
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP49: The Image of Evil
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheImageOfEvil() noexcept {
    // Set the next level to the start map of the next episode instead of '99', so the next episode is correctly selected on the main menu.
    modifyLinedefs(
        [](line_t& line) { line.tag = 51; },
        929, 936, 937, 938
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP50: Bad Dream
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_BadDream() noexcept {
    // Correct the level to return to from '7' (map number within the episode) to '37' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 37; },
        83
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP53: Open Season
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_OpenSeason() noexcept {
    // Fix a missing texture on a lift side at the start of the map
    gpSides[gpLines[749].sidenum[0]].bottomtexture = R_TextureNumForName("SUPPORT3");

    // Fix not being able to raise the starting platform if manually activating it just before crossing a line to raise it.
    // Use the new repeatable version of the 'raise' action provided by PsyDoom:
    gpLines[763].special = 128;

    // Fix confusion over the color of a keycard where a yellow key is placed in a red sector.
    // Shift the sector color more towards yellow/orange to avoid confusion, and alter the light level to make it blend better with nearby reds.
    modifySectors(
        [](sector_t& sector) {
            sector.colorid = 45;
            sector.lightlevel = 160;
        },
        43, 154, 161
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP57: Redemption
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Redemption() noexcept {
    // Fix a missing texture on a step side after it lowers
    gpSides[gpLines[597].sidenum[1]].bottomtexture = R_TextureNumForName("METAL03");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP58: Storage Facility
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
// Fix issues for MAP60: Dead Zone
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
// Fix issues for MAP61: Mill
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Mill() noexcept {
    // Fix a missing step texture after lowering some walls near the end of the level
    gpSides[gpLines[978].sidenum[1]].bottomtexture = R_TextureNumForName("BRICK19");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP62: Shipping Respawning
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ShippingRespawning() noexcept {
    // Fix some windows not showing any geometry past them due to a small completely closed sector in the middle - treat it as a void:
    addVoidFlagToLinedefs(5, 8, 1189, 1568);

    // Fix sky walls being rendered for some pillars outside that shouldn't have them
    addVoidFlagToLinedefs(20, 21, 22, 23, 14, 24, 25, 26, 16, 27, 28, 29, 18, 32, 31, 30);

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
    
    // Fix an automap line not rendering at a computer console
    unhideLinedefs(982);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP67: Mount Pain
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_MountPain() noexcept {
    // Fix being able to see bits of geometry that shouldn't be seen through some outside windows.
    // Force the fully closed sector containing the sky to render, hence forcing a skywall to render too. Also remove its floor.
    addVoidFlagToLinedefs(1360, 1412);
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
// Fix issues for MAP68: River Styx
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_RiverStyx() noexcept {
    // Set the next level to the start map of the next episode instead of '99', so the next episode is correctly selected on the main menu.
    modifyLinedefs(
        [](line_t& line) { line.tag = 71; },
        1840
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP69: Pharaoh
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
    addVoidFlagToLinedefs(890, 910, 911, 912, 906, 907, 908, 909);

    // Remove some skywalls around the ship near the exit area that shouldn't be there
    addVoidFlagToLinedefs(1103, 293, 1091, 746, 1393);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP70: Caribbean
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Caribbean() noexcept {
    // Correct the level to return to from '11' (map number within the episode) to '61' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 61; },
        662
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP73: Caughtyard
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Caughtyard() noexcept {
    // Fix being able to climb over the fort walls: the step is 24 units tall and needs to be 25 as it was in Plutonia originally
    gpSectors[45].floorheight += 1 * FRACUNIT;

    // Fix two missing textures at the entraces to one of the huts
    modifyLinedefs(
        [](line_t& line) { gpSides[line.sidenum[0]].bottomtexture = R_TextureNumForName("WOOD06"); },
        124, 136
    );

    // Fix a monster teleporter not working if the initial teleporter attempt fails.
    // Change the type of teleporter from 'once' to 'repeatable'.
    gpLines[359].special = 126;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP76: Speed
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Speed() noexcept {
    // Fix the player being able to see past the end of the sky in various places: raise outer walls bordering the sky
    modifySectors(
        [](sector_t& sector) {
            sector.floorheight += 16 * FRACUNIT;
            sector.ceilingheight += 16 * FRACUNIT;
        },
        130, 156
    );
    
    // Fix floors of an outer sky sometimes glitching in and out for one of the windows: the floor shouldn't be drawn
    gpSectors[12].floorpic = -1;
    
    // Fix some geometry showing through the sky.
    // Make the outer edge of sky walls void so that the one-sided sky walls draw behind it.
    addVoidFlagToLinedefs(1123, 1124, 1126, 1127, 1128, 1129, 1136, 1137, 1138);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP79: The Twilight
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheTwilight() noexcept {
    // Correct the target secret level number from '23' (map number within the episode) to '93' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 93; },
        120, 309, 911, 919
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP80: The Omen
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheOmen() noexcept {
    // Fix a chunk of wall missing and replaced with sky near the gazebo
    gpSectors[29].ceilingpic = R_GetOverrideFlatNum(R_FlatNumForName("GRASS03"));

    // Fix not being able to get into a switch area because the bars don't allow enough clearance once opened
    gpSectors[93].ceilingheight += 24 * FRACUNIT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP82: Neurosphere
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Neurosphere() noexcept {
    // Fix a teleporter not working, trapping the player in a blood pit.
    // For some reason a teleport destination marker seems to be missing - re-add it:
    mobj_t* const pTeleDest = P_SpawnMobj(1024 * FRACUNIT, -2112 * FRACUNIT, INT32_MIN, MT_TELEPORTMAN);
    pTeleDest->angle = ANG180 + ANG45;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP83: N-M-E
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_NME() noexcept {
    // Remove sky walls around some pillars that shouldn't have them
    for (int32_t lineIdx = 890; lineIdx <= 929; ++lineIdx) {
        gpLines[lineIdx].flags |= ML_VOID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP85: Impossible Mission
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_ImpossibleMission() noexcept {
    // Fix ceilings that are level with the sky - lower them a little to create a lip
    modifySectors(
        [](sector_t& sector) {
            sector.ceilingheight -= 22 * FRACUNIT;
        },
        145, 150, 152
    );

    // Fix the player being able to fall into pits that cannot be climbed out of.
    // Add triggers to raise the pits if the player falls in.
    modifyLinedefs(
        [](line_t& line) {
            line.special = 119;     // W1 Floor Raise to Next Higher Floor
            line.tag = 8;
        },
        610, 582, 587, 591
    );

    gpSectors[58].floorheight = 19 * FRACUNIT;  // Raise the pits the full way the first time: don't create a lip and then remove it later...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP89: Bunker
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Bunker() noexcept {
    // Fix a 'P_PlayerInSpecialSector: unknown special 235' error when stepping onto a teleporter pad
    gpSectors[176].special = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP91: The Sewers
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheSewers() noexcept {
    // Fix sky walls that shouldn't be around the floating lights
    addVoidFlagToLinedefs(
        1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308,     // Outer edges
        1434, 1435, 1436, 1437,                             // Top left light
        1438, 1439, 1440, 1441,                             // Top right light
        1426, 1427, 1428, 1429,                             // Center light
        1430, 1431, 1432, 1433,                             // Bottom left light
        1422, 1423, 1424, 1425                              // Bottom right light
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP92: Odyssey of Noises
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_OdysseyOfNoises() noexcept {
    // Fix automap lines that shouldn't be hidden
    unhideLinedefs(187, 188, 202, 203);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP93: Cyberden
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Cyberden() noexcept {
    // Correct the target secret level number from '24' (map number within the episode) to '94' (global map number)
    modifyLinedefs(
        [](line_t& line) { line.tag = 94; },
        729, 732, 734, 735
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP94: Go 2 It
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
// All of the map patches for this game type
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatchArray_GEC_ME_Beta3[] = {
    { 120087, 0x6CF71CAD6729761C, 0xB9AADBB88CC49757, patchMap_ForgottenSewers },           // MAP02
    {  62640, 0xA68435B9FEA82A74, 0xB539F9DEF881149D, fixWrongREDROK01 },                   // MAP05
    { 129546, 0x3B2546BA128349AE, 0xC4D9982A6D4C27DD, patchMap_AgainstTheeWickedly },       // MAP07
    { 141963, 0x0505711A9F4FB230, 0x86BDE1AB556902AC, patchMap_AndHellFollowed },           // MAP08
    {  96628, 0x808AD04C8D43EEC8, 0x73B0859F1115F292, patchMap_Betray },                    // MAP11
    { 125944, 0x2B2CE1174955191F, 0xDBDC93F792A48D6E, patchMap_TheChasm },                  // MAP13
    {  87975, 0x7CF1F4CF0C3427C5, 0x50A8701B4A994752, fixWrongREDROK01 },                   // MAP14
    {  51103, 0x01D1F698C89E0228, 0xF59370050D1164D0, patchMap_IconOfSin },                 // MAP16
    { 145156, 0x51134C2A5A51C83F, 0x643A88CF3906D95B, patchMap_Vivisection },               // MAP21
    { 166908, 0xA67A3D273E02C0D1, 0xFAF4DC1387C0FFAA, patchMap_InfernoOfBlood },            // MAP22
    { 179930, 0x488228856FD3DD7E, 0xF579F43D9E39ACB2, patchMap_BaronsBanquet },             // MAP23
    {  87074, 0x8CE5FFE1D040C140, 0x4E89A7383999004F, patchMap_TombOfMalevolence },         // MAP24
    { 113315, 0x843507387CE8BCBF, 0x5B4EB9EE56E95384, patchMap_Fear },                      // MAP26
    { 166962, 0x9F83C36FCCE657BD, 0x8E17C9FE4D19BFED, patchMap_BaphometDemense },           // MAP30
    { 119562, 0xD994CF0954312F96, 0xC15EC0A72C4C4FB4, patchMap_TitanManor },                // MAP31
    { 172689, 0xB8354D5A39E9F37A, 0x013E5A66B42A71F9, patchMap_TrappedOnTitan },            // MAP32
    { 152959, 0xBD44DD87CE623522, 0x57AEC452C8FB2CF7, patchMap_BlackTower },                // MAP34
    { 123320, 0x87ED0BB125C317ED, 0x47E362A5D5258526, patchMap_TEETH },                     // MAP36
    { 121126, 0x364CF475759D4689, 0x905D77EC9FE7AAD1, patchMap_TheImageOfEvil },            // MAP49
    {  11033, 0x630B968605A4F759, 0x8A9D02099C77ECD1, patchMap_BadDream },                  // MAP50
    {  92129, 0x508E355B9AC659BE, 0xA4E8B1A1202BB672, patchMap_OpenSeason },                // MAP53
    {  78120, 0x150CD634F190B42E, 0x5535698EAC736972, patchMap_Redemption },                // MAP57
    { 157892, 0x4A5802B317C04C27, 0xB7E3604C08F07666, patchMap_StorageFacility },           // MAP58
    { 123947, 0x3805BA7B953B38B3, 0x2F9DCB10186E7DB3, patchMap_DeadZone },                  // MAP60
    { 208388, 0x576180A7759F98D2, 0x76B5E5021B925739, patchMap_Mill },                      // MAP61
    { 179279, 0x8C446FE7E1528EBE, 0x34B40119DCDABD64, patchMap_ShippingRespawning },        // MAP62
    { 186321, 0xDDAABED7BB4FF8B9, 0xEF0F5277E3C96A4C, patchMap_MountPain },                 // MAP67
    { 164989, 0x413FE3E56F2C2453, 0x9E047C4ECCB4FEA7, patchMap_RiverStyx },                 // MAP68
    { 167156, 0xE21C586BF2242082, 0xFA5D9C91DB288B5E, patchMap_Pharaoh },                   // MAP69
    { 120117, 0x475B5271367FB4C1, 0xF1D3C564A0145DA6, patchMap_Caribbean },                 // MAP70
    {  48509, 0x3C16F377EEB066E6, 0x728CF548925EBC4E, patchMap_Caughtyard },                // MAP73
    { 115075, 0x6A5BA4600D51FA60, 0x5E2B0CBCFBC0A065, patchMap_Speed },                     // MAP76
    {  95176, 0xA5DA6BC16E6BF2C2, 0xD8A4986E39B4EB00, patchMap_TheTwilight },               // MAP79
    {  99229, 0xE3B921D12019C298, 0x93AD211E85EC33D7, patchMap_TheOmen },                   // MAP80
    { 142221, 0x86D4E6672DFAA384, 0x6370FE4DE9BE0DCE, patchMap_Neurosphere },               // MAP82
    {  98162, 0xC2584F9BA85B42A5, 0xAE958A8D617754FC, patchMap_NME },                       // MAP83
    { 154357, 0xBD3C80483B84E18A, 0x296AFAF8E748077E, patchMap_ImpossibleMission },         // MAP85
    { 138392, 0x9DA7F8CC0F0CB11F, 0xE3383B410F789D07, patchMap_Bunker },                    // MAP89
    { 144965, 0x11F026E6D12E8EC4, 0x045807938CC41667, patchMap_TheSewers },                 // MAP91
    { 167748, 0xBE395077BE0BE3EF, 0xD419B6C4BC527803, patchMap_OdysseyOfNoises },           // MAP92
    { 106517, 0x2EA30BD21131045A, 0x524AD93A1261BC8F, patchMap_Cyberden },                  // MAP93
    {  97336, 0x7DBB35F7DE4902C0, 0x93F71E0D6A338D71, patchMap_Go2It },                     // MAP94
};

const PatchList gPatches_GEC_ME_Beta3 = { gPatchArray_GEC_ME_Beta3, C_ARRAY_SIZE(gPatchArray_GEC_ME_Beta3) };

END_NAMESPACE(MapPatches)
