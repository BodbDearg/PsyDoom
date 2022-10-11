#include "ConfigSerialization_Multiplayer.h"

#include "Config.h"
#include "IniUtils.h"

#include <string>

BEGIN_NAMESPACE(ConfigSerialization)

using namespace Config;

//------------------------------------------------------------------------------------------------------------------------------------------
// Config field storage
//------------------------------------------------------------------------------------------------------------------------------------------
Config_Multiplayer gConfig_Multiplayer = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the config serializers for multiplayer related config
//------------------------------------------------------------------------------------------------------------------------------------------
void initCfgSerialization_Multiplayer() noexcept {
    auto& cfg = gConfig_Multiplayer;

    cfg.noFriendlyFire = makeConfigField(
        "NoFriendlyFire",
        "Players to not take damage from other players when playing coop.",
        gbNoFriendlyFire,
        true
    );

    cfg.fragLimit = makeConfigField(
        "FragLimit",
        "Set Frag limit for death-match. Level will exit when limit is reached.\n"
        "0 = infinite",
        gFragLimit,
        0
    );
}

END_NAMESPACE(ConfigSerialization)
