#include "ConfigSerialization_Cheats.h"

#include "Config.h"
#include "IniUtils.h"

#include <string>
#include <string_view>

BEGIN_NAMESPACE(ConfigSerialization)

using namespace Config;

//------------------------------------------------------------------------------------------------------------------------------------------
// Config field storage
//------------------------------------------------------------------------------------------------------------------------------------------
Config_Cheats gConfig_Cheats = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Helpers that make a config field for a cheat key sequence
//------------------------------------------------------------------------------------------------------------------------------------------
static ConfigField makeCheatKeySequenceConfigField(
    const char* const name,
    const char* const comment,
    CheatKeySequence& globalCheatKeySeq,
    const char* const defaultSetting
) noexcept {
    return makeConfigField(
        name,
        comment,
        [&globalCheatKeySeq](const IniUtils::IniValue& value) noexcept {
            Config::setCheatKeySequence(globalCheatKeySeq, value.strValue.c_str());
        },
        [&globalCheatKeySeq](IniUtils::IniValue& valueOut) noexcept {
            Config::getCheatKeySequence(globalCheatKeySeq, valueOut.strValue);
        },
        [&globalCheatKeySeq, defaultSetting]() noexcept {
            Config::setCheatKeySequence(globalCheatKeySeq, defaultSetting);
        }
    );
}

static ConfigField makeCheatKeySequenceConfigField(
    const char* const name,
    CheatKeySequence& globalCheatKeySeq,
    const char* const defaultSetting
) noexcept {
    return makeCheatKeySequenceConfigField(name, "", globalCheatKeySeq, defaultSetting);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the config serializers for cheats related config
//------------------------------------------------------------------------------------------------------------------------------------------
void initCfgSerialization_Cheats() noexcept {
    auto& cfg = gConfig_Cheats;

    // Developer convenience stuff
    cfg.enableDevCheatShortcuts = makeConfigField(
        "EnableDevCheatShortcuts",
        "Enable to allow convenience single key cheats.\n"
        "\n"
        "If you frequently use cheats for development purposes then these shortcuts might be useful.\n"
        "They are disabled by default since they can be accidentally invoked very easily.\n"
        "\n"
        "The cheat keys when this setting is enabled are as follows:\n"
        "\n"
        " F1: God mode\n"
        " F2: No-clip (new cheat added by PsyDoom)\n"
        " F3: All weapons keys and ammo\n"
        " F4: Level warp (note: secret maps can now be warped to also)\n"
        "\n"
        " F6: X-ray vision\n"
        " F7: VRAM Viewer (functionality hidden in retail)\n"
        " F8: No-target (new cheat added by PsyDoom)",
        gbEnableDevCheatShortcuts,
        false
    );

    cfg.enableDevInPlaceReloadFunctionKey = makeConfigField(
        "EnableDevInPlaceReloadFunctionKey",
        "Enable to allow a developer feature where the 'F11' key will do an 'in-place' reload of the\n"
        "current map. An 'in-place' reload does a normal map reload but restores the player's position and\n"
        "orientation to what they were prior to the reload.\n"
        "\n"
        "Using this feature allows the map to be 'refreshed' quickly and edits to be viewed in-engine\n"
        "quickly, enabling faster map iteration.\n"
        "\n"
        "Note: this function is disallowed in multiplayer games.",
        gbEnableDevInPlaceReloadFunctionKey,
        false
    );

    cfg.enableDevMapAutoReload = makeConfigField(
        "EnableDevMapAutoReload",
        "Enable to activate a developer feature where the game will automatically do an 'in-place' reload\n"
        "of the current map if it has changed on-disk. Useful for instantly viewing map edits in-engine.\n"
        "\n"
        "This feature only works for files overridden via the file overrides mechanism, only in single\n"
        "player mode and only on Windows.",
        gbEnableDevMapAutoReload,
        false
    );

    // Cheat key sequences
    cfg.cheatKeys_godMode = makeCheatKeySequenceConfigField(
        "CheatKeySequence_GodMode",
        "Keyboard key sequences which can be input to activate various cheats in the game.\n"
        "By default these are mapped to mimick PC DOOM II cheats as much as possible.\n"
        "\n"
        "Limitations:\n"
        "  (1) Only the A-Z and 0-9 keys are acceptable in a key sequence, other characters are ignored.\n"
        "  (2) The maximum sequence length is 16 keys.\n"
        "\n"
        "If you need to input these via a game controller you can do so by inputting the original PSX\n"
        "buttons on the pause menu. You can see which inputs map to original PSX buttons (for cheat entry)\n"
        "in the controls configuration .ini file. For reference, the original PSX cheat sequences were:\n"
        "\n"
        "     God Mode: Down, L2, Square, R1, Right, L1, Left, Circle\n"
        "     Level Warp: Right, Left, R2, R1, Triangle, L1, Circle, X\n"
        "     Weapons, armor & keys: X, Triangle, L1, Up, Down, R2, Left, Left\n"
        "     Show all map lines: Triangle, Triangle, L2, R2, L2, R2, R1, Square\n"
        "     Show all map things: Triangle, Triangle, L2, R2, L2, R2, R1, Circle\n"
        "     X-Ray vision: L1, R2, L2, R1, Right, Triangle, X, Right\n"
        "\n"
        "The new cheats added to PsyDoom are assigned the following original PSX buttons:\n"
        "\n"
        "     No-clip: Up, Up, Up, Up, Up, Up, Up, R1\n"
        "     VRAM viewer: Triangle, Square, Up, Left, Down, Right, X, Circle\n"
        "     No-target: X, Up, X, Up, Square, Square, X, Square",
        gCheatKeys_GodMode,
        "iddqd"
    );

    cfg.cheatKeys_noClip = makeCheatKeySequenceConfigField(
        "CheatKeySequence_NoClip",
        gCheatKeys_NoClip,
        "idclip"
    );

    cfg.cheatKeys_levelWarp = makeCheatKeySequenceConfigField(
        "CheatKeySequence_LevelWarp",
        gCheatKeys_LevelWarp,
        "idclev"
    );

    cfg.cheatKeys_weaponsKeysAndArmor = makeCheatKeySequenceConfigField(
        "CheatKeySequence_WeaponsKeysAndArmor",
        gCheatKeys_WeaponsKeysAndArmor,
        "idkfa"
    );

    cfg.cheatKeys_weaponsAndArmor = makeCheatKeySequenceConfigField(
        "CheatKeySequence_WeaponsAndArmor",
        gCheatKeys_WeaponsAndArmor,
        "idfa"
    );

    cfg.cheatKeys_allMapLinesOn = makeCheatKeySequenceConfigField(
        "CheatKeySequence_AllMapLinesOn",
        gCheatKeys_AllMapLinesOn,
        "iddt"
    );

    cfg.cheatKeys_allMapThingsOn = makeCheatKeySequenceConfigField(
        "CheatKeySequence_AllMapThingsOn",
        gCheatKeys_AllMapThingsOn,
        "idmt"
    );

    cfg.cheatKeys_xrayVision = makeCheatKeySequenceConfigField(
        "CheatKeySequence_XRayVision",
        gCheatKeys_XRayVision,
        "idray"
    );

    cfg.cheatKeys_vramViewer = makeCheatKeySequenceConfigField(
        "CheatKeySequence_VramViewer",
        gCheatKeys_VramViewer,
        "idram"
    );

    cfg.cheatKeys_noTarget = makeCheatKeySequenceConfigField(
        "CheatKeySequence_NoTarget",
        gCheatKeys_NoTarget,
        "idcloak"
    );
}

END_NAMESPACE(ConfigSerialization)
