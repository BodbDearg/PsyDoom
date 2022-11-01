#pragma once

#include "Macros.h"

#include <cstdint>
#include <string>

BEGIN_NAMESPACE(Config)

//------------------------------------------------------------------------------------------------------------------------------------------
// Game settings
//------------------------------------------------------------------------------------------------------------------------------------------
extern std::string      gCueFilePath;
extern bool             gbShowPerfCounters;
extern bool             gbInterpolateSectors;
extern bool             gbInterpolateMobj;
extern bool             gbInterpolateMonsters;
extern bool             gbInterpolateWeapon;
extern int32_t          gMainMemoryHeapSize;
extern bool             gbSkipIntros;
extern bool             gbUseFastLoading;
extern bool             gbEnableSinglePlayerLevelTimer;
extern int32_t          gUsePalTimings;
extern bool             gbUseDemoTimings;
extern bool             gbFixKillCount;
extern bool             gbUseMoveInputLatencyTweak;
extern bool             gbUseExtendedPlayerShootRange;
extern bool             gbFixLineActivation;
extern bool             gbFixMultiLineSpecialCrossing;
extern bool             gbUseItemPickupFix;
extern bool             gbUsePlayerRocketBlastFix;
extern bool             gbUseSuperShotgunDelayTweak;
extern int32_t          gUseFinalDoomPlayerMovement;
extern int32_t          gAllowMovementCancellation;
extern bool             gbAllowTurningCancellation;
extern bool             gbFixViewBobStrength;
extern bool             gbFixGravityStrength;
extern int32_t          gLostSoulSpawnLimit;
extern bool             gbUseLostSoulSpawnFix;
extern bool             gbUseLineOfSightOverflowFix;
extern bool             gbFixOutdoorBulletPuffs;
extern bool             gbFixBlockingGibsBug;
extern bool             gbFixSoundPropagation;
extern bool             gbFixSpriteVerticalWarp;
extern bool             gbAllowMultiMapPickup;
extern bool             gbEnableMapPatches_GamePlay;
extern bool             gbEnableMapPatches_Visual;
extern bool             gbEnableMapPatches_PsyDoom;
extern float            gViewBobbingStrength;

//------------------------------------------------------------------------------------------------------------------------------------------
// Video settings
//------------------------------------------------------------------------------------------------------------------------------------------
extern bool             gbFullscreen;
extern bool             gbEnableVSync;
extern int32_t          gOutputResolutionW;
extern int32_t          gOutputResolutionH;
extern float            gLogicalDisplayW;
extern bool             gbDisableVulkanRenderer;
extern int32_t          gVulkanRenderHeight;
extern bool             gbVulkanPixelStretch;
extern bool             gbVulkanTripleBuffer;
extern bool             gbVulkanDrawExtendedStatusBar;
extern bool             gbVulkanWidescreenEnabled;
extern int32_t          gAAMultisamples;
extern int32_t          gTopOverscanPixels;
extern int32_t          gBottomOverscanPixels;
extern bool             gbFloorRenderGapFix;
extern bool             gbSkyLeakFix;
extern bool             gbVulkanBrightenAutomap;
extern bool             gbUseVulkan32BitShading;
extern int32_t          gVramSizeInMegabytes;
extern std::string      gVulkanPreferredDevicesRegex;

//------------------------------------------------------------------------------------------------------------------------------------------
// Audio settings
//------------------------------------------------------------------------------------------------------------------------------------------
extern int32_t      gAudioBufferSize;
extern int32_t      gSpuRamSize;

//------------------------------------------------------------------------------------------------------------------------------------------
// Input settings
//------------------------------------------------------------------------------------------------------------------------------------------
extern float        gMouseTurnSpeed;
extern float        gGamepadDeadZone;
extern float        gGamepadFastTurnSpeed_High;
extern float        gGamepadFastTurnSpeed_Low;
extern float        gGamepadTurnSpeed_High;
extern float        gGamepadTurnSpeed_Low;
extern float        gAnalogToDigitalThreshold;

//------------------------------------------------------------------------------------------------------------------------------------------
// Multiplayer settings
//------------------------------------------------------------------------------------------------------------------------------------------
extern bool             gbCoopNoFriendlyFire;
extern bool             gbDmExitDisabled;
extern int32_t          gDmFragLimit;
extern int32_t          gCoopPreserveAmmoFactor;
extern bool             gbCoopPreserveKeys;
extern bool             gbCoopForceSpawnMpThings;
extern bool             gbDmActivateSpecialSectors;

//------------------------------------------------------------------------------------------------------------------------------------------
// Cheat settings
//------------------------------------------------------------------------------------------------------------------------------------------

// Cheat key sequences for various cheats: an array of up to 16 SDL scan codes.
// Unused key slots in the sequence will be set to '0'.
struct CheatKeySequence {
    uint8_t keys[16];
    static constexpr uint32_t MAX_KEYS = (uint32_t) C_ARRAY_SIZE(keys);
};

extern bool                 gbEnableDevCheatShortcuts;              // If 'true' then enable the convenience developer single cheat keys on the pause menu (keys F1-F8)
extern bool                 gbEnableDevInPlaceReloadFunctionKey;    // If 'true' then enable the development 'in-place map reload' function. This is activated with key F11.
extern bool                 gbEnableDevMapAutoReload;               // If 'true' then allow the game to automatically reload a map if it has changed on-disk
extern CheatKeySequence     gCheatKeys_GodMode;
extern CheatKeySequence     gCheatKeys_NoClip;
extern CheatKeySequence     gCheatKeys_LevelWarp;
extern CheatKeySequence     gCheatKeys_WeaponsKeysAndArmor;
extern CheatKeySequence     gCheatKeys_AllMapLinesOn;
extern CheatKeySequence     gCheatKeys_AllMapThingsOn;
extern CheatKeySequence     gCheatKeys_XRayVision;
extern CheatKeySequence     gCheatKeys_VramViewer;
extern CheatKeySequence     gCheatKeys_NoTarget;

//------------------------------------------------------------------------------------------------------------------------------------------
// Config dynamic defaults: these can change depending on the host environment
//------------------------------------------------------------------------------------------------------------------------------------------
extern int32_t      gDefaultVramSizeInMegabytes;
extern bool         gbCouldDetermineVulkanConfigDefaults;
extern int32_t      gDefaultAntiAliasingMultisamples;
extern int32_t      gDefaultVulkanRenderHeight;
extern bool         gbDefaultVulkanPixelStretch;

//------------------------------------------------------------------------------------------------------------------------------------------
// Which config files need re-saving
//------------------------------------------------------------------------------------------------------------------------------------------
extern bool     gbNeedSave_Audio;
extern bool     gbNeedSave_Cheats;
extern bool     gbNeedSave_Controls;
extern bool     gbNeedSave_Game;
extern bool     gbNeedSave_Graphics;
extern bool     gbNeedSave_Input;
extern bool     gbNeedSave_Multiplayer;

void init() noexcept;
void shutdown() noexcept;
bool didInit() noexcept;
void loadConfigFiles() noexcept;

void setCheatKeySequence(CheatKeySequence& sequence, const char* const pKeysStr) noexcept;
void getCheatKeySequence(const CheatKeySequence& sequence, std::string& strOut) noexcept;

END_NAMESPACE(Config)
