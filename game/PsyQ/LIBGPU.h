#pragma once

#include <cstdint>

// GP0 and GP1 GPU registers addresses.
// These two registers are used to read and write GPU data and for status/control.
// These addresses were NOT exposed in the original PsyQ SDK as far as I can see.
static constexpr uint32_t GPU_REG_GP0 = 0x1F801810;
static constexpr uint32_t GPU_REG_GP1 = 0x1F801814;

// Represents a rectangular area of the framebuffer.
// The coordinates assume 16-bit color values, for other modes coords will need to be scaled accordingly.
// Note: negative values or values exceeding the 1024x512 (16-bit) framebuffer are NOT allowed!
struct RECT {
    int16_t x;      // Position in VRAM
    int16_t y;
    int16_t w;      // Size in VRAM
    int16_t h;
};

static_assert(sizeof(RECT) == 8);

// Drawing primitive: unconnected flat shaded line
struct LINE_F2 {
    uint32_t    tag;
    uint8_t     r0;
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;
    int16_t     x0;
    int16_t     y0;
    int16_t     x1;
    int16_t     y1;
};

static_assert(sizeof(LINE_F2) == 16);

// Drawing primitive: arbitrarily sized sprite
struct SPRT {
    uint32_t    tag;
    uint8_t     r0;
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;
    int16_t     x0;         // Position of the sprite
    int16_t     y0;
    uint8_t     t_u0;       // Texture u/v coords: note could not use just 'v0' due to register name clashing
    uint8_t     t_v0;
    uint16_t    clut;       // Which CLUT to use for color indexing
    int16_t     w;          // Draw width and height of the sprite
    int16_t     h;
};

static_assert(sizeof(SPRT) == 20);

// Drawing primitive: modify the current draw mode as specified by 'LIBGPU_SetDrawMode()'
struct DR_MODE {
    uint32_t    tag;
    uint32_t    code[2];
};

static_assert(sizeof(DR_MODE) == 12);

void LIBGPU_ResetGraph() noexcept;
void LIBGPU_SetGraphReverse() noexcept;
void LIBGPU_SetGraphDebug() noexcept;
void LIBGPU_SetGraphQueue() noexcept;
void LIBGPU_GetGraphType() noexcept;
void LIBGPU_GetGraphDebug() noexcept;
void LIBGPU_DrawSyncCallback() noexcept;
void LIBGPU_SetDispMask() noexcept;
void LIBGPU_DrawSync() noexcept;
void LIBGPU_checkRECT() noexcept;
void LIBGPU_ClearImage() noexcept;

void LIBGPU_LoadImage(const RECT& dstRect, const uint32_t* const pImageData) noexcept;
void _thunk_LIBGPU_LoadImage() noexcept;

void LIBGPU_StoreImage() noexcept;
void LIBGPU_MoveImage() noexcept;
void LIBGPU_ClearOTag() noexcept;
void LIBGPU_ClearOTagR() noexcept;
void LIBGPU_DrawPrim() noexcept;
void LIBGPU_DrawOTag() noexcept;
void LIBGPU_PutDrawEnv() noexcept;
void LIBGPU_GetDrawEnv() noexcept;
void LIBGPU_PutDispEnv() noexcept;
void LIBGPU_GetDispEnv() noexcept;
void LIBGPU_GetODE() noexcept;
void LIBGPU_SetTexWindow() noexcept;
void LIBGPU_SetDrawArea() noexcept;
void LIBGPU_SetDrawOffset() noexcept;
void LIBGPU_SetPriority() noexcept;

void LIBGPU_SetDrawMode(
    DR_MODE& modePrim,
    const bool bCanDrawInDisplayArea,
    const bool bDitheringOn,
    const uint32_t texPageId,
    const RECT* const pNewTexWindow
) noexcept;

void _thunk_LIBGPU_SetDrawMode() noexcept;

