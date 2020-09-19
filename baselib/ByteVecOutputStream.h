#pragma once

#include "OutputStream.h"

#include <cstddef>
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------------------
// An output stream that serializes to a byte vector stored on the stream
//------------------------------------------------------------------------------------------------------------------------------------------
class ByteVecOutputStream final : public OutputStream {
public:
    inline ByteVecOutputStream() noexcept : mBytes() {}

    inline ByteVecOutputStream(ByteVecOutputStream&& other) noexcept
        : mBytes(std::move(other.mBytes))
    {
    }

    virtual void writeBytes(const void* const pSrcBytes, const size_t numBytes) THROWS override {
        const std::byte* const pSrcIter = (const std::byte*) pSrcBytes;
        mBytes.insert(mBytes.end(), pSrcIter, pSrcIter + numBytes);
    }

    virtual void fillBytes(const size_t numBytes, const std::byte byteValue) THROWS override {
        mBytes.resize(mBytes.size() + numBytes, byteValue);
    }

    virtual size_t tell() noexcept override {
        return mBytes.size();
    }

    virtual void flush() noexcept override {}

    inline std::vector<std::byte>& getBytes() noexcept {
        return mBytes;
    }

    inline const std::vector<std::byte>& getBytes() const noexcept {
        return mBytes;
    }

    // Clear the output and start writing again from byte 0
    inline void reset() noexcept {
        mBytes.clear();
    }

private:
    std::vector<std::byte>  mBytes;
};
