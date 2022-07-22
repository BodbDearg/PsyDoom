#pragma once

#include "ConfigSerialization.h"

namespace IniUtils {
    struct IniValue;
}

BEGIN_NAMESPACE(ConfigSerialization)

// N.B: must ONLY contain 'ConfigField' entries!
struct Config_Audio {
    ConfigField     audioBufferSize;
    ConfigField     spuRamSize;

    inline ConfigFieldList getFieldList() noexcept {
        static_assert(sizeof(*this) % sizeof(ConfigField) == 0);
        return { (ConfigField*) this, sizeof(*this) / sizeof(ConfigField) };
    };
};

extern Config_Audio gConfig_Audio;

void initCfgSerialization_Audio() noexcept;

END_NAMESPACE(ConfigSerialization)
