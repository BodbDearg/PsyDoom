//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan renderer, core manager/module.
// Manages high level render paths and details for the frame lifecycle with Vulkan.
// Also creates several key objects, such as the swapchain, the primary command buffer and so on.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VRenderer.h"

#if PSYDOOM_VULKAN_RENDERER

#include "CmdBufferRecorder.h"
#include "CmdBufferWaitCond.h"
#include "Gpu.h"
#include "LogicalDevice.h"
#include "PcPsx/Config.h"
#include "PcPsx/PlayerPrefs.h"
#include "PcPsx/PsxVm.h"
#include "PcPsx/Video.h"
#include "PhysicalDevice.h"
#include "PhysicalDeviceSelection.h"
#include "Semaphore.h"
#include "Swapchain.h"
#include "Texture.h"
#include "VCrossfader.h"
#include "VDrawing.h"
#include "VkFuncs.h"
#include "VPipelines.h"
#include "VPlaqueDrawer.h"
#include "VRenderPath_Crossfade.h"
#include "VRenderPath_Main.h"
#include "VRenderPath_Psx.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"

#include <regex>
#include <SDL_vulkan.h>

BEGIN_NAMESPACE(VRenderer)

// What color surface formats are supported for presentation, with the most preferential being first.
// B8G8R8A8 should be supported everwhere and apparently is the most performant, but just in case offer some alternatives...
static constexpr VkFormat ALLOWED_COLOR_SURFACE_FORMATS[] = {
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32
};

// What format we use for 32-bit color and the preferred format for 16-bit color.
// Note that 'VK_FORMAT_A1R5G5B5_UNORM_PACK16' is not required to be supported by Vulkan as a framebuffer attachment, but it's pretty ubiquitous *EXCEPT* on Apple/Metal.
// For Apple platforms 16-bit format support via Metal/MoltenVK is pretty much non existant, so fall back to using the 32-bit format instead.
// The blending won't look as correct, but at least it works (sigh)...
static constexpr VkFormat COLOR_16_FORMAT = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
static constexpr VkFormat COLOR_32_FORMAT = VK_FORMAT_B8G8R8A8_UNORM;

// Cached pointers to all Vulkan API functions
vgl::VkFuncs gVkFuncs;

// Set to true if the 16 bit color format is supported for the PSX and Vulkan framebuffers.
// Fallback to using the 32-bit color format if 16-bit is not available.
static bool gbCanPsxFbUse16BitColor;
static bool gbCanVulkanFbUse16BitColor;

// Render paths used
VRenderPath_Psx         gRenderPath_Psx;
VRenderPath_Main        gRenderPath_Main;
VRenderPath_Crossfade   gRenderPath_Crossfade;

// Coord system info: the width and height of the window/swap-chain surface we present to
uint32_t    gPresentSurfaceW;
uint32_t    gPresentSurfaceH;

// Coord system info: the width and height to use for framebuffers created - esentially the render/draw resolution.
// Also the inverse of that for quick division.
uint32_t    gFramebufferW;
uint32_t    gFramebufferH;
float       gInvFramebufferW;
float       gInvFramebufferH;

// Coord system info: the current logical display width for the game.
// This may change depending on the display size, if free stretching mode is enabled.
float gCurLogicalDisplayW;

// Coord system info: where the original PSX GPU framebuffer/coord-system maps to on the draw/render framebuffer
float gPsxCoordsFbX;
float gPsxCoordsFbY;
float gPsxCoordsFbW;
float gPsxCoordsFbH;

// Coord system conversion: scalars to convert from normalized device coords to the original PSX framebuffer coords (256x240).
// Also where the where the original PSX framebuffer coords start in NDC space.
//
// Note: these are updated only at the start of each frame and ONLY if there is a valid framebuffer.
// They should not be used outside of rendering and should only be used if the framebuffer is valid.
float gNdcToPsxScaleX, gNdcToPsxScaleY;
float gPsxNdcOffsetX,  gPsxNdcOffsetY;

static bool                         gbDidBeginFrame;                // Whether a frame was begun
static vgl::VulkanInstance          gVulkanInstance(gVkFuncs);      // Vulkan API instance object
static vgl::WindowSurface           gWindowSurface;                 // Window surface to draw on
static const vgl::PhysicalDevice*   gpPhysicalDevice;               // Physical device chosen for rendering
static VkFormat                     gPresentSurfaceFormat;          // What color format the surface we are presenting to should be in
static VkColorSpaceKHR              gPresentSurfaceColorspace;      // What colorspace the surface we are presenting to should use
static uint32_t                     gDrawSampleCount;               // The number of samples to use when drawing (if > 1 then MSAA is active)

vgl::LogicalDevice  gDevice(gVkFuncs);      // The logical device used for Vulkan
vgl::Swapchain      gSwapchain;             // The swapchain we present to

// Command buffer recorder helper object.
// This begins recording at the start of every frame.
vgl::CmdBufferRecorder gCmdBufferRec(gVkFuncs);

// Semaphores signalled when the acquired swapchain image is ready.
// We flip/flop between these when acquiring images, since there can be at most 1 other frame active.
// Unlike most other resources however, the ringbuffer index is NOT used to index this array.
// This is because sometimes we don't present a swapchain image in a frame and instead carry it over to the next frame.
static vgl::Semaphore gSwapImageReadySemaphores[2];

