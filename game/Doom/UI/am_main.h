#pragma once

#include <cstdint>

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

void AM_Start() noexcept;
void AM_Control(player_t& player) noexcept;
void AM_Drawer() noexcept;
