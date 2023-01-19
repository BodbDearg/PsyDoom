//------------------------------------------------------------------------------------------------------------------------------------------
// This is an entirely new menu added for PsyDoom.
// It is a root menu for loading, saving and password entry.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS

#include "loadsave_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "errormenu_main.h"
#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "FileUtils.h"
#include "m_main.h"
#include "o_main.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/SaveAndLoad.h"
#include "PsyDoom/SaveDataTypes.h"
#include "PsyDoom/Utils.h"
#include "pw_main.h"
#include "SmallString.h"
#include "st_main.h"

#include <algorithm>
#include <chrono>

// The available menu items
enum MenuItem : int32_t {
    menu_slot1,
    menu_slot2,
    menu_slot3,
    menu_slotQ,
    menu_slotA,
    menu_back,
    num_menu_items
};

// Represents a save slot
struct SaveFileInfo {
    SaveFileSlot    slot;       // Which save file slot this is
    char            label;      // Save slot displayed label ('1', '2', '3', or 'Q' for quicksave and 'A' for autosave)
    uint8_t         mapNum;     // Which map number the slot is for
    uint8_t         hrs;        // Hours played
    uint8_t         mins;       // Minutes played
    uint8_t         secs;       // Seconds played
    String32        mapName;    // Name of the map
};

