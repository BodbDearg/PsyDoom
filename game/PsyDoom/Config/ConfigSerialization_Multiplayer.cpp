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
        "Players will not take damage from other players when playing co-op.\n"
        "Note: players can still take damage from nearby exploding barrels.",
        gbNoFriendlyFire,
        false
    );

    cfg.preserveKeys = makeConfigField(
        "PreserveKeys",
        "Players retain previously collected keys when respawning.",
        gbPreserveKeys,
        false
    );

    cfg.exitDisabled = makeConfigField(
        "ExitDisabled",
        "Prevents exiting the level when playing deathmatch.\n"
        "Note: if frag limit is less than 1, exits will not be disabled.",
        gbExitDisabled,
        false
    );

    cfg.fragLimit = makeConfigField(
        "FragLimit",
        "Set frag limit for deathmatch. Level will exit when limit is reached.\n"
        "0 = disabled",
        gFragLimit,
        0
    );

    cfg.preserveAmmoFactor = makeConfigField(
        "PreserveAmmoFactor",
        "Determines whether a player keeps collected ammo and backpack when respawning after death.\n"
        "\n"
        "Allowed values:\n"
        " - None: Lose all ammo and backpack; bullets restored to 50 (0) (default setting)\n"
        " - All: Retain all ammo and keep backpack; bullets restored to at least 50 (1)\n"
        " - Half: Retain half ammo and keep backpack; bullets restored to at least 50 (2)",
        gPreserveAmmoFactor,
        0
    );
}

END_NAMESPACE(ConfigSerialization)
