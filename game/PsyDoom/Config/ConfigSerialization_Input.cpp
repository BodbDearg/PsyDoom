#include "ConfigSerialization_Input.h"

#include "Config.h"
#include "IniUtils.h"

#include <string>

BEGIN_NAMESPACE(ConfigSerialization)

using namespace Config;

//------------------------------------------------------------------------------------------------------------------------------------------
// Config field storage
//------------------------------------------------------------------------------------------------------------------------------------------
Config_Input gConfig_Input = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the config serializers for input related config
//------------------------------------------------------------------------------------------------------------------------------------------
void initCfgSerialization_Input() noexcept {
    auto& cfg = gConfig_Input;

    cfg.mouseTurnSpeed = makeConfigField(
        "MouseTurnSpeed",
        "How much turning movement to apply per pixel of mouse movement",
        gMouseTurnSpeed,
        7.0f
    );

    cfg.gamepadDeadZone = makeConfigField(
        "GamepadDeadZone",
        "0-1 range: controls when minor controller inputs are discarded.\n"
        "\n"
        "The default of '0.125' only registers movement if the stick is at least 12.5% moved.\n"
        "Setting too low may result in unwanted jitter and movement when the controller is resting.",
        gGamepadDeadZone,
        0.125f
    );

    cfg.gamepadFastTurnSpeed_High = makeConfigField(
        "GamepadFastTurnSpeed_High",
        "How fast to turn when running ('FastTurnSpeed') and when NOT running ('TurnSpeed').\n"
        "\n"
        "The game will mix between the 'High' and 'Low' speed values for when running or walking depending\n"
        "on how far the stick is pushed, using the 'high' speed value completely when the gamepad axis\n"
        "fully pushed in it's move direction. This replaces the accelerating turning movement of the\n"
        "original game and allows for more precise control.\n"
        "\n"
        "For reference, the original speed value ranges with the PSX D-PAD were:\n"
        " Walk: 300 - 1000\n"
        " Run:  800 - 1400",
        gGamepadFastTurnSpeed_High,
        1400.0f
    );

    cfg.gamepadFastTurnSpeed_Low = makeConfigField(
        "GamepadFastTurnSpeed_Low",
        "",
        gGamepadFastTurnSpeed_Low,
        800.0f
    );

    cfg.gamepadTurnSpeed_High = makeConfigField(
        "GamepadTurnSpeed_High",
        "",
        gGamepadTurnSpeed_High,
        1000.0f
    );

    cfg.gamepadTurnSpeed_Low = makeConfigField(
        "GamepadTurnSpeed_Low",
        "",
        gGamepadTurnSpeed_Low,
        600.0f
    );

    cfg.analogToDigitalThreshold = makeConfigField(
        "AnalogToDigitalThreshold",
        "0-1 range: controls the point at which an analog axis like a trigger, stick etc. is regarded\n"
        "as 'pressed' when treated as a digital input (e.g trigger used for 'shoot' action).\n"
        "\n"
        "The default of '0.5' (halfway depressed) is probably reasonable for most users.",
        gAnalogToDigitalThreshold,
        0.5f
    );
}

END_NAMESPACE(ConfigSerialization)
