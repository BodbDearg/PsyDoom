#include "Config.h"

#include "Asserts.h"
#include "ConfigSerialization.h"
#include "PsyDoom/ProgArgs.h"

#if PSYDOOM_VULKAN_RENDERER
    #include "PhysicalDevice.h"
    #include "PhysicalDeviceSelection.h"
    #include "PsyDoom/VideoBackend_Vulkan.h"
    #include "VkFuncs.h"
    #include "VulkanInstance.h"
#endif

#include <cstring>
#include <functional>
#include <map>
#include <SDL.h>
#include <vector>

BEGIN_NAMESPACE(Config)

//------------------------------------------------------------------------------------------------------------------------------------------
// Game config settings
//------------------------------------------------------------------------------------------------------------------------------------------
std::string     gCueFilePath;
bool            gbShowPerfCounters;
bool            gbInterpolateSectors;
bool            gbInterpolateMobj;
bool            gbInterpolateMonsters;
bool            gbInterpolateWeapon;
int32_t         gMainMemoryHeapSize;
bool            gbSkipIntros;
bool            gbUseFastLoading;
bool            gbEnableSinglePlayerLevelTimer;
int32_t         gUsePalTimings;
bool            gbUseDemoTimings;
bool            gbFixKillCount;
bool            gbUseMoveInputLatencyTweak;
bool            gbFixLineActivation;
bool            gbUseExtendedPlayerShootRange;
bool            gbFixMultiLineSpecialCrossing;
bool            gbUseItemPickupFix;
bool            gbUsePlayerRocketBlastFix;
bool            gbUseSuperShotgunDelayTweak;
int32_t         gUseFinalDoomPlayerMovement;
int32_t         gAllowMovementCancellation;
bool            gbAllowTurningCancellation;
bool            gbFixViewBobStrength;
bool            gbFixGravityStrength;
int32_t         gLostSoulSpawnLimit;
bool            gbUseLostSoulSpawnFix;
bool            gbUseLineOfSightOverflowFix;
bool            gbFixOutdoorBulletPuffs;
bool            gbFixBlockingGibsBug;
bool            gbFixSoundPropagation;
bool            gbFixSpriteVerticalWarp;
bool            gbAllowMultiMapPickup;
bool            gbEnableMapPatches_GamePlay;
bool            gbEnableMapPatches_Visual;
bool            gbEnableMapPatches_PsyDoom;
float           gViewBobbingStrength;

//------------------------------------------------------------------------------------------------------------------------------------------
// Graphics config settings
//------------------------------------------------------------------------------------------------------------------------------------------
bool            gbFullscreen;
bool            gbEnableVSync;
int32_t         gOutputResolutionW;
int32_t         gOutputResolutionH;
float           gLogicalDisplayW;
bool            gbDisableVulkanRenderer;
int32_t         gVulkanRenderHeight;
bool            gbVulkanPixelStretch;
bool            gbVulkanTripleBuffer;
bool            gbVulkanDrawExtendedStatusBar;
bool            gbVulkanWidescreenEnabled;
int32_t         gAAMultisamples;
int32_t         gTopOverscanPixels;
int32_t         gBottomOverscanPixels;
bool            gbFloorRenderGapFix;
bool            gbSkyLeakFix;
bool            gbVulkanBrightenAutomap;
bool            gbUseVulkan32BitShading;
int32_t         gVramSizeInMegabytes;
std::string     gVulkanPreferredDevicesRegex;

//------------------------------------------------------------------------------------------------------------------------------------------
// Audio config settings
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t     gAudioBufferSize;
int32_t     gSpuRamSize;

//------------------------------------------------------------------------------------------------------------------------------------------
// Input config settings
//------------------------------------------------------------------------------------------------------------------------------------------
float       gMouseTurnSpeed;
float       gGamepadDeadZone;
float       gGamepadFastTurnSpeed_High;
float       gGamepadFastTurnSpeed_Low;
float       gGamepadTurnSpeed_High;
float       gGamepadTurnSpeed_Low;
float       gAnalogToDigitalThreshold;

//------------------------------------------------------------------------------------------------------------------------------------------
// Cheat settings
//------------------------------------------------------------------------------------------------------------------------------------------
bool gbEnableDevCheatShortcuts;
bool gbEnableDevInPlaceReloadFunctionKey;
bool gbEnableDevMapAutoReload;

