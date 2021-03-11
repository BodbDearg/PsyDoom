#include "LIBGPU_CmdDispatch.h"

#include "Asserts.h"
#include "Gpu.h"
#include "PsxVm.h"
#include "Video.h"
#include "Vulkan/VDrawing.h"
#include "Vulkan/VRenderer.h"
#include "Vulkan/VTypes.h"

BEGIN_NAMESPACE(LIBGPU_CmdDispatch)

#if PSYDOOM_VULKAN_RENDERER

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function which binds a Vulkan pipeline to use for drawing LIBGPU textured primitives.
// Examines the current PSX GPU texture format and blend mode, and sets the draw pipeline accordingly.
// Also makes adjustments to the texture window based on the texture format, so that it's a format native coord and a 16-bit pixel coord.
// Also sets the alpha that the primitive should be drawn with, based on the blend mode.
//------------------------------------------------------------------------------------------------------------------------------------------
static void doSetupForVkRendererTexturedDraw(const bool bBlendPrimitive, uint16_t& outTexWinX, uint8_t& outDrawAlpha) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Note: we only only support a subset of possible blend mode and t
    if (gpu.blendMode != Gpu::BlendMode::Add) {
        // Normal case: most UI sprites are alpha blended
        ASSERT_LOG(gpu.blendMode == Gpu::BlendMode::Alpha50, "Unsupported blend mode and texture format combo!");

        switch (gpu.texFmt) {
            case Gpu::TexFmt::Bpp4:
                VDrawing::setDrawPipeline(VPipelineType::UI_4bpp);
                outTexWinX *= 4;
                break;

            case Gpu::TexFmt::Bpp8:
                VDrawing::setDrawPipeline(VPipelineType::UI_8bpp);
                outTexWinX *= 2;
                break;

            case Gpu::TexFmt::Bpp16:
                VDrawing::setDrawPipeline(VPipelineType::UI_16bpp);
                break;

            default:
                ASSERT_FAIL("Bad format!");
                break;
        }

        outDrawAlpha = (bBlendPrimitive) ? 64 : 128;
    }
    else {
        // Additive blending: used by the player weapon when the player is invisible
        ASSERT_LOG(gpu.texFmt == Gpu::TexFmt::Bpp8, "Unsupported blend mode and texture format combo!");

        VDrawing::setDrawPipeline(VPipelineType::UI_8bpp_Add);
        outTexWinX *= 2;
        outDrawAlpha = 128;
    }
}

