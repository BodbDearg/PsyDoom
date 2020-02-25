//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBGPU' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LIBGPU.h"

#include "LIBAPI.h"
#include "LIBC2.h"
#include "LIBETC.h"
#include "PcPsx/Endian.h"
#include "PcPsx/Finally.h"
#include "PsxVm/VmSVal.h"

#include <cstdarg>
#include <cstring>
#include <device/gpu/gpu.h>

// N.B: needs to happen AFTER Avocado includes due to clashes caused by the MIPS register macros
#include "PsxVm/PsxVm.h"

// The CLUT and texture page for the debug font
static uint16_t gDFontClutId;
static uint16_t gDFontTPageId;

// Display area for the debug font and whether to clear the background to 0,0,0 before drawing
static int16_t gDFontDispX;
static int16_t gDFontDispY;
static int16_t gDFontDispW;
static int16_t gDFontDispH;
static bool    gDFontClearBg;

// The buffer for debug font printing and how many chars it can hold
static constexpr int32_t DBG_MSG_BUF_SIZE = 1024;
static char gDbgMsgBuf[DBG_MSG_BUF_SIZE];

// Current print position in the debug message buffer
static int32_t gDbgMsgBufPos;

//------------------------------------------------------------------------------------------------------------------------------------------
// PC-PSX helper function that clears the current drawing area to the specified color
//------------------------------------------------------------------------------------------------------------------------------------------
static void clearDrawingArea(const uint8_t r, const uint8_t g, const uint8_t b) noexcept {
    gpu::GPU& gpu = *PsxVm::gpGpu;

    RGB rgb = {};
    rgb.r = r;
    rgb.g = g;
    rgb.b = b;

    const int32_t drawW = (gpu.drawingArea.right - gpu.drawingArea.left + 1) & 0xFFFF;
    const int32_t drawH = (gpu.drawingArea.bottom - gpu.drawingArea.top + 1) & 0xFFFF;

    gpu.arguments[0] = rgb.raw;
    gpu.arguments[1] = gpu.drawingArea.left | (gpu.drawingArea.top << 16);
    gpu.arguments[2] = drawW | (drawH << 16);

    gpu.cmdFillRectangle();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PC-PSX helper function that sets the current texture page on the GPU.
// Note that this will also set the semi-transparency mode, in addition to the texture bit depth.
//------------------------------------------------------------------------------------------------------------------------------------------
static void setGpuTPage(const uint16_t tpageId) noexcept {
    gpu::GPU& gpu = *PsxVm::gpGpu;
    
    // Set texture page position and bit rate
    gpu.gp0_e1.texturePageBaseX = (tpageId & 0x0F) >> 0;    // This is multiples of 64
    gpu.gp0_e1.texturePageBaseY = (tpageId & 0x10) >> 4;    // This is multiples of 256
    
    switch ((tpageId >> 7) & 0x3) {
        case 0: gpu.gp0_e1.texturePageColors = gpu::GP0_E1::TexturePageColors::bit4;
        case 1: gpu.gp0_e1.texturePageColors = gpu::GP0_E1::TexturePageColors::bit8;
        case 2: gpu.gp0_e1.texturePageColors = gpu::GP0_E1::TexturePageColors::bit15;
        default: break;
    }

    // Set transparency mode
    switch ((tpageId >> 5) & 0x3) {
        case 0: gpu.gp0_e1.semiTransparency = gpu::SemiTransparency::Bby2plusFby2;
        case 1: gpu.gp0_e1.semiTransparency = gpu::SemiTransparency::BplusF;
        case 2: gpu.gp0_e1.semiTransparency = gpu::SemiTransparency::BminusF;
        case 3: gpu.gp0_e1.semiTransparency = gpu::SemiTransparency::BplusFby4;
        default: break;
    }
}

void LIBGPU_ResetGraph() noexcept {
loc_8004BCC8:
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = 0x80080000;                                    // Result = 80080000
    s2 += 0x356;                                        // Result = 80080356
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lbu(s2);                                       // Load from: 80080356
    v0 = (v0 < 2);
    s1 = a0;
    if (v0 != 0) goto loc_8004BD14;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1B28;                                       // Result = STR_Sys_ResetGraph_Msg1[0] (80011B28)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = s1;
    ptr_call(v0);
loc_8004BD14:
    v0 = s1 & 3;
    s0 = s2 - 2;                                        // Result = 80080354
    if (v0 == 0) goto loc_8004BD38;
    {
        const bool bJump = (i32(v0) < 0);
        v0 = (i32(v0) < 4);
        if (bJump) goto loc_8004BEA4;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_8004BED4;
    }
    goto loc_8004BE80;
loc_8004BD38:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1B3C;                                       // Result = STR_Sys_ResetGraph_Msg2[0] (80011B3C)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5D14;                                       // Result = gLIBGPU_SYS_driver_table[0] (80075D14)
    a2 = s0;                                            // Result = 80080354
    LIBC2_printf();
    a0 = s0;                                            // Result = 80080354
    a1 = 0;                                             // Result = 00000000
    a2 = 0x80;                                          // Result = 00000080
    LIBGPU_SYS_memset();
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at + 0x355);                                 // Store to: 80080355
    LIBETC_ResetCallback();
    v0 = 0xFF0000;                                      // Result = 00FF0000
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5D54);                               // Load from: gpLIBGPU_SYS_driver_table (80075D54)
    v0 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 &= v0;
    LIBAPI_GPU_cw();
    a0 = 0;
    LIBGPU_SYS__reset();
    a0 = v0;
    v0 = a0;
    v1 = v0 & 0xFF;
    sb(v0, s2 - 0x2);                                   // Store to: 80080354
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 2);
        if (bJump) goto loc_8004BDF0;
    }
    if (v0 == 0) goto loc_8004BDDC;
    v0 = 0x400;                                         // Result = 00000400
    if (v1 == 0) goto loc_8004BE64;
    goto loc_8004BEAC;
