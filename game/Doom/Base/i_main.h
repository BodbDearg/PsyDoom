#pragma once

#include "Doom/doomdef.h"

struct DISPENV;
struct DRAWENV;
struct texture_t;

// Texture cache related constants
static constexpr uint32_t NUM_TCACHE_PAGES      = 11;
static constexpr uint32_t TCACHE_PAGE_SIZE      = 256;  // Square size: 256x256 pixels
static constexpr uint32_t TCACHE_CELL_SIZE      = 16;
static constexpr uint32_t TCACHE_CELLS_X        = TCACHE_PAGE_SIZE / TCACHE_CELL_SIZE;
static constexpr uint32_t TCACHE_CELLS_Y        = TCACHE_PAGE_SIZE / TCACHE_CELL_SIZE;
static constexpr uint32_t NUM_TCACHE_PAGE_CELLS = TCACHE_CELLS_X * TCACHE_CELLS_Y;
static constexpr uint32_t ALL_TPAGES_MASK       = (UINT32_MAX >> (32 - NUM_TCACHE_PAGES));

// Size of the temporary buffer that is used for WAD loading and other stuff - 64 KiB
static constexpr uint32_t TMP_BUFFER_SIZE = 0x10000;

// Gamepad button binding index: these are the actions which are configurable to different buttons.
// These also must be synchronized in a network game.
enum padbinding_t {
    PAD_BINDING_ATTACK,
    PAD_BINDING_USE,
    PAD_BINDING_STRAFE,
    PAD_BINDING_SPEED,
    PAD_BINDING_STRAFE_LEFT,
    PAD_BINDING_STRAFE_RIGHT,
    PAD_BINDING_WEAPON_BACK,
    PAD_BINDING_WEAPON_FORWARD,
    NUM_PAD_BINDINGS
};

// Type for a pressed button mask.
// Certain bits correspond to certain buttons on the PSX digital controller.
//
// TODO: create constants for available button bits. 
typedef uint32_t padbuttons_t;

extern const VmPtr<uint32_t>                                            gTCacheFillPage;
extern const VmPtr<uint32_t>                                            gTCacheFillCellX;
extern const VmPtr<uint32_t>                                            gTCacheFillCellY;
extern const VmPtr<uint32_t>                                            gTCacheFillRowCellH;
extern const VmPtr<uint32_t>                                            gLockedTexPagesMask;
extern const VmPtr<std::byte[TMP_BUFFER_SIZE]>                          gTmpBuffer;
extern const VmPtr<uint32_t>                                            gTotalVBlanks;
extern const VmPtr<uint32_t>                                            gLastTotalVBlanks;
extern const VmPtr<uint32_t>                                            gElapsedVBlanks;
extern const VmPtr<uint32_t>                                            gCurDispBufferIdx;
extern const VmPtr<DISPENV[2]>                                          gDispEnvs;
extern const VmPtr<DRAWENV[2]>                                          gDrawEnvs;
extern const VmPtr<uint32_t>                                            gNumFramesDrawn;
extern const VmPtr<uint32_t>                                            gCurPlayerIndex;
extern const VmPtr<padbuttons_t[NUM_PAD_BINDINGS]>                      gBtnBindings;
extern const VmPtr<VmPtr<padbuttons_t[NUM_PAD_BINDINGS]>[MAXPLAYERS]>   gpPlayerBtnBindings;
extern const VmPtr<texture_t>                                           gTex_STATUS;
extern const VmPtr<texture_t>                                           gTex_PAUSE;
extern const VmPtr<texture_t>                                           gTex_LOADING;
extern const VmPtr<texture_t>                                           gTex_NETERR;
extern const VmPtr<texture_t>                                           gTex_CONNECT;

void I_Main() noexcept;
void I_PSXInit() noexcept;
[[noreturn]] void I_Error(const char* const fmtMsg, ...) noexcept;
void I_ReadGamepad() noexcept;
void _thunk_I_LoadAndCacheTexLump() noexcept;

void I_CacheAndDrawSprite(texture_t& tex, const int16_t xpos, const int16_t ypos, const int16_t clutId) noexcept;
void _thunk_I_CacheAndDrawSprite() noexcept;

void I_DrawSprite(
    const uint16_t texPageId,
    const int16_t clutId,
    const int16_t xpos,
    const int16_t ypos,
    const uint8_t texU,
    const uint8_t texV,
    const uint16_t texW,
    const uint16_t texH
) noexcept;

void _thunk_I_DrawSprite() noexcept;

void I_DrawLoadingPlaque(texture_t& tex, const int16_t xpos, const int16_t ypos, const int16_t clutId) noexcept;
void _thunk_I_DrawLoadingPlaque() noexcept;

void I_IncDrawnFrameCount() noexcept;
void I_DrawPresent() noexcept;
void I_VsyncCallback() noexcept;
void I_Init() noexcept;

void I_CacheTex(texture_t& tex) noexcept;
void _thunk_I_CacheTex() noexcept;

void I_RemoveTexCacheEntry(texture_t& tex) noexcept;
void I_PurgeTexCache() noexcept;
void I_VramViewerDraw() noexcept;
void I_NetSetup() noexcept;
void I_NetUpdate() noexcept;
void I_NetHandshake() noexcept;
void I_NetSendRecv() noexcept;
void I_SubmitGpuCmds() noexcept;
void I_LocalButtonsToNet() noexcept;
void I_NetButtonsToLocal() noexcept;
