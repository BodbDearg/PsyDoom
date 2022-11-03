#pragma once

#include "ConfigSerialization.h"

namespace IniUtils {
    struct IniValue;
}

BEGIN_NAMESPACE(ConfigSerialization)

// N.B: must ONLY contain 'ConfigField' entries!
struct Config_Multiplayer {
    ConfigField     coopNoFriendlyFire;
    ConfigField     dmFragLimit;
    ConfigField     dmExitDisabled;
    ConfigField     coopPreserveAmmoFactor;
    ConfigField     coopPreserveKeys;
    ConfigField     coopForceSpawnMpThings;
    ConfigField     dmActivateSpecialSectors;

    inline ConfigFieldList getFieldList() noexcept {
        static_assert(sizeof(*this) % sizeof(ConfigField) == 0);
        return { (ConfigField*) this, sizeof(*this) / sizeof(ConfigField) };
    };
};

extern Config_Multiplayer gConfig_Multiplayer;

void initCfgSerialization_Multiplayer() noexcept;

END_NAMESPACE(ConfigSerialization)
