#include "Controls.h"

#include "Config.h"
#include "Input.h"
#include "PsxPadButtons.h"

#include <algorithm>
#include <cmath>
#include <regex>
#include <SDL.h>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Controls)

// Holds a single button, key or axis that is bound to a particular action
struct InputSrc {
    // Where the input is coming from
    enum : uint8_t {
        NULL_DEVICE,        // Always generates no input
        KEYBOARD_KEY,
        MOUSE_BUTTON,
        MOUSE_WHEEL,
        GAMEPAD_AXIS,
        GAMEPAD_BUTTON,
        JOYSTICK_AXIS,
        JOYSTICK_BUTTON,
        JOYSTICK_HAT
    } device;

    // Modifiers to apply to the input
    enum : uint8_t {
        MOD_NONE,           // Don't modify the input
        MOD_POS_SUBAXIS,    // Only use the 0.0 to +1.0 range of the axis and return it as a 0.0 to 1.0 axis
        MOD_NEG_SUBAXIS,    // Only use the 0.0 to -1.0 range of the axis and return it as a 0.0 to 1.0 axis
        MOD_INVERT,         // Invert/negate the axis inputs
    } modifier;

    // What particular button or axis is used
    uint16_t input;
};

// Defines a range of input sources in the global list of input sources
struct InputSrcRange {
    uint16_t startIndex;
    uint16_t size;
};

static std::vector<InputSrc>    gInputSources;              // The global list of input sources for all control bindings
static InputSrcRange            gBindings[NUM_BINDINGS];    // The inputs that each control binding uses
static std::string              gCurInputName;              // Temporary string used to hold the current input name

// Use case insensitive matching for all regexes and use ECMAScript
static constexpr auto REGEX_OPTIONS = std::regex_constants::ECMAScript | std::regex_constants::icase;

