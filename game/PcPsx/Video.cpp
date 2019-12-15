#include "Video.h"

#define PSX_VM_NO_REGISTER_MACROS 1
#include "PsxVm/PsxVm.h"

// Disabling certain Avocado warnings for MSVC
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4201)
    #pragma warning(disable: 4244)
#endif

#include <device/gpu/gpu.h>

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#include "Macros.h"
#include <SDL.h>
#include <thread>

BEGIN_NAMESPACE(PcPsx)

static SDL_Window*      gWindow;
static SDL_Renderer*    gRenderer;
static SDL_Texture*     gFramebufferTexture;
static SDL_Rect         gOutputRect;
static uint32_t*        gpFrameBuffer;
static uint32_t         gLastFrameTickCount;

static void lockFramebufferTexture() noexcept {
    int pitch = 0;
    if (SDL_LockTexture(gFramebufferTexture, nullptr, reinterpret_cast<void**>(&gpFrameBuffer), &pitch) != 0) {
        FATAL_ERROR("Failed to lock the framebuffer texture for writing!");
    }
}

static void unlockFramebufferTexture() noexcept {
    SDL_UnlockTexture(gFramebufferTexture);
}

static void presentSdlFramebuffer() noexcept {
    unlockFramebufferTexture();

    gOutputRect.x = 0;
    gOutputRect.y = 0;
    gOutputRect.w = 1280;
    gOutputRect.h = 1200;
    SDL_RenderCopy(gRenderer, gFramebufferTexture, nullptr, &gOutputRect);

    SDL_RenderPresent(gRenderer);
    lockFramebufferTexture();
}

static void copyPsxToSdlFramebuffer() noexcept {
    gpu::GPU& gpu = *PsxVm::gpGpu;
    const uint16_t* const vramPixels = gpu.vram.data();
    uint32_t* pDstPixel = gpFrameBuffer;

    for (uint32_t y = 0; y < 240; ++y) {
        const uint16_t* const rowPixels = vramPixels + (intptr_t) y * 1024;
        const uint32_t xStart = (uint32_t) gpu.displayAreaStartX;
        const uint32_t xEnd = xStart + 256;
        ASSERT(xEnd <= 512);

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

static void handleSdlWindowEvents() noexcept {
    // TODO: handle events and maybe move this elsewhere
    SDL_Event event = {};    
    while (SDL_PollEvent(&event) != 0);
}

static void doFrameRateLimiting() noexcept {
    const uint32_t startTickCount = SDL_GetTicks();

    while (true) {
        const uint32_t tickCount = SDL_GetTicks();
        const uint32_t oneFrameTickCount = 1000 / 30;
        const uint32_t elapsedTicks = tickCount - gLastFrameTickCount;

        if (elapsedTicks < oneFrameTickCount - 1) {
            std::this_thread::yield();
        } else {
            gLastFrameTickCount = tickCount;
            break;
        }
    }

    // Simulate the amount of emulator frames we waited for
    const uint32_t numEmulatorFramesPassed = (gLastFrameTickCount - startTickCount) / 60;

    for (uint32_t i = 0; i < numEmulatorFramesPassed; ++i) {
        emulate_frame();
    }
}

void initVideo() noexcept {
    // Initialize SDL subsystems
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        FATAL_ERROR("Unable to initialize SDL!");
    }

    // TODO: Provide a way to switch between fullscreen and back...
    Uint32 windowCreateFlags = 0;

    #ifndef __MACOSX__
        windowCreateFlags |= SDL_WINDOW_OPENGL;
    #endif

    gWindow = SDL_CreateWindow(
        "PC-PSX DOOM",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        (int32_t) 1280,
        (int32_t) 1200,
        windowCreateFlags
    );

    if (!gWindow) {
        FATAL_ERROR("Unable to create a window!");
    }

    // Create the renderer and framebuffer texture
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!gRenderer) {
        FATAL_ERROR("Failed to create renderer!");
    }

    gFramebufferTexture = SDL_CreateTexture(
        gRenderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        (int32_t) 256,
        (int32_t) 240
    );

    if (!gFramebufferTexture) {
        FATAL_ERROR("Failed to create a framebuffer texture!");
    }

    // Clear the renderer to black
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
    SDL_RenderClear(gRenderer);

    // Immediately lock the framebuffer texture for updating
    lockFramebufferTexture();

    // TODO: re-enable this later once mouse input works
    #if 0
        // Grab input and hide the cursor
        SDL_SetWindowGrab(gWindow, SDL_TRUE);
        SDL_ShowCursor(SDL_DISABLE);
    #endif
}

void shutdownVideo() noexcept {
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
    handleSdlWindowEvents();
    PsxVm::updateInput();
    copyPsxToSdlFramebuffer();
    presentSdlFramebuffer();
    doFrameRateLimiting();
}

SDL_Window* getWindow() noexcept {
    return gWindow;
}

END_NAMESPACE(PcPsx)