// Which of the two 'swap image ready' semaphores we are currently using.
// This field is only updated after swap image presentation or when the semaphores are created/recreated.
static uint32_t gCurSwapchainSemaphoreIdx;

// Semaphore signalled when rendering is done.
// One per ringbuffer slot, so we don't disturb a frame that is still being processed.
static vgl::Semaphore gRenderDoneSemaphores[vgl::Defines::RINGBUFFER_SIZE];

// Primary command buffers used for drawing operations in each frame.
// One for each ringbuffer slot, so we can record a new buffer while a previous frame's buffer is still executing.
static vgl::CmdBuffer gCmdBuffers[vgl::Defines::RINGBUFFER_SIZE];

// A mirrored copy of PSX VRAM (minus framebuffers) so we can access in the new Vulkan renderer.
// Any texture uploads to PSX VRAM will get passed along from LIBGPU and eventually find their way in here.
static vgl::Texture gPsxVramTexture;

// The current and next frame render paths to use: these should always be valid
static IVRendererPath* gpCurRenderPath;
static IVRendererPath* gpNextRenderPath;

// If true then skip presenting the next frame
static bool gbSkipNextFramePresent;

// If true then we must wait on the swapchain image semaphore this frame (we had to acquire it)
static bool gbDidAcquireSwapImageThisFrame;

