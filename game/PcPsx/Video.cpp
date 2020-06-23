//------------------------------------------------------------------------------------------------------------------------------------------
// Display setup and manipulation
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Video.h"

#include "Assert.h"
#include "Config.h"
#include "ProgArgs.h"
#include "PsxVm.h"
#include "Utils.h"

#include <SDL.h>

BEGIN_DISABLE_HEADER_WARNINGS
    #include <device/gpu/gpu.h>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Video)

static SDL_Window*      gWindow;
static SDL_Renderer*    gRenderer;
static SDL_Texture*     gFramebufferTexture;
static SDL_Rect         gOutputRect;
static uint32_t*        gpFrameBuffer;

// TODO: this is a temporary thing.
// Note that I am using the stretched NTSC resolution here for game resolution width, not the physical framebuffer width (256 pixels wide).
// See: https://doomwiki.org/wiki/Sony_PlayStation
constexpr int32_t GAME_RES_X = 293;
constexpr int32_t GAME_RES_Y = 240;

static void decideStartupResolution(int32_t& w, int32_t& h) noexcept {
    // Get the screen resolution.
    // On high DPI screens like MacOS retina the resolution returned will be virtual not physical resolution.
    SDL_DisplayMode displayMode;

    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        FatalErrors::raise("Failed to determine current screen video mode!");
    }

    // If in fullscreen then use the current screen resolution.
    // TODO: allow the user to specify a fullscreen resolution in the config file, if required.
    if (Config::gbFullscreen) {
        w = displayMode.w;
        h = displayMode.h;
        return;
    }

    // Make the windows multiple of the original resolution.
    // Allow some room for window edges and decoration.
    int32_t xMultiplier = std::max((displayMode.w - 20) / GAME_RES_X, 1);
    int32_t yMultiplier = std::max((displayMode.h - 40) / GAME_RES_Y, 1);
    int32_t multiplier = std::min(xMultiplier, yMultiplier);

    w = GAME_RES_X * multiplier;
    h = GAME_RES_Y * multiplier;
}

static void lockFramebufferTexture() noexcept {
    int pitch = 0;

    if (SDL_LockTexture(gFramebufferTexture, nullptr, reinterpret_cast<void**>(&gpFrameBuffer), &pitch) != 0) {
        FatalErrors::raise("Failed to lock the framebuffer texture for writing!");
    }
}

static void unlockFramebufferTexture() noexcept {
    SDL_UnlockTexture(gFramebufferTexture);
}

static void presentSdlFramebuffer() noexcept {
    // Get the size of the window.
    // Don't bother outputting if the window has been made zero sized.
    int32_t winSizeX = {};
    int32_t winSizeY = {};
    SDL_GetWindowSize(gWindow, &winSizeX, &winSizeY);

    if (winSizeX <= 0 || winSizeY <= 0)
        return;

    // Grab the framebuffer texture for writing to
    unlockFramebufferTexture();

    // Determine the scale to output at preserving the original game aspect ratio
    const float xScale = (float) winSizeX / (float) GAME_RES_X;
    const float yScale = (float) winSizeY / (float) GAME_RES_Y;
    const float scale = std::min(xScale, yScale);

    // Determine output width and height and center the framebuffer image in the window
    gOutputRect.w = (int32_t)(GAME_RES_X * scale);
    gOutputRect.h = (int32_t)(GAME_RES_Y * scale);
    gOutputRect.x = winSizeX / 2 - gOutputRect.w / 2;
    gOutputRect.y = winSizeY / 2 - gOutputRect.h / 2;
    SDL_RenderCopy(gRenderer, gFramebufferTexture, nullptr, &gOutputRect);

    // Present the rendered frame and re-lock the framebuffer texture
    SDL_RenderPresent(gRenderer);
    lockFramebufferTexture();
}

