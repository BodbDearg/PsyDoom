#include "Config.h"

#include "Asserts.h"
#include "Controls.h"
#include "FileUtils.h"
#include "Finally.h"
#include "IniUtils.h"
#include "Utils.h"

#include <cstring>
#include <functional>
#include <map>
#include <SDL.h>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Config)

// MSVC: 't' file open option will cause '\n' to become '\r\n' (platform native EOL).
// This is useful because it allows us to use '\n' in the code and have it converted transparently.
#if _MSC_VER
    static constexpr const char* FOPEN_WRITE_TEXT = "wt";
#else
    static constexpr const char* FOPEN_WRITE_TEXT = "w";
#endif

// A modification warning written to the top of every config file
static constexpr const char* const FILE_MODIFICATION_WARNING = 
R"(####################################################################################################
# WARNING: this file is periodically re-organized by PsyDoom when new settings become available.
# Existing setting values will be preserved, but may be reordered and have their comments updated.
# If you have anything other than the value of a setting to save, do not put it in this file!
####################################################################################################
)";

// A warning used to show that the fields following are unused
static constexpr const char* const UNUSED_FIELDS_WARNING = 
R"(####################################################################################################
# UNUSED FIELDS! PsyDoom has detected the following setting fields which are not recognized or used.
# These may be leftovers from an older version of PsyDoom, or perhaps user-added data.
# These settings can be deleted safely without any ill effects because PsyDoom will not use them.
####################################################################################################)";

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines functionality and data pertaining to a particular config field
//------------------------------------------------------------------------------------------------------------------------------------------
struct ConfigFieldHandler {
    const char* name;               // The string key for the config field
    const char* preamble;           // Proceeds the actual key and value: normally used for commenting but can be null/empty if not needed.
    const char* defaultValueStr;    // String representation of the default value. Note: the key/value is always put on a different line to the pre/postamble.
    const char* postamble;          // Comes after the key and value. Can be used to add an additional newline but can be null/empty if not needed.

    // Logic for parsing the config value
    std::function<void (const IniUtils::Entry& iniEntry)> parseFunc;