//------------------------------------------------------------------------------------------------------------------------------------------
// Decides the multisample anti-aliasing sample count based on user preferences and hardware capabilities
//------------------------------------------------------------------------------------------------------------------------------------------
static void decideDrawSampleCount() noexcept {
    // Must do at least 1 sample
    const int32_t wantedNumSamples = std::max(Config::gAAMultisamples, 1);

    // Vulkan specifies up to 64x MSAA currently...
    const VkPhysicalDeviceLimits& deviceLimits = gpPhysicalDevice->getProps().limits;
    const VkSampleCountFlags deviceMsaaModes = deviceLimits.framebufferColorSampleCounts;

    if ((wantedNumSamples >= 64) && (deviceMsaaModes & VK_SAMPLE_COUNT_64_BIT)) {
        gDrawSampleCount = 64;
    } else if ((wantedNumSamples >= 32) && (deviceMsaaModes & VK_SAMPLE_COUNT_32_BIT)) {
        gDrawSampleCount = 32;
    } else if ((wantedNumSamples >= 16) && (deviceMsaaModes & VK_SAMPLE_COUNT_16_BIT)) {
        gDrawSampleCount = 16;
    } else if ((wantedNumSamples >= 8) && (deviceMsaaModes & VK_SAMPLE_COUNT_8_BIT)) {
        gDrawSampleCount = 8;
    } else if ((wantedNumSamples >= 4) && (deviceMsaaModes & VK_SAMPLE_COUNT_4_BIT)) {
        gDrawSampleCount = 4;
    } else if ((wantedNumSamples >= 2) && (deviceMsaaModes & VK_SAMPLE_COUNT_2_BIT)) {
        gDrawSampleCount = 2;
    } else {
        gDrawSampleCount = 1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines if 16-bit color framebuffers and textures are possible for the specified device
//------------------------------------------------------------------------------------------------------------------------------------------
static void determine16BitColorSupport(const vgl::PhysicalDevice& device) noexcept {
    // Can the PSX framebuffer texture use a 16-bit format?
    // This is not used for any rendering, just for copying to and blitting from...
    gbCanPsxFbUse16BitColor = device.findFirstSupportedLinearTilingFormat(
        &COLOR_16_FORMAT,
        1,
        VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT
    );

    // Can the main draw framebuffer for the Vulkan renderer use 16-bit color?
    gbCanVulkanFbUse16BitColor = device.findFirstSupportedRenderTextureFormat(
        &COLOR_16_FORMAT,
        1,
        VK_IMAGE_TILING_OPTIMAL,
        // Color attachment can either be used as a transfer & sampling source (for blits and crossfades, with no MSAA) or an input attachment for MSAA resolve
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | ((gDrawSampleCount > 1) ? 0 : VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decides on the color format used for the window/presentation surface as well as the colorspace
//------------------------------------------------------------------------------------------------------------------------------------------
static void decidePresentSurfaceFormat() noexcept {
    // Firstly query the device surface capabilities
    vgl::DeviceSurfaceCaps surfaceCaps;

    if (!surfaceCaps.query(*gpPhysicalDevice, gWindowSurface))
        FatalErrors::raise("Failed to query device surface capabilities!");

    // Try to choose a valid surface format from what is available.
    // Note hardcoding the colorspace to deliberately to SRGB - not considering HDR10 etc. for this project.
    gPresentSurfaceFormat = surfaceCaps.getSupportedSurfaceFormat(ALLOWED_COLOR_SURFACE_FORMATS, C_ARRAY_SIZE(ALLOWED_COLOR_SURFACE_FORMATS));
    gPresentSurfaceColorspace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    if (gPresentSurfaceFormat == VK_FORMAT_UNDEFINED)
        FatalErrors::raise("Failed to find a suitable surface format to present with!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update the resolution to render at and also helper values which aid converting between coordinate systems.
// Updates everything based on the current present surface size.
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateCoordSysInfo() noexcept {
    // Save the size of the present surface
    if (gSwapchain.isValid()) {
        gPresentSurfaceW = gSwapchain.getSwapExtentWidth();
        gPresentSurfaceH = gSwapchain.getSwapExtentHeight();
    } else {
        gPresentSurfaceW = 0;
        gPresentSurfaceH = 0;
    }

    // Save the size of the framebuffer
    const bool bHaveValidPresentSurface = ((gPresentSurfaceW > 0) && (gPresentSurfaceH > 0));

    if (bHaveValidPresentSurface) {
        // Custom render height or just use the present surface dimensions?
        const int32_t userRenderH = Config::gVulkanRenderHeight;

        if (userRenderH > 0) {
            gFramebufferW = (uint32_t)(((uint64_t) userRenderH * gPresentSurfaceW) / gPresentSurfaceH);
            gFramebufferH = userRenderH;
        } else {
            gFramebufferW = gPresentSurfaceW;
            gFramebufferH = gPresentSurfaceH;
        }
    } else {
        gFramebufferW = 0;
        gFramebufferH = 0;
    }

    // Update the logical display width
    if (Config::gLogicalDisplayW <= 0) {
        if (gFramebufferH > 0) {
            const float yScale = (float) gFramebufferH / (float) Video::ORIG_DISP_RES_Y;
            gCurLogicalDisplayW = (float) gFramebufferW / yScale;
        } else {
            gCurLogicalDisplayW = 0.0f;
        }
    } else {
        gCurLogicalDisplayW = Config::gLogicalDisplayW;
    }

    // Get the rectangular region of the framebuffer that the original PlayStation framebuffer/coord-system would map to.
    // This info is also used by the new Vulkan renderer to setup coordinate systems and projection matrices.
    if (bHaveValidPresentSurface) {
        Video::getClassicFramebufferWindowRect(
            (float) gFramebufferW,
            (float) gFramebufferH,
            gPsxCoordsFbX,
            gPsxCoordsFbY,
            gPsxCoordsFbW,
            gPsxCoordsFbH
        );
    } else {
        gPsxCoordsFbX = 0;
        gPsxCoordsFbY = 0;
        gPsxCoordsFbW = 0;
        gPsxCoordsFbH = 0;
    }

    // Is the framebuffer being pixel stretched? If that is the case then adjust the width and height
    if (gFramebufferW > 0) {
        if (Config::gbVulkanPixelStretch) {
            const float displayedPsxRows = (float)(Video::ORIG_DRAW_RES_Y - Video::gTopOverscan - Video::gBotOverscan);
            const float xPixelStretch = (float) Video::ORIG_DRAW_RES_X / gCurLogicalDisplayW;
            const float yPixelStretch = displayedPsxRows / (float) Video::ORIG_DRAW_RES_Y;

            gFramebufferW = (int32_t) std::max(std::ceil((float) gFramebufferW * xPixelStretch), 1.0f);
            gPsxCoordsFbX *= xPixelStretch;
            gPsxCoordsFbW *= xPixelStretch;
            gPsxCoordsFbX = std::ceil(gPsxCoordsFbX);
            gPsxCoordsFbW = std::ceil(gPsxCoordsFbW);

            gFramebufferH = (int32_t) std::max(std::ceil((float) gFramebufferH * yPixelStretch), 1.0f);
            gPsxCoordsFbY *= yPixelStretch;
            gPsxCoordsFbH *= yPixelStretch;
            gPsxCoordsFbY = std::ceil(gPsxCoordsFbY);
            gPsxCoordsFbH = std::ceil(gPsxCoordsFbH);
        }
    }

    // Compute scalings and offsets to help convert from normalized device coords to original PSX framebuffer (256x240) coords.
    // This is useful for things like sky rendering, which work in NDC space.
    //
    // Notes:
    //  (1) If there is space leftover at the sides (when widescreen is enabled) then the original PSX framebuffer is centered in NDC space.
    //  (2) I don't allow vertical stretching of the original UI assets, hence the 'Y' axis conversions here are fixed.
    //      The game viewport will be letterboxed (rather than extend) if the view is too long on the vertical axis.
    //      Because of all this, it is assumed the PSX framebuffer uses all of NDC space vertically.
    const bool bAllowWidescreen = Config::gbVulkanWidescreenEnabled;

    if (bHaveValidPresentSurface) {
        const float blitWPercent = (bAllowWidescreen) ? (float) gPsxCoordsFbW / (float) gFramebufferW : 1.0f;
        const float displayedPsxRows = (float)(Video::ORIG_DRAW_RES_Y - Video::gTopOverscan - Video::gBotOverscan);

        gNdcToPsxScaleX = (0.5f / blitWPercent) * Video::ORIG_DRAW_RES_X;
        gNdcToPsxScaleY = 0.5f * displayedPsxRows;

        const float blitStartXPercent = (bAllowWidescreen) ? (float) gPsxCoordsFbX / (float) gFramebufferW : 0.0f;
        gPsxNdcOffsetX = blitStartXPercent * 2.0f;
        gPsxNdcOffsetY = ((float) -Video::gTopOverscan * 2.0f) / displayedPsxRows;
    } else {
        gNdcToPsxScaleX = 0.0f;
        gNdcToPsxScaleY = 0.0f;
        gPsxNdcOffsetX = 0.0f;
        gPsxNdcOffsetY = 0.0f;
    }

    // Compute inverse framebuffer width and height
    if (bHaveValidPresentSurface) {
        gInvFramebufferW = 1.0f / (float) gFramebufferW;
        gInvFramebufferH = 1.0f / (float) gFramebufferH;
    } else {
        gInvFramebufferW = 0.0f;
        gInvFramebufferH = 0.0f;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create or recreate the 'swap image ready' semaphores
//------------------------------------------------------------------------------------------------------------------------------------------
static void recreateSwapImageReadySemaphores() noexcept {
    gCurSwapchainSemaphoreIdx = 0;
    
    for (vgl::Semaphore& semaphore : gSwapImageReadySemaphores) {
        semaphore.destroy();
        
        if (!semaphore.init(gDevice))
            FatalErrors::raise("Failed to create a Vulkan 'swapchain image ready' semaphore!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recreates the swapchain and new Vulkan renderer framebuffers if required and returns 'false' if that is not possible to do currently.
// Recreation might fail validly if the window is currently zero sized for example.
//
// Note: this function also creates the 'swap image ready' semaphores that are used with the swapchain.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool ensureValidSwapchainAndFramebuffers() noexcept {
    // Sanity checks
    ASSERT(gpCurRenderPath);

    // No swapchain or invalid swapchain? If that is the case then try to create or re-create...
    if ((!gSwapchain.isValid()) || gSwapchain.needsRecreate() || VRenderer::isSwapchainOutOfDate()) {
        gDevice.waitUntilDeviceIdle();
        gSwapchain.destroy();

        if (!gSwapchain.init(gDevice, gPresentSurfaceFormat, gPresentSurfaceColorspace, Config::gbVulkanTripleBuffer))
            return false;
            
        // Create or recreate the swap image synchronization semaphores too. We do this every time the swapchain is recreated in case
        // one of the 'image ready' semaphores got signalled before drawing commands could consume that signal.
        // The semaphores must always be in an unsignalled state before being used by 'vkAcquireNextImageKHR'.
        recreateSwapImageReadySemaphores();
    }

    // Do we need to update coord system info?
    const uint32_t presentSurfaceW = gSwapchain.getSwapExtentWidth();
    const uint32_t presentSurfaceH = gSwapchain.getSwapExtentHeight();

    if ((presentSurfaceW != gPresentSurfaceW) || (presentSurfaceH != gPresentSurfaceH)) {
        updateCoordSysInfo();
    }

    // Ensure the current render path has valid framebuffers
    ASSERT(gFramebufferW > 0);
    ASSERT(gFramebufferH > 0);
    return gpCurRenderPath->ensureValidFramebuffers(gFramebufferW, gFramebufferH);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the specified physical device is suitable for use with PsyDoom and it's Vulkan renderer.
// Checks the capabilities of the device to see if it can do what we need and whether certain required formats are supported.
// This suitability check doesn't consider device surface capabilities; it just verifies what is possible to check for 'headless' mode.
//------------------------------------------------------------------------------------------------------------------------------------------
bool isHeadlessPhysicalDeviceSuitable(const vgl::PhysicalDevice& device) noexcept {
    // Must pass basic suitability checks first
    if (!vgl::PhysicalDeviceSelection::checkBasicHeadlessDeviceSuitability(device))
        return false;

    // Check that the PSX framebuffer can be created in either 16-bit or 32-bit color mode.
    // We prefer 16-bit but fallback to 32-bit when that is unavailable:
    constexpr VkFormatFeatureFlags PSX_FB_FORMAT_FEATURES = VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT;
    const bool bPsxFbFormatSupported = (
        (device.findFirstSupportedLinearTilingFormat(&COLOR_16_FORMAT, 1, PSX_FB_FORMAT_FEATURES) != VK_FORMAT_UNDEFINED) ||
        (device.findFirstSupportedLinearTilingFormat(&COLOR_32_FORMAT, 1, PSX_FB_FORMAT_FEATURES) != VK_FORMAT_UNDEFINED)
    );

    if (!bPsxFbFormatSupported)
        return false;

    // Check that texture format used for PSX VRAM is supported
    constexpr VkFormat R16_UINT_FORMAT = VK_FORMAT_R16_UINT;

    if (!device.findFirstSupportedTextureFormat(&R16_UINT_FORMAT, 1, VK_IMAGE_TILING_OPTIMAL, 0))
        return false;

    // Check that the texture format used for the Vulkan renderer framebuffer is supported in either 16-bit color 32-bit color.
    // Because we don't know if MSAA will be enabled at this point, check the requirements for when both MSAA is enabled AND disabled.
    constexpr VkFormatFeatureFlags VK_FB_FORMAT_FEATURES = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT;
    const bool bVkFbFormatSupported = (
        device.findFirstSupportedRenderTextureFormat(&COLOR_16_FORMAT, 1, VK_IMAGE_TILING_OPTIMAL, VK_FB_FORMAT_FEATURES) ||
        device.findFirstSupportedRenderTextureFormat(&COLOR_32_FORMAT, 1, VK_IMAGE_TILING_OPTIMAL, VK_FB_FORMAT_FEATURES) 
    );

    if (!bVkFbFormatSupported)
        return false;

    // Make sure that the format used for the plaque drawer is supported
    constexpr VkFormat PLAQUE_FORMAT = VK_FORMAT_B8G8R8A8_UNORM;

    if (!device.findFirstSupportedTextureFormat(&PLAQUE_FORMAT, 1, VK_IMAGE_TILING_OPTIMAL, 0))
        return false;

    // Make sure one of the output surface formats is supported
    const bool bFoundSupportedSurfaceFormat = device.findFirstSupportedRenderTextureFormat(
        ALLOWED_COLOR_SURFACE_FORMATS,
        C_ARRAY_SIZE(ALLOWED_COLOR_SURFACE_FORMATS),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_BLIT_DST_BIT      // So it can be blitted to
    );

    if (!bFoundSupportedSurfaceFormat)
        return false;

    // Make sure all required vertex attribute formats are supported
    constexpr VkFormat VERTEX_ATTRIB_FORMATS[] = {
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R8G8B8A8_UINT,
        VK_FORMAT_R16G16_UINT,
        VK_FORMAT_R32G32_SFLOAT
    };

    for (VkFormat format : VERTEX_ATTRIB_FORMATS) {
        if (!device.findFirstSupportedBufferFormat(&format, 1, VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT))
            return false;
    }

    // Device is good if we get to here!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the specified physical device is suitable for use with PsyDoom and it's Vulkan renderer.
// Checks the capabilities of the device to see if it can do what we need and whether certain required formats are supported.
// This suitability check considers both headless device capabilities, and capabilities with respect to the given output surface.
//------------------------------------------------------------------------------------------------------------------------------------------
bool isPhysicalDeviceSuitable(const vgl::PhysicalDevice& device, const vgl::DeviceSurfaceCaps& surfaceCaps) noexcept {
    // Must pass basic suitability checks first
    if (!vgl::PhysicalDeviceSelection::checkBasicDeviceSuitability(device, surfaceCaps))
        return false;

    // Must pass all of the headless checks
    if (!isHeadlessPhysicalDeviceSuitable(device))
        return false;

    // Make sure at least one of the allowed output surface formats are supported
    if (!surfaceCaps.getSupportedSurfaceFormat(ALLOWED_COLOR_SURFACE_FORMATS, C_ARRAY_SIZE(ALLOWED_COLOR_SURFACE_FORMATS)))
        return false;

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Coord sys info is initially invalid
    updateCoordSysInfo();

    // Initialize the Vulkan API and the window surface
    if (!gVulkanInstance.init(Video::gpSdlWindow))
        FatalErrors::raise("Failed to initialize a Vulkan API instance!");

    if (!gWindowSurface.init(Video::gpSdlWindow, gVulkanInstance))
        FatalErrors::raise("Failed to initialize a Vulkan window surface!");

    // Choose a device to use and try to use the preferred device regex if set.
    // If the regex is not specified or it does not produce a valid device selection then try selection against all devices.
    const char* const preferredGpusRegexStr = Config::getVulkanPreferredDevicesRegex();

    if (preferredGpusRegexStr && preferredGpusRegexStr[0]) {
        try {
            std::regex preferredGpusRegex(preferredGpusRegexStr, std::regex_constants::ECMAScript | std::regex_constants::icase);

            gpPhysicalDevice = vgl::PhysicalDeviceSelection::selectBestDevice(
                gVulkanInstance.getPhysicalDevices(),
                gWindowSurface,
                [&](const vgl::PhysicalDevice& device, const vgl::DeviceSurfaceCaps& surfaceCaps) -> bool {
                    if (!std::regex_search(device.getName(), preferredGpusRegex))
                        return false;

                    return isPhysicalDeviceSuitable(device, surfaceCaps);
                }
            );

        } catch (...) {
            FatalErrors::raiseF("Invalid value for 'VulkanPreferredDevicesRegex' - not a valid regex:\n%s", preferredGpusRegexStr);
        }
    }

    if (!gpPhysicalDevice) {
        gpPhysicalDevice = vgl::PhysicalDeviceSelection::selectBestDevice(
            gVulkanInstance.getPhysicalDevices(),
            gWindowSurface,
            isPhysicalDeviceSuitable
        );
    }

    if (!gpPhysicalDevice) {
        FatalErrors::raise(
            "Failed to find a suitable rendering device for use with Vulkan!\n"
            "Set 'DisableVulkanRenderer' to '1' in 'graphics_cfg.ini' as a workaround if this error continues to occur."
        );
    }

    // Decide whether 16-bit color is possible, draw sample count and window/present surface format
    decideDrawSampleCount();
    determine16BitColorSupport(*gpPhysicalDevice);
    decidePresentSurfaceFormat();

    // Initialize the logical Vulkan device used for commands and operations and then 
    if (!gDevice.init(*gpPhysicalDevice, &gWindowSurface))
        FatalErrors::raise("Failed to initialize a Vulkan logical device!");

    // Initialize all pipeline components: must be done BEFORE creating render paths, as they rely on some components
    VPipelines::initPipelineComponents(gDevice, gDrawSampleCount);

    // Initialize all render paths
    const VkFormat drawColorFormat = (Config::gbUseVulkan32BitShading || (!gbCanVulkanFbUse16BitColor)) ? COLOR_32_FORMAT : COLOR_16_FORMAT;

    gRenderPath_Psx.init(gDevice, (gbCanPsxFbUse16BitColor) ? COLOR_16_FORMAT : COLOR_32_FORMAT);
    gRenderPath_Main.init(gDevice, gDrawSampleCount, drawColorFormat, COLOR_32_FORMAT);
    gRenderPath_Crossfade.init(gDevice, gSwapchain, gPresentSurfaceFormat, gRenderPath_Main);

    // Create all of the pipelines needed, these use the previously created pipeline components
    VPipelines::initPipelines(gRenderPath_Main, gRenderPath_Crossfade, gDrawSampleCount);

    // Create the 'render done' semaphores
    for (vgl::Semaphore& semaphore : gRenderDoneSemaphores) {
        if (!semaphore.init(gDevice))
            FatalErrors::raise("Failed to create the Vulkan 'render done' semaphore!");
    }

    // Init the command buffers
    for (vgl::CmdBuffer& cmdBuffer : gCmdBuffers) {
        if (!cmdBuffer.init(gDevice.getCmdPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY))
            FatalErrors::raise("Failed to create a Vulkan command buffer required for rendering!");
    }

    // Initialize the texture representing PSX VRAM and clear it all to black.
    // Note: use the R16 UINT format as we will have to unpack in the shader depending on the PSX texture format being used.
    Gpu::Core& psxGpu = PsxVm::gGpu;

    if (!gPsxVramTexture.initAs2dTexture(gDevice, VK_FORMAT_R16_UINT, psxGpu.ramPixelW, psxGpu.ramPixelH)) {
        FatalErrors::raise(
            "Failed to create a Vulkan texture for PSX VRAM!\n"
            "Set 'DisableVulkanRenderer' to '1' in 'graphics_cfg.ini' as a workaround if this error continues to occur."
        );
    }

    {
        std::byte* const pBytes = gPsxVramTexture.lock();
        std::memset(pBytes, 0, gPsxVramTexture.getLockedSizeInBytes());
        gPsxVramTexture.unlock();
    }

    // Initialize the draw command submission module, crossfader and loading plaque drawer
    VDrawing::init(gDevice, gPsxVramTexture);
    VCrossfader::init(gDevice);
    VPlaqueDrawer::init(gDevice);

    // Set the initial render path and make it active.
    if (PlayerPrefs::shouldStartupWithVulkanRenderer()) {
        gpCurRenderPath = &gRenderPath_Main;
        gpNextRenderPath = &gRenderPath_Main;
    } else {
        gpCurRenderPath = &gRenderPath_Psx;
        gpNextRenderPath = &gRenderPath_Psx;
    }

    // Try to init the swapchain and framebuffers
    ensureValidSwapchainAndFramebuffers();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void destroy() noexcept {
    // Finish up the current frame if we begun it
    if (gbDidBeginFrame) {
        endFrame();
    }

    // Wait for the device to finish before proceeding
    if (gDevice.isValid()) {
        gDevice.waitUntilDeviceIdle();
    }

    // Tear everything down and make sure to destroy immediate where we have the option
    VPlaqueDrawer::destroy();
    VCrossfader::destroy();
    VDrawing::shutdown();

    gbDidAcquireSwapImageThisFrame = false;
    gbSkipNextFramePresent = false;
    gpNextRenderPath = nullptr;
    gpCurRenderPath = nullptr;
    gPsxVramTexture.destroy(true);

    for (vgl::CmdBuffer& cmdBuffer : gCmdBuffers) {
        cmdBuffer.destroy(true);
    }

    for (vgl::Semaphore& semaphore : gRenderDoneSemaphores) {
        semaphore.destroy();
    }
    
    gCurSwapchainSemaphoreIdx = 0;

    for (vgl::Semaphore& semaphore : gSwapImageReadySemaphores) {
        semaphore.destroy();
    }

    gRenderPath_Crossfade.destroy();
    gRenderPath_Main.destroy();
    gRenderPath_Psx.destroy();

    VPipelines::shutdown();
    gSwapchain.destroy();
    gDevice.destroy();
    gDrawSampleCount = 0;
    gpPhysicalDevice = nullptr;
    gWindowSurface.destroy();
    gVulkanInstance.destroy();
    gbCanVulkanFbUse16BitColor = false;
    gbCanPsxFbUse16BitColor = false;
    gVkFuncs = {};

    // Clear all coord sys info
    updateCoordSysInfo();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform tasks that need to be done on frame startup.
// Returns 'true' if a frame was successfully started and a draw can proceed, 'false' if no drawing should take place.
// Alternatively 'isRendering()' can be queried at any time to tell if drawing can take place.
//------------------------------------------------------------------------------------------------------------------------------------------
bool beginFrame() noexcept {
    // Must have ended the frame or not started one previously
    ASSERT(!gbDidBeginFrame);
    gbDidBeginFrame = true;

    // Do a render path switch if requested
    gpCurRenderPath = gpNextRenderPath;

    // Recreate the swapchain and framebuffers if required and bail if that operation failed
    if (!ensureValidSwapchainAndFramebuffers())
        return false;

    // Acquire a swapchain image and bail if that failed.
    // Note: we might already have an image if we skipped presenting a frame last time around.
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    uint32_t swapchainIdx;
    
    if (gSwapchain.getAcquiredImageIdx() == vgl::Swapchain::INVALID_IMAGE_IDX) {
        swapchainIdx = gSwapchain.acquireImage(gSwapImageReadySemaphores[gCurSwapchainSemaphoreIdx]);
        gbDidAcquireSwapImageThisFrame = (swapchainIdx != vgl::Swapchain::INVALID_IMAGE_IDX);
    } else {
        swapchainIdx = gSwapchain.getAcquiredImageIdx();
        gbDidAcquireSwapImageThisFrame = false;
    }

    if (swapchainIdx == vgl::Swapchain::INVALID_IMAGE_IDX)
        return false;

    // Begin recording the command buffer for this frame
    gCmdBufferRec.beginPrimaryCmdBuffer(gCmdBuffers[ringbufferIdx], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // The first command is to wait for any transfers to finish
    {
        VkMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        gCmdBufferRec.addPipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            1,
            &barrier,
            0,
            nullptr
        );
    }

    // Render path specific frame start
    gpCurRenderPath->beginFrame(gSwapchain, gCmdBufferRec);
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a render path is active for this frame and whether we are actually going to do some drawing.
// This will be the case if we have a valid swapchain and render path framebuffers and have successfully started a frame.
//------------------------------------------------------------------------------------------------------------------------------------------
bool isRendering() noexcept {
    return gCmdBufferRec.isRecording();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// End the current frame and present to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void endFrame() noexcept {
    // Must have begun the frame
    ASSERT(gbDidBeginFrame);
    gbDidBeginFrame = false;

    // Render path specific frame end, if we are rendering; note: may cause transfer tasks to be kicked off...
    if (isRendering()) {
        // MacOS: if the window has been resized just before we present, then skip the frame.
        // Otherwise Metal errors will occur and the GPU driver will start causing issues.
        #if __APPLE__
            if (isSwapchainOutOfDate()) {
                skipNextFramePresent();
            }
        #endif
    
        // Finish up the frame for the render path
        gpCurRenderPath->endFrame(gSwapchain, gCmdBufferRec);
    }

    // Begin executing any pending transfers
    vgl::TransferMgr& transferMgr = gDevice.getTransferMgr();
    transferMgr.executePreFrameTransferTask();

    // If we are not rendering a frame then simply wait until transfers have finished (device idle) and exit
    if (!isRendering()) {
        gDevice.waitUntilDeviceIdle();
        return;
    }

    // End command recording and submit the command buffer to the device.
    // Wait for the current swapchain image to be acquired before executing this command buffer.
    // Signal the current ringbuffer slot fence when drawing is done.
    gCmdBufferRec.endCmdBuffer();
    
    vgl::RingbufferMgr& ringbufferMgr = gDevice.getRingbufferMgr();
    const uint32_t ringbufferIdx = ringbufferMgr.getBufferIndex();
    
    {
        // Conditions that the command buffer waits on.
        // Just wait on the swap chain image to be acquired, unless we didn't actually have to acquire one this frame.
        // Sometimes we might have just had a swap image due to a previous frame that wasn't presented...
        std::vector<vgl::CmdBufferWaitCond> cmdBufferWaitConds;

        if (gbDidAcquireSwapImageThisFrame) {
            vgl::CmdBufferWaitCond& cmdsWaitCond = cmdBufferWaitConds.emplace_back();
            cmdsWaitCond.pSemaphore = &gSwapImageReadySemaphores[gCurSwapchainSemaphoreIdx];
            cmdsWaitCond.blockedStageFlags = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        }

        // Note: signal the render done semaphore and the ringbuffer slot fence when finished.
        // Skip signalling the render semaphore however if we are not presenting, as the swapchain will not be able to consume it.
        // It needs to be in an unsignalled state the next time we go to use it...
        vgl::Fence& ringbufferSlotFence = ringbufferMgr.getCurrentBufferFence();
        vgl::Semaphore* pSignalSemaphore = (gbSkipNextFramePresent) ? nullptr : &gRenderDoneSemaphores[ringbufferIdx];

        gDevice.submitCmdBuffer(
            gCmdBuffers[ringbufferIdx],
            cmdBufferWaitConds,
            pSignalSemaphore,
            &ringbufferSlotFence
        );
    }

    // Present the current swapchain image once all the commands have finished, unless skip was requested.
    // Also use a different semaphore (if presenting) for the next frame.
    if (!gbSkipNextFramePresent) {
        gSwapchain.presentAcquiredImage(gRenderDoneSemaphores[ringbufferIdx]);
        gCurSwapchainSemaphoreIdx ^= 1;
    } else {
        // Skipping showing this frame? Don't bother presenting, and wait for all commands to finish
        gDevice.waitUntilDeviceIdle();
        gbSkipNextFramePresent = false;
    }

    // Move onto the next ringbuffer index and clear the command buffer used: will get it again once we begin a frame.
    gbDidAcquireSwapImageThisFrame = false;
    ringbufferMgr.acquireNextBuffer();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies a rectangular area of pixels (of at least 1x1 pixels) from the PSX GPU's VRAM to the Vulkan texture that mirrors it.
// This makes updates to PSX VRAM visible to the new native Vulkan renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
void pushPsxVramUpdates(const uint16_t rectLx, const uint16_t rectRx, const uint16_t rectTy, const uint16_t rectBy) noexcept {
    // Sanity check the rectangle bounds.
    // Note that the rect IS allowed to wraparound to the other side of VRAM, but the coordinates must remain within range.
    // Wraparound is allowed because PSX VRAM writes wrap and LIBGPU supports that.
    Gpu::Core& psxGpu = PsxVm::gGpu;
    const uint32_t vramW = psxGpu.ramPixelW;
    const uint32_t vramH = psxGpu.ramPixelH;

    ASSERT(rectLx < vramW);
    ASSERT(rectRx < vramW);
    ASSERT(rectTy < vramH);
    ASSERT(rectBy < vramH);

    // Does the update rect wrap horizontally?
    // If so then split up into two non-wrapping rectangles.
    if (rectRx < rectLx) {
        pushPsxVramUpdates(0, rectRx, rectTy, rectBy);
        pushPsxVramUpdates(rectLx, (uint16_t)(vramW - 1), rectTy, rectBy);
        return;
    }

    // Does the update rect wrap vertically?
    // If so then split up into two non-wrapping rectangles.
    if (rectBy < rectTy) {
        pushPsxVramUpdates(rectLx, rectRx, 0, rectBy);
        pushPsxVramUpdates(rectLx, rectRx, rectTy, (uint16_t)(vramH - 1));
        return;
    }

    // This is the size of the area to be updated and the number of bytes in each row
    const uint32_t copyRectW = (uint32_t) rectRx + 1 - rectLx;
    const uint32_t copyRectH = (uint32_t) rectBy + 1 - rectTy;
    const uint32_t copyRowSize = copyRectW * sizeof(uint16_t);

    // Lock the required region of the texture and copy in the updates, row by row
    uint16_t* pDstBytes = (uint16_t*) gPsxVramTexture.lock(rectLx, rectTy, 0, 0, copyRectW, copyRectH, 1, 1);
    const uint16_t* pSrcBytes = psxGpu.pRam + rectLx + ((uintptr_t) rectTy * vramW);

    for (uint32_t row = 0; row < copyRectH; ++row) {
        std::memcpy(pDstBytes, pSrcBytes, copyRowSize);
        pDstBytes += copyRectW;
        pSrcBytes += vramW;
    }

    // Unlock the texture to begin uploading the updates
    gPsxVramTexture.unlock();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Util function: initializes uniform fields in 'VShaderUniforms_Draw' that are sourced from this renderer module.
// Used to avoid repeating the same code everywhere.
//------------------------------------------------------------------------------------------------------------------------------------------
void initRendererUniformFields(VShaderUniforms_Draw& uniforms) noexcept {
    uniforms.ndcToPsxScaleX = gNdcToPsxScaleX;
    uniforms.ndcToPsxScaleY = gNdcToPsxScaleY;
    uniforms.psxNdcOffsetX = gPsxNdcOffsetX;
    uniforms.psxNdcOffsetY = gPsxNdcOffsetY;
    uniforms.invDrawResX = gInvFramebufferW;
    uniforms.invDrawResY = gInvFramebufferH;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the render path which is currently active for this frame
//------------------------------------------------------------------------------------------------------------------------------------------
IVRendererPath& getActiveRenderPath() noexcept {
    ASSERT(gpCurRenderPath);
    return *gpCurRenderPath;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the render path which will be active in the next frame
//------------------------------------------------------------------------------------------------------------------------------------------
IVRendererPath& getNextRenderPath() noexcept {
    ASSERT(gpNextRenderPath);
    return *gpNextRenderPath;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the render path which will be active in the next frame
//------------------------------------------------------------------------------------------------------------------------------------------
void setNextRenderPath(IVRendererPath& renderPath) noexcept {
    gpNextRenderPath = &renderPath;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if the classic PSX render path is the current render path being used
//------------------------------------------------------------------------------------------------------------------------------------------
bool isUsingPsxRenderPath() noexcept {
    return (gpCurRenderPath == &gRenderPath_Psx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helpers to switch between the PSX and main Vulkan render paths.
// The switch happens at the beginning of the next frame.
//------------------------------------------------------------------------------------------------------------------------------------------
void switchToPsxRenderPath() noexcept {
    setNextRenderPath(gRenderPath_Psx);
}

void switchToMainVulkanRenderPath() noexcept {
    setNextRenderPath(gRenderPath_Main);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Requests that the current frame being rendered (or the next frame that WILL be rendered) NOT be presented.
// This is useful in some situations such as rendering crossfades.
//------------------------------------------------------------------------------------------------------------------------------------------
void skipNextFramePresent() noexcept {
    gbSkipNextFramePresent = true;
}

bool willSkipNextFramePresent() noexcept {
    return gbSkipNextFramePresent;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the swapchain size is out of date and thus whether it needs to be recreated
//------------------------------------------------------------------------------------------------------------------------------------------
bool isSwapchainOutOfDate() noexcept {
    // We're always out of date if we have an invalid swapchain or window surface
    if ((!gSwapchain.isValid()) || (!gWindowSurface.isValid()))
        return true;

    // See if the size is out of date
    int winW = {}, winH = {};
    SDL_Vulkan_GetDrawableSize(gWindowSurface.getSdlWindow(), &winW, &winH);
    return ((winW != (int) gSwapchain.getSwapExtentWidth()) || (winH != (int) gSwapchain.getSwapExtentHeight()));
}

END_NAMESPACE(VRenderer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
