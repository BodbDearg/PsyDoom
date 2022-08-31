#include "ConfigSerialization_Controls.h"

#include "Config.h"
#include "IniUtils.h"
#include "PsyDoom/Controls.h"

#include <string>

BEGIN_NAMESPACE(ConfigSerialization)

using namespace Config;

//------------------------------------------------------------------------------------------------------------------------------------------
// Config field storage
//------------------------------------------------------------------------------------------------------------------------------------------
Config_Controls gConfig_Controls = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// A documentation header to place at the start of the controls INI file
//------------------------------------------------------------------------------------------------------------------------------------------
const char* const CONTROL_BINDINGS_INI_HEADER = 
R"(#---------------------------------------------------------------------------------------------------
# Control bindings: available input source names/identifiers.
#
# Assign these inputs to actions listed below to setup control bindings.
# Separate multiple input sources for one action using commas (,).
#
# Notes:
#   (1) Gamepad inputs (i.e. Xbox controller style inputs) can only be used for certain types of
#       game controllers which are supported and recognized by the SDL library that PsyDoom uses.
#       If you find your controller is not supported, use generic/numbered joystick inputs instead.
#   (2) All input source names are case insensitive.
#   (3) The ',' (comma) keyboard key must be escaped/prefixed by backslash (\) when used as an input:
#           \,
#   (4) Similar keyboard keys are collapsed into a range for brevity (e.g A-Z).
#   (5) For a full list of available keyboard key names, including very uncommon ones, see:
#         https://wiki.libsdl.org/SDL_Scancode
#
# Mouse buttons:
#       Mouse Left              Mouse X1
#       Mouse Right             Mouse X2
#       Mouse Middle
#
# Mouse wheel axes:
#       Mouse Wheel+
#       Mouse Wheel-
#
# SDL recognized gamepad, axes:
#       Gamepad LeftTrigger     Gamepad RightTrigger
#       Gamepad LeftX-          Gamepad LeftX+          Gamepad LeftY-          Gamepad LeftY+
#       Gamepad RightX-         Gamepad RightX+         Gamepad RightY-         Gamepad RightY+
#
# SDL recognized gamepad, buttons:
#       Gamepad A               Gamepad DpUp            Gamepad LeftStick
#       Gamepad B               Gamepad DpDown          Gamepad RightStick
#       Gamepad X               Gamepad DpLeft          Gamepad LeftShoulder
#       Gamepad Y               Gamepad DpRight         Gamepad RightShoulder
#       Gamepad Back            Gamepad Start           Gamepad Guide
#
# Generic joystick inputs: axes, buttons and hat/d-pad directions.
# Replace '[1-99]' with the desired button, hat or axis number:
#       Joystick Button[1-99]   Joystick Hat[1-99] Up
#       Joystick Axis[1-99]+    Joystick Hat[1-99] Down
#       Joystick Axis[1-99]-    Joystick Hat[1-99] Left
#                               Joystick Hat[1-99] Right
#
# Keyboard keys (commonly used, see link above for full list):
#       A-Z                     Return                  Backspace               Home
#       0-9                     Escape                  Pause                   End
#       Keypad 0-9              Space                   PageUp                  Insert
#       F1-F12                  Tab                     PageDown                Delete
#       Left                    Right                   PrintScreen             CapsLock
#       Up                      Down                    ScrollLock              Numlock
#       Left Ctrl               Left Shift              Left Alt                Application
#       Right Ctrl              Right Shift             Right Alt               Menu
#       Left GUI                Right GUI               -                       = 
#       [                       ]                       \                       #
#       ;                       '                       \,                      `
#       .                       /                       Keypad /                Keypad *
#       Keypad -                Keypad +                Keypad Enter            Keypad .
#---------------------------------------------------------------------------------------------------

)";

