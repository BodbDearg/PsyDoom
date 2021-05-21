#pragma once

#include "WadFile.h"

BEGIN_NAMESPACE(WadUtils)

void decompressLump(const void* const pSrc, void* const pDst) noexcept;
int32_t getDecompressedLumpSize(const void* const pSrc) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Converts a name into a 64-bit integer of 8 characters maximum. Also performs an ASCII uppercase operation.
// This operation can be also performed at compile time to precompute the conversion to a single 64-bit integer.
//------------------------------------------------------------------------------------------------------------------------------------------
inline constexpr WadLumpName makeUppercaseLumpName(const char* const name) noexcept {
    // Get up to 8 characters of the name in uppercase
    WadLumpName lumpName = WadLumpName(0, 0, 0, 0, 0, 0, 0, 0);

    constexpr auto ucase = [](const char c) noexcept {
        return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    };

    do {
        const char c0 = name[0];    if (c0) { lumpName.chars[0] = ucase(c0); } else { break; }
        const char c1 = name[1];    if (c1) { lumpName.chars[1] = ucase(c1); } else { break; }
        const char c2 = name[2];    if (c2) { lumpName.chars[2] = ucase(c2); } else { break; }
        const char c3 = name[3];    if (c3) { lumpName.chars[3] = ucase(c3); } else { break; }
        const char c4 = name[4];    if (c4) { lumpName.chars[4] = ucase(c4); } else { break; }
        const char c5 = name[5];    if (c5) { lumpName.chars[5] = ucase(c5); } else { break; }
        const char c6 = name[6];    if (c6) { lumpName.chars[6] = ucase(c6); } else { break; }
        const char c7 = name[7];    if (c7) { lumpName.chars[7] = ucase(c7); } else { break; }
    } while (false);

    return lumpName;
}

END_NAMESPACE(WadUtils)
