#include "Utils.h"

#include "Doom/Game/p_tick.h"
#include "FatalErrors.h"
#include "Input.h"
#include "Network.h"
#include "ProgArgs.h"
#include "PsxVm.h"
#include "Video.h"
#include "Wess/psxcd.h"
#include "Wess/psxspu.h"
#include "Wess/wessapi.h"

#include <chrono>
#include <SDL.h>
#include <thread>

BEGIN_NAMESPACE(Utils)

static constexpr const char* const SAVE_FILE_ORG        = "com.codelobster";    // Root folder to save config in (in a OS specific writable prefs location)
static constexpr const char* const SAVE_FILE_PRODUCT    = "PsyDoom";            // Sub-folder within the root folder to save the config in

// Because typing this is a pain...
typedef std::chrono::high_resolution_clock::time_point timepoint_t;

// When we last did platform updates
static timepoint_t gLastPlatformUpdateTime = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the folder that PsyDoom uses for user config and save data.
// If the folder does not exist then it is created, and if that fails a fatal error is issued.
// The Path is returned with a trailing path separator, so can be combined with a file name without any other modifications.
//------------------------------------------------------------------------------------------------------------------------------------------
std::string getOrCreateUserDataFolder() noexcept {
    char* const pCfgFilePath = SDL_GetPrefPath(SAVE_FILE_ORG, SAVE_FILE_PRODUCT);

    if (!pCfgFilePath) {
        FatalErrors::raise("Unable to create or determine the user data folder (config/save folder) for PsyDoom!");
    }

    std::string path = pCfgFilePath;
    SDL_free(pCfgFilePath);
    return path;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Run update actions that have to be done periodically, including running the window and processing sound
//------------------------------------------------------------------------------------------------------------------------------------------
void doPlatformUpdates() noexcept {
    // In headless mode we can skip this entirely
    if (ProgArgs::gbHeadlessMode)
        return;

    // Only do updates if enough time has elapsed.
    // Do this to prevent excessive CPU usage in loops that are periodically trying to update sound etc. while waiting for some event.
    const timepoint_t now = std::chrono::high_resolution_clock::now();
    
    if (now - gLastPlatformUpdateTime < std::chrono::milliseconds(4))
        return;
    
    // Actually do the platform updates
    gLastPlatformUpdateTime = now;

    PsxVm::generateTimerEvents();
    Network::doUpdates();
    Input::update();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for a condition to be true and only abort if the condition is met or if the user has requested an app quit.
// Returns 'true' if the condition was met, or 'false' if aborted due to a user quit or because the game is in headless mode.
// While we are waiting video and the window are updated, so the application does not freeze and can be quit.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static bool waitForCond(const T& condLamba) noexcept {
    // We never wait in headless mode
    if (ProgArgs::gbHeadlessMode)
        return false;

    while (true) {
        // Is the condition now true?
        if (condLamba())
            return true;

        // Abort because the user asked for an app quit?
        if (Input::isQuitRequested())
            return false;

        // Ok have to wait for a bit, do platform updates and a refresh of the display.
        // Refreshing the display helps prevent a brief (temporary) stutter issue after long pauses - I'm not sure why, maybe an SDL bug?
        // Doing a vsync'd present also reduces idle CPU usage a lot more than a spinning loop with thread yield.
        Video::displayFramebuffer();
        Utils::threadYield();
        doPlatformUpdates();
    }

    // Should never get here!
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for a number of seconds while still doing platform updates; returns 'false' if wait was aborted
//------------------------------------------------------------------------------------------------------------------------------------------
bool waitForSeconds(const float seconds) noexcept {
    const clock_t startTime = clock();

    return waitForCond([&]() noexcept {
        const clock_t now = clock();
        const double elapsed = (double)(now - startTime) / (double) CLOCKS_PER_SEC;
        return (elapsed >= seconds);
    });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Implements an original PSX Doom wait loop with tweaks for PC to keep the app responsive; returns 'false' if the wait was aborted.
// Waits until at least 1 CD audio sector has been read before continuing.
//------------------------------------------------------------------------------------------------------------------------------------------
bool waitForCdAudioPlaybackStart() noexcept {
    return waitForCond([]() noexcept {
        return (psxcd_elapsed_sectors() != 0);
    });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Implements an original PSX Doom wait loop with tweaks for PC to keep the app responsive; returns 'false' if the wait was aborted.
// Waits until a specified music or sound sequence has exited the specified status or the application is quitting.
//------------------------------------------------------------------------------------------------------------------------------------------
bool waitUntilSeqEnteredStatus(const int32_t sequenceIdx, const SequenceStatus status) noexcept {
    return waitForCond([=]() noexcept {
        return (wess_seq_status(sequenceIdx) == status);
    });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Implements an original PSX Doom wait loop with tweaks for PC to keep the app responsive; returns 'false' if the wait was aborted.
// Waits until a specified music or sound sequence has exited the specified status or the application is quitting.
//------------------------------------------------------------------------------------------------------------------------------------------
bool waitUntilSeqExitedStatus(const int32_t sequenceIdx, const SequenceStatus status) noexcept {
    return waitForCond([=]() noexcept {
        return (wess_seq_status(sequenceIdx) != status);
    });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Implements an original PSX Doom wait loop with tweaks for PC to keep the app responsive; returns 'false' if the wait was aborted.
// Waits until at CD audio has finished fading out.
//------------------------------------------------------------------------------------------------------------------------------------------
bool waitForCdAudioFadeOut() noexcept {
    return waitForCond([=]() noexcept {
        return (!psxspu_get_cd_fade_status());
    });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Yield some CPU time to the host machine
//------------------------------------------------------------------------------------------------------------------------------------------
void threadYield() noexcept {
    std::this_thread::yield();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the cheat sequence to execute on the pause menu using helper shortcut keys in development mode.
// These save the pain of inputting button sequences and make it easy to cheat.
//------------------------------------------------------------------------------------------------------------------------------------------
cheatseq_t getDevCheatSequenceToExec() noexcept {
    // TODO: getDevCheatSequenceToExec: disable this in non developer builds, or at least provide a toggle (defaulted to off)
    const uint8_t* const pKbState = SDL_GetKeyboardState(nullptr);

    if (pKbState[SDL_SCANCODE_F1] != 0) {
        return CHT_SEQ_GOD_MODE;
    } else if (pKbState[SDL_SCANCODE_F2] != 0) {
        return CHT_SEQ_NOCLIP;
    } else if (pKbState[SDL_SCANCODE_F3] != 0) {
        return CHT_SEQ_WEAPONS_AND_AMMO;
    } else if (pKbState[SDL_SCANCODE_F4] != 0) {
        return CHT_SEQ_LEVEL_WARP;
    } else if (pKbState[SDL_SCANCODE_F5] != 0) {
        return CHT_SEQ_XRAY_VISION;
    } else if (pKbState[SDL_SCANCODE_F6] != 0) {
        return CHT_SEQ_SHOW_ALL_MAP_THINGS;
    } else if (pKbState[SDL_SCANCODE_F7] != 0) {
        return CHT_SEQ_SHOW_ALL_MAP_LINES;
    } else if (pKbState[SDL_SCANCODE_F8] != 0) {
        return CHT_SEQ_VRAM_VIEWER;
    }

    return (cheatseq_t) UINT32_MAX;
}

END_NAMESPACE(Utils)
