//------------------------------------------------------------------------------------------------------------------------------------------
// Logic for the various finale screens, including the end of game cast sequence.
// Note that I have adopted and mixed some of the code from Final Doom here because it is more flexible to support multiple games.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "f_finale.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/RendererVk/rv_utils.h"
#include "m_main.h"
#include "PcPsx/Game.h"
#include "PcPsx/Input.h"
#include "PcPsx/PsxPadButtons.h"
#include "PcPsx/Utils.h"
#include "PcPsx/Video.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "PsyQ/LIBGPU.h"

// Win text for Doom 1 and 2, and Final Doom.
// Note that this is all in the Final Doom format, which is an unbounded C-String.
static const char* const gDoom1WinText[] = {
    "you have won!",
    "your victory enabled",
    "humankind to evacuate",
    "earth and escape the",
    "nightmare.",
    "but then earth control",
    "pinpoints the source",
    "of the alien invasion.",
    "you are their only hope.",
    "you painfully get up",
    "and return to the fray.",
    nullptr
};

static const char* const gDoom2WinText[] = {
    "you did it!",
    "by turning the evil of",
    "the horrors of hell in",
    "upon itself you have",
    "destroyed the power of",
    "the demons.",
    "their dreadful invasion",
    "has been stopped cold!",
    "now you can retire to",
    "a lifetime of frivolity.",
    "congratulations!",
    nullptr
};

static const char* const gFinalDoomWinText_MasterLevels[] = {
    "you have assaulted and",
    "triumphed over the most",
    "vicious realms that the",
    "demented minds of our",
    "designers could devise.",
    "the havoc you left",
    "behind you as you",
    "smashed your way",
    "through the master",
    "levels is mute tribute",
    "to your prowess.",
    "you have earned the",
    "title of",
    "Master of Destruction.",
    nullptr
};

static const char* const gFinalDoomWinText_Tnt[] = {
    "suddenly all is silent",
    "from one horizon to the",
    "other.",
    "the agonizing echo of",
    "hell fades away.",
    "the nightmare sky",
    "turns blue.",
    "the heaps of monster",
    "corpses begin to dissolve",
    "along with the evil stench",
    "that filled the air.",
    "maybe you_have done it.",
    "Have you really won...",
    nullptr
};

static const char* const gFinalDoomWinText_Plutonia[] = {
    "you_gloat_over_the",
    "carcass_of_the_guardian.",
    "with_its_death_you_have",
    "wrested_the_accelerator",
    "from_the_stinking_claws",
    "of_hell._you_are_done.",
    "hell_has_returned_to",
    "pounding_dead_folks",
    "instead_of_good_live_ones.",
    "remember_to_tell_your",
    "grandkids_to_put_a_rocket",
    "launcher_in_your_coffin.",
    "If_you_go_to_hell_when",
    "you_die_you_will_need_it",
    "for_some_cleaning_up.",
    nullptr
};

// The top y position of the win text for Doom 1 and 2, and Final Doom
static constexpr int32_t DOOM1_TEXT_YPOS = 45;
static constexpr int32_t DOOM2_TEXT_YPOS = 45;
static constexpr int32_t MASTERLEV_TEXT_YPOS = 22;
static constexpr int32_t TNT_TEXT_YPOS = 29;
static constexpr int32_t PLUTONIA_TEXT_YPOS = 15;

// The cast of characters to display
struct castinfo_t {
    const char*     name;
    mobjtype_t      type;
};

