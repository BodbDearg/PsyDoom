#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(movie)

class MBlockBitStream;

//------------------------------------------------------------------------------------------------------------------------------------------
// Responsible for reading, decoding and storing a single block of color/luminance information in a MDEC movie.
// The area represented is an 8x8 pixel square.
// Housed within a 'macro' block that contains all of the color channels for a 16x16 block of pixels.
//------------------------------------------------------------------------------------------------------------------------------------------
struct Block {
    static constexpr uint32_t PIXELS_W = 8;     // Width of the block in pixels
    static constexpr uint32_t PIXELS_H = 8;     // Height of the block in pixels

    void clear() noexcept;
    bool readAndDecode(MBlockBitStream& inputStream, const int16_t quantizationScale) noexcept;

    int16_t mValues[PIXELS_H][PIXELS_W];
};

END_NAMESPACE(movie)
