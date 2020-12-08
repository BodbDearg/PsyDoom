#pragma once

#include "Macros.h"

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Enum representing an input source on a non-generic game controller recognized by SDL (axis or button)
//------------------------------------------------------------------------------------------------------------------------------------------
enum class GamepadInput : uint8_t {
    BTN_A,
    BTN_B,
    BTN_X,
    BTN_Y,
    BTN_BACK,
    BTN_GUIDE,
    BTN_START,
    BTN_LEFT_STICK,
    BTN_RIGHT_STICK,
    BTN_LEFT_SHOULDER,
    BTN_RIGHT_SHOULDER,
    BTN_DPAD_UP,
    BTN_DPAD_DOWN,
    BTN_DPAD_LEFT,
    BTN_DPAD_RIGHT,
    AXIS_LEFT_X,
    AXIS_LEFT_Y,
    AXIS_RIGHT_X,
    AXIS_RIGHT_Y,
    AXIS_TRIG_LEFT,
    AXIS_TRIG_RIGHT,

    // N.B: must keep last for 'NUM_GAMEPAD_INPUTS' constant!
    INVALID
};

static constexpr uint8_t NUM_GAMEPAD_INPUTS = (uint32_t) GamepadInput::INVALID;

BEGIN_NAMESPACE(GamepadInputUtils)

bool isAxis(const GamepadInput input) noexcept;
GamepadInput getOppositeAxis(const GamepadInput input) noexcept;
GamepadInput sdlButtonToInput(const uint8_t button) noexcept;
GamepadInput sdlAxisToInput(const uint8_t axis) noexcept;

END_NAMESPACE(GamepadInputUtils)
