//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBGPU' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LIBGPU.h"

#include "Asserts.h"
#include "Gpu.h"
#include "LIBETC.h"
#include "PcPsx/BitShift.h"
#include "PcPsx/LIBGPU_CmdDispatch.h"
#include "PcPsx/PsxVm.h"
#include "PcPsx/Video.h"
#include "PcPsx/Vulkan/VRenderer.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

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
// PsyDoom helper function that clears the current drawing area to the specified color
//------------------------------------------------------------------------------------------------------------------------------------------
static void clearDrawingArea(const uint8_t r, const uint8_t g, const uint8_t b) noexcept {
    // Skip clearing the PSX drawing area if we are using the new Vulkan renderer for this frame
    #if PSYDOOM_VULKAN_RENDERER
        if (Video::isUsingVulkanRenderPath())
            return;
    #endif

    Gpu::Color24F clearColor = {};
    clearColor.comp.r = r;
    clearColor.comp.g = g;
    clearColor.comp.b = b;

    Gpu::Core& gpu = PsxVm::gGpu;

    Gpu::clearRect(
        gpu,
        Gpu::color24FTo16(clearColor),
        gpu.drawAreaLx,
        gpu.drawAreaTy,
        (gpu.drawAreaRx - gpu.drawAreaLx) + 1,
        (gpu.drawAreaBy - gpu.drawAreaTy) + 1
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Resets the GPU to the default state.
// This is unimplemented for this cut down version of LIBGPU.
//
// Mode meanings:
//  0 = Reset everything
//  1 = Cancel all drawing and flush the command buffer.
//  3 = Initialize draw environment but preserve display environment.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_ResetGraph([[maybe_unused]] const int32_t resetMode) noexcept {
    // This doesn't need to do anything in this emulated environment for PSX DOOM.
    // When PsyDoom previously used the Avocado PSX emulator I verified it doesn't result in GPU state changes when it is called...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the graphics debugging level from 0-2.
// This call is IGNORED or this cut down version of LIBGPU.
//
// Mode meanings:
//  0 = Reset everything
//  1 = Cancel all drawing and flush the command buffer.
//  3 = Initialize draw environment but preserve display environment.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetGraphDebug([[maybe_unused]] const int32_t debugLevel) noexcept {
    // Nothing to do here...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable/disable display output. If the mask is non zero then the display is enabled.
// PsyDoom: this flag is not supported in the new GPU implementation so this call is a no-op.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetDispMask([[maybe_unused]] const int32_t mask) noexcept {
    // No-op for PsyDoom...
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
    // When we submit something to the 'gpu' it is handled immediately, in a blocking fashion.
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the given image data to the given area in VRAM.
// The image format is assumed to be 16-bit.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_LoadImage(const RECT& dstRect, const uint16_t* const pImageData) noexcept {
    // Sanity checks
    Gpu::Core& gpu = PsxVm::gGpu;

    ASSERT(pImageData);
    ASSERT(dstRect.w > 0);
    ASSERT(dstRect.h > 0);
    ASSERT(dstRect.w <= gpu.ramPixelW);
    ASSERT(dstRect.h <= gpu.ramPixelH);

    // Determine the destination bounds and row size for the copy.
    // Note that we must wrap horizontal coordinates (see comments below).
    const uint16_t rowW = dstRect.w;
    const uint16_t dstLx = (uint16_t)(dstRect.x) & gpu.ramXMask;
    const uint16_t dstRx = (uint16_t)(dstRect.x + rowW - 1) & gpu.ramXMask;
    const uint16_t dstTy = (uint16_t)(dstRect.y);
    const uint16_t dstBy = (uint16_t)(dstRect.y + dstRect.h - 1);

    // Copy each row into VRAM
    uint16_t* const pVram = gpu.pRam;
    const uint16_t* pSrcPixels = pImageData;

    for (uint32_t dstY = dstTy; dstY <= dstBy; ++dstY) {
        // Note: destination Y wrapped due to behavior mentioned in NO$PSX specs (see comments below)
        const uint16_t dstYWrapped = dstY & gpu.ramYMask;
        uint16_t* const pDstRow = pVram + (intptr_t) dstYWrapped * gpu.ramPixelW;

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
            const int32_t numWrappedPixels = dstLx + rowW - gpu.ramPixelW;
            const int32_t numNonWrappedPixels = rowW - numWrappedPixels;
            
            std::memcpy(pDstRow + dstLx, pSrcPixels, numNonWrappedPixels * sizeof(uint16_t));
            pSrcPixels += numNonWrappedPixels;

            std::memcpy(pDstRow, pSrcPixels, numWrappedPixels * sizeof(uint16_t));
            pSrcPixels += numWrappedPixels;
        }
    }

    // Vulkan renderer: push this upload to the Vulkan texture mirroring PSX VRAM
    #if PSYDOOM_VULKAN_RENDERER
        if (Video::gBackendType == Video::BackendType::Vulkan) {
            VRenderer::pushPsxVramUpdates(dstLx, dstRx, dstTy, dstBy);
        }
    #endif  // #if PSYDOOM_VULKAN_RENDERER
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copy one part of VRAM to another part of VRAM
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBGPU_MoveImage(const RECT& srcRect, const int32_t dstX, const int32_t dstY) noexcept {
    // Sanity checks
    Gpu::Core& gpu = PsxVm::gGpu;

    ASSERT((srcRect.w > 0) && (srcRect.h > 0));
    ASSERT((srcRect.x >= 0) && (srcRect.y >= 0));
    ASSERT(srcRect.x + srcRect.w <= gpu.ramPixelW);
    ASSERT(srcRect.y + srcRect.h <= gpu.ramPixelH);
    ASSERT((dstX >= 0) && (dstY >= 0));
    ASSERT(dstX + srcRect.w <= gpu.ramPixelW);
    ASSERT(dstY + srcRect.y <= gpu.ramPixelH);

    // Copy each row
    const uint32_t numRows = srcRect.h;
    const uint32_t rowSize = srcRect.w * sizeof(uint16_t);

    const uint16_t* pSrcRow = gpu.pRam + srcRect.x + (intptr_t) srcRect.y * gpu.ramPixelW;
    uint16_t* pDstRow = gpu.pRam + dstX + (intptr_t) dstY * gpu.ramPixelW;

    for (uint32_t rowIdx = 0; rowIdx < numRows; ++rowIdx) {
        std::memcpy(pDstRow, pSrcRow, rowSize);
        pSrcRow += gpu.ramPixelW;
        pDstRow += gpu.ramPixelW;
    }

    return 0;   // This is the position of the command in the queue, according to PsyQ docs - don't care about this...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the drawing environment and potentially clears the screen also.
// The draw environment includes the display area, texture page and texture window settings, blending settings and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
DRAWENV& LIBGPU_PutDrawEnv(DRAWENV& env) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;
    
    // Set drawing area and offset
    gpu.drawAreaLx = env.clip.x;
    gpu.drawAreaTy = env.clip.y;
    gpu.drawAreaRx = env.clip.x + env.clip.w - 1;
    gpu.drawAreaBy = env.clip.y + env.clip.h - 1;
    
    gpu.drawOffsetX = env.ofs[0];
    gpu.drawOffsetY = env.ofs[1];
    
    // Set the texture window offset and mask.
    // Note: texture windows are assumed powers of 2 for the GPU, this is the only way masking and wraparound can work properly...
    gpu.texWinX = env.tw.x;
    gpu.texWinY = env.tw.y;

    if (env.tw.w == 0) {
        gpu.texWinXMask = 0;
    } else if (env.tw.w > 1) {
        gpu.texWinXMask = env.tw.w - 1;
    } else {
        gpu.texWinXMask = 1;
    }
    
    if (env.tw.h == 0) {
        gpu.texWinYMask = 0;
    } else if (env.tw.h > 1) {
        gpu.texWinYMask = env.tw.h - 1;
    } else {
        gpu.texWinYMask = 1;
    }

    // Set the texture page.
    // Note: also previously setup dithering and whether the 'draw to display area' flag was set here.
    // Those features are no longer supported however by PsyDoom's simplified PSX Gpu emulation.
    LIBGPU_CmdDispatch::setGpuTexPageId(env.tpage);

    // Clear the screen if specified by the DRAWENV.
    // Fill the draw area with the specified background color.
    if (env.isbg) {
        clearDrawingArea(env.r0, env.g0, env.b0);
    }

    return env;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the current display enviroment; this includes information about where in the framebuffer we display from, video resolution etc.
// PsyDoom: video mode is ignored except for the region being displayed, that is the important info...
//------------------------------------------------------------------------------------------------------------------------------------------
DISPENV& LIBGPU_PutDispEnv(DISPENV& env) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;
    gpu.displayAreaX = env.disp.x;
    gpu.displayAreaY = env.disp.y;
    gpu.displayAreaW = env.screen.w;
    gpu.displayAreaH = env.screen.h;
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
    const uint16_t texPageId,
    const RECT* const pNewTexWindow
) noexcept {
    // Set primitive size
    modePrim.tag &= 0x00FFFFFF;
    modePrim.tag |= uint32_t(2) << 24;

    // Set draw mode field
    modePrim.code[0] = LIBGPU_SYS_get_mode(bCanDrawInDisplayArea, bDitheringOn, texPageId);

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Similar to the PSYQ macro version of this function - LIBGPU_getTPage.
//
// In the original PSYQ SDK this version did some additional checks against 'GetGraphType' to determine how to compute the texture page id,
// but those checks are skipped here for simplicity. I'm not sure what 'LIBGPU_GetGraphType' did, it may have been a check for special
// development GPU types.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t LIBGPU_GetTPage(
    const int32_t texFmt,
    const int32_t semiTransRate,
    const int32_t tpageX,
    const int32_t tpageY
) noexcept {
    // This is just an alias for 'LIBGPU_getTPage' (macro  'getTPage') in this PSYQ SDK re-implementation
    return LIBGPU_getTPage(texFmt, semiTransRate, tpageX, tpageY);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the CLUT id for a CLUT uploaded to the given coordinate in VRAM.
// Note: the x address is limited to being in multiples of 16.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t LIBGPU_GetClut(const int32_t x, const int32_t y) noexcept {
    return (uint16_t)(d_lshift<6>(y) | (d_rshift<4>(x) & 0x3F));
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
//  dispX, dispY:   Top left position of the screen RECT to output the debug text to.
//  dispW, dispH:   Bounds of the area to output debug text to.
//  bClearBg:       If true then the background is cleared to 0,0,0 when 'flushing' (displaying) debug text.
//                  (This field is IGNORED for this re-implementation of LIBGPU)
//  maxChars:       Maximum number of characters to print.
//                  (This field is IGNORED for this re-implementation of LIBGPU)
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
    [[maybe_unused]] const bool bClearBg,
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
        DR_MODE drawModePrim = {};
        LIBGPU_SetDrawMode(drawModePrim, false, false, gDFontTPageId, nullptr);
        LIBGPU_CmdDispatch::submit(drawModePrim);
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

            LIBGPU_CmdDispatch::submit(spritePrim);
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

    env.dtd = true;
    env.dfe = (h != 480);

    env.r0 = 0;
    env.g0 = 0;
    env.b0 = 0;
  
    env.isbg = false;
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