static const castinfo_t gCastOrder[] = {
    { "Zombieman",              MT_POSSESSED    },
    { "Shotgun Guy",            MT_SHOTGUY      },
    { "Heavy Weapon Dude",      MT_CHAINGUY     },
    { "Imp",                    MT_TROOP        },
    { "Demon",                  MT_SERGEANT     },
    { "Lost Soul",              MT_SKULL        },
    { "Cacodemon",              MT_HEAD         },
    { "Hell Knight",            MT_KNIGHT       },
    { "Baron Of Hell",          MT_BRUISER      },
    { "Arachnotron",            MT_BABY         },
    { "Pain Elemental",         MT_PAIN         },
    { "Revenant",               MT_UNDEAD       },
    { "Mancubus",               MT_FATSO        },
    { "The Spider Mastermind",  MT_SPIDER       },
    { "The Cyberdemon",         MT_CYBORG       },
    { "Our Hero",               MT_PLAYER       },
    { nullptr,                  MT_PLAYER       }       // Null marker
};

// Used for the cast finale - what stage we are at
enum finalestage_t : int32_t {
    F_STAGE_TEXT,
    F_STAGE_SCROLLTEXT,
    F_STAGE_CAST,
};

static finalestage_t        gFinaleStage;           // What stage of the finale we are on
static int32_t              gFinTextYPos;           // Current y position of the top finale line
static char                 gFinIncomingLine[64];   // Text for the incoming line
static int32_t              gFinIncomingLineLen;    // How many characters are being displayed for the incomming text line
static const char* const*   gFinTextLines;          // Which text lines we are using for this finale text?
static int32_t              gFinLinesDone;          // How many full lines we are displaying
static int32_t              gCastNum;               // Which of the cast characters (specified by index) we are showing
static int32_t              gCastTics;              // Tracks current time in the current cast state
static int32_t              gCastFrames;            // Tracks how many frames a cast member has done - used to decide when to attack
static int32_t              gCastOnMelee;           // If non zero then the cast member should do a melee attack
static bool                 gbCastDeath;            // Are we killing the current cast member?
static const state_t*       gpCastState;            // Current state being displayed for the cast character
static texture_t            gTex_DEMON;             // The demon (icon of sin) background for the DOOM II finale

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the Ultimate DOOM style finale (text only, no cast sequence) screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Start() noexcept {
    // Draw the loading plaque, purge the texture cache and load up the background needed
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());
    I_PurgeTexCache();
    I_CacheTex(gTex_BACK);

    // Init finale
    gFinLinesDone = 0;
    gFinIncomingLineLen = 0;
    gFinIncomingLine[0] = 0;

    if (Game::isFinalDoom()) {
        if (Game::getMapEpisode(gGameMap) == 1) {
            gFinTextLines = gFinalDoomWinText_MasterLevels;
            gFinTextYPos = MASTERLEV_TEXT_YPOS;
        } else {
            gFinTextLines = gFinalDoomWinText_Tnt;
            gFinTextYPos = TNT_TEXT_YPOS;
        }
    } else {
        gFinTextLines = gDoom1WinText;
        gFinTextYPos = DOOM1_TEXT_YPOS;
    }

    // Play the finale cd track
    psxcd_play_at_andloop(
        gCDTrackNum[cdmusic_finale_doom1_final_doom],
        gCdMusicVol,
        0,
        0,
        gCDTrackNum[cdmusic_credits_demo],
        gCdMusicVol,
        0,
        0
    );

    // Wait until some cd audio has been read
    Utils::waitForCdAudioPlaybackStart();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called to shut down the Ultimate DOOM style finale (text only, no cast sequence) screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    gbGamePaused = false;
    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the Ultimate DOOM style finale (text only, no cast sequence) screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t F1_Ticker() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0)
            return ga_nothing;
    #endif

    // Grab inputs and set global game action
    gGameAction = ga_nothing;

    #if PSYDOOM_MODS
        const TickInputs& inputs = gTickInputs[gCurPlayerIndex];
        const TickInputs& oldInputs = gOldTickInputs[gCurPlayerIndex];
        const bool bMenuOk = (inputs.bMenuOk && (!oldInputs.bMenuOk));
    #else
        const padbuttons_t ticButtons = gTicButtons[gCurPlayerIndex];
        const padbuttons_t oldTicButtons = gOldTicButtons[gCurPlayerIndex];
        const bool bMenuOk = ((ticButtons != oldTicButtons) && (ticButtons & PAD_ACTION_BTNS));
    #endif

    // Not sure why this screen is updating cheats or checking for pause...
    P_CheckCheats();
    
    if (gbGamePaused)
        return gGameAction;

    // Check to see if the text needs to advance more, or if we can exit
    const char* const textLine = gFinTextLines[gFinLinesDone];

    if (textLine) {
        // Text is not yet done popping up: only advance if time has elapsed and on every 2nd tick
        if ((gGameTic > gPrevGameTic) && ((gGameTic & 1) == 0)) {
            // Get the current incoming text line text and see if we need to move onto another
            if (textLine[gFinIncomingLineLen] == 0) {
                // We've reached the end of this line, move onto a new one
                gFinIncomingLineLen = 0;
                gFinLinesDone++;
            } else {
                D_strncpy(gFinIncomingLine, textLine, gFinIncomingLineLen);
            }
            
            // Null terminate the incomming text line and include this character in the length
            gFinIncomingLine[gFinIncomingLineLen] = 0;
            gFinIncomingLineLen++;
        }
    }
    else if (bMenuOk) {
        // If all the lines are done and an action button is just pressed then exit the screen
        return ga_exit;
    }
    
    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the rendering for the Ultimate Doom style finale screen (text popping up gradually)
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Drawer() noexcept {
    // Increment the frame count (for the texture cache) and draw the background
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    I_CacheAndDrawSprite(gTex_BACK, 0, 0, Game::getTexPalette_BACK());

    // Show both the incoming and fully displayed text lines
    int32_t ypos = gFinTextYPos;
    
    for (int32_t lineIdx = 0; lineIdx < gFinLinesDone; ++lineIdx) {
        I_DrawString(-1, ypos, gFinTextLines[lineIdx]);
        ypos += 14;
    }
    
    I_DrawString(-1, ypos, gFinIncomingLine);

    // Not sure why the finale screen would be 'paused'?
    if (gbGamePaused) {
        I_DrawPausedOverlay();
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the DOOM II style finale (text, followed by cast) screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F2_Start() noexcept {
    // Show the loading plaque and purge the texture cache
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());
    I_PurgeTexCache();

    // Load the background and sprites needed.
    I_LoadAndCacheTexLump(gTex_DEMON, "DEMON", 0);

    #if !PSYDOOM_MODS
        // PsyDoom: don't bother loading the sprites yet, let the code grab them from the main IWAD as required.
        // This means we don't need to worry about loading the correct blocks file for various different game types.
        P_LoadBlocks(CdFileId::MAPSPR60_IMG);
    #endif

    // Initialize the finale text
    const mobjinfo_t& mobjInfo = gMObjInfo[gCastOrder[0].type];
    const state_t& state = gStates[mobjInfo.seestate];

    gFinaleStage = F_STAGE_TEXT;
    gFinLinesDone = 0;
    gFinIncomingLineLen = 0;
    gFinIncomingLine[0] = 0;

    if (Game::isFinalDoom()) {
        gFinTextLines = gFinalDoomWinText_Plutonia;
        gFinTextYPos = PLUTONIA_TEXT_YPOS;
    } else {
        gFinTextLines = gDoom2WinText;
        gFinTextYPos = DOOM2_TEXT_YPOS;
    }
    
    // Initialize the cast display
    gCastNum = 0;
    gpCastState = &state;
    gCastTics = state.tics;
    gbCastDeath = false;
    gCastFrames = 0;
    gCastOnMelee = 0;
    
    // Load sound for the finale
    S_LoadMapSoundAndMusic(Game::getNumMaps() + 1);

    // Play the finale cd track
    psxcd_play_at_andloop(
        gCDTrackNum[(Game::isFinalDoom()) ? cdmusic_finale_doom1_final_doom : cdmusic_finale_doom2],
        gCdMusicVol,
        0,
        0,
        gCDTrackNum[cdmusic_credits_demo],
        gCdMusicVol,
        0,
        0
    );

    // Wait until some cd audio has been read
    Utils::waitForCdAudioPlaybackStart();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called to shut down the DOOM II style finale (text, followed by cast) screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F2_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    gbGamePaused = false;
    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the DOOM II style finale (text, followed by cast) screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t F2_Ticker() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0)
            return ga_nothing;
    #endif

    // Grab inputs and set global game action
    gGameAction = ga_nothing;

    #if PSYDOOM_MODS
        const TickInputs& inputs = gTickInputs[gCurPlayerIndex];
        const TickInputs& oldInputs = gOldTickInputs[gCurPlayerIndex];
        const bool bMenuOk = (inputs.bMenuOk && (!oldInputs.bMenuOk));
    #else
        const padbuttons_t ticButtons = gTicButtons[gCurPlayerIndex];
        const padbuttons_t oldTicButtons = gOldTicButtons[gCurPlayerIndex];
        const bool bMenuOk = ((ticButtons != oldTicButtons) && (ticButtons & PAD_ACTION_BTNS));
    #endif

    // Not sure why this screen is updating cheats or checking for pause...
    P_CheckCheats();
    
    if (gbGamePaused)
        return gGameAction;
    
    // Handle whatever finale stage we are on
    if (gFinaleStage == F_STAGE_TEXT) {
        // Currently popping up text: only advance if time has elapsed and on every 2nd tick
        if ((gGameTic > gPrevGameTic) && ((gGameTic & 1) == 0)) {
            // Get the current incoming text line text and see if we need to move onto another
            const char* const textLine = gFinTextLines[gFinLinesDone];

            if (textLine) {
                if (textLine[gFinIncomingLineLen] == 0) {
                    // We've reached the end of this line, move onto a new one
                    gFinLinesDone++;
                    gFinIncomingLineLen = 0;
                } else {
                    D_strncpy(gFinIncomingLine, textLine, gFinIncomingLineLen);
                }
            } else {
                // If we're done all the lines then begin scrolling the text up
                gFinaleStage = F_STAGE_SCROLLTEXT;
            }

            // Null terminate the incomming text line and include this character in the length
            gFinIncomingLine[gFinIncomingLineLen] = 0;
            gFinIncomingLineLen++;
        }
    }
    else if (gFinaleStage == F_STAGE_SCROLLTEXT) {
        // Scrolling the finale text upwards for a bit before the cast call
        gFinTextYPos -= 1;
        
        if (gFinTextYPos < -200) {
            gFinaleStage = F_STAGE_CAST;
        }
    }
    else if (gFinaleStage == F_STAGE_CAST)  {
        // Doing the cast call: see first if the player is shooting the current character
        if ((!gbCastDeath) && bMenuOk) {
            // Shooting this character! Play the shotgun sound:
            S_StartSound(nullptr, sfx_shotgn);

            // Play enemy death sound if it has it
            const mobjinfo_t& mobjinfo = gMObjInfo[gCastOrder[gCastNum].type];
            
            if (mobjinfo.deathsound != 0) {
                S_StartSound(nullptr, mobjinfo.deathsound);
            }

            // Begin playing the death animation
            const state_t& deathState = gStates[mobjinfo.deathstate];

            gbCastDeath = true;
            gCastFrames = 0;
            gpCastState = &deathState;
            gCastTics = deathState.tics;
        }

        // Only advance character animation if time has passed
        if (gGameTic > gPrevGameTic) {
            if (gbCastDeath && (gpCastState->nextstate == S_NULL)) {
                // Character is dead and there is no state which follows (death anim is finished): switch to the next character
                gCastNum++;
                gbCastDeath = false;

                // Loop back around to the first character when we reach the end
                if (gCastOrder[gCastNum].name  == nullptr) {
                    gCastNum = 0;
                }

                // Initialize frame count, set state to the 'see' state and make a noise if there is a 'see' sound
                const castinfo_t& castinfo = gCastOrder[gCastNum];
                const mobjinfo_t& mobjinfo = gMObjInfo[castinfo.type];

                gCastFrames = 0;
                gpCastState = &gStates[mobjinfo.seestate];

                if (mobjinfo.seesound != 0) {
                    S_StartSound(nullptr, mobjinfo.seesound);
                }
            } else {
                // Character is not dead, advance the time until the next state
                gCastTics -= 1;

                // If we still have time until the next state then there is nothing more to do
                if (gCastTics > 0)
                    return ga_nothing;
                
                // Hacked in logic to play sounds on certain frames of animation
                sfxenum_t soundId;

                switch (gpCastState->nextstate) {
                    case S_PLAY_ATK2:   soundId = sfx_dshtgn;   break;
                    case S_POSS_ATK2:   soundId = sfx_pistol;   break;
                    case S_SPOS_ATK2:   soundId = sfx_shotgn;   break;
                    case S_SKEL_FIST2:  soundId = sfx_skeswg;   break;
                    case S_SKEL_FIST4:  soundId = sfx_skepch;   break;
                    case S_SKEL_MISS2:  soundId = sfx_skeatk;   break;
                    case S_FATT_ATK2:
                    case S_FATT_ATK5:
                    case S_FATT_ATK8:   soundId = sfx_firsht;   break;
                    case S_CPOS_ATK2:
                    case S_CPOS_ATK3:
                    case S_CPOS_ATK4:   soundId = sfx_pistol;   break;
                    case S_TROO_ATK3:   soundId = sfx_claw;     break;
                    case S_SARG_ATK2:   soundId = sfx_sgtatk;   break;
                    case S_BOSS_ATK2:
                    case S_BOS2_ATK2:
                    case S_HEAD_ATK2:   soundId = sfx_firsht;   break;
                    case S_SKULL_ATK2:  soundId = sfx_sklatk;   break;
                    case S_SPID_ATK2:
                    case S_SPID_ATK3:   soundId = sfx_pistol;   break;
                    case S_BSPI_ATK2:   soundId = sfx_plasma;   break;
                    case S_CYBER_ATK2:
                    case S_CYBER_ATK4:
                    case S_CYBER_ATK6:  soundId = sfx_rlaunc;   break;
                    case S_PAIN_ATK3:   soundId = sfx_sklatk;   break;

                    default: soundId = sfx_None;
                }

                if (soundId != sfx_None) {
                    S_StartSound(nullptr, soundId);
                }
            }

            // Advance onto the next state
            gpCastState = &gStates[gpCastState->nextstate];
            gCastFrames++;

            const castinfo_t& castinfo = gCastOrder[gCastNum];
            const mobjinfo_t& mobjinfo = gMObjInfo[castinfo.type];

            // Every 12 frames make the enemy attack.
            // Alternate between melee and ranged attacks where possible.
            if (gCastFrames == 12) {
                if (gCastOnMelee) {
                    gpCastState = &gStates[mobjinfo.meleestate];
                } else {
                    gpCastState = &gStates[mobjinfo.missilestate];
                }

                // If there wasn't a ranged or melee attack then try using the opposite attack type
                gCastOnMelee ^= 1;

                if (gpCastState == &gStates[S_NULL]) {
                    if (gCastOnMelee) {
                        gpCastState = &gStates[mobjinfo.meleestate];
                    } else {
                        gpCastState = &gStates[mobjinfo.missilestate];
                    }
                }
            }

            // If it is the player then every so often put it into the 'see' state
            if ((gCastFrames == 24) || (gpCastState == &gStates[S_PLAY])) {
                gpCastState = &gStates[mobjinfo.seestate];
                gCastFrames = 0;
            }
            
            // Update the number of tics to stay in this state.
            // If the state defines no time limit then make up one:
            gCastTics = gpCastState->tics;

            if (gCastTics == -1) {
                gCastTics = TICRATE;
            }
        }
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the rendering for the Doom II style finale screen (text, then cast of characters)
//------------------------------------------------------------------------------------------------------------------------------------------
void F2_Drawer() noexcept {
    // Draw the icon of sin background and increment frame count for the texture cache
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    I_CacheAndDrawSprite(gTex_DEMON, 0, 0, gPaletteClutIds[MAINPAL]);

    // See whether we are drawing the text or the cast of characters
    if (gFinaleStage >= F_STAGE_TEXT && gFinaleStage <= F_STAGE_SCROLLTEXT) {
        // Still showing the text, show both the incoming and fully displayed text lines
        int32_t ypos = gFinTextYPos;

        for (int32_t lineIdx = 0; lineIdx < gFinLinesDone; ++lineIdx) {
            I_DrawString(-1, ypos, gFinTextLines[lineIdx]);
            ypos += 14;
        }
        
        I_DrawString(-1, ypos, gFinIncomingLine);
    }
    else if (gFinaleStage == F_STAGE_CAST) {
        // Showing the cast of character, get the texture for the current sprite to show and cache it
        const state_t& state = *gpCastState;
        const spritedef_t& spriteDef = gSprites[state.sprite];
        const spriteframe_t& spriteFrame = spriteDef.spriteframes[state.frame & FF_FRAMEMASK];

        texture_t& spriteTex = gpSpriteTextures[spriteFrame.lump[0] - gFirstSpriteLumpNum];
        I_CacheTex(spriteTex);

        // Setup and draw the sprite for the cast character
        #if PSYDOOM_MODS
            // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
            POLY_FT4 polyPrim = {};
        #else
            POLY_FT4& polyPrim = *(POLY_FT4*) LIBETC_getScratchAddr(128);
        #endif

        do {
            // PsyDoom: new drawing path for when the Vulkan renderer is in use.
            // This is needed to avoid artifacts when MSAA is enabled.
            #if PSYDOOM_VULKAN_RENDERER
                if (Video::isUsingVulkanRenderPath()) {
                    // Set the correct draw pipeline
                    VDrawing::setDrawPipeline(VPipelineType::UI_8bpp);

                    // Decide on sprite position and texture coordinates
                    float ul, ur;
                    const int16_t ypos = 180 - spriteTex.offsetY;
                    int16_t xpos;

                    if (!spriteFrame.flip[0]) {
                        ul = 0.0f;
                        ur = (float) spriteTex.width;
                        xpos = HALF_SCREEN_W - spriteTex.offsetX;
                    } else {
                        ul = (float) spriteTex.width;
                        ur = (float) 0.0f;
                        xpos = HALF_SCREEN_W + spriteTex.offsetX - spriteTex.width;
                    }

                    const float vt = 0.0f;
                    const float vb = (float) spriteTex.height;
                    const float xl = (float) xpos;
                    const float xr = (float) xpos + (float) spriteTex.width;
                    const float yt = (float) ypos;
                    const float yb = (float) ypos + (float) spriteTex.height;

                    // Get the sprite texture window and clut location.
                    // Make sprite texture coordinates relative to a clamping texture window.
                    uint16_t texWinX, texWinY, texWinW, texWinH;
                    RV_GetTexWinXyWh(spriteTex, texWinX, texWinY, texWinW, texWinH);

                    uint16_t clutX, clutY;
                    RV_ClutIdToClutXy(gPaletteClutIds[MAINPAL], clutX, clutY);

                    // Draw the sprite and skip the classic draw code
                    VDrawing::addWorldQuad(
                        xl, yt, 0.0f, ul, vt,
                        xr, yt, 0.0f, ur, vt,
                        xr, yb, 0.0f, ur, vb,
                        xl, yb, 0.0f, ul, vb,
                        128, 128, 128,
                        clutX, clutY,
                        texWinX, texWinY, texWinW, texWinH,
                        VLightDimMode::None, 128, 128, 128, 128
                    );

                    break;
                }
            #endif

            // Classic drawing code: use a PolyFT4 to draw the character
            LIBGPU_SetPolyFT4(polyPrim);
            LIBGPU_SetShadeTex(polyPrim, true);
            polyPrim.clut = gPaletteClutIds[MAINPAL];
            polyPrim.tpage = spriteTex.texPageId;

            const int16_t ypos = 180 - spriteTex.offsetY;
            int16_t xpos;

            if (!spriteFrame.flip[0]) {
                // PsyDoom: corrected UV coordinates to avoid pixel doubling and use 16-bit coords in case of overflow
                #if PSYDOOM_LIMIT_REMOVING
                    polyPrim.u0 = (LibGpuUV)(spriteTex.texPageCoordX);
                    polyPrim.u1 = (LibGpuUV)(spriteTex.texPageCoordX + spriteTex.width);
                    polyPrim.u2 = (LibGpuUV)(spriteTex.texPageCoordX);
                    polyPrim.u3 = (LibGpuUV)(spriteTex.texPageCoordX + spriteTex.width);
                #else
                    polyPrim.u0 = spriteTex.texPageCoordX;
                    polyPrim.u1 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
                    polyPrim.u2 = spriteTex.texPageCoordX;
                    polyPrim.u3 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
                #endif

                xpos = HALF_SCREEN_W - spriteTex.offsetX;
            } else {
                // PsyDoom: corrected UV coordinates to avoid pixel doubling and use 16-bit coords in case of overflow
                #if PSYDOOM_LIMIT_REMOVING
                    polyPrim.u0 = (LibGpuUV)(spriteTex.texPageCoordX + spriteTex.width);
                    polyPrim.u1 = (LibGpuUV)(spriteTex.texPageCoordX);
                    polyPrim.u2 = (LibGpuUV)(spriteTex.texPageCoordX + spriteTex.width);
                    polyPrim.u3 = (LibGpuUV)(spriteTex.texPageCoordX);
                #else
                    polyPrim.u0 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
                    polyPrim.u1 = spriteTex.texPageCoordX;
                    polyPrim.u2 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
                    polyPrim.u3 = spriteTex.texPageCoordX;
                #endif
            
                xpos = HALF_SCREEN_W + spriteTex.offsetX - spriteTex.width;
            }

            LIBGPU_setXY4(polyPrim,
                xpos,                       ypos,
                spriteTex.width + xpos,     ypos,
                xpos,                       spriteTex.height + ypos,
                spriteTex.width + xpos,     spriteTex.height + ypos
            );
        
            // PsyDoom: corrected UV coordinates to avoid pixel doubling and use 16-bit coords in case of overflow
            #if PSYDOOM_LIMIT_REMOVING
                polyPrim.v0 = (LibGpuUV) spriteTex.texPageCoordY;
                polyPrim.v1 = (LibGpuUV) spriteTex.texPageCoordY;
                polyPrim.v2 = (LibGpuUV) spriteTex.texPageCoordY + spriteTex.height;
                polyPrim.v3 = (LibGpuUV) spriteTex.texPageCoordY + spriteTex.height;
            #else
                polyPrim.v0 = spriteTex.texPageCoordY;
                polyPrim.v1 = spriteTex.texPageCoordY;
                polyPrim.v2 = spriteTex.texPageCoordY + (uint8_t) spriteTex.height - 1;
                polyPrim.v3 = spriteTex.texPageCoordY + (uint8_t) spriteTex.height - 1;
            #endif

            I_AddPrim(polyPrim);
        } while (0);

        // Draw screen title and current character name
        I_DrawString(-1, 20, "Cast Of Characters");
        I_DrawString(-1, 208, gCastOrder[gCastNum].name);
    }

    // Not sure why the finale screen would be 'paused'?
    if (gbGamePaused) {
        I_DrawPausedOverlay();
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
