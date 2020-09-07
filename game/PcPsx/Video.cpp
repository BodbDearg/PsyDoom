//------------------------------------------------------------------------------------------------------------------------------------------
// Display setup and manipulation
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Video.h"

#include "Asserts.h"
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

// The original render/draw and output/display resolution of the game: the game rendered to a 256x240 framebuffer but stretched this image to
// approximately 292.57x240 in square pixel terms - even though the game asks for a 320x200 'pixel' display (CRTs did not have pixels).
//
// For more info on PS1, NES and PSX Doom pixel aspect ratios, see:
//  https://github.com/libretro/beetle-psx-libretro/issues/510
//  http://forums.nesdev.com/viewtopic.php?t=8983
//  https://doomwiki.org/wiki/Sony_PlayStation
//
constexpr int32_t ORIG_DRAW_RES_X = 256;
constexpr int32_t ORIG_DRAW_RES_Y = 240;
constexpr int32_t ORIG_DISP_RES_X = 292;
constexpr int32_t ORIG_DISP_RES_Y = 240;

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

    // Are we using a free aspect ratio mode, specified by using a logical display width of <= 0?
    // If so then just stretch the output image in any way to fill the window.
    if (Config::gLogicalDisplayW <= 0) {
        gOutputRect.x = 0;
        gOutputRect.y = 0;
        gOutputRect.w = winSizeX;
        gOutputRect.h = winSizeY;
    }
    else {
        // If not using a free aspect ratio then determine the scale to output at, while preserving the chosen aspect ratio.
        // The chosen aspect ratio is determined by the user's logical display resolution width.
        const float xScale = (float) winSizeX / (float) Config::gLogicalDisplayW;
        const float yScale = (float) winSizeY / (float) ORIG_DISP_RES_Y;
        const float scale = std::min(xScale, yScale);

        // Determine output width and height and center the framebuffer image in the window
        gOutputRect.w = (int32_t)((float) Config::gLogicalDisplayW * scale);
        gOutputRect.h = (int32_t)((float) ORIG_DISP_RES_Y * scale);
        gOutputRect.x = winSizeX / 2 - gOutputRect.w / 2;
        gOutputRect.y = winSizeY / 2 - gOutputRect.h / 2;
    }

    // Need to clear the window if we are not filling the whole screen.
    // Some stuff like NVIDIA video recording notifications can leave marks in the unused regions otherwise...
    if ((gOutputRect.w != winSizeX) || (gOutputRect.h != winSizeY)) {
        SDL_RenderClear(gRenderer);
    }
    
    // Blit the framebuffer to the display
    SDL_RenderCopy(gRenderer, gFramebufferTexture, nullptr, &gOutputRect);

    // Present the rendered frame and re-lock the framebuffer texture
    SDL_RenderPresent(gRenderer);
    lockFramebufferTexture();
}

static void copyPsxToSdlFramebuffer() noexcept {
    gpu::GPU& gpu = *PsxVm::gpGpu;
    const uint16_t* const vramPixels = gpu.vram.data();
    uint32_t* pDstPixel = gpFrameBuffer;
    
    for (uint32_t y = 0; y < ORIG_DRAW_RES_Y; ++y) {
        const uint16_t* const rowPixels = vramPixels + ((intptr_t) y + gpu.displayAreaStartY) * 1024;   // Note: VRAM size is '1024x512 @ 16 bpp'
        const uint32_t xStart = (uint32_t) gpu.displayAreaStartX;
        const uint32_t xEnd = xStart + ORIG_DRAW_RES_X;
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
    Uint32 windowCreateFlags = (
        SDL_WINDOW_MOUSE_FOCUS |
        SDL_WINDOW_INPUT_FOCUS |
        SDL_WINDOW_INPUT_GRABBED |
        SDL_WINDOW_MOUSE_CAPTURE
    );
    
    windowCreateFlags |= (Config::gbFullscreen) ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;
    
    #if !__APPLE__
        windowCreateFlags |= SDL_WINDOW_OPENGL;
    #endif
    
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
        (int32_t) ORIG_DRAW_RES_X,
        (int32_t) ORIG_DRAW_RES_Y
    );

    if (!gFramebufferTexture) {
        FatalErrors::raise("Failed to create a framebuffer texture!");
    }

    // Clear the renderer to black
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
    SDL_RenderClear(gRenderer);

    // Immediately lock the framebuffer texture for updating
    lockFramebufferTexture();

    // Hide the cursor and switch to relative input mode.
    //
    // Note: added the set focus and warp mouse calls for MacOS to prevent a strange freezing issue on pressing the
    // left mouse button after launch. I believe that issue was due to the mouse cursor being at the edge of the window.
    // Pressing the left mouse in this situation freezes the app since the OS belives it is resizing.
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowInputFocus(gWindow);
    SDL_WarpMouseInWindow(gWindow, winSizeX / 2, winSizeY / 2);
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

    // Otherwise handle and ensure the window is updated after we do the swap
    copyPsxToSdlFramebuffer();
    presentSdlFramebuffer();
    Utils::doPlatformUpdates();
}

SDL_Window* getWindow() noexcept {
    return gWindow;
}

END_NAMESPACE(Video)
