#pragma once

#include "Macros.h"

#include <cstdint>

struct SDL_Window;

BEGIN_NAMESPACE(Video)

// The original render/draw and output/display resolution of the game: the game rendered to a 256x240 framebuffer but stretched this image to
// approximately 292.57x240 in square pixel terms - even though the game asks for a 320x200 'pixel' display (CRTs did not have pixels).
//
// For more info on PS1, NES and PSX Doom pixel aspect ratios, see:
//  https://github.com/libretro/beetle-psx-libretro/issues/510
//  http://forums.nesdev.com/viewtopic.php?t=8983
//  https://doomwiki.org/wiki/Sony_PlayStation
//
static constexpr int32_t ORIG_DRAW_RES_X = 256;
static constexpr int32_t ORIG_DRAW_RES_Y = 240;
static constexpr int32_t ORIG_DISP_RES_X = 292;
static constexpr int32_t ORIG_DISP_RES_Y = 240;

// Which video backend is being used
enum BackendType {
    SDL,        // Using an SDL backend which only supports the classic renderer
    Vulkan      // Using a Vulkan backend which supports both the classic renderer and the new Vulkan renderer
};

extern BackendType  gBackendType;
extern SDL_Window*  gpSdlWindow;

void initVideo() noexcept;
void shutdownVideo() noexcept;

void getClassicFramebufferWindowRect(
    const uint32_t windowW,
    const uint32_t windowH,
    int32_t& rectX,
    int32_t& rectY,
    uint32_t& rectW,
    uint32_t& rectH
) noexcept;

void displayFramebuffer() noexcept;
bool isUsingVulkanRenderPath() noexcept;

END_NAMESPACE(Video)
