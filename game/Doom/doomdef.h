#pragma once

#include <cstdint>

// Represents a high level result of running a game loop (MiniLoop).
// Determines where the game navigates to next.
enum gameaction_t : uint32_t {
    ga_died         = 7,        // Player died or menu timed out
    ga_exitdemo     = 9,        // Player aborted the demo screens
};
