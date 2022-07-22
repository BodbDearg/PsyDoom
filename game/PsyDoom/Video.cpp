//------------------------------------------------------------------------------------------------------------------------------------------
// Display setup and manipulation
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Video.h"

#include "Asserts.h"
#include "Config/Config.h"
#include "Gpu.h"
#include "ProgArgs.h"
#include "PsxVm.h"
#include "Utils.h"
#include "VideoBackend_SDL.h"
#include "VideoBackend_Vulkan.h"
#include "Vulkan/VRenderer.h"

#include <algorithm>
#include <SDL.h>

BEGIN_NAMESPACE(Video)

// Standard SDL video backend that only supports the classic renderer
static VideoBackend_SDL gVideoBackend_SDL;

#if PSYDOOM_VULKAN_RENDERER
    // Vulkan video backend that can do the classic renderer plus a new hardware accelerated Vulkan renderer
    static VideoBackend_Vulkan gVideoBackend_Vulkan;
#endif

// Pointer to the video backend implementation in use
static IVideoBackend* gpVideoBackend;

SDL_Window*     gpSdlWindow;    // The SDL window being used
BackendType     gBackendType;   // Which type of video backend is in use
int32_t         gTopOverscan;   // Sanitized config input: number of pixels to discard at the top of the screen in terms of the original 256x240 framebuffer
int32_t         gBotOverscan;   // Sanitized config input: number of pixels to discard at the bottom of the screen in terms of the original 256x240 framebuffer

