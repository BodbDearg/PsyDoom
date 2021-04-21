#pragma once

#if !PSYDOOM_MODS

#include <cstdint>

// Texture cache related constants
static constexpr uint32_t NUM_TCACHE_PAGES      = 11;
static constexpr uint32_t TCACHE_PAGE_SIZE      = 256;  // Square size: 256x256 pixels
static constexpr uint32_t TCACHE_CELL_SIZE      = 16;
static constexpr uint32_t TCACHE_CELLS_X        = TCACHE_PAGE_SIZE / TCACHE_CELL_SIZE;
static constexpr uint32_t TCACHE_CELLS_Y        = TCACHE_PAGE_SIZE / TCACHE_CELL_SIZE;
static constexpr uint32_t NUM_TCACHE_PAGE_CELLS = TCACHE_CELLS_X * TCACHE_CELLS_Y;
static constexpr uint32_t ALL_TPAGES_MASK       = (UINT32_MAX >> (32 - NUM_TCACHE_PAGES));

extern uint32_t         gTCacheFillPage;
extern uint32_t         gTCacheFillCellX;
extern uint32_t         gTCacheFillCellY;
extern uint32_t         gTCacheFillRowCellH;
extern uint32_t         gLockedTexPagesMask;
extern padbuttons_t*    gpPlayerCtrlBindings[MAXPLAYERS];

void I_CacheTex(texture_t& tex) noexcept;
void I_RemoveTexCacheEntry(texture_t& tex) noexcept;
void I_PurgeTexCache() noexcept;
void I_VramViewerDraw(const int32_t texPageNum) noexcept;
void I_NetSendRecv() noexcept;
uint32_t I_LocalButtonsToNet(const padbuttons_t pCtrlBindings[NUM_CTRL_BINDS]) noexcept;
padbuttons_t* I_NetButtonsToLocal(const uint32_t encodedBindings) noexcept;

#endif  // !PSYDOOM_MODS
