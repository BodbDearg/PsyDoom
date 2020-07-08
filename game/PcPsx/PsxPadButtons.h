#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Header containing bit masks for PlayStation digital controller buttons, as well as mouse buttons.
// These correspond to the original bitmask values used in the PsyQ SDK.
//------------------------------------------------------------------------------------------------------------------------------------------
#include <cstdint>

static constexpr uint16_t PAD_UP            = 0x1000;
static constexpr uint16_t PAD_DOWN          = 0x4000;
static constexpr uint16_t PAD_LEFT          = 0x8000;
static constexpr uint16_t PAD_RIGHT         = 0x2000;
static constexpr uint16_t PAD_TRIANGLE      = 0x10;
static constexpr uint16_t PAD_CROSS         = 0x40;
static constexpr uint16_t PAD_SQUARE        = 0x80;
static constexpr uint16_t PAD_CIRCLE        = 0x20;
static constexpr uint16_t PAD_L1            = 0x4;
static constexpr uint16_t PAD_L2            = 0x1;
static constexpr uint16_t PAD_R1            = 0x8;
static constexpr uint16_t PAD_R2            = 0x2;
static constexpr uint16_t PAD_START         = 0x800;
static constexpr uint16_t PAD_SELECT        = 0x100;
static constexpr uint16_t PSX_MOUSE_LEFT    = 0x400;
static constexpr uint16_t PSX_MOUSE_RIGHT   = 0x200;

static constexpr uint16_t PAD_DIRECTION_BTNS = (
    PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT
);

static constexpr uint16_t PAD_ACTION_BTNS = (
    PAD_TRIANGLE | PAD_CROSS | PAD_SQUARE | PAD_CIRCLE
);

static constexpr uint16_t PAD_SHOULDER_BTNS = (
    PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2
);

static constexpr uint16_t PAD_ANY_BTNS = (
    PAD_DIRECTION_BTNS | PAD_ACTION_BTNS | PAD_SHOULDER_BTNS | PAD_START | PAD_SELECT
);

static constexpr uint16_t PSX_MOUSE_ANY_BTNS = (
    PSX_MOUSE_LEFT | PSX_MOUSE_RIGHT
);
