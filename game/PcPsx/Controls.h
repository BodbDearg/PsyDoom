#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(Controls)

// All of the bindable controls/actions available
enum class Binding : uint16_t {
    // Analog movement and turning: turning speed is subject to the gamepad turn speed settings
    Analog_MoveForwardBack,
    Analog_MoveLeftRight,
    Analog_Turn,
    // Digital movement and turning: turn acceleration is based on the classic controls
    Digital_TurnLeft,
    Digital_TurnRight,
    Digital_MoveForward,
    Digital_MoveBackward,
    Digital_StrafeLeft,
    Digital_StrafeRight,
    // Weapon switching: relative and direct
    Weapon_Scroll,
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
};

static constexpr uint16_t NUM_BINDINGS = (uint16_t) Binding::PSXCheatCode_R2 + 1;

void init() noexcept;
void shutdown() noexcept;
void clearAllBindings() noexcept;
void parseBinding(const Binding binding, const char* str) noexcept;
float getFloat(const Binding binding) noexcept;
bool getBool(const Binding binding) noexcept;
uint16_t getPSXCheatButtonBits() noexcept;

END_NAMESPACE(Controls)
