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
    int16_t     x;      // Position in VRAM
    int16_t     y;
    int16_t     w;      // Size in VRAM
    int16_t     h;
};

static_assert(sizeof(RECT) == 8);

// Drawing primitive: unconnected flat shaded line
struct LINE_F2 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive
    uint8_t     r0;         // Line color
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Line 2d coords
    int16_t     y0;
    int16_t     x1;
    int16_t     y1;
};

static_assert(sizeof(LINE_F2) == 16);

// Drawing primitive: flat shaded textured triangle (polygon 3)
struct POLY_FT3 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive
    uint8_t     r0;         // Color to shade the primitive with
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Vertex 1: position
    int16_t     y0;
    uint8_t     tu0;        // Vertex 1: texture coords
    uint8_t     tv0;
    uint16_t    clut;       // Color lookup table id for 4/8-bit textures
    int16_t     x1;         // Vertex 2: position
    int16_t     y1;
    uint8_t     tu1;        // Vertex 2: texture coords
    uint8_t     tv1;
    uint16_t    tpage;      // Texture page id
    int16_t     x2;         // Vertex 3: position
    int16_t     y2;
    uint8_t     tu2;        // Vertex 3: texture coords
    uint8_t     tv2;
    uint16_t    pad;        // Not used
};

static_assert(sizeof(POLY_FT3) == 32);

// Drawing primitive: flat shaded colored quad (polygon 4)
struct POLY_F4 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive
    uint8_t     r0;         // Color to shade the primitive with
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Vertex 1: position
    int16_t     y0;
    int16_t     x1;         // Vertex 2: position
    int16_t     y1;
    int16_t     x2;         // Vertex 3: position
    int16_t     y2;
    int16_t     x3;         // Vertex 4: position
    int16_t     y3;
};

static_assert(sizeof(POLY_F4) == 24);

// Drawing primitive: flat shaded textured quad (polygon 4)
struct POLY_FT4 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive
    uint8_t     r0;         // Color to shade the primitive with
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Vertex 1: position
    int16_t     y0;
    uint8_t     tu0;        // Vertex 1: texture coords
    uint8_t     tv0;
    uint16_t    clut;       // Color lookup table id for 4/8-bit textures
    int16_t     x1;         // Vertex 2: position
    int16_t     y1;
    uint8_t     tu1;        // Vertex 2: texture coords
    uint8_t     tv1;
    uint16_t    tpage;      // Texture page id
    int16_t     x2;         // Vertex 3: position
    int16_t     y2;    
    uint8_t     tu2;        // Vertex 3: texture coords
    uint8_t     tv2;
    uint16_t    pad1;       // Not used
    int16_t     x3;         // Vertex 4: position
    int16_t     y3;
    uint8_t     tu3;        // Vertex 4: texture coords
    uint8_t     tv3;
    uint16_t    pad2;       // Not used
};

static_assert(sizeof(POLY_FT4) == 40);

// Drawing primitive: arbitrarily sized sprite
struct SPRT {
    uint32_t    tag;        // The primitive size and 24-bit pointer to next primitive
    uint8_t     r0;         // Color to apply to the sprite
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Position of the sprite
    int16_t     y0;
    uint8_t     tu0;        // Texture u/v coords: note could not use just 'v0' due to register name clashing
    uint8_t     tv0;
    int16_t     clut;       // Which CLUT to use for color indexing
    int16_t     w;          // Draw width and height of the sprite
    int16_t     h;
};

static_assert(sizeof(SPRT) == 20);

// Drawing primitive: modify the current draw mode
struct DR_MODE {
    uint32_t    tag;
    uint32_t    code[2];    // The settings made via 'LIBGPU_SetDrawMode()'
};

static_assert(sizeof(DR_MODE) == 12);

// Drawing primitive: modify the current texture window as specified by 'LIBGPU_SetTexWindow()'
struct DR_TWIN {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive
    uint32_t    code[2];    // The settings made via 'LIBGPU_SetTexWindow()'
};

