#include "Utils.h"

#include "Doom/Game/p_tick.h"
#include "PsxVm/PsxVm.h"
#include "Video.h"

#include <SDL.h>
#include <thread>

BEGIN_NAMESPACE(Utils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Run update actions that have to be done periodically, including running the window and processing sound
//------------------------------------------------------------------------------------------------------------------------------------------
void do_platform_updates() noexcept {
    emulate_sound_if_required();
    Video::handleSdlWindowEvents();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for a number of seconds while still doing platform updates
//------------------------------------------------------------------------------------------------------------------------------------------
void wait_for_seconds(float seconds) noexcept {
    const clock_t startTime = clock();

    while (true) {
        const clock_t now = clock();
        const double elapsed = (double)(now - startTime) / (double) CLOCKS_PER_SEC;

        if (elapsed > seconds)
            break;
        
        do_platform_updates();
        thread_yield();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Yield some CPU time to the host machine
//------------------------------------------------------------------------------------------------------------------------------------------
void thread_yield() noexcept {
    std::this_thread::yield();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the cheat sequence to execute on the pause menu using helper shortcut keys in development mode.
// These save the pain of inputting button sequences and make it easy to cheat.
//------------------------------------------------------------------------------------------------------------------------------------------
cheatseq_t getDevCheatSequenceToExec() noexcept {
    // TODO: disable this in non developer builds, or at least provide a toggle (defaulted to off)
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
