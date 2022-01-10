//------------------------------------------------------------------------------------------------------------------------------------------
// A module responsible for decoding a macro block in an MDEC movie
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MacroBlockDecoder.h"

#include "Asserts.h"
#include "Block.h"

#include <algorithm>

BEGIN_NAMESPACE(movie)
BEGIN_NAMESPACE(MacroBlockDecoder)

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to decode a 16x16 block of pixels in the movie.
// Takes the quantization scale for the frame as input and outputs the pixels to the specified array.
// Returns 'false' on failure to read or decode the block.
// For more on this, see: https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
bool decode(
    MBlockBitStream& inputStream,
    const int16_t quantizationScale,
    uint32_t pPixelsOut[PIXELS_H][PIXELS_W]
) noexcept {
    ASSERT(pPixelsOut);

    // Firstly decode the 6 blocks within this macro block:
    // 
    //  cr = Chroma Red
    //  cb = Chroma Blue
    //  y[0] = Luma (top left)
    //  y[1] = Luma (top right)
    //  y[2] = Luma (bottom left)
    //  y[3] = Luma (bottom right)
    //
    // Note that the chroma blocks cover a 16x16 pixel area but each luma block covers a 8x8 pixel area.
    // Thus luma resolution is twice that of color.
    Block blockCr, blockCb, blockY[4];

    const bool bDecodedAllBlocksOk = (
        blockCr.readAndDecode(inputStream, quantizationScale) &&
        blockCb.readAndDecode(inputStream, quantizationScale) &&
        blockY[0].readAndDecode(inputStream, quantizationScale) &&
        blockY[1].readAndDecode(inputStream, quantizationScale) &&
        blockY[2].readAndDecode(inputStream, quantizationScale) &&
        blockY[3].readAndDecode(inputStream, quantizationScale)
    );

    if (!bDecodedAllBlocksOk)
        return false;

    // Process each pixel and convert to ABGR8888 format
    for (uint32_t y = 0; y < PIXELS_H; ++y) {
        for (uint32_t x = 0; x < PIXELS_W; ++x) {
            // Firstly get the chroma red and blue values as well as the luma value
            const float chromaRf = (float) blockCr.mValues[y / 2][x / 2];
            const float chromaBf = (float) blockCb.mValues[y / 2][x / 2];
            const Block& lumaBlock = blockY[(y / 8) * 2 + (x / 8)];
            const float lumaF = (float)(lumaBlock.mValues[y % 8][x % 8] + 128);     // Note: +128 for JPEG 'level shift' (see jpsxdec docs for more on this)

            // Convert YCbCr to RGB and clamp between 0 and 255
            const float colorRf = std::clamp(lumaF + 1.4020f * chromaRf + 0.5f, 0.0f, 255.0f);
            const float colorGf = std::clamp(lumaF - 0.3437f * chromaBf - 0.7143f * chromaRf + 0.5f, 0.0f, 255.0f);
            const float colorBf = std::clamp(lumaF + 1.7720f * chromaBf + 0.5f, 0.0f, 255.0f);

            // Convert to 8-bit RGB and save the output pixel in ABGR8888 format
            const uint32_t colorR = (uint32_t) colorRf;
            const uint32_t colorG = (uint32_t) colorGf;
            const uint32_t colorB = (uint32_t) colorBf;

            pPixelsOut[y][x] = 0xFF000000 | (colorB << 16) | (colorG << 8) | colorR;
        }
    }

    return true;
}

END_NAMESPACE(MacroBlockDecoder)
END_NAMESPACE(movie)
