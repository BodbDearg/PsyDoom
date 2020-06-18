#include "ControllerInput.h"

#include <SDL.h>

BEGIN_NAMESPACE(ControllerInputUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if an input is an axis of some sort
//------------------------------------------------------------------------------------------------------------------------------------------
bool isAxis(const ControllerInput input) noexcept {
    switch (input) {
        case ControllerInput::AXIS_LEFT_X:
        case ControllerInput::AXIS_LEFT_Y:
        case ControllerInput::AXIS_RIGHT_X:
        case ControllerInput::AXIS_RIGHT_Y:
        case ControllerInput::AXIS_TRIG_LEFT:
        case ControllerInput::AXIS_TRIG_RIGHT:
            return true;

        default:
            return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// If an axis has an opposite axis (complementing 2d axis pair) then the function returns the opposite axis.
// Otherwise the input axis is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
ControllerInput getOppositeAxis(const ControllerInput input) noexcept {
    switch (input) {
        case ControllerInput::AXIS_LEFT_X:      return ControllerInput::AXIS_LEFT_Y;
        case ControllerInput::AXIS_LEFT_Y:      return ControllerInput::AXIS_LEFT_X;
        case ControllerInput::AXIS_RIGHT_X:     return ControllerInput::AXIS_RIGHT_Y;
        case ControllerInput::AXIS_RIGHT_Y:     return ControllerInput::AXIS_RIGHT_X;

        default:
            return input;
    }    
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL button to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
ControllerInput sdlButtonToInput(const uint8_t button) noexcept {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_A:               return ControllerInput::BTN_A;
        case SDL_CONTROLLER_BUTTON_B:               return ControllerInput::BTN_B;
        case SDL_CONTROLLER_BUTTON_X:               return ControllerInput::BTN_X;
        case SDL_CONTROLLER_BUTTON_Y:               return ControllerInput::BTN_Y;
        case SDL_CONTROLLER_BUTTON_BACK:            return ControllerInput::BTN_BACK;
        case SDL_CONTROLLER_BUTTON_GUIDE:           return ControllerInput::BTN_GUIDE;
        case SDL_CONTROLLER_BUTTON_START:           return ControllerInput::BTN_START;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:       return ControllerInput::BTN_LEFT_STICK;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:      return ControllerInput::BTN_RIGHT_STICK;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:    return ControllerInput::BTN_LEFT_SHOULDER;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:   return ControllerInput::BTN_RIGHT_SHOULDER;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:         return ControllerInput::BTN_DPAD_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:       return ControllerInput::BTN_DPAD_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:       return ControllerInput::BTN_DPAD_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:      return ControllerInput::BTN_DPAD_RIGHT;

        default:
            return ControllerInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
ControllerInput sdlAxisToInput(const uint8_t axis) noexcept {
    switch (axis) {
        case SDL_CONTROLLER_AXIS_LEFTX:             return ControllerInput::AXIS_LEFT_X;
        case SDL_CONTROLLER_AXIS_LEFTY:             return ControllerInput::AXIS_LEFT_Y;
        case SDL_CONTROLLER_AXIS_RIGHTX:            return ControllerInput::AXIS_RIGHT_X;
        case SDL_CONTROLLER_AXIS_RIGHTY:            return ControllerInput::AXIS_RIGHT_Y;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:       return ControllerInput::AXIS_TRIG_LEFT;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:      return ControllerInput::AXIS_TRIG_RIGHT;

        default:
            return ControllerInput::INVALID;
    }
}

END_NAMESPACE(ControllerInputUtils)
