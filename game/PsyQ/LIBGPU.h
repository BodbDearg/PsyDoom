#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// IMPORTANT note for PsyDoom: GPU primitive encoding changes:
//
//  Originally the 'pointer to the next primitive' field for all GPU primitives was a 24-bit absolute memory address within the 2 MiB of
//  the PlayStation. I changed it's meaning/semantics to be relative to the start of the GPU command buffer currently in use, since the
//  value is only 24-bits and will not work in a modern 64-bit (or indeed 32-bit) environment. This change preserves the original size
//  of the GPU primitives, while allowing them to work correctly in a 64-bit environment.
//------------------------------------------------------------------------------------------------------------------------------------------

#include <cstdint>
#include <cstddef>

// The data type used for LIBGPU UV coordinates.
// PsyDoom upgrades this to 16-bit signed from the original 8-bit unsigned UVs.
// This enables things like tall walls to be drawn easily, proper UV mapping for 256 pixel wide textures etc.
#if PSYDOOM_LIMIT_REMOVING
    typedef int16_t LibGpuUV;
#else
    typedef uint8_t LibGpuUV;
#endif

// Represents a rectangular area of the framebuffer.
// The coordinates assume 16-bit color values, for other modes coords will need to be scaled accordingly.
// Note: negative values or values exceeding the 1024x512 (16-bit) framebuffer are NOT allowed!
struct RECT {
    int16_t     x;      // Position in VRAM
    int16_t     y;
    int16_t     w;      // Size in VRAM
    int16_t     h;
};

// Drawing primitive: unconnected flat shaded line
struct LINE_F2 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive (PsyDoom: these fields are not needed anymore & ignored)
    uint8_t     r0;         // Line color
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Line 2d coords
    int16_t     y0;
    int16_t     x1;
    int16_t     y1;
};

// Drawing primitive: flat shaded textured triangle (polygon 3)
struct POLY_FT3 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive (PsyDoom: these fields are not needed anymore & ignored)
    uint8_t     r0;         // Color to shade the primitive with
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Vertex 1: position
    int16_t     y0;
    LibGpuUV    u0;         // Vertex 1: texture coords
    LibGpuUV    v0;
    uint16_t    clut;       // Color lookup table id for 4/8-bit textures
    int16_t     x1;         // Vertex 2: position
    int16_t     y1;
    LibGpuUV    u1;         // Vertex 2: texture coords
    LibGpuUV    v1;
    uint16_t    tpage;      // Texture page id
    int16_t     x2;         // Vertex 3: position
    int16_t     y2;
    LibGpuUV    u2;         // Vertex 3: texture coords
    LibGpuUV    v2;

// PsyDoom: this bloats the structure more than needed with 16-bit uvs
#if !PSYDOOM_LIMIT_REMOVING
    uint16_t    pad;        // Not used
#endif
};

// Drawing primitive: flat shaded colored quad (polygon 4)
struct POLY_F4 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive (PsyDoom: these fields are not needed anymore & ignored)
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

// Drawing primitive: flat shaded textured quad (polygon 4)
struct POLY_FT4 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive (PsyDoom: these fields are not needed anymore & ignored)
    uint8_t     r0;         // Color to shade the primitive with
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Vertex 1: position
    int16_t     y0;
    LibGpuUV    u0;         // Vertex 1: texture coords
    LibGpuUV    v0;
    uint16_t    clut;       // Color lookup table id for 4/8-bit textures
    int16_t     x1;         // Vertex 2: position
    int16_t     y1;
    LibGpuUV    u1;         // Vertex 2: texture coords
    LibGpuUV    v1;
    uint16_t    tpage;      // Texture page id
    int16_t     x2;         // Vertex 3: position
    int16_t     y2;
    LibGpuUV    u2;         // Vertex 3: texture coords
    LibGpuUV    v2;

// PsyDoom: this bloats the structure more than needed with 16-bit uvs
#if !PSYDOOM_LIMIT_REMOVING
    uint16_t    pad1;       // Not used
