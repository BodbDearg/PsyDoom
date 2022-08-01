#include "Controls.h"

#include "Asserts.h"
#include "Config/Config.h"
#include "Input.h"
#include "PsxPadButtons.h"

#include <algorithm>
#include <cmath>
#include <regex>
#include <SDL.h>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Controls)

// Whether this module has been initialized
static bool gbDidInit = false;

// The list of inputs that each control binding uses
static BindingData gBindings[(uint16_t) Binding::NUM_BINDINGS];

// Temporary string used to hold the current input name
static std::string gCurInputName;

// Use case insensitive matching for all regexes and use ECMAScript
static constexpr auto REGEX_OPTIONS = std::regex_constants::ECMAScript | std::regex_constants::icase;

// Regexes for parsing generic joystick control bindings
static const std::regex gRegex_JoystickAxis_Pos     = std::regex(R"(^JOYSTICK\s+AXIS(\d+)\+$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickAxis_Neg     = std::regex(R"(^JOYSTICK\s+AXIS(\d+)-$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickButton       = std::regex(R"(^JOYSTICK\s+BUTTON(\d+)$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickHat_Up       = std::regex(R"(^JOYSTICK\s+HAT(\d+)\s+UP$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickHat_Down     = std::regex(R"(^JOYSTICK\s+HAT(\d+)\s+DOWN$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickHat_Left     = std::regex(R"(^JOYSTICK\s+HAT(\d+)\s+LEFT$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickHat_Right    = std::regex(R"(^JOYSTICK\s+HAT(\d+)\s+RIGHT$)", REGEX_OPTIONS);

// A struct returned when data for an invalid binding is requested
static const BindingData INVALID_BINDING_DATA = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Is the character ASCII whitespace?
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isAsciiWhitespace(const char c) noexcept {
    return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\v') || (c == '\f'));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the next input name from a list of comma separated input names and return a pointer after this name in the string.
// The name returned is uppercased.
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* getNextInputNameUpper(const char* const str, std::string& nameOut) noexcept {
    // Firstly skip leading whitespace and commas
    nameOut.clear();
    const char* pCurChar = str;

    for (char c1 = *pCurChar; c1 != 0; c1 = *++pCurChar) {
        const bool bSkipChar = (isAsciiWhitespace(c1) || (c1 == ','));

        if (!bSkipChar)
            break;
    }

    // Add in all characters in the string until we encounter a ',' delimiter or the string end
    for (char c2 = *pCurChar; c2 != 0; c2 = *++pCurChar) {
        if (c2 == ',')
            break;

        // Escaped comma ahead or just an ordinary character?
        if ((c2 == '\\') && (pCurChar[1] == ',')) {
            nameOut.push_back(',');
            ++pCurChar;
        } else {
            nameOut.push_back((char) std::toupper(c2));
        }
    }

    // Remove trailing whitespace
    while ((!nameOut.empty()) && isAsciiWhitespace(nameOut.back())) {
        nameOut.pop_back();
    }

    return pCurChar;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get a raw (unmodified) input as a floating point value.
// Digital inputs such as keyboard keys are returned as either 0 or 1.
//------------------------------------------------------------------------------------------------------------------------------------------
static float getRawInput(const InputSrc src) noexcept {
    if (src.device == InputSrc::KEYBOARD_KEY) {
        return (Input::isKeyboardKeyPressed(src.input)) ? 1.0f : 0.0f;
    }
    else if (src.device == InputSrc::MOUSE_BUTTON) {
        return (Input::isMouseButtonPressed((MouseButton) src.input)) ? 1.0f : 0.0f;
    }
    else if (src.device == InputSrc::MOUSE_WHEEL) {
        // Note: treat X-axis wheel movement as if it was the Y-axis to accomodate MacOS and 'shift scrolling'.
        // The shift key on MacOS can turn vertical scrolling into horizontal scrolling.
        return -Input::getMouseWheelAxisMovement(0) + Input::getMouseWheelAxisMovement(1);
    }
    else if (src.device == InputSrc::GAMEPAD_AXIS) {
        // N.B: do deadzone adjustments so that the axis value range starts outside of the deadzone
        return Input::getAdjustedGamepadInputValue((GamepadInput) src.input, Config::gGamepadDeadZone);
    }
    else if (src.device == InputSrc::GAMEPAD_BUTTON) {
        return Input::getGamepadInputValue((GamepadInput) src.input);
    }
    else if (src.device == InputSrc::JOYSTICK_AXIS) {
        return Input::getJoystickAxisValue(src.input);
    }
    else if (src.device == InputSrc::JOYSTICK_BUTTON) {
        return (Input::isJoystickButtonPressed(src.input)) ? 1.0f : 0.0f;
    }
    else if (src.device == InputSrc::JOYSTICK_HAT) {
        return (Input::isJoystickHatPressed(src.input)) ? 1.0f : 0.0f;
    }

    return 0.0f;   // Null or unsupported input source
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get an input restricted to the sub-axis specified by the input source
//------------------------------------------------------------------------------------------------------------------------------------------
static float getSubAxisInput(const InputSrc inputSrc) noexcept {
    const float rawInput = getRawInput(inputSrc);

    if (inputSrc.subaxis == InputSrc::SUBAXIS_NEG)
        return -std::clamp(rawInput, -1.0f, 0.0f);;

    ASSERT(inputSrc.subaxis == InputSrc::SUBAXIS_POS);
    return std::clamp(rawInput, 0.0f, 1.0f);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given input has just been pressed.
// Note: doesn't need to consider sub-axis filters for this particular query - we can just use the raw inputs.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isInputJustPressed(const InputSrc src) noexcept {
    if (src.device == InputSrc::KEYBOARD_KEY) {
        return Input::isKeyboardKeyJustPressed(src.input);
    }
    else if (src.device == InputSrc::MOUSE_BUTTON) {
        return Input::isMouseButtonJustPressed((MouseButton) src.input);
    }
    else if (src.device == InputSrc::MOUSE_WHEEL) {
        // This would be an odd binding but count any mouse wheel movement as the button being just pressed.
        // It's wiped the frame after it is used...
        return ((Input::getMouseWheelAxisMovement(0) != 0) || (Input::getMouseWheelAxisMovement(1) != 0));
    }
    else if ((src.device == InputSrc::GAMEPAD_AXIS) || (src.device == InputSrc::GAMEPAD_BUTTON)) {
        return Input::isGamepadInputJustPressed((GamepadInput) src.input);
    }
    else if (src.device == InputSrc::JOYSTICK_AXIS) {
        return Input::isJoystickAxisJustPressed(src.input);
    }
    else if (src.device == InputSrc::JOYSTICK_BUTTON) {
        return Input::isJoystickButtonJustPressed(src.input);
    }
    else if (src.device == InputSrc::JOYSTICK_HAT) {
        return Input::isJoystickHatJustPressed(src.input);
    }

    return false;   // Null or unsupported input source
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given input has just been released.
// Note: doesn't need to consider sub-axis filters for this particular query - we can just use the raw inputs.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isInputJustReleased(const InputSrc src) noexcept {
    if (src.device == InputSrc::KEYBOARD_KEY) {
        return Input::isKeyboardKeyJustReleased(src.input);
    }
    else if (src.device == InputSrc::MOUSE_BUTTON) {
        return Input::isMouseButtonJustReleased((MouseButton) src.input);
    }
    else if (src.device == InputSrc::MOUSE_WHEEL) {
        // This would be an odd binding but count any mouse wheel movement as the button being just released.
        // It's wiped the frame after it is used...
        return ((Input::getMouseWheelAxisMovement(0) != 0) || (Input::getMouseWheelAxisMovement(1) != 0));
    }
    else if ((src.device == InputSrc::GAMEPAD_AXIS) || (src.device == InputSrc::GAMEPAD_BUTTON)) {
        return Input::isGamepadInputJustReleased((GamepadInput) src.input);
    }
    else if (src.device == InputSrc::JOYSTICK_AXIS) {
        return Input::isJoystickAxisJustReleased(src.input);
    }
    else if (src.device == InputSrc::JOYSTICK_BUTTON) {
        return Input::isJoystickButtonJustReleased(src.input);
    }
    else if (src.device == InputSrc::JOYSTICK_HAT) {
        return Input::isJoystickHatJustReleased(src.input);
    }

    return false;   // Null or unsupported input source
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current input value for a particular control binding in floating point format.
// If there are multiple sources then their inputs are combined additively.
//------------------------------------------------------------------------------------------------------------------------------------------
static float getInputForBinding(const Binding binding) noexcept {
    // Invalid bindings never have any inputs
    const uint16_t bindingIdx = (uint16_t) binding;

    if (bindingIdx >= (uint16_t) Binding::NUM_BINDINGS)
        return 0.0f;

    // Sum up the contribution from all the different input sources
    const BindingData& bindingData = gBindings[bindingIdx];
    const uint32_t numInputSources = bindingData.numInputSources;
    float inputValue = 0.0f;

    for (uint16_t i = 0; i < numInputSources; ++i) {
        inputValue += getSubAxisInput(bindingData.inputSources[i]);
    }

    return inputValue;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine an input source from the given uppercase string containing it's name: returns 'false' if no source could be determined
//------------------------------------------------------------------------------------------------------------------------------------------
static bool getInputSrcFromNameUpper(const std::string& nameUpper, InputSrc& inputSrc) noexcept {
    // Null/default the input source initially
    inputSrc = {};

    if (nameUpper.find("MOUSE ") == 0) {
        // Probably some sort of mouse input
        if (nameUpper == "MOUSE WHEEL+") {
            inputSrc.device = InputSrc::MOUSE_WHEEL;
            inputSrc.subaxis = InputSrc::SUBAXIS_POS;
        }
        else if (nameUpper == "MOUSE WHEEL-") {
            inputSrc.device = InputSrc::MOUSE_WHEEL;
            inputSrc.subaxis = InputSrc::SUBAXIS_NEG;
        }
        else if (nameUpper == "MOUSE LEFT") {
            inputSrc.device = InputSrc::MOUSE_BUTTON;
            inputSrc.input = (uint16_t) MouseButton::LEFT;
        }
        else if (nameUpper == "MOUSE RIGHT") {
            inputSrc.device = InputSrc::MOUSE_BUTTON;
            inputSrc.input = (uint16_t) MouseButton::RIGHT;
        }
        else if (nameUpper == "MOUSE MIDDLE") {
            inputSrc.device = InputSrc::MOUSE_BUTTON;
            inputSrc.input = (uint16_t) MouseButton::MIDDLE;
        }
        else if (nameUpper == "MOUSE X1") {
            inputSrc.device = InputSrc::MOUSE_BUTTON;
            inputSrc.input = (uint16_t) MouseButton::X1;
        }
        else if (nameUpper == "MOUSE X2") {
            inputSrc.device = InputSrc::MOUSE_BUTTON;
            inputSrc.input = (uint16_t) MouseButton::X2;
        }
    }
    else if (nameUpper.find("GAMEPAD ") == 0) {
        // Probably some sort of gamepad button or axis
        constexpr size_t NAME_PREFIX_LEN = sizeof("GAMEPAD ") - 1;
        const SDL_GameControllerButton button = SDL_GameControllerGetButtonFromString(nameUpper.c_str() + NAME_PREFIX_LEN);

        if (button != SDL_CONTROLLER_BUTTON_INVALID) {
            inputSrc.device = InputSrc::GAMEPAD_BUTTON;
            inputSrc.input = (uint16_t) GamepadInputUtils::sdlButtonToInput((uint8_t) button);
            return true;
        }

        // Maybe a gamepad axis
        if (nameUpper == "GAMEPAD LEFTX-") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_X;
            inputSrc.subaxis = InputSrc::SUBAXIS_NEG;
        }
        else if (nameUpper == "GAMEPAD LEFTX+") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_X;
            inputSrc.subaxis = InputSrc::SUBAXIS_POS;
        }
        else if (nameUpper == "GAMEPAD LEFTY-") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_Y;
            inputSrc.subaxis = InputSrc::SUBAXIS_NEG;
        }
        else if (nameUpper == "GAMEPAD LEFTY+") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_Y;
            inputSrc.subaxis = InputSrc::SUBAXIS_POS;
        }
        else if (nameUpper == "GAMEPAD RIGHTX-") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_X;
            inputSrc.subaxis = InputSrc::SUBAXIS_NEG;
        }
        else if (nameUpper == "GAMEPAD RIGHTX+") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_X;
            inputSrc.subaxis = InputSrc::SUBAXIS_POS;
        }
        else if (nameUpper == "GAMEPAD RIGHTY-") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_Y;
            inputSrc.subaxis = InputSrc::SUBAXIS_NEG;
        }
        else if (nameUpper == "GAMEPAD RIGHTY+") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_Y;
            inputSrc.subaxis = InputSrc::SUBAXIS_POS;
        }
        else if (nameUpper == "GAMEPAD LEFTTRIGGER") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_TRIG_LEFT;
        }
        else if (nameUpper == "GAMEPAD RIGHTTRIGGER") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_TRIG_RIGHT;
        }
    }
    else if (nameUpper.find("JOYSTICK ") == 0) {
        // Probably a generic joystick input (not inverted) - try one of the regexes for generic joystick inputs.
        // Note: for axis, button etc. numbers they are '1' based to the user but '0' based in code - hence we subtract '1' everywhere here...
        std::smatch regexMatches;

        if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickAxis_Pos)) {
            inputSrc.device = InputSrc::JOYSTICK_AXIS;
            inputSrc.input = (uint16_t)(std::stoi(regexMatches[1]) - 1);
            inputSrc.subaxis = InputSrc::SUBAXIS_POS;
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickAxis_Neg)) {
            inputSrc.device = InputSrc::JOYSTICK_AXIS;
            inputSrc.input = (uint16_t)(std::stoi(regexMatches[1]) - 1);
            inputSrc.subaxis = InputSrc::SUBAXIS_NEG;
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickButton)) {
            inputSrc.device = InputSrc::JOYSTICK_BUTTON;
            inputSrc.input = (uint16_t)(std::stoi(regexMatches[1]) - 1);
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickHat_Up)) {
            inputSrc.device = InputSrc::JOYSTICK_HAT;
            inputSrc.input = Input::JoyHat(Input::JoyHatDir::Up, (uint8_t)(std::stoi(regexMatches[1]) - 1));
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickHat_Down)) {
            inputSrc.device = InputSrc::JOYSTICK_HAT;
            inputSrc.input = Input::JoyHat(Input::JoyHatDir::Down, (uint8_t)(std::stoi(regexMatches[1]) - 1));
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickHat_Left)) {
            inputSrc.device = InputSrc::JOYSTICK_HAT;
            inputSrc.input = Input::JoyHat(Input::JoyHatDir::Left, (uint8_t)(std::stoi(regexMatches[1]) - 1));
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickHat_Right)) {
            inputSrc.device = InputSrc::JOYSTICK_HAT;
            inputSrc.input = Input::JoyHat(Input::JoyHatDir::Right, (uint8_t)(std::stoi(regexMatches[1]) - 1));
        }
    }
    else {
        // If nothing else then it's probably a keyboard key
        const SDL_Scancode scancode = SDL_GetScancodeFromName(nameUpper.c_str());

        if (scancode != SDL_SCANCODE_UNKNOWN) {
            inputSrc.device = InputSrc::KEYBOARD_KEY;
            inputSrc.input = (uint16_t) scancode;
            return true;
        }
    }

    return (inputSrc.device != InputSrc::NULL_DEVICE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: adds an input source to the specified binding (if there is room)
//------------------------------------------------------------------------------------------------------------------------------------------
static void addInputSourceToBinding(const InputSrc& inputSrc, BindingData& bindingData) noexcept {
    if (bindingData.numInputSources < MAX_BINDING_INPUTS) {
        bindingData.inputSources[bindingData.numInputSources] = inputSrc;
        bindingData.numInputSources++;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Startup logic for the controls system
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    gbDidInit = true;
    clearAllBindings();
    gCurInputName.reserve(64);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown logic for the controls system
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    clearAllBindings();
    gCurInputName.clear();
    gCurInputName.shrink_to_fit();
    gbDidInit = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the controls module has been initialized
//------------------------------------------------------------------------------------------------------------------------------------------
bool didInit() noexcept {
    return gbDidInit;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clear all control mappings
//------------------------------------------------------------------------------------------------------------------------------------------
void clearAllBindings() noexcept {
    for (BindingData& bindingData : gBindings) {
        bindingData = {};
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse the controls for a specified binding
//------------------------------------------------------------------------------------------------------------------------------------------
void parseBinding(const Binding binding, const char* const str) noexcept {
    // Ignore if the binding is not valid
    const uint16_t bindingIdx = (uint16_t) binding;

    if (bindingIdx >= (uint16_t) Binding::NUM_BINDINGS)
        return;
    
    // Clear the binding
    BindingData& bindingData = gBindings[bindingIdx];
    bindingData = {};

    // Parse the inputs for this binding
    for (const char* pCurSubstr = str; *pCurSubstr != 0;) {
        // Get the name of this input (uppercase) and ignore if invalid
        pCurSubstr = getNextInputNameUpper(pCurSubstr, gCurInputName);

        if (gCurInputName.empty())
            continue;

        // Try to determine what this input is and ignore if not valid
        InputSrc inputSrc = {};

        if (!getInputSrcFromNameUpper(gCurInputName, inputSrc))
            continue;

        // Otherwise add the input source to the binding
        addInputSourceToBinding(inputSrc, bindingData);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the binding data for a particular control binding
//------------------------------------------------------------------------------------------------------------------------------------------
const BindingData& getBindingData(const Binding binding) noexcept {
    const uint16_t bindingIdx = (uint16_t) binding;
    return (bindingIdx < (uint16_t) Binding::NUM_BINDINGS) ? gBindings[bindingIdx] : INVALID_BINDING_DATA;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get a control binding value in floating point format
//------------------------------------------------------------------------------------------------------------------------------------------
float getFloat(const Binding binding) noexcept {
    return getInputForBinding(binding);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get a control binding value in boolean format
//------------------------------------------------------------------------------------------------------------------------------------------
bool getBool(const Binding binding) noexcept {
    return (std::fabs(getInputForBinding(binding)) > Config::gAnalogToDigitalThreshold);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given input binding has just been pressed - useful for toggle type actions
//------------------------------------------------------------------------------------------------------------------------------------------
bool isJustPressed(const Binding binding) noexcept {
    // Invalid bindings are never just pressed
    const uint16_t bindingIdx = (uint16_t) binding;
    
    if (bindingIdx >= (uint16_t) Binding::NUM_BINDINGS)
        return false;

    // Check to see if any of the input sources are just pressed
    const BindingData& bindingData = gBindings[bindingIdx];
    const uint32_t numInputSources = bindingData.numInputSources;

    for (uint32_t i = 0; i < numInputSources; ++i) {
        const InputSrc inputSrc = bindingData.inputSources[i];

        if (isInputJustPressed(inputSrc))
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given input binding has just been released - useful for toggle type actions
//------------------------------------------------------------------------------------------------------------------------------------------
bool isJustReleased(const Binding binding) noexcept {
    // Invalid bindings are never just released
    const uint16_t bindingIdx = (uint16_t) binding;
    
    if (bindingIdx >= (uint16_t) Binding::NUM_BINDINGS)
        return false;

    // Check to see if any of the input sources are just released
    const BindingData& bindingData = gBindings[bindingIdx];
    const uint32_t numInputSources = bindingData.numInputSources;

    for (uint32_t i = 0; i < numInputSources; ++i) {
        const InputSrc inputSrc = bindingData.inputSources[i];

        if (isInputJustReleased(inputSrc))
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets a bitmask of original 'PlayStation controller' buttons that are pressed for the purposes of entering original cheat codes.
// The bitmask is not used for any purpose other than this in PsyDoom, we don't use original PSX control inputs anymore.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t getPSXCheatButtonBits() noexcept {
    uint16_t buttonBits = 0;

    auto addButtonBit = [&](const Binding binding, const uint16_t toButtonBits) noexcept {
        if (getBool(binding)) {
            buttonBits |= toButtonBits;
        }
    };

    addButtonBit(Binding::PSXCheatCode_Up, PAD_UP);
    addButtonBit(Binding::PSXCheatCode_Down, PAD_DOWN);
    addButtonBit(Binding::PSXCheatCode_Left, PAD_LEFT);
    addButtonBit(Binding::PSXCheatCode_Right, PAD_RIGHT);
    addButtonBit(Binding::PSXCheatCode_Triangle, PAD_TRIANGLE);
    addButtonBit(Binding::PSXCheatCode_Square, PAD_SQUARE);
    addButtonBit(Binding::PSXCheatCode_Circle, PAD_CIRCLE);
    addButtonBit(Binding::PSXCheatCode_Cross, PAD_CROSS);
    addButtonBit(Binding::PSXCheatCode_L1, PAD_L1);
    addButtonBit(Binding::PSXCheatCode_R1, PAD_R1);
    addButtonBit(Binding::PSXCheatCode_L2, PAD_L2);
    addButtonBit(Binding::PSXCheatCode_R2, PAD_R2);

    return buttonBits;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: converts an input source into a textual string representing that input source and appends it to the given string.
// Note: if conversion fails then nothing will be appended.
//------------------------------------------------------------------------------------------------------------------------------------------
void appendInputSrcToStr(const InputSrc& src, std::string& outputStr) noexcept {
    if (src.device == InputSrc::KEYBOARD_KEY) {
        if (const char* const scancodeName = SDL_GetScancodeName((SDL_Scancode) src.input)) {
            outputStr += scancodeName;
        }
    }
    else if (src.device == InputSrc::MOUSE_BUTTON) {
        switch ((MouseButton) src.input) {
            case MouseButton::LEFT:     outputStr += "Mouse Left";      break;
            case MouseButton::RIGHT:    outputStr += "Mouse Right";     break;
            case MouseButton::MIDDLE:   outputStr += "Mouse Middle";    break;
            case MouseButton::X1:       outputStr += "Mouse X1";        break;
            case MouseButton::X2:       outputStr += "Mouse X2";        break;
        }
    }
    else if (src.device == InputSrc::MOUSE_WHEEL) {
        switch (src.subaxis) {
            case InputSrc::SUBAXIS_POS:     outputStr += "Mouse Wheel+";    break;
            case InputSrc::SUBAXIS_NEG:     outputStr += "Mouse Wheel-";    break;
        }
    }
    else if (src.device == InputSrc::GAMEPAD_AXIS) {
        switch ((GamepadInput) src.input) {
            case GamepadInput::AXIS_LEFT_X: {
                switch (src.subaxis) {
                    case InputSrc::SUBAXIS_POS:     outputStr += "Gamepad LeftX+";  break;
                    case InputSrc::SUBAXIS_NEG:     outputStr += "Gamepad LeftX-";  break;
                }
            }   break;

            case GamepadInput::AXIS_LEFT_Y: {
                switch (src.subaxis) {
                    case InputSrc::SUBAXIS_POS:     outputStr += "Gamepad LeftY+";  break;
                    case InputSrc::SUBAXIS_NEG:     outputStr += "Gamepad LeftY-";  break;
                }
            }   break;

            case GamepadInput::AXIS_RIGHT_X: {
                switch (src.subaxis) {
                    case InputSrc::SUBAXIS_POS:     outputStr += "Gamepad RightX+";     break;
                    case InputSrc::SUBAXIS_NEG:     outputStr += "Gamepad RightX-";     break;
                }
            }   break;

            case GamepadInput::AXIS_RIGHT_Y: {
                switch (src.subaxis) {
                    case InputSrc::SUBAXIS_POS:     outputStr += "Gamepad RightY+";     break;
                    case InputSrc::SUBAXIS_NEG:     outputStr += "Gamepad RightY-";     break;
                }
            }   break;

            case GamepadInput::AXIS_TRIG_LEFT:      outputStr += "Gamepad LeftTrigger";     break;
            case GamepadInput::AXIS_TRIG_RIGHT:     outputStr += "Gamepad RightTrigger";    break;
        }
    }
    else if (src.device == InputSrc::GAMEPAD_BUTTON) {
        switch ((GamepadInput) src.input) {
            case GamepadInput::BTN_A:               outputStr += "Gamepad A";               break;
            case GamepadInput::BTN_B:               outputStr += "Gamepad B";               break;
            case GamepadInput::BTN_X:               outputStr += "Gamepad X";               break;
            case GamepadInput::BTN_Y:               outputStr += "Gamepad Y";               break;
            case GamepadInput::BTN_BACK:            outputStr += "Gamepad Back";            break;
            case GamepadInput::BTN_GUIDE:           outputStr += "Gamepad Guide";           break;
            case GamepadInput::BTN_START:           outputStr += "Gamepad Start";           break;
            case GamepadInput::BTN_LEFT_STICK:      outputStr += "Gamepad LeftStick";       break;
            case GamepadInput::BTN_RIGHT_STICK:     outputStr += "Gamepad RightStick";      break;
            case GamepadInput::BTN_LEFT_SHOULDER:   outputStr += "Gamepad LeftShoulder";    break;
            case GamepadInput::BTN_RIGHT_SHOULDER:  outputStr += "Gamepad RightShoulder";   break;
            case GamepadInput::BTN_DPAD_UP:         outputStr += "Gamepad DpUp";            break;
            case GamepadInput::BTN_DPAD_DOWN:       outputStr += "Gamepad DpDown";          break;
            case GamepadInput::BTN_DPAD_LEFT:       outputStr += "Gamepad DpLeft";          break;
            case GamepadInput::BTN_DPAD_RIGHT:      outputStr += "Gamepad DpRight";         break;
        }
    }
    else if (src.device == InputSrc::JOYSTICK_AXIS) {
        char axisNumStr[32];
        std::snprintf(axisNumStr, C_ARRAY_SIZE(axisNumStr), "%d", (int) src.input + 1);

        switch (src.subaxis) {
            case InputSrc::SUBAXIS_POS:
                outputStr += "Joystick Axis";
                outputStr += axisNumStr;
                outputStr += '+';
                break;

            case InputSrc::SUBAXIS_NEG:
                outputStr += "Joystick Axis";
                outputStr += axisNumStr;
                outputStr += '-';
                break;
        }
    }
    else if (src.device == InputSrc::JOYSTICK_BUTTON) {
        char buttonNumStr[32];
        std::snprintf(buttonNumStr, C_ARRAY_SIZE(buttonNumStr), "%d", (int) src.input + 1);
        outputStr += "Joystick Button";
        outputStr += buttonNumStr;
    }
    else if (src.device == InputSrc::JOYSTICK_HAT) {
        const size_t origOutputStrSize = outputStr.size();
        const Input::JoyHat hatInput = src.input;

        char hatNumStr[32];
        std::snprintf(hatNumStr, C_ARRAY_SIZE(hatNumStr), "%d", (int) hatInput.fields.hatNum + 1);
        outputStr += " Joystick Hat";
        outputStr += hatNumStr;
        outputStr += ' ';
        
        switch (hatInput.fields.hatDir) {
            case Input::JoyHatDir::Up:      outputStr += "Up";      break;
            case Input::JoyHatDir::Down:    outputStr += "Down";    break;
            case Input::JoyHatDir::Left:    outputStr += "Left";    break;
            case Input::JoyHatDir::Right:   outputStr += "Right";   break;
        }

        // If the hat direction is invalid then roll back the output string to it's former state
        if (outputStr.back() == ' ') {
            outputStr.resize(origOutputStrSize);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: converts a control binding to a string representing the binding.
// Note: if conversion fails then an empty string is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
void bindingToStr(const Binding binding, std::string& outputStr) noexcept {
    // Add all input sources for the binding to the string
    outputStr.clear();
    const BindingData& bindingData = getBindingData(binding);

    for (uint32_t i = 0; i < bindingData.numInputSources; ++i) {
        const size_t oldOutputStrSize = outputStr.size();
        appendInputSrcToStr(bindingData.inputSources[i], outputStr);
        const size_t newOutputStrSize = outputStr.size();

        // If we actually appended the input source then speculatively add a separator after it.
        // This way further input sources can just be appended without any other checks.
        if (newOutputStrSize > oldOutputStrSize) {
            outputStr += ',';
            outputStr += ' ';
        }
    }

    // Remove any unused trailing input source separator at the end (that was added speculatively)
    if (outputStr.size() >= 2) {
        outputStr.pop_back();
        outputStr.pop_back();
    }
}

END_NAMESPACE(Controls)
