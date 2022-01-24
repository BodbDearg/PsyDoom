#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(Config)

//------------------------------------------------------------------------------------------------------------------------------------------
// Game settings
//------------------------------------------------------------------------------------------------------------------------------------------
const char*     getCueFilePath() noexcept;
extern bool     gbUncapFramerate;
extern bool     gbInterpolateSectors;
extern bool     gbInterpolateMobj;
extern bool     gbInterpolateMonsters;
extern bool     gbInterpolateWeapon;
extern int32_t  gMainMemoryHeapSize;
extern bool     gbSkipIntros;
extern bool     gbUseFastLoading;
extern bool     gbEnableSinglePlayerLevelTimer;
extern int32_t  gUsePalTimings;
extern bool     gbUseDemoTimings;
extern bool     gbUseMoveInputLatencyTweak;
extern bool     gbUseExtendedPlayerShootRange;
extern bool     gbUseItemPickupFix;
extern bool     gbUsePlayerRocketBlastFix;
extern bool     gbUseSuperShotgunDelayTweak;
extern int32_t  gUseFinalDoomPlayerMovement;
extern int32_t  gAllowMovementCancellation;
extern bool     gbAllowTurningCancellation;
extern bool     gbFixViewBobStrength;
extern bool     gbFixGravityStrength;
extern int32_t  gLostSoulSpawnLimit;
extern bool     gbUseLostSoulSpawnFix;
extern bool     gbUseLineOfSightOverflowFix;
extern bool     gbFixOutdoorBulletPuffs;
extern bool     gbFixBlockingGibsBug;
extern bool     gbFixSoundPropagation;
extern float    gViewBobbingStrength;

//------------------------------------------------------------------------------------------------------------------------------------------
// Video settings
//------------------------------------------------------------------------------------------------------------------------------------------
extern bool     gbFullscreen;
extern int32_t  gOutputResolutionW;
extern int32_t  gOutputResolutionH;
extern float    gLogicalDisplayW;
extern bool     gbDisableVulkanRenderer;
extern int32_t  gVulkanRenderHeight;
extern bool     gbVulkanPixelStretch;
extern bool     gbVulkanTripleBuffer;
extern bool     gbVulkanDrawExtendedStatusBar;
extern bool     gbVulkanWidescreenEnabled;
extern int32_t  gAAMultisamples;
extern int32_t  gTopOverscanPixels;
extern int32_t  gBottomOverscanPixels;
extern bool     gbFloorRenderGapFix;
extern bool     gbSkyLeakFix;
extern bool     gbVulkanBrightenAutomap;
extern bool     gbUseVulkan32BitShading;
extern int32_t  gVramSizeInMegabytes;
const char*     getVulkanPreferredDevicesRegex() noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Audio settings
//------------------------------------------------------------------------------------------------------------------------------------------
extern int32_t  gAudioBufferSize;
extern int32_t  gSpuRamSize;

//------------------------------------------------------------------------------------------------------------------------------------------
// Input settings
//------------------------------------------------------------------------------------------------------------------------------------------
extern float    gMouseTurnSpeed;
extern float    gGamepadDeadZone;
extern float    gGamepadFastTurnSpeed_High;
extern float    gGamepadFastTurnSpeed_Low;
extern float    gGamepadTurnSpeed_High;
extern float    gGamepadTurnSpeed_Low;
extern float    gAnalogToDigitalThreshold;

//------------------------------------------------------------------------------------------------------------------------------------------
// Cheat settings
//------------------------------------------------------------------------------------------------------------------------------------------

extern bool gbEnableDevCheatShortcuts;              // If 'true' then enable the convenience developer single cheat keys on the pause menu (keys F1-F8)
extern bool gbEnableDevInPlaceReloadFunctionKey;    // If 'true' then enable the development 'in-place map reload' function. This is activated with key F11.
extern bool gbEnableDevMapAutoReload;               // If 'true' then allow the game to automatically reload a map if it has changed on-disk

// Cheat key sequences for various cheats: an array of up to 16 SDL scan codes.
// Unused key slots in the sequence will be set to '0'.
struct CheatKeySequence {
    uint8_t keys[16];
    static constexpr uint32_t MAX_KEYS = (uint32_t) C_ARRAY_SIZE(keys);
};

extern CheatKeySequence gCheatKeys_GodMode;
extern CheatKeySequence gCheatKeys_NoClip;
extern CheatKeySequence gCheatKeys_LevelWarp;
extern CheatKeySequence gCheatKeys_WeaponsKeysAndArmor;
extern CheatKeySequence gCheatKeys_AllMapLinesOn;
extern CheatKeySequence gCheatKeys_AllMapThingsOn;
extern CheatKeySequence gCheatKeys_XRayVision;
extern CheatKeySequence gCheatKeys_VramViewer;
extern CheatKeySequence gCheatKeys_NoTarget;

void init() noexcept;
void shutdown() noexcept;

END_NAMESPACE(IniUtils)
