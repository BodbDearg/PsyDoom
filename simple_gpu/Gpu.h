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
//  (7) Drawing primitives are always modulated by the primitive color, there is no mode where this does not happen.
//  (8) The GPU 'mask bit' for masking pixels is not supported, Doom did not use this.
//  (9) X and Y flipping textures is not supported; original PS1 models did not have this anyway so games could not use it.
//  (10) All rendering/command primitives are fed directly to the GPU and handled immediately - command buffers are not supported.
//  (11) Only rectangles, lines and triangle primitives are supported. Quads must be decomposed externally into triangles.
//  (12) The full range of draw primitives exposed by the original LIBGPU is NOT provided, only the ones that Doom uses.
//  (13) Various not that useful bits of GPU state have been removed, for example the 'display enable' flag (originally in the status reg)
//  (14) The drawing and display areas must not wrap around in VRAM, it is assumed they do not.
//  (15) CLUTs are not allowed to wrap around in VRAM, it is assumed they do not.
//
// There are some improvements over an original PS1 GPU also, which can allow extended capabilities:
//  (1) The texture window and page can exceed 256x256 units.
//  (2) VRAM can be made bigger than the standard 1024x512 pixels.
//  (3) Texture coordinates are now 16-bit, which allows for (1) to be taken advantage of.
//------------------------------------------------------------------------------------------------------------------------------------------
BEGIN_NAMESPACE(Gpu)

// The original VRAM width and height (in 16-bit pixels) for the PS1
static constexpr uint16_t PS1_VRAM_W = 1024;
static constexpr uint16_t PS1_VRAM_H = 512;

//----------------------------------------------------------------------------------------------------------------------
// Represents a 24-bit RGB888 color used by the GPU with each component in 1.7 fixed point format.
// This is used as an intermediate color strength or multiplier for GPU commands and rendering.
// The value '128' is equal to 1.0 or full strength and values over that are 'overbright'.
//----------------------------------------------------------------------------------------------------------------------
union Color24F {
    // The individual components of the color
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t x;      // Unused component/padding
    } comp;

    // The full 32-bits of the color (8-bits are padding)
    uint32_t bits;

    inline Color24F() noexcept : bits(0) {}
    inline Color24F(const uint32_t bits) noexcept : bits(bits) {}
    inline operator uint32_t() const noexcept { return bits; }
};

//----------------------------------------------------------------------------------------------------------------------
// Represents a 15-bit RGB555 color used by the GPU plus 1 bit that says whether blending is enabled for the pixel.
// This is used as a dest/output format for the framebuffer and also as an input format for 16-bit textures & CLUTs.
//----------------------------------------------------------------------------------------------------------------------
union Color16 {
    // The individual components of the color
    struct {
        uint16_t r : 5;
        uint16_t g : 5;
        uint16_t b : 5;
        uint16_t t : 1;     // Blending/semi-transparency flag: if set then blend the pixel, if drawing a blended primitive
    } comp;

    // The full 16-bits of the color
    uint16_t bits;

    inline Color16() noexcept : bits(0) {}
    inline Color16(const uint16_t bits) noexcept : bits(bits) {}
    inline operator uint16_t() const noexcept { return bits; }
};

//----------------------------------------------------------------------------------------------------------------------
// Represents a 'semi-transparency' or blending mode for the GPU between foreground (fg) and background (bg) colors
//----------------------------------------------------------------------------------------------------------------------
enum class BlendMode : uint8_t {
    Alpha50,        // 50% opacity alpha blend (bg/2 + fg/2)
    Add,            // Additive blend at 100% opacity (bg + fg)
    Subtract,       // Subtractive blend at 100% opacity (bg - fg)
    Add25           // Additive blend at 25% opacity (bg + fg/4)
};

//----------------------------------------------------------------------------------------------------------------------
// Texture format used by the GPU
//----------------------------------------------------------------------------------------------------------------------
enum class TexFmt : uint8_t {
    Bpp4,           // 4-bits per pixel color indexed (using a CLUT)
    Bpp8,           // 8-bits per pixel color indexed (using a CLUT)
    Bpp16,          // 15-bit direct RGB color plus a 1-bit semi-transparency flag (16-bits per pixel overall)
};

//----------------------------------------------------------------------------------------------------------------------
// What type of drawing to do
//----------------------------------------------------------------------------------------------------------------------
enum class DrawMode : uint8_t {
    FlatColored,            // Draw the geometry in a single color with no blending
    FlatColoredBlended,     // Draw the geometry with a single color and blending
    Textured,               // Draw the geometry textured with color modulation by a single color (no blending)
    TexturedBlended,        // Draw the geometry textured with color modulation by a single color (blending enabled)
};

//----------------------------------------------------------------------------------------------------------------------
// GPU drawing primitives: rectangles, lines and triangles
//----------------------------------------------------------------------------------------------------------------------
struct DrawRect {
    int16_t     x;          // Position of rectangle: x
    int16_t     y;          // Position of rectangle: y
    uint16_t    w;          // Width of rectangle. Note: not allowed to exceed 1023!
    uint16_t    h;          // Height of rectangle. Note: not allowed to exceed 511!
    uint16_t    u;          // Top left texcoord: u
    uint16_t    v;          // Top right texcoord: v
    Color24F    color;      // Color to shade the rectangle with
};

