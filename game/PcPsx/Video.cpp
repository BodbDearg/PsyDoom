//------------------------------------------------------------------------------------------------------------------------------------------
// Display setup and manipulation
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Video.h"

#include "Asserts.h"
#include "Config.h"
#include "Gpu.h"
#include "ProgArgs.h"
#include "PsxVm.h"
#include "Utils.h"

#include "VideoBackend_Vulkan.h"
#include "VideoBackend_SDL.h"

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

SDL_Window*     gpSdlWindow;        // The SDL window being used
BackendType     gBackendType;       // Which type of video backend is in use

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

    // If in fullscreen then use the current screen resolution
    if (Config::gbFullscreen) {
        w = displayMode.w;
        h = displayMode.h;
        return;
    }

    // In windowed mode make the window a multiple of the logical display resolution.
    // Also allow some room for window edges and other OS decoration...
    // If a free aspect ratio is being used (width <= 0) then just use the original display resolution width for this calculation.
    const int32_t logicalDispW = (Config::gLogicalDisplayW <= 0) ? ORIG_DISP_RES_X : Config::gLogicalDisplayW;

    int32_t xMultiplier = std::max((displayMode.w - 20) / logicalDispW, 1);
    int32_t yMultiplier = std::max((displayMode.h - 40) / ORIG_DISP_RES_Y, 1);
    int32_t multiplier = std::min(xMultiplier, yMultiplier);

    w = logicalDispW * multiplier;
    h = ORIG_DISP_RES_Y * multiplier;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: determines which video backend to use (requires a window to do so)
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineVideoBackend() noexcept {
    #if PSYDOOM_VULKAN_RENDERER
        if (VideoBackend_Vulkan::isBackendSupported()) {
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
// Display the currently renderered PSX GPU frame to the screen; this is used by the classic renderer
//------------------------------------------------------------------------------------------------------------------------------------------
void displayFramebuffer() noexcept {
    // Ignore call in headless mode otherwise handle and ensure the window is updated after we do the swap
    if (ProgArgs::gbHeadlessMode)
        return;

    gpVideoBackend->displayFramebuffer();
    Utils::doPlatformUpdates();
}

END_NAMESPACE(Video)