CheatKeySequence gCheatKeys_GodMode;
CheatKeySequence gCheatKeys_NoClip;
CheatKeySequence gCheatKeys_LevelWarp;
CheatKeySequence gCheatKeys_WeaponsKeysAndArmor;
CheatKeySequence gCheatKeys_AllMapLinesOn;
CheatKeySequence gCheatKeys_AllMapThingsOn;
CheatKeySequence gCheatKeys_XRayVision;
CheatKeySequence gCheatKeys_VramViewer;
CheatKeySequence gCheatKeys_NoTarget;

//------------------------------------------------------------------------------------------------------------------------------------------
// Multiplayer settings
//------------------------------------------------------------------------------------------------------------------------------------------
bool        gbNoFriendlyFire;
bool        gbExitDisabled;
int32_t     gFragLimit;
int32_t     gPreserveAmmoFactor;
bool        gbPreserveKeys;
bool        gbMPThings;

//------------------------------------------------------------------------------------------------------------------------------------------
// Config dynamic defaults: these can change depending on the host environment
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t     gDefaultVramSizeInMegabytes             = -1;
bool        gbCouldDetermineVulkanConfigDefaults    = false;
int32_t     gDefaultAntiAliasingMultisamples        = 4;
int32_t     gDefaultVulkanRenderHeight              = -1;
bool        gbDefaultVulkanPixelStretch             = false;

//------------------------------------------------------------------------------------------------------------------------------------------
// Which config files need re-saving
//------------------------------------------------------------------------------------------------------------------------------------------
bool    gbNeedSave_Audio;
bool    gbNeedSave_Cheats;
bool    gbNeedSave_Controls;
bool    gbNeedSave_Game;
bool    gbNeedSave_Graphics;
bool    gbNeedSave_Input;
bool    gbNeedSave_Multiplayer;

