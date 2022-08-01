#pragma once

#include "ConfigSerialization.h"

namespace IniUtils {
    struct IniValue;
}

BEGIN_NAMESPACE(ConfigSerialization)

extern const char* const CONTROL_BINDINGS_INI_HEADER;

// N.B: must ONLY contain 'ConfigField' entries!
struct Config_Controls {
    ConfigField     analog_moveForward;
    ConfigField     analog_moveBackward;
    ConfigField     analog_strafeLeft;
    ConfigField     analog_strafeRight;
    ConfigField     analog_turnLeft;
    ConfigField     analog_turnRight;
    ConfigField     digital_moveForward;
    ConfigField     digital_moveBackward;
    ConfigField     digital_strafeLeft;
    ConfigField     digital_strafeRight;
    ConfigField     digital_turnLeft;
    ConfigField     digital_turnRight;
    ConfigField     action_use;
    ConfigField     action_attack;
    ConfigField     action_respawn;
    ConfigField     modifier_run;
    ConfigField     modifier_strafe;
    ConfigField     toggle_autorun;
    ConfigField     quicksave;
    ConfigField     quickload;
    ConfigField     toggle_pause;
    ConfigField     toggle_map;
    ConfigField     toggle_renderer;
    ConfigField     toggle_viewPlayer;
    ConfigField     weapon_scrollUp;
    ConfigField     weapon_scrollDown;
    ConfigField     weapon_previous;
    ConfigField     weapon_next;
    ConfigField     weapon_fistChainsaw;
    ConfigField     weapon_pistol;
    ConfigField     weapon_shotgun;
    ConfigField     weapon_superShotgun;
    ConfigField     weapon_chaingun;
    ConfigField     weapon_rocketLauncher;
    ConfigField     weapon_plasmaRifle;
    ConfigField     weapon_bfg;
    ConfigField     menu_up;
    ConfigField     menu_down;
    ConfigField     menu_left;
    ConfigField     menu_right;
    ConfigField     menu_ok;
    ConfigField     menu_back;
    ConfigField     menu_start;
    ConfigField     menu_enterPasswordChar;
    ConfigField     menu_deletePasswordChar;
    ConfigField     automap_zoomIn;
    ConfigField     automap_zoomOut;
    ConfigField     automap_moveUp;
    ConfigField     automap_moveDown;
    ConfigField     automap_moveLeft;
    ConfigField     automap_moveRight;
    ConfigField     automap_pan;
    ConfigField     psxCheatCode_up;
    ConfigField     psxCheatCode_down;
    ConfigField     psxCheatCode_left;
    ConfigField     psxCheatCode_right;
    ConfigField     psxCheatCode_triangle;
    ConfigField     psxCheatCode_circle;
    ConfigField     psxCheatCode_cross;
    ConfigField     psxCheatCode_square;
    ConfigField     psxCheatCode_l1;
    ConfigField     psxCheatCode_r1;
    ConfigField     psxCheatCode_l2;
    ConfigField     psxCheatCode_r2;

    inline ConfigFieldList getFieldList() noexcept {
        static_assert(sizeof(*this) % sizeof(ConfigField) == 0);
        return { (ConfigField*) this, sizeof(*this) / sizeof(ConfigField) };
    };
};

extern Config_Controls gConfig_Controls;

void initCfgSerialization_Controls() noexcept;

END_NAMESPACE(ConfigSerialization)
