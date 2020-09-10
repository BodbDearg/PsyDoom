#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides a byte oriented input stream from a given chunk of memory.
// The stream is merely a view/wrapper around the given memory chunk and does NOT own the memory.
//------------------------------------------------------------------------------------------------------------------------------------------
class ByteInputStream {
public:
    class StreamException {};   // Thrown when there is a problem reading etc.

    inline ByteInputStream(const std::byte* const pData, const uint32_t size) noexcept
        : mpData(pData)
        , mSize(size)
        , mCurByteIdx(0)
    {
    }

    inline ByteInputStream(const ByteInputStream& other) noexcept = default;

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Get the current offset and data at the current offset
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline uint32_t tell() const noexcept {
        return mCurByteIdx;
    }

    inline const std::byte* getCurData() const noexcept {
        return mpData + mCurByteIdx;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Get and ensure bytes left
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline uint32_t getNumBytesLeft() const noexcept {
        return mSize - mCurByteIdx;
    }

    inline bool hasBytesLeft(const uint32_t numBytes = 1) const noexcept {
        return (getNumBytesLeft() >= numBytes);
    }

    inline void ensureBytesLeft(const uint32_t numBytes = 1) THROWS {
        if (!hasBytesLeft(numBytes)) {
            throw StreamException();
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Reading and consuming bytes
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline void consume(const uint32_t numBytes) THROWS {
        ensureBytesLeft(numBytes);
        mCurByteIdx += numBytes;
    }

    template <class T>
    inline void read(T& output) THROWS {
        ensureBytesLeft(sizeof(T));
        std::memcpy(&output, mpData + mCurByteIdx, sizeof(T));
        mCurByteIdx += sizeof(T);
    }

    template <class T>
    inline T read() THROWS {
        ensureBytesLeft(sizeof(T));

        T output;
        std::memcpy(&output, mpData + mCurByteIdx, sizeof(T));
        mCurByteIdx += sizeof(T);

        return output;
    }

    inline void readBytes(void* const pDstBytes, const uint32_t numBytes) THROWS {
        ensureBytesLeft(numBytes);
        std::memcpy(pDstBytes, mpData + mCurByteIdx, numBytes);
        mCurByteIdx += numBytes;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Aligns the memory stream to the given byte boundary (2, 4, 8 etc.)
    // Note: call is ignored if at the end of the stream!
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline void align(const uint32_t numBytes) THROWS {
        if (mCurByteIdx >= mSize)
            return;

        if (numBytes >= 2) {
            const uint32_t modulus = mCurByteIdx % numBytes;

            if (modulus > 0) {
                const uint32_t numBytesToAlign = numBytes - modulus;
                ensureBytesLeft(numBytesToAlign);
                mCurByteIdx += numBytesToAlign;
            }
        }
    }

private:
    const std::byte* const  mpData;
    const uint32_t          mSize;
    uint32_t                mCurByteIdx;
};