    // Logic to set the config value to its default when it's not available in the .ini file
    std::function<void ()> setValueToDefaultFunc;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Graphics config settings
//------------------------------------------------------------------------------------------------------------------------------------------
bool            gbFullscreen;
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

const char* getVulkanPreferredDevicesRegex() noexcept { return gVulkanPreferredDevicesRegex.c_str(); }

static const ConfigFieldHandler GRAPHICS_CFG_INI_HANDLERS[] = {
    {
        "Fullscreen",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Fullscreen or windowed mode toggle.\n"
        "# Set to '1' cause the PsyDoom to launch in fullscreen mode, and '0' to use windowed mode.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbFullscreen = iniEntry.getBoolValue(true); },
        []() { gbFullscreen = true; }
    },
    {
        "OutputResolutionW",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Output resolution (width & height) for both the Vulkan and Classic renderers.\n"
        "# In windowed mode this is the size of the window.\n"
        "# In fullscreen mode this is the actual screen resolution to use.\n"
        "# If the values are '0' or less (auto resolution) then this means use the current (desktop)\n"
        "# resolution in fullscreen mode, and auto-decide the window size in windowed mode.\n"
        "#---------------------------------------------------------------------------------------------------",
        "-1", "",
        [](const IniUtils::Entry& iniEntry) { gOutputResolutionW = iniEntry.getIntValue(-1); },
        []() { gOutputResolutionW = -1; }
    },
    {
        "OutputResolutionH",
        "", "-1", "\n",
        [](const IniUtils::Entry& iniEntry) { gOutputResolutionH = iniEntry.getIntValue(-1); },
        []() { gOutputResolutionH = -1; }
    },
    {
        "AntiAliasingMultisamples",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Vulkan renderer only: the number of multisamples to use for anti-aliasing the view.\n"
        "# Increasing the number of samples can help smooth edges and prevent texture shimmer, but can be\n"
        "# costly to do at high resolutions or on weaker GPUs. 4x is probably reasonable for most GPUs and\n"
        "# screen resolution combinations, given the low requirements of Doom.\n"
        "# Note: if the hardware is unable to support the number of samples specified then the next available\n"
        "# sample count downwards will be selected.\n"
        "#---------------------------------------------------------------------------------------------------",
        "4", "\n",
        [](const IniUtils::Entry& iniEntry) { gAAMultisamples = iniEntry.getIntValue(4); },
        []() { gAAMultisamples = 4; }
    },
    {
        "VulkanRenderHeight",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Vulkan renderer: determines the vertical resolution (in pixels) of the render/draw framebuffer.\n"
        "# You can use this setting to render at a different resolution to the display resolution.\n"
        "#\n"
        "# Note: the horizontal render resolution is automatically determined using this setting, the\n"
        "# current display/window resolution, and the setting of 'VulkanPixelStretch' (see below).\n"
        "#\n"
        "# Example values:\n"
        "#  240 = Use the original PSX vertical resolution (240p: for best results use pixel stretch!)\n"
        "#  480 = Use 2x the original PSX vertical resolution (480p: for best results use pixel stretch!)\n"
        "#   -1 = Use the native vertical resolution of the display or window\n"
        "#---------------------------------------------------------------------------------------------------",
        "-1", "\n",
        [](const IniUtils::Entry& iniEntry) { gVulkanRenderHeight = iniEntry.getIntValue(-1); },
        []() { gVulkanRenderHeight = -1; }
    },
    {
        "VulkanPixelStretch",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Vulkan renderer: controls whether rendered pixels are stretched horizontally on output according\n"
        "# to the stretch factor determined via 'LogicalDisplayWidth' and also vertically due to overscan.\n"
        "# It makes the Vulkan renderer mimic the pixel stretching of the original PlayStation renderer\n"
        "# more closely and will cause the draw resolution to be reduced as a result.\n"
        "#\n"
        "# Pixel stretch is highly recommended if you want to match the rasterization of the original 240p\n"
        "# renderer or some other low multiple of that like 480p. Without stretching, at low resolutions you\n"
        "# may find that UI elements do not rasterize very well due to nearest neighbor filtering and the\n"
        "# draw resolution not being an integer multiple of the original resolution.\n"
        "#\n"
        "# If you are rendering at modern resolutions like 1080p or 1440p however it is recommended that you\n"
        "# leave this setting off ('0') so that the Vulkan renderer can operate at the highest resolutions\n"
        "# and not suffer aliasing from having to stretch the framebuffer on output.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbVulkanPixelStretch = iniEntry.getBoolValue(false); },
        []() { gbVulkanPixelStretch = false; }
    },
    {
        "VulkanTripleBuffer",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If the Vulkan video backend is active and the Vulkan API is in use, whether to use triple\n"
        "# buffering for output presentation. This setting affects both the classic renderer when it is\n"
        "# output via Vulkan and the new Vulkan renderer itself.\n"
        "#\n"
        "# If enabled ('1') the game will render frames as fast as possible and may possibly discard\n"
        "# previously rendered/queued frames while waiting for the display to become ready for output.\n"
        "#\n"
        "# Enabling ensures the most up-to-date view is shown when the time comes to display and helps to\n"
        "# reduce perceived input latency but will also greatly increase GPU and CPU usage.\n"
        "# Disable if you prefer to lower energy usage for slightly increased input latency.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbVulkanTripleBuffer = iniEntry.getBoolValue(false); },
        []() { gbVulkanTripleBuffer = false; }
    },
    {
        "VulkanDrawExtendedStatusBar",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Vulkan renderer only: draw extensions to the in-game status bar for widescreen mode?\n"
        "# PsyDoom can extend the original PSX status bar to 'support' widescreen mode by repeating the part\n"
        "# of the bar which contains all the weapon numbers. If you disable this setting, then a black\n"
        "# letterbox will be rendered instead. This setting is also ignored if Vulkan widescreen is disabled.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbVulkanDrawExtendedStatusBar = iniEntry.getBoolValue(true); },
        []() { gbVulkanDrawExtendedStatusBar = true; }
    },
    {
        "VulkanWidescreenEnabled",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Vulkan renderer only: allow extended widescreen rendering?\n"
        "# The in-game Vulkan renderer is capable of a wider field of view than the classic renderer on\n"
        "# modern widescreen displays. If desired however you can disable this ('0') for an aspect ratio more\n"
        "# like the original game with cropping at the sides of the screen.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbVulkanWidescreenEnabled = iniEntry.getBoolValue(true); },
        []() { gbVulkanWidescreenEnabled = true; }
    },
    {
        "UseVulkan32BitShading",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Vulkan renderer only: whether higher precision 32-bit shading and framebuffers should be used.\n"
        "# The original PSX framebuffer was only 16-bit, and by default the Vulkan renderer also uses this\n"
        "# color mode to replicate the original game's lighting and shading as closely as possible.\n"
        "# Setting this to '1' (enabled) will allow for smoother 32-bit shading with less color banding\n"
        "# artifacts, but will also increase the overall image brightness and reduce contrast - making the\n"
        "# display seem more 'washed out' and less atmospheric. It's recommended to leave this setting\n"
        "# disabled for better visuals, but if you dislike banding a lot then it can be enabled if needed.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseVulkan32BitShading = iniEntry.getBoolValue(false); },
        []() { gbUseVulkan32BitShading = false; }
    },
    {
        "DisableVulkanRenderer",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If set to '1' then the new Vulkan/hardware renderer will be completely disabled and the game will\n"
        "# behave as if Vulkan is not available for use to PsyDoom, even if the opposite is true.\n"
        "# If you only want to run PsyDoom using the classic PSX renderer then enabling this setting will\n"
        "# save on system & video RAM and other resources. By default if Vulkan rendering is possible then\n"
        "# the Vulkan renderer always needs to be active and use memory in order to allow for fast toggling\n"
        "# between renderers.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbDisableVulkanRenderer = iniEntry.getBoolValue(false); },
        []() { gbDisableVulkanRenderer = false; }
    },
    {
        "LogicalDisplayWidth",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Determines the game's horizontal stretch factor and affects both the Classic and Vulkan renderers.\n"
        "#\n"
        "# Given an original logical resolution of 256x240 pixels, this is the width to stretch that out to.\n"
        "# If set to '292' for example, then the stretch factor would be 292/256 or 1.14 approximately.\n"
        "#\n"
        "# Some background information:\n"
        "# PSX Doom (NTSC) originally rendered to a 256x240 framebuffer but stretched that to approximately\n"
        "# 292x240 (in square pixel terms, or 320x240 in CRT non-square pixels) on output; this made the game\n"
        "# feel much flatter and more compressed than the PC original. The PAL version also did the exact\n"
        "# same stretching and simply added additional letterboxing to fill the extra scanlines on the top\n"
        "# and bottom of the screen.\n"
        "#\n"
        "# Typical values:\n"
        "#  292 = Use (approximately) the original PSX Doom stretch factor for the most authentic look.\n"
        "#  256 = Don't stretch horizontally. Makes for a more PC-like view, but some art won't look right.\n"
        "#  -1  = Stretch to fill: the logical display width is automatically set such that the original PSX\n"
        "#        view area and UI elements will fill the display completely, regardless of how wide it is.\n"
        "#---------------------------------------------------------------------------------------------------",
        "292", "\n",
        [](const IniUtils::Entry& iniEntry) { gLogicalDisplayW = iniEntry.getFloatValue(292.0f); },
        []() { gLogicalDisplayW = 292.0f; }
    },
    {
        "TopOverscanPixels",
        "#---------------------------------------------------------------------------------------------------\n"
        "# With respect to the original resolution of 256x240, determines how many rows of pixels are\n"
        "# discarded at the top and bottom of the screen in order to emulate 'overscan' behavior of old CRT\n"
        "# televisions. Affects the display and aspect ratio for both the Vulkan and Classic renderers.\n"
        "#\n"
        "# Accounting for overscan can help yield an aspect ratio closer to how the game looked on CRT TVs of\n"
        "# it's time and can also be used to remove a region of dead space underneath the in-game HUD.\n"
        "# By default, PsyDoom will chop off 6 pixel rows at the bottom of the screen to remove some of the\n"
        "# HUD dead space without truncating the status bar or Doomguy's mugshot; the top of the screen will\n"
        "# also be left untouched by default in order to maximize the viewable area.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "",
        [](const IniUtils::Entry& iniEntry) { gTopOverscanPixels = iniEntry.getIntValue(0); },
        []() { gTopOverscanPixels = 0; }
    },
    {
        "BottomOverscanPixels",
        "", "6", "\n",
        [](const IniUtils::Entry& iniEntry) { gBottomOverscanPixels = iniEntry.getIntValue(6); },
        []() { gBottomOverscanPixels = 6; }
    },
    {
        "FloorRenderGapFix",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Classic renderer only: enable/disable a precision fix for the floor renderer to prevent gaps in\n"
        "# the floor on some maps. This fix helps prevent some noticeable glitches on larger outdoor maps\n"
        "# like 'Tower Of Babel'. Set to '1' to enable the fix, and '0' to disable (original PSX behavior).\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbFloorRenderGapFix = iniEntry.getBoolValue(true); },
        []() { gbFloorRenderGapFix = true; }
    },
    {
        "SkyLeakFix",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Classic renderer only: enable a limit removing feature that fixes ceilings and walls sometimes\n"
        "# poking through the sky if sectors beyond the sky have higher ceilings. This isn't really much of\n"
        "# an issue for the original retail maps but may frustrate the design of user maps if not fixed.\n"
        "#\n"
        "# Notes:\n"
        "#  (1) The Vulkan renderer has this fix always enabled.\n"
        "#  (2) Enabling this setting will reduce performance in Classic renderer mode due to extra sky\n"
        "#      wall columns that must be drawn to patch over 'leaks' from rooms beyond.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbSkyLeakFix = iniEntry.getBoolValue(true); },
        []() { gbSkyLeakFix = true; }
    },
    {
        "VulkanBrightenAutomap",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Vulkan renderer only: if '1' then automap lines will be brightened to compensate for them\n"
        "# appearing perceptually darker at higher resolutions, due to the lines being much thinner.\n"
        "# This tweak helps a high res Vulkan automap feel more like the original (low resolution) automap.\n"
        "# If you are running the Vulkan renderer at low resolution however, you may want to disable this.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbVulkanBrightenAutomap = iniEntry.getBoolValue(true); },
        []() { gbVulkanBrightenAutomap = true; }
    },
    {
        "VramSizeInMegabytes",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Specifies how many megabytes of video RAM are available to hold sprites and textures in the game.\n"
        "# The PlayStation originally had 1 MiB of VRAM, expanding this allows for much more complex user\n"
        "# maps to be supported and helps eliminate the 'Texture Cache Overflow' warning on busy maps with\n"
        "# huge amounts of enemies onscreen.\n"
        "#\n"
        "# Acceptable values are 1, 2, 4, 8, 16, 32, 64 and 128. Values in-between will be rounded up.\n"
        "# If <= 0 is specified then the default value will be used, which is presently 128 MiB.\n"
        "#---------------------------------------------------------------------------------------------------",
        "-1", "\n",
        [](const IniUtils::Entry& iniEntry) { gVramSizeInMegabytes = iniEntry.getIntValue(); },
        []() { gVramSizeInMegabytes = -1; }
    },
    {
        "VulkanPreferredDevicesRegex",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Vulkan renderer: a case insensitive regex that can specify which GPUs are preferable to use.\n"
        "# Useful in multi-GPU systems where you want to override PsyDoom's default 'best' device selection\n"
        "# and choose a particular GPU to render with. If the regex matches part of the device's name then it\n"
        "# is considered 'preferred' and selected over all other devices. If no preferred device can be\n"
        "# chosen, then PsyDoom will attempt to select what it thinks is the best non-preferred device as a\n"
        "# fallback.\n"
        "#\n"
        "# Example values:\n"
        "#  AMD\n"
        "#  NVIDIA.*RTX.*\n"
        "#  Intel\n"
        "#---------------------------------------------------------------------------------------------------",
        "", "\n",
        [](const IniUtils::Entry& iniEntry) { gVulkanPreferredDevicesRegex = iniEntry.value; },
        []() { gVulkanPreferredDevicesRegex = ""; }
    },
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Audio config settings
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t gAudioBufferSize;
int32_t gSpuRamSize;

static const ConfigFieldHandler AUDIO_CFG_INI_HANDLERS[] = {
    {
        "AudioBufferSize",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Audio buffer size, in 44.1 KHz sound samples.\n"
        "# Lower values reduce sound latency and improve music timing precision.\n"
        "# Setting the buffer size too low however may cause audio instability or stutter on some systems.\n"
        "# If set to '0' (auto) then PsyDoom will use a default value, which is '128' samples currently.\n"
        "# Mostly this setting can be left alone but if you are experiencing sound issues, try adjusting.\n"
        "#\n"
        "# Some example values and their corresponding added sound latency (MS):\n"
        "#  64   = ~1.45 MS\n"
        "#  128  = ~2.9 MS\n"
        "#  256  = ~5.8 MS\n"
        "#  512  = ~11.6 MS\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gAudioBufferSize = iniEntry.getIntValue(0); },
        []() { gAudioBufferSize = 0; }
    },
    {
        "SpuRamSize",
        "#---------------------------------------------------------------------------------------------------\n"
        "# The size of available SPU RAM for loading sounds and sampled music instruments, in bytes.\n"
        "# If <= 0 then PsyDoom will auto-configure the amount to 16 MiB, which is 32x the original 512 KiB\n"
        "# that the PlayStation allowed. This greatly extended limit fixes a few bugs with sounds not playing\n"
        "# on some maps and provides ample room for custom user maps and music. Note that values lower than\n"
        "# 512 KiB will be ignored for compatibility reasons. This setting also has no effect if you build\n"
        "# PsyDoom without limit removing features.\n"
        "#---------------------------------------------------------------------------------------------------",
        "-1", "\n",
        [](const IniUtils::Entry& iniEntry) { gSpuRamSize = iniEntry.getIntValue(-1); },
        []() { gSpuRamSize = -1; }
    },
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Game config settings
//------------------------------------------------------------------------------------------------------------------------------------------
#if __APPLE__
    static constexpr const char* const DEFAULT_CUE_FILE_PATH = "";  // Not provided on MacOS because the user must input an absolute path
#else
    static constexpr const char* const DEFAULT_CUE_FILE_PATH = "Doom.cue";
#endif

static std::string      gCueFilePath;
bool                    gbUncapFramerate;
bool                    gbInterpolateSectors;
bool                    gbInterpolateMobj;
bool                    gbInterpolateMonsters;
bool                    gbInterpolateWeapon;
int32_t                 gMainMemoryHeapSize;
bool                    gbSkipIntros;
bool                    gbUseFastLoading;
bool                    gbEnableSinglePlayerLevelTimer;
int32_t                 gUsePalTimings;
bool                    gbUseDemoTimings;
bool                    gbUseMoveInputLatencyTweak;
bool                    gbUseExtendedPlayerShootRange;
bool                    gbUseItemPickupFix;
bool                    gbUsePlayerRocketBlastFix;
bool                    gbUseSuperShotgunDelayTweak;
int32_t                 gUseFinalDoomPlayerMovement;
int32_t                 gAllowMovementCancellation;
bool                    gbAllowTurningCancellation;
bool                    gbFixViewBobStrength;
bool                    gbFixGravityStrength;
int32_t                 gLostSoulSpawnLimit;
bool                    gbUseLostSoulSpawnFix;
bool                    gbUseLineOfSightOverflowFix;
bool                    gbFixOutdoorBulletPuffs;
bool                    gbFixBlockingGibsBug;
bool                    gbFixSoundPropagation;
float                   gViewBobbingStrength;

const char* getCueFilePath() noexcept { return gCueFilePath.c_str(); }

static const ConfigFieldHandler GAME_CFG_INI_HANDLERS[] = {
    {
        "CueFilePath",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Path to the .cue file for the PlayStation 'Doom' or 'Final Doom' disc, or other supported mod.\n"
        "# A valid .cue (cue sheet) file for the desired game must be provided in order to run PsyDoom.\n"
        "# A relative or absolute path can be used; relative paths are relative to the current OS working\n"
        "# directory, which is normally the directory that the PsyDoom executable is found in.\n"
        "#\n"
        "# Notes:\n"
        "#  (1) On MacOS the full path to this file MUST be specified because the working directory for\n"
        "#      PsyDoom may be randomized due to OS security restrictions. A default cue file location\n"
        "#      will also NOT be provided on MacOS for this same reason.\n"
        "#  (2) This setting can also be overriden with the '-cue <CUE_PATH>' command-line argument.\n"
        "#---------------------------------------------------------------------------------------------------",
        DEFAULT_CUE_FILE_PATH, "\n",
        [](const IniUtils::Entry& iniEntry) { gCueFilePath = iniEntry.value; },
        []() { gCueFilePath = DEFAULT_CUE_FILE_PATH; }
    },
    {
        "UncapFramerate",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Uncapped framerate toggle.\n"
        "# Setting to '1' allows PsyDoom to run beyond the original 30 FPS cap of PSX Doom.\n"
        "# Frames in between the original 30 FPS keyframes will have movements and rotations interpolated.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUncapFramerate = iniEntry.getBoolValue(true); },
        []() { gbUncapFramerate = true; }
    },
    {
        "InterpolateSectors",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Interpolation settings for when uncapped framerates are enabled.\n"
        "# Set these to '1' to enable smoothing of motion for the listed elements, '0' to disable.\n"
        "#\n"
        "# Interpolation switches:\n"
        "#\n"
        "# Sectors:     Whether sector floor, ceiling and wall motion is smoothed. Also causes objects pushed\n"
        "#              by ceilings or floors to have that motion smoothed too, if enabled.\n"
        "# Mobj:        Whether map objects have motion due to velocity/momentum smoothed.\n"
        "#              Only affects missiles and motion caused by forces like explosions.\n"
        "#              Does not affect walking/flying movements generated by monster AI.\n"
        "# Monsters:    Whether walking/flying movements generated by monster AI are smoothed.\n"
        "#              This is disabled by default because it looks bad for various reasons.\n"
        "# Weapon:      Whether to interpolate the weapon sway of the player.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "",
        [](const IniUtils::Entry& iniEntry) { gbInterpolateSectors = iniEntry.getBoolValue(true); },
        []() { gbInterpolateSectors = true; }
    },
    {
        "InterpolateMobj",
        "", "1", "",
        [](const IniUtils::Entry& iniEntry) { gbInterpolateMobj = iniEntry.getBoolValue(true); },
        []() { gbInterpolateMobj = true; }
    },
    {
        "InterpolateMonsters",
        "", "0", "",
        [](const IniUtils::Entry& iniEntry) { gbInterpolateMonsters = iniEntry.getBoolValue(false); },
        []() { gbInterpolateMonsters = false; }
    },
    {
        "InterpolateWeapon",
        "", "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbInterpolateWeapon = iniEntry.getBoolValue(true); },
        []() { gbInterpolateWeapon = true; }
    },
    {
        "MainMemoryHeapSize",
        "#---------------------------------------------------------------------------------------------------\n"
        "# How much system RAM is available to Doom's 'Zone Memory' heap allocator.\n"
        "# Many memory allocations in the game are serviced by this system so in effect this setting defines\n"
        "# one of the main memory limits for the game. If the value of this setting is <= 0 then PsyDoom will\n"
        "# reserve a default amount of memory - presently 64 MiB. This should be enough for even the most\n"
        "# demanding user maps. For reference, the original PSX Doom had about 1.3 MiB of heap space\n"
        "# available, though it also used less RAM in general being a 32-bit program instead of 64-bit.\n"
        "# WARNING: setting this value too low may result in the game crashing with an out of memory error!\n"
        "#---------------------------------------------------------------------------------------------------",
        "-1", "\n",
        [](const IniUtils::Entry& iniEntry) { gMainMemoryHeapSize = iniEntry.getIntValue(-1); },
        []() { gMainMemoryHeapSize = -1; }
    },
    {
        "SkipIntros",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If enabled ('1') then all intro logos and movies will be skipped on game startup.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbSkipIntros = iniEntry.getBoolValue(false); },
        []() { gbSkipIntros = false; }
    },
    {
        "UseFastLoading",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Skip crossfades and waiting for sounds to finish playing during loading transitions?\n"
        "# PsyDoom doesn't really have loading times given the speed of modern hardware however it still\n"
        "# implements logic from the original game which artificially causes a certain amount of 'loading'.\n"
        "# This logic waits for certain sounds to finish before allowing loading to 'complete' and also\n"
        "# performs screen crossfades of a fixed duration. If you enable this setting ('1') then PsyDoom will\n"
        "# skip doing all that and load everything as fast as possible. This may be desirable for speed\n"
        "# running but can make loading transitions more jarring and cause sounds to cut out abruptly.\n"
        "# If you are playing normally it is probably more pleasant to leave this setting disabled.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseFastLoading = iniEntry.getBoolValue(false); },
        []() { gbUseFastLoading = false; }
    },
    {
        "EnableSinglePlayerLevelTimer",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Enable an optional end of level time display, in single player mode?\n"
        "# Setting to '1' shows the real time taken to complete a level, including any time spent in pause or\n"
        "# option menus. This alters the display of the intermission screen slightly and condenses the\n"
        "# stats shown in order to make room for time. This setting is not enabled by default because of font\n"
        "# limitations making the time display look odd - the '.' separator must be used instead of ':'.\n"
        "# The time is displayed in 3 components, minutes, seconds and hundredths of seconds and is limited\n"
        "# to showing a maximum display of 999.59.99 - which is probably more than enough for any level.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbEnableSinglePlayerLevelTimer = iniEntry.getBoolValue(false); },
        []() { gbEnableSinglePlayerLevelTimer = false; }
    },
    {
        "UsePalTimings",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Whether or not to use movement & timing code from the PAL version of the game.\n"
        "# This does not alter the refresh rate of the game, just how the game logic is processed & advanced.\n"
        "# The PAL version simulates the world and enemies slightly faster but the player moves at a slower\n"
        "# rate, making the game more difficult. View bobbing is also much stronger in the PAL version unless\n"
        "# the 'view bob fix' is applied to make it more consistent with NTSC (PsyDoom default behavior).\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#\n"
        "# Allowed values:\n"
        "#   0 = Use NTSC timings\n"
        "#   1 = Use PAL timings\n"
        "#  -1 = Auto-decide based on the game disc region\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gUsePalTimings = iniEntry.getIntValue(0); },
        []() { gUsePalTimings = 0; }
    },
    {
        "UseDemoTimings",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Whether to restrict player update logic to a consistent tick-rate that advances at the same speed\n"
        "# as enemies and the game world; this forced tick-rate will be '15 Hz' when using NTSC timings.\n"
        "# Normally player logic updates at 30 Hz when using NTSC timings, framerate permitting.\n"
        "#\n"
        "# If this mode is enabled ('1') then input lag will be increased, and player physics will feel more\n"
        "# 'floaty' due to a bug in the original game which causes weaker gravity under lower framerates.\n"
        "# Generally this setting should be left disabled, unless you are really curious...\n"
        "# Its main use is to ensure consistent demo recording & playback, where it will be force enabled.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseDemoTimings = iniEntry.getBoolValue(false); },
        []() { gbUseDemoTimings = false; }
    },
    {
        "UseMoveInputLatencyTweak",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Whether to use a tweak to the original player movement code which attempts to reduce input latency\n"
        "# for sideways and forward movement. The effect of this will be subtle but should improve gameplay.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseMoveInputLatencyTweak = iniEntry.getBoolValue(true); },
        []() { gbUseMoveInputLatencyTweak = true; }
    },
    {
        "UseExtendedPlayerShootRange",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If enabled, extends the following player attack limits:\n"
        "#  - Max shoot/hitscan distance, from '2048' to '8192'.\n"
        "#  - Max auto-aim distance, from '1024' to '8192'.\n"
        "#  - Max BFG spray/tracer distance, from '1024' to '2048'.\n"
        "#\n"
        "# Extending these limits can make combat on large open maps less frustrating.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseExtendedPlayerShootRange = iniEntry.getBoolValue(true); },
        []() { gbUseExtendedPlayerShootRange = true; }
    },
    {
        "UseItemPickupFix",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If enabled ('1') then fix a bug from the original game where sometimes the player is prevented\n"
        "# from picking up items if they are close to other items that cannot be picked up.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseItemPickupFix = iniEntry.getBoolValue(true); },
        []() { gbUseItemPickupFix = true; }
    },
    {
        "UsePlayerRocketBlastFix",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Whether to apply a fix for a bug in the original games where sometimes the player would not take\n"
        "# splash damage from rockets launched very close to walls.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUsePlayerRocketBlastFix = iniEntry.getBoolValue(true); },
        []() { gbUsePlayerRocketBlastFix = true; }
    },
    {
        "UseSuperShotgunDelayTweak",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Whether to apply a gameplay tweak to reduce the initial firing delay for the Super Shotgun.\n"
        "# This tweak makes it more responsive and useful during fast action by reducing input latency and\n"
        "# brings it more in line with the feel of the PC Super Shotgun. The tweak shifts some of the initial\n"
        "# firing delay to later in the animation sequence, so that the overall firing time does not take any\n"
        "# longer than it would normally. The tweak is disabled by default to preserve the original PSX feel\n"
        "# but may be desirable for users who prefer this weapon to handle more like the PC version.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseSuperShotgunDelayTweak = iniEntry.getBoolValue(false); },
        []() { gbUseSuperShotgunDelayTweak = false; }
    },
    {
        "UseFinalDoomPlayerMovement",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Whether to use player movement & turning logic from Final Doom rather than the original PSX Doom.\n"
        "# In the original PSX Doom, player forward move speed is slightly greater than side move speed.\n"
        "# The way player logic is handled also produces slightly different results for the same inputs.\n"
        "# In Final Doom, player forward move speed is changed to be the same as side move speed.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#\n"
        "# Allowed values:\n"
        "#   0 = Always use the original PSX Doom player movement & turning logic\n"
        "#   1 = Always use the PSX Final Doom player movement & turning logic\n"
        "#  -1 = Auto-decide based on the game being played\n"
        "#---------------------------------------------------------------------------------------------------",
        "-1", "\n",
        [](const IniUtils::Entry& iniEntry) { gUseFinalDoomPlayerMovement = iniEntry.getIntValue(-1); },
        []() { gUseFinalDoomPlayerMovement = -1; }
    },
    {
        "AllowMovementCancellation",
        "#---------------------------------------------------------------------------------------------------\n"
        "# For digital movement only: whether doing opposite movements (at the same time) such as forward and\n"
        "# back causes them to cancel each other out. In Final Doom this was the case, but not so for the\n"
        "# original PSX Doom which instead just picked one of the directions to move in.\n"
        "# This setting does not affect analog movement from game controllers which can always cancel.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#\n"
        "# Allowed values:\n"
        "#   0 = Opposite movements never cancel each other out\n"
        "#   1 = Opposite movements always cancel each other out\n"
        "#  -1 = Auto-decide based on the game being played\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gAllowMovementCancellation = iniEntry.getIntValue(1); },
        []() { gAllowMovementCancellation = 1; }
    },
    {
        "AllowTurningCancellation",
        "#---------------------------------------------------------------------------------------------------\n"
        "# For digital turning only: whether doing opposite left/right turns at the same time causes the\n"
        "# actions to cancel each other out. Both Doom and Final Doom did NOT do any form of cancellation\n"
        "# for conflicting digital turn movements, therefore if you want the original behavior set to '0'.\n"
        "# This setting does not affect any turning other than digital, all other turning can always cancel.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbAllowTurningCancellation = iniEntry.getBoolValue(true); },
        []() { gbAllowTurningCancellation = true; }
    },
    {
        "FixViewBobStrength",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Fix a bug from the original PSX Doom where view bobbing is not as strong when the game is running\n"
        "# at 30 FPS versus 15 FPS? Enabling this setting will make view bobbing stronger, and much more\n"
        "# like the original PC version. This fix also adjusts view bobbing when using PAL timings, so that\n"
        "# it is not overly strong when walking.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbFixViewBobStrength = iniEntry.getBoolValue(true); },
        []() { gbFixViewBobStrength = true; }
    },
    {
        "FixGravityStrength",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Fix a bug from the original PSX Doom where gravity applies at inconsistent strengths depending on\n"
        "# the current framerate. The fix makes the gravity strength behave consistently at all framerates.\n"
        "#\n"
        "# Originally gravity was 2x as strong when the game was running at 30 FPS versus when the game was\n"
        "# running at 15 FPS. Since the performance of the game was often closer to 15 FPS in most cases,\n"
        "# this fix makes gravity behave at its 15 FPS strength consistently for greater authenticity.\n"
        "#\n"
        "# IMPORTANT: disabling this fix may certain jumps like in 'The Mansion' and 'The Gauntlet'\n"
        "# impossible to perform but will make gravity feel less floaty. Most maps can be played at the\n"
        "# stronger gravity level however without issue.\n"
        "#\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbFixGravityStrength = iniEntry.getBoolValue(true); },
        []() { gbFixGravityStrength = true; }
    },
    {
        "LostSoulSpawnLimit",
        "#---------------------------------------------------------------------------------------------------\n"
        "# How many Lost Souls can be in a level before Pain Elementals stop spawning more?\n"
        "# The original PSX Doom intended to have a limit of 24 (like the PC version) but due to a bug there\n"
        "# was actually no cap on the number of Lost Souls that could be spawned. PSX Final Doom fixed this\n"
        "# bug however and introduced a limit of 16 Lost Souls, which in turn caused problems on some maps.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#\n"
        "# Allowed values:\n"
        "#    0 = Auto decide the limit based on the game (Doom vs Final Doom), in a faithful manner\n"
        "#   >0 = Limit the number of Lost Souls to this much\n"
        "#   <0 = Apply no limit\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gLostSoulSpawnLimit = iniEntry.getIntValue(0); },
        []() { gLostSoulSpawnLimit = 0; }
    },
    {
        "UseLostSoulSpawnFix",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If enabled ('1') then apply a fix to the original game logic to try and prevent Lost Souls from\n"
        "# being occasionally spawned outside of level bounds.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseLostSoulSpawnFix = iniEntry.getBoolValue(true); },
        []() { gbUseLostSoulSpawnFix = true; }
    },
    {
        "UseLineOfSightOverflowFix",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If enabled ('1') then apply a fix to the original game logic to prevent numeric overflows from\n"
        "# occurring in the enemy 'line of sight' code. These errors make monsters unable to see the player\n"
        "# sometimes, usually when there are large differences between sector floor and ceiling heights.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbUseLineOfSightOverflowFix = iniEntry.getBoolValue(true); },
        []() { gbUseLineOfSightOverflowFix = true; }
    },
    {
        "FixOutdoorBulletPuffs",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If enabled ('1') then fix a Doom engine bug where bullet puffs don't appear sometimes when\n"
        "# shooting certain walls outdoors.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbFixOutdoorBulletPuffs = iniEntry.getBoolValue(true); },
        []() { gbFixOutdoorBulletPuffs = true; }
    },
    {
        "FixBlockingGibsBug",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If enabled ('1') then fix an original bug where sometimes monster gibs can block the player if an\n"
        "# enemy is crushed whilst playing it's death animation sequence.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbFixBlockingGibsBug = iniEntry.getBoolValue(true); },
        []() { gbFixBlockingGibsBug = true; }
    },
    {
        "FixSoundPropagation",
        "#---------------------------------------------------------------------------------------------------\n"
        "# If enabled ('1') then fix an original PSX Doom bug where sound can travel through certain kinds of\n"
        "# closed doors when it shouldn't be able to. This bug can be observed with the small window into the\n"
        "# secret room, in MAP03 of Doom. It allows sound to pass through it even though it is closed.\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1", "\n",
        [](const IniUtils::Entry& iniEntry) { gbFixSoundPropagation = iniEntry.getBoolValue(true); },
        []() { gbFixSoundPropagation = true; }
    },
    {
        "ViewBobbingStrength",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Multiplier for view bobbing strength, from 0.0 to 1.0 (or above, to make the walk bob stronger).\n"
        "# Note: this setting is ignored during demos and networked games where you are not the host/server.\n"
        "#---------------------------------------------------------------------------------------------------",
        "1.0", "\n",
        [](const IniUtils::Entry& iniEntry) { gViewBobbingStrength = iniEntry.getFloatValue(1.0f); },
        []() { gViewBobbingStrength = 1.0f; }
    },
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Input config settings
//------------------------------------------------------------------------------------------------------------------------------------------
float   gMouseTurnSpeed;
float   gGamepadDeadZone;
float   gGamepadFastTurnSpeed_High;
float   gGamepadFastTurnSpeed_Low;
float   gGamepadTurnSpeed_High;
float   gGamepadTurnSpeed_Low;
float   gAnalogToDigitalThreshold;

static const ConfigFieldHandler INPUT_CFG_INI_HANDLERS[] = {
    {
        "MouseTurnSpeed",
        "#---------------------------------------------------------------------------------------------------\n"
        "# How much turning movement to apply per pixel of mouse movement\n"
        "#---------------------------------------------------------------------------------------------------",
        "12.0", "\n",
        [](const IniUtils::Entry& iniEntry) { gMouseTurnSpeed = iniEntry.getFloatValue(12.0f); },
        []() { gMouseTurnSpeed = 12.0f; }
    },
    {
        "GamepadDeadZone",
        "#---------------------------------------------------------------------------------------------------\n"
        "# 0-1 range: controls when minor controller inputs are discarded.\n"
        "# The default of '0.125' only registers movement if the stick is at least 12.5% moved.\n"
        "# Setting too low may result in unwanted jitter and movement when the controller is resting.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0.125", "\n",
        [](const IniUtils::Entry& iniEntry) { gGamepadDeadZone = iniEntry.getFloatValue(0.125f); },
        []() { gGamepadDeadZone = 0.125f; }
    },
    {
        "GamepadFastTurnSpeed_High",
        "#---------------------------------------------------------------------------------------------------\n"
        "# How fast to turn when running ('FastTurnSpeed') and when NOT running ('TurnSpeed').\n"
        "# The game will mix between the 'High' and 'Low' speed values for when running or walking depending\n"
        "# on how far the stick is pushed, using the 'high' speed value completely when the gamepad axis\n"
        "# fully pushed in it's move direction. This replaces the accelerating turning movement of the\n"
        "# original game and allows for more precise control. For reference, the original speed value ranges\n"
        "# with the PSX D-PAD were:\n"
        "#  Walk: 300 - 1000\n"
        "#  Run:  800 - 1400\n"
        "#---------------------------------------------------------------------------------------------------",
        "1400.0", "",
        [](const IniUtils::Entry& iniEntry) { gGamepadFastTurnSpeed_High = iniEntry.getFloatValue(1400.0f); },
        []() { gGamepadFastTurnSpeed_High = 1400.0f; }
    },
    {
        "GamepadFastTurnSpeed_Low",
        "", "800.0", "",
        [](const IniUtils::Entry& iniEntry) { gGamepadFastTurnSpeed_Low = iniEntry.getFloatValue(800.0f); },
        []() { gGamepadFastTurnSpeed_Low = 800.0f; }
    },
    {
        "GamepadTurnSpeed_High",
        "", "1000.0",  "",
        [](const IniUtils::Entry& iniEntry) { gGamepadTurnSpeed_High = iniEntry.getFloatValue(1000.0f); },
        []() { gGamepadTurnSpeed_High = 1000.0f; }
    },
    {
        "GamepadTurnSpeed_Low",
        "", "600.0", "\n",
        [](const IniUtils::Entry& iniEntry) { gGamepadTurnSpeed_Low = iniEntry.getFloatValue(600.0f); },
        []() { gGamepadTurnSpeed_Low = 600.0f; }
    },
    {
        "AnalogToDigitalThreshold",
        "#---------------------------------------------------------------------------------------------------\n"
        "# 0-1 range: controls the point at which an analog axis like a trigger, stick etc. is regarded\n"
        "# as 'pressed' when treated as a digital input (e.g trigger used for 'shoot' action).\n"
        "# The default of '0.5' (halfway depressed) is probably reasonable for most users.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0.5", "\n",
        [](const IniUtils::Entry& iniEntry) { gAnalogToDigitalThreshold = iniEntry.getFloatValue(0.5f); },
        []() { gAnalogToDigitalThreshold = 0.5f; }
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Control bindings
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* const CONTROL_BINDINGS_INI_HEADER = 
R"(#---------------------------------------------------------------------------------------------------
# Control bindings: available input source names/identifiers.
#
# Assign these inputs to actions listed below to setup control bindings.
# Separate multiple input sources for one action using commas (,).
#
# Notes:
#   (1) Gamepad inputs (i.e. Xbox controller style inputs) can only be used for certain types of
#       game controllers which are supported and recognized by the SDL library that PsyDoom uses.
#       If you find your controller is not supported, use generic/numbered joystick inputs instead.
#   (2) All input source names are case insensitive.
#   (3) The following keyboard keys names must be escaped/prefixed with backslash (\) when used:
#         = [ ] # ; ,
#   (4) Similar keyboard keys are collapsed into a range for brevity (e.g A-Z).
#   (5) For a full list of available keyboard key names, including very uncommon ones, see:
#         https://wiki.libsdl.org/SDL_Scancode
#
# Mouse Buttons:
#       Mouse Left              Mouse X1
#       Mouse Right             Mouse X2
#       Mouse Middle
#
# Mouse Wheel axis, normal and inverted (-N to +N range):
#       Mouse Wheel
#       Inv Mouse Wheel
#
# Mouse Wheel axis, positive or negative sub-axis only (0 to +N range):
#       Mouse Wheel+
#       Mouse Wheel-
#
# SDL recognized gamepad: axes with a -1.0 to +1.0 range: normal & inverted
#       Gamepad LeftX           Gamepad RightX          Inv Gamepad LeftX       Inv Gamepad RightX
#       Gamepad LeftY           Gamepad RightY          Inv Gamepad LeftY       Inv Gamepad RightY
#
# SDL recognized gamepad: trigger axes and positive or negative axis subsets with a 0.0 to 1.0 range:
#       Gamepad LeftTrigger     Gamepad RightTrigger
#       Gamepad LeftX-          Gamepad LeftX+          Gamepad LeftY-          Gamepad LeftY+
#       Gamepad RightX-         Gamepad RightX+         Gamepad RightY-         Gamepad RightY+
#
# SDL recognized gamepad: buttons:
#       Gamepad A               Gamepad DpUp            Gamepad LeftStick
#       Gamepad B               Gamepad DpDown          Gamepad RightStick
#       Gamepad X               Gamepad DpLeft          Gamepad LeftShoulder
#       Gamepad Y               Gamepad DpRight         Gamepad RightShoulder
#       Gamepad Back            Gamepad Start           Gamepad Guide
#
# Generic joystick inputs: axes, axis subsets or inversions, buttons and hat/d-pad directions.
# Replace '1-99' with the desired button, hat or axis number:
#       Joystick Button1-99     Joystick Axis1-99
#       Joystick Hat1-99 Up     Joystick Axis1-99+
#       Joystick Hat1-99 Down   Joystick Axis1-99-
#       Joystick Hat1-99 Left   Inv Joystick Axis1-99
#       Joystick Hat1-99 Right
#
# Keyboard keys (commonly used, see link above for full list):
#       A-Z                     Return                  Backspace               Home
#       0-9                     Escape                  Pause                   End
#       Keypad 0-9              Space                   PageUp                  Insert
#       F1-F12                  Tab                     PageDown                Delete
#       Left                    Right                   PrintScreen             CapsLock
#       Up                      Down                    ScrollLock              Numlock
#       Left Ctrl               Left Shift              Left Alt                Application
#       Right Ctrl              Right Shift             Right Alt               Menu
#       Left GUI                Right GUI               VolumeUp                VolumeDown
#       -                       \=                      \[                      \]
#       \                       \#                      \;                      '
#       \,                      `                       .                       /
#       Keypad /                Keypad *                Keypad -                Keypad +
#       Keypad Enter            Keypad .
#---------------------------------------------------------------------------------------------------

)";

// Helper macros
#define CONTROL_BIND_GROUP_HEADER(GROUP_HEADER, BINDING_NAME, BINDING_VALUE)\
    {\
        #BINDING_NAME,\
        GROUP_HEADER, BINDING_VALUE, "",\
        [](const IniUtils::Entry& iniEntry) { Controls::parseBinding(Controls::Binding::BINDING_NAME, iniEntry.value.c_str()); },\
        []() { Controls::parseBinding(Controls::Binding::BINDING_NAME, BINDING_VALUE); }\
    }

#define CONTROL_BIND_GROUP_MIDDLE(BINDING_NAME, BINDING_VALUE)\
    {\
        #BINDING_NAME,\
        "", BINDING_VALUE, "",\
        [](const IniUtils::Entry& iniEntry) { Controls::parseBinding(Controls::Binding::BINDING_NAME, iniEntry.value.c_str()); },\
        []() { Controls::parseBinding(Controls::Binding::BINDING_NAME, BINDING_VALUE); }\
    }

#define CONTROL_BIND_GROUP_FOOTER(BINDING_NAME, BINDING_VALUE)\
    {\
        #BINDING_NAME,\
        "", BINDING_VALUE, "\n",\
        [](const IniUtils::Entry& iniEntry) { Controls::parseBinding(Controls::Binding::BINDING_NAME, iniEntry.value.c_str()); },\
        []() { Controls::parseBinding(Controls::Binding::BINDING_NAME, BINDING_VALUE); }\
    }

static const ConfigFieldHandler CONTROL_BINDINGS_INI_HANDLERS[] = {
    CONTROL_BIND_GROUP_HEADER(
        "#---------------------------------------------------------------------------------------------------\n"
        "# Analog movement and turning actions: these must a gamepad axis with a -1.0 to +1.0 range.\n"
        "# Note: analog turn sensitivity is specified by the gamepad sensitivity values in input_cfg.ini.\n"
        "#---------------------------------------------------------------------------------------------------",
        Analog_MoveForwardBack, "Gamepad LeftY"
    ),
    CONTROL_BIND_GROUP_MIDDLE(Analog_MoveLeftRight, "Gamepad LeftX"),
    CONTROL_BIND_GROUP_FOOTER(Analog_Turn, "Gamepad RightX"),
    //--------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------
    CONTROL_BIND_GROUP_HEADER(
        "#---------------------------------------------------------------------------------------------------\n"
        "# Digital movement and turning actions. Turn sensitivity and acceleration are based on the original\n"
        "# PSX values, though greater precision can be achieved if using uncapped framerates.\n"
        "#---------------------------------------------------------------------------------------------------",
        Digital_MoveForward, "W, Up, Gamepad DpUp"
    ),
    CONTROL_BIND_GROUP_MIDDLE(Digital_MoveBackward, "S, Down, Gamepad DpDown"),
    CONTROL_BIND_GROUP_MIDDLE(Digital_StrafeLeft, "A"),
    CONTROL_BIND_GROUP_MIDDLE(Digital_StrafeRight, "D"),
    CONTROL_BIND_GROUP_MIDDLE(Digital_TurnLeft, "Left, Gamepad DpLeft"),
    CONTROL_BIND_GROUP_FOOTER(Digital_TurnRight, "Right, Gamepad DpRight"),
    //--------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------
    CONTROL_BIND_GROUP_HEADER(
        "#---------------------------------------------------------------------------------------------------\n"
        "# In-game actions & modifiers\n"
        "#---------------------------------------------------------------------------------------------------",
        Action_Use, "Space, E, Mouse Right, Gamepad B"
    ),
    CONTROL_BIND_GROUP_MIDDLE(Action_Attack, "Mouse Left, Gamepad RightTrigger, Left Ctrl, Right Ctrl, Gamepad Y"),
    CONTROL_BIND_GROUP_MIDDLE(Action_Respawn, "Mouse Left, Gamepad RightTrigger, Left Ctrl, Right Ctrl, Gamepad Y"),
    CONTROL_BIND_GROUP_MIDDLE(Modifier_Run, "Left Shift, Right Shift, Gamepad X, Gamepad LeftTrigger"),
    CONTROL_BIND_GROUP_MIDDLE(Modifier_Strafe, "Left Alt, Right Alt, Gamepad A"),
    CONTROL_BIND_GROUP_MIDDLE(Toggle_Autorun, "CapsLock"),
    CONTROL_BIND_GROUP_MIDDLE(Quicksave, "F5"),
    CONTROL_BIND_GROUP_FOOTER(Quickload, "F9"),
    //--------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------
    CONTROL_BIND_GROUP_HEADER(
        "#---------------------------------------------------------------------------------------------------\n"
        "# Toggle in-game pause, automap, and toggle between the Classic and Vulkan renderer (if possible).\n"
        "# Also a control to toggle which player is viewed when playing back multiplayer demos."
        "#---------------------------------------------------------------------------------------------------",
        Toggle_Pause, "Escape, P, Pause, Gamepad Start"
    ),
    CONTROL_BIND_GROUP_MIDDLE(Toggle_Map, "Tab, M, Gamepad Back"),
    CONTROL_BIND_GROUP_MIDDLE(Toggle_Renderer, "`"),
    CONTROL_BIND_GROUP_FOOTER(Toggle_ViewPlayer, "V"),
    //--------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------
    CONTROL_BIND_GROUP_HEADER(
        "#---------------------------------------------------------------------------------------------------\n"
        "# Weapon switching: relative and absolute\n"
        "#---------------------------------------------------------------------------------------------------",
        Weapon_Scroll, "Mouse Wheel"
    ),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_Previous, "PageDown, \\[, Gamepad LeftShoulder"),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_Next, "PageUp, \\], Gamepad RightShoulder"),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_FistChainsaw, "1"),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_Pistol, "2"),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_Shotgun, "3"),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_SuperShotgun, "4"),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_Chaingun, "5"),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_RocketLauncher, "6"),
    CONTROL_BIND_GROUP_MIDDLE(Weapon_PlasmaRifle, "7"),
    CONTROL_BIND_GROUP_FOOTER(Weapon_BFG, "8"),
    //--------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------
    CONTROL_BIND_GROUP_HEADER(
        "#---------------------------------------------------------------------------------------------------\n"
        "# Menu & UI controls\n"
        "#---------------------------------------------------------------------------------------------------",
        Menu_Up, "Up, Gamepad DpUp, Gamepad LeftY-, Gamepad RightY-"
    ),
    CONTROL_BIND_GROUP_MIDDLE(Menu_Down, "Down, Gamepad DpDown, Gamepad LeftY+, Gamepad RightY+"),
    CONTROL_BIND_GROUP_MIDDLE(Menu_Left, "Left, Gamepad DpLeft, Gamepad LeftX-, Gamepad RightX-"),
    CONTROL_BIND_GROUP_MIDDLE(Menu_Right, "Right, Gamepad DpRight, Gamepad LeftX+, Gamepad RightX+"),
    CONTROL_BIND_GROUP_MIDDLE(Menu_Ok, "Return, Space, Mouse Left, Left Ctrl, Right Ctrl, Gamepad A, Gamepad RightTrigger"),
    CONTROL_BIND_GROUP_MIDDLE(Menu_Back, "Escape, Tab, Mouse Right, Gamepad B, Gamepad Back"),
    CONTROL_BIND_GROUP_MIDDLE(Menu_Start, "Gamepad Start"),
    CONTROL_BIND_GROUP_MIDDLE(Menu_EnterPasswordChar, "Return, Space, Mouse Left, Left Ctrl, Right Ctrl, Gamepad A, Gamepad RightTrigger"),
    CONTROL_BIND_GROUP_FOOTER(Menu_DeletePasswordChar, "Delete, Backspace, Mouse Right, Gamepad X, Gamepad LeftTrigger"),
    //--------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------
    CONTROL_BIND_GROUP_HEADER(
        "#---------------------------------------------------------------------------------------------------\n"
        "# Automap controls\n"
        "#---------------------------------------------------------------------------------------------------",
        Automap_ZoomIn, "\\=, Keypad +, Gamepad RightTrigger"
    ),
    CONTROL_BIND_GROUP_MIDDLE(Automap_ZoomOut, "-, Keypad -, Gamepad LeftTrigger"),
    CONTROL_BIND_GROUP_MIDDLE(Automap_MoveUp, "Up, W, Gamepad DpUp, Gamepad LeftY-, Gamepad RightY-"),
    CONTROL_BIND_GROUP_MIDDLE(Automap_MoveDown, "Down, S, Gamepad DpDown, Gamepad LeftY+, Gamepad RightY+"),
    CONTROL_BIND_GROUP_MIDDLE(Automap_MoveLeft, "Left, A, Gamepad DpLeft, Gamepad LeftX-, Gamepad RightX-"),
    CONTROL_BIND_GROUP_MIDDLE(Automap_MoveRight, "Right, D, Gamepad DpRight, Gamepad LeftX+, Gamepad RightX+"),
    CONTROL_BIND_GROUP_FOOTER(Automap_Pan, "F, Left Alt, Right Alt, Gamepad A"),
    //--------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------
    CONTROL_BIND_GROUP_HEADER(
        "#---------------------------------------------------------------------------------------------------\n"
        "# Mappings to the original PlayStation controller buttons for the sole purpose of entering cheat\n"
        "# code sequences on the pause menu, the original way. For example inputs mapped to the PSX 'Cross'\n"
        "# button will be interpreted as that while attempting to enter an original cheat code sequence.\n"
        "#---------------------------------------------------------------------------------------------------",
        PSXCheatCode_Up, "Up, W, Gamepad DpUp, Gamepad LeftY-, Gamepad RightY-"
    ),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_Down, "Down, S, Gamepad DpDown, Gamepad LeftY+, Gamepad RightY+"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_Left, "Left, Gamepad DpLeft, Gamepad LeftX-, Gamepad RightX-"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_Right, "Right, Gamepad DpRight, Gamepad LeftX+, Gamepad RightX+"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_Triangle, "Mouse Left, Left Ctrl, Right Ctrl, Gamepad Y"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_Circle, "Space, Mouse Right, Gamepad B"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_Cross, "F, Left Alt, Right Alt, Gamepad A"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_Square, "Left Shift, Right Shift, Gamepad X"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_L1, "A, Gamepad LeftShoulder"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_R1, "D, Gamepad RightShoulder"),
    CONTROL_BIND_GROUP_MIDDLE(PSXCheatCode_L2, "PageDown, \\[, Gamepad LeftTrigger"),
    CONTROL_BIND_GROUP_FOOTER(PSXCheatCode_R2, "PageUp, \\], Gamepad RightTrigger"),
};