static_assert(sizeof(DR_TWIN) == 12);

// Settings for the front buffer (buffer being displayed)
struct DISPENV {
    RECT        disp;           // What to to display from the frame buffer. Width can be: 256, 320, 384, 512, or 640. Height can be: 240 or 480.
    RECT        screen;         // Output screen display area. 0,0 top left of monitor, 256, 240 is lower right.
    uint8_t     isinter;        // If '1' the display is interlaced
    uint8_t     isrgb24;        // If '1' the display is RGB24
    uint8_t     _pad[2];        // Unused/reserved
};

static_assert(sizeof(DISPENV) == 20);

// Draw primitive for setting a new draw environment
struct DR_ENV {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive
    uint32_t    code[15];   // Data populated by 'SetDrawEnv'
};

static_assert(sizeof(DR_ENV) == 64);

// Settings for the back buffer (buffer being drawn to)
struct DRAWENV {
     RECT       clip;           // Rectangular area to restrict drawing to. Must be between 0,0 to 1023,511 (inclusive).
     int16_t    ofs[2];         // Pixel offsets added to primitives before drawing
     RECT       tw;             // Specify what part of VRAM to wrap within
     uint16_t   tpage;          // Starting/initial value of for the current texture page
     uint8_t    dtd;            // If '1' dithering is enabled
     uint8_t    dfe;            // If '1' drawing to the display area is allowed
     uint8_t    isbg;           // If '1' clear the background with color 'r0', 'g0', 'b0' when the drawing environment is set
     uint8_t    r0, g0, b0;     // Clear color to use if 'isbg' is '1'.
     DR_ENV     dr_env;         // For internal PsyQ SDK use only
};

static_assert(sizeof(DRAWENV) == 92);

void LIBGPU_ResetGraph() noexcept;
void LIBGPU_SetGraphDebug() noexcept;
void LIBGPU_GetGraphType() noexcept;
void LIBGPU_DrawSyncCallback() noexcept;
void LIBGPU_SetDispMask() noexcept;

int32_t LIBGPU_DrawSync(const int32_t mode) noexcept;
void _thunk_LIBGPU_DrawSync() noexcept;

void LIBGPU_checkRECT() noexcept;

void LIBGPU_LoadImage(const RECT& dstRect, const uint32_t* const pImageData) noexcept;
void _thunk_LIBGPU_LoadImage() noexcept;

int32_t LIBGPU_MoveImage(const RECT& src, const int32_t dstX, const int32_t dstY) noexcept;
void _thunk_LIBGPU_MoveImage() noexcept;

void LIBGPU_DrawOTag() noexcept;
DRAWENV& LIBGPU_PutDrawEnv(DRAWENV& env) noexcept;
DISPENV& LIBGPU_PutDispEnv(DISPENV& env) noexcept;

void LIBGPU_SetTexWindow(DR_TWIN& prim, const RECT& texWin) noexcept;
void _thunk_LIBGPU_SetTexWindow() noexcept;

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

uint32_t LIBGPU_SYS_get_tw(const RECT* const pRect) noexcept;
void _thunk_LIBGPU_SYS_get_tw() noexcept;

void LIBGPU_SYS_get_dx() noexcept;
void LIBGPU_SYS__dws() noexcept;
void LIBGPU_SYS__ctl() noexcept;
void LIBGPU_SYS__getctl() noexcept;
void LIBGPU_SYS__cwc() noexcept;
void LIBGPU_SYS__addque() noexcept;
void LIBGPU_SYS__addque2() noexcept;
void LIBGPU_SYS__exeque() noexcept;
void LIBGPU_SYS__reset() noexcept;
void LIBGPU_SYS__sync() noexcept;
void LIBGPU_SYS_set_alarm() noexcept;
void LIBGPU_SYS_get_alarm() noexcept;
void LIBGPU_SYS_memset() noexcept;

