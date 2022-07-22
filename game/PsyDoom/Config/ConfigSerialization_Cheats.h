#pragma once

#include "ConfigSerialization.h"

namespace IniUtils {
    struct IniValue;
}

BEGIN_NAMESPACE(ConfigSerialization)

// N.B: must ONLY contain 'ConfigField' entries!
struct Config_Cheats {
    ConfigField     enableDevCheatShortcuts;
    ConfigField     enableDevInPlaceReloadFunctionKey;
    ConfigField     enableDevMapAutoReload;
    ConfigField     cheatKeys_godMode;
    ConfigField     cheatKeys_noClip;
    ConfigField     cheatKeys_levelWarp;
    ConfigField     cheatKeys_weaponsKeysAndArmor;
    ConfigField     cheatKeys_allMapLinesOn;
    ConfigField     cheatKeys_allMapThingsOn;
    ConfigField     cheatKeys_xrayVision;
    ConfigField     cheatKeys_vramViewer;
    ConfigField     cheatKeys_noTarget;

    inline ConfigFieldList getFieldList() noexcept {
        static_assert(sizeof(*this) % sizeof(ConfigField) == 0);
        return { (ConfigField*) this, sizeof(*this) / sizeof(ConfigField) };
    };
};

extern Config_Cheats gConfig_Cheats;

void initCfgSerialization_Cheats() noexcept;

END_NAMESPACE(ConfigSerialization)
