#include "Frame.h"

#include "CDXAFileStreamer.h"
#include "Endian.h"
#include "FatalErrors.h"
#include "MacroBlockDecoder.h"
#include "MBlockBitStream.h"

BEGIN_NAMESPACE(movie)

// In an MDEC video sector there's just 2048 bytes of data, and 32 of that is taken up by the header
constexpr uint32_t VIDEO_DATA_BYTES_PER_SECTOR = 2048 - sizeof(FrameSectorHeader);

//------------------------------------------------------------------------------------------------------------------------------------------
// Endian correction: swaps the bytes in the frame header
//------------------------------------------------------------------------------------------------------------------------------------------
void FrameSectorHeader::byteSwap() noexcept {
    Endian::byteSwapInPlace(status);
    Endian::byteSwapInPlace(type);
    Endian::byteSwapInPlace(chunkNum);
    Endian::byteSwapInPlace(numChunks);
    Endian::byteSwapInPlace(frameNum);
    Endian::byteSwapInPlace(numDemuxedBytes);
    Endian::byteSwapInPlace(frameW);
    Endian::byteSwapInPlace(frameH);
    Endian::byteSwapInPlace(numCodeChunks);
    Endian::byteSwapInPlace(_unused1);
    Endian::byteSwapInPlace(quantizationScale);
    Endian::byteSwapInPlace(mdecVersion);
    Endian::byteSwapInPlace(_unused2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the frame with no data
//------------------------------------------------------------------------------------------------------------------------------------------
Frame::Frame() noexcept
    : mFirstSecHdr()
    , mpDemuxedData(nullptr)
    , mDemuxedDataSize(0)
    , mDemuxedDataCapacity(0)
    , mpPixelBuffer(nullptr)
    , mPixelBufferCapacity(0)
{
    ensureDemuxedDataBufferCapacity(sizeof(VIDEO_DATA_BYTES_PER_SECTOR) * 8);       // Should be enough for all frames...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does cleanup for the frame
//------------------------------------------------------------------------------------------------------------------------------------------
Frame::~Frame() noexcept {
    if (mpDemuxedData) {
        std::free(mpDemuxedData);
        mpDemuxedData = nullptr;
    }

    if (mpPixelBuffer) {
        std::free(mpPixelBuffer);
        mpPixelBuffer = nullptr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears the frame data
//------------------------------------------------------------------------------------------------------------------------------------------
void Frame::clear() noexcept {
    mFirstSecHdr = {};
    mDemuxedDataSize = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to read and decode a frame of video.
// Returns false if that is not possible due to the end of the file being encountered, or some sort of error.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Frame::read(CDXAFileStreamer& cdStreamer, const uint8_t channelNum) noexcept {
    clear();
    const bool bSuccess = (demuxFrame(cdStreamer, channelNum) && decodeMacroBlocks());

    if (!bSuccess) {
        clear();
    }

    return bSuccess;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ensures the demuxed data buffer has at least the specified size
//------------------------------------------------------------------------------------------------------------------------------------------
void Frame::ensureDemuxedDataBufferCapacity(const uint32_t capacity) noexcept {
    if (capacity < mDemuxedDataCapacity)
        return;

    mDemuxedDataCapacity = capacity;
    mpDemuxedData = (std::byte*) std::realloc(mpDemuxedData, capacity);

    if (!mpDemuxedData) {
        FatalErrors::outOfMemory();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ensures the pixel buffer has at least the specified capacity
//------------------------------------------------------------------------------------------------------------------------------------------
void Frame::ensurePixelBufferCapacity(const uint32_t capacity) noexcept {
    if (capacity < mPixelBufferCapacity)
        return;

    mPixelBufferCapacity = capacity;
    mpPixelBuffer = (uint32_t*) std::realloc(mpPixelBuffer, capacity * sizeof(uint32_t));

    if (!mpDemuxedData) {
        FatalErrors::outOfMemory();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Grabs the header for a frame for the specified CD sector and saves it in the output
//------------------------------------------------------------------------------------------------------------------------------------------
void Frame::getFrameSectorHeader(const CDXASector& sector, FrameSectorHeader& hdrOut) noexcept {
    std::memcpy(&hdrOut, sector.data, sizeof(FrameSectorHeader));

    if constexpr (Endian::isBig()) {
        hdrOut.byteSwap();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds the compressed frame data in the specified sector to the end of the demuxed data buffer
//------------------------------------------------------------------------------------------------------------------------------------------
void Frame::bufferFrameData(const CDXASector& sector) noexcept {
    // Make sure we have enough room to hold another sector
    if (mDemuxedDataSize + VIDEO_DATA_BYTES_PER_SECTOR > mDemuxedDataCapacity) {
        const uint32_t reserveAmount = std::max(mDemuxedDataCapacity * 2u, mDemuxedDataCapacity + VIDEO_DATA_BYTES_PER_SECTOR);
        ensureDemuxedDataBufferCapacity(reserveAmount);
    }

    // Buffer the data
    static_assert(VIDEO_DATA_BYTES_PER_SECTOR + sizeof(FrameSectorHeader) <= sizeof(sector.data));
    std::memcpy(mpDemuxedData + mDemuxedDataSize, sector.data + sizeof(FrameSectorHeader), VIDEO_DATA_BYTES_PER_SECTOR);
    mDemuxedDataSize += VIDEO_DATA_BYTES_PER_SECTOR;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Demultiplexes the raw data for a frame by reading it from the specified CD streamer.
// Saves the raw/encoded frame data on this frame. Only processes the specified channel number.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Frame::demuxFrame(CDXAFileStreamer& cdStreamer, const uint8_t channelNum) noexcept {
    // This helper lambda tells if an XA sector is one we are interested in
    const auto isValidVideoSector = [channelNum](const CDXASector& sector) noexcept {
        return (sector.header.isVideoSector() && (sector.header.channelNum == channelNum));
    };

    // Grab the first sector in the frame and abort if not found.
    // Otherwise save the header for the sector and buffer it's data:
    const CDXASector* const pFirstSector = cdStreamer.peekOrReadSectorType(isValidVideoSector);

    if (!pFirstSector)
        return false;

    getFrameSectorHeader(*pFirstSector, mFirstSecHdr);

    // Validate the header and abort if not good, otherwise buffer the frame data and mark us done with that sector
    const bool bValidFirstSectorHeader = (
        (mFirstSecHdr.isMdecHeaderType()) &&
        (mFirstSecHdr.chunkNum == 0) &&     // Expect the frame sectors/chunks to be in sequential order!
        (mFirstSecHdr.numChunks > 0) &&
        (mFirstSecHdr.numDemuxedBytes > 0) &&
        (mFirstSecHdr.frameW > 0) &&
        (mFirstSecHdr.frameH > 0) &&
        (mFirstSecHdr.numCodeChunks > 0) &&
        (mFirstSecHdr.quantizationScale >= 0) &&
        ((mFirstSecHdr.mdecVersion == 1) || (mFirstSecHdr.mdecVersion == 2))    // Version 2 is the only format we handle, version 1 is the same also. Version 3 is UNSUPPORTED!
    );

    if (!bValidFirstSectorHeader)
        return false;

    bufferFrameData(*pFirstSector);
    cdStreamer.freeSector(*pFirstSector);

    // Read all subsequent sectors
    for (uint32_t chunkNum = 1; chunkNum < mFirstSecHdr.numChunks; ++chunkNum) {
        // If a valid sector is not found then reading the frame fails!
        const CDXASector* const pSector = cdStreamer.peekOrReadSectorType(isValidVideoSector);

        if (!pSector)
            return false;

        // Read the frame header for this sector and verify that it is valid
        FrameSectorHeader hdr;
        getFrameSectorHeader(*pSector, hdr);

        // Verify this sector's frame header is as we expect:
        const bool bValidSectorHeader = (
            hdr.isMdecHeaderType() && 
            (hdr.chunkNum == chunkNum) &&   // Expect the frame sectors/chunks to be in sequential order!
            (hdr.numChunks == mFirstSecHdr.numChunks) &&
            (hdr.frameNum == mFirstSecHdr.frameNum) &&
            (hdr.numDemuxedBytes == mFirstSecHdr.numDemuxedBytes) &&
            (hdr.frameW == mFirstSecHdr.frameW) &&
            (hdr.frameH == mFirstSecHdr.frameH) &&
            (hdr.numCodeChunks == mFirstSecHdr.numCodeChunks) &&
            (hdr.quantizationScale == mFirstSecHdr.quantizationScale) &&
            (hdr.mdecVersion == mFirstSecHdr.mdecVersion)
        );

        if (!bValidSectorHeader)
            return false;

        // Buffer the data and mark the sector consumed
        bufferFrameData(*pSector);
        cdStreamer.freeSector(*pSector);
    }

    return true;    // All good if we got to here
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decodes the 16x16 pixel blocks that make up the the movie frame and saves them to the pixel buffer
//------------------------------------------------------------------------------------------------------------------------------------------
bool Frame::decodeMacroBlocks() noexcept {
    // Ensure we have enough room in the buffer for the decoded frame
    ensurePixelBufferCapacity((uint32_t) mFirstSecHdr.frameW * mFirstSecHdr.frameH);

    // There must be more than 8-bytes in the stream (see below)
    if (mDemuxedDataSize <= 8)
        return false;

    // Create a bitstream for the demuxed frame data that we will use to decode.
    // Note that we skip the first 8 bytes of the stream since it contains info that is redundant or not used.
    // 
    // The first 8 bytes contain:
    //  uint16_t    The number of 32-byte chunks of MDEC codes that would need to be sent to the MDEC
    //              chip to decode the frame. This is not needed by this decoder.
    //  uint16_t    Should be '0x3800'.
    //  uint16_t    The quantization scale of the frame - duplicated from the sector headers.
    //  uint16_t    The codec version for the frame - duplicated from the sector headers.
    //
    MBlockBitStream frameDataStream;
    frameDataStream.open((const uint16_t*)(mpDemuxedData + 8), (mDemuxedDataSize - 8) / sizeof(uint16_t));

    // The macro blocks are arranged in a column major order, decode in that fashion.
    // If the frame size is not an even multiple of 16 then the extra pixels are simply padding that are ignored.
    const uint32_t blocksW = (mFirstSecHdr.frameW + 15u) / 16u;
    const uint32_t blocksH = (mFirstSecHdr.frameH + 15u) / 16u;

    for (uint32_t bx = 0; bx < blocksW; ++bx) {
        for (uint32_t by = 0; by < blocksH; ++by) {
            // Decode this block of pixels and abort if failed
            uint32_t blockPixels[16][16];

            if (!MacroBlockDecoder::decode(frameDataStream, mFirstSecHdr.quantizationScale, blockPixels))
                return false;

            // Copy the pixels to the pixel buffer, the ones that are in range at least
            const uint32_t dstStartX = bx * 16u;
            const uint32_t dstStartY = by * 16u;
            const uint32_t dstEndX = std::min(dstStartX + 16u, (uint32_t) mFirstSecHdr.frameW);
            const uint32_t dstEndY = std::min(dstStartY + 16u, (uint32_t) mFirstSecHdr.frameH);
            const uint32_t copyRectW = dstEndX - dstStartX;
            const uint32_t copyRectH = dstEndY - dstStartY;

            for (uint32_t y = 0; y < copyRectH; ++y) {
                for (uint32_t x = 0; x < copyRectW; ++x) {
                    const uint32_t dstX = dstStartX + x;
                    const uint32_t dstY = dstStartY + y;
                    mpPixelBuffer[mFirstSecHdr.frameW * dstY + dstX] = blockPixels[y][x];
                }
            }
        }
    }

    return true;
}

END_NAMESPACE(movie)