loc_8004BDDC:
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 == v0);
        v0 = s1 & 8;
        if (bJump) goto loc_8004BE1C;
    }
    goto loc_8004BEAC;
loc_8004BDF0:
    v0 = s1 & 8;
    {
        const bool bJump = (v0 == 0);
        v0 = a0 + 1;
        if (bJump) goto loc_8004BE00;
    }
    sb(v0, s2 - 0x2);                                   // Store to: 80080354
loc_8004BE00:
    v0 = 0x400;                                         // Result = 00000400
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x35A);                                 // Store to: 8008035A
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x358);                                 // Store to: 80080358
    goto loc_8004BEAC;
loc_8004BE1C:
    {
        const bool bJump = (v0 == 0);
        v0 = a0 + 1;
        if (bJump) goto loc_8004BE60;
    }
    a0 = 0x9000000;                                     // Result = 09000000
    sb(v0, s2 - 0x2);                                   // Store to: 80080354
    v0 = 0x400;                                         // Result = 00000400
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x35A);                                 // Store to: 8008035A
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x358);                                 // Store to: 80080358
    a0 |= 1;                                            // Result = 09000001
    LIBGPU_SYS__ctl();
    goto loc_8004BEAC;
loc_8004BE60:
    v0 = 0x400;                                         // Result = 00000400
loc_8004BE64:
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x358);                                 // Store to: 80080358
    v0 = 0x200;                                         // Result = 00000200
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x35A);                                 // Store to: 8008035A
    goto loc_8004BEAC;
loc_8004BE80:
    a0 = 1;
    LIBGPU_SYS__reset();
    goto loc_8004BEAC;
loc_8004BEA4:
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_8004BED4;
loc_8004BEAC:
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x364;                                        // Result = 80080364
    a0 = s0;                                            // Result = 80080364
    a1 = -1;                                            // Result = FFFFFFFF
    a2 = 0x5C;                                          // Result = 0000005C
    LIBGPU_SYS_memset();
    a0 = s0 + 0x5C;                                     // Result = gLIBGPU_SYS_p0_82[3] (800803C0)
    a1 = -1;                                            // Result = FFFFFFFF
    a2 = 0x14;                                          // Result = 00000014
    LIBGPU_SYS_memset();
loc_8004BED4:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void LIBGPU_SetGraphDebug() noexcept {
loc_8004C004:
    sp -= 0x18;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 += 0x356;                                        // Result = 80080356
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = lbu(v1);                                       // Load from: 80080356
    sb(a0, v1);                                         // Store to: 80080356
    a0 &= 0xFF;
    v0 = s0;
    if (a0 == 0) goto loc_8004C05C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = lbu(v1);                                       // Load from: 80080356
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lbu(a2 + 0x354);                               // Load from: 80080354
    a3 = 0x80080000;                                    // Result = 80080000
    a3 = lbu(a3 + 0x357);                               // Load from: 80080357
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1B74;                                       // Result = STR_Sys_SetDebug_Msg[0] (80011B74)
    ptr_call(v0);
    v0 = s0;
loc_8004C05C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_GetGraphType() noexcept {
loc_8004C11C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x354);                               // Load from: 80080354
    return;
}

void LIBGPU_SetDispMask() noexcept {
loc_8004C198:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x356);                               // Load from: 80080356
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = (v0 < 2);
    sw(ra, sp + 0x14);
    if (v0 != 0) goto loc_8004C1D4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1BD0;                                       // Result = STR_Sys_SetDisplayMask_Msg[0] (80011BD0)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = s0;
    ptr_call(v0);
loc_8004C1D4:
    a0 = 0x3000000;                                     // Result = 03000000
    a0 |= 1;                                            // Result = 03000001
    if (s0 == 0) goto loc_8004C1EC;
    a0 = 0x3000000;                                     // Result = 03000000
loc_8004C1EC:
    LIBGPU_SYS__ctl();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for all GPU operations to complete or return the amount left.
