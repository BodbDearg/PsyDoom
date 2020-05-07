#include "Utils.h"

#include "Doom/Game/p_tick.h"
#include <SDL.h>

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
