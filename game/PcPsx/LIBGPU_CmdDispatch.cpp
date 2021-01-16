#include "LIBGPU_CmdDispatch.h"

#include "Asserts.h"
#include "Gpu.h"
#include "PsxVm.h"
#include "Video.h"
#include "Vulkan/VDrawing.h"
#include "Vulkan/VRenderer.h"

BEGIN_NAMESPACE(LIBGPU_CmdDispatch)

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the GPU texture page, texture format, and blending (semi-transparency) mode from a 16-bit word as encoded by LIBGPU
//------------------------------------------------------------------------------------------------------------------------------------------
void setGpuTexPageId(const uint16_t texPageId) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Set texture format
    switch ((texPageId >> 7) & 0x3) {
        case 0: gpu.texFmt = Gpu::TexFmt::Bpp4;     break;
        case 1: gpu.texFmt = Gpu::TexFmt::Bpp8;     break;
        case 2: gpu.texFmt = Gpu::TexFmt::Bpp16;    break;
        default: break;
    }

    // Set texture page position and size (256x256 pixels)
    gpu.texPageX = ((texPageId & 0x0F) >> 0) * 64;
    gpu.texPageY = ((texPageId & 0x10) >> 4) * 256;
    gpu.texPageXMask = 0xFF;
    gpu.texPageYMask = 0xFF;

    // Set blend/semi-transparency mode
    switch ((texPageId >> 5) & 0x3) {
        case 0: gpu.blendMode = Gpu::BlendMode::Alpha50;    break;
        case 1: gpu.blendMode = Gpu::BlendMode::Add;        break;
        case 2: gpu.blendMode = Gpu::BlendMode::Subtract;   break;
        case 3: gpu.blendMode = Gpu::BlendMode::Add25;      break;
        default: break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the GPU CLUT position, from a 16-bit word as encoded by LIBGPU
//------------------------------------------------------------------------------------------------------------------------------------------
void setGpuClutId(const uint16_t clutId) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;
    const uint16_t clutX = (clutId & 0x3Fu) << 4;       // Clut X position is restricted to multiples of 16
    const uint16_t clutY = (clutId >> 6u) & 0xFFFu;
    gpu.clutX = clutX;
    gpu.clutY = clutY;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the GPU texture window from a 32-bit word as encoded by LIBGPU
//------------------------------------------------------------------------------------------------------------------------------------------
void setGpuTexWin(const uint32_t texWin) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Note: all offsets are in multiples of 8 units.
    // The masks are also always for power of two textures.
    // Both these restrictions are required by the PsyQ SDK when encoding a texture window.
    const uint8_t twOffsetX = (texWin >> 10) & 0x1Fu;
    const uint8_t twOffsetY = (texWin >> 15) & 0x1Fu;
    const uint8_t twMaskX = (texWin >> 0) & 0x1Fu;
    const uint8_t twMaskY = (texWin >> 5) & 0x1Fu;

    gpu.texWinX = twOffsetX * 8;
    gpu.texWinY = twOffsetY * 8;

    if (twMaskX != 0) {
        const uint8_t texW = (uint8_t) -(int8_t)(twMaskX << 3);
        gpu.texWinXMask = texW - 1;
    } else {
        gpu.texWinXMask = 0xFFu;    // 256 pixel range
    }

    if (twMaskY != 0) {
        const uint8_t texH = (uint8_t) -(int8_t)(twMaskY << 3);
        gpu.texWinYMask = texH - 1;
    } else {
        gpu.texWinYMask = 0xFFu;    // 256 pixel range
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command primitive to set the drawing mode: sets the texture page and window.
// Originally this command would have also set the dithering and 'draw in display area' flag but both of those are no longer supported by
// PsyDoom's new PSX GPU implementation, so we ignore those aspects of the command.
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const DR_MODE& drawMode) noexcept {
    const uint16_t texPageId = drawMode.code[0] & 0x9FF;
    setGpuTexPageId(texPageId);
    setGpuTexWin(drawMode.code[1]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command primitive to set the drawing mode: sets the texture page and window
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const DR_TWIN& texWin) noexcept {
    setGpuTexWin(texWin.code[0]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw a variable sized sprite
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const SPRT& sprite) noexcept { 
    Gpu::Core& gpu = PsxVm::gGpu;

    // Set the CLUT to use
    setGpuClutId(sprite.clut);

    // Setup the rectangle to be drawn then submit to the GPU
    const bool bColorSprite = ((sprite.code & 0x1) == 0);
    const bool bBlendSprite = sprite.code & 0x2;

    Gpu::DrawRect drawRect = {};
    drawRect.x = sprite.x0;
    drawRect.y = sprite.y0;
    drawRect.w = sprite.w;
    drawRect.h = sprite.h;
    drawRect.u = sprite.tu0;
    drawRect.v = sprite.tv0;
    drawRect.color.comp.r = (bColorSprite) ? sprite.r0 : 128;   // Note: '128' is '1.0' or full strength color if we don't want to modulate
    drawRect.color.comp.g = (bColorSprite) ? sprite.g0 : 128;
    drawRect.color.comp.b = (bColorSprite) ? sprite.b0 : 128;

    // Are we using the Vulkan renderer? If so submit via that.
    // This allows us to re-use a lot of the old PSX 2D rendering without any changes.
    #if PSYDOOM_VULKAN_RENDERER
        if (Video::usingVulkanRenderer()) {
            ASSERT_LOG(gpu.blendMode == Gpu::BlendMode::Alpha50, "Only alpha blending is supported for PSX renderer sprites forwarded to Vulkan!");

            const uint16_t texWinW = gpu.texWinXMask + 1;   // This calculation should work because the mask should always be for POW2 texture wrapping
            const uint16_t texWinH = gpu.texWinYMask + 1;

            if (VRenderer::canSubmitDrawCmds()) {
                VDrawing::addAlphaBlendedUISprite(
                    drawRect.x,
                    drawRect.y,
                    drawRect.w,
                    drawRect.h,
                    drawRect.u,
                    drawRect.v,
                    drawRect.color.comp.r,
                    drawRect.color.comp.g,
                    drawRect.color.comp.b,
                    128,
                    gpu.clutX,
                    gpu.clutY,
                    gpu.texPageX + gpu.texWinX,
                    gpu.texPageY + gpu.texWinY,
                    texWinW / 2,    // Texture window is in terms of 8bpp pixels (format dependent) but HW renderer uses VRAM coords (16bpp pixels) - correct for this
                    texWinH,
                    gpu.texFmt,
                    bBlendSprite
                );
            }

            return;
        }
    #endif  // #if PSYDOOM_VULKAN_RENDERER

    if (bBlendSprite) {
        Gpu::draw<Gpu::DrawMode::TexturedBlended>(gpu, drawRect);
    } else {
        Gpu::draw<Gpu::DrawMode::Textured>(gpu, drawRect);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw an 8x8 pixel sprite
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const SPRT_8& sprite8) noexcept {
    // Convert to a general sized sprite and re-use that submission function.
    // The 8x8 pixel case is only for rendering the old 'I_Error' message font so a small bit of extra overhead doesn't matter...
    SPRT sprite = {};
    sprite.r0 = sprite8.r0;
    sprite.g0 = sprite8.g0;
    sprite.b0 = sprite8.b0;
    sprite.code = sprite8.code;
    sprite.x0 = sprite8.x0;
    sprite.y0 = sprite8.y0;
    sprite.tu0 = sprite8.tu0;
    sprite.tv0 = sprite8.tv0;
    sprite.clut = sprite8.clut;
    sprite.w = 8;
    sprite.h = 8;

    submit(sprite);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw a line
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const LINE_F2& line) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Setup the line to be drawn then submit to the GPU
    const bool bColorLine = ((line.code & 0x1) == 0);
    const bool bBlendLine = line.code & 0x2;

    Gpu::DrawLine drawLine = {};
    drawLine.x1 = line.x0;
    drawLine.y1 = line.y0;
    drawLine.x2 = line.x1;
    drawLine.y2 = line.y1;
    drawLine.color.comp.r = (bColorLine) ? line.r0 : 128;   // Note: '128' is '1.0' or full strength color if we don't want to modulate
    drawLine.color.comp.g = (bColorLine) ? line.g0 : 128;
    drawLine.color.comp.b = (bColorLine) ? line.b0 : 128;

    // Are we using the Vulkan renderer? If so submit via that.
    // This allows us to re-use a lot of the old PSX 2D rendering without any changes.
    #if PSYDOOM_VULKAN_RENDERER
        if (Video::usingVulkanRenderer()) {
            ASSERT_LOG(gpu.blendMode == Gpu::BlendMode::Alpha50, "Only alpha blending is supported for PSX renderer lines forwarded to Vulkan!");

            if (VRenderer::canSubmitDrawCmds()) {
                VDrawing::addAlphaBlendedUILine(
                    drawLine.x1,
                    drawLine.y1,
                    drawLine.x2,
                    drawLine.y2,
                    drawLine.color.comp.r,
                    drawLine.color.comp.g,
                    drawLine.color.comp.b,
                    128,
                    bBlendLine
                );
            }

            return;
        }
    #endif

    if (bBlendLine) {
        Gpu::draw<Gpu::DrawMode::FlatColoredBlended>(gpu, drawLine);
    } else {
        Gpu::draw<Gpu::DrawMode::FlatColored>(gpu, drawLine);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw a flat shaded and textured polygon
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const POLY_FT3& poly) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Set texture page and format, and the CLUT to use
    setGpuTexPageId(poly.tpage);
    setGpuClutId(poly.clut);

    // Setup the triangle to be drawn then submit to the GPU
    const bool bColorPoly = ((poly.code & 0x1) == 0);
    const bool bBlendPoly = poly.code & 0x2;

    Gpu::DrawTriangle drawTri = {};
    drawTri.x1 = poly.x0;
    drawTri.y1 = poly.y0;
    drawTri.u1 = poly.tu0;
    drawTri.v1 = poly.tv0;
    drawTri.x2 = poly.x1;
    drawTri.y2 = poly.y1;
    drawTri.u2 = poly.tu1;
    drawTri.v2 = poly.tv1;
    drawTri.x3 = poly.x2;
    drawTri.y3 = poly.y2;
    drawTri.u3 = poly.tu2;
    drawTri.v3 = poly.tv2;
    drawTri.color.comp.r = (bColorPoly) ? poly.r0 : 128;   // Note: '128' is '1.0' or full strength color if we don't want to modulate
    drawTri.color.comp.g = (bColorPoly) ? poly.g0 : 128;
    drawTri.color.comp.b = (bColorPoly) ? poly.b0 : 128;

    if (bBlendPoly) {
        Gpu::draw<Gpu::DrawMode::TexturedBlended>(gpu, drawTri);
    } else {
        Gpu::draw<Gpu::DrawMode::Textured>(gpu, drawTri);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw non-textured (color only) quad
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const POLY_F4& poly) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Setup the triangles to be drawn then submit to the GPU
    const bool bColorPoly = ((poly.code & 0x1) == 0);
    const bool bBlendPoly = poly.code & 0x2;

    const uint8_t r = (bColorPoly) ? poly.r0 : 128;     // Note: '128' is '1.0' or full strength color if we don't want to modulate
    const uint8_t g = (bColorPoly) ? poly.g0 : 128;
    const uint8_t b = (bColorPoly) ? poly.b0 : 128;

    Gpu::DrawTriangle drawTri1 = {};
    Gpu::DrawTriangle drawTri2 = {};

    drawTri1.x1 = poly.x0;
    drawTri1.y1 = poly.y0;
    drawTri1.x2 = poly.x1;
    drawTri1.y2 = poly.y1;
    drawTri1.x3 = poly.x2;
    drawTri1.y3 = poly.y2;
    drawTri1.color.comp.r = r;
    drawTri1.color.comp.g = g;
    drawTri1.color.comp.b = b;

    drawTri2.x1 = poly.x1;
    drawTri2.y1 = poly.y1;
    drawTri2.x2 = poly.x2;
    drawTri2.y2 = poly.y2;
    drawTri2.x3 = poly.x3;
    drawTri2.y3 = poly.y3;
    drawTri2.color.comp.r = r;
    drawTri2.color.comp.g = g;
    drawTri2.color.comp.b = b;

    if (bBlendPoly) {
        Gpu::draw<Gpu::DrawMode::FlatColoredBlended>(gpu, drawTri1);
        Gpu::draw<Gpu::DrawMode::FlatColoredBlended>(gpu, drawTri2);
    } else {
        Gpu::draw<Gpu::DrawMode::FlatColored>(gpu, drawTri1);
        Gpu::draw<Gpu::DrawMode::FlatColored>(gpu, drawTri2);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw a textured quad
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const POLY_FT4& poly) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Set texture page and format, and the CLUT to use
    setGpuTexPageId(poly.tpage);
    setGpuClutId(poly.clut);

    // Setup the triangles to be drawn then submit to the GPU
    const bool bColorPoly = ((poly.code & 0x1) == 0);
    const bool bBlendPoly = poly.code & 0x2;

    const uint8_t r = (bColorPoly) ? poly.r0 : 128;     // Note: '128' is '1.0' or full strength color if we don't want to modulate
    const uint8_t g = (bColorPoly) ? poly.g0 : 128;
    const uint8_t b = (bColorPoly) ? poly.b0 : 128;

    Gpu::DrawTriangle drawTri1 = {};
    Gpu::DrawTriangle drawTri2 = {};

    drawTri1.x1 = poly.x0;
    drawTri1.y1 = poly.y0;
    drawTri1.u1 = poly.tu0;
    drawTri1.v1 = poly.tv0;
    drawTri1.x2 = poly.x1;
    drawTri1.y2 = poly.y1;
    drawTri1.u2 = poly.tu1;
    drawTri1.v2 = poly.tv1;
    drawTri1.x3 = poly.x2;
    drawTri1.y3 = poly.y2;
    drawTri1.u3 = poly.tu2;
    drawTri1.v3 = poly.tv2;
    drawTri1.color.comp.r = r;
    drawTri1.color.comp.g = g;
    drawTri1.color.comp.b = b;

    drawTri2.x1 = poly.x1;
    drawTri2.y1 = poly.y1;
    drawTri2.u1 = poly.tu1;
    drawTri2.v1 = poly.tv1;
    drawTri2.x2 = poly.x2;
    drawTri2.y2 = poly.y2;
    drawTri2.u2 = poly.tu2;
    drawTri2.v2 = poly.tv2;
    drawTri2.x3 = poly.x3;
    drawTri2.y3 = poly.y3;
    drawTri2.u3 = poly.tu3;
    drawTri2.v3 = poly.tv3;
    drawTri2.color.comp.r = r;
    drawTri2.color.comp.g = g;
    drawTri2.color.comp.b = b;

    if (bBlendPoly) {
        Gpu::draw<Gpu::DrawMode::TexturedBlended>(gpu, drawTri1);
        Gpu::draw<Gpu::DrawMode::TexturedBlended>(gpu, drawTri2);
    } else {
        Gpu::draw<Gpu::DrawMode::Textured>(gpu, drawTri1);
        Gpu::draw<Gpu::DrawMode::Textured>(gpu, drawTri2);
    }
}

END_NAMESPACE(LIBGPU_CmdDispatch)
