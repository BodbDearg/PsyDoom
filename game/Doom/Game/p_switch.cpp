#include "p_switch.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "doomdata.h"
#include "p_ceiling.h"
#include "p_doors.h"
#include "p_floor.h"
#include "p_lights.h"
#include "p_plats.h"
#include "p_setup.h"
#include "p_spec.h"

// Defines the two textures used by a switch
struct switchlist_t {
    char    name1[9];
    char    name2[9];
};

// All of the switch textures in the game
static const switchlist_t gAlphSwitchList[] = {
    { "SW1BMET",  "SW2BMET"  },
    { "SW1BRICK", "SW2BRICK" },
    { "SW1BRNZ",  "SW2BRNZ"  },
    { "SW1BROWN", "SW2BROWN" },
    { "SW1MARB",  "SW2MARB"  },
    { "SW1MET",   "SW2MET"   },
    { "SW1METAL", "SW2METAL" },
    { "SW1NEW02", "SW2NEW02" },
    { "SW1NEW03", "SW2NEW03" },
    { "SW1NEW04", "SW2NEW04" },
    { "SW1NEW05", "SW2NEW05" },
    { "SW1NEW06", "SW2NEW06" },
    { "SW1NEW10", "SW2NEW10" },
    { "SW1NEW11", "SW2NEW11" },
    { "SW1NEW13", "SW2NEW13" },
    { "SW1NEW14", "SW2NEW14" },
    { "SW1NEW20", "SW2NEW20" },
    { "SW1NEW25", "SW2NEW25" },
    { "SW1NEW26", "SW2NEW26" },
    { "SW1NEW28", "SW2NEW28" },
    { "SW1NEW29", "SW2NEW29" },
    { "SW1NEW30", "SW2NEW30" },
    { "SW1NEW31", "SW2NEW31" },
    { "SW1NEW32", "SW2NEW32" },
    { "SW1NEW33", "SW2NEW33" },
    { "SW1NEW34", "SW2NEW34" },
    { "SW1NEW35", "SW2NEW35" },
    { "SW1NEW36", "SW2NEW36" },
    { "SW1NEW39", "SW2NEW39" },
    { "SW1NEW40", "SW2NEW40" },
    { "SW1NEW41", "SW2NEW41" },
    { "SW1NEW42", "SW2NEW42" },
    { "SW1NEW45", "SW2NEW45" },
    { "SW1NEW46", "SW2NEW46" },
    { "SW1NEW47", "SW2NEW47" },
    { "SW1NEW51", "SW2NEW51" },
    { "SW1NEW57", "SW2NEW57" },
    { "SW1NEW60", "SW2NEW60" },
    { "SW1NEW63", "SW2NEW63" },
    { "SW1NEW65", "SW2NEW65" },
    { "SW1NEW66", "SW2NEW66" },
    { "SW1NEW68", "SW2NEW68" },
    { "SW1NEW69", "SW2NEW69" },
    { "SW1NEW70", "SW2NEW70" },
    { "SW1RED",   "SW2RED"   },
    { "SW1RUST",  "SW2RUST"  },
    { "SW1SKULL", "SW2SKULL" },
    { "SW1STAR",  "SW2STAR"  },
    { "SW1STEEL", "SW2STEEL" },
};

static constexpr int32_t NUM_SWITCHES   = C_ARRAY_SIZE(gAlphSwitchList);    // Number of switch types in the game
static constexpr int32_t BUTTONTIME     = 1 * TICRATE;                      // How long it takes for a switch to go back to it's original state (1 second)

// The list of currently active buttons/switches
button_t gButtonList[MAXBUTTONS];

// The 2 lumps for each switch texture in the game
static int32_t gSwitchList[NUM_SWITCHES * 2];