#endif

    int16_t     x3;         // Vertex 4: position
    int16_t     y3;
    LibGpuUV    u3;         // Vertex 4: texture coords
    LibGpuUV    v3;

// PsyDoom: this bloats the structure more than needed with 16-bit uvs
#if !PSYDOOM_LIMIT_REMOVING
    uint16_t    pad2;       // Not used
#endif
};

// Drawing primitive: arbitrarily sized sprite
struct SPRT {
    uint32_t    tag;        // The primitive size and 24-bit pointer to next primitive (PsyDoom: these fields are not needed anymore & ignored)
    uint8_t     r0;         // Color to apply to the sprite
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Position of the sprite
    int16_t     y0;
    LibGpuUV    u0;         // Texture u/v coords
    LibGpuUV    v0;
    int16_t     clut;       // Which CLUT to use for color indexing
    int16_t     w;          // Draw width and height of the sprite
    int16_t     h;
};

// Drawing primitive: 8x8 sprite
struct SPRT_8 {
    uint32_t    tag;        // The primitive size and 24-bit pointer to next primitive (PsyDoom: these fields are not needed anymore & ignored)
    uint8_t     r0;         // Color to apply to the sprite
    uint8_t     g0;
    uint8_t     b0;
    uint8_t     code;       // Type info for the hardware
    int16_t     x0;         // Position of the sprite
    int16_t     y0;
    LibGpuUV    u0;         // Texture u/v coords
    LibGpuUV    v0;
    int16_t     clut;       // Which CLUT to use for color indexing
};

#if PSYDOOM_MODS
    // New for PsyDoom: a flat shaded textured row of Doom floor pixels.
    // This is used to accelerate the classic renderer and simplify the operations the GPU has to perform.
    struct FLOORROW_FT {
        uint8_t     r0;         // Color to shade the primitive with
        uint8_t     g0;
        uint8_t     b0;
        uint8_t     code;       // Type info for the hardware
        int16_t     x0;         // The 'x' position of the row vertices and the row 'y' value
        int16_t     x1;
        int16_t     y0;
        uint16_t    clut;       // Color lookup table id for 4/8-bit textures
        uint16_t    tpage;      // Texture page id
        LibGpuUV    u0;         // Texture coords for vertex 1 and 2
        LibGpuUV    v0;
        LibGpuUV    u1;
        LibGpuUV    v1;
    };

    // New for PsyDoom: a flat shaded textured column of Doom wall pixels.
    // This is used to accelerate the classic renderer and simplify the operations the GPU has to perform.
    struct WALLCOL_FT {
        uint8_t     r0;         // Color to shade the primitive with
        uint8_t     g0;
        uint8_t     b0;
        uint8_t     code;       // Type info for the hardware
        int16_t     y0;         // The 'y' position of the column vertices and the column 'x' value
        int16_t     y1;
        int16_t     x0;
        uint16_t    clut;       // Color lookup table id for 4/8-bit textures
        uint16_t    tpage;      // Texture page id
        LibGpuUV    u0;         // Constant 'u' texture coord and 'v' texture coords for vertex 1 and 2
        LibGpuUV    v0;
        LibGpuUV    v1;
    };
#endif

// Drawing primitive: modify the current draw mode
struct DR_MODE {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive (PsyDoom: these fields are not needed anymore & ignored)
    uint32_t    code[2];    // The settings made via 'LIBGPU_SetDrawMode()'
};

// Drawing primitive: modify the current texture window as specified by 'LIBGPU_SetTexWindow()'
struct DR_TWIN {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive (PsyDoom: these fields are not needed anymore & ignored)
    uint32_t    code[2];    // The settings made via 'LIBGPU_SetTexWindow()'
};

// Settings for the front buffer (buffer being displayed)
struct DISPENV {
    RECT        disp;           // What to to display from the frame buffer. Width can be: 256, 320, 384, 512, or 640. Height can be: 240 or 480.
    RECT        screen;         // Output screen display area. 0,0 top left of monitor, 256, 240 is lower right.
    uint8_t     isinter;        // If '1' the display is interlaced
    uint8_t     isrgb24;        // If '1' the display is RGB24
    uint8_t     _pad[2];        // Unused/reserved
};