uint16_t LIBGPU_GetTPage(
    const int32_t texFmt,
    const int32_t semiTransRate,
    const int32_t tpageX,
    const int32_t tpageY
) noexcept;

void _thunk_LIBGPU_GetTPage() noexcept;

uint16_t LIBGPU_GetClut(const int32_t x, const int32_t y) noexcept;

void LIBGPU_NextPrim() noexcept;        // TODO: still needed?
void LIBGPU_IsEndPrim() noexcept;       // TODO: still needed?
void LIBGPU_AddPrim() noexcept;         // TODO: still needed?
void LIBGPU_AddPrims() noexcept;        // TODO: still needed?
void LIBGPU_CatPrim() noexcept;         // TODO: still needed?
void LIBGPU_TermPrim() noexcept;        // TODO: still needed?
void LIBGPU_SetSemiTrans(void* const pPrim, const bool bTransparent) noexcept;
void LIBGPU_SetShadeTex(void* const pPrim, const bool bDisableShading) noexcept;
void _thunk_LIBGPU_SetShadeTex() noexcept;

void LIBGPU_SetPolyFT3(POLY_FT3& poly) noexcept;
void LIBGPU_SetPolyF4(POLY_F4& poly) noexcept;
void LIBGPU_SetPolyFT4(POLY_FT4& poly) noexcept;
void LIBGPU_SetSprt8() noexcept;
void LIBGPU_SetSprt(SPRT & sprt) noexcept;
void LIBGPU_SetTile() noexcept;
void LIBGPU_SetLineF2(LINE_F2& line) noexcept;
void LIBGPU_MargePrim() noexcept;       // TODO: still needed?
void LIBGPU_SetDumpFnt() noexcept;
void LIBGPU_FntLoad() noexcept;
void LIBGPU_FntOpen() noexcept;
void LIBGPU_FntFlush() noexcept;
void LIBGPU_FntPrint() noexcept;
void LIBGPU_LoadTPage() noexcept;
void LIBGPU_LoadClut() noexcept;