//------------------------------------------------------------------------------------------------------------------------------------------
// Helpers that define a control binding config field
//------------------------------------------------------------------------------------------------------------------------------------------
static ConfigField makeControlConfigField(
    const char* const comment,
    const char* const name,
    Controls::Binding binding,
    const char* const defaultSetting
) noexcept {
    return makeConfigField(
        name,
        comment,
        [=](const IniUtils::IniValue& value) noexcept { Controls::parseBinding(binding, value.strValue.c_str()); },
        [=](IniUtils::IniValue& valueOut) noexcept { Controls::bindingToStr(binding, valueOut.strValue); },
        [=]() noexcept { Controls::parseBinding(binding, defaultSetting); }
    );
}

static ConfigField makeControlConfigField(
    const char* const name,
    Controls::Binding binding,
    const char* const defaultSetting
) noexcept {
    return makeControlConfigField("", name, binding, defaultSetting);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the config serializers for control related config
//------------------------------------------------------------------------------------------------------------------------------------------
void initCfgSerialization_Controls() noexcept {
    auto& cfg = gConfig_Controls;

    // Helper macros to remove some of the repetition
    #define CONTROL_FIELD(BINDING_NAME, DEFAULT_VALUE)\
        makeControlConfigField(#BINDING_NAME, Controls::Binding::BINDING_NAME, DEFAULT_VALUE)

    #define CONTROL_FIELD_WITH_DOC(COMMENT, BINDING_NAME, DEFAULT_VALUE)\
        makeControlConfigField(COMMENT, #BINDING_NAME, Controls::Binding::BINDING_NAME, DEFAULT_VALUE)

    // Analog movement and turning actions
    cfg.analog_moveForward = CONTROL_FIELD_WITH_DOC(
        "Analog movement and turning actions.\n"
        "Note: analog turn sensitivity can be modified via the 'Input' section.",
        Analog_MoveForward,
        "Gamepad LeftY-"
    );

    cfg.analog_moveBackward = CONTROL_FIELD(Analog_MoveBackward, "Gamepad LeftY+");
    cfg.analog_strafeLeft = CONTROL_FIELD(Analog_StrafeLeft, "Gamepad LeftX-");
    cfg.analog_strafeRight = CONTROL_FIELD(Analog_StrafeRight, "Gamepad LeftX+");
    cfg.analog_turnLeft = CONTROL_FIELD(Analog_TurnLeft, "Gamepad RightX-");
    cfg.analog_turnRight = CONTROL_FIELD(Analog_TurnRight, "Gamepad RightX+");

    // Digital movement and turning actions
    cfg.digital_moveForward = CONTROL_FIELD_WITH_DOC(
        "Digital movement and turning actions. Turn sensitivity and acceleration are based on the original\n"
        "PSX values, though greater precision can be achieved if using uncapped framerates.",
        Digital_MoveForward,
        "W, Up, Gamepad DpUp"
    );

    cfg.digital_moveBackward = CONTROL_FIELD(Digital_MoveBackward, "S, Down, Gamepad DpDown");
    cfg.digital_strafeLeft = CONTROL_FIELD(Digital_StrafeLeft, "A");
    cfg.digital_strafeRight = CONTROL_FIELD(Digital_StrafeRight, "D");
    cfg.digital_turnLeft = CONTROL_FIELD(Digital_TurnLeft, "Left, Gamepad DpLeft");
    cfg.digital_turnRight = CONTROL_FIELD(Digital_TurnRight, "Right, Gamepad DpRight");

    // In-game actions & modifiers
    cfg.action_use = CONTROL_FIELD_WITH_DOC(
        "In-game actions & modifiers",
        Action_Use,
        "Space, E, Mouse Right, Gamepad B"
    );

    cfg.action_attack = CONTROL_FIELD(Action_Attack, "Mouse Left, Gamepad RightTrigger, Left Ctrl, Right Ctrl, Gamepad Y");
    cfg.action_respawn = CONTROL_FIELD(Action_Respawn, "Mouse Left, Gamepad RightTrigger, Left Ctrl, Right Ctrl, Gamepad Y");
    cfg.modifier_run = CONTROL_FIELD(Modifier_Run, "Left Shift, Right Shift, Gamepad X, Gamepad LeftTrigger");
    cfg.modifier_strafe = CONTROL_FIELD(Modifier_Strafe, "Left Alt, Right Alt, Gamepad A");
    cfg.toggle_autorun = CONTROL_FIELD(Toggle_Autorun, "CapsLock");
    cfg.quicksave = CONTROL_FIELD(Quicksave, "F5");
    cfg.quickload = CONTROL_FIELD(Quickload, "F9");

    // Toggles
    cfg.toggle_pause = CONTROL_FIELD_WITH_DOC(
        "Toggle in-game pause, automap, and toggle between the Classic and Vulkan renderer (if possible).\n"
        "Also a control to toggle which player is viewed when playing back multiplayer demos.",
        Toggle_Pause,
        "Escape, P, Pause, Gamepad Start"
    );

    cfg.toggle_map = CONTROL_FIELD(Toggle_Map, "Tab, M, Gamepad Back");
    cfg.toggle_renderer = CONTROL_FIELD(Toggle_Renderer, "`");
    cfg.toggle_viewPlayer = CONTROL_FIELD(Toggle_ViewPlayer, "V");

    // Weapon switching
    cfg.weapon_scrollUp = CONTROL_FIELD_WITH_DOC(
        "Weapon switching: relative and absolute.\n"
        "\n"
        "Note that the weapon 'scrollUp/Down' actions work much better with the mouse wheel since they\n"
        "allow multiple weapons to be scrolled past in one frame. This helps rapid scrolling feel much\n"
        "more responsive.",
        Weapon_ScrollUp,
        "Mouse Wheel+"
    );
    cfg.weapon_scrollDown = CONTROL_FIELD(Weapon_ScrollDown, "Mouse Wheel-");
    cfg.weapon_previous = CONTROL_FIELD(Weapon_Previous, "PageDown, [, Gamepad LeftShoulder");
    cfg.weapon_next = CONTROL_FIELD(Weapon_Next, "PageUp, ], Gamepad RightShoulder");
    cfg.weapon_fistChainsaw = CONTROL_FIELD(Weapon_FistChainsaw, "1");
    cfg.weapon_pistol = CONTROL_FIELD(Weapon_Pistol, "2");
    cfg.weapon_shotgun = CONTROL_FIELD(Weapon_Shotgun, "3");
    cfg.weapon_superShotgun = CONTROL_FIELD(Weapon_SuperShotgun, "4");
    cfg.weapon_chaingun = CONTROL_FIELD(Weapon_Chaingun, "5");
    cfg.weapon_rocketLauncher = CONTROL_FIELD(Weapon_RocketLauncher, "6");
    cfg.weapon_plasmaRifle = CONTROL_FIELD(Weapon_PlasmaRifle, "7");
    cfg.weapon_bfg = CONTROL_FIELD(Weapon_BFG, "8");

    // Menu & UI controls
    cfg.menu_up = CONTROL_FIELD_WITH_DOC(
        "Menu & UI controls",
        Menu_Up,
        "Up, Gamepad DpUp, Gamepad LeftY-, Gamepad RightY-"
    );

    cfg.menu_down = CONTROL_FIELD(Menu_Down, "Down, Gamepad DpDown, Gamepad LeftY+, Gamepad RightY+");
    cfg.menu_left = CONTROL_FIELD(Menu_Left, "Left, Gamepad DpLeft, Gamepad LeftX-, Gamepad RightX-");
    cfg.menu_right = CONTROL_FIELD(Menu_Right, "Right, Gamepad DpRight, Gamepad LeftX+, Gamepad RightX+");
    cfg.menu_ok = CONTROL_FIELD(Menu_Ok, "Return, Space, Mouse Left, Left Ctrl, Right Ctrl, Gamepad A, Gamepad RightTrigger");
    cfg.menu_back = CONTROL_FIELD(Menu_Back, "Escape, Tab, Mouse Right, Gamepad B, Gamepad Back");
    cfg.menu_start = CONTROL_FIELD(Menu_Start, "Gamepad Start");
    cfg.menu_enterPasswordChar = CONTROL_FIELD(Menu_EnterPasswordChar, "Return, Space, Mouse Left, Left Ctrl, Right Ctrl, Gamepad A, Gamepad RightTrigger");
    cfg.menu_deletePasswordChar = CONTROL_FIELD(Menu_DeletePasswordChar, "Delete, Backspace, Mouse Right, Gamepad X, Gamepad LeftTrigger");

    // Automap controls
    cfg.automap_zoomIn = CONTROL_FIELD_WITH_DOC(
        "Automap controls",
        Automap_ZoomIn,
        "=, Keypad +, Gamepad RightTrigger"
    );

    cfg.automap_zoomOut = CONTROL_FIELD(Automap_ZoomOut, "-, Keypad -, Gamepad LeftTrigger");
    cfg.automap_moveUp = CONTROL_FIELD(Automap_MoveUp, "Up, W, Gamepad DpUp, Gamepad LeftY-, Gamepad RightY-");
    cfg.automap_moveDown = CONTROL_FIELD(Automap_MoveDown, "Down, S, Gamepad DpDown, Gamepad LeftY+, Gamepad RightY+");
    cfg.automap_moveLeft = CONTROL_FIELD(Automap_MoveLeft, "Left, A, Gamepad DpLeft, Gamepad LeftX-, Gamepad RightX-");
    cfg.automap_moveRight = CONTROL_FIELD(Automap_MoveRight, "Right, D, Gamepad DpRight, Gamepad LeftX+, Gamepad RightX+");
    cfg.automap_pan = CONTROL_FIELD(Automap_Pan, "F, Left Alt, Right Alt, Gamepad A");

    // PSX button bindings for cheat codes
    cfg.psxCheatCode_up = CONTROL_FIELD_WITH_DOC(
        "Mappings to the original PlayStation controller buttons for the sole purpose of entering cheat\n"
        "code sequences on the pause menu, the original way.\n"
        "\n"
        "For example inputs mapped to the PSX 'Cross' button will be interpreted as that while attempting\n"
        "to enter an original cheat code sequence.",
        PSXCheatCode_Up,
        "Up, W, Gamepad DpUp, Gamepad LeftY-, Gamepad RightY-"
    );

    cfg.psxCheatCode_down = CONTROL_FIELD(PSXCheatCode_Down, "Down, S, Gamepad DpDown, Gamepad LeftY+, Gamepad RightY+");
    cfg.psxCheatCode_left = CONTROL_FIELD(PSXCheatCode_Left, "Left, Gamepad DpLeft, Gamepad LeftX-, Gamepad RightX-");
    cfg.psxCheatCode_right = CONTROL_FIELD(PSXCheatCode_Right, "Right, Gamepad DpRight, Gamepad LeftX+, Gamepad RightX+");
    cfg.psxCheatCode_triangle = CONTROL_FIELD(PSXCheatCode_Triangle, "Mouse Left, Left Ctrl, Right Ctrl, Gamepad Y");
    cfg.psxCheatCode_circle = CONTROL_FIELD(PSXCheatCode_Circle, "Space, Mouse Right, Gamepad B");
    cfg.psxCheatCode_cross = CONTROL_FIELD(PSXCheatCode_Cross, "F, Left Alt, Right Alt, Gamepad A");
    cfg.psxCheatCode_square = CONTROL_FIELD(PSXCheatCode_Square, "Left Shift, Right Shift, Gamepad X");
    cfg.psxCheatCode_l1 = CONTROL_FIELD(PSXCheatCode_L1, "A, Gamepad LeftShoulder");
    cfg.psxCheatCode_r1 = CONTROL_FIELD(PSXCheatCode_R1, "D, Gamepad RightShoulder");
    cfg.psxCheatCode_l2 = CONTROL_FIELD(PSXCheatCode_L2, "PageDown, [, Gamepad LeftTrigger");
    cfg.psxCheatCode_r2 = CONTROL_FIELD(PSXCheatCode_R2, "PageUp, ], Gamepad RightTrigger");

    // Done with these
    #undef CONTROL_FIELD
    #undef CONTROL_FIELD_WITH_DOC
}

END_NAMESPACE(ConfigSerialization)
