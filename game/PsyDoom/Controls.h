#pragma once

#include "Macros.h"

#include <cstdint>
#include <string>

BEGIN_NAMESPACE(Controls)

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the bindable controls/actions available
//------------------------------------------------------------------------------------------------------------------------------------------
enum class Binding : uint16_t {
    // Analog turning and movement: turning speed is subject to the gamepad turn speed settings
    Analog_TurnLeft,
    Analog_TurnRight,
    Analog_MoveForward,
    Analog_MoveBackward,
    Analog_StrafeLeft,
    Analog_StrafeRight,
    // Digital turning and movement: turn acceleration is based on the classic controls
    Digital_TurnLeft,
    Digital_TurnRight,
    Digital_MoveForward,
    Digital_MoveBackward,
    Digital_StrafeLeft,
    Digital_StrafeRight,
    // Weapon switching: relative and direct
    Weapon_ScrollUp,
    Weapon_ScrollDown,
    Weapon_Previous,
    Weapon_Next,
    Weapon_FistChainsaw,
    Weapon_Pistol,
    Weapon_Shotgun,
    Weapon_SuperShotgun,
    Weapon_Chaingun,
    Weapon_RocketLauncher,
    Weapon_PlasmaRifle,
    Weapon_BFG,
    // In-game actions and modifiers
    Action_Use,
    Action_Attack,
    Action_Respawn,
    Modifier_Run,
    Modifier_Strafe,
    Toggle_Autorun,
    // Pause/automap toggle
    Toggle_Pause,
    Toggle_Map,
    // Automap control
    Automap_ZoomIn,
    Automap_ZoomOut,
    Automap_MoveUp,
    Automap_MoveDown,
    Automap_MoveLeft,
    Automap_MoveRight,
    Automap_Pan,
    // Menu Control
    Menu_Up,
    Menu_Down,
    Menu_Left,
    Menu_Right,
    Menu_Ok,
    Menu_Start,
    Menu_Back,
    Menu_EnterPasswordChar,
    Menu_DeletePasswordChar,
    // For entering the original cheat codes only: maps to the original PSX buttons for this purpose
    PSXCheatCode_Up,
    PSXCheatCode_Down,
    PSXCheatCode_Left,
    PSXCheatCode_Right,
    PSXCheatCode_Triangle,
    PSXCheatCode_Circle,
    PSXCheatCode_Cross,
    PSXCheatCode_Square,
    PSXCheatCode_L1,
    PSXCheatCode_R1,
    PSXCheatCode_L2,
    PSXCheatCode_R2,
    // Miscellaneous
    Toggle_Renderer,
    Toggle_ViewPlayer,  // Playback of multiplayer demos: toggle which player is being viewed
    Quicksave,
    Quickload,
    // How many bindings there are
    NUM_BINDINGS
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds a single button, key or axis that is bound to a particular action
//------------------------------------------------------------------------------------------------------------------------------------------
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

    // Which portion of the axis to use, positive or negative
    enum : uint8_t {
        SUBAXIS_POS,        // Only use the 0.0 to +1.0 range of the axis and return it as a 0.0 to 1.0 axis
        SUBAXIS_NEG         // Only use the 0.0 to -1.0 range of the axis and return it as a 0.0 to 1.0 axis
    } subaxis;

    // What particular button or axis is used
    uint16_t input;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// The maximum number of inputs per control binding
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint32_t MAX_BINDING_INPUTS = 15;

//------------------------------------------------------------------------------------------------------------------------------------------
// Stores all of the input sources for a particular binding
//------------------------------------------------------------------------------------------------------------------------------------------
struct BindingData {
    uint32_t    numInputSources;
    InputSrc    inputSources[MAX_BINDING_INPUTS];
};

void init() noexcept;
void shutdown() noexcept;
bool didInit() noexcept;

void clearAllBindings() noexcept;
void parseBinding(const Binding binding, const char* const str) noexcept;
const BindingData& getBindingData(const Binding binding) noexcept;

float getFloat(const Binding binding) noexcept;
bool getBool(const Binding binding) noexcept;
bool isJustPressed(const Binding binding) noexcept;
bool isJustReleased(const Binding binding) noexcept;
uint16_t getPSXCheatButtonBits() noexcept;

void appendInputSrcToStr(const InputSrc& src, std::string& outputStr) noexcept;
void bindingToStr(const Binding binding, std::string& outputStr) noexcept;

END_NAMESPACE(Controls)
