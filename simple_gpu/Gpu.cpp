#include "Gpu.h"

#include "Asserts.h"

#include <algorithm>

BEGIN_NAMESPACE(Gpu)

//------------------------------------------------------------------------------------------------------------------------------------------
// Rounds the given number up to the next power of two if it's not a power of two
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t nextPow2(const uint32_t numIn) noexcept {
    if (numIn == 0)
        return 0;

    uint32_t highestBit = 0;

    for (uint32_t bitsLeft = numIn >> 1; bitsLeft != 0; bitsLeft >>= 1) {
        ++highestBit;
    }

    const uint32_t floorPow2 = (uint32_t) 1u << highestBit;
    return (floorPow2 < numIn) ? floorPow2 << 1 : floorPow2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Core initialization and teardown
//------------------------------------------------------------------------------------------------------------------------------------------
void initCore(Core& core, const uint16_t ramPixelW, const uint16_t ramPixelH) noexcept {
    // Zero init everything by default, allocate the VRAM and then default some state
    ASSERT(ramPixelW > 0);
    ASSERT(ramPixelH > 0);

    core = {};

    core.pRam = new std::uint16_t[(size_t) ramPixelW * ramPixelH];
    std::memset(core.pRam, 0, sizeof(uint16_t) * ramPixelW * ramPixelH);    // Zero init VRAM

    core.ramPixelW = nextPow2(ramPixelW);
    core.ramPixelH = nextPow2(ramPixelH);
    core.ramXMask = ramPixelW - 1;
    core.ramYMask = ramPixelH - 1;

    core.drawAreaLx = 0;
    core.drawAreaRx = 256;
    core.drawAreaTy = 0;
    core.drawAreaBy = 240;
    core.displayAreaX = 0;
    core.displayAreaY = 0;
    core.displayAreaW = 256;
    core.displayAreaH = 240;

    core.texPageX = 256;
    core.texPageY = 0;
    core.texPageXMask = 0xFF;
    core.texPageXMask = 0xFF;
    core.texWinX = 0;
    core.texWinY = 0;
    core.texWinXMask = 0xFF;
    core.texWinYMask = 0xFF;

    core.blendMode = BlendMode::Alpha50;
    core.texFmt = TexFmt::Bpp16;
    core.clutX = 0;
    core.clutY = 240;
}

void destroyCore(Core& core) noexcept {
    delete[] core.pRam;
    core = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sanity checks GPU state in debug to make sure it is good for drawing
//------------------------------------------------------------------------------------------------------------------------------------------
static void sanityCheckGpuDrawState([[maybe_unused]] const Core& core) noexcept {
    // Drawing area cannot wrap around in RAM
    ASSERT(core.drawAreaRx < core.ramPixelW);
    ASSERT(core.drawAreaBy < core.ramPixelH);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a single unsigned 16-bit value from VRAM at the given absolute coordinate and wrap to VRAM boundaries
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t vramReadU16(const Core& core, const uint16_t x, const uint16_t y) noexcept {
    const uint16_t xt = x & core.ramXMask;
    const uint16_t yt = y & core.ramYMask;
    return core.pRam[yt * core.ramPixelW + xt];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a single unsigned 16-bit value to VRAM at the given absolute coordinate and wrap to VRAM boundaries
//------------------------------------------------------------------------------------------------------------------------------------------
void vramWriteU16(const Core& core, const uint16_t x, const uint16_t y, const uint16_t value) noexcept {
    const uint16_t xt = x & core.ramXMask;
    const uint16_t yt = y & core.ramYMask;
    core.pRam[yt * core.ramPixelW + xt] = value;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a texture using the given texture coordinate within the current texture page and window.
// This internal version is templated for higher performance.
//------------------------------------------------------------------------------------------------------------------------------------------
template <TexFmt TexFmt>
static Color16 readTexel(
    const Core& core,
    const uint16_t coordX,
    const uint16_t coordY
) noexcept {
    // First mask the coordinates by the texture window mask
    uint16_t vramX = coordX & core.texWinXMask;
    uint16_t vramY = coordY & core.texWinYMask;
    vramX += core.texWinX;
    vramY += core.texWinY;

    if constexpr (TexFmt == TexFmt::Bpp4) {
        vramX /= 4;
    } else if constexpr (TexFmt == TexFmt::Bpp8) {
        vramX /= 2;
    }

    vramX &= core.texPageXMask;
    vramY &= core.texPageYMask;
    vramX += core.texPageX;
    vramY += core.texPageY;

    // Read the VRAM pixel and then handle according to color mode
    const uint16_t vramPixel = vramReadU16(core, vramX, vramY);

    if constexpr (TexFmt == TexFmt::Bpp4) {
        // 4-bit color: each 16-bit pixel has 4 CLUT indexes. Get the CLUT index then lookup the color from the CLUT.
        const uint16_t clutIdx = (vramPixel >> ((coordX & 3) * 4)) & 0xF;
        return vramReadU16(core, core.clutX + clutIdx, core.clutY);
    }
    else if constexpr (TexFmt == TexFmt::Bpp8) {
        // 8-bit color: each 16-bit pixel has 2 CLUT indexes. Get the CLUT index then lookup the color from the CLUT.
        const uint16_t clutIdx = (vramPixel >> ((coordX & 1) * 8)) & 0xFF;
        return vramReadU16(core, core.clutX + clutIdx, core.clutY);
    }
    else {
        // Direct 16-bit color, no CLUT
        static_assert(TexFmt == TexFmt::Bpp16);
        return vramPixel;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a texel using dynamic dispatch and non-cached gpu state
//------------------------------------------------------------------------------------------------------------------------------------------
Color16 readTexel(const Core& core, const uint16_t coordX, const uint16_t coordY) noexcept {
    switch (core.texFmt) {
        case TexFmt::Bpp4:  return readTexel<TexFmt::Bpp4>(core, coordX, coordY);
        case TexFmt::Bpp8:  return readTexel<TexFmt::Bpp8>(core, coordX, coordY);

        default:
            ASSERT(core.texFmt == TexFmt::Bpp16);
            return readTexel<TexFmt::Bpp16>(core, coordX, coordY);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given pixel is inside the drawing area
//------------------------------------------------------------------------------------------------------------------------------------------
bool isPixelInDrawArea(const Core& core, const uint16_t x, const uint16_t y) noexcept {
    return (
        (x >= core.drawAreaLx) &&
        (x <= core.drawAreaRx) &&
        (y >= core.drawAreaTy) &&
        (y <= core.drawAreaBy)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears a region of VRAM to the specified color
//------------------------------------------------------------------------------------------------------------------------------------------
void clearRect(const Core& core, const Color16 color, const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h) noexcept {
    // Caching GPU state
    uint16_t* const pRam = core.pRam;
    const uint16_t ramPixelW = core.ramPixelW;
    const uint16_t ramPixelH = core.ramPixelH;

    // Clear the rectangle
    const uint16_t begX = std::min(x, ramPixelW);
    const uint16_t begY = std::min(y, ramPixelH);
    const uint16_t endX = (uint16_t) std::min(x + w, (int32_t) ramPixelW);
    const uint16_t endY = (uint16_t) std::min(y + h, (int32_t) ramPixelH);

    for (uint16_t y = begY; y < endY; ++y) {
        for (uint16_t x = begX; x < endX; ++x) {
            pRam[(uint32_t) y * ramPixelW + x] = color;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a 24-bit color to 16-bit.
// Note that the blending/semi-transparency flag is set to '0' during this conversion and overbright colors are saturated.
//------------------------------------------------------------------------------------------------------------------------------------------
Color16 color24FTo16(const Color24F colorIn) noexcept {
    Color16 colorOut;
    colorOut.comp.r = (uint16_t) std::min((uint32_t) colorIn.comp.r * 31u / 128u, 31u);
    colorOut.comp.g = (uint16_t) std::min((uint32_t) colorIn.comp.g * 31u / 128u, 31u);
    colorOut.comp.b = (uint16_t) std::min((uint32_t) colorIn.comp.b * 31u / 128u, 31u);
    return colorOut;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Modulate a 16-bit color by a 24-bit one where the components are in 1.7 fixed point format
//------------------------------------------------------------------------------------------------------------------------------------------
Color16 colorMul(const Color16 color1, const Color24F color2) noexcept {
    Color16 out;
    out.comp.r = (uint16_t) std::min((color1.comp.r * color2.comp.r) >> 7, 31);
    out.comp.g = (uint16_t) std::min((color1.comp.g * color2.comp.g) >> 7, 31);
    out.comp.b = (uint16_t) std::min((color1.comp.b * color2.comp.b) >> 7, 31);
    out.comp.t = color1.comp.t;
    return out;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Blend the two colors (foreground and background) and return the result
//------------------------------------------------------------------------------------------------------------------------------------------
Color16 colorBlend(const Color16 bg, const Color16 fg, const BlendMode mode) noexcept {
    Color16 result = fg;

    switch (mode) {
        case BlendMode::Alpha50:
            result.comp.r = (uint16_t)((bg.comp.r + fg.comp.r) >> 1);
            result.comp.g = (uint16_t)((bg.comp.g + fg.comp.g) >> 1);
            result.comp.b = (uint16_t)((bg.comp.b + fg.comp.b) >> 1);
            break;

        case BlendMode::Add:
            result.comp.r = (uint16_t) std::min(bg.comp.r + fg.comp.r, 31);
            result.comp.g = (uint16_t) std::min(bg.comp.g + fg.comp.g, 31);
            result.comp.b = (uint16_t) std::min(bg.comp.b + fg.comp.b, 31);
            break;

        case BlendMode::Subtract:
            result.comp.r = (uint16_t) std::max((int32_t) bg.comp.r - (int32_t) fg.comp.r, 0);
            result.comp.g = (uint16_t) std::max((int32_t) bg.comp.g - (int32_t) fg.comp.g, 0);
            result.comp.b = (uint16_t) std::max((int32_t) bg.comp.b - (int32_t) fg.comp.b, 0);
            break;

        case BlendMode::Add25:
            result.comp.r = (uint16_t) std::min(bg.comp.r + (fg.comp.r >> 2), 31);
            result.comp.g = (uint16_t) std::min(bg.comp.g + (fg.comp.g >> 2), 31);
            result.comp.b = (uint16_t) std::min(bg.comp.b + (fg.comp.b >> 2), 31);
            break;
    }

    return result;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing a rectangle - internal implementation tailored to each texture format
//------------------------------------------------------------------------------------------------------------------------------------------
template <DrawMode DrawMode, TexFmt TexFmt>
static void draw(const Core& core, const DrawRect& rect) noexcept {
    sanityCheckGpuDrawState(core);

    // According to the NO$PSX specs rectangle sizes cannot exceed 1023 x 511
    if ((rect.w >= 1024) || (rect.h >= 512))
        return;

    // Clip the rectangle bounds to the draw area and generate adjustments to the uv coords if that happens.
    // Note must translate the rectangle according to the draw offset too...
    const uint16_t rectTx = rect.x + core.drawOffsetX;
    const uint16_t rectTy = rect.y + core.drawOffsetY;

    uint16_t begX = rectTx;
    uint16_t begY = rectTy;
    uint16_t topLeftU = rect.u;
    uint16_t topLeftV = rect.v;

    if (begX < core.drawAreaLx) {
        topLeftU += core.drawAreaLx - begX;
        begX = core.drawAreaLx;
    }

    if (begY < core.drawAreaTy) {
        topLeftV += core.drawAreaTy - begY;
        begY = core.drawAreaTy;
    }

    const uint16_t endX = std::min(rectTx + rect.w, core.drawAreaRx + 1);
    const uint16_t endY = std::min(rectTy + rect.h, core.drawAreaBy + 1);

    // If we are in flat colored mode then decide the foreground color for every pixel in the rectangle
    const Color24F rectColor = rect.color;
    Color16 fgColor;

    if constexpr ((DrawMode == DrawMode::FlatColored) || (DrawMode == DrawMode::FlatColoredBlended)) {
        fgColor = color24FTo16(rectColor);
    }

    // Fill in the rectangle pixels
    uint16_t curV = topLeftV;

    for (uint16_t y = begY; y < endY; ++y, ++curV) {
        uint16_t curU = topLeftU;

        for (uint16_t x = begX; x < endX; ++x, ++curU) {
            // Get the foreground color for the rectangle pixel if the rectangle is textured.
            // If the pixel is transparent then also skip it, otherwise modulate it by the primitive color...
            if constexpr ((DrawMode == DrawMode::Textured) || (DrawMode == DrawMode::TexturedBlended)) {
                fgColor = readTexel<TexFmt>(core, curU, curV);

                if (fgColor.bits == 0)
                    continue;

                fgColor = colorMul(fgColor, rectColor);
            }

            // Do blending with the background if that is enabled
            if constexpr ((DrawMode == DrawMode::FlatColoredBlended) || (DrawMode == DrawMode::TexturedBlended)) {
                const Color16 bgColor = vramReadU16(core, x, y);
                fgColor = colorBlend(bgColor, fgColor, core.blendMode);
            }

            // Save the output pixel
            vramWriteU16(core, x, y, fgColor);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing a rectangle - external interface
//------------------------------------------------------------------------------------------------------------------------------------------
template <DrawMode DrawMode>
void draw(const Core& core, const DrawRect& rect) noexcept {
    if (core.texFmt == TexFmt::Bpp4) {
        draw<DrawMode, TexFmt::Bpp4>(core, rect);
    } else if (core.texFmt == TexFmt::Bpp8) {
        draw<DrawMode, TexFmt::Bpp8>(core, rect);
    } else {
        ASSERT(core.texFmt == TexFmt::Bpp16);
        draw<DrawMode, TexFmt::Bpp16>(core, rect);
    }
}

// Instantiate the variants of this function
template void draw<DrawMode::FlatColored>(const Core& core, const DrawRect& rect) noexcept;
template void draw<DrawMode::FlatColoredBlended>(const Core& core, const DrawRect& rect) noexcept;
template void draw<DrawMode::Textured>(const Core& core, const DrawRect& rect) noexcept;
template void draw<DrawMode::TexturedBlended>(const Core& core, const DrawRect& rect) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing a line
//------------------------------------------------------------------------------------------------------------------------------------------
template <DrawMode DrawMode>
void draw(const Core& core, const DrawLine& line) noexcept {
    sanityCheckGpuDrawState(core);

    // Translate the line by the drawing offset
    const int32_t lineX1 = line.x1 + core.drawOffsetX;
    const int32_t lineY1 = line.y1 + core.drawOffsetY;
    const int32_t lineX2 = line.x2 + core.drawOffsetX;
    const int32_t lineY2 = line.y2 + core.drawOffsetY;

    // Firstly get the component deltas
    const int32_t dx = (int32_t) lineX2 - lineX1;
    const int32_t dy = (int32_t) lineY2 - lineY1;
    const int32_t absDx = std::abs(dx);
    const int32_t absDy = std::abs(dy);

    // According to the NO$PSX specs line distances cannot exceed 1023 x 511
    if ((absDx >= 1024) || (absDy >= 512))
        return;

    // Get the color to shade the line with
    const Color16 lineColor = color24FTo16(line.color);

    // This is Bresenham's line drawing algorithm. In order to handle all 4 cases of line we swap components as required so that the
    // delta error increment is on the dimension with the smallest change in the line. Also we insure the primary dimension with the
    // largest increment always steps in the positive direction:
    bool bLineIsSteep;

    int32_t a1;
    int32_t a2;
    int32_t b1;
    int32_t b2;

    if (absDy > absDx) {
        bLineIsSteep = true;
        a1 = lineY1;
        a2 = lineY2;
        b1 = lineX1;
        b2 = lineX2;
    } else {
        bLineIsSteep = false;
        a1 = lineX1;
        a2 = lineX2;
        b1 = lineY1;
        b2 = lineY2;
    }

    if (a1 > a2) {
        std::swap(a1, a2);
        std::swap(b1, b2);
    }

    // Compute the deltas and delta error per pixel as per Bresenham's algorithm
    const int32_t da = a2 - a1;
    const int32_t db = b2 - b1;
    const int32_t derror = std::abs(db) * 2;
    const int32_t bstep = (db < 0) ? -1 : +1;

    int32_t error = 0;
    int32_t b = b1;

    // Plot pixels: this loop could be optimized more and clipping could be employed but Doom doesn't render lines too much.
    // It's probably not worth the effort going crazy on this...
    constexpr bool bBlend = (DrawMode == DrawMode::FlatColoredBlended);
    const BlendMode blendMode = core.blendMode;

    for (int32_t a = a1; a <= a2; ++a) {
        const int32_t x = (bLineIsSteep) ? b : a;
        const int32_t y = (bLineIsSteep) ? a : b;

        if (isPixelInDrawArea(core, x, y)) {
            const Color16 color = (bBlend) ? colorBlend(vramReadU16(core, x, y), lineColor, blendMode) : lineColor;
            vramWriteU16(core, x, y, color);
        }

        // Time to step the minor change dimension according to Bresenham's algorithm?
        error += derror;

        if (error > da) {
            b += bstep;
            error -= da * 2;
        }
    }
}

// Instantiate the variants of this function
template void draw<DrawMode::FlatColored>(const Core& core, const DrawLine& line) noexcept;
template void draw<DrawMode::FlatColoredBlended>(const Core& core, const DrawLine& line) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing a triangle - internal implementation tailored to each texture format. Follows the rasterization steps outlined here:
//  https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
//------------------------------------------------------------------------------------------------------------------------------------------
template <DrawMode DrawMode, TexFmt TexFmt>
static void draw(const Core& core, const DrawTriangle& triangle) noexcept {
    sanityCheckGpuDrawState(core);
    
    // Apply the draw offset to the triangle points
    const int16_t drawOffsetX = core.drawOffsetX;
    const int16_t drawOffsetY = core.drawOffsetY;
    const int32_t p1x = triangle.x1 + drawOffsetX;
    const int32_t p1y = triangle.y1 + drawOffsetY;
    const int32_t p2x = triangle.x2 + drawOffsetX;
    const int32_t p2y = triangle.y2 + drawOffsetY;
    const int32_t p3x = triangle.x3 + drawOffsetX;
    const int32_t p3y = triangle.y3 + drawOffsetY;

    // Compute the rectangular area of the triangle to be rasterized.
    // Note that according to the No$PSX specs the right and bottom coordinates in polygons are NOT included.
    // Also, the triangle is skipped if the distances between the vertices exceed 1023x511 in the x and y dimensions.
    const int32_t minX = std::min(std::min(p1x, p2x), p3x);
    const int32_t minY = std::min(std::min(p1y, p2y), p3y);
    const int32_t maxX = std::max(std::max(p1x, p2x), p3x);
    const int32_t maxY = std::max(std::max(p1y, p2y), p3y);

    if ((maxX - minX >= 1024) || (maxY - minY) >= 512)
        return;

    const int32_t lx = std::max((int32_t) core.drawAreaLx, minX);
    const int32_t rx = std::min((int32_t) core.drawAreaRx, maxX - 1);
    const int32_t ty = std::max((int32_t) core.drawAreaTy, minY);
    const int32_t by = std::min((int32_t) core.drawAreaBy, maxY - 1);

    // Precompute the edge deltas used in the edge functions
    const float p1xf = (float) p1x;     const float p1yf = (float) p1y;
    const float p2xf = (float) p2x;     const float p2yf = (float) p2y;
    const float p3xf = (float) p3x;     const float p3yf = (float) p3y;

    const float e1dx = p2xf - p1xf;
    const float e1dy = p2yf - p1yf;
    const float e2dx = p3xf - p2xf;
    const float e2dy = p3yf - p2yf;
    const float e3dx = p1xf - p3xf;
    const float e3dy = p1yf - p3yf;

    // If we are in flat colored mode then decide the foreground color for every pixel in the triangle
    const Color24F triangleColor = triangle.color;
    Color16 fgColor;

    if constexpr ((DrawMode == DrawMode::FlatColored) || (DrawMode == DrawMode::FlatColoredBlended)) {
        fgColor = color24FTo16(triangleColor);
    }

    // Cache the uv coords
    const float u1 = triangle.u1;    const float v1 = triangle.v1;
    const float u2 = triangle.u2;    const float v2 = triangle.v2;
    const float u3 = triangle.u3;    const float v3 = triangle.v3;

    // Process each pixel
    for (int32_t y = ty; y <= by; ++y) {
        // Cache the pointer to the destination row in VRAM
        uint16_t* const pRamRow = core.pRam + ((intptr_t) y * core.ramPixelW);

        // Compute the portions of the edge function that are constant with respect to Y.
        // See below comments for more details on the edge function:
        const float yf = (float) y;
        const float ef1_y = -(p1xf * e1dy + (yf - p1yf) * e1dx);
        const float ef2_y = -(p2xf * e2dy + (yf - p2yf) * e2dx);
        const float ef3_y = -(p3xf * e3dy + (yf - p3yf) * e3dx);

        for (int32_t x = lx; x <= rx; ++x) {
            // Compute the 'edge function' or the magnitude of the cross product between an edge and a vector from this point.
            // This value for all 3 edges tells us whether the point is inside the triangle, and also lets us compute barycentric coordinates.
            const float xf = (float) x;
            const float ef1 = xf * e1dy + ef1_y;
            const float ef2 = xf * e2dy + ef2_y;
            const float ef3 = xf * e3dy + ef3_y;

            // The point is inside the triangle if the sign of all edge functions matches.
            // This handles triangles that are wound the opposite way.
            const bool bSign1 = (ef1 <= 0);
            const bool bSign2 = (ef2 <= 0);
            const bool bSign3 = (ef3 <= 0);

            if ((bSign1 != bSign2) || (bSign2 != bSign3))
                continue;

            // Compute the total signed triangle area and from that the weights of each vertex
            const float triArea = ef1 + ef2 + ef3;
            const float invTriArea = 1.0f / triArea;

            const float w1 = ef2 * invTriArea;
            const float w2 = ef3 * invTriArea;
            const float w3 = ef1 * invTriArea;

            // Compute the texture coordinate to use and do ceil() type round operation - this produces more pleasing results for floor/ceiling spans in Doom
            const uint16_t u = (uint16_t)(u1 * w1 + u2 * w2 + u3 * w3 + 0.999f);
            const uint16_t v = (uint16_t)(v1 * w1 + v2 * w2 + v3 * w3 + 0.999f);

            // Get the foreground color for the triangle pixel if the triangle is textured.
            // If the pixel is transparent then also skip it, otherwise modulate it by the primitive color...
            if constexpr ((DrawMode == DrawMode::Textured) || (DrawMode == DrawMode::TexturedBlended)) {
                fgColor = readTexel<TexFmt>(core, u, v);

                if (fgColor.bits == 0)
                    continue;
                
                fgColor = colorMul(fgColor, triangleColor);
            }

            // Do blending with the background if that is enabled
            if constexpr ((DrawMode == DrawMode::FlatColoredBlended) || (DrawMode == DrawMode::TexturedBlended)) {
                const Color16 bgColor = vramReadU16(core, x, y);
                fgColor = colorBlend(bgColor, fgColor, core.blendMode);
            }

            // Save the output pixel
            pRamRow[x] = fgColor;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing a triangle - external interface
//------------------------------------------------------------------------------------------------------------------------------------------
template <DrawMode DrawMode>
void draw(const Core& core, const DrawTriangle& triangle) noexcept {
    if (core.texFmt == TexFmt::Bpp4) {
        draw<DrawMode, TexFmt::Bpp4>(core, triangle);
    } else if (core.texFmt == TexFmt::Bpp8) {
        draw<DrawMode, TexFmt::Bpp8>(core, triangle);
    } else {
        ASSERT(core.texFmt == TexFmt::Bpp16);
        draw<DrawMode, TexFmt::Bpp16>(core, triangle);
    }
}

// Instantiate the variants of this function
template void draw<DrawMode::FlatColored>(const Core& core, const DrawTriangle& triangle) noexcept;
template void draw<DrawMode::FlatColoredBlended>(const Core& core, const DrawTriangle& triangle) noexcept;
template void draw<DrawMode::Textured>(const Core& core, const DrawTriangle& triangle) noexcept;
template void draw<DrawMode::TexturedBlended>(const Core& core, const DrawTriangle& triangle) noexcept;

END_NAMESPACE(Gpu)