static LoadSaveMenuMode     gMenuMode;                  // What mode the menu is operating in
static uint8_t              gSlotHighlightPhase;        // Current phase for the slot highlight effect
static SaveFileInfo         gSaveFiles[5];              // The 5 save file slots
static int32_t              gFocusedSaveSlot;           // Which save slot is currently focused (-1 if none)
static bool                 gbLoadSaveOnLevelStart;     // If true then load the currently buffered save on loading the next map

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to read the header for the specified save file and populates the slot info from it.
// Does nothing if the save does not exist.
//------------------------------------------------------------------------------------------------------------------------------------------
static void ReadSaveHeader(SaveFileInfo& save) noexcept {
    std::string saveFilePath = SaveAndLoad::getSaveFilePath(save.slot);

    if (!FileUtils::fileExists(saveFilePath.c_str()))
        return;

    try {
        // Read the file header
        FileInputStream file = FileInputStream(saveFilePath.c_str());
        SaveFileHdr saveHdr;
        file.read(saveHdr);

        // Make sure it is valid for this version of the game
        if (saveHdr.validateFileId() && saveHdr.validateVersion()) {
            // Save the map information
            save.mapNum = (uint8_t) saveHdr.mapNum;
            save.mapName = saveHdr.mapName;

            // Grab the time played and split it up into hours, minutes and seconds
            constexpr int64_t SECS_PER_MIN = 60;
            constexpr int64_t SECS_PER_HOUR = 60 * SECS_PER_MIN;
            constexpr int64_t MAX_SECONDS = SECS_PER_HOUR * 255 + SECS_PER_MIN * 59 + 59;

            int64_t secondsLeft = std::clamp(saveHdr.secondsPlayed, (int64_t) 0, MAX_SECONDS);
            save.hrs = (uint8_t)(secondsLeft / SECS_PER_HOUR);
            secondsLeft %= SECS_PER_HOUR;
            save.mins = (uint8_t)(secondsLeft / SECS_PER_MIN);
            secondsLeft %= SECS_PER_MIN;
            save.secs = (uint8_t)(secondsLeft);
        }
    }
    catch (...) {
        // Ignore...
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if there is a save slot currently focused
//------------------------------------------------------------------------------------------------------------------------------------------
static bool IsSaveSlotFocused() noexcept {
    return ((gFocusedSaveSlot >= 0) && (gFocusedSaveSlot < (int32_t) C_ARRAY_SIZE(gSaveFiles)));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws a sprite from the STATUS image
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawStatusSprite(
    const int16_t x,
    const int16_t y,
    const uint8_t u,
    const uint8_t v,
    const uint16_t w,
    const uint16_t h,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b
) noexcept {
    const LibGpuUV tpU = (LibGpuUV)(gTex_STATUS.texPageCoordX + u);
    const LibGpuUV tpV = (LibGpuUV)(gTex_STATUS.texPageCoordY + v);
    I_DrawColoredSprite(gTex_STATUS.texPageId, Game::getTexClut_STATUS(), x, y, tpU, tpV, w, h, r, g, b, false);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the skull cursor at the specified position
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawCursor(const int16_t cursorX, const int16_t cursorY) noexcept {
    DrawStatusSprite(
        (int16_t) cursorX,
        (int16_t) cursorY,
        (int16_t)(M_SKULL_TEX_U + (uint8_t) gCursorFrame * M_SKULL_W),
        (int16_t)(M_SKULL_TEX_V),
        M_SKULL_W,
        M_SKULL_H,
        128,
        128,
        128
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws a piece of the save slot background
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawSaveSlotBgPiece(const int16_t x, const int16_t y, const uint8_t u, const uint16_t w, const bool bDim) noexcept {
    const uint8_t colRgb = (bDim) ? 64 : 128;
    DrawStatusSprite(x, y, u, 0, w, 32, colRgb, colRgb, colRgb);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws a save slot at the specified position
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawSaveSlot(const SaveFileInfo& save, const int16_t slotX, const int16_t slotY, const bool bHighlight, const bool bDim) noexcept {
    // Draw the highlight around the slot's background if highlighted
    if (bHighlight) {
        // Controls the strength of the slot highlight (a 16 tick loop)
        constexpr uint8_t HIGHLIGHT_STRENGTH[16] = {
            0, 1, 2, 3, 4, 5, 6, 7,
            7, 6, 5, 4, 3, 2, 1, 0,
        };

        POLY_F4 quad = {};
        LIBGPU_SetPolyF4(quad);
        LIBGPU_setRGB0(quad, 255, 128 + HIGHLIGHT_STRENGTH[gSlotHighlightPhase & 0xF] * 16, 64);
        LIBGPU_setXY4(quad, slotX - 1, slotY - 1, slotX + 235, slotY - 1, slotX - 1, slotY + 33, slotX + 235, slotY + 33);
        I_AddPrim(quad);
    }

    // Draw the background for the slot
    DrawSaveSlotBgPiece(slotX, slotY, 193, 60, bDim);
    DrawSaveSlotBgPiece(slotX + 60, slotY, 196, 57, bDim);
    DrawSaveSlotBgPiece(slotX + 117, slotY, 196, 57, bDim);
    DrawSaveSlotBgPiece(slotX + 174, slotY, 196, 60, bDim);

    // Draw the slot label
    {
        const char slotLabel[2] = { save.label, 0 };
        I_DrawString(slotX + 4, slotY + 8, slotLabel);
    }

    // Draw the map number and level time
    {
        char strBuffer[128];

        if (save.mapNum > 0) {
            if (save.hrs > 0) {
                std::snprintf(strBuffer, C_ARRAY_SIZE(strBuffer), "Map%02d %d:%02d:%02d", (int) save.mapNum, (int) save.hrs, (int) save.mins, (int) save.secs);
            } else {
                std::snprintf(strBuffer, C_ARRAY_SIZE(strBuffer), "Map%02d %d:%02d", (int) save.mapNum, (int) save.mins, (int) save.secs);
            }
        } else {
            std::snprintf(strBuffer, C_ARRAY_SIZE(strBuffer), "-");
        }

        I_DrawStringSmall(slotX + 24, slotY + 6, strBuffer, Game::getTexClut_STATUS(), 128, 128, 128, false, true);
    }

    // Draw the map name
    if (save.mapNum > 0) {
        I_DrawStringSmall(slotX + 24, slotY + 19, save.mapName.c_str().data(), Game::getTexClut_STATUS(), 128, 128, 128, false, true);
    } else {
        I_DrawStringSmall(slotX + 24, slotY + 19, "-", Game::getTexClut_STATUS(), 128, 128, 128, false, true);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the mode that the load/save menu operates in
//------------------------------------------------------------------------------------------------------------------------------------------
void LoadSave_SetMode(const LoadSaveMenuMode mode) noexcept {
    gMenuMode = mode;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void LoadSave_Init() noexcept {
    S_StartSound(nullptr, sfx_pistol);

    // Initialize various menu things
    gCursorFrame = 0;
    gCursorPos[gCurPlayerIndex] = 0;
    gVBlanksUntilMenuMove[gCurPlayerIndex] = 0;
    gSlotHighlightPhase = 0;
    gFocusedSaveSlot = -1;

    // Do basic empty initialization of the save file slots
    std::memset(&gSaveFiles, 0, sizeof(gSaveFiles));

    gSaveFiles[0].label = '1';  gSaveFiles[0].slot = SaveFileSlot::SAVE1;
    gSaveFiles[1].label = '2';  gSaveFiles[1].slot = SaveFileSlot::SAVE2;
    gSaveFiles[2].label = '3';  gSaveFiles[2].slot = SaveFileSlot::SAVE3;
    gSaveFiles[3].label = 'q';  gSaveFiles[3].slot = SaveFileSlot::QUICKSAVE;
    gSaveFiles[4].label = 'a';  gSaveFiles[4].slot = SaveFileSlot::AUTOSAVE;

    // Read the headers for each save
    for (SaveFileInfo& save : gSaveFiles) {
        ReadSaveHeader(save);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void LoadSave_Shutdown([[maybe_unused]] const gameaction_t exitAction) noexcept {
    gCursorPos[gCurPlayerIndex] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs update logic for the menu: does menu controls
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t LoadSave_Update() noexcept {
    // PsyDoom: in all UIs tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    const uint32_t playerIdx = gCurPlayerIndex;

    if (gPlayersElapsedVBlanks[playerIdx] <= 0) {
        gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
        return ga_nothing;
    }

    // Animate the skull cursor and slot highlight
    gSlotHighlightPhase++;

    if ((gGameTic > gPrevGameTic) && ((gGameTic & 3) == 0)) {
        gCursorFrame ^= 1;
    }

    // Gather menu inputs and exit if the back button has just been pressed
    const TickInputs& inputs = gTickInputs[playerIdx];
    const TickInputs& oldInputs = gOldTickInputs[playerIdx];

    const bool bMenuBack = (inputs.fMenuBack() && (!oldInputs.fMenuBack()));
    const bool bMenuOk = (inputs.fMenuOk() && (!oldInputs.fMenuOk()));
    const bool bMenuUp = inputs.fMenuUp();
    const bool bMenuDown = inputs.fMenuDown();
    const bool bMenuMove = (bMenuUp || bMenuDown);

    // Handle the back command (de-focus current save slot or exit the menu)
    const bool bSaveSlotFocused = IsSaveSlotFocused();

    if (bMenuBack) {
        if (bSaveSlotFocused) {
            gFocusedSaveSlot = -1;
            S_StartSound(nullptr, sfx_swtchx);
            return ga_nothing;
        } else {
            S_StartSound(nullptr, sfx_pistol);
            return ga_exit;
        }
    }

    // Check for up/down movement (only if there is a save slot not focused)
    if ((!bMenuMove) || bSaveSlotFocused) {
        // If there are no direction buttons pressed then the next move is allowed instantly
        gVBlanksUntilMenuMove[playerIdx] = 0;
    } else {
        // Direction buttons pressed or held down, check to see if we can move up/down now
        gVBlanksUntilMenuMove[playerIdx] -= gPlayersElapsedVBlanks[playerIdx];

        if (gVBlanksUntilMenuMove[playerIdx] <= 0) {
            gVBlanksUntilMenuMove[playerIdx] = 15;

            if (bMenuDown) {
                gCursorPos[playerIdx]++;

                if (gCursorPos[playerIdx] >= num_menu_items) {
                    gCursorPos[playerIdx] = 0;
                }

                S_StartSound(nullptr, sfx_pstop);
            }
            else if (bMenuUp) {
                gCursorPos[playerIdx]--;

                if (gCursorPos[playerIdx] < 0) {
                    gCursorPos[playerIdx] = num_menu_items - 1;
                }

                S_StartSound(nullptr, sfx_pstop);
            }
        }
    }

    // Menu options: for when a slot is focused and not
    if (bSaveSlotFocused) {
        // Slot focused, try to load or save a game if OK is pressed
        if (bMenuOk) {
            const SaveFileSlot saveSlot = gSaveFiles[gFocusedSaveSlot].slot;
            gameaction_t action = {};

            if (gMenuMode == LoadSaveMenuMode::Load) {
                S_StartSound(nullptr, sfx_pistol);
                action = LoadGameForSlot(saveSlot, LoadGameContext::Menu);
            } else {
                S_StartSound(nullptr, sfx_pistol);
                action = SaveGameForSlot(saveSlot, SaveGameContext::Menu);
            }

            if (action == ga_exitmenus) {
                // If exiting menus then ignore the attack key until it is next released.
                // This key can be used as the 'OK' command in menus too...
                gbIgnoreCurrentAttack = true;
            }

            if (action != ga_nothing)
                return action;
        }
        else if (bMenuDown || bMenuUp) {
            // If there is up or down menu movement then cancel the focus
            gFocusedSaveSlot = -1;
        }
    }
    else {
        // Normal menu operation: no slot focused
        const int32_t cursorPos = gCursorPos[playerIdx];
        const MenuItem curMenuItem = (MenuItem) cursorPos;

        switch (curMenuItem) {
            // Focus a save slot
            case menu_slot1:
            case menu_slot2:
            case menu_slot3:
            case menu_slotQ:
            case menu_slotA: {
                if (bMenuOk) {
                    // If loading then only allow focusing the slot if it's not empty
                    const int32_t saveSlotIdx = curMenuItem - menu_slot1;
                    ASSERT((saveSlotIdx >= 0) && (saveSlotIdx < (int32_t) C_ARRAY_SIZE(gSaveFiles)));
                    const SaveFileInfo& saveFile = gSaveFiles[saveSlotIdx];

                    const bool bDisallowFocus = (gMenuMode == LoadSaveMenuMode::Load) && (saveFile.mapNum <= 0);

                    if (!bDisallowFocus) {
                        // Normal case: can load the game so allow focusing the slot
                        gFocusedSaveSlot = saveSlotIdx;
                        S_StartSound(nullptr, sfx_swtchn);
                    } else {
                        // No save for the slot so just play a sound instead!
                        S_StartSound(nullptr, sfx_pstop);
                    }
                }
            }   break;

            // Go back to the options menu
            case menu_back: {
                if (bMenuOk) {
                    S_StartSound(nullptr, sfx_pistol);
                    return ga_exit;
                }
            } break;

            default:
                break;
        }
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void LoadSave_Draw() noexcept {
    // Increment the frame count for the texture cache and begin drawing
    I_IncDrawnFrameCount();
    Utils::onBeginUIDrawing();

    // Draw the background
    const bool bSaveSlotFocused = IsSaveSlotFocused();
    const uint8_t colRGB = (bSaveSlotFocused) ? 64 : 128;
    O_DrawBackground(gTex_OptionsBg, Game::getTexClut_OptionsBg(), colRGB, colRGB, colRGB);

    // Draw the save slots
    constexpr auto doDimSlot = [=](const int32_t slotNum) noexcept {
        const bool bOutOfFocus = ((gFocusedSaveSlot >= 0) && (slotNum != gFocusedSaveSlot));
        const bool bInvalidLoadSlot = ((gMenuMode == LoadSaveMenuMode::Load) && (gSaveFiles[slotNum].mapNum <= 0));
        return (bOutOfFocus || bInvalidLoadSlot);
    };

    DrawSaveSlot(gSaveFiles[0], 21, 30, (gFocusedSaveSlot == 0), doDimSlot(0));
    DrawSaveSlot(gSaveFiles[1], 21, 65, (gFocusedSaveSlot == 1), doDimSlot(1));
    DrawSaveSlot(gSaveFiles[2], 21, 100, (gFocusedSaveSlot == 2), doDimSlot(2));
    DrawSaveSlot(gSaveFiles[3], 21, 135, (gFocusedSaveSlot == 3), doDimSlot(3));
    DrawSaveSlot(gSaveFiles[4], 21, 170, (gFocusedSaveSlot == 4), doDimSlot(4));

    // Don't do any rendering if we are about to exit the menu
    if (gGameAction == ga_nothing) {
        // Menu title and back option
        I_DrawString(-1, 8, (gMenuMode == LoadSaveMenuMode::Load) ? "Load Game" : "Save Game");

        if (gFocusedSaveSlot < 0) {
            I_DrawString(-1, 211, "Back");
        }

        // Draw the skull cursor (only if there isn't a save slot focused)
        if (gFocusedSaveSlot < 0) {
            const int32_t curMenuOption = gCursorPos[gCurPlayerIndex];
            int16_t cursorX = 2;
            int16_t cursorY = 75;

            switch (curMenuOption) {
                case menu_slot1: cursorY = 36;  break;
                case menu_slot2: cursorY = 71;  break;
                case menu_slot3: cursorY = 106; break;
                case menu_slotQ: cursorY = 141; break;
                case menu_slotA: cursorY = 176; break;

                case menu_back:
                    cursorX = 80;
                    cursorY = 209;
                    break;
            }

            DrawCursor(cursorX, cursorY);
        }
    }

    // PsyDoom: draw any enabled performance counters
    #if PSYDOOM_MODS
        I_DrawEnabledPerfCounters();
    #endif

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Save the game for the specified save slot
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t SaveGameForSlot(const SaveFileSlot slot, const SaveGameContext saveContext) noexcept {
    ASSERT_LOG(gNetGame == gt_single, "Should only be called in single player games!");

    // Do the save and remember temporarily the slot being used
    SaveAndLoad::gCurSaveSlot = slot;
    bool bSuccess = false;

    try {
        const std::string savePath = SaveAndLoad::getSaveFilePath(slot);
        FileOutputStream file(savePath.c_str(), false);
        bSuccess = SaveAndLoad::save(file);
    }
    catch (...) {
        // Ignore...
    }

    // Display the result to the HUD and clear the current slot being loaded.
    // Skip the HUD message however if the unusual situation arises where we are auto-saving on level start and there is already
    // a message being displayed. The message already being displayed is likely a warning of some sort, so it should get priority.
    const bool bHudAlreadyHasMessage = ((gStatusBar.message != nullptr) && (gStatusBar.messageTicsLeft > 0));
    const bool bIsAutoSaving = (saveContext == SaveGameContext::Autosave);
    const bool bSkipHudMessage = (bHudAlreadyHasMessage && bIsAutoSaving);

    if (!bSkipHudMessage) {
        DisplaySavedHudMessage(saveContext, bSuccess);
    }

    SaveAndLoad::gCurSaveSlot = SaveFileSlot::NONE;

    // Exit all menus and unpause the game
    gbUnpauseAfterOptionsMenu = true;
    return ga_exitmenus;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the game for the specified save slot
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t LoadGameForSlot(const SaveFileSlot slot, const LoadGameContext loadContext) noexcept {
    // Initially assume we are not trying to load a save game when we load the next map.
    // Also remember the slot we are loading from.
    gbLoadSaveOnLevelStart = false;
    SaveAndLoad::gCurSaveSlot = slot;

    // Read the save first of all
    ReadSaveResult readSaveResult = ReadSaveResult::IO_ERROR;

    try {
        const std::string savePath = SaveAndLoad::getSaveFilePath(slot);
        FileInputStream file(savePath.c_str());
        readSaveResult = SaveAndLoad::read(file);
    }
    catch (...) {
        // Ignore...
    }

    // If that failed then show an error menu
    switch (readSaveResult) {
        case ReadSaveResult::OK:            break;
        case ReadSaveResult::BAD_FILE_ID:   return RunLoadGameErrorMenu_BadFileId();
        case ReadSaveResult::BAD_VERSION:   return RunLoadGameErrorMenu_BadFileVersion();
        case ReadSaveResult::BAD_MAP_NUM:   return RunLoadGameErrorMenu_BadMapNum();
        case ReadSaveResult::IO_ERROR:      return RunLoadGameErrorMenu_IOError();
    }

    // Do we need to switch map?
    // If that is the case we must warp to the next map and try to load the save on starting it.
    const bool bIsMapLoaded = (gbIsLevelDataCached && (SaveAndLoad::getBufferedSaveMapNum() == gGameMap));

    if (!bIsMapLoaded) {
        gbLoadSaveOnLevelStart = true;
        gGameMap = SaveAndLoad::getBufferedSaveMapNum();
        gStartMapOrEpisode = gGameMap;
        return ga_warped;
    }
    
    // We are already on the correct map!
    // Load the save game in-place and clear out the buffered save file data.
    const LoadSaveResult loadSaveResult = SaveAndLoad::load();
    SaveAndLoad::clearBufferedSave();

    // If that failed then show an error menu
    switch (loadSaveResult) {
        case LoadSaveResult::OK:            break;
        case LoadSaveResult::BAD_MAP_HASH:  return RunLoadGameErrorMenu_BadMapHash();
        case LoadSaveResult::BAD_MAP_DATA:  return RunLoadGameErrorMenu_BadMapData();
    }

    // Display what was loaded when we are loading in place and clear the currently loading slot
    DisplayLoadedHudMessage(loadContext, true);
    SaveAndLoad::gCurSaveSlot = SaveFileSlot::NONE;

    // Exit all menus and unpause the game
    gbUnpauseAfterOptionsMenu = true;
    return ga_exitmenus;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns and clears whether an attempt should be made to load the currently buffered save on loading the next level
//------------------------------------------------------------------------------------------------------------------------------------------
bool ShouldLoadSaveOnLevelStart() noexcept {
    return gbLoadSaveOnLevelStart;
}

void ClearLoadSaveOnLevelStartFlag() noexcept {
    gbLoadSaveOnLevelStart = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays a message for the specified save slot being saved to
//------------------------------------------------------------------------------------------------------------------------------------------
void DisplaySavedHudMessage(const SaveGameContext saveContext, const bool bSuccess) noexcept {
    const char* msg = nullptr;

    if (bSuccess) {
        switch (saveContext) {
            case SaveGameContext::Menu:         msg = "Game saved.";        break;
            case SaveGameContext::Autosave:     msg = "Game autosaved.";    break;
            case SaveGameContext::Quicksave:    msg = "Quicksaved.";        break;
        }
    } else {
        switch (saveContext) {
            case SaveGameContext::Menu:         msg = "Save failed!";       break;
            case SaveGameContext::Autosave:     msg = "Autosave failed!";   break;
            case SaveGameContext::Quicksave:    msg = "Quicksave failed!";  break;
        }
    }

    if (msg) {
        gStatusBar.message = msg;
        gStatusBar.messageTicsLeft = 30;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays a HUD message for the specified save slot being loaded from
//------------------------------------------------------------------------------------------------------------------------------------------
void DisplayLoadedHudMessage(const LoadGameContext loadContext, const bool bSuccess) noexcept {
    const char* msg = nullptr;

    if (bSuccess) {
        switch (loadContext) {
            case LoadGameContext::Menu:         msg = "Game loaded.";       break;
            case LoadGameContext::Quickload:    msg = "Quickloaded.";       break;
        }
    } else {
        switch (loadContext) {
            case LoadGameContext::Menu:         msg = "Load failed!";       break;
            case LoadGameContext::Quickload:    msg = "Quickload failed!";  break;
        }
    }

    if (msg) {
        gStatusBar.message = msg;
        gStatusBar.messageTicsLeft = 30;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: attempts to quicksave the game
//------------------------------------------------------------------------------------------------------------------------------------------
void DoQuicksave() noexcept {
    SaveGameForSlot(SaveFileSlot::QUICKSAVE, SaveGameContext::Quicksave);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: attempts to quickload the current quicksave
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t DoQuickload() noexcept {
    // Only do quickload if the file actually exists
    const std::string saveFilePath = SaveAndLoad::getSaveFilePath(SaveFileSlot::QUICKSAVE);

    if (FileUtils::fileExists(saveFilePath.c_str())) {
        return LoadGameForSlot(SaveFileSlot::QUICKSAVE, LoadGameContext::Quickload);
    } else {
        gStatusBar.message = "Quick.sav does not exist!";
        gStatusBar.messageTicsLeft = 30;
        return ga_nothing;
    }
}

#endif  // #if PSYDOOM_MODS
