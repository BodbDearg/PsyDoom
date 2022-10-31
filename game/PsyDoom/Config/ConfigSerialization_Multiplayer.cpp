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

    cfg.coopNoFriendlyFire = makeConfigField(
        "NoFriendlyFire",
        "Cooperative: Players will not take damage from other players.\n"
        "\n"
        "Note: players can still take damage from nearby exploding barrels\n"
        "and being telefragged by the other player.",
        gbCoopNoFriendlyFire,
        false
    );

    cfg.coopForceSpawnMpThings = makeConfigField(
        "MPThings",
        "Cooperative: This will add things to the map that are tagged for multiplayer only,\n"
        "such as: additional monsters, weapons, and powerups.\n"
        "\n"
        "Note: the additional weapons and powerups are normally only found in deathmatch.\n"
        "The additional monsters were possibly intended for another game mode that was\n"
        "never implemented or carried over from PC Doom and never removed.",
        gbCoopForceSpawnMpThings,
        false
    );

    cfg.coopPreserveKeys = makeConfigField(
        "PreserveKeys",
        "Cooperative: Players retain previously collected keys when respawning.",
        gbCoopPreserveKeys,
        false
    );

    cfg.coopPreserveAmmoFactor = makeConfigField(
        "PreserveAmmoFactor",
        "Cooperative: Players retain previously collected ammo and backpack when respawning.\n"
        "\n"
        "Allowed values:\n"
        " - None: Lose all ammo and backpack; bullets restored to 50 (0) (default setting)\n"
        " - All: Retain all ammo and keep backpack; bullets restored to at least 50 (1)\n"
        " - Half: Retain half ammo and keep backpack; bullets restored to at least 50 (2)",
        gCoopPreserveAmmoFactor,
        0
    );

    cfg.dmFragLimit = makeConfigField(
        "FragLimit",
        "Deathmatch: Sets the frag limit. Map will exit when limit is reached.\n"
        "\n"
        "0 = disabled",
        gDmFragLimit,
        0
    );

    cfg.dmExitDisabled = makeConfigField(
        "ExitDisabled",
        "Deathmatch: Prevents exiting the map.\n"
        "\n"
        "Note: if the frag limit is less than 1, exits will not be disabled.",
        gbDmExitDisabled,
        false
    );
}

END_NAMESPACE(ConfigSerialization)
