#pragma once

#include "Macros.h"

#include <cstdint>
#include <vector>

BEGIN_NAMESPACE(movie)

class CDXAFileStreamer;
struct CDXASector;

//------------------------------------------------------------------------------------------------------------------------------------------
// 32-byte header for a CD sector containing video data in a STR file.
// For more info on this see: https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
struct FrameSectorHeader {
    uint16_t    status;                 // Unknown flags (don't use this)
    uint16_t    type;                   // 0x8001 for an MDEC sector header
    uint16_t    chunkNum;               // Which multiplexed chunk/sector number this is for the same frame
    uint16_t    numChunks;              // How many chunks in this frame
    uint32_t    frameNum;               // Which frame this is
    uint32_t    numDemuxedBytes;        // How many bytes of frame data there is in the demultiplexed frame (when the data in all sectors is stitched together)
    uint16_t    frameW;                 // Width of the frame in pixels
    uint16_t    frameH;                 // Height of the frame in pixels
    uint16_t    numCodeChunks;          // How many 32-byte blocks it takes to hold all of the uncompressed MDEC code chunks
    uint16_t    _unused1;               // Always '0x3800'
    int16_t     quantizationScale;      // The quantization scale for the frame (MDEC decompression var)
    uint16_t    mdecVersion;            // What decompression method is used, we only support version '2'
    uint32_t    _unused2;               // Always '0'

    void byteSwap() noexcept;
    bool isMdecHeaderType() const noexcept { return (type == 0x8001); }
};

static_assert(sizeof(FrameSectorHeader) == 32);

//------------------------------------------------------------------------------------------------------------------------------------------
// A class responsible for decoding an MDEC video frame from a STR file and storing it
//------------------------------------------------------------------------------------------------------------------------------------------
class Frame {
public:
    inline uint16_t getWidth() const noexcept { return mFirstSecHdr.frameW; }
    inline uint16_t getHeight() const noexcept { return mFirstSecHdr.frameH; }
    inline const uint16_t* getPixels() const noexcept { return mpPixelBuffer; }

    Frame() noexcept;
    ~Frame() noexcept;
    void clear() noexcept;
    bool read(CDXAFileStreamer& cdStreamer, const uint8_t channelNum) noexcept;

private:
    Frame(const Frame& other) = delete;
    Frame& operator = (const Frame& other) = delete;

    void ensureDemuxedDataBufferCapacity(const uint32_t capacity) noexcept;
    void ensurePixelBufferCapacity(const uint32_t capacity) noexcept;
    void getFrameSectorHeader(const CDXASector& sector, FrameSectorHeader& hdrOut) noexcept;
    void bufferFrameData(const CDXASector& sector) noexcept;
    bool demuxFrame(CDXAFileStreamer& cdStreamer, const uint8_t channelNum) noexcept;
    bool decodeMacroBlocks() noexcept;

    FrameSectorHeader   mFirstSecHdr;           // Holds the header for the first sector in the frame, subsequent sectors largely duplicate this info
    std::byte*          mpDemuxedData;          // Buffer holding the de-multiplexed compressed data for the frame
    uint32_t            mDemuxedDataSize;       // How much of the demuxed frame data buffer is occupied
    uint32_t            mDemuxedDataCapacity;   // Size of the demuxed frame data buffer
    uint16_t*           mpPixelBuffer;          // Pixel buffer for holding decoded frame data
    uint32_t            mPixelBufferCapacity;   // The number of pixels that the pixel buffer can hold
};

END_NAMESPACE(movie)