// Done with these helper macros
#undef CONTROL_BIND_GROUP_HEADER
#undef CONTROL_BIND_GROUP_MIDDLE
#undef CONTROL_BIND_GROUP_FOOTER

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

static void setCheatKeySequence(CheatKeySequence& sequence, const char* const pKeysStr) noexcept {
    // Assign a cheat key sequence a value from a string.
    // Only alpha and numeric characters are allowed and the maximum sequence length is 16.
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
        else if (c >= '0' && c <= '9') {
            const uint8_t key = (uint8_t) SDL_SCANCODE_0 + (uint8_t) c - '0';
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

static const ConfigFieldHandler CHEATS_CFG_INI_HANDLERS[] = {
    {
        "EnableDevCheatShortcuts",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Set to '1' to enable convenience single key cheats.\n"
        "# If you frequently use cheats for development purposes then these shortcuts might be useful.\n"
        "# They are disabled by default since they can be accidentally invoked very easily.\n"
        "#\n"
        "# The cheat keys when this setting is enabled are as follows:\n"
        "#\n"
        "#      F1: God mode\n"
        "#      F2: No-clip (new cheat for PC port!)\n"
        "#      F3: All weapons keys and ammo\n"
        "#      F4: Level warp (note: secret maps can now be warped to also)\n"
        "#\n"
        "#      F6: X-ray vision\n"
        "#      F7: VRAM Viewer (functionality hidden in retail)\n"
        "#      F8: No-target (new cheat for PC port!)\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbEnableDevCheatShortcuts = iniEntry.getBoolValue(); },
        []() { gbEnableDevCheatShortcuts = false; }
    },
    {
        "EnableDevInPlaceReloadFunctionKey",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Set to '1' to enable a developer feature where the 'F11' key will do an 'in-place' reload of the\n"
        "# current map. An 'in-place' reload does a normal map reload but restores the player's position and\n"
        "# orientation to what they were prior to the reload. Using this feature allows the map to be\n"
        "# 'refreshed' quickly and edits to be viewed in-engine quickly, enabling faster map iteration.\n"
        "# Note: this function is disallowed in multiplayer games.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbEnableDevInPlaceReloadFunctionKey = iniEntry.getBoolValue(); },
        []() { gbEnableDevInPlaceReloadFunctionKey = false; }
    },
    {
        "EnableDevMapAutoReload",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Set to '1' to enable a developer feature where the game will automatically do an 'in-place' reload\n"
        "# of the current map if it has changed on-disk. Useful for instantly viewing map edits in-engine.\n"
        "# This feature only works for files overridden via the file overrides mechanism, only in single\n"
        "# player mode and only on Windows.\n"
        "#---------------------------------------------------------------------------------------------------",
        "0", "\n",
        [](const IniUtils::Entry& iniEntry) { gbEnableDevMapAutoReload = iniEntry.getBoolValue(); },
        []() { gbEnableDevMapAutoReload = false; }
    },
    {
        "CheatKeySequence_GodMode",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Keyboard key sequences which can be input to activate various cheats in the game.\n"
        "# By default these are mapped to mimick PC DOOM II cheats as much as possible.\n"
        "#\n"
        "# Limitations:\n"
        "#  (1) Only the A-Z and 0-9 keys are acceptable in a key sequence, other characters are ignored.\n"
        "#  (2) The maximum sequence length is 16 keys.\n"
        "#\n"
        "# If you need to input these via a game controller you can do so by inputting the original PSX\n"
        "# buttons on the pause menu. You can see which inputs map to original PSX buttons (for cheat entry)\n"
        "# in the controls configuration .ini file. For reference, the original PSX cheat sequences were:\n"
        "#\n"
        "#      God Mode:               Down, L2, Square, R1, Right, L1, Left, Circle\n"
        "#      Level Warp:             Right, Left, R2, R1, Triangle, L1, Circle, X\n"
        "#      Weapons, armor & keys:  X, Triangle, L1, Up, Down, R2, Left, Left\n"
        "#      Show all map lines:     Triangle, Triangle, L2, R2, L2, R2, R1, Square\n"
        "#      Show all map things:    Triangle, Triangle, L2, R2, L2, R2, R1, Circle\n"
        "#      X-Ray vision:           L1, R2, L2, R1, Right, Triangle, X, Right\n"
        "#\n"
        "# The new cheats added to PsyDoom are assigned the following original PSX buttons:\n"
        "#\n"
        "#      No-clip:                Up, Up, Up, Up, Up, Up, Up, R1\n"
        "#      VRAM viewer:            Triangle, Square, Up, Left, Down, Right, X, Circle\n"
        "#      No-target:              X, Up, X, Up, Square, Square, X, Square\n"
        "#---------------------------------------------------------------------------------------------------",
        "iddqd", "",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_GodMode, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_GodMode, "iddqd"); }
    },
    {
        "CheatKeySequence_NoClip",
        "", "idclip", "",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_NoClip, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_NoClip, "idclip"); }
    },
    {
        "CheatKeySequence_LevelWarp",
        "", "idclev", "",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_LevelWarp, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_LevelWarp, "idclev"); }
    },
    {
        "CheatKeySequence_WeaponsKeysAndArmor",
        "", "idkfa", "",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_WeaponsKeysAndArmor, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_WeaponsKeysAndArmor, "idkfa"); }
    },
    {
        "CheatKeySequence_AllMapLinesOn",
        "", "iddt", "",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_AllMapLinesOn, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_AllMapLinesOn, "iddt"); }
    },
    {
        "CheatKeySequence_AllMapThingsOn",
        "", "idmt", "",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_AllMapThingsOn, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_AllMapThingsOn, "idmt"); }
    },
    {
        "CheatKeySequence_XRayVision",
        "", "idray", "",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_XRayVision, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_XRayVision, "idray"); }
    },
    {
        "CheatKeySequence_VramViewer",
        "", "idram", "",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_VramViewer, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_VramViewer, "idram"); }
    },
    {
        "CheatKeySequence_NoTarget",
        "", "idcloak", "\n",
        [](const IniUtils::Entry& iniEntry) { setCheatKeySequence(gCheatKeys_NoTarget, iniEntry.value.c_str()); },
        []() { setCheatKeySequence(gCheatKeys_NoTarget, "idcloak"); }
    },
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Other config parser related state
//------------------------------------------------------------------------------------------------------------------------------------------

