#pragma once

#include "GamepadInput.h"
#include "Macros.h"
#include "MouseButton.h"
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper functions that manage user events and inputs received from the SDL library.
//
// Note that gamepad inputs can come from both the 'Game Controller' or more generic 'Joystick' APIs of SDL.
// Some controllers will only be supported through the generic 'Joystick' API however, and not be recognized by SDL as a game controller.
//------------------------------------------------------------------------------------------------------------------------------------------
BEGIN_NAMESPACE(Input)

// N.B: MUST match the SDL headers! (I don't want to expose SDL via this header)
// Verified via static assert in the .cpp file. 
inline static constexpr uint16_t NUM_KEYBOARD_KEYS = 512;

// The number of mouse wheel axes there are.
// 0 = x-axis and 1 = y-axis.
// There may be a y-axis on some laptop touch pads:
inline static constexpr uint8_t NUM_MOUSE_WHEEL_AXES = 2;

// Direction for a joystick hat (d-pad)
enum JoyHatDir : uint16_t {
    Up      = 0,
    Down    = 1,
    Left    = 2,
    Right   = 3
};

// Holds a joystick hat (d-pad) direction and hat number
union JoyHat {
    struct {
        JoyHatDir   hatDir : 2;
        uint16_t    hatNum : 14;
    } fields;

    uint16_t bits;

    inline JoyHat() noexcept : bits() {}
    inline JoyHat(const uint16_t bits) noexcept : bits(bits) {}

    inline JoyHat(const JoyHatDir dir, const uint16_t hatNum) noexcept : bits() {
        fields.hatDir = dir;
        fields.hatNum = hatNum & 0x3FFF;
    }

    inline operator uint16_t() const noexcept { return bits; }

    inline bool operator == (const JoyHat& other) const noexcept { return (bits == other.bits); }
    inline bool operator != (const JoyHat& other) const noexcept { return (bits != other.bits); }
};

static_assert(sizeof(JoyHat) == 2);

void init() noexcept;
void shutdown() noexcept;
void update() noexcept;
void consumeEvents() noexcept;
void consumeMouseMovements() noexcept;
bool isQuitRequested() noexcept;
void requestQuit() noexcept;
bool areAnyKeysOrButtonsPressed() noexcept;

// Get a list of things pressed, just pressed or just released
const std::vector<uint16_t>& getKeyboardKeysPressed() noexcept;
const std::vector<uint16_t>& getKeyboardKeysJustPressed() noexcept;
const std::vector<uint16_t>& getKeyboardKeysJustReleased() noexcept;

const std::vector<MouseButton>& getMouseButtonsPressed() noexcept;
const std::vector<MouseButton>& getMouseButtonsJustPressed() noexcept;
const std::vector<MouseButton>& getMouseButtonsJustReleased() noexcept;

const std::vector<GamepadInput>& getGamepadInputsPressed() noexcept;
const std::vector<GamepadInput>& getGamepadInputsJustPressed() noexcept;
const std::vector<GamepadInput>& getGamepadInputsJustReleased() noexcept;

const std::vector<uint32_t>& getJoystickAxesPressed() noexcept;
const std::vector<uint32_t>& getJoystickAxesJustPressed() noexcept;
const std::vector<uint32_t>& getJoystickAxesJustReleased() noexcept;

const std::vector<uint32_t>& getJoystickButtonsPressed() noexcept;
const std::vector<uint32_t>& getJoystickButtonsJustPressed() noexcept;
const std::vector<uint32_t>& getJoystickButtonsJustReleased() noexcept;

const std::vector<JoyHat>& getJoystickHatsPressed() noexcept;
const std::vector<JoyHat>& getJoystickHatsJustPressed() noexcept;
const std::vector<JoyHat>& getJoystickHatsJustReleased() noexcept;

// Query input state and whether something is just pressed or released
bool isKeyboardKeyPressed(const uint16_t key) noexcept;
bool isKeyboardKeyJustPressed(const uint16_t key) noexcept;
bool isKeyboardKeyReleased(const uint16_t key) noexcept;
bool isKeyboardKeyJustReleased(const uint16_t key) noexcept;

bool isMouseButtonPressed(const MouseButton button) noexcept;
bool isMouseButtonJustPressed(const MouseButton button) noexcept;
bool isMouseButtonReleased(const MouseButton button) noexcept;
bool isMouseButtonJustReleased(const MouseButton button) noexcept;

bool isGamepadInputPressed(const GamepadInput input) noexcept;
bool isGamepadInputJustPressed(const GamepadInput input) noexcept;
bool isGamepadInputJustReleased(const GamepadInput input) noexcept;

bool isJoystickAxisPressed(const uint32_t axis) noexcept;
bool isJoystickAxisJustPressed(const uint32_t axis) noexcept;
bool isJoystickAxisJustReleased(const uint32_t axis) noexcept;

bool isJoystickButtonPressed(const uint32_t button) noexcept;
bool isJoystickButtonJustPressed(const uint32_t button) noexcept;
bool isJoystickButtonJustReleased(const uint32_t button) noexcept;

bool isJoystickHatPressed(const JoyHat hat) noexcept;
bool isJoystickHatJustPressed(const JoyHat hat) noexcept;
bool isJoystickHatJustReleased(const JoyHat hat) noexcept;

// Gamepad & generic joystick axis inputs
float getGamepadInputValue(const GamepadInput input) noexcept;
float getJoystickAxisValue(const uint32_t axis) noexcept;

float getAdjustedGamepadInputValue(const GamepadInput input, float deadZone) noexcept;
float getAdjustedJoystickAxisValue(const uint32_t axis, const float deadZone) noexcept;

// Mouse inputs
float getMouseXMovement() noexcept;
float getMouseYMovement() noexcept;
float getMouseWheelAxisMovement(const uint8_t axis) noexcept;

END_NAMESPACE(Input)