// Draw primitive for setting a new draw environment
struct DR_ENV {
    uint32_t    tag;        // The primitive size and 24-bit pointer to the next primitive (PsyDoom: these fields are not needed anymore & ignored)
    uint32_t    code[15];   // Data populated by 'SetDrawEnv'
};

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

void LIBGPU_ResetGraph(const int32_t resetMode) noexcept;
void LIBGPU_SetGraphDebug(const int32_t debugLevel) noexcept;
void LIBGPU_SetDispMask(const int32_t mask) noexcept;

int32_t LIBGPU_DrawSync(const int32_t mode) noexcept;

void LIBGPU_LoadImage(const RECT& dstRect, const uint16_t* const pImageData) noexcept;
int32_t LIBGPU_MoveImage(const RECT& srcRect, const int32_t dstX, const int32_t dstY) noexcept;
DRAWENV& LIBGPU_PutDrawEnv(DRAWENV& env) noexcept;
DISPENV& LIBGPU_PutDispEnv(DISPENV& env) noexcept;

void LIBGPU_SetTexWindow(DR_TWIN& prim, const RECT& texWin) noexcept;

void LIBGPU_SetDrawMode(
    DR_MODE& modePrim,
    const bool bCanDrawInDisplayArea,
    const bool bDitheringOn,
    const uint16_t texPageId,
    const RECT* const pNewTexWindow
) noexcept;

uint32_t LIBGPU_SYS_get_mode(
    const bool bCanDrawInDisplayArea,
    const bool bDitheringOn,
    const uint16_t texPageId
) noexcept;

uint32_t LIBGPU_SYS_get_tw(const RECT* const pRect) noexcept;

uint16_t LIBGPU_GetTPage(
    const int32_t texFmt,
    const int32_t semiTransRate,
    const int32_t tpageX,
    const int32_t tpageY
) noexcept;

uint16_t LIBGPU_GetClut(const int32_t x, const int32_t y) noexcept;

void LIBGPU_SetPolyFT3(POLY_FT3& poly) noexcept;
void LIBGPU_SetPolyF4(POLY_F4& poly) noexcept;
void LIBGPU_SetPolyFT4(POLY_FT4& poly) noexcept;
void LIBGPU_SetSprt8(SPRT_8& sprite) noexcept;
void LIBGPU_SetSprt(SPRT & sprt) noexcept;
void LIBGPU_SetLineF2(LINE_F2& line) noexcept;

#if PSYDOOM_MODS
    void LIBGPU_SetFloorRowFT(FLOORROW_FT& row) noexcept;
    void LIBGPU_SetWallColFT(WALLCOL_FT& col) noexcept;
#endif

void LIBGPU_SetDumpFnt(const int32_t printStreamId) noexcept;
void LIBGPU_FntLoad(const int32_t dstX, const int32_t dstY) noexcept;

int32_t LIBGPU_FntOpen(
    const int32_t dispX,
    const int32_t dispY,
    const int32_t dispW,
    const int32_t dispH,
    const bool bClearBg,
    const int32_t maxChars
) noexcept;

void LIBGPU_FntFlush(const int32_t printStreamId) noexcept;
void LIBGPU_FntPrint(const int32_t printStreamId, const char* const fmtMsg, ...) noexcept;

uint16_t LIBGPU_LoadTPage(
    const void* pImageData,
    const int32_t bitDepth,
    const int32_t semiTransRate,
    const int32_t dstX,
    const int32_t dstY,
    const int32_t imgW,
    const int32_t imgH
) noexcept;

uint16_t LIBGPU_LoadClut(const uint16_t* pColors, const int32_t x, const int32_t y) noexcept;

