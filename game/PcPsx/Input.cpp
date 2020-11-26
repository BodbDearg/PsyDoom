#include "Input.h"

#include "ControllerInput.h"
#include "Config.h"
#include "FatalErrors.h"
#include "ProgArgs.h"
#include "PsxVm.h"
#include "Video.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <SDL.h>

BEGIN_NAMESPACE(Input)

// Holds the current state of a generic joystick axis
struct JoystickAxis {
    uint32_t    axis;
    float       value;
};

static bool                             gbIsQuitRequested;
static const Uint8*                     gpKeyboardState;
static int                              gNumKeyboardStateKeys;
static std::vector<uint16_t>            gKeyboardKeysPressed;
static std::vector<uint16_t>            gKeyboardKeysJustPressed;
static std::vector<uint16_t>            gKeyboardKeysJustReleased;
static std::vector<MouseButton>         gMouseButtonsPressed;
static std::vector<MouseButton>         gMouseButtonsJustPressed;
static std::vector<MouseButton>         gMouseButtonsJustReleased;
static float                            gControllerInputs[NUM_CONTROLLER_INPUTS];
static std::vector<ControllerInput>     gControllerInputsPressed;
static std::vector<ControllerInput>     gControllerInputsJustPressed;
static std::vector<ControllerInput>     gControllerInputsJustReleased;
static std::vector<JoystickAxis>        gJoystickAxes;
static std::vector<uint32_t>            gJoystickAxesPressed;
static std::vector<uint32_t>            gJoystickAxesJustPressed;
static std::vector<uint32_t>            gJoystickAxesJustReleased;
static std::vector<uint32_t>            gJoystickButtonsPressed;
static std::vector<uint32_t>            gJoystickButtonsJustPressed;
static std::vector<uint32_t>            gJoystickButtonsJustReleased;
static std::vector<JoyHat>              gJoystickHatsPressed;
static std::vector<JoyHat>              gJoystickHatsJustPressed;
static std::vector<JoyHat>              gJoystickHatsJustReleased;

static SDL_GameController*  gpGameController;
static SDL_Joystick*        gpJoystick;             // Note: this is managed by game controller, not freed by this code!
static SDL_JoystickID       gJoystickId;

// Mouse movements this frame
static float gMouseMovementX;
static float gMouseMovementY;
static float gMouseWheelAxisMovements[NUM_MOUSE_WHEEL_AXES];

//------------------------------------------------------------------------------------------------------------------------------------------
// Vector utility functions
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static inline bool vectorContainsValue(const std::vector<T>& vec, const T val) noexcept {
    const auto endIter = vec.end();
    const auto iter = std::find(vec.begin(), endIter, val);
    return (iter != endIter);
}

template <class T>
static inline void removeValueFromVector(const T val, std::vector<T>& vec) noexcept {
    auto endIter = vec.end();
    auto iter = std::find(vec.begin(), endIter, val);

    while (iter != endIter) {
        iter = vec.erase(iter);
        endIter = vec.end();
        iter = std::find(iter, endIter, val);
    }
}

