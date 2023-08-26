#include "ConfigSerialization_Graphics.h"

#include "Config.h"
#include "IniUtils.h"

#include <string>

BEGIN_NAMESPACE(ConfigSerialization)

using namespace Config;

//------------------------------------------------------------------------------------------------------------------------------------------
// Config field storage
//------------------------------------------------------------------------------------------------------------------------------------------
Config_Graphics gConfig_Graphics = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the config serializers for graphics related config
//------------------------------------------------------------------------------------------------------------------------------------------
void initCfgSerialization_Graphics() noexcept {
    auto& cfg = gConfig_Graphics;

    cfg.fullscreen = makeConfigField(
        "Fullscreen",
        "Fullscreen or windowed mode toggle.\n"
        "Enable make PsyDoom launch in fullscreen mode, disable to use windowed mode.",
        gbFullscreen,
        true
    );

    cfg.enableVSync = makeConfigField(
        "EnableVSync",
        "If enabled then request that video output be synchronized with the display to try and avoid\n"
        "artifacts like tearing. Note that this setting might not be respected depending on the host OS\n"
        "and display driver. It is just a request/hint only.",
        gbEnableVSync,
        true
    );

    cfg.outputResolutionW = makeConfigField(
        "OutputResolutionW",
        "Output resolution (width & height) for both the Vulkan and Classic renderers.\n"
        "In windowed mode this is the size of the window.\n"
        "In fullscreen mode this is the actual screen resolution to use.\n"
        "\n"
        "If the values are '0' or less (auto resolution) then this means use the current (desktop)\n"
        "resolution in fullscreen mode, and auto-decide the window size in windowed mode.",
        gOutputResolutionW,
        -1
    );

    cfg.outputResolutionH = makeConfigField(
        "OutputResolutionH",
        "",
        gOutputResolutionH,
        -1
    );

    cfg.outputDisplayIndex = makeConfigField(
        "OutputDisplayIndex",
        "Which display to use for PsyDoom for both fullscreen and windowed modes.\n"
        "If '-1' or an invalid index is specified then PsyDoom will auto-decide based on which display the\n"
        "mouse is within at game launch time.\n"
        "\n"
        "Note: display index '0' is the 1st display, '1' the 2nd display and so on.\n",
        gOutputDisplayIndex,
        -1
    );

    cfg.sectorLightPercentage = makeConfigField(
        "SectorLightPercentage",
        "Manipulate light level of sectors.\n"
        "Useful for older monitors or to make the game even more creepy.\n"
        "\n"
        "Example values:\n"
        "75 - makes game 25% darker\n"
        "125 - makes game 25% lighter\n",
        gSectorLightPercentage,
        100
    );

    cfg.exclusiveFullscreenMode = makeConfigField(
        "ExclusiveFullscreenMode",
        "If fullscreen is enabled, whether to use an exclusive ('real') fullscreen mode instead of 'faking'\n"
        "it via a screen sized borderless window.\n"
        "\n"
        "Exclusive fullscreen mode may offer very small performance benefits but will not handle overlays\n"
        "or task switching as well as a borderless window. In most cases it is probably preferable to leave\n"
        "this setting disabled.",
        gbExclusiveFullscreenMode,
        false
    );
    
    cfg.antiAliasingMultisamples = makeConfigField(
        "AntiAliasingMultisamples",
        "Vulkan renderer only: the number of multisamples to use for anti-aliasing the view.\n"
        "Increasing the number of samples can help smooth edges and prevent texture shimmer, but can be\n"
        "costly to do at high resolutions or on weaker GPUs. 4x is probably reasonable for most GPUs and\n"
        "screen resolution combinations, given the low requirements of Doom.\n"
        "\n"
        "Note: if the hardware is unable to support the number of samples specified then the next available\n"
        "sample count downwards will be selected.",
        gAAMultisamples,
        gDefaultAntiAliasingMultisamples
    );

    cfg.vulkanRenderHeight = makeConfigField(
        "VulkanRenderHeight",
        "Vulkan renderer: determines the vertical resolution (in pixels) of the render/draw framebuffer.\n"
        "You can use this setting to render at a different resolution to the display resolution.\n"
        "\n"
        "Note: the horizontal render resolution is automatically determined using this setting, the\n"
        "current display/window resolution, and the setting of 'VulkanPixelStretch' (see below).\n"
        "\n"
        "Example values:\n"
        " 240 = Use the original PSX vertical resolution (240p: for best results use pixel stretch!)\n"
        " 480 = Use 2x the original PSX vertical resolution (480p: for best results use pixel stretch!)\n"
        "  -1 = Use the native vertical resolution of the display or window",
        gVulkanRenderHeight,
        gDefaultVulkanRenderHeight
    );

    cfg.vulkanPixelStretch = makeConfigField(
        "VulkanPixelStretch",
        "Vulkan renderer: controls whether rendered pixels are stretched horizontally on output according\n"
        "to the stretch factor determined via 'LogicalDisplayWidth' and also vertically due to overscan.\n"
        "It makes the Vulkan renderer mimic the pixel stretching of the original PlayStation renderer\n"
        "more closely and will cause the draw resolution to be reduced as a result.\n"
        "\n"
        "Pixel stretch is highly recommended if you want to match the rasterization of the original 240p\n"
        "renderer or some other low multiple of that like 480p. Without stretching, at low resolutions you\n"
        "may find that UI elements do not rasterize very well due to nearest neighbor filtering and the\n"
        "draw resolution not being an integer multiple of the original resolution.\n"
        "\n"
        "If you are rendering at modern resolutions like 1080p or 1440p however it is recommended that you\n"
        "leave this setting off so that the Vulkan renderer can operate at the highest resolutions and not\n"
        "suffer aliasing from having to stretch the framebuffer on output.",
        gbVulkanPixelStretch,
        false
    );

    cfg.vulkanTripleBuffer = makeConfigField(
        "VulkanTripleBuffer",
        "If the Vulkan video backend is active and the Vulkan API is in use, whether to use triple\n"
        "buffering for output presentation. This setting affects both the classic renderer when it is\n"
        "output via Vulkan and the new Vulkan renderer itself.\n"
        "\n"
        "If enabled the game will render frames as fast as possible and may possibly discard previously\n"
        "rendered/queued frames while waiting for the display to become ready for output.\n"
        "\n"
        "Enabling ensures the most up-to-date view is shown when the time comes to display and helps to\n"
        "reduce perceived input latency but will also greatly increase GPU and CPU usage.\n"
        "Disable if you prefer to lower energy usage for slightly increased input latency.",
        gbVulkanTripleBuffer,
        false
    );

    cfg.vulkanDrawExtendedStatusBar = makeConfigField(
        "VulkanDrawExtendedStatusBar",
        "Vulkan renderer only: draw extensions to the in-game status bar for widescreen mode?\n"
        "\n"
        "PsyDoom can extend the original PSX status bar to 'support' widescreen mode by repeating the part\n"
        "of the bar which contains all the weapon numbers. If you disable this setting, then a black\n"
        "letterbox will be rendered instead. This setting is also ignored if Vulkan widescreen is disabled.",
        gbVulkanDrawExtendedStatusBar,
        true
    );

    cfg.vulkanWidescreenEnabled = makeConfigField(
        "VulkanWidescreenEnabled",
        "Vulkan renderer only: allow extended widescreen rendering?\n"
        "\n"
        "The in-game Vulkan renderer is capable of a wider field of view than the classic renderer on\n"
        "modern widescreen displays. If desired however you can disable this for an aspect ratio more\n"
        "like the original game with cropping at the sides of the screen.\n",
        gbVulkanWidescreenEnabled,
        true
    );

    cfg.useVulkan32BitShading = makeConfigField(
        "UseVulkan32BitShading",
        "Vulkan renderer only: whether higher precision 32-bit shading and framebuffers should be used.\n"
        "\n"
        "The original PSX framebuffer was only 16-bit, and by default the Vulkan renderer also uses this\n"
        "color mode to replicate the original game's lighting and shading as closely as possible.\n"
        "\n"
        "Enabling this will allow for smoother 32-bit shading with less color banding artifacts, but will\n"
        "also increase the overall image brightness and reduce contrast - making the display seem more\n"
        "'washed out' and less atmospheric. It's recommended to leave this setting disabled for better\n"
        "visuals, but if you dislike banding a lot then it can be enabled if needed.",
        gbUseVulkan32BitShading,
        false
    );

    cfg.disableVulkanRenderer = makeConfigField(
        "DisableVulkanRenderer",
        "If enabled then the new Vulkan/hardware renderer will be completely disabled and the game will\n"
        "behave as if Vulkan is not available for use to PsyDoom, even if the opposite is true.\n"
        "\n"
        "If you only want to run PsyDoom using the classic PSX renderer then enabling this setting will\n"
        "save on system & video RAM and other resources. By default if Vulkan rendering is possible then\n"
        "the Vulkan renderer always needs to be active and use memory in order to allow for fast toggling\n"
        "between renderers.",
        gbDisableVulkanRenderer,
        false
    );

    cfg.logicalDisplayWidth = makeConfigField(
        "LogicalDisplayWidth",
        "Determines the game's horizontal stretch factor and affects both the Classic and Vulkan renderers.\n"
        "\n"
        "Given an original logical resolution of 256x240 pixels, this is the width to stretch that out to.\n"
        "If set to '292' for example, then the stretch factor would be 292/256 or 1.14 approximately.\n"
        "\n"
        "Some background information:\n"
        "PSX Doom (NTSC) originally rendered to a 256x240 framebuffer but stretched that to approximately\n"
        "292x240 (in square pixel terms, or 320x240 in CRT non-square pixels) on output; this made the game\n"
        "feel much flatter and more compressed than the PC original. The PAL version also did the exact\n"
        "same stretching and simply added additional letterboxing to fill the extra scanlines on the top\n"
        "and bottom of the screen.\n"
        "\n"
        "Typical values:\n"
        " 292 = Use (approximately) the original PSX Doom stretch factor for the most authentic look.\n"
        " 256 = Don't stretch horizontally. Makes for a more PC-like view, but some art won't look right.\n"
        " -1  = Stretch to fill: the logical display width is automatically set such that the original PSX\n"
        "       view area and UI elements will fill the display completely, regardless of how wide it is.",
        gLogicalDisplayW,
        292.0f
    );

    cfg.topOverscanPixels = makeConfigField(
        "TopOverscanPixels",
        "With respect to the original resolution of 256x240, determines how many rows of pixels are\n"
        "discarded at the top and bottom of the screen in order to emulate 'overscan' behavior of old CRT\n"
        "televisions. Affects the display and aspect ratio for both the Vulkan and Classic renderers.\n"
        "\n"
        "Accounting for overscan can help yield an aspect ratio closer to how the game looked on CRT TVs of\n"
        "it's time and can also be used to remove a region of dead space underneath the in-game HUD.\n"
        "By default, PsyDoom will chop off 6 pixel rows at the bottom of the screen to remove some of the\n"
        "HUD dead space without truncating the status bar or Doomguy's mugshot; the top of the screen will\n"
        "also be left untouched by default in order to maximize the viewable area.",
        gTopOverscanPixels,
        0
    );

    cfg.bottomOverscanPixels = makeConfigField(
        "BottomOverscanPixels",
        "",
        gBottomOverscanPixels,
        6
    );

    cfg.enhanceWallDrawPrecision = makeConfigField(
        "EnhanceWallDrawPrecision",
        "Classic renderer only: whether to enable a new enhanced precision mode when rendering walls.\n"
        "Helps prevent textures from stretching and sliding horizontally and makes texture mapping much\n"
        "more temporally stable when using an uncapped framerate.",
        gbEnhanceWallDrawPrecision,
        true
    );

    cfg.floorRenderGapFix = makeConfigField(
        "FloorRenderGapFix",
        "Classic renderer only: whether to enable a precision fix for the floor renderer to prevent gaps\n"
        "in the floor on some maps. This fix helps prevent some noticeable glitches on larger outdoor maps\n"
        "like 'Tower Of Babel'.",
        gbFloorRenderGapFix,
        true
    );

    cfg.skyLeakFix = makeConfigField(
        "SkyLeakFix",
        "Classic renderer only: enable a limit removing feature that fixes ceilings and walls sometimes\n"
        "poking through the sky if sectors beyond the sky have higher ceilings. This isn't really much of\n"
        "an issue for the original retail maps but may frustrate the design of user maps if not fixed.\n"
        "\n"
        "Notes:\n"
        " (1) The Vulkan renderer has this fix always enabled.\n"
        " (2) Enabling this setting will reduce performance in Classic renderer mode due to extra sky\n"
        "     wall columns that must be drawn to patch over 'leaks' from rooms beyond.",
        gbSkyLeakFix,
        true
    );

    cfg.vulkanBrightenAutomap = makeConfigField(
        "VulkanBrightenAutomap",
        "Vulkan renderer only: if enabled then automap lines will be brightened to compensate for them\n"
        "appearing perceptually darker at higher resolutions, due to the lines being much thinner.\n"
        "\n"
        "This tweak helps a high res Vulkan automap feel more like the original (low resolution) automap.\n"
        "If you are running the Vulkan renderer at low resolution however, you may want to disable this.",
        gbVulkanBrightenAutomap,
        true
    );

    cfg.useExtendedAutomapColors = makeConfigField(
        "UseExtendedAutomapColors",
        "If enabled then live enemies and special bonus items (that count towards the player's item total)\n"
        "will be colored differently when displayed on the automap via the 'Map All Things' cheat.\n"
        "This makes identifying all remaining enemies and bonus items easier.\n",
        gbUseExtendedAutomapColors,
        false
    );

    cfg.vramSizeInMegabytes = makeConfigField(
        "VramSizeInMegabytes",
        "Specifies how many megabytes of video RAM are available to hold sprites and textures in the game.\n"
        "The PlayStation originally had 1 MiB of VRAM, expanding this allows for much more complex user\n"
        "maps to be supported and helps eliminate the 'Texture Cache Overflow' warning on busy maps with\n"
        "huge amounts of enemies onscreen.\n"
        "\n"
        "Acceptable values are 1, 2, 4, 8, 16, 32, 64 and 128. Values in-between will be rounded up.\n"
        "If <= 0 is specified then the maximum amount possible will be used.\n",
        gVramSizeInMegabytes,
        gDefaultVramSizeInMegabytes
    );

    cfg.vulkanPreferredDevicesRegex = makeConfigField(
        "VulkanPreferredDevicesRegex",
        "Vulkan renderer: a case insensitive regex that can specify which GPUs are preferable to use.\n"
        "\n"
        "Useful in multi-GPU systems where you want to override PsyDoom's default 'best' device selection\n"
        "and choose a particular GPU to render with. If the regex matches part of the device's name then it\n"
        "is considered 'preferred' and selected over all other devices. If no preferred device can be\n"
        "chosen, then PsyDoom will attempt to select what it thinks is the best non-preferred device as a\n"
        "fallback.\n"
        "\n"
        "Example values:\n"
        " AMD\n"
        " NVIDIA.*RTX.*\n"
        " Intel",
        gVulkanPreferredDevicesRegex,
        ""
    );
}

END_NAMESPACE(ConfigSerialization)
