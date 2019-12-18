#pragma once

#include <cstdint>

// Represents a high level result of running a game loop (MiniLoop).
// Determines where the game navigates to next.
enum gameaction_t : uint32_t {
    ga_nothing      = 0,        // No game action
    ga_died         = 7,        // Player died or menu timed out
    ga_exitdemo     = 9,        // Player aborted the demo screens
};

// Maximum number of ticks in a demo.
// The maximum allowed demo size is 16384 ticks (demo size 64 KiB).
const int32_t MAX_DEMO_TICKS = 16384;