//
// Mode:
//  0 = Block until all drawing operations are complete
//  1 = Return the number of drawing operations currently in progress.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBGPU_DrawSync([[maybe_unused]] const int32_t mode) noexcept {
    // This function doesn't need to do anything in this emulated environment.
    // When we submit something to the 'gpu' then Avocado executes it immediately and blocks before returning.
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the given image data to the given area in VRAM.
// The image format is assumed to be 16-bit.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_LoadImage(const RECT& dstRect, const uint16_t* const pImageData) noexcept {
    // Sanity checks
    ASSERT(pImageData);
    ASSERT(dstRect.w > 0);
    ASSERT(dstRect.h > 0);
    ASSERT(dstRect.w <= gpu::VRAM_WIDTH);
    ASSERT(dstRect.h <= gpu::VRAM_HEIGHT);

    // Determine the destination bounds and row size for the copy.
    // Note that we must wrap horizontal coordinates (see comments below).
    const uint16_t rowW = dstRect.w;
    const uint16_t dstLx = (uint16_t)(dstRect.x            ) % (uint16_t) gpu::VRAM_WIDTH;
    const uint16_t dstRx = (uint16_t)(dstRect.x + rowW     ) % (uint16_t) gpu::VRAM_WIDTH;
    const uint16_t dstTy = (uint16_t)(dstRect.y            );
    const uint16_t dstBy = (uint16_t)(dstRect.y + dstRect.h);

    // Copy each row into VRAM
    gpu::GPU& gpu = *PsxVm::gpGpu;
    uint16_t* const pVram = gpu.vram.data();

    const uint16_t* pSrcPixels = pImageData;

    for (uint32_t dstY = dstTy; dstY < dstBy; ++dstY) {
        // Note: destination Y wrapped due to behavior mentioned in NO$PSX specs (see comments below)
        const uint16_t dstYWrapped = dstY % gpu::VRAM_HEIGHT;
        uint16_t* const pDstRow = pVram + (intptr_t) dstYWrapped * gpu::VRAM_WIDTH;

        // According to the following specs:
        //  https://problemkaputt.de/psx-spx.htm#graphicsprocessingunitgpu
        // Under "GPU Memory Transfer Commands" and "Wrapping".
        // If a load operation happens to exceed the bounds of VRAM, then it will wrap around to the opposite side of VRAM.
        // 
        // If we detect this situation then we need to split the copy up into two parts.
        // The 2nd copy part will begin at the left edge of VRAM.
        //
        if (dstLx <= dstRx) {
            // Usual case: no wraparound, so we can do a simple memcpy for the entire row
            std::memcpy(pDstRow + dstLx, pSrcPixels, rowW * sizeof(uint16_t));
            pSrcPixels += rowW;
        }
        else {
            // The copy wraps around to the left side of VRAM, need to do 2 separate memcpy operations:
            const int32_t numWrappedPixels = dstLx + rowW - gpu::VRAM_WIDTH;
            const int32_t numNonWrappedPixels = rowW - numWrappedPixels;
            
            std::memcpy(pDstRow + dstLx, pSrcPixels, numNonWrappedPixels * sizeof(uint16_t));
            pSrcPixels += numNonWrappedPixels;

            std::memcpy(pDstRow, pSrcPixels, numWrappedPixels * sizeof(uint16_t));
            pSrcPixels += numWrappedPixels;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copy one part of VRAM to another part of VRAM
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBGPU_MoveImage(const RECT& srcRect, const int32_t dstX, const int32_t dstY) noexcept {
    // Sanity checks
    ASSERT((srcRect.w > 0) && (srcRect.h > 0));
    ASSERT((srcRect.x >= 0) && (srcRect.y >= 0));
    ASSERT(srcRect.x + srcRect.w <= gpu::VRAM_WIDTH);
    ASSERT(srcRect.y + srcRect.h <= gpu::VRAM_HEIGHT);
    ASSERT((dstX >= 0) && (dstY >= 0));
    ASSERT(dstX + srcRect.w <= gpu::VRAM_WIDTH);
    ASSERT(dstY + srcRect.y <= gpu::VRAM_WIDTH);

    // Copy each row
    const uint32_t numRows = srcRect.h;
    const uint32_t rowSize = srcRect.w * sizeof(uint16_t);

    gpu::GPU& gpu = *PsxVm::gpGpu;

    const uint16_t* pSrcRow = gpu.vram.data() + srcRect.x + (intptr_t) srcRect.y * gpu::VRAM_WIDTH;
    uint16_t* pDstRow = gpu.vram.data() + dstX + (intptr_t) dstY * gpu::VRAM_WIDTH;

    for (uint32_t rowIdx = 0; rowIdx < numRows; ++rowIdx) {
        std::memcpy(pDstRow, pSrcRow, rowSize);
        pSrcRow += gpu::VRAM_WIDTH;
        pDstRow += gpu::VRAM_WIDTH;
    }

    return 0;   // This is the position of the command in the queue, according to PsyQ docs - don't care about this...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submit a linked list of GPU drawing primitives.
// These may include drawing primitves, or primitives that set GPU state.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_DrawOTag(const void* const pPrimList) noexcept {
    const uint32_t* pCurPrimWord = (const uint32_t*) pPrimList;

    while (true) {
        // Read the tag for this primitive and determine its data size in words.
        // Also determine the next primitive address (24-bit relative, and absolute).
        const uint32_t tag = pCurPrimWord[0];
        ++pCurPrimWord;

        const uint32_t numDataWords = tag >> 24;
        const uint32_t nextPrimAddr24 = tag & 0x00FFFFFF;
        const uint32_t nextPrimAddrAbs = nextPrimAddr24 | 0x80000000;

        // Submit the primitive's data words to the GPU
        uint32_t dataWordsLeft = numDataWords;

        while (dataWordsLeft > 0) {
            const uint32_t dataWord = pCurPrimWord[0];
            ++pCurPrimWord;
            --dataWordsLeft;
            writeGP0(dataWord);
        }

        // Stop if we've reached the end of the primitive list, otherwise move onto the next one
        if (nextPrimAddr24 == 0x00FFFFFF)
            break;

        pCurPrimWord = vmAddrToPtr<uint32_t>(nextPrimAddrAbs);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the drawing environment and potentially clears the screen also.
// The draw environment includes the display area, texture page and texture window settings, blending settings and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
DRAWENV& LIBGPU_PutDrawEnv(DRAWENV& env) noexcept {
    gpu::GPU& gpu = *PsxVm::gpGpu;
    
    // Set drawing area and offset
    gpu.drawingArea.left = env.clip.x;
    gpu.drawingArea.top = env.clip.y;
    gpu.drawingArea.right = env.clip.x + env.clip.w - 1;
    gpu.drawingArea.bottom = env.clip.y + env.clip.h - 1;
    
    gpu.drawingOffsetX = env.ofs[0];
    gpu.drawingOffsetY = env.ofs[1];
    
    // Set the texture window offset and mask.
    // Note that the units are specified in multiples of 8 for the GPU.
    gpu.gp0_e2.offsetX = env.tw.x / 8;
    gpu.gp0_e2.offsetY = env.tw.y / 8;

    if (env.tw.w == 0) {
        gpu.gp0_e2.maskX = 0;
    } else if (env.tw.w > 1) {
        gpu.gp0_e2.maskX = (env.tw.w / 8) - 1;
    } else {
        gpu.gp0_e2.maskX = 1;
    }
    
    if (env.tw.h == 0) {
        gpu.gp0_e2.maskY = 0;
    } else if (env.tw.h > 1) {
        gpu.gp0_e2.maskY = (env.tw.h / 8) - 1;
    } else {
        gpu.gp0_e2.maskY = 1;
    }

    // Set the texture page
    setGpuTPage(env.tpage);

    // Set dithering and draw to display area flags
    gpu.gp0_e1.dither24to15 = (env.dtd != 0);
    gpu.gp0_e1.drawingToDisplayArea = (env.dfe != 0) ?
        gpu::GP0_E1::DrawingToDisplayArea::allowed :
        gpu::GP0_E1::DrawingToDisplayArea::prohibited;

    // Clear the screen if specified by the DRAWENV.
    // Fill the draw area with the specified background color.
    if (env.isbg) {
        clearDrawingArea(env.r0, env.g0, env.b0);
    }

    return env;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the current display enviroment. This includes information about where in the framebuffer we display from, video resolution etc.
//------------------------------------------------------------------------------------------------------------------------------------------
DISPENV& LIBGPU_PutDispEnv(DISPENV& env) noexcept {
    gpu::GPU& gpu = *PsxVm::gpGpu;

    // Set display width, height and color depth
    switch (env.disp.w) {
        case 256: {
            gpu.gp1_08.horizontalResolution1 = gpu::GP1_08::HorizontalResolution::r256;
            gpu.gp1_08.horizontalResolution2 = gpu::GP1_08::HorizontalResolution2::normal;
        }   break;
        case 320: {
            gpu.gp1_08.horizontalResolution1 = gpu::GP1_08::HorizontalResolution::r320;
            gpu.gp1_08.horizontalResolution2 = gpu::GP1_08::HorizontalResolution2::normal;
        }   break;
        case 386: {
            gpu.gp1_08.horizontalResolution1 = {};
            gpu.gp1_08.horizontalResolution2 = gpu::GP1_08::HorizontalResolution2::r386;
        }   break;
        case 512: {
            gpu.gp1_08.horizontalResolution1 = gpu::GP1_08::HorizontalResolution::r512;
            gpu.gp1_08.horizontalResolution2 = gpu::GP1_08::HorizontalResolution2::normal;
        }   break;
        case 640: {
            gpu.gp1_08.horizontalResolution1 = gpu::GP1_08::HorizontalResolution::r640;
            gpu.gp1_08.horizontalResolution2 = gpu::GP1_08::HorizontalResolution2::normal;
        }   break;

        default: {
            ASSERT_FAIL("Bad display mode width!");
        }   break;
    }

    switch (env.disp.h) {
        case 240: gpu.gp1_08.verticalResolution = gpu::GP1_08::VerticalResolution::r240; break;
        case 480: gpu.gp1_08.verticalResolution = gpu::GP1_08::VerticalResolution::r480; break;

        default: {
            ASSERT_FAIL("Bad display mode height!");
        }   break;
    }

    gpu.gp1_08.interlace = (env.isinter != 0);
    gpu.gp1_08.colorDepth = (env.isrgb24 != 0) ?
        gpu::GP1_08::ColorDepth::bit24 :
        gpu::GP1_08::ColorDepth::bit15;

    // Set the location in VRAM to display from
    gpu.displayAreaStartX = env.disp.x;
    gpu.displayAreaStartY = env.disp.y;

    // Set the display range if specified, otherwise leave alone (I've never seen DOOM try to modify this)
    if ((env.screen.w != 0) && (env.screen.h != 0)) {
        gpu.displayRangeX1 = env.screen.x;
        gpu.displayRangeX2 = env.screen.x + env.screen.w;
        gpu.displayRangeY1 = env.screen.y;
        gpu.displayRangeY2 = env.screen.y + env.screen.h;
    }

    return env;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a draw primitive that sets the current texture window.
// Use the specified RECT as the window.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetTexWindow(DR_TWIN& prim, const RECT& texWin) noexcept {
    LIBGPU_setlen(prim, 2);
    prim.code[0] = LIBGPU_SYS_get_tw(&texWin);
    prim.code[1] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates a 'set draw mode' primitive with the specified settings.
// Note that the texture window is optional and if not specified then the current window will be used.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetDrawMode(
    DR_MODE& modePrim,
    const bool bCanDrawInDisplayArea,
    const bool bDitheringOn,
    const uint32_t texPageId,
    const RECT* const pNewTexWindow
) noexcept {
    // Set primitive size
    modePrim.tag &= 0x00FFFFFF;
    modePrim.tag |= uint32_t(2) << 24;

    // Set draw mode field
    modePrim.code[0] = LIBGPU_SYS_get_mode(bCanDrawInDisplayArea, bDitheringOn, (uint16_t) texPageId);

    // Set the texture window field
    if (pNewTexWindow) {
        modePrim.code[1] = LIBGPU_SYS_get_tw(pNewTexWindow);
    } else {
        modePrim.code[1] = LIBGPU_SYS_get_tw(nullptr);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal PsyQ SDK function: gets the 1st code field for a 'DR_MODE' primitive based on the given draw mode params.
// For more details see "GP0(E1h) - Draw Mode setting (aka "Texpage")":
//      https://problemkaputt.de/psx-spx.htm#gpudisplaycontrolcommandsgp1
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBGPU_SYS_get_mode(
    const bool bCanDrawInDisplayArea,
    const bool bDitheringOn,
    const uint16_t texPageId
) noexcept {
    // Note: I've simplified this a lot from the actual disassembled code.
    //
    // Depending on the value of an unknown LIBGPU global byte (via an if/else statement), the bit values used for dithering, tex page id etc. here
    // would be very different. I've cut out the these alternate values for all the fields because I couldn't see any mention of them in the NO$PSX specs.
    // They may be different values required for special development GPUs or arcade boards, I'm not sure...
    constexpr uint32_t CMD_HEADER = 0xE1000000;

    const uint32_t ditherBits = (bDitheringOn != 0) ? 0x200 : 0x0;
    const uint32_t texPageIdBits = texPageId & 0x9FF;
    const uint32_t drawInDisplayAreaBits = (bCanDrawInDisplayArea != 0) ? 0x400 : 0x0;

    return CMD_HEADER | ditherBits | texPageIdBits | drawInDisplayAreaBits;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal (non-exposed) PSY-Q SDK Function:
// Encodes the given rect into a 32-bit texture window setting used by the hardware.
// For more details see "GP0(E2h) - Texture Window setting":
//      https://problemkaputt.de/psx-spx.htm#gpudisplaycontrolcommandsgp1
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBGPU_SYS_get_tw(const RECT* const pRect) noexcept {
    // If no rect then return an invalid encoding
    if (!pRect)
        return 0;
    
    // Encode the texture window using 5 bits for each piece of info
    constexpr uint32_t CMD_HEADER = 0xE2000000;

    const uint32_t twOffsetX = (uint16_t)(pRect->x & 0xFF) >> 3;
    const uint32_t twOffsetY = (uint16_t)(pRect->y & 0xFF) >> 3;
    const uint32_t twMaskX = (uint16_t)(-pRect->w & 0xFF) >> 3;
    const uint32_t twMaskY = (uint16_t)(-pRect->h & 0xFF) >> 3;

    return (CMD_HEADER | (twOffsetY << 15) | (twOffsetX << 10) | (twMaskY << 5) | twMaskX);
}

void LIBGPU_SYS__ctl() noexcept {
loc_8004DD64:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    sw(a0, v0);
    
    v0 = a0 >> 24;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x3D4;                                        // Result = gLIBGPU_SYS_ctlbuf[0] (800803D4)
    at += v0;
    sb(a0, at);
    return;
}

void LIBGPU_SYS__reset() noexcept {
loc_8004E47C:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a0 = 0;                                             // Result = 00000000
    LIBETC_SetIntrMask();
    a1 = 0;                                             // Result = 00000000
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5D98);                                 // Store to: gpLIBGPU_SYS__qout (80075D98)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    a0 = 0;                                             // Result = 00000000
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x350);                                 // Store to: 80080350
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5D94);                                // Store to: gpLIBGPU_SYS__qin (80075D94)
loc_8004E4BC:
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D4;                                        // Result = gLIBGPU_SYS__que[0] (800804D4)
    at += a0;
    sw(0, at);
    a1++;
    v0 = (i32(a1) < 0x40);
    a0 += 0x60;
    if (v0 != 0) goto loc_8004E4BC;
    v0 = 1;                                             // Result = 00000001
    if (s0 == 0) goto loc_8004E4F4;
    {
        const bool bJump = (s0 == v0);
        v0 = 0x401;                                     // Result = 00000401
        if (bJump) goto loc_8004E544;
    }
    goto loc_8004E590;
loc_8004E4F4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D80);                               // Load from: 80075D80
    v0 = 0x401;                                         // Result = 00000401
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D90);                               // Load from: 80075D90
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x3D4;                                        // Result = gLIBGPU_SYS_ctlbuf[0] (800803D4)
    v0 = lw(v1);
    a1 = 0;                                             // Result = 00000000
    v0 |= 0x800;
    sw(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    a2 = 0x100;                                         // Result = 00000100
    sw(0, v0);
    LIBGPU_SYS_memset();
    goto loc_8004E590;
loc_8004E544:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D80);                               // Load from: 80075D80
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D90);                               // Load from: 80075D90
    v0 = lw(v1);
    v0 |= 0x800;
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = 0x2000000;                                     // Result = 02000000
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = 0x1000000;                                     // Result = 01000000
    sw(v0, v1);
loc_8004E590:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 + 0x350);                                // Load from: 80080350
    LIBETC_SetIntrMask();
    v0 = 0;                                             // Result = 00000000
    if (s0 != 0) goto loc_8004E630;
    v1 = 0x10000000;                                    // Result = 10000000
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v1 |= 7;                                            // Result = 10000007
    sw(v1, v0);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    v1 = 0xFF0000;                                      // Result = 00FF0000
    v0 = lw(a0);
    v1 |= 0xFFFF;                                       // Result = 00FFFFFF
    v0 &= v1;
    v1 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v0 != v1);
        v1 = 0xE1000000;                                // Result = E1000000
        if (bJump) goto loc_8004E5E8;
    }
    v0 = 3;                                             // Result = 00000003
    goto loc_8004E630;
