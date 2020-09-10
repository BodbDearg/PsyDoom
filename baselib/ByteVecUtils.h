#pragma once

#include "Asserts.h"
#include "Macros.h"

#include <cstddef>
#include <vector>

BEGIN_NAMESPACE(ByteVecUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Append the specified number of filler bytes to the byte vector
//------------------------------------------------------------------------------------------------------------------------------------------
void fill(std::vector<std::byte>& dstVec, const uint32_t numBytes, const std::byte fillerByte) noexcept {
    dstVec.resize(dstVec.size() + numBytes, fillerByte);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a generic value to the byte vector by copying it's raw bytes.
// Only suitable for serializing POD types.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline void write(std::vector<std::byte>& dstVec, const T& value) noexcept {
    const size_t oldSize = dstVec.size();
    dstVec.resize(oldSize + sizeof(T));
    std::memcpy(dstVec.data() + oldSize, &value, sizeof(T));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a specified number of bytes to the byte vector
//------------------------------------------------------------------------------------------------------------------------------------------
inline void writeBytes(std::vector<std::byte>& dstVec, const void* const pSrcBytes, const uint32_t numBytes) noexcept {
    const size_t oldSize = dstVec.size();
    dstVec.resize(oldSize + numBytes);
    std::memcpy(dstVec.data() + oldSize, pSrcBytes, numBytes);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pad the byte vector to be a multiple of the given size
//------------------------------------------------------------------------------------------------------------------------------------------
inline void pad(std::vector<std::byte>& dstVec, const uint32_t byteMultiple, const std::byte padByte = std::byte(0)) noexcept {
    if (byteMultiple >= 2) {
        const uint32_t modulus = dstVec.size() % byteMultiple;

        if (modulus > 0) {
            const uint32_t numPadBytes = byteMultiple - modulus;
            fill(dstVec, numPadBytes, padByte);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Concat one byte vector onto another.
// Note: the source and destination cannot be the same!
//------------------------------------------------------------------------------------------------------------------------------------------
inline void concat(std::vector<std::byte>& dstVec, const std::vector<std::byte>& srcVec) noexcept {
    ASSERT(&dstVec != &srcVec);
    dstVec.insert(dstVec.end(), srcVec.begin(), srcVec.end());
}

END_NAMESPACE(ByteVecUtils)