// Regexes for parsing generic joystick control bindings
static const std::regex gRegex_JoystickAxis_Norm    = std::regex(R"(^JOYSTICK\s+AXIS(\d+)$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickAxis_PosOnly = std::regex(R"(^JOYSTICK\s+AXIS(\d+)\+$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickAxis_NegOnly = std::regex(R"(^JOYSTICK\s+AXIS(\d+)-$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickAxis_Inv     = std::regex(R"(^INV\s+JOYSTICK\s+AXIS(\d+)$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickButton       = std::regex(R"(^JOYSTICK\s+BUTTON(\d+)$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickHat_Up       = std::regex(R"(^JOYSTICK\s+HAT(\d+)\s+UP$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickHat_Down     = std::regex(R"(^JOYSTICK\s+HAT(\d+)\s+DOWN$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickHat_Left     = std::regex(R"(^JOYSTICK\s+HAT(\d+)\s+LEFT$)", REGEX_OPTIONS);
static const std::regex gRegex_JoystickHat_Right    = std::regex(R"(^JOYSTICK\s+HAT(\d+)\s+RIGHT$)", REGEX_OPTIONS);

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
// Get an input with modifiers applied
//------------------------------------------------------------------------------------------------------------------------------------------
static float getInputWithModifiers(const InputSrc src) noexcept {
    const float rawValue = getRawInput(src);

    if (src.modifier == InputSrc::MOD_INVERT) {
        return -rawValue;
    }
    else if (src.modifier == InputSrc::MOD_POS_SUBAXIS) {
        return std::clamp(rawValue, 0.0f, 1.0f);
    }
    else if (src.modifier == InputSrc::MOD_NEG_SUBAXIS) {
        return -std::clamp(rawValue, -1.0f, 0.0f);
    }

    return rawValue;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given input has just been pressed.
// Note: doesn't need to consider modifiers for this particular query - we can just use the raw inputs.
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
// Get the current input for a particular control binding in floating point format.
// If there are multiple sources then their inputs are combined additively.
//------------------------------------------------------------------------------------------------------------------------------------------
static float getInputForBinding(const Binding binding) noexcept {
    float input = 0.0f;
    uint16_t bindingIdx = (uint16_t) binding;

    if (bindingIdx < NUM_BINDINGS) {
        const InputSrcRange inputRange = gBindings[bindingIdx];

        for (uint16_t i = 0; i < inputRange.size; ++i) {
            const InputSrc inputSrc = gInputSources[inputRange.startIndex + i];
            input += getInputWithModifiers(inputSrc);
        }
    }

    return input;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine an input source from the given uppercase string containing it's name: returns 'false' if no source could be determined
//------------------------------------------------------------------------------------------------------------------------------------------
static bool getInputSrcFromNameUpper(const std::string& nameUpper, InputSrc& inputSrc) noexcept {
    // Null/default the input source initially
    inputSrc = {};

    if (nameUpper.find("MOUSE ") == 0) {
        // Probably some sort of mouse input
        if (nameUpper == "MOUSE WHEEL") {
            inputSrc.device = InputSrc::MOUSE_WHEEL;
        } 
        else if (nameUpper == "MOUSE WHEEL+") {
            inputSrc.device = InputSrc::MOUSE_WHEEL;
            inputSrc.modifier = InputSrc::MOD_POS_SUBAXIS;
        }
        else if (nameUpper == "MOUSE WHEEL-") {
            inputSrc.device = InputSrc::MOUSE_WHEEL;
            inputSrc.modifier = InputSrc::MOD_NEG_SUBAXIS;
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
    else if (nameUpper == "INV MOUSE WHEEL") {
        inputSrc.device = InputSrc::MOUSE_WHEEL;
        inputSrc.modifier = InputSrc::MOD_INVERT;
    }
    else if (nameUpper.find("GAMEPAD ") == 0) {
        // Probably some sort of gamepad button or axis (not inverted)
        constexpr size_t NAME_PREFIX_LEN = sizeof("GAMEPAD ") - 1;
        const SDL_GameControllerButton button = SDL_GameControllerGetButtonFromString(nameUpper.c_str() + NAME_PREFIX_LEN);

        if (button != SDL_CONTROLLER_BUTTON_INVALID) {
            inputSrc.device = InputSrc::GAMEPAD_BUTTON;
            inputSrc.input = (uint16_t) GamepadInputUtils::sdlButtonToInput((uint8_t) button);
            return true;
        }

        // Maybe a gamepad axis
        const SDL_GameControllerAxis axis = SDL_GameControllerGetAxisFromString(nameUpper.c_str() + NAME_PREFIX_LEN);
        
        if (axis != SDL_CONTROLLER_AXIS_INVALID) {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInputUtils::sdlAxisToInput((uint8_t) axis);
            return true;
        }

        // Try one of the special axes
        if (nameUpper == "GAMEPAD LEFTX-") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_X;
            inputSrc.modifier = InputSrc::MOD_NEG_SUBAXIS;
        }
        else if (nameUpper == "GAMEPAD LEFTX+") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_X;
            inputSrc.modifier = InputSrc::MOD_POS_SUBAXIS;
        }
        else if (nameUpper == "GAMEPAD LEFTY-") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_Y;
            inputSrc.modifier = InputSrc::MOD_NEG_SUBAXIS;
        }
        else if (nameUpper == "GAMEPAD LEFTY+") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_Y;
            inputSrc.modifier = InputSrc::MOD_POS_SUBAXIS;
        }
        else if (nameUpper == "GAMEPAD RIGHTX-") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_X;
            inputSrc.modifier = InputSrc::MOD_NEG_SUBAXIS;
        }
        else if (nameUpper == "GAMEPAD RIGHTX+") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_X;
            inputSrc.modifier = InputSrc::MOD_POS_SUBAXIS;
        }
        else if (nameUpper == "GAMEPAD RIGHTY-") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_Y;
            inputSrc.modifier = InputSrc::MOD_NEG_SUBAXIS;
        }
        else if (nameUpper == "GAMEPAD RIGHTY+") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_Y;
            inputSrc.modifier = InputSrc::MOD_POS_SUBAXIS;
        }
    }
    else if (nameUpper.find("JOYSTICK ") == 0) {
        // Probably a generic joystick input (not inverted) - try one of the regexes for generic joystick inputs.
        // Note: for axis, button etc. numbers they are '1' based to the user but '0' based in code - hence we subtract '1' everywhere here...
        std::smatch regexMatches;

        if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickAxis_Norm)) {
            inputSrc.device = InputSrc::JOYSTICK_AXIS;
            inputSrc.input = (uint16_t)(std::stoi(regexMatches[1]) - 1);
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickAxis_PosOnly)) {
            inputSrc.device = InputSrc::JOYSTICK_AXIS;
            inputSrc.input = (uint16_t)(std::stoi(regexMatches[1]) - 1);
            inputSrc.modifier = InputSrc::MOD_POS_SUBAXIS;
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickAxis_NegOnly)) {
            inputSrc.device = InputSrc::JOYSTICK_AXIS;
            inputSrc.input = (uint16_t)(std::stoi(regexMatches[1]) - 1);
            inputSrc.modifier = InputSrc::MOD_NEG_SUBAXIS;
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickButton)) {
            inputSrc.device = InputSrc::JOYSTICK_BUTTON;
            inputSrc.input = (uint16_t)(std::stoi(regexMatches[1]) - 1);
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickHat_Up)) {
            inputSrc.device = InputSrc::JOYSTICK_HAT;
            inputSrc.input = Input::JoyHat(Input::JoyHatDir::Up, (uint16_t)(std::stoi(regexMatches[1]) - 1));
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickHat_Down)) {
            inputSrc.device = InputSrc::JOYSTICK_HAT;
            inputSrc.input = Input::JoyHat(Input::JoyHatDir::Down, (uint16_t)(std::stoi(regexMatches[1]) - 1));
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickHat_Left)) {
            inputSrc.device = InputSrc::JOYSTICK_HAT;
            inputSrc.input = Input::JoyHat(Input::JoyHatDir::Left, (uint16_t)(std::stoi(regexMatches[1]) - 1));
        }
        else if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickHat_Right)) {
            inputSrc.device = InputSrc::JOYSTICK_HAT;
            inputSrc.input = Input::JoyHat(Input::JoyHatDir::Right, (uint16_t)(std::stoi(regexMatches[1]) - 1));
        }
    }
    else if (nameUpper.find("INV GAMEPAD ") == 0) {
        // Probably an inverted gamepad axis
        if (nameUpper == "INV GAMEPAD LEFTX") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_X;
            inputSrc.modifier = InputSrc::MOD_INVERT;
        }
        else if (nameUpper == "INV GAMEPAD LEFTY") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_LEFT_Y;
            inputSrc.modifier = InputSrc::MOD_INVERT;
        }
        else if (nameUpper == "INV GAMEPAD RIGHTX") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_X;
            inputSrc.modifier = InputSrc::MOD_INVERT;
        }
        else if (nameUpper == "INV GAMEPAD RIGHTY") {
            inputSrc.device = InputSrc::GAMEPAD_AXIS;
            inputSrc.input = (uint16_t) GamepadInput::AXIS_RIGHT_Y;
            inputSrc.modifier = InputSrc::MOD_INVERT;
        }
    }
    else if (nameUpper.find("INV JOYSTICK ") == 0) {
        // Probably an inverted joystick axis, try to match it
        std::smatch regexMatches;

        if (std::regex_search(nameUpper, regexMatches, gRegex_JoystickAxis_Inv)) {
            inputSrc.device = InputSrc::JOYSTICK_AXIS;
            inputSrc.input = (uint16_t)(std::stoi(regexMatches[1]) - 1);
            inputSrc.modifier = InputSrc::MOD_INVERT;
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
// Startup logic for the controls system
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    gInputSources.reserve(256);
    gCurInputName.reserve(64);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown logic for the controls system
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    clearAllBindings();
    gInputSources.shrink_to_fit();
    gCurInputName.clear();
    gCurInputName.shrink_to_fit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clear all control mappings
//------------------------------------------------------------------------------------------------------------------------------------------
void clearAllBindings() noexcept {
    for (InputSrcRange& range : gBindings) {
        range = {};
    }

    gInputSources.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse the controls for a specified binding
//------------------------------------------------------------------------------------------------------------------------------------------
void parseBinding(const Binding binding, const char* const str) noexcept {
    // Ignore if the binding is not valid
    uint16_t bindingIdx = (uint16_t) binding;

    if (bindingIdx >= NUM_BINDINGS)
        return;

    // Clear the binding
    InputSrcRange& inputSrcRange = gBindings[bindingIdx];
    inputSrcRange = {};

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

        // Save the input
        if (inputSrcRange.size == 0) {
            inputSrcRange.startIndex = (uint16_t) gInputSources.size();
        }

        inputSrcRange.size++;
        gInputSources.push_back(inputSrc);
    }
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
    uint16_t bindingIdx = (uint16_t) binding;

    if (bindingIdx < NUM_BINDINGS) {
        const InputSrcRange inputRange = gBindings[bindingIdx];

        for (uint16_t i = 0; i < inputRange.size; ++i) {
            const InputSrc inputSrc = gInputSources[inputRange.startIndex + i];

            if (isInputJustPressed(inputSrc))
                return true;
        }
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

END_NAMESPACE(Controls)
