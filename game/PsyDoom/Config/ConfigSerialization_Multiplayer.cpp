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
        "CoopNoFriendlyFire",
        "Cooperative: disable taking damage from other players?\n"
        "\n"
        "Notes:\n"
        "(1) This setting does not affect barrel or telefrag damage.\n"
        "(2) This setting is ignored during demos and networked games where you are not the host/server.",
        gbCoopNoFriendlyFire,
        false
    );

    cfg.coopForceSpawnDeathmatchThings = makeConfigField(
        "CoopForceSpawnDeathmatchThings",
        "Cooperative: add things to the map which are flagged for deathmatch only?\n"
        "This includes additional monsters, weapons, and items.\n"
        "\n"
        "Notes:\n"
        "(1) The additional weapons and items are normally only found in deathmatch.\n"
        "(2) The additional monsters were possibly intended for another game mode that was\n"
        "never implemented or carried over from PC Doom and never removed.\n"
        "(2) This setting is ignored during demos and networked games where you are not the host/server.",
        gbCoopForceSpawnDeathmatchThings,
        false
    );

    cfg.coopPreserveKeys = makeConfigField(
        "CoopPreserveKeys",
        "Cooperative: preserve collected keys on respawn?\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbCoopPreserveKeys,
        false
    );

    cfg.coopPreserveAmmoFactor = makeConfigField(
        "CoopPreserveAmmoFactor",
        "Cooperative: how much ammo should be preserved when respawning.\n"
        "Also whether the backpack should be kept on respawn.\n"
        "\n"
        "Allowed values:\n"
        " - None: lose all ammo and the backpack. Bullets are restored to 50 (0) (default setting).\n"
        " - All:  retain all ammo and keep the backpack. Bullets are restored to at least 50 (1).\n"
        " - Half: retain half ammo and keep the backpack. Bullets are restored to at least 50 (2).\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gCoopPreserveAmmoFactor,
        0
    );

    cfg.dmFragLimit = makeConfigField(
        "DmFragLimit",
        "Deathmatch: specifies the frag limit. The map will exit when limit is reached.\n"
        "A value of '0' means there is no frag limit.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gDmFragLimit,
        0
    );

    cfg.dmExitDisabled = makeConfigField(
        "DmExitDisabled",
        "Deathmatch: prevent exiting the map?\n"
        "\n"
        "Notes:\n"
        "(1) If the frag limit is less than 1, exits will not be disabled.\n"
        "(2) This setting is ignored during demos and networked games where you are not the host/server.",
        gbDmExitDisabled,
        false
    );

    cfg.dmActivateBossSpecialSectors = makeConfigField(
        "DmActivateBossSpecialSectors",
        "Deathmatch: automatically activate boss-related special sectors upon entering a map?\n"
        "\n"
        "Notes:\n"
        "(1) This will lower any platforms and open any doors which normally activate when killing the\n"
        "final boss enemy in a map (e.g. the Barons of Hell in 'Phobos Anomaly').\n"
        "(2) This setting is ignored during demos and networked games where you are not the host/server.",
        gbDmActivateBossSpecialSectors,
        false
    );
}

END_NAMESPACE(ConfigSerialization)
