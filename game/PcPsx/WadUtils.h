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
    WadLumpName lumpName = {};

    for (int i = 0; i < MAX_WAD_LUMPNAME; ++i) {
        const char c = name[i];

        if (c == 0)
            break;

        if (c >= 'a' && c <= 'z') {
            lumpName.chars[i] = c - 32;
        } else {
            lumpName.chars[i] = c;
        }
    }

    return lumpName;
}

END_NAMESPACE(WadUtils)