template <class T>
static inline void emptyAndShrinkVector(std::vector<T>& vec) noexcept {
    vec.clear();
    vec.shrink_to_fit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update the value of a joystick axis in the vector of values: removes the value if it has reached '0'
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateJoystickAxisValue(const uint32_t axis, const float value) noexcept {
    // Search for the existing value of this axis: will need to remove or update it if found
    auto iter = std::find_if(gJoystickAxes.begin(), gJoystickAxes.end(), [=](const JoystickAxis& axisValue) noexcept { return (axisValue.axis == axis); });

    if (value == 0.0f) {
        if (iter != gJoystickAxes.end()) {
            gJoystickAxes.erase(iter);
        }
    }
    else {
        if (iter != gJoystickAxes.end()) {
            iter->value = value;
        } else {
            gJoystickAxes.emplace_back(JoystickAxis{ axis, value});
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis value to a -1 to + 1 range float.
//------------------------------------------------------------------------------------------------------------------------------------------
static float sdlAxisValueToFloat(const int16_t axis) noexcept {
    if (axis >= 0) {
        return (float) axis / 32767.0f;
    } else {
        return (float) axis / 32768.0f;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Close the currently open game controller (if any).
// Also clears up any related inputs.
//------------------------------------------------------------------------------------------------------------------------------------------
static void closeCurrentGameController() noexcept {
    std::memset(gControllerInputs, 0, sizeof(gControllerInputs));
    gControllerInputsPressed.clear();
    gControllerInputsJustPressed.clear();
    gControllerInputsJustReleased.clear();

    gJoystickAxes.clear();
    gJoystickAxesPressed.clear();
    gJoystickAxesJustPressed.clear();
    gJoystickAxesJustReleased.clear();

    gJoystickButtonsPressed.clear();
    gJoystickButtonsJustPressed.clear();
    gJoystickButtonsJustReleased.clear();

    gJoystickHatsPressed.clear();
    gJoystickHatsJustPressed.clear();
    gJoystickHatsJustReleased.clear();

    if (gpGameController) {
        SDL_GameControllerClose(gpGameController);
        gpGameController = nullptr;
    }

    gpJoystick = nullptr;
    gJoystickId = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Rescans for the SDL game controllers to use: just uses the first available game controller.
// This may choose wrong in a multi-gamepad situation but the user can always disconnect one to clarify what is wanted.
// Most computer users would probably only want one gamepad at a time anyway?
//------------------------------------------------------------------------------------------------------------------------------------------
static void rescanGameControllers() noexcept {
    // If we already have a gamepad then just re-check that it is still connected
    if (gpGameController) {
        if (!SDL_GameControllerGetAttached(gpGameController)) {
            closeCurrentGameController();
        }
    }

    // See if there are any joysticks connected.
    // Note: a return of < 0 means an error, which we will ignore:
    const int numJoysticks = SDL_NumJoysticks();

    for (int joyIdx = 0; joyIdx < numJoysticks; ++joyIdx) {
        // If we find a valid game controller then try to open it.
        // If we succeed then our work is done!
        if (SDL_IsGameController(joyIdx)) {
            gpGameController = SDL_GameControllerOpen(joyIdx);

            if (gpGameController) {
                gpJoystick = SDL_GameControllerGetJoystick(gpGameController);
                gJoystickId = SDL_JoystickInstanceID(gpJoystick);
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update the status for the specified joystick hat and generate events if required
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateJoystickHat(const JoyHat hat, const bool bPressed) noexcept {
    if (bPressed) {
        // Just pressed?
        if (!vectorContainsValue(gJoystickHatsPressed, hat)) {
            removeValueFromVector(hat, gJoystickHatsJustReleased);
            gJoystickHatsPressed.push_back(hat);
            gJoystickHatsJustPressed.push_back(hat);
        }
    }
    else {
        // Just released?
        if (vectorContainsValue(gJoystickHatsPressed, hat)) {
            gJoystickHatsJustReleased.push_back(hat);
            removeValueFromVector(hat, gJoystickHatsPressed);
            removeValueFromVector(hat, gJoystickHatsJustPressed);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle events sent by SDL (keypresses and such)
//------------------------------------------------------------------------------------------------------------------------------------------
static void handleSdlEvents() noexcept {
    SDL_Event sdlEvent;
    bool bConsumeEvents = false;
    
    while (SDL_PollEvent(&sdlEvent) != 0) {
        switch (sdlEvent.type) {
            case SDL_QUIT:
                // The application is requesting to quit
                gbIsQuitRequested = true;
                break;
            
            case SDL_WINDOWEVENT: {
                switch (sdlEvent.window.event) {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        // Note: don't grab mouse input here, wait until the window's client area is actually clicked.
                        // This makes resizing and such easier in windowed mode.
                        bConsumeEvents = true;
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        SDL_ShowCursor(SDL_ENABLE);
                        SDL_SetWindowGrab(Video::getWindow(), SDL_FALSE);
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        break;
                }
            }   break;
            
            case SDL_KEYDOWN: {
                const uint16_t scancode = (uint16_t) sdlEvent.key.keysym.scancode;

                if (scancode < NUM_KEYBOARD_KEYS) {
                    removeValueFromVector(scancode, gKeyboardKeysJustReleased);
                    gKeyboardKeysPressed.push_back(scancode);
                    gKeyboardKeysJustPressed.push_back(scancode);
                }
            }   break;

            case SDL_KEYUP: {
                const uint16_t scancode = (uint16_t) sdlEvent.key.keysym.scancode;

                if (scancode < NUM_KEYBOARD_KEYS) {
                    removeValueFromVector(scancode, gKeyboardKeysPressed);
                    removeValueFromVector(scancode, gKeyboardKeysJustPressed);
                    gKeyboardKeysJustReleased.push_back(scancode);
                }
            }   break;

            case SDL_MOUSEBUTTONDOWN: {
                // Capture the mouse on a click if we haven't captured it yet
                if (!SDL_GetRelativeMouseMode()) {
                    SDL_ShowCursor(SDL_DISABLE);
                    SDL_SetWindowGrab(Video::getWindow(), SDL_TRUE);
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    bConsumeEvents = true;
                }

                // Handle the button click
                const MouseButton button = (MouseButton)(sdlEvent.button.button - 1);

                if ((uint8_t) button < NUM_MOUSE_BUTTONS) {
                    removeValueFromVector(button, gMouseButtonsJustReleased);
                    gMouseButtonsPressed.push_back(button);
                    gMouseButtonsJustPressed.push_back(button);
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                const MouseButton button = (MouseButton)(sdlEvent.button.button - 1);

                if ((uint8_t) button < NUM_MOUSE_BUTTONS) {
                    removeValueFromVector(button, gMouseButtonsPressed);
                    removeValueFromVector(button, gMouseButtonsJustPressed);
                    gMouseButtonsJustReleased.push_back(button);
                }
            } break;

            case SDL_MOUSEMOTION: {
                // Only register movement if we have captured the mouse
                if (SDL_GetRelativeMouseMode()) {
                    gMouseMovementX += (float) sdlEvent.motion.xrel;
                    gMouseMovementY += (float) sdlEvent.motion.yrel;
                } else {
                    gMouseMovementX = 0.0f;
                    gMouseMovementY = 0.0f;
                }
            } break;

            case SDL_MOUSEWHEEL: {
                // Only register movement if we have captured the mouse
                if (SDL_GetRelativeMouseMode()) {
                    gMouseWheelAxisMovements[0] += (float) sdlEvent.wheel.x;
                    gMouseWheelAxisMovements[1] += (float) sdlEvent.wheel.y;
                } else {
                    gMouseWheelAxisMovements[0] = 0.0f;
                    gMouseWheelAxisMovements[1] = 0.0f;
                }
            } break;

            case SDL_CONTROLLERAXISMOTION: {
                if (sdlEvent.cbutton.which == gJoystickId) {
                    const ControllerInput input = ControllerInputUtils::sdlAxisToInput(sdlEvent.caxis.axis);

                    if (input != ControllerInput::INVALID) {
                        const float pressedThreshold = Config::gAnalogToDigitalThreshold;
                        const uint8_t inputIdx = (uint8_t) input;

                        // See if there is a change in the 'pressed' status
                        const bool bPrevPressed = (std::abs(gControllerInputs[inputIdx]) >= pressedThreshold);
                        const float inputF = sdlAxisValueToFloat(sdlEvent.caxis.value);
                        const float inputFAbs = std::abs(inputF);
                        const bool bNowPressed = (inputFAbs >= pressedThreshold);

                        // Update input value
                        gControllerInputs[inputIdx] = inputF;

                        // Generate events for the analog input
                        if (bPrevPressed != bNowPressed) {
                            if (bNowPressed) {
                                removeValueFromVector(input, gControllerInputsJustReleased);
                                gControllerInputsPressed.push_back(input);
                                gControllerInputsJustPressed.push_back(input);
                            } else {
                                removeValueFromVector(input, gControllerInputsPressed);
                                removeValueFromVector(input, gControllerInputsJustPressed);
                                gControllerInputsJustReleased.push_back(input);
                            }
                        }
                    }
                }
            }   break;

            case SDL_JOYAXISMOTION: {
                if (sdlEvent.jaxis.which == gJoystickId) {
                    // See if there is a change in the 'pressed' status
                    const float pressedThreshold = Config::gAnalogToDigitalThreshold;
                    const uint32_t axis = sdlEvent.jaxis.axis;

                    const bool bPrevPressed = (std::abs(Input::getJoystickAxisValue(axis)) >= pressedThreshold);
                    const float inputF = sdlAxisValueToFloat(sdlEvent.jaxis.value);
                    const float inputFAbs = std::abs(inputF);
                    const bool bNowPressed = (inputFAbs >= pressedThreshold);

                    // Update input value
                    updateJoystickAxisValue(axis, inputF);

                    // Generate events for the analog input
                    if (bPrevPressed != bNowPressed) {
                        if (bNowPressed) {
                            removeValueFromVector(axis, gJoystickAxesJustReleased);
                            gJoystickAxesPressed.push_back(axis);
                            gJoystickAxesJustPressed.push_back(axis);
                        } else {
                            removeValueFromVector(axis, gJoystickAxesPressed);
                            removeValueFromVector(axis, gJoystickAxesJustPressed);
                            gJoystickAxesJustReleased.push_back(axis);
                        }
                    }
                }
            }   break;

            case SDL_CONTROLLERBUTTONDOWN: {
                if (sdlEvent.cbutton.which == gJoystickId) {
                    const ControllerInput input = ControllerInputUtils::sdlButtonToInput(sdlEvent.cbutton.button);

                    if (input != ControllerInput::INVALID) {
                        removeValueFromVector(input, gControllerInputsJustReleased);
                        gControllerInputsPressed.push_back(input);
                        gControllerInputsJustPressed.push_back(input);
                        gControllerInputs[(uint8_t) input] = 1.0f;
                    }
                }
            }   break;

            case SDL_JOYBUTTONDOWN: {
                if (sdlEvent.jbutton.which == gJoystickId) {
                    const uint32_t button = sdlEvent.jbutton.button;
                    removeValueFromVector(button, gJoystickButtonsJustReleased);
                    gJoystickButtonsPressed.push_back(button);
                    gJoystickButtonsJustPressed.push_back(button);
                }
            }   break;

            case SDL_CONTROLLERBUTTONUP: {
                if (sdlEvent.cbutton.which == gJoystickId) {
                    const ControllerInput input = ControllerInputUtils::sdlButtonToInput(sdlEvent.cbutton.button);

                    if (input != ControllerInput::INVALID) {
                        gControllerInputsJustReleased.push_back(input);
                        removeValueFromVector(input, gControllerInputsPressed);
                        removeValueFromVector(input, gControllerInputsJustPressed);
                        gControllerInputs[(uint8_t) input] = 0.0f;
                    }
                }
            }   break;

            case SDL_JOYBUTTONUP: {
                if (sdlEvent.jbutton.which == gJoystickId) {
                    const uint32_t button = sdlEvent.jbutton.button;
                    gJoystickButtonsJustReleased.push_back(button);
                    removeValueFromVector(button, gJoystickButtonsPressed);
                    removeValueFromVector(button, gJoystickButtonsJustPressed);
                }
            }   break;

            case SDL_JOYHATMOTION: {
                if (sdlEvent.jhat.which == gJoystickId) {
                    const uint16_t hatNum = sdlEvent.jhat.hat;
                    updateJoystickHat(JoyHat(JoyHatDir::Up, hatNum), (sdlEvent.jhat.value & SDL_HAT_UP));
                    updateJoystickHat(JoyHat(JoyHatDir::Down, hatNum), (sdlEvent.jhat.value & SDL_HAT_DOWN));
                    updateJoystickHat(JoyHat(JoyHatDir::Left, hatNum), (sdlEvent.jhat.value & SDL_HAT_LEFT));
                    updateJoystickHat(JoyHat(JoyHatDir::Right, hatNum), (sdlEvent.jhat.value & SDL_HAT_RIGHT));
                }
            }   break;

            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
                rescanGameControllers();
                break;
        }
    }

    if (bConsumeEvents) {
        consumeEvents();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize input handling
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        FatalErrors::raise("Failed to initialize the SDL joystick input subsystem!");
    }

    SDL_GameControllerEventState(SDL_ENABLE);       // Want game controller events

    gbIsQuitRequested = false;
    
    gpKeyboardState = SDL_GetKeyboardState(&gNumKeyboardStateKeys);
    gKeyboardKeysJustPressed.reserve(16);
    gKeyboardKeysJustReleased.reserve(16);

    gMouseButtonsPressed.reserve(NUM_MOUSE_BUTTONS);
    gMouseButtonsJustPressed.reserve(NUM_MOUSE_BUTTONS);
    gMouseButtonsJustReleased.reserve(NUM_MOUSE_BUTTONS);

    gControllerInputsPressed.reserve(NUM_CONTROLLER_INPUTS);
    gControllerInputsJustPressed.reserve(NUM_CONTROLLER_INPUTS);
    gControllerInputsJustReleased.reserve(NUM_CONTROLLER_INPUTS);

    gJoystickAxes.reserve(16);
    gJoystickAxesPressed.reserve(16);
    gJoystickAxesJustPressed.reserve(16);
    gJoystickAxesJustReleased.reserve(16);

    gJoystickButtonsPressed.reserve(32);
    gJoystickButtonsJustPressed.reserve(32);
    gJoystickButtonsJustReleased.reserve(32);

    gJoystickHatsPressed.reserve(32);
    gJoystickHatsJustPressed.reserve(32);
    gJoystickHatsJustReleased.reserve(32);

    gMouseMovementX = 0.0f;
    gMouseMovementY = 0.0f;

    rescanGameControllers();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown input handling
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    consumeEvents();
    closeCurrentGameController();

    gMouseMovementX = 0.0f;
    gMouseMovementY = 0.0f;

    emptyAndShrinkVector(gJoystickHatsJustReleased);
    emptyAndShrinkVector(gJoystickHatsJustPressed);
    emptyAndShrinkVector(gJoystickHatsPressed);

    emptyAndShrinkVector(gJoystickButtonsJustReleased);
    emptyAndShrinkVector(gJoystickButtonsJustPressed);
    emptyAndShrinkVector(gJoystickButtonsPressed);
    
    emptyAndShrinkVector(gJoystickAxesJustReleased);
    emptyAndShrinkVector(gJoystickAxesJustPressed);
    emptyAndShrinkVector(gJoystickAxesPressed);
    emptyAndShrinkVector(gJoystickAxes);

    emptyAndShrinkVector(gControllerInputsJustReleased);
    emptyAndShrinkVector(gControllerInputsJustPressed);
    emptyAndShrinkVector(gControllerInputsPressed);

    emptyAndShrinkVector(gMouseButtonsJustReleased);
    emptyAndShrinkVector(gMouseButtonsJustPressed);
    emptyAndShrinkVector(gMouseButtonsPressed);

    emptyAndShrinkVector(gKeyboardKeysJustReleased);
    emptyAndShrinkVector(gKeyboardKeysJustPressed);
    emptyAndShrinkVector(gKeyboardKeysPressed);

    gpKeyboardState = nullptr;
    gbIsQuitRequested = false;

    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Generates input events like key down; should be called once per frame
//------------------------------------------------------------------------------------------------------------------------------------------
void update() noexcept {
    if (!ProgArgs::gbHeadlessMode) {
        handleSdlEvents();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Discards input events and movements.
// Should be called whenever inputs have been processed for a frame.
//------------------------------------------------------------------------------------------------------------------------------------------
void consumeEvents() noexcept {
    // Clear all events
    gKeyboardKeysJustPressed.clear();
    gKeyboardKeysJustReleased.clear();
    gMouseButtonsJustPressed.clear();
    gMouseButtonsJustReleased.clear();
    gControllerInputsJustPressed.clear();
    gControllerInputsJustReleased.clear();
    gJoystickAxesJustPressed.clear();
    gJoystickAxesJustReleased.clear();
    gJoystickButtonsJustPressed.clear();
    gJoystickButtonsJustReleased.clear();
    gJoystickHatsJustPressed.clear();
    gJoystickHatsJustReleased.clear();

    // Clear all movement deltas
    static_assert(NUM_MOUSE_WHEEL_AXES == 2);
    gMouseWheelAxisMovements[0] = 0.0f;
    gMouseWheelAxisMovements[1] = 0.0f;

    consumeMouseMovements();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Consumes all current mouse movement deltas
//------------------------------------------------------------------------------------------------------------------------------------------
void consumeMouseMovements() noexcept {
    gMouseMovementX = 0;
    gMouseMovementY = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns true if the user requested that the app be quit via close button
//------------------------------------------------------------------------------------------------------------------------------------------
bool isQuitRequested() noexcept {
    return gbIsQuitRequested;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allows the ability to quit via code
//------------------------------------------------------------------------------------------------------------------------------------------
void requestQuit() noexcept {
    gbIsQuitRequested = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns true if any keys or buttons are pressed
//------------------------------------------------------------------------------------------------------------------------------------------
bool areAnyKeysOrButtonsPressed() noexcept {
    // Check keyboard and mouse for any button pressed
    if (!gKeyboardKeysPressed.empty())
        return true;
    
    if (!gMouseButtonsPressed.empty())
        return true;
    
    // Check game controller or generic joypad for any digital (or converted to digital) input
    if (gpGameController) {
        if (!gControllerInputsPressed.empty())
            return true;

        if (!gJoystickAxesPressed.empty())
            return true;

        if (!gJoystickButtonsPressed.empty())
            return true;

        if (!gJoystickHatsPressed.empty())
            return true;
    }

    return false;
}

const std::vector<uint16_t>& getKeyboardKeysPressed() noexcept {
    return gKeyboardKeysPressed;
}

const std::vector<uint16_t>& getKeyboardKeysJustPressed() noexcept {
    return gKeyboardKeysJustPressed;
}

const std::vector<uint16_t>& getKeyboardKeysJustReleased() noexcept {
    return gKeyboardKeysJustReleased;
}

const std::vector<MouseButton>& getMouseButtonsPressed() noexcept {
    return gMouseButtonsPressed;
}

const std::vector<MouseButton>& getMouseButtonsJustPressed() noexcept {
    return gMouseButtonsJustPressed;
}

const std::vector<MouseButton>& getMouseButtonsJustReleased() noexcept {
    return gMouseButtonsJustReleased;
}

const std::vector<ControllerInput>& getControllerInputsPressed() noexcept {
    return gControllerInputsPressed;
}

const std::vector<ControllerInput>& getControllerInputsJustPressed() noexcept {
    return gControllerInputsJustPressed;
}

const std::vector<ControllerInput>& getControllerInputsJustReleased() noexcept {
    return gControllerInputsJustReleased;
}

const std::vector<uint32_t>& getJoystickAxesPressed() noexcept {
    return gJoystickAxesPressed;
}

const std::vector<uint32_t>& getJoystickAxesJustPressed() noexcept {
    return gJoystickAxesJustPressed;
}

const std::vector<uint32_t>& getJoystickAxesJustReleased() noexcept {
    return gJoystickAxesJustReleased;
}

const std::vector<uint32_t>& getJoystickButtonsPressed() noexcept {
    return gJoystickButtonsPressed;
}

const std::vector<uint32_t>& getJoystickButtonsJustPressed() noexcept {
    return gJoystickButtonsJustPressed;
}

const std::vector<uint32_t>& getJoystickButtonsJustReleased() noexcept {
    return gJoystickButtonsJustReleased;
}

const std::vector<JoyHat>& getJoystickHatsPressed() noexcept {
    return gJoystickHatsPressed;
}

const std::vector<JoyHat>& getJoystickHatsJustPressed() noexcept {
    return gJoystickHatsJustPressed;
}

const std::vector<JoyHat>& getJoystickHatsJustReleased() noexcept {
    return gJoystickHatsJustReleased;
}

bool isKeyboardKeyPressed(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysPressed, key);
}

bool isKeyboardKeyJustPressed(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysJustPressed, key);
}

bool isKeyboardKeyReleased(const uint16_t key) noexcept {
    return (!isKeyboardKeyPressed(key));
}

bool isKeyboardKeyJustReleased(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysJustReleased, key);
}

bool isMouseButtonPressed(const MouseButton button) noexcept {
    return vectorContainsValue(gMouseButtonsPressed, button);
}

bool isMouseButtonJustPressed(const MouseButton button) noexcept {
    return vectorContainsValue(gMouseButtonsJustPressed, button);
}

bool isMouseButtonReleased(const MouseButton button) noexcept {
    return (!vectorContainsValue(gMouseButtonsPressed, button));
}

bool isMouseButtonJustReleased(const MouseButton button) noexcept {
    return vectorContainsValue(gMouseButtonsJustReleased, button);
}

bool isControllerInputPressed(const ControllerInput input) noexcept {
    return vectorContainsValue(gControllerInputsPressed, input);
}

bool isControllerInputJustPressed(const ControllerInput input) noexcept {
    return vectorContainsValue(gControllerInputsJustPressed, input);
}

bool isControllerInputJustReleased(const ControllerInput input) noexcept {
    return vectorContainsValue(gControllerInputsJustReleased, input);
}

bool isJoystickAxisPressed(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesPressed, axis);
}

bool isJoystickAxisJustPressed(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesJustPressed, axis);
}

bool isJoystickAxisJustReleased(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesJustReleased, axis);
}

bool isJoystickButtonPressed(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsPressed, button);
}

bool isJoystickButtonJustPressed(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsJustPressed, button);
}

bool isJoystickButtonJustReleased(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsJustReleased, button);
}

bool isJoystickHatPressed(const JoyHat hat) noexcept {
    return vectorContainsValue(gJoystickHatsPressed, hat);
}

bool isJoystickHatJustPressed(const JoyHat hat) noexcept {
    return vectorContainsValue(gJoystickHatsJustPressed, hat);
}

bool isJoystickHatJustReleased(const JoyHat hat) noexcept {
    return vectorContainsValue(gJoystickHatsJustReleased, hat);
}

float getControllerInputValue(const ControllerInput input) noexcept {
    const uint8_t inputIdx = (uint8_t) input;
    return (inputIdx < NUM_CONTROLLER_INPUTS) ? gControllerInputs[inputIdx] : 0.0f;
}

float getJoystickAxisValue(const uint32_t axis) noexcept {
    for (const JoystickAxis& axisAndValue : gJoystickAxes) {
        if (axisAndValue.axis == axis)
            return axisAndValue.value;
    }

    return 0.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get an input which is adjusted for the given deadzone, such that the range of input values (0-1) starts just outside the deadzone.
// If the input has an opposite axis (2d axis pair), then the deadzone is treated like a circle and the following adjustment method is used:
//  https://www.gamasutra.com/blogs/JoshSutphin/20130416/190541/Doing_Thumbstick_Dead_Zones_Right.php
// If the controller input is not analog then no deadzone adjustments are performed.
//------------------------------------------------------------------------------------------------------------------------------------------
float getAdjustedControllerInputValue(const ControllerInput input, const float deadZone) noexcept {
    const float rawAxis = getControllerInputValue(input);
    const float clampedDeadZone = std::clamp(deadZone, 0.0f, 0.9999f);
    
    const ControllerInput oppositeInput = ControllerInputUtils::getOppositeAxis(input);
    const bool b2dAxisPair = (input != oppositeInput);
    
    if (b2dAxisPair) {
        // A 2d-axis pair
        const float rawAxisOpp = getControllerInputValue(oppositeInput);
        const float axisVecLen = std::sqrtf(rawAxis * rawAxis + rawAxisOpp * rawAxisOpp);

        const float axisNormalized = (axisVecLen > 0) ? rawAxis / axisVecLen : 0.0f;
        const float axisRescale = std::max((axisVecLen - clampedDeadZone) / (1.0f - clampedDeadZone), 0.0f);

        if (axisNormalized >= 0.0f) {
            return std::clamp(axisNormalized * axisRescale, 0.0f, 1.0f);
        } else {
            return std::clamp(axisNormalized * axisRescale, -1.0f, 0.0f);
        }
    }

    if (ControllerInputUtils::isAxis(input)) {
        // Simple 1d axis: just rescale based on the deadzone
        if (rawAxis >= 0) {
            return std::clamp((rawAxis - clampedDeadZone) / (1.0f - clampedDeadZone), 0.0f, 1.0f);
        } else {
            return std::clamp((rawAxis + clampedDeadZone) / (1.0f - clampedDeadZone), -1.0f, 0.0f);
        }
    }

    return rawAxis;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Same as 'getAdjustedControllerInputValue' except for a generic joystick axis.
// Because we don't know much about the axis or neighboring axes, the behavior of this might not be as good.
//------------------------------------------------------------------------------------------------------------------------------------------
float getAdjustedJoystickAxisValue(const uint32_t axis, const float deadZone) noexcept {
    const float rawAxis = getJoystickAxisValue(axis);
    const float clampedDeadZone = std::clamp(deadZone, 0.0f, 0.9999f);
    
    if (rawAxis >= 0) {
        return std::clamp((rawAxis - clampedDeadZone) / (1.0f - clampedDeadZone), 0.0f, 1.0f);
    } else {
        return std::clamp((rawAxis + clampedDeadZone) / (1.0f - clampedDeadZone), -1.0f, 0.0f);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the amount of mouse movement since events were last consumed (x-axis)
//------------------------------------------------------------------------------------------------------------------------------------------
float getMouseXMovement() noexcept {
    return gMouseMovementX;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the amount of mouse movement since events were last consumed (y-axis)
//------------------------------------------------------------------------------------------------------------------------------------------
float getMouseYMovement() noexcept {
    return gMouseMovementY;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current movement amount for a mouse wheel axis
//------------------------------------------------------------------------------------------------------------------------------------------
float getMouseWheelAxisMovement(const uint8_t axis) noexcept {
    return (axis < 2) ? gMouseWheelAxisMovements[axis] : 0.0f;
}

END_NAMESPACE(Input)
