#pragma once

#include <cstdint>

struct texture_t;

// Texture cache page dimensions
static constexpr uint32_t TCACHE_PAGE_SIZE = 256;   // Square size of a texture page, 256x256 pixels
static constexpr uint32_t TCACHE_CELL_SIZE = 16;    // Square size of a cell in a texture page, 16x16 pixels

// Numer of texture cache cells in the x and y direction per page, and total cells per page
static constexpr uint32_t TCACHE_CELLS_X        = TCACHE_PAGE_SIZE / TCACHE_CELL_SIZE;
static constexpr uint32_t TCACHE_CELLS_Y        = TCACHE_PAGE_SIZE / TCACHE_CELL_SIZE;
static constexpr uint32_t NUM_TCACHE_PAGE_CELLS = TCACHE_CELLS_X * TCACHE_CELLS_Y;

#if PSYDOOM_MODS
    void I_InitTexCache() noexcept;
    uint32_t I_GetNumTexCachePages() noexcept;
    uint32_t I_GetCurTexCacheFillPage() noexcept;
    void I_SetTexCacheFillPage(const uint32_t pageIdx) noexcept;
    void I_LockTexCachePage(const uint32_t pageIdx) noexcept;
    void I_UnlockAllTexCachePages() noexcept;
#endif

void I_CacheTex(texture_t& tex) noexcept;
void I_RemoveTexCacheEntry(texture_t& tex) noexcept;
void I_PurgeTexCache() noexcept;
void I_VramViewerDraw(const uint32_t texPageIdx) noexcept;