#endif  // #if PSYDOOM_VULKAN_RENDERER

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
    const bool bBlendSprite = (sprite.code & 0x2);

    Gpu::DrawRect drawRect = {};
    drawRect.x = sprite.x0;
    drawRect.y = sprite.y0;
    drawRect.w = sprite.w;
    drawRect.h = sprite.h;
    drawRect.u = sprite.u0;
    drawRect.v = sprite.v0;
    drawRect.color.comp.r = (bColorSprite) ? sprite.r0 : 128;   // Note: '128' is '1.0' or full strength color if we don't want to modulate
    drawRect.color.comp.g = (bColorSprite) ? sprite.g0 : 128;
    drawRect.color.comp.b = (bColorSprite) ? sprite.b0 : 128;

    // Are we using the Vulkan renderer? If so submit via that.
    // This allows us to re-use a lot of the old PSX 2D rendering without any changes.
    #if PSYDOOM_VULKAN_RENDERER
        if (Video::isUsingVulkanRenderPath()) {
            uint16_t texWinX = gpu.texPageX + gpu.texWinX;      // Note: needs to be adjusted from 16bpp coords to coords for whatever format we are using (see below)
            uint16_t texWinY = gpu.texPageY + gpu.texWinY;

            if (VRenderer::isRendering()) {
                // Determine the draw alpha, correct the 'texWinX' coord to be in terms of the sprite's pixel format and set the correct draw pipeline
                uint8_t drawAlpha = {};
                doSetupForVkRendererTexturedDraw(bBlendSprite, texWinX, drawAlpha);

                // Draw the sprite and restrict the texture window to cover the exact area of VRAM occupied by the sprite.
                // Ignoring the gpu texture window/page settings in this way and restricting to the exact pixels used by the
                // sprite helps to avoid stitching artifacts, especially when MSAA is active.
                VDrawing::addUISprite(
                    drawRect.x,
                    drawRect.y,
                    drawRect.w,
                    drawRect.h,
                    0,                          // UV coords are local to the texture window, which covers the entire sprite area
                    0,
                    drawRect.color.comp.r,
                    drawRect.color.comp.g,
                    drawRect.color.comp.b,
                    drawAlpha,
                    gpu.clutX,
                    gpu.clutY,
                    texWinX + sprite.u0,
                    texWinY + sprite.v0,
                    // Size the texture window to cover only the region of sprite pixels that we need, to avoid artifacts as mentioned above
                    std::min((uint16_t) sprite.w, drawRect.w),
                    std::min((uint16_t) sprite.h, drawRect.h)
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
    sprite.u0 = sprite8.u0;
    sprite.v0 = sprite8.v0;
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
    ASSERT_LOG((line.code & 0x2) == 0, "Blending not supported for lines! Doom shouldn't need this functionality!");

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
        if (Video::isUsingVulkanRenderPath()) {
            ASSERT_LOG(gpu.blendMode == Gpu::BlendMode::Alpha50, "Only alpha blending is supported for PSX renderer lines forwarded to Vulkan!");

            if (VRenderer::isRendering()) {
                VDrawing::setDrawPipeline(VPipelineType::Lines);
                VDrawing::addUILine(
                    drawLine.x1,
                    drawLine.y1,
                    drawLine.x2,
                    drawLine.y2,
                    drawLine.color.comp.r,
                    drawLine.color.comp.g,
                    drawLine.color.comp.b
                );
            }

            return;
        }
    #endif

    Gpu::draw<Gpu::DrawMode::FlatColored>(gpu, drawLine);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw a flat shaded and textured triangle.
// Note: this function does not support pass-through to the Vulkan renderer; it's not needed since it's just ussed by the Classic renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const POLY_FT3& poly) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Set texture page and format, and the CLUT to use
    setGpuTexPageId(poly.tpage);
    setGpuClutId(poly.clut);

    // Setup the triangle to be drawn then submit to the GPU
    const bool bColorPoly = ((poly.code & 0x1) == 0);
    const bool bBlendPoly = (poly.code & 0x2);

    Gpu::DrawTriangle drawTri = {};
    drawTri.x1 = poly.x0;
    drawTri.y1 = poly.y0;
    drawTri.u1 = poly.u0;
    drawTri.v1 = poly.v0;
    drawTri.x2 = poly.x1;
    drawTri.y2 = poly.y1;
    drawTri.u2 = poly.u1;
    drawTri.v2 = poly.v1;
    drawTri.x3 = poly.x2;
    drawTri.y3 = poly.y2;
    drawTri.u3 = poly.u2;
    drawTri.v3 = poly.v2;
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
    const bool bBlendPoly = (poly.code & 0x2);

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

    // Are we using the Vulkan renderer? If so submit via that.
    // This allows us to re-use a lot of the old PSX 2D rendering without any changes.
    #if PSYDOOM_VULKAN_RENDERER
        if (Video::isUsingVulkanRenderPath()) {
            ASSERT_LOG((!bBlendPoly), "Don't support blending for POLY_F4 primitives forwarded to the Vulkan renderer!");

            if (VRenderer::isRendering()) {
                VDrawing::setDrawPipeline(VPipelineType::Colored);
                VDrawing::addFlatColoredQuad(
                    poly.x0, poly.y0, 0.0f,
                    poly.x1, poly.y1, 0.0f,
                    poly.x3, poly.y3, 0.0f,
                    poly.x2, poly.y2, 0.0f,
                    r,
                    g,
                    b
                );
            }

            return;
        }
    #endif  // #if PSYDOOM_VULKAN_RENDERER

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
    const bool bBlendPoly = (poly.code & 0x2);

    const uint8_t r = (bColorPoly) ? poly.r0 : 128;     // Note: '128' is '1.0' or full strength color if we don't want to modulate
    const uint8_t g = (bColorPoly) ? poly.g0 : 128;
    const uint8_t b = (bColorPoly) ? poly.b0 : 128;

    Gpu::DrawTriangle drawTri1 = {};
    Gpu::DrawTriangle drawTri2 = {};

    drawTri1.x1 = poly.x0;
    drawTri1.y1 = poly.y0;
    drawTri1.u1 = poly.u0;
    drawTri1.v1 = poly.v0;
    drawTri1.x2 = poly.x1;
    drawTri1.y2 = poly.y1;
    drawTri1.u2 = poly.u1;
    drawTri1.v2 = poly.v1;
    drawTri1.x3 = poly.x2;
    drawTri1.y3 = poly.y2;
    drawTri1.u3 = poly.u2;
    drawTri1.v3 = poly.v2;
    drawTri1.color.comp.r = r;
    drawTri1.color.comp.g = g;
    drawTri1.color.comp.b = b;

    drawTri2.x1 = poly.x1;
    drawTri2.y1 = poly.y1;
    drawTri2.u1 = poly.u1;
    drawTri2.v1 = poly.v1;
    drawTri2.x2 = poly.x2;
    drawTri2.y2 = poly.y2;
    drawTri2.u2 = poly.u2;
    drawTri2.v2 = poly.v2;
    drawTri2.x3 = poly.x3;
    drawTri2.y3 = poly.y3;
    drawTri2.u3 = poly.u3;
    drawTri2.v3 = poly.v3;
    drawTri2.color.comp.r = r;
    drawTri2.color.comp.g = g;
    drawTri2.color.comp.b = b;

    // Are we using the Vulkan renderer? If so submit via that.
    // This allows us to re-use a lot of the old PSX 2D rendering without any changes.
    #if PSYDOOM_VULKAN_RENDERER
        if (Video::isUsingVulkanRenderPath()) {
            uint16_t texWinW = gpu.texWinXMask + 1;             // This calculation should work because the mask should always be for POW2 texture wrapping
            uint16_t texWinH = gpu.texWinYMask + 1;
            uint16_t texWinX = gpu.texPageX + gpu.texWinX;      // Note: needs to be adjusted from 16bpp coords to coords for whatever format we are using (see below)
            uint16_t texWinY = gpu.texPageY + gpu.texWinY;

            if (VRenderer::isRendering()) {
                // Determine the draw alpha, set the correct pipeline then add the quad to be drawn
                uint8_t drawAlpha = {};
                doSetupForVkRendererTexturedDraw(bBlendPoly, texWinX, drawAlpha);

                VDrawing::addWorldQuad(
                    poly.x0, poly.y0, 0.0f, poly.u0, poly.v0,
                    poly.x1, poly.y1, 0.0f, poly.u1, poly.v1,
                    poly.x3, poly.y3, 0.0f, poly.u3, poly.v3,
                    poly.x2, poly.y2, 0.0f, poly.u2, poly.v2,
                    r,
                    g,
                    b,
                    gpu.clutX,
                    gpu.clutY,
                    texWinX,
                    texWinY,
                    texWinW,
                    texWinH,
                    VLightDimMode::None,
                    128,
                    128,
                    128,
                    drawAlpha
                );
            }

            return;
        }
    #endif  // #if PSYDOOM_VULKAN_RENDERER

    if (bBlendPoly) {
        Gpu::draw<Gpu::DrawMode::TexturedBlended>(gpu, drawTri1);
        Gpu::draw<Gpu::DrawMode::TexturedBlended>(gpu, drawTri2);
    } else {
        Gpu::draw<Gpu::DrawMode::Textured>(gpu, drawTri1);
        Gpu::draw<Gpu::DrawMode::Textured>(gpu, drawTri2);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw a textured row of Doom floor pixels.
// Note: this function does not support pass-through to the Vulkan renderer; it's not needed since it's just ussed by the Classic renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const FLOORROW_FT& row) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Set texture page and format, and the CLUT to use
    setGpuTexPageId(row.tpage);
    setGpuClutId(row.clut);

    // Setup the row to be drawn then submit to the GPU
    const bool bColorRow = ((row.code & 0x1) == 0);
    const bool bBlendRow = (row.code & 0x2);

    Gpu::DrawFloorRow drawRow = {};
    drawRow.x1 = row.x0;
    drawRow.x2 = row.x1;
    drawRow.y = row.y0;
    drawRow.u1 = row.u0;
    drawRow.v1 = row.v0;
    drawRow.u2 = row.u1;
    drawRow.v2 = row.v1;
    drawRow.color.comp.r = (bColorRow) ? row.r0 : 128;   // Note: '128' is '1.0' or full strength color if we don't want to modulate
    drawRow.color.comp.g = (bColorRow) ? row.g0 : 128;
    drawRow.color.comp.b = (bColorRow) ? row.b0 : 128;

    if (bBlendRow) {
        Gpu::draw<Gpu::DrawMode::TexturedBlended>(gpu, drawRow);
    } else {
        Gpu::draw<Gpu::DrawMode::Textured>(gpu, drawRow);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle a command to draw a textured column of Doom wall pixels.
// Note: this function does not support pass-through to the Vulkan renderer; it's not needed since it's just ussed by the Classic renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
void submit(const WALLCOL_FT& col) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;

    // Set texture page and format, and the CLUT to use
    setGpuTexPageId(col.tpage);
    setGpuClutId(col.clut);

    // Setup the column to be drawn then submit to the GPU
    const bool bColorCol = ((col.code & 0x1) == 0);
    const bool bBlendCol = (col.code & 0x2);

    Gpu::DrawWallCol drawCol = {};
    drawCol.y1 = col.y0;
    drawCol.y2 = col.y1;
    drawCol.x = col.x0;
    drawCol.u = col.u0;
    drawCol.v1 = col.v0;
    drawCol.v2 = col.v1;
    drawCol.color.comp.r = (bColorCol) ? col.r0 : 128;   // Note: '128' is '1.0' or full strength color if we don't want to modulate
    drawCol.color.comp.g = (bColorCol) ? col.g0 : 128;
    drawCol.color.comp.b = (bColorCol) ? col.b0 : 128;

    if (bBlendCol) {
        Gpu::draw<Gpu::DrawMode::TexturedBlended>(gpu, drawCol);
    } else {
        Gpu::draw<Gpu::DrawMode::Textured>(gpu, drawCol);
    }
}

END_NAMESPACE(LIBGPU_CmdDispatch)
