#pragma once

#include "InputStream.h"

#include <cstring>

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides a byte oriented input stream from a given chunk of memory.
// The stream is merely a view/wrapper around the given memory chunk and does NOT own the memory.
//------------------------------------------------------------------------------------------------------------------------------------------
class ByteInputStream final : public InputStream {
public:
    inline ByteInputStream(const std::byte* const pData, const uint32_t size) noexcept
        : mpData(pData)
        , mSize(size)
        , mCurByteIdx(0)
    {
    }

    inline ByteInputStream(const ByteInputStream& other) noexcept = default;

    virtual void readBytes(void* const pDstBytes, const size_t numBytes) THROWS override {
        ensureBytesLeft(numBytes);
        std::memcpy(pDstBytes, mpData + mCurByteIdx, numBytes);
        mCurByteIdx += numBytes;
    }

    virtual void skipBytes(const size_t numBytes) THROWS override {
        ensureBytesLeft(numBytes);
        mCurByteIdx += numBytes;
    }

    virtual size_t tell() noexcept override {
        return mCurByteIdx;
    }

    virtual bool isAtEnd() noexcept override {
        return (mCurByteIdx >= mSize);
    }

    // Return the next value ahead (of the given type) but do not consume the input.
    // This is possible for this stream type because we have all of the bytes available to us at all times.
    template <class T>
    T peek() {
        ensureBytesLeft(sizeof(T));
        T output;
        std::memcpy(&output, mpData + mCurByteIdx, sizeof(T));
        return output;
    }

private:
    inline void ensureBytesLeft(const size_t numBytes) THROWS {
        if ((numBytes > mSize) || (mCurByteIdx + numBytes > mSize)) {
            throw StreamException();
        }
    }

    const std::byte* const  mpData;
    const size_t            mSize;
    size_t                  mCurByteIdx;
};
