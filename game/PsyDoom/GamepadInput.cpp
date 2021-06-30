#include "GamepadInput.h"

#include <SDL.h>

BEGIN_NAMESPACE(GamepadInputUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if an input is an axis of some sort
//------------------------------------------------------------------------------------------------------------------------------------------
bool isAxis(const GamepadInput input) noexcept {
    switch (input) {
        case GamepadInput::AXIS_LEFT_X:
        case GamepadInput::AXIS_LEFT_Y:
        case GamepadInput::AXIS_RIGHT_X:
        case GamepadInput::AXIS_RIGHT_Y:
        case GamepadInput::AXIS_TRIG_LEFT:
        case GamepadInput::AXIS_TRIG_RIGHT:
            return true;

        default:
            return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// If an axis has an opposite axis (complementing 2d axis pair) then the function returns the opposite axis.
// Otherwise the input axis is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput getOppositeAxis(const GamepadInput input) noexcept {
    switch (input) {
        case GamepadInput::AXIS_LEFT_X:     return GamepadInput::AXIS_LEFT_Y;
        case GamepadInput::AXIS_LEFT_Y:     return GamepadInput::AXIS_LEFT_X;
        case GamepadInput::AXIS_RIGHT_X:    return GamepadInput::AXIS_RIGHT_Y;
        case GamepadInput::AXIS_RIGHT_Y:    return GamepadInput::AXIS_RIGHT_X;

        default:
            return input;
    }    
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL button to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlButtonToInput(const uint8_t button) noexcept {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_A:               return GamepadInput::BTN_A;
        case SDL_CONTROLLER_BUTTON_B:               return GamepadInput::BTN_B;
        case SDL_CONTROLLER_BUTTON_X:               return GamepadInput::BTN_X;
        case SDL_CONTROLLER_BUTTON_Y:               return GamepadInput::BTN_Y;
        case SDL_CONTROLLER_BUTTON_BACK:            return GamepadInput::BTN_BACK;
        case SDL_CONTROLLER_BUTTON_GUIDE:           return GamepadInput::BTN_GUIDE;
        case SDL_CONTROLLER_BUTTON_START:           return GamepadInput::BTN_START;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:       return GamepadInput::BTN_LEFT_STICK;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:      return GamepadInput::BTN_RIGHT_STICK;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:    return GamepadInput::BTN_LEFT_SHOULDER;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:   return GamepadInput::BTN_RIGHT_SHOULDER;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:         return GamepadInput::BTN_DPAD_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:       return GamepadInput::BTN_DPAD_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:       return GamepadInput::BTN_DPAD_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:      return GamepadInput::BTN_DPAD_RIGHT;

        default:
            return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlAxisToInput(const uint8_t axis) noexcept {
    switch (axis) {
        case SDL_CONTROLLER_AXIS_LEFTX:             return GamepadInput::AXIS_LEFT_X;
        case SDL_CONTROLLER_AXIS_LEFTY:             return GamepadInput::AXIS_LEFT_Y;
        case SDL_CONTROLLER_AXIS_RIGHTX:            return GamepadInput::AXIS_RIGHT_X;
        case SDL_CONTROLLER_AXIS_RIGHTY:            return GamepadInput::AXIS_RIGHT_Y;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:       return GamepadInput::AXIS_TRIG_LEFT;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:      return GamepadInput::AXIS_TRIG_RIGHT;

        default:
            return GamepadInput::INVALID;
    }
}

END_NAMESPACE(GamepadInputUtils)