//------------------------------------------------------------------------------------------------------------------------------------------
// Whether the config system was initialized
//------------------------------------------------------------------------------------------------------------------------------------------
bool gbDidInit;

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines dynamic defaults for Vulkan related config
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineVulkanDynamicConfigDefaults() noexcept {
    // Initialize SDL temporarily for this
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    // Determine if the main display is a high density one, like a 4K monitor
    bool bIsHighDensityDisplay = false;
    
    {
        float diagDpi = {};
        float horzDpi = {};
        float vertDpi = {};

        if (SDL_GetDisplayDPI(0, &diagDpi, &horzDpi, &vertDpi) == 0) {
            if (vertDpi >= 120.0f) {
                bIsHighDensityDisplay = true;
            }
        }
    }

    // Vulkan: determine if the 'best' device to use has more than 3 GiB of RAM.
    // If that is the case consider it a more 'powerful' GPU and thus able to use MSAA.
    // Also check to see if the device is a Raspberry Pi, special case that device since it is low powered.
    bool bIsVulkanLowMemDevice = true;
    bool bIsVulkanRpiDevice = false;

    gbCouldDetermineVulkanConfigDefaults = false;

    #if PSYDOOM_VULKAN_RENDERER
        Video::VideoBackend_Vulkan::withTempVkInstance([&](vgl::VulkanInstance& vkInstance) {
            const std::vector<vgl::PhysicalDevice>& gpus = vkInstance.getPhysicalDevices();
            const vgl::PhysicalDevice* const pGpu = vgl::PhysicalDeviceSelection::selectBestHeadlessDevice(gpus, nullptr);

            if (pGpu) {
                gbCouldDetermineVulkanConfigDefaults = true;
                bIsVulkanLowMemDevice = (pGpu->getDeviceMem() <= (uint64_t) 3u * 1024u * 1024u * 1024u);    // <= 3 GiB
                const char* const gpuName = pGpu->getProps().deviceName;
                bIsVulkanRpiDevice = (std::strstr(gpuName, "V3D") == gpuName);  // On Raspberry Pi the driver identifies the device starting with 'V3D'

                // The Raspberry Pi 4 only supports up to 4096x4096 textures, which restricts PsyDoom to a maximum VRAM size of 32 MiB.
                // Use this as the default VRAM amount on the RPI for now... Perhaps this limit can be revisted for future RPI models?
                if (bIsVulkanRpiDevice) {
                    gDefaultVramSizeInMegabytes = 32;
                }
            }
        });
    #endif  // #if PSYDOOM_VULKAN_RENDERER

    // Determine video defaults
    if (bIsVulkanRpiDevice) {
        // For the Raspberry Pi default to 480p rendering because it is so low powered.
        // Also use the same pixel stretch that the PSX used to avoid UI elements having rows and columns truncated or doubled.
        // Essentially these settings turn the game into a sort of 'Crispy' PSX Doom (double normal resolution).
        gDefaultAntiAliasingMultisamples = 1;
        gDefaultVulkanRenderHeight = 480;
        gbDefaultVulkanPixelStretch = true;
    } else {
        // For all other devices render at full resolution with no pixel stretch.
        // Turn off MSAA however by default if the device is considered to be low powered.
        const bool bUseMsaa = ((!bIsHighDensityDisplay) && (!bIsVulkanLowMemDevice));
        gDefaultAntiAliasingMultisamples = (bUseMsaa) ? 4 : 1;
        gDefaultVulkanRenderHeight = -1;
        gbDefaultVulkanPixelStretch = false;
    }

    // Cleanup
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines dynamic defaults for config values based on the host environment and hardware
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineDynamicConfigDefaults() noexcept {
    // Default to the maximum possible VRAM size (128 MiB) unless we are running the Vulkan renderer on a Raspberry Pi
    gDefaultVramSizeInMegabytes = -1;

    // Determine Vulkan defaults but skip if in headless mode - don't setup anything video related!
    if (!ProgArgs::gbHeadlessMode) {
        determineVulkanDynamicConfigDefaults();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read all config for the app, and write any config files that need to be updated with changed data
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Determine dynamic config defaults and init config serialization
    gbDidInit = true;
    determineDynamicConfigDefaults();
    ConfigSerialization::init();

    // Read all config files.
    // Also, if any files need saving after default initializing new fields then do that now.
    ConfigSerialization::readAllConfigFiles();
    ConfigSerialization::writeAllConfigFiles(false);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs shutdown and cleanup for the config system
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    ConfigSerialization::shutdown();
    gbDidInit = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the config system was already initialized
//------------------------------------------------------------------------------------------------------------------------------------------
bool didInit() noexcept {
    return gbDidInit;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Assigns a cheat key sequence a value from a string.
// Only alpha and numeric characters are allowed and the maximum sequence length is 16.
//------------------------------------------------------------------------------------------------------------------------------------------
void setCheatKeySequence(CheatKeySequence& sequence, const char* const pKeysStr) noexcept {
    ASSERT(pKeysStr);
    uint32_t keyIdx = 0;
    const char* pCurChar = pKeysStr;

    do {
        const char c = (char) std::toupper(pCurChar[0]);
        ++pCurChar;

        if (c == 0)
            break;

        if (c >= 'A' && c <= 'Z') {
            const uint8_t key = (uint8_t) SDL_SCANCODE_A + (uint8_t) c - 'A';
            sequence.keys[keyIdx] = key;
            ++keyIdx;
        }
        else if (c == '0') {
            sequence.keys[keyIdx] = SDL_SCANCODE_0;
            ++keyIdx;
        }
        else if (c >= '1' && c <= '9') {
            const uint8_t key = (uint8_t) SDL_SCANCODE_1 + (uint8_t) c - '1';
            sequence.keys[keyIdx] = key;
            ++keyIdx;
        }
    } while (keyIdx < CheatKeySequence::MAX_KEYS);

    // If we did not complete the sequence then null the rest of the chars
    while (keyIdx < CheatKeySequence::MAX_KEYS) {
        sequence.keys[keyIdx] = 0;
        ++keyIdx;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: returns the specified cheat key sequence as an ASCII string
//------------------------------------------------------------------------------------------------------------------------------------------
void getCheatKeySequence(const CheatKeySequence& sequence, std::string& strOut) noexcept {
    strOut.clear();

    for (uint8_t key : sequence.keys) {
        if ((key >= SDL_SCANCODE_A) && (key <= SDL_SCANCODE_Z)) {
            strOut.push_back((char)('a' + key - SDL_SCANCODE_A));
        }
        else if (key == SDL_SCANCODE_0) {
            strOut.push_back('0');
        }
        else if ((key >= SDL_SCANCODE_1) && (key <= SDL_SCANCODE_9)) {
            strOut.push_back((char)('1' + key - SDL_SCANCODE_1));
        }
        else {
            break;  // Unsupported character, or null terminator - end the sequence
        }
    }
}

END_NAMESPACE(Config)