DRAWENV& LIBGPU_SetDefDrawEnv(DRAWENV& env, const int32_t x, const int32_t y, const int32_t w, const  int32_t h) noexcept;
DISPENV& LIBGPU_SetDefDispEnv(DISPENV& disp, const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable/disable semi-transparency on the specified drawing primitive.
// Note: originally this was not a templated function and just modified the 7th byte (the 'code' field) on whatever you passed in.
// This version is more type safe and clearer in its intent.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PrimType>
void LIBGPU_SetSemiTrans(PrimType& prim, const bool bTransparent) noexcept {
    if (bTransparent) {
        prim.code |= 0x2;
    } else {
        prim.code &= 0xFD;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable or disable texture shading on a specified primitive; when shading is disabled, the texture is displayed as-is.
// Note: originally this was not a templated function and just modified the 7th byte (the 'code' field) on whatever you passed in.
// This version is more type safe and clearer in its intent.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PrimType>
void LIBGPU_SetShadeTex(PrimType& prim, const bool bDisableShading) noexcept {
    if (bDisableShading) {
        prim.code |= 1;
    } else {
        prim.code &= 0xFE;
    }
}

#if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom extension: set whether the drawing primitive discards pixels during texture mapping if all pixels are '0'.
// This is a GPU extension which can be used to disable masking for things like the sky.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PrimType>
void LIBGPU_SetDisableMasking(PrimType& prim, const bool bDisable) noexcept {
    if (bDisable) {
        prim.code |= 0x80;
    } else {
        prim.code &= 0x7F;
    }
}

#endif  // #if PSYDOOM_MODS

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
inline constexpr void LIBGPU_setUV0(T& prim, const LibGpuUV u0, const LibGpuUV v0) noexcept {
    prim.u0 = u0;
    prim.v0 = v0;
}

// Set the uv coordinates for a triangle draw primitive
template <class T>
inline constexpr void LIBGPU_setUV3(
    T& prim,
    const LibGpuUV u0,
    const LibGpuUV v0,
    const LibGpuUV u1,
    const LibGpuUV v1,
    const LibGpuUV u2,
    const LibGpuUV v2
) noexcept {
    prim.u0 = u0;
    prim.v0 = v0;
    prim.u1 = u1;
    prim.v1 = v1;
    prim.u2 = u2;
    prim.v2 = v2;
}

// Set the uv coordinates for a quad draw primitive
template <class T>
inline constexpr void LIBGPU_setUV4(
    T& prim,
    const LibGpuUV u0,
    const LibGpuUV v0,
    const LibGpuUV u1,
    const LibGpuUV v1,
    const LibGpuUV u2,
    const LibGpuUV v2,
    const LibGpuUV u3,
    const LibGpuUV v3
) noexcept {
    prim.u0 = u0;
    prim.v0 = v0;
    prim.u1 = u1;
    prim.v1 = v1;
    prim.u2 = u2;
    prim.v2 = v2;
    prim.u3 = u3;
    prim.v3 = v3;
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
//  (1) bitDepth : What format the texture is in.
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
    const int32_t bitDepth,
    const int32_t semiTransRate,
    const int32_t tpageX,
    const int32_t tpageY
) noexcept {
     return (uint16_t)(
        ((bitDepth & 0x3) << 7)|
        ((semiTransRate & 0x3) << 5)|
        ((tpageY & 0x100) >> 4)|
        ((tpageX & 0x3ff) >> 6)|
        ((tpageY & 0x200) << 2)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a primitive is the end of the primitives list
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline bool LIBGPU_IsEndPrim(const T& prim) noexcept {
    return ((prim.tag & 0x00FFFFFF) == 0x00FFFFFF);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Caps or marks the end of a primitive list.
// No further primitives after the given primitive are processed.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline void LIBGPU_TermPrim(T& prim) noexcept {
    prim.tag |= 0x00FFFFFF;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Debug font resources used by LIBGPU
//------------------------------------------------------------------------------------------------------------------------------------------
extern const uint8_t    gLIBGPU_DebugFont_Texture[2048];
extern const uint16_t   gLIBGPU_DebugFont_Clut[256];
