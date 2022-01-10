#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(movie)

class MBlockBitStream;

BEGIN_NAMESPACE(MacroBlockDecoder)

static constexpr uint32_t PIXELS_W = 16;    // Width of the decoded block in pixels
static constexpr uint32_t PIXELS_H = 16;    // Height of the decoded block in pixels

bool decode(
    MBlockBitStream& inputStream,
    const int16_t quantizationScale,
    uint32_t pPixelsOut[PIXELS_H][PIXELS_W]     // 32-bit ABGR8888 format
) noexcept;

END_NAMESPACE(MacroBlockDecoder)
END_NAMESPACE(movie)
