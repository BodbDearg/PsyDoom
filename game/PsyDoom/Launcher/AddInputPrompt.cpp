//------------------------------------------------------------------------------------------------------------------------------------------
// A module reponsible for showing and running the prompt to add a new input for a control binding
//------------------------------------------------------------------------------------------------------------------------------------------
#include "AddInputPrompt.h"

#if PSYDOOM_LAUNCHER

#include "PsyDoom/Input.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Double_Window.H>
END_DISABLE_HEADER_WARNINGS

#include <chrono>
#include <memory>
#include <SDL.h>

BEGIN_NAMESPACE(AddInputPrompt)

// Use this timer type for the prompt
typedef std::chrono::high_resolution_clock timer_t;

//------------------------------------------------------------------------------------------------------------------------------------------
// State for the prompt
//------------------------------------------------------------------------------------------------------------------------------------------
struct AddInputPromptState {
    Fl_Double_Window*       pWindow;
    Fl_Box*                 pBodyText;
    char                    bodyMsg[256];
    timer_t::time_point     startTime;
    int                     duration;
    Controls::InputSrc      inputSrc;
    uint16_t                keyboardKeyPressed;
} gPrompt;

//------------------------------------------------------------------------------------------------------------------------------------------
// Converts an FLTK key code for the keyboard event currently being handled to an SDL scan code.
// Returns '0' if no conversion could be performed.
// 
// Note: assumes that an FLTK keyboard event IS actually currently being handled.
// Calling this function when that is not true may cause unexpected results since it relies on some global event state.
//------------------------------------------------------------------------------------------------------------------------------------------
static uint16_t convertCurrentFltkEventKeyToSdlScancode(const int fltkKey) noexcept {
    // Printable ASCII?
    switch (fltkKey) {
        case ' ':   return SDL_SCANCODE_SPACE;
        case '\'':  return SDL_SCANCODE_APOSTROPHE;
        case ',':   return SDL_SCANCODE_COMMA;
        case '-':   return SDL_SCANCODE_MINUS;
        case '.':   return SDL_SCANCODE_PERIOD;
        case '/':   return SDL_SCANCODE_SLASH;
        case '0':   return SDL_SCANCODE_0;
        case '1':   return SDL_SCANCODE_1;
        case '2':   return SDL_SCANCODE_2;
        case '3':   return SDL_SCANCODE_3;
        case '4':   return SDL_SCANCODE_4;
        case '5':   return SDL_SCANCODE_5;
        case '6':   return SDL_SCANCODE_6;
        case '7':   return SDL_SCANCODE_7;
        case '8':   return SDL_SCANCODE_8;
        case '9':   return SDL_SCANCODE_9;
        case ';':   return SDL_SCANCODE_SEMICOLON;
        case '=':   return SDL_SCANCODE_EQUALS;
        case 'A':   return SDL_SCANCODE_A;
        case 'B':   return SDL_SCANCODE_B;
        case 'C':   return SDL_SCANCODE_C;
        case 'D':   return SDL_SCANCODE_D;
        case 'E':   return SDL_SCANCODE_E;
        case 'F':   return SDL_SCANCODE_F;
        case 'G':   return SDL_SCANCODE_G;
        case 'H':   return SDL_SCANCODE_H;
        case 'I':   return SDL_SCANCODE_I;
        case 'J':   return SDL_SCANCODE_J;
        case 'K':   return SDL_SCANCODE_K;
        case 'L':   return SDL_SCANCODE_L;
        case 'M':   return SDL_SCANCODE_M;
        case 'N':   return SDL_SCANCODE_N;
        case 'O':   return SDL_SCANCODE_O;
        case 'P':   return SDL_SCANCODE_P;
        case 'Q':   return SDL_SCANCODE_Q;
        case 'R':   return SDL_SCANCODE_R;
        case 'S':   return SDL_SCANCODE_S;
        case 'T':   return SDL_SCANCODE_T;
        case 'U':   return SDL_SCANCODE_U;
        case 'V':   return SDL_SCANCODE_V;
        case 'W':   return SDL_SCANCODE_W;
        case 'X':   return SDL_SCANCODE_X;
        case 'Y':   return SDL_SCANCODE_Y;
        case 'Z':   return SDL_SCANCODE_Z;
        case '[':   return SDL_SCANCODE_LEFTBRACKET;
        case '\\':  return SDL_SCANCODE_BACKSLASH;
        case ']':   return SDL_SCANCODE_RIGHTBRACKET;
        case '`':   return SDL_SCANCODE_GRAVE;
        case 'a':   return SDL_SCANCODE_A;
        case 'b':   return SDL_SCANCODE_B;
        case 'c':   return SDL_SCANCODE_C;
        case 'd':   return SDL_SCANCODE_D;
        case 'e':   return SDL_SCANCODE_E;
        case 'f':   return SDL_SCANCODE_F;
        case 'g':   return SDL_SCANCODE_G;
        case 'h':   return SDL_SCANCODE_H;
        case 'i':   return SDL_SCANCODE_I;
        case 'j':   return SDL_SCANCODE_J;
        case 'k':   return SDL_SCANCODE_K;
        case 'l':   return SDL_SCANCODE_L;
        case 'm':   return SDL_SCANCODE_M;
        case 'n':   return SDL_SCANCODE_N;
        case 'o':   return SDL_SCANCODE_O;
        case 'p':   return SDL_SCANCODE_P;
        case 'q':   return SDL_SCANCODE_Q;
        case 'r':   return SDL_SCANCODE_R;
        case 's':   return SDL_SCANCODE_S;
        case 't':   return SDL_SCANCODE_T;
        case 'u':   return SDL_SCANCODE_U;
        case 'v':   return SDL_SCANCODE_V;
        case 'w':   return SDL_SCANCODE_W;
        case 'x':   return SDL_SCANCODE_X;
        case 'y':   return SDL_SCANCODE_Y;
        case 'z':   return SDL_SCANCODE_Z;
    }

    // Numeric keypad key?
    if ((fltkKey >= FL_KP) && (fltkKey <= FL_KP_Last)) {
        const int keypadKey = fltkKey - FL_KP;

        switch (keypadKey) {
            case '\b':  return SDL_SCANCODE_KP_BACKSPACE;
            case '\t':  return SDL_SCANCODE_KP_TAB;
            case ' ':   return SDL_SCANCODE_KP_SPACE;
            case '!':   return SDL_SCANCODE_KP_EXCLAM;
            case '#':   return SDL_SCANCODE_KP_HASH;
            case '%':   return SDL_SCANCODE_KP_PERCENT;
            case '&':   return SDL_SCANCODE_KP_AMPERSAND;
            case '(':   return SDL_SCANCODE_KP_LEFTPAREN;
            case ')':   return SDL_SCANCODE_KP_RIGHTPAREN;
            case '*':   return SDL_SCANCODE_KP_MULTIPLY;
            case '+':   return SDL_SCANCODE_KP_PLUS;
            case ',':   return SDL_SCANCODE_KP_COMMA;
            case '-':   return SDL_SCANCODE_KP_MINUS;
            case '.':   return SDL_SCANCODE_KP_PERIOD;
            case '/':   return SDL_SCANCODE_KP_DIVIDE;
            case '0':   return SDL_SCANCODE_KP_0;
            case '1':   return SDL_SCANCODE_KP_1;
            case '2':   return SDL_SCANCODE_KP_2;
            case '3':   return SDL_SCANCODE_KP_3;
            case '4':   return SDL_SCANCODE_KP_4;
            case '5':   return SDL_SCANCODE_KP_5;
            case '6':   return SDL_SCANCODE_KP_6;
            case '7':   return SDL_SCANCODE_KP_7;
            case '8':   return SDL_SCANCODE_KP_8;
            case '9':   return SDL_SCANCODE_KP_9;
            case ':':   return SDL_SCANCODE_KP_COLON;
            case '<':   return SDL_SCANCODE_KP_LESS;
            case '=':   return SDL_SCANCODE_KP_EQUALS;
            case '>':   return SDL_SCANCODE_KP_GREATER;
            case '@':   return SDL_SCANCODE_KP_AT;
            case 'A':   return SDL_SCANCODE_KP_A;
            case 'B':   return SDL_SCANCODE_KP_B;
            case 'C':   return SDL_SCANCODE_KP_C;
            case 'D':   return SDL_SCANCODE_KP_D;
            case 'E':   return SDL_SCANCODE_KP_E;
            case 'F':   return SDL_SCANCODE_KP_F;
            case '^':   return SDL_SCANCODE_KP_POWER;
            case 'a':   return SDL_SCANCODE_KP_A;
            case 'b':   return SDL_SCANCODE_KP_B;
            case 'c':   return SDL_SCANCODE_KP_C;
            case 'd':   return SDL_SCANCODE_KP_D;
            case 'e':   return SDL_SCANCODE_KP_E;
            case 'f':   return SDL_SCANCODE_KP_F;
            case '{':   return SDL_SCANCODE_KP_LEFTBRACE;
            case '|':   return SDL_SCANCODE_KP_VERTICALBAR;
            case '}':   return SDL_SCANCODE_KP_RIGHTBRACE;
        }
    }

    // Function key?
    if ((fltkKey >= FL_F) && (fltkKey <= FL_F_Last)) {
        const int funcNum = fltkKey - FL_F;

        switch (funcNum) {
            case 1:     return SDL_SCANCODE_F1;
            case 2:     return SDL_SCANCODE_F2;
            case 3:     return SDL_SCANCODE_F3;
            case 4:     return SDL_SCANCODE_F4;
            case 5:     return SDL_SCANCODE_F5;
            case 6:     return SDL_SCANCODE_F6;
            case 7:     return SDL_SCANCODE_F7;
            case 8:     return SDL_SCANCODE_F8;
            case 9:     return SDL_SCANCODE_F9;
            case 10:    return SDL_SCANCODE_F10;
            case 11:    return SDL_SCANCODE_F11;
            case 12:    return SDL_SCANCODE_F12;
            case 13:    return SDL_SCANCODE_F13;
            case 14:    return SDL_SCANCODE_F14;
            case 15:    return SDL_SCANCODE_F15;
            case 16:    return SDL_SCANCODE_F16;
            case 17:    return SDL_SCANCODE_F17;
            case 18:    return SDL_SCANCODE_F18;
            case 19:    return SDL_SCANCODE_F19;
            case 20:    return SDL_SCANCODE_F20;
            case 21:    return SDL_SCANCODE_F21;
            case 22:    return SDL_SCANCODE_F22;
            case 23:    return SDL_SCANCODE_F23;
            case 24:    return SDL_SCANCODE_F24;
        }
    }

    // Other type of special key?
    switch (fltkKey) {
        case FL_BackSpace:      return SDL_SCANCODE_BACKSPACE;
        case FL_Tab:            return SDL_SCANCODE_TAB;
        case FL_Iso_Key:        return SDL_SCANCODE_BACKSLASH;
        case FL_Enter:          return SDL_SCANCODE_RETURN;
        case FL_Pause:          return SDL_SCANCODE_PAUSE;
        case FL_Scroll_Lock:    return SDL_SCANCODE_SCROLLLOCK;
        case FL_Escape:         return SDL_SCANCODE_ESCAPE;
        case FL_Home:           return SDL_SCANCODE_HOME;
        case FL_Left:           return SDL_SCANCODE_LEFT;
        case FL_Up:             return SDL_SCANCODE_UP;
        case FL_Right:          return SDL_SCANCODE_RIGHT;
        case FL_Down:           return SDL_SCANCODE_DOWN;
        case FL_Page_Up:        return SDL_SCANCODE_PAGEUP;
        case FL_Page_Down:      return SDL_SCANCODE_PAGEDOWN;
        case FL_End:            return SDL_SCANCODE_END;
        case FL_Print:          return SDL_SCANCODE_PRINTSCREEN;
        case FL_Insert:         return SDL_SCANCODE_INSERT;
        case FL_Menu:           return SDL_SCANCODE_MENU;
        case FL_Help:           return SDL_SCANCODE_HELP;
        case FL_Num_Lock:       return SDL_SCANCODE_NUMLOCKCLEAR;
        case FL_KP_Enter:       return SDL_SCANCODE_KP_ENTER;

        // Handling left shift versus right shift seems to be somewhat buggy or inconsistent with FLTK so do it this way instead.
        // During testing I found that FLTK said left shift was pressed when it was actually right shift.
        // Double check which key is down via 'event_key()' as a workaround....
        // 
        // This is a slightly dirty workaround since it relies on global 'current' event state.
        // It wouldn't work if we had to support multiple pressed keys at a time, but thankfully that's not neccessary for inputting controls.
        case FL_Shift_L:
            if (Fl::event_key(FL_Shift_L))
                return SDL_SCANCODE_LSHIFT;
            else if (Fl::event_key(FL_Shift_R))
                return SDL_SCANCODE_RSHIFT;
            else
                return SDL_SCANCODE_LSHIFT;

        case FL_Shift_R:
            if (Fl::event_key(FL_Shift_R))
                return SDL_SCANCODE_RSHIFT;
            else if (Fl::event_key(FL_Shift_L))
                return SDL_SCANCODE_LSHIFT;
            else
                return SDL_SCANCODE_RSHIFT;

        case FL_Control_L:      return SDL_SCANCODE_LCTRL;
        case FL_Control_R:      return SDL_SCANCODE_RCTRL;
        case FL_Caps_Lock:      return SDL_SCANCODE_CAPSLOCK;
        case FL_Meta_L:         return SDL_SCANCODE_LGUI;
        case FL_Meta_R:         return SDL_SCANCODE_RGUI;
        case FL_Alt_L:          return SDL_SCANCODE_LALT;
        case FL_Alt_R:          return SDL_SCANCODE_RALT;
        case FL_Delete:         return SDL_SCANCODE_DELETE;
        case FL_Volume_Down:    return SDL_SCANCODE_VOLUMEDOWN;
        case FL_Volume_Mute:    return SDL_SCANCODE_AUDIOMUTE;
        case FL_Volume_Up:      return SDL_SCANCODE_VOLUMEUP;
        case FL_Media_Play:     return SDL_SCANCODE_AUDIOPLAY;
        case FL_Media_Stop:     return SDL_SCANCODE_AUDIOSTOP;
        case FL_Media_Prev:     return SDL_SCANCODE_AUDIOPREV;
        case FL_Media_Next:     return SDL_SCANCODE_AUDIONEXT;
        case FL_Home_Page:      return SDL_SCANCODE_WWW;
        case FL_Mail:           return SDL_SCANCODE_MAIL;
        case FL_Search:         return SDL_SCANCODE_AC_SEARCH;
        case FL_Back:           return SDL_SCANCODE_AC_BACK;
        case FL_Forward:        return SDL_SCANCODE_AC_FORWARD;
        case FL_Stop:           return SDL_SCANCODE_AC_STOP;
        case FL_Refresh:        return SDL_SCANCODE_AC_REFRESH;
        case FL_Sleep:          return SDL_SCANCODE_SLEEP;
        case FL_Favorites:      return SDL_SCANCODE_AC_BOOKMARKS;
    }

    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Event handler for unhandled FLTK events for the 'Add input' prompt.
// This is used to get keyboard key pressed events from FLTK.
//------------------------------------------------------------------------------------------------------------------------------------------
static int onAddInputPromptEvent(const int event) noexcept {
    // Ignore non keyboard pressed events and don't consume when ignoring
    if ((event != FL_KEYDOWN) && (event != FL_SHORTCUT))
        return 0;

    // Translate from an FLTK key to an SDL scancode since PsyDoom uses those internally:
    const int fltkKey = Fl::event_original_key();
    const uint16_t sdlScancode = convertCurrentFltkEventKeyToSdlScancode(fltkKey);

    // Save and consume the keyboard event
    gPrompt.keyboardKeyPressed = sdlScancode;
    return 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns an input source for an input currently being provided by the player
//------------------------------------------------------------------------------------------------------------------------------------------
static Controls::InputSrc getCurrentInput() noexcept {
    using namespace Controls;

    // Update what gamepad and joystick inputs are being provided. Note that this module CAN'T be used to get SDL keyboard and mouse inputs
    // because they are tied to an SDL window, which we don't have at this point.
    Input::update();

    // Pressing a keyboard key?
    // We receive those via callbacks from FLTK.
    if ((gPrompt.keyboardKeyPressed != 0) && (gPrompt.keyboardKeyPressed < SDL_NUM_SCANCODES)) {
        return InputSrc{ InputSrc::KEYBOARD_KEY, InputSrc::SUBAXIS_POS, gPrompt.keyboardKeyPressed };
    }

    // Pressing a mouse button?
    // Note that we need to poll SDL in this way because we don't yet have an SDL window to receive mouse events.
    const int pressedMouseButtonMask = SDL_GetGlobalMouseState(nullptr, nullptr);

    if (pressedMouseButtonMask != 0) {
        int buttonNum = 0;
        int buttonBits = pressedMouseButtonMask;

        while (buttonBits > 0) {
            buttonBits >>= 1;
            buttonNum++;
        }

        return InputSrc{ InputSrc::MOUSE_BUTTON, InputSrc::SUBAXIS_POS, (uint16_t)(buttonNum - 1)};
    }

    // Require gamepad and joystick inputs to be 75% strength or greater before they are accepted.
    // This is to try and prevent accidentally binding the wrong axis.
    const float BIND_THRESHOLD = 0.75f;

    // Check for gamepad inputs
    for (uint16_t inputIdx = 0; inputIdx < NUM_GAMEPAD_INPUTS; ++inputIdx) {
        // Get the input value
        const GamepadInput input = (GamepadInput) inputIdx;
        const float inputValue = Input::getGamepadInputValue(input);

        if (GamepadInputUtils::isAxis(input)) {
            if (inputValue >= +BIND_THRESHOLD)
                return InputSrc{ InputSrc::GAMEPAD_AXIS, InputSrc::SUBAXIS_POS, inputIdx };

            if (inputValue <= -BIND_THRESHOLD)
                return InputSrc{ InputSrc::GAMEPAD_AXIS, InputSrc::SUBAXIS_NEG, inputIdx };
        }
        else {
            if (inputValue >= +BIND_THRESHOLD)
                return InputSrc{ InputSrc::GAMEPAD_BUTTON, InputSrc::SUBAXIS_POS, inputIdx };
        }
    }

    // Check for generic joystick inputs
    for (const Input::JoystickAxis& axis : Input::getActiveJoystickAxes()) {
        if (axis.value >= +BIND_THRESHOLD)
            return InputSrc{ InputSrc::JOYSTICK_AXIS, InputSrc::SUBAXIS_POS, (uint16_t) axis.axis };

        if (axis.value <= -BIND_THRESHOLD)
            return InputSrc{ InputSrc::JOYSTICK_AXIS, InputSrc::SUBAXIS_NEG, (uint16_t) axis.axis };
    }

    if (const std::vector<uint32_t>& btns = Input::getJoystickButtonsPressed(); btns.size() > 0)
        return InputSrc{ InputSrc::JOYSTICK_BUTTON, InputSrc::SUBAXIS_POS, (uint16_t) btns.back() };

    if (const std::vector<Input::JoyHat>& hats = Input::getJoystickHatsPressed(); hats.size() > 0)
        return InputSrc{ InputSrc::JOYSTICK_HAT, InputSrc::SUBAXIS_POS, (uint16_t) hats.back() };

    return {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs an update for the 'Add input' prompt
//------------------------------------------------------------------------------------------------------------------------------------------
static void updatePrompt([[maybe_unused]] void* const pUserData) noexcept {
    // Has the prompt timed out? If so then close up the window:
    const timer_t::time_point now = timer_t::now();
    const auto secondsElapsed = std::chrono::duration_cast<std::chrono::seconds>(now - gPrompt.startTime).count();

    if (secondsElapsed >= gPrompt.duration) {
        gPrompt.pWindow->hide();
        return;
    }

    // Update the body message for the prompt
    std::snprintf(
        gPrompt.bodyMsg,
        sizeof(gPrompt.bodyMsg),
        "Press a key, button, trigger, or move a controller axis.\n"
        "The supplied input will be added to the control binding.\n"
        "\n"
        "This prompt will self-cancel in %d second(s)...",
        (int)(gPrompt.duration - secondsElapsed)
    );

    gPrompt.pBodyText->label(gPrompt.bodyMsg);

    // Check to see if there is an input being made.
    // If there are no inputs being made then just schedule another call to update the prompt in a little bit.
    gPrompt.inputSrc = getCurrentInput();

    if (gPrompt.inputSrc.device == Controls::InputSrc::NULL_DEVICE) {
        Fl::add_timeout(0.05, updatePrompt);
        return;
    }

    // An input is being made, close up the prompt:
    gPrompt.pWindow->hide();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shows the prompt to add a new input for a control binding.
// Blocks all other windows until the user enters an input or the modal times out.
// Returns the input the user supplied, if any.
// If no input was supplied by the user then the input's device will be set to 'NULL_DEVICE'.
//------------------------------------------------------------------------------------------------------------------------------------------
Controls::InputSrc show() noexcept {
    // Need to initialize SDL video subsystem in order to get global mouse state without crashing on some platforms (Linux)
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
        return {};

    // Get the screen dimensions
    int screenX = {}, screenY = {};
    int screenW = {}, screenH = {};
    Fl::screen_work_area(screenX, screenY, screenW, screenH);

    // Make a small modal window to prompt the user for input and move it to the center of the screen
    const auto pPromptWindow = std::make_unique<Fl_Double_Window>((screenW - 500) / 2, (screenH - 80) / 2, 500, 120, "Add input");
    pPromptWindow->set_modal();

    // Setup the prompt global state and also allocate the body text for the dialog
    gPrompt = {};
    gPrompt.pWindow = pPromptWindow.get();
    gPrompt.pBodyText = new Fl_Box(0, 0, 500, 120);
    gPrompt.startTime = timer_t::now();
    gPrompt.duration = 5;

    // Setup input handling.
    // Note that we can only use this module to get gamepad and joystick inputs for the prompt.
    // We can't get keyboard and mouse events via SDL because we don't have an SDL window at this point!
    Input::init();

    // Show the window and do one update of the prompt.
    // Triggering this one update will schedule other future updates.
    pPromptWindow->show();
    updatePrompt(nullptr);

    // Run the window until it's closed and while it's running receive unhandled FLTK events via a callback.
    // This is how we can get keyboard keys pressed from FLTK.
    Fl::add_handler(onAddInputPromptEvent);

    while (pPromptWindow->shown()) {
        Fl::wait();
    }

    Fl::remove_handler(onAddInputPromptEvent);

    // Done with input handling for now (the main game will setup this again later).
    // Also cleanup the previously initialized SDL video subsystem.
    Input::shutdown();
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    // Return the input that the user provided, if any:
    return gPrompt.inputSrc;
}

END_NAMESPACE(AddInputPrompt)

#endif  // #if PSYDOOM_LAUNCHER
