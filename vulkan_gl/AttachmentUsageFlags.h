#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(vgl)

// Bit flags representing how an attachment is used in a particular subpass within a renderpass
namespace AttachmentUsageFlagBits {
    constexpr const uint8_t READ    = 0x01;     // The attachment is read from
    constexpr const uint8_t WRITE   = 0x02;     // The attachment is written to

    // The number of bits required to hold all of these flags
    constexpr const uint8_t NUM_BITS = 2;
}

// Actual type of a field holding these flags
typedef uint8_t AttachmentUsageFlags;

END_NAMESPACE(vgl)