struct DrawLine {
    int16_t     x1;         // Line point 1: x
    int16_t     y1;         // Line point 1: y
    int16_t     x2;         // Line point 2: x
    int16_t     y2;         // Line point 2: y
    Color24F    color;      // Color to draw the line with
};

struct DrawTriangle {
    int16_t     x1;         // Triangle point 1: x
    int16_t     y1;         // Triangle point 1: y
    int16_t     u1;         // Triangle point 1: u texcoord
    int16_t     v1;         // Triangle point 1: v texcoord
    int16_t     x2;         // Triangle point 2: x
    int16_t     y2;         // Triangle point 2: y
    int16_t     u2;         // Triangle point 2: u texcoord
    int16_t     v2;         // Triangle point 2: v texcoord
    int16_t     x3;         // Triangle point 3: x
    int16_t     y3;         // Triangle point 3: y
    int16_t     u3;         // Triangle point 3: u texcoord
    int16_t     v3;         // Triangle point 3: v texcoord
    Color24F    color;      // Color to draw the triangle with
};

//----------------------------------------------------------------------------------------------------------------------
// The GPU core/device itself
//----------------------------------------------------------------------------------------------------------------------
struct Core {
    uint16_t*       pRam;               // The VRAM for the GPU: declared as an array of 16-bit/2-byte pixels
    uint16_t        ramPixelW;          // The width of VRAM (in terms of 16-bit/2-byte pixels) - always a power of 2
    uint16_t        ramPixelH;          // The height of VRAM (in terms of 16-bit/2-byte pixels) - always a power of 2
    uint16_t        ramXMask;           // A mask which wraps coordinates to be inside of VRAM
    uint16_t        ramYMask;           // A mask which wraps coordinates to be inside of VRAM
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
    uint16_t        texPageX;           // Location of the area used for texturing (top left X, in terms of 16-bit pixels)
    uint16_t        texPageY;           // Location of the area used for texturing (top left Y, in terms of 16-bit pixels)
    uint16_t        texPageXMask;       // Mask used to wrap X coordinates to be within the texture page (e.g 0xFF for 256 pixel wrap, in terms of 16-bit pixels)
    uint16_t        texPageYMask;       // Mask used to wrap Y coordinates to be within the texture page (e.g 0xFF for 256 pixel wrap, in terms of 16-bit pixels)
    uint16_t        texWinX;            // Location of a window within the texture page to use for texturing (top left X, in terms of 16-bit pixels)
    uint16_t        texWinY;            // Location of a window within the texture page to use for texturing (top left Y, in terms of 16-bit pixels)
    uint16_t        texWinXMask;        // Masks X coordinates to be within the texture window (e.g 0xF for 16 pixel wrap, in terms of 16-bit pixels)
    uint16_t        texWinYMask;        // Masks Y coordinates to be within the texture window (e.g 0xF for 16 pixel wrap, in terms of 16-bit pixels)
    BlendMode       blendMode;          // Blend mode for blended/semi-transparent geometry
    TexFmt          texFmt;             // Current texture format in use
    uint16_t        clutX;              // X position of the current CLUT/color-index table in 16-bit VRAM pixels (CLUT is arranged in a row at this location)
    uint16_t        clutY;              // Y position of the current CLUT/color-index table in 16-bit VRAM pixels (CLUT is arranged in a row at this location)

    // CLUT cache to speed up texture mapping and the settings it was last saved with
    uint16_t        clutCacheX;
    uint16_t        clutCacheY;
    TexFmt          clutCacheFmt;
    Color16         clutCache[256];
};

// Initializing and shutting down a core
void initCore(Core& core, const uint16_t ramPixelW, const uint16_t ramPixelH) noexcept;
void destroyCore(Core& core) noexcept;

// VRAM reading
uint16_t vramReadU16(const Core& core, const uint16_t x, const uint16_t y) noexcept;
void vramWriteU16(Core& core, const uint16_t x, const uint16_t y, const uint16_t value) noexcept;
Color16 readTexel(Core& core, const uint16_t coordX, const uint16_t coordY) noexcept;

// Miscellaneous
void updateClutCache(Core& core) noexcept;
bool isPixelInDrawArea(const Core& core, const uint16_t x, const uint16_t y) noexcept;
void clearRect(Core& core, const Color16 color, const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h) noexcept;

// Color manipulation
Color16 color24FTo16(const Color24F colorIn) noexcept;
Color16 colorMul(const Color16 color1, const Color24F color2) noexcept;
Color16 colorBlend(const Color16 bg, const Color16 fg, const BlendMode mode) noexcept;

// Drawing functions: note that lines CANNOT be textured!
template <DrawMode DrawMode>
void draw(Core& core, const DrawRect& rect) noexcept;

template <DrawMode DrawMode>
void draw(Core& core, const DrawLine& line) noexcept;

template <DrawMode DrawMode>
void draw(Core& core, const DrawTriangle& triangle) noexcept;

END_NAMESPACE(Gpu)
