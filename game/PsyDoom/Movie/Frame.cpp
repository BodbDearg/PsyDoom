#include "Frame.h"

#include "CDXAFileStreamer.h"
#include "Endian.h"
#include "FatalErrors.h"

BEGIN_NAMESPACE(movie)

// In an MDEC video sector there's just 2048 bytes of data, and 32 of that is taken up by the header
constexpr uint32_t VIDEO_DATA_BYTES_PER_SECTOR = 2048 - sizeof(FrameSectorHeader);

//------------------------------------------------------------------------------------------------------------------------------------------
// Endian correction: swaps the bytes in the frame header
//------------------------------------------------------------------------------------------------------------------------------------------
void FrameSectorHeader::byteSwap() noexcept {
    status = Endian::byteSwap(status);
    type = Endian::byteSwap(type);
    chunkNum = Endian::byteSwap(chunkNum);
    numChunks = Endian::byteSwap(numChunks);
    frameNum = Endian::byteSwap(frameNum);
    numDemuxedBytes = Endian::byteSwap(numDemuxedBytes);
    frameW = Endian::byteSwap(frameW);
    frameH = Endian::byteSwap(frameH);
    numCodeChunks = Endian::byteSwap(numCodeChunks);
    _unused1 = Endian::byteSwap(_unused1);
    quantizationScale = Endian::byteSwap(quantizationScale);
    mdecVersion = Endian::byteSwap(mdecVersion);
    _unused2 = Endian::byteSwap(_unused2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the frame with no data
//------------------------------------------------------------------------------------------------------------------------------------------
Frame::Frame() noexcept
    : mFirstSecHdr()
    , mpDemuxedData(nullptr)
    , mDemuxedDataSize(0)
    , mDemuxedDataCapacity(0)
{
    ensureDemuxedDataBufferCapacity(sizeof(VIDEO_DATA_BYTES_PER_SECTOR) * 8);       // Should be enough for all frames...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does cleanup for the frame
//------------------------------------------------------------------------------------------------------------------------------------------
Frame::~Frame() noexcept {
    clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears the frame data
//------------------------------------------------------------------------------------------------------------------------------------------
void Frame::clear() noexcept {
    mFirstSecHdr = {};

    if (mpDemuxedData) {
        std::free(mpDemuxedData);
        mpDemuxedData = nullptr;
    }

    mDemuxedDataSize = 0;
    mDemuxedDataCapacity = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to read and decode a frame of video.
// Returns false if that is not possible due to the end of the file being encountered, or some sort of error.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Frame::read(CDXAFileStreamer& cdStreamer, const uint8_t channelNum) noexcept {
    clear();
    const bool bSuccess = demuxFrame(cdStreamer, channelNum);   // TODO: decode the frame also!

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

END_NAMESPACE(movie)