//------------------------------------------------------------------------------------------------------------------------------------------
// Pick which resolution to use for the game's window on startup
//------------------------------------------------------------------------------------------------------------------------------------------
static void decideStartupResolution(int32_t& w, int32_t& h) noexcept {
    // Get the screen resolution.
    // On high DPI screens like MacOS retina the resolution returned will be virtual not physical resolution.
    SDL_DisplayMode displayMode;

    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        FatalErrors::raise("Failed to determine current screen video mode!");
    }

    // Determine automatically decided resolution
    int32_t autoResolutionW = {};
    int32_t autoResolutionH = {};

    if (Config::gbFullscreen) {
        // If in fullscreen then use the current screen resolution
        autoResolutionW = displayMode.w;
        autoResolutionH = displayMode.h;
    }
    else {
        // In windowed mode make the window a multiple of the logical display resolution.
        // Also allow some room for window edges and other OS decoration...
        // If a free aspect ratio is being used (width <= 0) then just use the original display resolution width for this calculation.
        const float logicalDispW = (Config::gLogicalDisplayW <= 0.0f) ? (float) ORIG_DISP_RES_X : Config::gLogicalDisplayW;

        const float xScale = std::max(((float) displayMode.w - 20.0f) / logicalDispW, 1.0f);
        const float yScale = std::max(((float) displayMode.h - 40.0f) / (float) ORIG_DISP_RES_Y, 1.0f);
        const int32_t scale = std::max((int32_t) std::min(xScale, yScale), 1);

        autoResolutionW = (int32_t)(logicalDispW * scale);
        autoResolutionH = (int32_t)((float) ORIG_DISP_RES_Y * scale);
    }

    // Save the actual resolution to use, taking into account user overrides
    w = (Config::gOutputResolutionW > 0) ? Config::gOutputResolutionW : autoResolutionW;
    h = (Config::gOutputResolutionH > 0) ? Config::gOutputResolutionH : autoResolutionH;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: determines which video backend to use (requires a window to do so)
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineVideoBackend() noexcept {
    #if PSYDOOM_VULKAN_RENDERER
        if ((!Config::gbDisableVulkanRenderer) && VideoBackend_Vulkan::isBackendSupported()) {
            gpVideoBackend = &gVideoBackend_Vulkan;
            gBackendType = BackendType::Vulkan;
            return;
        }
    #endif

    gpVideoBackend = &gVideoBackend_SDL;
    gBackendType = BackendType::SDL;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: determines the SDL window creation flags
//------------------------------------------------------------------------------------------------------------------------------------------
static Uint32 getSdlWindowCreateFlags() noexcept {
    ASSERT(gpVideoBackend);

    Uint32 windowCreateFlags = (
        SDL_WINDOW_MOUSE_FOCUS |
        SDL_WINDOW_INPUT_FOCUS |
        SDL_WINDOW_INPUT_GRABBED |
        SDL_WINDOW_MOUSE_CAPTURE
    );

    windowCreateFlags |= (Config::gbFullscreen) ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;
    windowCreateFlags |= gpVideoBackend->getSdlWindowCreateFlags();
    return windowCreateFlags;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the renderering API and creates the main game window
//------------------------------------------------------------------------------------------------------------------------------------------
void initVideo() noexcept {
    // Ignore call in headless mode
    if (ProgArgs::gbHeadlessMode)
        return;

    // Initialize SDL subsystems and determine the video backend (one must always be chosen)
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        FatalErrors::raise("Unable to initialize SDL!");
    }

    determineVideoBackend();
    ASSERT(gpVideoBackend);

    // This is the window title to use
    #ifdef GAME_VERSION_STR
        constexpr const char* gameVersionStr = "PsyDoom " GAME_VERSION_STR;
    #else
        constexpr const char* gameVersionStr = "PsyDoom <UNKNOWN_VERSION>";
    #endif

    // Set and sanitize overscan settings
    gTopOverscan = std::clamp(Config::gTopOverscanPixels, 0, ORIG_DRAW_RES_Y / 2 - 1);
    gBotOverscan = std::clamp(Config::gBottomOverscanPixels, 0, ORIG_DRAW_RES_Y / 2 - 1);

    // Decide what window size to use
    int32_t winSizeX = 0;
    int32_t winSizeY = 0;
    decideStartupResolution(winSizeX, winSizeY);

    // Create the window
    const int32_t windowX = SDL_WINDOWPOS_CENTERED;
    const int32_t windowY = SDL_WINDOWPOS_CENTERED;

    gpSdlWindow = SDL_CreateWindow(
        gameVersionStr,
        windowX,
        windowY,
        winSizeX,
        winSizeY,
        getSdlWindowCreateFlags()
    );

    if (!gpSdlWindow) {
        FatalErrors::raise("Unable to create a window!");
    }

    // Initialize the video backend
    gpVideoBackend->initRenderers(gpSdlWindow);

    // Hide the cursor and switch to relative input mode.
    //
    // Note: added the set focus and warp mouse calls for MacOS to prevent a strange freezing issue on pressing the
    // left mouse button after launch. I believe that issue was due to the mouse cursor being at the edge of the window.
    // Pressing the left mouse in this situation freezes the app since the OS belives it is resizing.
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowInputFocus(gpSdlWindow);
    SDL_WarpMouseInWindow(gpSdlWindow, winSizeX / 2, winSizeY / 2);
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the main game window and tears down rendering APIs
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdownVideo() noexcept {
    // Ignore call in headless mode
    if (ProgArgs::gbHeadlessMode)
        return;

    // Cleanup for the video backend
    if (gpVideoBackend) {
        gpVideoBackend->destroyRenderers();
        gpVideoBackend = nullptr;
    }

    gBackendType = {};

    // Destroy the SDL window and shutdown all SDL video
    if (gpSdlWindow) {
        SDL_SetWindowGrab(gpSdlWindow, SDL_FALSE);
        SDL_DestroyWindow(gpSdlWindow);
        gpSdlWindow = nullptr;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current window size in pixels (NOT points!)
//------------------------------------------------------------------------------------------------------------------------------------------
void getWindowSizeInPixels(uint32_t& width, uint32_t& height) noexcept {
    // Get the SDL renderer associated with the window and from that get the pixel size
    SDL_Renderer* const pRenderer = (gpSdlWindow) ? SDL_GetRenderer(gpSdlWindow) : nullptr;
    int sdlWidth = 0;
    int sdlHeight = 0;

    if (pRenderer) {
        SDL_GetRendererOutputSize(pRenderer, &sdlWidth, &sdlHeight);
    }

    width = sdlWidth;
    height = sdlHeight;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// This function determines where in the window the framebuffer for the classic PSX renderer should be output to.
// Considers the current window size (specified in pixels) and user scaling settings.
//------------------------------------------------------------------------------------------------------------------------------------------
void getClassicFramebufferWindowRect(
    const float windowW,
    const float windowH,
    float& rectX,
    float& rectY,
    float& rectW,
    float& rectH
) noexcept {
    //If the window size is zero then make the output rect zero also
    if ((windowW <= 0) || (windowH <= 0)) {
        rectX = 0;
        rectY = 0;
        rectW = 0;
        rectH = 0;
        return;
    }

    // Are we using a free aspect ratio mode, specified by using a logical display width of <= 0?
    // If so then just stretch the output image in any way to fill the window.
    if (Config::gLogicalDisplayW <= 0.0f) {
        rectX = 0;
        rectY = 0;
        rectW = windowW;
        rectH = windowH;
    } else {
        // If not using a free aspect ratio then determine the scale to output at, while preserving the chosen aspect ratio.
        // The chosen aspect ratio is determined by the user's logical display resolution width.
        const float logicalXResolution = Config::gLogicalDisplayW;
        const float xScale = windowW / logicalXResolution;
        const float yScale = windowH / (float) ORIG_DISP_RES_Y;
        const float scale = std::min(xScale, yScale);

        // Determine output width and height and center the framebuffer image in the window
        rectW = logicalXResolution * scale;
        rectH = (float) ORIG_DISP_RES_Y * scale;
        rectX = (windowW - rectW) * 0.5f;
        rectY = (windowH - rectH) * 0.5f;
    }

    // Ensure the coordinates are within screen bounds.
    // Note that rectX, rectY could technically go past screen bounds if the width or height is '0', but we shouldn't draw in that case.
    rectX = std::clamp(rectX, 0.0f, windowW);
    rectY = std::clamp(rectY, 0.0f, windowH);

    if (rectX + rectW > windowW) {
        rectW = windowW - rectX;
    }

    if (rectY + rectH > windowH) {
        rectH = windowH - rectY;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Display the currently renderered PSX GPU frame to the screen; this is used by the classic renderer
//------------------------------------------------------------------------------------------------------------------------------------------
void displayFramebuffer() noexcept {
    // Ignore call in headless mode otherwise handle and ensure the window is updated after we do the swap
    if (ProgArgs::gbHeadlessMode)
        return;

    gpVideoBackend->displayFramebuffer();
    Utils::doPlatformUpdates();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if any of the render paths used by the new Vulkan renderer are currently in use.
// Note: will return 'false' if we are using a Vulkan backend but just outputting the classic PSX renderer via Vulkan.
//------------------------------------------------------------------------------------------------------------------------------------------
bool isUsingVulkanRenderPath() noexcept {
    #if PSYDOOM_VULKAN_RENDERER
        if (gBackendType == BackendType::Vulkan)
            return (!VRenderer::isUsingPsxRenderPath());    // Anything other than the PSX render path is to do with the new Vulkan renderer
    #endif

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the currently active video backend
//------------------------------------------------------------------------------------------------------------------------------------------
IVideoBackend& getCurrentBackend() noexcept {
    #if PSYDOOM_VULKAN_RENDERER
        if (gBackendType == BackendType::Vulkan)
            return gVideoBackend_Vulkan;
    #endif

    return gVideoBackend_SDL;
}

END_NAMESPACE(Video)
