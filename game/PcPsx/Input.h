#pragma once

#include "ControllerInput.h"
#include "MouseButton.h"
#include "Macros.h"
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper functions that manage user events and inputs received from the SDL library
//------------------------------------------------------------------------------------------------------------------------------------------
BEGIN_NAMESPACE(Input)

// N.B: MUST match the SDL headers! (I don't want to expose SDL via this header)
// Verified via static assert in the .cpp file. 
inline static constexpr uint16_t NUM_KEYBOARD_KEYS = 512;

// The number of mouse wheel axes there are.
// 0 = x-axis and 1 = y-axis.
// There may be a y-axis on some laptop touch pads:
inline static constexpr uint8_t NUM_MOUSE_WHEEL_AXES = 2;

void init() noexcept;
void shutdown() noexcept;
void update() noexcept;
void consumeEvents() noexcept;
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

const std::vector<ControllerInput>& getControllerInputsPressed() noexcept;
const std::vector<ControllerInput>& getControllerInputsJustPressed() noexcept;
const std::vector<ControllerInput>& getControllerInputsJustReleased() noexcept;

// Query input state and whether something is just pressed or released
bool isKeyboardKeyPressed(const uint16_t key) noexcept;
bool isKeyboardKeyJustPressed(const uint16_t key) noexcept;
bool isKeyboardKeyReleased(const uint16_t key) noexcept;
bool isKeyboardKeyJustReleased(const uint16_t key) noexcept;

bool isMouseButtonPressed(const MouseButton button) noexcept;
bool isMouseButtonJustPressed(const MouseButton button) noexcept;
bool isMouseButtonReleased(const MouseButton button) noexcept;
bool isMouseButtonJustReleased(const MouseButton button) noexcept;

bool isControllerInputPressed(const ControllerInput input) noexcept;
bool isControllerInputJustPressed(const ControllerInput input) noexcept;
bool isControllerInputJustReleased(const ControllerInput input) noexcept;
float getControllerInputValue(const ControllerInput input) noexcept;
float getAdjustedControllerInputValue(const ControllerInput input, float deadZone) noexcept;

// Get the amount of mouse movement this frame on the x and y axes
float getMouseXMovement() noexcept;
float getMouseYMovement() noexcept;

// The the current movement amount for a mouse wheel axis
float getMouseWheelAxisMovement(const uint8_t axis) noexcept;

END_NAMESPACE(Input)
