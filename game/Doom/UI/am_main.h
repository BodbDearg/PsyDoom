#pragma once

#include <cstdint>

struct mobj_t;
struct player_t;

// The size of the triangles for thing displays on the automap
static constexpr int32_t AM_THING_TRI_SIZE = 24;

// Automap line colors: in 8-bit pairs for each color component, RGB order starting with the highest 8 bits
static constexpr uint32_t AM_COLOR_RED     = 0xA40000;
static constexpr uint32_t AM_COLOR_GREEN   = 0x00C000;
static constexpr uint32_t AM_COLOR_BROWN   = 0x8A5C30;
static constexpr uint32_t AM_COLOR_YELLOW  = 0xCCCC00;
static constexpr uint32_t AM_COLOR_GREY    = 0x808080;
static constexpr uint32_t AM_COLOR_AQUA    = 0x0080FF;

// PsyDoom additions to the color list.
// Also automap colors to use if deliberately brightening automap lines for the Vulkan renderer.
// The brightening is done to compensate for lines appearing dimmer, due to them being thinner at high resolutions.
#if PSYDOOM_MODS
    static constexpr uint32_t AM_COLOR_MAGENTA          = 0xC000C0;
    static constexpr uint32_t BRIGHT_AM_COLOR_RED       = 0xFF0000;
    static constexpr uint32_t BRIGHT_AM_COLOR_GREEN     = 0x00FF00;
    static constexpr uint32_t BRIGHT_AM_COLOR_BROWN     = 0xFFAA59;
    static constexpr uint32_t BRIGHT_AM_COLOR_YELLOW    = 0xFFFF00;
    static constexpr uint32_t BRIGHT_AM_COLOR_GREY      = 0xBBBBBB;
    static constexpr uint32_t BRIGHT_AM_COLOR_AQUA      = 0x0080FF;
    static constexpr uint32_t BRIGHT_AM_COLOR_MAGENTA   = 0xFF00FF;
#endif

void AM_Start() noexcept;
void AM_Control(player_t& player) noexcept;
void AM_Drawer() noexcept;

#if PSYDOOM_MODS
    uint32_t AM_GetPlayerColor(const int32_t playerIdx, const bool bBrighten) noexcept;
    uint32_t AM_GetMobjColor(const mobj_t& mobj, const bool bBrighten) noexcept;
#endif
