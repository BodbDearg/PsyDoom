#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// PlayStation 1 GPU emulation: simplified.
// This is a heavily stripped down implementation of a PlayStation 1 style GPU specifically for PsyDoom.
// 
// Simplifications made for this GPU include:
//  (1) The removal of all links to an emulated PlayStation system, including DMA and interrupts etc.
//  (2) All memory transfer stuff, status registers and I/O registers are removed, the host game can just access everything directly.
//  (3) This GPU does not concern itself with output format or video timings (PAL vs NTSC) - it just stores the VRAM region being displayed.
//  (4) The output display format is assumed to be 15-bit color, 24-bit color is not supported.
//  (5) Dithering is not supported, since Doom did not use this at all.
//  (6) Gouraud shaded primitives are not supported, Doom did not use these.
//  (7) The GPU 'mask bit' for masking pixels is not supported, Doom did not use this.
//  (8) X and Y flipping textures is not supported; original PS1 models did not have this anyway so games could not use it.
//  (9) All rendering/command primitives are fed directly to the GPU and handled immediately - command buffers are not supported.
//  (10) The full range of draw primitives exposed by the original LIBGPU is NOT provided, only the ones that Doom uses.
//  (11) Various not that useful bits of GPU state have been removed, for example the 'display enable' flag (originally in the status reg)
//------------------------------------------------------------------------------------------------------------------------------------------
BEGIN_NAMESPACE(Gpu)

// Represents a 'semi-transparency' or blending mode for the GPU
enum class BlendMode : uint8_t {
    Alpha50,    // 50% opacity alpha blend (dst/2 + src/2)
    Add,        // Additive blend at 100% opacity (dst + src)
    Subtract,   // Subtractive blend at 100% opacity (dst - src)
    Add25       // Additive blend at 25% opacity (dst + src/4)
};

// Texture format used by the GPU
enum class TexFmt : uint8_t {
    Bpp4,       // 4-bits per pixel color indexed (using a CLUT)
    Bpp8,       // 8-bits per pixel color indexed (using a CLUT)
    Bpp16,      // 15-bit direct RGB color plus a 1-bit semi-transparency flag (16-bits per pixel overall)
};

// The GPU core/device itself
struct Core {
    std::byte*      pRam;               // The VRAM for the GPU
    uint16_t        ramPixelW;          // The width of VRAM (in terms of 16-bit/2-byte pixels)
    uint16_t        ramPixelH;          // The height of VRAM (in terms of 16-bit/2-byte pixels)
    int16_t         drawOffsetX;        // X offset added to vertices before rasterizing, brings the geometry into the area of VRAM being drawn to
    int16_t         drawOffsetY;        // Y offset added to vertices before rasterizing, brings the geometry into the area of VRAM being drawn to
    uint16_t        drawAreaLx;         // The area of VRAM being drawn to (left X, inclusive)
    uint16_t        drawAreaRx;         // The area of VRAM being drawn to (right X, inclusive)
    uint16_t        drawAreaTy;         // The area of VRAM being drawn to (top Y, inclusive)
    uint16_t        drawAreaBy;         // The area of VRAM being drawn to (bottom Y, inclusive)
    uint16_t        displayAreaX;       // The area of VRAM being displayed (top left X)
    uint16_t        displayAreaY;       // The area of VRAM being displayed (top left Y)
    uint16_t        displayAreaW;       // The area of VRAM being displayed (width)
    uint16_t        displayAreaH;       // The area of VRAM being displayed (height)
    uint16_t        texPageX;           // Location of the area used for texturing (top left X)
    uint16_t        texPageY;           // Location of the area used for texturing (top left Y)
    uint16_t        texPageXMask;       // Mask used to wrap X coordinates to be within the texture page (e.g 0xFF for 256 pixel wrap)
    uint16_t        texPageYMask;       // Mask used to wrap Y coordinates to be within the texture page (e.g 0xFF for 256 pixel wrap)
    uint16_t        texWinX;            // Location of a window within the texture page to use for texturing (top left X)
    uint16_t        texWinY;            // Location of a window within the texture page to use for texturing (top left Y)
    uint16_t        texWinXMask;        // Masks X coordinates to be within the texture window (e.g 0xF for 16 pixel wrap)
    uint16_t        texWinYMask;        // Masks Y coordinates to be within the texture window (e.g 0xF for 16 pixel wrap)
    BlendMode       blendMode;          // Blend mode for blended/semi-transparent geometry
    TexFmt          texFmt;             // Current texture format in use
    uint16_t        clutX;              // X position of the current CLUT/color-index table in vram pixels (CLUT is arranged in a row at this location)
    uint16_t        clutY;              // X position of the current CLUT/color-index table in vram pixels (CLUT is arranged in a row at this location)
};

END_NAMESPACE(Gpu)
