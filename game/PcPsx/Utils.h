#pragma once

#include <cstdint>

// Tells if the host CPU is little endian
static inline constexpr bool isHostCpuLittleEndian() noexcept {
    // There's no way to tell this in C++ 17 but in C++ 20 there will be.
    // For now assume little endian, since that pretty much dominates computing these days:
    return true;
}
