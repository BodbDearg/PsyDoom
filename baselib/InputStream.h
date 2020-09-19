#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a generic input stream
//------------------------------------------------------------------------------------------------------------------------------------------
class InputStream {
public:
    // Thrown when there is a problem reading or opening the stream, etc.
    class StreamException {};

    InputStream() noexcept {}
    virtual ~InputStream() noexcept {}

    // Read a specified number of bytes from the stream into the given buffer.
    // The buffer is assumed to be not null if there is data to be read.
    virtual void readBytes(void* const pDstBytes, const size_t numBytes) THROWS = 0;

    // Discard a specified number of bytes from the stream
    virtual void skipBytes(const size_t numBytes) THROWS = 0;

    // Returns the current offset in the stream
    virtual size_t tell() THROWS = 0;

    // Tells if the end of the stream has been reached
    virtual bool isAtEnd() THROWS = 0;

    // Read and return directly the specified generic type from the input stream
    template <class T>
    inline T read() THROWS {
        T value;
        readBytes(&value, sizeof(T));
        return value;
    }

    // Reads and returns in-place the specified generic type from the input stream
    template <class T>
    inline void read(T& dstValue) THROWS {
        readBytes(&dstValue, sizeof(T));
    }

    // Read C-Style array of a specified number of entries from the stream
    template <class T>
    inline void readArray(T* const pDstValues, const size_t numValues) THROWS {
        readBytes(pDstValues, sizeof(T) * numValues);
    }

    // Align the stream so that the current offset is a multiple of the given byte count (4, 8, 16 etc.)
    inline void align(const size_t toNumBytes) THROWS {
        if (toNumBytes < 2)
            return;

        const size_t offset = tell();
        const size_t modulus = offset % toNumBytes;

        if (modulus > 0) {
            const size_t numSkipBytes = toNumBytes - modulus;
            skipBytes(numSkipBytes);
        }
    }
};
