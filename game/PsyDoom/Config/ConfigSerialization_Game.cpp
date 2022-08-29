#include "ConfigSerialization_Game.h"

#include "Config.h"
#include "IniUtils.h"

#include <string>

BEGIN_NAMESPACE(ConfigSerialization)

using namespace Config;

// A default cue file path is NOT provided on MacOS because the user must input an absolute path.
// We can't detect .cue files placed in the same directory as PsyDoom on MacOS due to the 'translocation' security feature.
#if __APPLE__
    static constexpr const char* const DEFAULT_CUE_FILE_PATH = "";
#else
    static constexpr const char* const DEFAULT_CUE_FILE_PATH = "Doom.cue";
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Config field storage
//------------------------------------------------------------------------------------------------------------------------------------------
Config_Game gConfig_Game = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the config serializers for game related config
//------------------------------------------------------------------------------------------------------------------------------------------
void initCfgSerialization_Game() noexcept {
    auto& cfg = gConfig_Game;

    cfg.cueFilePath = makeConfigField(
        "CueFilePath",
        "Default path to the .cue file for PlayStation 'Doom' or 'Final Doom', or other valid game disc.\n"
        "This cue file path will be used when none is specified via the 'Launcher' tab or command line.\n"
        "See PsyDoom's main README.md file for a list of supported game discs.\n"
        "\n"
        "A valid .cue (cue sheet) file for the desired game must be provided in order to run PsyDoom.\n"
        "A relative or absolute path can be used; relative paths are relative to the current OS working\n"
        "directory, which is normally the directory that the PsyDoom executable is found in.\n"
        "\n"
        "Notes:\n"
        "(1) On MacOS the full path to this file MUST be specified because the working directory for\n"
        "PsyDoom may be randomized due to OS security restrictions. A default cue file location will\n"
        "also NOT be provided on MacOS for this same reason.\n"
        "(2) This setting can also be overriden with the '-cue <CUE_PATH>' command-line argument.",
        gCueFilePath,
        DEFAULT_CUE_FILE_PATH
    );

    cfg.uncapFramerate = makeConfigField(
        "UncapFramerate",
        "Uncapped framerate toggle.\n"
        "Enabling this setting allows PsyDoom to run beyond the original 30 FPS cap of PSX Doom.\n"
        "Frames in between the original 30 FPS keyframes will have movements and rotations interpolated.",
        gbUncapFramerate,
        true
    );

    cfg.showPerfCounters = makeConfigField(
        "ShowPerfCounters",
        "If enabled then show performance counters (FPS & frame duration) during gameplay and menus.",
        gbShowPerfCounters,
        false
    );

    cfg.interpolateSectors = makeConfigField(
        "InterpolateSectors",
        "When uncapped framerates are enabled, whether sector floor, ceiling and wall motion is smoothed.\n"
        "Also causes objects pushed by ceilings or floors to have that motion smoothed too, if enabled.",
        gbInterpolateSectors,
        true
    );

    cfg.interpolateMobj = makeConfigField(
        "InterpolateMobj",
        "When uncapped framerates are enabled, whether map objects have motion due to velocity/momentum\n"
        "smoothed. Only affects missiles and motion caused by forces like explosions.\n"
        "Does not affect walking/flying movements generated by monster AI.",
        gbInterpolateMobj,
        true
    );

    cfg.interpolateMonsters = makeConfigField(
        "InterpolateMonsters",
        "When uncapped framerates are enabled, whether walking/flying movements generated by monster AI are\n"
        "smoothed. This is disabled by default because it looks bad for various reasons.",
        gbInterpolateMonsters,
        false
    );

    cfg.interpolateWeapon = makeConfigField(
        "InterpolateWeapon",
        "When uncapped framerates are enabled, whether to interpolate the weapon sway of the player.",
        gbInterpolateWeapon,
        true
    );

    cfg.mainMemoryHeapSize = makeConfigField(
        "MainMemoryHeapSize",
        "How much system RAM is available to Doom's 'Zone Memory' heap allocator (in bytes).\n"
        "Many memory allocations in the game are serviced by this system so in effect this setting defines\n"
        "one of the main memory limits for the game.\n"
        "\n"
        "If the value of this setting is <= 0 then PsyDoom will reserve a default amount of memory - presently\n"
        "64 MiB. This should be enough for even the most demanding user maps.\n"
        "\n"
        "For reference, the original PSX Doom had about 1.3 MiB of heap space\n"
        "available, though it also used less RAM in general being a 32-bit program instead of 64-bit.\n"
        "WARNING: setting this value too low may result in the game crashing with an out of memory error!",
        gMainMemoryHeapSize,
        -1
    );

    cfg.skipIntros = makeConfigField(
        "SkipIntros",
        "If enabled then all intro logos and movies will be skipped on game startup.",
        gbSkipIntros,
        false
    );

    cfg.useFastLoading = makeConfigField(
        "UseFastLoading",
        "Skip crossfades and waiting for sounds to finish playing during loading transitions?\n"
        "PsyDoom doesn't really have loading times given the speed of modern hardware however it still\n"
        "implements logic from the original game which artificially causes a certain amount of 'loading'.\n"
        "This logic waits for certain sounds to finish before allowing loading to 'complete' and also\n"
        "performs screen crossfades of a fixed duration. If you enable this setting then PsyDoom will\n"
        "skip doing all that and load everything as fast as possible. This may be desirable for speed\n"
        "running but can make loading transitions more jarring and cause sounds to cut out abruptly.\n"
        "If you are playing normally it is probably more pleasant to leave this setting disabled.",
        gbUseFastLoading,
        false
    );

    cfg.enableSinglePlayerLevelTimer = makeConfigField(
        "EnableSinglePlayerLevelTimer",
        "Enable an optional end of level time display, in single player mode?\n"
        "When enabled shows the real time taken to complete a level, including any time spent in pause or\n"
        "option menus. This alters the display of the intermission screen slightly and condenses the\n"
        "stats shown in order to make room for time. This setting is not enabled by default because of font\n"
        "limitations making the time display look odd - the '.' separator must be used instead of ':'.\n"
        "The time is displayed in 3 components, minutes, seconds and hundredths of seconds and is limited\n"
        "to showing a maximum display of 999.59.99 - which is probably more than enough for any level.",
        gbEnableSinglePlayerLevelTimer,
        false
    );

    cfg.usePalTimings = makeConfigField(
        "UsePalTimings",
        "Whether or not to use movement & timing code from the PAL version of the game.\n"
        "This does not alter the refresh rate of the game, just how the game logic is processed & advanced.\n"
        "NTSC is the default since it offers the most responsive gameplay.\n"
        "\n"
        "The PAL version simulates the world and enemies slightly faster but the player moves at a slower\n"
        "rate, making the game more difficult. View bobbing is also much stronger in the PAL version unless\n"
        "the 'view bob fix' is applied to make it more consistent with NTSC (PsyDoom default behavior).\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "\n"
        "Allowed values:\n"
        "- Use NTSC timings (0)\n"
        "- Use PAL timings (1)\n"
        "- Auto-decide based on the game disc region (-1)",
        gUsePalTimings,
        0
    );

    cfg.useDemoTimings = makeConfigField(
        "UseDemoTimings",
        "Whether to restrict player update logic to a consistent tick-rate that advances at the same speed\n"
        "as enemies and the game world; this forced tick-rate will be '15 Hz' when using NTSC timings.\n"
        "Normally player logic updates at 30 Hz when using NTSC timings, framerate permitting.\n"
        "For PAL timings when this setting is enabled the forced player tick rate will be 12.5 Hz.\n"
        "\n"
        "If this mode is enabled then input lag will be increased, and player physics will feel more\n"
        "'floaty' due to a bug in the original game which causes weaker gravity under lower framerates.\n"
        "Generally this setting should be left disabled, unless you are really curious...\n"
        "Its main use is to ensure consistent demo recording & playback, where it will be force enabled.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbUseDemoTimings,
        false
    );

    cfg.fixKillCount = makeConfigField(
        "FixKillCount",
        "If enabled then fix the total enemies count for the level to include any monsters that have been\n"
        "spawned or respawned. Each time a monster gets spawned or respawned, the total is increased.\n"
        "The fix prevents the total kills from exceeding 100% when enemies like Pain Elementals appear in\n"
        "a map and allows the number of enemies remaining to be gauged more accurately.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbFixKillCount,
        true
    );

    cfg.useMoveInputLatencyTweak = makeConfigField(
        "UseMoveInputLatencyTweak",
        "Whether to use a tweak to the original player movement code which attempts to reduce input latency\n"
        "for sideways and forward movement. The effect of this will be subtle but should improve gameplay.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbUseMoveInputLatencyTweak,
        true
    );

    cfg.fixLineActivation = makeConfigField(
        "FixLineActivation",
        "If enabled then adjust line activation logic to fix some problem cases where valid attempts to use\n"
        "switches, doors and elevators would not register. Example problem cases are switches at 45 degree\n"
        "angles to other walls, or switches surrounded by lots of other geometry.\n"
        "\n"
        "Notes:\n"
        " (1) This fix will also remove many known exploits where switches can be used through walls!\n"
        " (2) This setting is ignored during demos and networked games where you are not the host/server.",
        gbFixLineActivation,
        true
    );

    cfg.useExtendedPlayerShootRange = makeConfigField(
        "UseExtendedPlayerShootRange",
        "If enabled, extends the following player attack limits:\n"
        " - Max shoot/hitscan distance, from '2048' to '8192'.\n"
        " - Max auto-aim distance, from '1024' to '8192'.\n"
        " - Max BFG spray/tracer distance, from '1024' to '2048'.\n"
        "\n"
        "Extending these limits can make combat on large open maps less frustrating.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbUseExtendedPlayerShootRange,
        true
    );

    cfg.fixMultiLineSpecialCrossing = makeConfigField(
        "FixMultiLineSpecialCrossing",
        "Fix a limitation where only 1 line special can be crossed/activated per frame?\n"
        "\n"
        "If enabled then the player can potentially trigger multiple line specials at the same time.\n"
        "Removing this limitation may be important for new maps with many line triggers in close proximity.\n"
        "\n"
        "Notes:\n"
        "(1) In each frame the same line special and tag is still only allowed to be triggered once.\n"
        "This restriction prevents for example the player from activating the same teleporter twice in\n"
        "one frame if crossing over 2 of it's edges (at a corner).\n"
        "(2) This setting is ignored during demos and networked games where you are not the host/server.",
        gbFixMultiLineSpecialCrossing,
        true
    );

    cfg.useItemPickupFix = makeConfigField(
        "UseItemPickupFix",
        "If enabled then fix a bug from the original game where sometimes the player is prevented\n"
        "from picking up items if they are close to other items that cannot be picked up.",
        gbUseItemPickupFix,
        true
    );

    cfg.usePlayerRocketBlastFix = makeConfigField(
        "UsePlayerRocketBlastFix",
        "Whether to apply a fix for a bug in the original games where sometimes the player would not take\n"
        "splash damage from rockets launched very close to walls.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbUsePlayerRocketBlastFix,
        true
    );

    cfg.useSuperShotgunDelayTweak = makeConfigField(
        "UseSuperShotgunDelayTweak",
        "Whether to apply a gameplay tweak to reduce the initial firing delay for the Super Shotgun.\n"
        "This tweak makes it more responsive and useful during fast action by reducing input latency and\n"
        "brings it more in line with the feel of the PC Super Shotgun. The tweak shifts some of the initial\n"
        "firing delay to later in the animation sequence, so that the overall firing time does not take any\n"
        "longer than it would normally.\n"
        "\n"
        "The tweak is disabled by default to preserve the original PSX feel but may be desirable for users\n"
        "who prefer this weapon to handle more like the PC version.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbUseSuperShotgunDelayTweak,
        false
    );

    cfg.useFinalDoomPlayerMovement = makeConfigField(
        "UseFinalDoomPlayerMovement",
        "Whether to use player movement & turning logic from Final Doom rather than the original PSX Doom.\n"
        "\n"
        "In the original PSX Doom, player forward move speed is slightly greater than side move speed.\n"
        "The way player logic is handled also produces slightly different results for the same inputs.\n"
        "In Final Doom, player forward move speed is changed to be the same as side move speed.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "\n"
        "Allowed values:\n"
        " - Always use the original PSX Doom player movement & turning logic (0)\n"
        " - Always use the PSX Final Doom player movement & turning logic (1)\n"
        " - Auto-decide based on the game being played (-1) (default setting)",
        gUseFinalDoomPlayerMovement,
        -1
    );

    cfg.allowMovementCancellation = makeConfigField(
        "AllowMovementCancellation",
        "For digital movement only: whether doing opposite movements (at the same time) such as forward and\n"
        "back causes them to cancel each other out.\n"
        "\n"
        "In Final Doom this was the case, but not so for the original PSX Doom which instead just picked one\n"
        "of the directions to move in. This setting does not affect analog movement from game controllers\n"
        "which can always cancel.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "\n"
        "Allowed values:\n"
        " - Opposite movements never cancel each other out (0)\n"
        " - Opposite movements always cancel each other out (1)\n"
        " - Auto-decide based on the game being played (-1)",
        gAllowMovementCancellation,
        1
    );

    cfg.allowTurningCancellation = makeConfigField(
        "AllowTurningCancellation",
        "For digital turning only: whether doing opposite left/right turns at the same time causes the\n"
        "actions to cancel each other out. Both Doom and Final Doom did NOT do any form of cancellation\n"
        "for conflicting digital turn movements. If you want the original behavior disable this setting.\n"
        "\n"
        "This setting does not affect any turning other than digital, all other turning can always cancel.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbAllowTurningCancellation,
        true
    );

    cfg.fixViewBobStrength = makeConfigField(
        "FixViewBobStrength",
        "Fix a bug from the original PSX Doom where view bobbing is not as strong when the game is running\n"
        "at 30 FPS versus 15 FPS? Enabling this setting will make view bobbing stronger, and much more\n"
        "like the original PC version. This fix also adjusts view bobbing when using PAL timings, so that\n"
        "it is not overly strong when walking.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbFixViewBobStrength,
        true
    );

    cfg.fixGravityStrength = makeConfigField(
        "FixGravityStrength",
        "Fix a bug from the original PSX Doom where gravity applies at inconsistent strengths depending on\n"
        "the current framerate. The fix makes the gravity strength behave consistently at all framerates.\n"
        "\n"
        "Originally gravity was 2x as strong when the game was running at 30 FPS versus when the game was\n"
        "running at 15 FPS. Since the performance of the game was often closer to 15 FPS in most cases,\n"
        "this fix makes gravity behave at its 15 FPS strength consistently for greater authenticity.\n"
        "\n"
        "IMPORTANT: disabling this fix may certain jumps like in 'The Mansion' and 'The Gauntlet'\n"
        "impossible to perform but will make gravity feel less floaty. Most maps can be played at the\n"
        "stronger gravity level however without issue.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbFixGravityStrength,
        true
    );

    cfg.lostSoulSpawnLimit = makeConfigField(
        "LostSoulSpawnLimit",
        "How many Lost Souls can be in a level before Pain Elementals stop spawning more?\n"
        "\n"
        "The original PSX Doom intended to have a limit of 24 (like the PC version) but due to a bug there\n"
        "was actually no cap on the number of Lost Souls that could be spawned. PSX Final Doom fixed this\n"
        "bug however and introduced a limit of 16 Lost Souls, which in turn caused problems on some maps.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "\n"
        "Allowed values:\n"
        "   0 = Auto decide the limit based on the game (Doom vs Final Doom), in a faithful manner\n"
        " >0 = Limit the number of Lost Souls to this much\n"
        " <0 = Apply no limit",
        gLostSoulSpawnLimit,
        0
    );

    cfg.useLostSoulSpawnFix = makeConfigField(
        "UseLostSoulSpawnFix",
        "If enabled then apply a fix to the original game logic to try and prevent Lost Souls from being\n"
        "occasionally spawned outside of level bounds.",
        gbUseLostSoulSpawnFix,
        true
    );

    cfg.useLineOfSightOverflowFix = makeConfigField(
        "UseLineOfSightOverflowFix",
        "If enabled then apply a fix to the original game logic to prevent numeric overflows from occurring\n"
        "in the enemy 'line of sight' code. These errors make monsters unable to see the player sometimes,\n"
        "usually when there are large differences between sector floor and ceiling heights.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbUseLineOfSightOverflowFix,
        true
    );

    cfg.fixOutdoorBulletPuffs = makeConfigField(
        "FixOutdoorBulletPuffs",
        "If enabled then fix a Doom engine bug where bullet puffs don't appear sometimes when shooting\n"
        "certain walls outdoors.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbFixOutdoorBulletPuffs,
        true
    );

    cfg.fixBlockingGibsBug = makeConfigField(
        "FixBlockingGibsBug",
        "If enabled then fix an original bug where sometimes monster gibs can block the player if an enemy\n"
        "is crushed while playing it's death animation sequence.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbFixBlockingGibsBug,
        true
    );

    cfg.fixSoundPropagation = makeConfigField(
        "FixSoundPropagation",
        "If enabled then fix an original PSX Doom bug where sound can travel through certain kinds of\n"
        "closed doors when it shouldn't be able to. This bug can be observed with the small window into the\n"
        "secret room, in MAP03 of Doom. It allows sound to pass through it even though it is closed.\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gbFixSoundPropagation,
        true
    );

    cfg.enableMapPatches_GamePlay = makeConfigField(
        "EnableMapPatches_GamePlay",
        "Whether to enable built-in fixes provided by PsyDoom for maps on various supported game discs.\n"
        "These patches will only be applied if the original map is unmodified.\n"
        "\n"
        "This setting enables map patches that affect gameplay in some way.\n"
        "Examples: fixing progression blockers, fixing broken secrets.\n"
        "\n"
        "Note: this setting is ignored during demos and networked games where you not the host.",
        gbEnableMapPatches_GamePlay,
        true
    );

    cfg.enableMapPatches_Visual = makeConfigField(
        "EnableMapPatches_Visual",
        "Whether to enable built-in fixes provided by PsyDoom for maps on various supported game discs.\n"
        "These patches will only be applied if the original map is unmodified.\n"
        "\n"
        "This setting enables map patches that affect just visual issues, which have zero gameplay effect.\n"
        "Examples: fixing missing textures, fixing texture alignment.",
        gbEnableMapPatches_Visual,
        true
    );

    cfg.enableMapPatches_PsyDoom = makeConfigField(
        "EnableMapPatches_PsyDoom",
        "Whether to enable built-in fixes provided by PsyDoom for maps on various supported game discs.\n"
        "These patches will only be applied if the original map is unmodified.\n"
        "\n"
        "This setting enables patching of issues unique to PsyDoom that do not occur in the original game.\n"
        "These issues are mostly caused by differences in how skies are handled versus standard PSX Doom.\n"
        "\n"
        "In order to fix many more problems, PsyDoom skies behave more like PC Doom skies in that they do\n"
        "not let you see past them into the 'void' and therefore (potentially) into other rooms past that.\n"
        "This works well most of the time but occasionally slight tweaks are needed to adjust for PsyDoom.\n"
        "These patches perform those tweaks.",
        gbEnableMapPatches_PsyDoom,
        true
    );

    cfg.viewBobbingStrength = makeConfigField(
        "ViewBobbingStrength",
        "Multiplier for view bobbing strength, from 0.0 to 1.0 (or above, to make the walk bob stronger).\n"
        "Note: this setting is ignored during demos and networked games where you are not the host/server.",
        gViewBobbingStrength,
        1.0f
    );
}

END_NAMESPACE(ConfigSerialization)