// Set to true if a new config file or new config fields were generated
static bool gbDidGenerateNewConfig;

//------------------------------------------------------------------------------------------------------------------------------------------
// Saves an updated (or completely new) version of the specified config file.
// Existing settings are preserved except for settings that have been removed.
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateConfigFile(
    const std::string& cfgFilePath,
    const ConfigFieldHandler* configFieldHandlers,
    const size_t numConfigFieldHandlers,
    const char* const fileHeader,
    const FileData& existingCfgFileData
) noexcept {
    // Extract all fields from the existing file data (if we have it)
    std::map<std::string, std::string> existingFields;

    if (existingCfgFileData.bytes) {
        IniUtils::parseIniFromString(
            (const char*) existingCfgFileData.bytes.get(),
            existingCfgFileData.size,
            [&](const IniUtils::Entry& iniEntry) noexcept {
                existingFields[iniEntry.key] = iniEntry.value;
            }
        );
    }

    // Open the config file for writing
    std::FILE* const pFile = std::fopen(cfgFilePath.c_str(), FOPEN_WRITE_TEXT);

    if (!pFile)
        return;

    // Write the modification warning and file header if existing
    std::fprintf(pFile, "%s\n", FILE_MODIFICATION_WARNING);

    if (fileHeader && fileHeader[0]) {
        std::fprintf(pFile, "%s\n", fileHeader);
    }

    // Helper lambda: tells if a string ends in a new line character
    const auto endsInNewLine = [](const char* const str) noexcept {
        if (str) {
            if (const size_t len = std::strlen(str); len > 0) {
                const char lastChar = str[len - 1];
                return ((lastChar == '\n') || (lastChar == '\r'));
            }
        }

        return false;
    };

    // Write all config fields out to the file and as we find existing field values, remove them from the map to mark as consumed.
    // We'll dump any unused fields found at the end of the file after we're done.
    for (size_t i = 0; i < numConfigFieldHandlers; ++i) {
        const ConfigFieldHandler& handler = configFieldHandlers[i];

        // Write the preamble for the field, if specified:
        if (handler.preamble && handler.preamble[0]) {
            std::fprintf(pFile, (endsInNewLine(handler.preamble)) ? "%s" : "%s\n", handler.preamble);
        }

        // Write the key and value for the field and use the pre-existing value if it's there.
        // If the existing value is not there, fallback to using the default one.
        const auto existingValIter = existingFields.find(handler.name);

        if (existingValIter != existingFields.end()) {
            std::fprintf(pFile, "%s = %s\n", handler.name, existingValIter->second.c_str());
            existingFields.erase(existingValIter);
        } else {
            std::fprintf(pFile, "%s = %s\n", handler.name, handler.defaultValueStr);
        }

        // Write the postamble for the field, if specified:
        if (handler.postamble && handler.postamble[0]) {
            std::fprintf(pFile, (endsInNewLine(handler.postamble)) ? "%s" : "%s\n", handler.postamble);
        }
    }

    // If there are unused config fields at the end then dump them out following a warning
    if (!existingFields.empty()) {
        std::fprintf(pFile, "%s\n", UNUSED_FIELDS_WARNING);

        for (const auto& kvp : existingFields) {
            std::fprintf(pFile, "%s = %s\n", kvp.first.c_str(), kvp.second.c_str());
        }
    }

    // Close to flush the stream and finish up
    std::fclose(pFile);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse a given config file using the given config field handlers
//------------------------------------------------------------------------------------------------------------------------------------------
static void parseConfigFile(
    const std::string& configFolder,
    const char* fileName,
    const ConfigFieldHandler* configFieldHandlers,
    const size_t numConfigFieldHandlers,
    const char* const cfgFileHeader
) noexcept {
    // Set all values to their initial defaults (until we parse otherwise)
    for (size_t i = 0; i < numConfigFieldHandlers; ++i) {
        configFieldHandlers[i].setValueToDefaultFunc();
    }

    // Store which config field handlers have parsed config here
    std::vector<bool> executedConfigHandler;
    executedConfigHandler.resize(numConfigFieldHandlers);

    // Read and parse the ini file (if it exists).
    // Allow the file to exist in two different locations: (1) The current working directory and (2) The normal config folder.
    // The configuration file in the current working directory takes precedence over the one in the config folder.
    std::string cfgFilePath;
    bool bCfgFileExists;

    if (FileUtils::fileExists(fileName)) {
        cfgFilePath = fileName;
        bCfgFileExists = true;
    } else {
        cfgFilePath = configFolder + fileName;
        bCfgFileExists = FileUtils::fileExists(cfgFilePath.c_str());
    }

    const FileData cfgFileData = (bCfgFileExists) ? FileUtils::getContentsOfFile(cfgFilePath.c_str(), 8, std::byte(0)) : FileData();

    if (bCfgFileExists && cfgFileData.bytes) {
        IniUtils::parseIniFromString(
            (const char*) cfgFileData.bytes.get(),
            cfgFileData.size,
            [&](const IniUtils::Entry& iniEntry) noexcept {
                // Try to find a matching config field handler.
                // This is not an especially smart or fast way of doing it, but performance isn't an issue here:
                for (size_t i = 0; i < numConfigFieldHandlers; ++i) {
                    const ConfigFieldHandler& handler = configFieldHandlers[i];

                    if (iniEntry.key != handler.name)
                        continue;

                    handler.parseFunc(iniEntry);
                    executedConfigHandler[i] = true;
                    break;
                }
            }
        );
    }

    // If we are missing expected config fields then we need to reopen the config and append to it
    size_t numMissingConfigFields = 0;

    for (bool executed : executedConfigHandler) {
        if (!executed) {
            numMissingConfigFields++;
        }
    }

    // Do we need to re-save the config file?
    if (numMissingConfigFields > 0) {
        gbDidGenerateNewConfig = true;
        updateConfigFile(cfgFilePath, configFieldHandlers, numConfigFieldHandlers, cfgFileHeader, cfgFileData);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse all config files
//------------------------------------------------------------------------------------------------------------------------------------------
static void parseAllConfigFiles(const std::string& configFolder) noexcept {
    parseConfigFile(configFolder, "graphics_cfg.ini",   GRAPHICS_CFG_INI_HANDLERS,  C_ARRAY_SIZE(GRAPHICS_CFG_INI_HANDLERS), nullptr);
    parseConfigFile(configFolder, "audio_cfg.ini",      AUDIO_CFG_INI_HANDLERS,     C_ARRAY_SIZE(AUDIO_CFG_INI_HANDLERS), nullptr);
    parseConfigFile(configFolder, "game_cfg.ini",       GAME_CFG_INI_HANDLERS,      C_ARRAY_SIZE(GAME_CFG_INI_HANDLERS), nullptr);
    parseConfigFile(configFolder, "input_cfg.ini",      INPUT_CFG_INI_HANDLERS,     C_ARRAY_SIZE(INPUT_CFG_INI_HANDLERS), nullptr);
    parseConfigFile(configFolder, "cheats_cfg.ini",     CHEATS_CFG_INI_HANDLERS,    C_ARRAY_SIZE(CHEATS_CFG_INI_HANDLERS), nullptr);

    parseConfigFile(
        configFolder,
        "control_bindings.ini",
        CONTROL_BINDINGS_INI_HANDLERS,
        C_ARRAY_SIZE(CONTROL_BINDINGS_INI_HANDLERS),
        CONTROL_BINDINGS_INI_HEADER
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read all config for the app, and generate config files for the user if it's the first launch.
// If new config keys are missing then they will be appended to the existing config files.
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    Controls::clearAllBindings();

    const std::string configFolder = Utils::getOrCreateUserDataFolder();
    parseAllConfigFiles(configFolder);

    // If we generated new config inform the user so changes can be made if required
    if (gbDidGenerateNewConfig) {
        std::string cfgFileMessage =
            "Hey, just a heads up! PsyDoom has generated and defaulted some new configuration settings in one or more .ini files.\n"
            "If you would like to review or edit PsyDoom's settings, you can normally find the .ini files at the following location:\n\n";

        cfgFileMessage.append(configFolder, 0, configFolder.length() - 1);
        cfgFileMessage += "\n\n";
        cfgFileMessage += "Change these files before proceeding to customize game settings.\n";
        cfgFileMessage += "\n";
        cfgFileMessage += "Note: if you wish, you can copy the .ini files to the application's working directory and these .ini\n";
        cfgFileMessage += "files will be recognized and take precedence over the ones in the folder mentioned above.";

        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION,
            "Configuring PsyDoom: new settings available",
            cfgFileMessage.c_str(),
            nullptr
        );

        // Re-parse the config once more after showing the message, just in case the user made changes
        parseAllConfigFiles(configFolder);
    }
}

void shutdown() noexcept {
    // Nothing to do here yet...
}

END_NAMESPACE(Config)
