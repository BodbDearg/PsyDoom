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

// Game control binding index: these are the actions which are configurable to different buttons.
// These also must be synchronized in a network game.
enum controlbinding_t : int32_t {
    cbind_attack,
    cbind_use,
    cbind_strafe,
    cbind_speed,
    cbind_strafe_left,
    cbind_strafe_right,
    cbind_weapon_back,
    cbind_weapon_forward,
    NUM_CTRL_BINDS
};

// Configurable buttons on the d-pad.
// These give an index into the array of button masks for configurable buttons.
// That in turn allows us to match buttons to bindable actions.
enum : int32_t {
    bindablebtn_triangle,
    bindablebtn_circle,
    bindablebtn_cross,
    bindablebtn_square,
    bindablebtn_l1,
    bindablebtn_r1,
    bindablebtn_l2,
    bindablebtn_r2,
    NUM_BINDABLE_BTNS
};

// Button bits/masks for gamepad buttons
static constexpr uint16_t PAD_UP        = 0x1000;
static constexpr uint16_t PAD_DOWN      = 0x4000;
static constexpr uint16_t PAD_LEFT      = 0x8000;
static constexpr uint16_t PAD_RIGHT     = 0x2000;
static constexpr uint16_t PAD_TRIANGLE  = 0x10;
static constexpr uint16_t PAD_CROSS     = 0x40;
static constexpr uint16_t PAD_SQUARE    = 0x80;
static constexpr uint16_t PAD_CIRCLE    = 0x20;
static constexpr uint16_t PAD_L1        = 0x4;
static constexpr uint16_t PAD_L2        = 0x1;
static constexpr uint16_t PAD_R1        = 0x8;
static constexpr uint16_t PAD_R2        = 0x2;
static constexpr uint16_t PAD_START     = 0x800;
static constexpr uint16_t PAD_SELECT    = 0x100;

static constexpr uint16_t PAD_DIRECTION_BTNS = (
    PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT
);

static constexpr uint16_t PAD_ACTION_BTNS = (
    PAD_TRIANGLE | PAD_CROSS | PAD_SQUARE | PAD_CIRCLE
);

static constexpr uint16_t PAD_SHOULDER_BTNS = (
    PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2
);

static constexpr uint16_t PAD_ANY_BTNS = (
    PAD_DIRECTION_BTNS | PAD_ACTION_BTNS | PAD_SHOULDER_BTNS | PAD_START | PAD_SELECT
);

// Type for a pressed button mask.
// Certain bits correspond to certain buttons on the PSX digital controller.
typedef uint32_t padbuttons_t;

extern const VmPtr<uint32_t>                                                gTCacheFillPage;
extern const VmPtr<uint32_t>                                                gTCacheFillCellX;
extern const VmPtr<uint32_t>                                                gTCacheFillCellY;
extern const VmPtr<uint32_t>                                                gTCacheFillRowCellH;
extern const VmPtr<uint32_t>                                                gLockedTexPagesMask;
extern const VmPtr<std::byte[TMP_BUFFER_SIZE]>                              gTmpBuffer;
extern const VmPtr<uint32_t>                                                gTotalVBlanks;
extern const VmPtr<uint32_t>                                                gLastTotalVBlanks;
extern const VmPtr<uint32_t>                                                gElapsedVBlanks;
extern const VmPtr<uint32_t>                                                gCurDispBufferIdx;
extern const VmPtr<DISPENV[2]>                                              gDispEnvs;
extern const VmPtr<DRAWENV[2]>                                              gDrawEnvs;
extern const VmPtr<uint32_t>                                                gNumFramesDrawn;
extern const VmPtr<int32_t>                                                 gCurPlayerIndex;
extern const VmPtr<padbuttons_t[NUM_CTRL_BINDS]>                            gCtrlBindings;
extern const VmPtr<VmPtr<padbuttons_t[NUM_CTRL_BINDS]>[MAXPLAYERS]>         gpPlayerCtrlBindings;
extern const padbuttons_t                                                   gBtnMasks[NUM_BINDABLE_BTNS];
extern const VmPtr<texture_t>                                               gTex_STATUS;
extern const VmPtr<texture_t>                                               gTex_PAUSE;
extern const VmPtr<texture_t>                                               gTex_LOADING;
extern const VmPtr<texture_t>                                               gTex_NETERR;
extern const VmPtr<texture_t>                                               gTex_CONNECT;

void I_Main() noexcept;
void I_PSXInit() noexcept;

[[noreturn]] void I_Error(const char* const fmtMsg, ...) noexcept;

void I_ReadGamepad() noexcept;
void I_LoadAndCacheTexLump(texture_t& tex, const char* const name, int32_t lumpNum) noexcept;
void I_CacheAndDrawSprite(texture_t& tex, const int16_t xpos, const int16_t ypos, const int16_t clutId) noexcept;

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

void I_DrawLoadingPlaque(texture_t& tex, const int16_t xpos, const int16_t ypos, const int16_t clutId) noexcept;
void I_IncDrawnFrameCount() noexcept;
void I_DrawPresent() noexcept;
void I_VsyncCallback() noexcept;
void I_Init() noexcept;
void I_CacheTex(texture_t& tex) noexcept;
void I_RemoveTexCacheEntry(texture_t& tex) noexcept;
void I_PurgeTexCache() noexcept;
void I_VramViewerDraw(const int32_t texPageNum) noexcept;
void I_NetSetup() noexcept;
void I_NetUpdate() noexcept;
void I_NetHandshake() noexcept;
void I_NetSendRecv() noexcept;
void I_SubmitGpuCmds() noexcept;
uint32_t I_LocalButtonsToNet(const padbuttons_t pCtrlBindings[NUM_CTRL_BINDS]) noexcept;
padbuttons_t* I_NetButtonsToLocal(const uint32_t encodedBindings) noexcept;
