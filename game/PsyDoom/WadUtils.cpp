//------------------------------------------------------------------------------------------------------------------------------------------
// Free standing utility functions relating to WAD files
//------------------------------------------------------------------------------------------------------------------------------------------
#include "WadUtils.h"

BEGIN_NAMESPACE(WadUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Decompresses the given compressed lump data into the given output buffer.
// The compression algorithm used is a form of LZSS.
// Assumes the output buffer is sized big enough to hold all of the decompressed data.
//------------------------------------------------------------------------------------------------------------------------------------------
void decompressLump(const void* const pSrc, void* const pDst) noexcept {
    const uint8_t* pSrcByte = (const uint8_t*) pSrc;
    uint8_t* pDstByte = (uint8_t*) pDst;

    uint32_t idByte = 0;        // Controls whether there is compressed or uncompressed data ahead
    uint32_t haveIdByte = 0;    // Controls when to read an id byte, when '0' we need to read another one

    while (true) {
        // Read the id byte if required.
        // We need 1 id byte for every 8 bytes of uncompressed output, or every 8 runs of compressed data.
        if (haveIdByte == 0) {
            idByte = *pSrcByte;
            ++pSrcByte;
        }

        haveIdByte = (haveIdByte + 1) & 7;

        if (idByte & 1) {
            // Compressed data ahead: the first 12-bits tells where to take repeated data from.
            // The remaining 4-bits tell how many bytes of repeated data to take.
            const uint32_t srcByte1 = pSrcByte[0];
            const uint32_t srcByte2 = pSrcByte[1];
            pSrcByte += 2;

            const int32_t srcOffset = ((srcByte1 << 4) | (srcByte2 >> 4)) + 1;
            const int32_t numRepeatedBytes = (srcByte2 & 0xF) + 1;

            // A value of '1' is a special value and means we have reached the end of the compressed stream
            if (numRepeatedBytes == 1)
                break;

            const uint8_t* const pRepeatedBytes = pDstByte - srcOffset;

            for (int32_t i = 0; i < numRepeatedBytes; ++i) {
                *pDstByte = pRepeatedBytes[i];
                ++pDstByte;
            }
        } else {
            // Uncompressed data: just copy the input byte
            *pDstByte = *pSrcByte;
            ++pSrcByte;
            ++pDstByte;
        }

        idByte >>= 1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Similar to 'decompressLump' except that it does not do any decompression.
// Instead this function returns the decompressed size of the lump data, given just the data itself.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getDecompressedLumpSize(const void* const pSrc) noexcept {
    // This code is pretty much a replica of 'decompressLump()' - see that function for more details/comments.
    // The only difference here is that we don't save the decompressed data and just count the number of output bytes instead.
    const uint8_t* pSrcByte = (uint8_t*) pSrc;
    int32_t size = 0;

    uint32_t idByte = 0;
    uint32_t haveIdByte = 0;

    while (true) {
        if (haveIdByte == 0) {
            idByte = *pSrcByte;
            ++pSrcByte;
        }

        haveIdByte = (haveIdByte + 1) & 7;

        if (idByte & 1) {
            // Note: not bothering to read the byte containing only positional information for the replicated data.
            // We are only interested in the byte count for this function.
            const uint32_t srcByte2 = pSrcByte[1];
            pSrcByte += 2;
            const uint32_t numRepeatedBytes = (srcByte2 & 0xF) + 1;

            if (numRepeatedBytes == 1)
                break;

            size += numRepeatedBytes;
        } else {
            ++size;
            ++pSrcByte;
        }

        idByte >>= 1;
    }

    return size;
}

END_NAMESPACE(WadUtils)
