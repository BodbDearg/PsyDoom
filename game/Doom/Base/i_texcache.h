#pragma once

#include <cstdint>

struct texture_t;

// Texture cache page dimensions.
// For limit removing we resize texture pages to be 1024x512 instead of 256x256, allowing for textures up to that size.
#if PSYDOOM_LIMIT_REMOVING
    static constexpr uint32_t TCACHE_PAGE_W = 1024;     // Pixel width of a texture page
    static constexpr uint32_t TCACHE_PAGE_H = 512;      // Pixel height of a texture page
#else
    static constexpr uint32_t TCACHE_PAGE_W = 256;      // Pixel width of a texture page
    static constexpr uint32_t TCACHE_PAGE_H = 256;      // Pixel height of a texture page
#endif

static constexpr uint32_t TCACHE_CELL_SIZE = 16;        // Square size of a cell in a texture page, 16x16 pixels

// Numer of texture cache cells in the x and y direction per page, and total cells per page
static constexpr uint32_t TCACHE_CELLS_X        = TCACHE_PAGE_W / TCACHE_CELL_SIZE;
static constexpr uint32_t TCACHE_CELLS_Y        = TCACHE_PAGE_H / TCACHE_CELL_SIZE;
static constexpr uint32_t NUM_TCACHE_PAGE_CELLS = TCACHE_CELLS_X * TCACHE_CELLS_Y;

#if PSYDOOM_MODS
    void I_InitTexCache() noexcept;
    uint32_t I_GetNumTexCachePages() noexcept;
    uint32_t I_GetCurTexCacheFillPage() noexcept;
    void I_SetTexCacheFillPage(const uint32_t pageIdx) noexcept;
    void I_PurgeTexCachePage(const uint32_t pageIdx) noexcept;
    
    // In limit removing builds we use a per-texture locking mechanism rather than per-page, also loose packing behavior can be controlled.
    // This finer granularity allows for better control over which areas of VRAM are reserved and how textures are packed.
    #if PSYDOOM_LIMIT_REMOVING
        void I_TexCacheUseLoosePacking(const bool bUseLoosePacking) noexcept;
        void I_LockAllWallAndFloorTextures(const bool bLock) noexcept;
    #else
        void I_LockTexCachePage(const uint32_t pageIdx) noexcept;
        void I_UnlockAllTexCachePages() noexcept;
    #endif
#endif

void I_CacheTex(texture_t& tex) noexcept;
void I_RemoveTexCacheEntry(texture_t& tex) noexcept;
void I_PurgeTexCache() noexcept;
void I_VramViewerDraw(const uint32_t texPageIdx) noexcept;