static void copyPsxToSdlFramebuffer() noexcept {
    gpu::GPU& gpu = *PsxVm::gpGpu;
    const uint16_t* const vramPixels = gpu.vram.data();
    uint32_t* pDstPixel = gpFrameBuffer;

    for (uint32_t y = 0; y < 240; ++y) {
        const uint16_t* const rowPixels = vramPixels + ((intptr_t) y + gpu.displayAreaStartY) * 1024;
        const uint32_t xStart = (uint32_t) gpu.displayAreaStartX;
        const uint32_t xEnd = xStart + 256;
        ASSERT(xEnd <= 1024);

        for (uint32_t x = xStart; x < xEnd; ++x) {
            const uint16_t srcPixel = rowPixels[x];
            const uint32_t r = ((srcPixel >> 10) & 0x1F) << 3;
            const uint32_t g = ((srcPixel >> 5 ) & 0x1F) << 3;
            const uint32_t b = ((srcPixel >> 0 ) & 0x1F) << 3;
            
            *pDstPixel = (
               0xFF000000 |
               (r << 16) |
               (g << 8 ) |
               (b << 0 )
            );

            ++pDstPixel;
        }
    }
}

void initVideo() noexcept {
    // Ignore call in headless mode
    if (ProgArgs::gbHeadlessMode)
        return;

    // Initialize SDL subsystems
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        FatalErrors::raise("Unable to initialize SDL!");
    }

    // This is the window title to use
    #ifdef GAME_VERSION_STR
        constexpr const char* gameVersionStr = "PsyDoom " GAME_VERSION_STR;
    #else
        constexpr const char* gameVersionStr = "PsyDoom <UNKNOWN_VERSION>";
    #endif

    // Determine the window creation flags
    Uint32 windowCreateFlags = SDL_WINDOW_OPENGL;
    windowCreateFlags |= (Config::gbFullscreen) ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;

    // Decide what window size to use
    int32_t winSizeX = 0;
    int32_t winSizeY = 0;
    decideStartupResolution(winSizeX, winSizeY);

    // Create the window
    gWindow = SDL_CreateWindow(
        gameVersionStr,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        winSizeX,
        winSizeY,
        windowCreateFlags
    );

    if (!gWindow) {
        FatalErrors::raise("Unable to create a window!");
    }

    // Create the renderer and framebuffer texture
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!gRenderer) {
        FatalErrors::raise("Failed to create renderer!");
    }

    gFramebufferTexture = SDL_CreateTexture(
        gRenderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        (int32_t) 256,
        (int32_t) 240
    );

    if (!gFramebufferTexture) {
        FatalErrors::raise("Failed to create a framebuffer texture!");
    }

    // Clear the renderer to black
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
    SDL_RenderClear(gRenderer);

    // Immediately lock the framebuffer texture for updating
    lockFramebufferTexture();

    // Grab input, focus the window and hide the cursor.
    // Also request the mouse to be always centered in the window and mouse input to be relative.
    SDL_SetWindowGrab(gWindow, SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowInputFocus(gWindow);
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void shutdownVideo() noexcept {
    // Ignore call in headless mode
    if (ProgArgs::gbHeadlessMode)
        return;

    // Do the cleanup
    gpFrameBuffer = nullptr;
    
    if (gRenderer) {
        SDL_DestroyRenderer(gRenderer);
        gRenderer = nullptr;
    }

    if (gWindow) {
        SDL_SetWindowGrab(gWindow, SDL_FALSE);
        SDL_DestroyWindow(gWindow);
        gWindow = nullptr;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    gOutputRect = {};
}

void displayFramebuffer() noexcept {
    // Ignore call in headless mode
    if (ProgArgs::gbHeadlessMode)
        return;

    // Otherwise handle and ensure sound etc. is up to date before and after the buffer swap
    Utils::doPlatformUpdates();
    copyPsxToSdlFramebuffer();
    presentSdlFramebuffer();
    Utils::doPlatformUpdates();
}

SDL_Window* getWindow() noexcept {
    return gWindow;
}

END_NAMESPACE(Video)
