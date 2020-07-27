#pragma once

#include "Doom/doomdef.h"

#include <cstddef>

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

// Size of the temporary buffer that is used for WAD loading and other stuff - 64 KiB.
// PsyDoom: small adjustment of +8 so that the textures for the 'legals' UI (and other large textures) can load without overflow.
#if PSYDOOM_MODS
    static constexpr uint32_t TMP_BUFFER_SIZE = 0x10008;
#else
    static constexpr uint32_t TMP_BUFFER_SIZE = 0x10000;
#endif

// Game control binding index: these are the actions which are configurable to different buttons.
// These also must be synchronized in a network game.
enum controlbinding_t : int32_t {
    cbind_attack,
    cbind_use,
    cbind_strafe,
    cbind_run,
    cbind_strafe_left,
    cbind_strafe_right,
    cbind_prev_weapon,
    cbind_next_weapon,
    cbind_move_backward,
    cbind_move_forward,
    NUM_CTRL_BINDS
};

// Configurable buttons on the PlayStation's digital controller and the PlayStation mouse.
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
    bindablebtn_mouse_left,
    bindablebtn_mouse_right,
    NUM_BINDABLE_BTNS
};

// Type for a pressed button mask.
// Certain bits correspond to certain buttons on the PSX digital controller.
typedef uint32_t padbuttons_t;

extern uint32_t             gTCacheFillPage;
extern uint32_t             gTCacheFillCellX;
extern uint32_t             gTCacheFillCellY;
extern uint32_t             gTCacheFillRowCellH;
extern uint32_t             gLockedTexPagesMask;
extern std::byte            gTmpBuffer[TMP_BUFFER_SIZE];
extern uint32_t             gTotalVBlanks;
extern uint32_t             gLastTotalVBlanks;
extern uint32_t             gElapsedVBlanks;
extern uint32_t             gCurDispBufferIdx;
extern DISPENV              gDispEnvs[2];
extern DRAWENV              gDrawEnvs[2];
extern uint32_t             gNumFramesDrawn;
extern int32_t              gCurPlayerIndex;
extern padbuttons_t         gCtrlBindings[NUM_CTRL_BINDS];
extern int32_t              gPsxMouseSensitivity;
extern const padbuttons_t   gBtnMasks[NUM_BINDABLE_BTNS];
extern texture_t            gTex_STATUS;
extern texture_t            gTex_PAUSE;
extern texture_t            gTex_LOADING;
extern texture_t            gTex_NETERR;
extern texture_t            gTex_CONNECT;

void I_Main() noexcept;
void I_PSXInit() noexcept;

[[noreturn]] void I_Error(const char* const fmtMsg, ...) noexcept;

uint32_t I_ReadGamepad() noexcept;
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
bool I_NetUpdate() noexcept;
void I_NetHandshake() noexcept;
void I_SubmitGpuCmds() noexcept;

#if PSYDOOM_MODS
    int32_t I_GetTotalVBlanks() noexcept;
#endif