loc_8004E5E8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(v0);
    v1 |= 0x1000;                                       // Result = E1001000
    v0 &= 0x3FFF;
    v0 |= v1;
    sw(v0, a0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(v0);
    v0 = lw(v1);
    v0 >>= 12;
    v0 &= 1;
loc_8004E630:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_SYS_memset() noexcept {
loc_8004E884:
    sp -= 8;
    v0 = a2 - 1;
    if (a2 == 0) goto loc_8004E8A4;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8004E894:
    sb(a1, a0);
    v0--;
    a0++;
    if (v0 != v1) goto loc_8004E894;
loc_8004E8A4:
    sp += 8;
    return;
}

uint16_t LIBGPU_GetTPage(
    const int32_t texFmt,
    const int32_t semiTransRate,
    const int32_t tpageX,
    const int32_t tpageY
) noexcept {
    a0 = texFmt;
    a1 = semiTransRate;
    a2 = tpageX;
    a3 = tpageY;

    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s3, sp + 0x1C);
    s3 = a2;
    sw(s1, sp + 0x14);
    sw(ra, sp + 0x20);
    s1 = a3;
    LIBGPU_GetGraphType();
    v1 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v0 == v1);
        v1 = s0 & 3;
        if (bJump) goto loc_8004E974;
    }
    LIBGPU_GetGraphType();
    v1 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v0 != v1);
        v1 = s0 & 3;
        if (bJump) goto loc_8004E99C;
    }