void LIBGPU_SetDrawEnv() noexcept;
void LIBGPU_SYS_get_mode() noexcept;
void LIBGPU_SYS_get_cs() noexcept;
void LIBGPU_SYS_get_ce() noexcept;
void LIBGPU_SYS_get_ofs() noexcept;
void LIBGPU_SYS_get_tw() noexcept;
void LIBGPU_SYS_get_dx() noexcept;
void LIBGPU_SYS__status() noexcept;
void LIBGPU_SYS__otc() noexcept;
void LIBGPU_SYS__clr() noexcept;
void LIBGPU_SYS__dws() noexcept;
void LIBGPU_SYS__drs() noexcept;
void LIBGPU_SYS__ctl() noexcept;
void LIBGPU_SYS__getctl() noexcept;
void LIBGPU_SYS__cwb() noexcept;
void LIBGPU_SYS__cwc() noexcept;
void LIBGPU_SYS__param() noexcept;
void LIBGPU_SYS__addque() noexcept;
void LIBGPU_SYS__addque2() noexcept;
void LIBGPU_SYS__exeque() noexcept;
void LIBGPU_SYS__reset() noexcept;
void LIBGPU_SYS__sync() noexcept;
void LIBGPU_SYS_set_alarm() noexcept;
void LIBGPU_SYS_get_alarm() noexcept;
void LIBGPU_SYS_memset() noexcept;
void LIBGPU_GetTPage() noexcept;
void LIBGPU_GetClut() noexcept;
void LIBGPU_DumpTPage() noexcept;
void LIBGPU_DumpClut() noexcept;
void LIBGPU_NextPrim() noexcept;
void LIBGPU_IsEndPrim() noexcept;
void LIBGPU_AddPrim() noexcept;
void LIBGPU_AddPrims() noexcept;
void LIBGPU_CatPrim() noexcept;
void LIBGPU_TermPrim() noexcept;
void LIBGPU_SetSemiTrans() noexcept;

void LIBGPU_SetShadeTex(void* const pPrim, const bool bDisableShading) noexcept;
void _thunk_LIBGPU_SetShadeTex() noexcept;

void LIBGPU_SetPolyF3() noexcept;
void LIBGPU_SetPolyFT3() noexcept;
void LIBGPU_SetPolyG3() noexcept;
void LIBGPU_SetPolyGT3() noexcept;
void LIBGPU_SetPolyF4() noexcept;
void LIBGPU_SetPolyFT4() noexcept;
void LIBGPU_SetPolyG4() noexcept;
void LIBGPU_SetPolyGT4() noexcept;
void LIBGPU_SetSprt8() noexcept;
void LIBGPU_SetSprt16() noexcept;

void LIBGPU_SetSprt(SPRT & sprt) noexcept;
void _thunk_LIBGPU_SetSprt() noexcept;

void LIBGPU_SetTile1() noexcept;
void LIBGPU_SetTile8() noexcept;
void LIBGPU_SetTile16() noexcept;
void LIBGPU_SetTile() noexcept;
void LIBGPU_SetBlockFill() noexcept;
void LIBGPU_SetLineF2(LINE_F2& line) noexcept;
void LIBGPU_SetLineG2() noexcept;
void LIBGPU_SetLineF3() noexcept;
void LIBGPU_SetLineG3() noexcept;
void LIBGPU_SetLineF4() noexcept;
void LIBGPU_SetLineG4() noexcept;
void LIBGPU_MargePrim() noexcept;
void LIBGPU_DumpDrawEnv() noexcept;
void LIBGPU_DumpDispEnv() noexcept;
void LIBGPU_SetDumpFnt() noexcept;
void LIBGPU_FntLoad() noexcept;
void LIBGPU_FntOpen() noexcept;
void LIBGPU_FntFlush() noexcept;
void LIBGPU_FntPrint() noexcept;
void LIBGPU_LoadTPage() noexcept;
void LIBGPU_LoadClut() noexcept;
void LIBGPU_SetDefDrawEnv() noexcept;
void LIBGPU_SetDefDispEnv() noexcept;

// Set the color on a draw primitive
template <class T>
inline void LIBGPU_setRGB0(T& prim, const uint8_t r0, const uint8_t g0, const uint8_t b0) noexcept {
    prim.r0 = r0;
    prim.g0 = g0;
    prim.b0 = b0;
}

// Set a 2d point on a draw primitive
template <class T>
inline void LIBGPU_setXY0(T& prim, const int16_t x0, const int16_t y0) noexcept {
    prim.x0 = x0;
    prim.y0 = y0;
}

// Set 2d start and end points on a draw primitive
template <class T>
inline void LIBGPU_setXY2(T& prim, const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1) noexcept {
    prim.x0 = x0;
    prim.y0 = y0;
    prim.x1 = x1;
    prim.y1 = y1;
}

// Set the width and height on a draw primitive
template <class T>
inline void LIBGPU_setWH(T& prim, const int16_t w, const int16_t h) noexcept {
    prim.w = w;
    prim.h = h;
}

// Set the uv coordinates for a draw primitive
template <class T>
inline void LIBGPU_setUV0(T& prim, const uint8_t u, const uint8_t v) noexcept {
    prim.t_u0 = u;
    prim.t_v0 = v;
}

// Set the length of the payload for a primitive
template <class T>
inline void LIBGPU_setlen(T& prim, const uint8_t len) noexcept {
    prim.tag &= 0x00FFFFFF;
    prim.tag |= (uint32_t) len << 24;
}
