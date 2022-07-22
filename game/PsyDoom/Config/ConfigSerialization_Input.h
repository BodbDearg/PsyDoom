#pragma once

#include "ConfigSerialization.h"

namespace IniUtils {
    struct IniValue;
}

BEGIN_NAMESPACE(ConfigSerialization)

// N.B: must ONLY contain 'ConfigField' entries!
struct Config_Input {
    ConfigField     mouseTurnSpeed;
    ConfigField     gamepadDeadZone;
    ConfigField     gamepadFastTurnSpeed_High;
    ConfigField     gamepadFastTurnSpeed_Low;
    ConfigField     gamepadTurnSpeed_High;
    ConfigField     gamepadTurnSpeed_Low;
    ConfigField     analogToDigitalThreshold;

    inline ConfigFieldList getFieldList() noexcept {
        static_assert(sizeof(*this) % sizeof(ConfigField) == 0);
        return { (ConfigField*) this, sizeof(*this) / sizeof(ConfigField) };
    };
};

extern Config_Input gConfig_Input;

void initCfgSerialization_Input() noexcept;

END_NAMESPACE(ConfigSerialization)
