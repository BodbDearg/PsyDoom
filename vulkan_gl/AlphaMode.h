#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Enum representing how alpha is handled (premultipled vs non-premultiplied).
// This doesn't affect the texture format in any way, it's just a hint to the application how to handle the texture's alpha channel.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class AlphaMode : uint8_t {
    UNSPECIFIED,        // Alpha mode is not specified; up to user code to determine
    PREMULTIPLIED,      // Alpha is premultiplied
    STRAIGHT            // Alpha is NOT premultiplied
};

static const uint8_t AlphaMode_NUM_VALUES = uint8_t(AlphaMode::STRAIGHT) + 1;

END_NAMESPACE(vgl)