//------------------------------------------------------------------------------------------------------------------------------------------
// Caches textures for all active switches in the level.
// Must be done after 64 pixel wide wall textures have been cached in order to work.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_InitSwitchList() noexcept {
    int32_t* pSwitchLump = gSwitchList;

    for (int32_t switchIdx = 0; switchIdx < NUM_SWITCHES; ++switchIdx) {
        // Get both textures for the switch
        const int32_t tex1Lump = R_TextureNumForName(gAlphSwitchList[switchIdx].name1);
        const int32_t tex2Lump = R_TextureNumForName(gAlphSwitchList[switchIdx].name2);
        
        texture_t& tex1 = gpTextures[tex1Lump];
        texture_t& tex2 = gpTextures[tex2Lump];

        // Cache the other switch texture if one of the switch textures is loaded
        if ((tex1.texPageId) != 0 && (tex2.texPageId == 0)) {
            I_CacheTex(tex2);
        }

        if ((tex2.texPageId != 0) && (tex1.texPageId == 0)) {
            I_CacheTex(tex1);
        }
        
        // Save what lumps the switch uses
        pSwitchLump[0] = tex1Lump;
        pSwitchLump[1] = tex2Lump;
        pSwitchLump += 2;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Save the state of a switch (current texture) for later restoration after a specified delay.
// Used to implement buttons that switch back after a while.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_StartButton(line_t& line, const bwhere_e where, const int32_t texture, const int32_t countdownTime) noexcept {
    // Try to find a slot to save the state of the button in
    for (int32_t btnIdx = 0; btnIdx < MAXBUTTONS; ++btnIdx) {
        button_t& button = gButtonList[btnIdx];
        
        if (button.btimer == 0) {
            // Button slot is not in use: save the button state for later restoration and end search
            button.line = &line;
            button.where = where;
            button.btexture = texture;
            button.btimer = countdownTime;
            button.soundorg = (mobj_t*) &line.frontsector->soundorg;
            return;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flips the switch texture for the given line to the opposite switch texture.
// If the switch is usable again, schedule it to switch back after a while or otherwise mark it unusable.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ChangeSwitchTexture(line_t& line, const bool bUseAgain) noexcept {
    // If the switch is once only then wipe the special so it can't be used again
    if (!bUseAgain) {
        line.special = 0;
    }
    
    // Choose the sound for the switch.
    // Use a different sound for the exit switch (special = 11).
    const sfxenum_t soundId = (line.special == 11) ? sfx_swtchx : sfx_swtchn;

    // Try to match a portion of the wall (upper, middle, lower) against a switch texture.
    // When a match is found flip the texture to the opposite switch texture.
    // If the switch is usable again, switch it back after a while.
    side_t& side = gpSides[line.sidenum[0]];
    
    for (int32_t switchListIdx = 0; switchListIdx < NUM_SWITCHES * 2; ++switchListIdx) {
        const int32_t switchTex = gSwitchList[switchListIdx];

        // Note: for all these cases the button should have a 'NULL' sound origin set because it's struct has been zero intialized.
        // Therefore the initial switch sound will not play positionally, and will always be at full volume.
        // I wonder is this odd for deathmatch though? 3DO DOOM appears to use sector that the switch is in for the sound origin...
        if (switchTex == side.toptexture) {
            S_StartSound(gButtonList[0].soundorg, soundId);
            side.toptexture = gSwitchList[switchListIdx ^ 1];

            if (bUseAgain) {
                P_StartButton(line, top, switchTex, BUTTONTIME);
            }

            return;
        }

        if (switchTex == side.midtexture) {
            S_StartSound(gButtonList[0].soundorg, soundId);
            side.midtexture = gSwitchList[switchListIdx ^ 1];

            if (bUseAgain) {
                P_StartButton(line, middle, switchTex, BUTTONTIME);
            }

            return;
        }

        if (switchTex == side.bottomtexture) {
            S_StartSound(gButtonList[0].soundorg, soundId);
            side.bottomtexture = gSwitchList[switchListIdx ^ 1];

            if (bUseAgain) {
                P_StartButton(line, bottom, switchTex, BUTTONTIME);
            }

            return;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to use the given line and activate whatever special thing it does; used to activate switches and doors.
// Returns 'true' if the line could be used, 'false' if not.
// Assumes the line has a special and only attempts to activate the front side of the line, since that is all that is allowed.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_UseSpecialLine(mobj_t& mobj, line_t& line) noexcept {
    // For monsters only certain types of lines can be used
    if (!mobj.player) {
        // Monsters cannot activate lines that are marked as secrets
        if (line.flags & ML_SECRET)
            return false;
        
        // Presently only ordinary manual doors can be activated by monsters
        switch (line.special) {
            case 1:     // Manual door raise
                break;

            default:    // NOT allowed!
                return false;
        }
    }

    // Try to activate the line's special
    switch (line.special) {
        //----------------------------------------------------------------------------------------------------------------------------------
        // Manually activated doors and locked doors
        //----------------------------------------------------------------------------------------------------------------------------------
        case 26:    // Blue door raise
        case 27:    // Yellow door raise 
        case 28:    // Red door raise
        case 32:    // Blue door open
        case 33:    // Red door open
        case 34:    // Yellow door open
        {      
            if (P_CheckKeyLock(line, mobj)) {
                EV_VerticalDoor(line, mobj);
            }
        }   break;

        case 1:     // Vertical door
        case 31:    // Manual door open
        case 117:   // Blazing door raise
        case 118:   // Blazing door open
        {     
            EV_VerticalDoor(line, mobj);
        }   break;

        case 99:    // Blue blazing door open
        case 134:   // Red blazing door open
        case 136:   // Yellow blazing door open
        {     
            if (P_CheckKeyLock(line, mobj)) {
                if (EV_DoDoor(line, BlazeOpen)) {
                    P_ChangeSwitchTexture(line, true);
                }
            }
        }   break;

        case 133:   // Blue blazing door open
        case 135:   // Red blazing door open
        case 137:   // Yellow blazing door open
        {
            if (P_CheckKeyLock(line, mobj)) {
                if (EV_DoDoor(line, BlazeOpen)) {
                    P_ChangeSwitchTexture(line, true);
                }
            }
        }   break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Repeatable switches
        //----------------------------------------------------------------------------------------------------------------------------------

        // Close door
        case 42: {
            if (EV_DoDoor(line, Close)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Lower ceiling to floor
        case 43: {
            if (EV_DoCeiling(line, lowerToFloor)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Lower floor to surrounding floor height
        case 45: {
            if (EV_DoFloor(line, lowerFloor)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Lower floor to lowest
        case 60: {
            if (EV_DoFloor(line, lowerFloorToLowest)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Open door
        case 61: {
            if (EV_DoDoor(line, Open)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Platform - down, wait, up, stay
        case 62: {
            if (EV_DoPlat(line, downWaitUpStay, 1)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Raise door
        case 63: {
            if (EV_DoDoor(line, Normal)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Raise floor to ceiling
        case 64: {
            if (EV_DoFloor(line, raiseFloor)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Raise floor - crush
        case 65: {
            if (EV_DoFloor(line, raiseFloorCrush)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Raise floor 24 and change texture
        case 66: {
            if (EV_DoPlat(line, raiseAndChange, 24)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Raise floor 32 and change texture
        case 67: {
            if (EV_DoPlat(line, raiseAndChange, 32)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Raise plat to next highest floor and change texture
        case 68: {
            if (EV_DoPlat(line, raiseToNearestAndChange, 0)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Raise floor to next highest floor
        case 69: {
            if (EV_DoFloor(line, raiseFloorToNearest)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Turbo lower floor
        case 70: {
            if (EV_DoFloor(line, turboLower)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Blazing door raise (faster than turbo!)
        case 114: {
            if (EV_DoDoor(line, BlazeRaise)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Blazing door open (faster than turbo!)
        case 115: {
            if (EV_DoDoor(line, BlazeOpen)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Blazing door close (faster than turbo!)
        case 116: {
            if (EV_DoDoor(line, BlazeClose)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Blazing platform - down, wait, up, stay
        case 123: {
            if (EV_DoPlat(line, blazeDWUS, 0)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Light turn on
        case 138: {
            EV_LightTurnOn(line, 255);
            P_ChangeSwitchTexture(line, true);
        }   break;

        // Light turn off
        case 139: {
            EV_LightTurnOn(line, 35);
            P_ChangeSwitchTexture(line, true);
        }   break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Once-only switches
        //----------------------------------------------------------------------------------------------------------------------------------

        // Build stairs
        case 7: {
            if (EV_BuildStairs(line, build8)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Change donut
        case 9: {
            if (EV_DoDonut(line)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Exit level
        case 11: {
            G_ExitLevel();
            P_ChangeSwitchTexture(line, false);
        }   break;

        // Raise floor 32 and change texture
        case 14: {
            if (EV_DoPlat(line, raiseAndChange, 32)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Raise floor 24 and change texture
        case 15: {
            if (EV_DoPlat(line, raiseAndChange, 24)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        //  Raise floor to next highest floor
        case 18: {
            if (EV_DoFloor(line, raiseFloorToNearest)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Raise platform to next highest floor and change texture
        case 20: {
            if (EV_DoPlat(line, raiseToNearestAndChange, 0)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Platform - down, wait, up, stay
        case 21: {
            if (EV_DoPlat(line, downWaitUpStay, 0)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Lower floor to lowest
        case 23: {
            if (EV_DoFloor(line, lowerFloorToLowest)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Raise door
        case 29: {
            if (EV_DoDoor(line, Normal)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Lower ceiling to floor
        case 41: {
            if (EV_DoCeiling(line, lowerToFloor)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Ceiling crush and raise
        case 49: {
            if (EV_DoCeiling(line, crushAndRaise)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Close door
        case 50: {
            if (EV_DoDoor(line, Close)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Secret exit
        case 51: {
            G_SecretExitLevel(line.tag);
            P_ChangeSwitchTexture(line, false);
        }   break;

        // Raise floor crush
        case 55: {
            if (EV_DoFloor(line, raiseFloorCrush)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Turbo lower floor
        case 71: {
            if (EV_DoFloor(line, turboLower)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Raise floor
        case 101: {
            if (EV_DoFloor(line, raiseFloor)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Lower floor to surrounding floor height
        case 102: {
            if (EV_DoFloor(line, lowerFloor)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Open door
        case 103: {
            if (EV_DoDoor(line, Open)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;
        
        // Blazing door raise (faster than turbo!)
        case 111: {
            if (EV_DoDoor(line, BlazeRaise)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Blazing door open (faster than turbo!)
        case 112: {
            if (EV_DoDoor(line, BlazeOpen)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Blazing door close (faster than turbo!)
        case 113: {
            if (EV_DoDoor(line, BlazeClose)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Blazing platform - down, wait, up, stay
        case 122: {
            if (EV_DoPlat(line, blazeDWUS, 0)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Build stairs - turbo 16
        case 127: {
            if (EV_BuildStairs(line, turbo16)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        default:
            break;
    }

    return true;
}