loc_8004E974:
    v1 <<= 9;
    v0 = s2 & 3;
    v0 <<= 7;
    v1 |= v0;
    v0 = s1 & 0x300;
    v0 = u32(i32(v0) >> 3);
    v1 |= v0;
    v0 = s3 & 0x3FF;
    v0 = u32(i32(v0) >> 6);
    goto loc_8004E9CC;
loc_8004E99C:
    v1 <<= 7;
    v0 = s2 & 3;
    v0 <<= 5;
    v1 |= v0;
    v0 = s1 & 0x100;
    v0 = u32(i32(v0) >> 4);
    v1 |= v0;
    v0 = s3 & 0x3FF;
    v0 = u32(i32(v0) >> 6);
    v1 |= v0;
    v0 = s1 & 0x200;
    v0 <<= 2;
loc_8004E9CC:
    v0 |= v1;
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;

    return (uint16_t) v0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the CLUT id for a CLUT uploaded to the given coordinate in VRAM.
// Note: the x address is limited to being in multiples of 16.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t LIBGPU_GetClut(const int32_t x, const int32_t y) noexcept {
    return (uint16_t)((y << 6) | ((x >> 4) & 0x3F));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable/disable semi-transparency on the specified drawing primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetSemiTrans(void* const pPrim, const bool bTransparent) noexcept {
    uint8_t& primCode = ((uint8_t*) pPrim)[7];

    if (bTransparent) {
        primCode |= 0x2;
    } else {
        primCode &= 0xFD;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable or disable texture shading on a specified primitive.
// When shading is disabled, the texture is displayed as-is.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetShadeTex(void* const pPrim, const bool bDisableShading) noexcept {
    uint8_t& primCode = ((uint8_t*) pPrim)[7];

    if (bDisableShading) {
        primCode |= 1;
    } else {
        primCode &= 0xFE;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a flat shaded textured triangle primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetPolyFT3(POLY_FT3& poly) noexcept {
    LIBGPU_setlen(poly, 7);
    poly.code = 0x24;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a flat shaded quad primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetPolyF4(POLY_F4& poly) noexcept {
    LIBGPU_setlen(poly, 5);
    poly.code = 0x28;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a flat shaded textured quad primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetPolyFT4(POLY_FT4& poly) noexcept {
    LIBGPU_setlen(poly, 9);
    poly.code = 0x2C;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize an 8x8 sprite primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetSprt8(SPRT_8& sprite) noexcept {
    LIBGPU_setlen(sprite, 3);
    sprite.code = 0x74;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a sprite primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetSprt(SPRT & sprt) noexcept {
    LIBGPU_setlen(sprt, 4);
    sprt.code = 0x64;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the specified primitive as a flat shaded and unconnected line
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetLineF2(LINE_F2& line) noexcept {
    LIBGPU_setlen(line, 3);
    line.code = 0x40;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set which print stream to use for debug printing.
// Note: for this re-implementation of LIBGPU multiple debug print streams are NOT supported, therfore this call is ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetDumpFnt([[maybe_unused]] const int32_t printStreamId) noexcept {
    // Ignored.. Not supporting multiple debug print streams!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the font used for debug printing to the given location in VRAM.
// This step must be performed prior to debug printing.
//
// The font data occupies 128x32 pixels in 4-bit mode or 32x32 pixels when interpreted as 16-bit.
// The CLUT for the font is placed at location 'dstX, dstY + 128'.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_FntLoad(const int32_t dstX, const int32_t dstY) noexcept {
    gDFontClutId = LIBGPU_LoadClut(gLIBGPU_DebugFont_Clut, dstX, dstY + 128);
    gDFontTPageId = LIBGPU_LoadTPage((const int16_t*) gLIBGPU_DebugFont_Texture, 0, 0, dstX, dstY, 128, 32);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open a print stream for debug printing with the debug font.
// Sets the parameters for print area and whether to clear the background to 0,0,0.
// The id of the opened print stream is returned.
//
// Params:
//  dispX, dispY:   top left position of the screen RECT to output the debug text to.
//  dispW, dispH:   bounds of the area to output debug text to.
//  bClearBg:       if true then the background is cleared to 0,0,0 when 'flushing' (displaying) debug text.
//  maxChars:       maximum number of characters to print. This field is IGNORED for this re-implementation of LIBGPU.
//
// Note:
//  (1) For this re-implementation of LIBGPU multiple debug print streams are NOT supported, this call always returns '0'.
//  (2) There is no way or need provided to close this 'stream'. It's always there and always has a fixed overhead.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBGPU_FntOpen(
    const int32_t dispX,
    const int32_t dispY,
    const int32_t dispW,
    const int32_t dispH,
    const bool bClearBg,
    [[maybe_unused]] const int32_t maxChars
) noexcept {    
    gDFontDispX = (int16_t) dispX;
    gDFontDispY = (int16_t) dispY;
    gDFontDispW = (int16_t) dispW;
    gDFontDispH = (int16_t) dispH;
    gDbgMsgBufPos = 0;

    return 0;   // Print stream id is always 0 in this cut down LIBGPU!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flushes the message buffer for debug printing so it is displayed to the screen.
// After this call the buffer position is also reset.
// Note: the given print stream id field is IGNORED because this reimplementation of LIBGPU does not support multiple debug print streams.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_FntFlush([[maybe_unused]] const int32_t printStreamId) noexcept {
    // Clear the drawing area if specified
    if (gDFontClearBg) {
        clearDrawingArea(0, 0, 0);
    }

    // Set the current texture page id to use and clear the texture window
    {
        DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);
        LIBGPU_SetDrawMode(drawModePrim, false, false, gDFontTPageId, nullptr);
        LIBGPU_TermPrim(drawModePrim);
        LIBGPU_DrawOTag(&drawModePrim);
        LIBGPU_DrawSync(0);
    }

    // Use this primitive to do the printing - do common setup
    SPRT_8 spritePrim = {};
    LIBGPU_SetSprt8(spritePrim);
    LIBGPU_TermPrim(spritePrim);
    LIBGPU_setRGB0(spritePrim, 127, 127, 127);
    spritePrim.clut = gDFontClutId;
    
    // Print all characters in the message buffer
    const int16_t xend = gDFontDispX + gDFontDispW;
    const int16_t yend = gDFontDispY + gDFontDispH;

    int16_t xpos = gDFontDispX;
    int16_t ypos = gDFontDispY;

    for (int32_t charIdx = 0; charIdx < gDbgMsgBufPos; ++charIdx) {
        // Get the character and ignore if a non printable control char or past the printable range.
        // Only 64 glyphs are supported for the debug font, from decimal 32 to 95 (inclusive).
        // For lower case alpha characters we upper case so that they may be printed:
        const char c = gDbgMsgBuf[charIdx];
        const char cUpper = (c >= 'a' && c <= 'z') ? c - 32 : c;

        if (cUpper == ' ') {
            xpos += 8;
        }
        else if (cUpper == '\t') {
            xpos += 32;
        }
        else if (cUpper == '\n') {
            xpos = gDFontDispX;
            ypos += 8;
        }
        else if (cUpper >= 32 && cUpper <= 95) {
            // Normal non space font character, this one we will actually print
            const uint8_t glyphIdx = (uint8_t)(cUpper - 32);
            const uint8_t glyphRow = glyphIdx / 16;
            const uint8_t glyphCol = glyphIdx % 16;

            spritePrim.x0 = xpos;
            spritePrim.y0 = ypos;
            spritePrim.tu0 = glyphCol * 8;
            spritePrim.tv0 = glyphRow * 8;

            LIBGPU_DrawOTag(&spritePrim);
            LIBGPU_DrawSync(0);

            xpos += 8;
        }

        // Bounds checking
        if (xpos >= xend) {
            xpos = gDFontDispX;
            ypos += 8;
        }

        if (ypos >= yend) {
            break;
        }
    }

    // Reset the buffer position for next time round
    gDbgMsgBufPos = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print the given formatted message to the given print stream id.
//
// Notes:
//  (1) Print stream id is IGNORED for this reimplementation of LIBGPU. Only a single debug print stream is supported!
//  (2) The debug font must be 'flushed' to display the message.
//  (3) If the message buffer is full then the rest of the message will be ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_FntPrint([[maybe_unused]] const int32_t printStreamId, const char* const fmtMsg, ...) noexcept {
    // Message buffer already full (counting the null) ? If so then ignore...
    if (gDbgMsgBufPos + 1 >= DBG_MSG_BUF_SIZE)
        return;

    // Print the message to the message buffer
    va_list args;
    va_start(args, fmtMsg);

    const int32_t bufCharsAvailable = DBG_MSG_BUF_SIZE - gDbgMsgBufPos;
    const int numCharsWritten = vsnprintf(gDbgMsgBuf, (size_t) bufCharsAvailable, fmtMsg, args);
    gDbgMsgBufPos += numCharsWritten;
    
    va_end(args);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the given texture into VRAM.
// Returns the texture page id for the loaded texture.
//
// Params:
//  (1) pImageData:     The image data.
//  (2) bitDepth:       Image bit depth: 0 = 4 bit; 1 = 8 bit; 2 = 16 bit
//  (3) semiTransRate:  Semi transparency rate: see 'LIBGPU_getTPage' for more details.
//  (4) dstX, dstY,     Bounds of the destination RECT in vram.
//      imgW, imgH:     Note: these coordinates are in terms of the image format (4/8/16 bpp), and will be divided accordingly
//                      to obtain the address in VRAM when the pixels are interpreted as 16-bit.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t LIBGPU_LoadTPage(
    const void* pImageData,
    const int32_t bitDepth,
    const int32_t semiTransRate,
    const int32_t dstX,
    const int32_t dstY,
    const int32_t imgW,
    const int32_t imgH
) noexcept {
    // Figure out the destination RECT in VRAM in terms of 16-bit pixels
    RECT dstRect;  
    dstRect.x = (int16_t) dstX;
    dstRect.y = (int16_t) dstY;
    dstRect.h = (int16_t) imgH;
    
    if (bitDepth == 0) {
        dstRect.w = (int16_t)(imgW / 4);    // 4-bits per pixel mode
    } else if (bitDepth == 1) {        
        dstRect.w = (int16_t)(imgW / 2);    // 8-bits per pixel mode
    } else if (bitDepth == 2) {
        dstRect.w = (int16_t) imgW;         // 16-bits per pixel mode
    }

    // Upload the image to VRAM and return it's texture page id
    LIBGPU_LoadImage(dstRect, (const uint16_t*) pImageData);
    return LIBGPU_GetTPage(bitDepth, semiTransRate, dstX, dstY);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Uploads the given color lookup table to the specified location in VRAM.
// Returns the Clut ID for the uploaded CLUT.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t LIBGPU_LoadClut(const uint16_t* pColors, const int32_t x, const int32_t y) noexcept {
    RECT dstRect;
    dstRect.x = (int16_t) x;
    dstRect.y = (int16_t) y;
    dstRect.w = 256;
    dstRect.h = 1;
    LIBGPU_LoadImage(dstRect, pColors);

    return LIBGPU_GetClut(x, y);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the given draw environment struct to the defaults with the given draw area
//------------------------------------------------------------------------------------------------------------------------------------------
DRAWENV& LIBGPU_SetDefDrawEnv(DRAWENV& env, const int32_t x, const int32_t y, const int32_t w, const  int32_t h) noexcept {
  env.clip.x = (int16_t) x;
  env.clip.y = (int16_t) y;
  env.clip.w = (int16_t) w;
  env.clip.h = (int16_t) h;

  env.ofs[0] = (int16_t) x;
  env.ofs[1] = (int16_t) y;

  env.tw.x = 0;
  env.tw.y = 0;
  env.tw.w = 0;
  env.tw.h = 0;
  env.tpage = LIBGPU_GetTPage(0, 0, 640, 0);

  env.dtd = 1;
  env.dfe = (h != 480);

  env.r0 = 0;
  env.g0 = 0;
  env.b0 = 0;  
  
  env.isbg = 0;
  return env;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the given display environment struct to the defaults with the given display area
//------------------------------------------------------------------------------------------------------------------------------------------
DISPENV& LIBGPU_SetDefDispEnv(DISPENV& disp, const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept {
  disp.disp.x = (int16_t) x;
  disp.disp.y = (int16_t) y;
  disp.disp.w = (int16_t) w;
  disp.disp.h = (int16_t) h;

  disp.screen.x = 0;
  disp.screen.y = 0;
  disp.screen.w = 0;
  disp.screen.h = 0;

  disp.isinter = 0;
  disp.isrgb24 = 0;
  
  disp._pad[0] = 0;
  disp._pad[1] = 0;
  
  return disp;
}
