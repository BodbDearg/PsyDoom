#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a generic output stream
//------------------------------------------------------------------------------------------------------------------------------------------
class OutputStream {
public:
    // Thrown when there is a problem writing or opening the stream, etc.
    class StreamException {};

    OutputStream() noexcept {}
    virtual ~OutputStream() noexcept {}

    // Write a specified number of bytes to the stream from the given buffer
    virtual void writeBytes(const void* const pSrcBytes, const size_t numBytes) THROWS = 0;

    // Fill a specified number of bytes in the stream with the given byte value
    virtual void fillBytes(const size_t numBytes, const std::byte byteValue) THROWS = 0;

    // Returns the current offset in the stream
    virtual size_t tell() THROWS = 0;

    // Flush any pending/buffered writes to the output stream.
    // Should be called at the end of writing to guarantee all data has been written 100% successfully.
    virtual void flush() THROWS = 0;

    // Write directly the specified generic type to the input stream
    template <class T>
    inline void write(const T& srcValue) THROWS {
        writeBytes(&srcValue, sizeof(T));
    }

    // Write a C-Style array of a specified number of entries to the stream
    template <class T>
    inline void writeArray(const T* const pSrcValues, const size_t numValues) THROWS {
        writeBytes(pSrcValues, sizeof(T) * numValues);
    }

    // Fill the stream so its size is a multiple of the specified byte alignment amount
    inline void padAlign(const size_t toNumBytes, const std::byte padByte = std::byte(0)) THROWS {
        if (toNumBytes < 2)
            return;
        
        const size_t offset = tell();
        const size_t modulus = offset % toNumBytes;

        if (modulus > 0) {
            const size_t numFillBytes = toNumBytes - modulus;
            fillBytes(numFillBytes, padByte);
        }
    }
};