DRAWENV& LIBGPU_SetDefDrawEnv(DRAWENV& env, const int32_t x, const int32_t y, const int32_t w, const  int32_t h) noexcept;
DISPENV& LIBGPU_SetDefDispEnv(DISPENV& disp, const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept;

// Set the color on a draw primitive
template <class T>
inline void LIBGPU_setRGB0(T& prim, const uint8_t r0, const uint8_t g0, const uint8_t b0) noexcept {
    prim.r0 = r0;
    prim.g0 = g0;
    prim.b0 = b0;
}

// Set a 2d point on a draw primitive
template <class T>
inline constexpr void LIBGPU_setXY0(T& prim, const int16_t x0, const int16_t y0) noexcept {
    prim.x0 = x0;
    prim.y0 = y0;
}

// Set 2d start and end points on a draw primitive
template <class T>
inline constexpr void LIBGPU_setXY2(T& prim, const int16_t x0, const int16_t y0, const int16_t x1, const int16_t y1) noexcept {
    prim.x0 = x0;
    prim.y0 = y0;
    prim.x1 = x1;
    prim.y1 = y1;
}

// Set 2d triangle points on a draw primitive
template <class T>
inline constexpr void LIBGPU_setXY3(
    T& prim,
    const int16_t x0,
    const int16_t y0,
    const int16_t x1,
    const int16_t y1,
    const int16_t x2,
    const int16_t y2
) noexcept {
    prim.x0 = x0;
    prim.y0 = y0;
    prim.x1 = x1;
    prim.y1 = y1;
    prim.x2 = x2;
    prim.y2 = y2;
}

// Set 2d triangle points on a quad primitive
template <class T>
inline constexpr void LIBGPU_setXY4(
    T& prim,
    const int16_t x0,
    const int16_t y0,
    const int16_t x1,
    const int16_t y1,
    const int16_t x2,
    const int16_t y2,
    const int16_t x3,
    const int16_t y3
) noexcept {
    prim.x0 = x0;
    prim.y0 = y0;
    prim.x1 = x1;
    prim.y1 = y1;
    prim.x2 = x2;
    prim.y2 = y2;
    prim.x3 = x3;
    prim.y3 = y3;
}

// Set the width and height on a draw primitive
template <class T>
inline constexpr void LIBGPU_setWH(T& prim, const int16_t w, const int16_t h) noexcept {
    prim.w = w;
    prim.h = h;
}

// Set the uv coordinates for a draw primitive
template <class T>
inline constexpr void LIBGPU_setUV0(T& prim, const uint8_t tu0, const uint8_t tv0) noexcept {
    prim.tu0 = tu0;
    prim.tv0 = tv0;
}

// Set the uv coordinates for a triangle draw primitive
template <class T>
inline constexpr void LIBGPU_setUV3(
    T& prim,
    const uint8_t tu0,
    const uint8_t tv0,
    const uint8_t tu1,
    const uint8_t tv1,
    const uint8_t tu2,
    const uint8_t tv2
) noexcept {
    prim.tu0 = tu0;
    prim.tv0 = tv0;
    prim.tu1 = tu1;
    prim.tv1 = tv1;
    prim.tu2 = tu2;
    prim.tv2 = tv2;
}

// Set the uv coordinates for a quad draw primitive
template <class T>
inline constexpr void LIBGPU_setUV4(
    T& prim,
    const uint8_t tu0,
    const uint8_t tv0,
    const uint8_t tu1,
    const uint8_t tv1,
    const uint8_t tu2,
    const uint8_t tv2,
    const uint8_t tu3,
    const uint8_t tv3
) noexcept {
    prim.tu0 = tu0;
    prim.tv0 = tv0;
    prim.tu1 = tu1;
    prim.tv1 = tv1;
    prim.tu2 = tu2;
    prim.tv2 = tv2;
    prim.tu3 = tu3;
    prim.tv3 = tv3;
}

// Set the length of the payload for a primitive
template <class T>
inline constexpr void LIBGPU_setlen(T& prim, const uint8_t len) noexcept {
    prim.tag &= 0x00FFFFFF;
    prim.tag |= (uint32_t) len << 24;
}

// Set the bounds of a RECT
inline constexpr void LIBGPU_setRECT(RECT& rect, const int16_t x, const int16_t y, const int16_t w, const int16_t h) noexcept {
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes the 'tpage' (texture page) member of a drawing primitive.
// Note: the texture page address is limited to a multiple of 64 in the X direction and a multiple of 256 in the Y direction.
//
// Inputs:
//  (1) texMode : What format the texture is in.
//          0 = 4 bit indexed (uses CLUT)
//          1 = 8 bit indexed (uses CLUT)
//          2 = 16 bit raw/direct
//  (2) semiTransRate : Semi transparency rate.
//      Controls the blend equation, how much of the 'src' (new pixel) and 'dst' (previous pixel) to use.
//          0 = 0.5 * dst + 0.5 * src   (50% alpha blend)
//          1 = 1.0 * dst + 1.0 * src   (additive blend)
//          2 = 1.0 * dst - 1.0 * src   (subtractive blend)
//          3 = 1.0 * dst + 0.25 * src  (additive 25% blend)
//  (3) tpageX : texture page address (x, must be multiple of 64)
//  (4) tpageY : texture page address (y, must be multiple of 256)
//------------------------------------------------------------------------------------------------------------------------------------------
inline constexpr uint16_t LIBGPU_getTPage(
    const int32_t texFmt,
    const int32_t semiTransRate,
    const int32_t tpageX,
    const int32_t tpageY
) noexcept {
     return (uint16_t)(
        ((texFmt & 0x3) << 7)|
        ((semiTransRate & 0x3) << 5)|
        ((tpageY & 0x100) >> 4)|
        ((tpageX & 0x3ff) >> 6)|
        ((tpageY & 0x200) << 2)
    );
}
