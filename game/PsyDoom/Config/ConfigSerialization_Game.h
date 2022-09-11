#pragma once

#include "ConfigSerialization.h"

namespace IniUtils {
    struct IniValue;
}

BEGIN_NAMESPACE(ConfigSerialization)

// N.B: must ONLY contain 'ConfigField' entries!
struct Config_Game {
    ConfigField     cueFilePath;
    ConfigField     uncapFramerate;
    ConfigField     showPerfCounters;
    ConfigField     interpolateSectors;
    ConfigField     interpolateMobj;
    ConfigField     interpolateMonsters;
    ConfigField     interpolateWeapon;
    ConfigField     mainMemoryHeapSize;
    ConfigField     skipIntros;
    ConfigField     useFastLoading;
    ConfigField     enableSinglePlayerLevelTimer;
    ConfigField     usePalTimings;
    ConfigField     useDemoTimings;
    ConfigField     fixKillCount;
    ConfigField     useMoveInputLatencyTweak;
    ConfigField     fixLineActivation;
    ConfigField     useExtendedPlayerShootRange;
    ConfigField     fixMultiLineSpecialCrossing;
    ConfigField     useItemPickupFix;
    ConfigField     usePlayerRocketBlastFix;
    ConfigField     useSuperShotgunDelayTweak;
    ConfigField     useFinalDoomPlayerMovement;
    ConfigField     allowMovementCancellation;
    ConfigField     allowTurningCancellation;
    ConfigField     fixViewBobStrength;
    ConfigField     fixGravityStrength;
    ConfigField     lostSoulSpawnLimit;
    ConfigField     useLostSoulSpawnFix;
    ConfigField     useLineOfSightOverflowFix;
    ConfigField     fixOutdoorBulletPuffs;
    ConfigField     fixBlockingGibsBug;
    ConfigField     fixSoundPropagation;
    ConfigField     fixSpriteVerticalWarp;
    ConfigField     enableMapPatches_GamePlay;
    ConfigField     enableMapPatches_Visual;
    ConfigField     enableMapPatches_PsyDoom;
    ConfigField     viewBobbingStrength;

    inline ConfigFieldList getFieldList() noexcept {
        static_assert(sizeof(*this) % sizeof(ConfigField) == 0);
        return { (ConfigField*) this, sizeof(*this) / sizeof(ConfigField) };
    };
};

extern Config_Game gConfig_Game;

void initCfgSerialization_Game() noexcept;

END_NAMESPACE(ConfigSerialization)
