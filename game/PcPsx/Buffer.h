#pragma once

#include "FatalErrors.h"

#include <cstddef>
#include <cstdlib>

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides a simple byte buffer that can be resized if required
//------------------------------------------------------------------------------------------------------------------------------------------
class Buffer {
public:
    inline Buffer() noexcept
        : mpBytes(nullptr)
        , mSize(0)
    {
    }

    inline Buffer(size_t size) noexcept : Buffer() {
        resize(size);
    }

    inline Buffer(Buffer&& other) noexcept
        : mpBytes(other.mpBytes)
        , mSize(other.mSize)
    {
        other.mpBytes = nullptr;
        other.mSize = 0;
    }

    inline ~Buffer() noexcept {
        free();
    }

    inline std::byte* bytes() const noexcept {
        return mpBytes;
    }

    inline size_t size() const noexcept {
        return mSize;
    }

    // Ensure the buffer meets a certain minimum size.
    // Resizes the buffer as neccesary to meet the target, but may size the buffer bigger in anticipation of even bigger allocations in future.
    inline void ensureSize(const size_t targetSize) noexcept {
        // Already big enough?
        if (mSize >= targetSize)
            return;

        // Grow the size 2x until it meets the target, then resize the buffer
        size_t newSize = (mSize != 0) ? mSize : 1;

        while (newSize < targetSize) {
            newSize *= 2;
        }

        resize(newSize);
    }

    // Resize the buffer to be the exact specified size (in bytes).
    // Existing buffer contents are preserved up to the min(oldSize, newSize).
    inline void resize(const size_t newSize) noexcept {
        // Don't resize if we don't have to
        if (newSize == mSize)
            return;

        // Do the expansion or contraction
        if (newSize > 0) {
            std::byte* const pOldBytes = mpBytes;
            mpBytes = (std::byte*) std::realloc(mpBytes, newSize);

            if (mpBytes) {
                mSize = newSize;
            } else {
                mpBytes = pOldBytes;            // Realloc failed! Just saving the old pointer so it can be freed on shutdown.
                FatalErrors::outOfMemory();     // Die with out of memory.
            }
        } else {
            free();
        }
    }

    // Frees up the buffer's memory.
    // After the call the buffer becomes size '0'.
    inline void free() noexcept {
        if (mpBytes) {
            std::free(mpBytes);
        }

        mpBytes = nullptr;
        mSize = 0;
    }

private:
    // These operations are disallowed
    Buffer(const Buffer& other) = delete;
    Buffer& operator = (const Buffer& other) = delete;
    Buffer& operator = (Buffer&& other) = delete;

    std::byte*  mpBytes;
    size_t      mSize;
};
