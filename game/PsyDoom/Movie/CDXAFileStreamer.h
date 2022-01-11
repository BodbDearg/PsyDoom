#pragma once

#include "Macros.h"

#include <memory>
#include <vector>

class FileInputStream;
struct DiscInfo;
struct IsoFileSys;

BEGIN_NAMESPACE(movie)

//------------------------------------------------------------------------------------------------------------------------------------------
// Header info for CD-XA sectors on a CD-ROM.
// For more info on this see: https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
struct CDXAHeader {
    // Submode flags
    static constexpr uint8_t SMF_END_OF_AUDIO   = 0x01;     // Sector is the end of audio
    static constexpr uint8_t SMF_VIDEO          = 0x02;     // Sector is a video sector
    static constexpr uint8_t SMF_AUDIO          = 0x04;     // Sector is an audio sector
    static constexpr uint8_t SMF_DATA           = 0x08;     // Sector is a data sector
    static constexpr uint8_t SMF_TRIGGER        = 0x10;     // Meaning unclear (don't need it anyway)
    static constexpr uint8_t SMF_FORM2          = 0x20;     // If unset sector is 'Form 1' (2048 data bytes), otherwise 'Form 2' (2324 data bytes)
    static constexpr uint8_t SMF_REAL_TIME      = 0x40;     // Normally always set for '.STR' files
    static constexpr uint8_t SMF_EOF            = 0x80;     // Sector marks the end of the file

    uint8_t     interleavedFile;    // '1' if the file is interleaved, '0' if not
    uint8_t     channelNum;         // What audio or video channel this sector is for
    uint8_t     submodeFlags;       // Bit flags indicating what type of sector this is
    uint8_t     codingInfo;         // Sample rate and stereo mode info etc.

    inline bool isInterleaved() const noexcept { return (interleavedFile != 0); }
    inline bool isEndOfAudio() const noexcept { return (submodeFlags & SMF_END_OF_AUDIO); }
    inline bool isVideoSector() const noexcept { return (submodeFlags & SMF_VIDEO); }
    inline bool isAudioSector() const noexcept { return (submodeFlags & SMF_AUDIO); }
    inline bool isDataSector() const noexcept { return (submodeFlags & SMF_DATA); }

    inline uint32_t getNumDataBytes() const noexcept {
        return (submodeFlags & SMF_FORM2) ? 2324 : 2048;
    }

    inline bool isStereo() const noexcept {
        return (codingInfo & 0x3) ? true : false;       // Stored in bits 0..1: should be '0' for mono and '1' for stereo
    }

    inline uint32_t getSampleRate() const noexcept {
        return (codingInfo & 0xC) ? 18900 : 37800;      // Stored in bits 2..3: should be '0' for 37.8 kHz and '1' for 18.9 kHz
    }

    inline uint32_t getBitsPerSample() const noexcept {
        return (codingInfo & 0x30) ? 8 : 4;             // Stored in bits 4..5: should be '0' for 4-bits, '1' for 8-bits
    }
};

static_assert(sizeof(CDXAHeader) == 4);

//------------------------------------------------------------------------------------------------------------------------------------------
// A raw 2352 byte sector on the CD-ROM with CD-XA header info. The mdec sector streamer deals in terms of raw sectors
// because it needs to be able to determine whether individual sectors in a .STR (stream) file are for audio or video data.
//------------------------------------------------------------------------------------------------------------------------------------------
struct CDXASector {
    uint8_t         syncSequence[12];       // Used by CD-ROM readers (should be: 00 FF FF FF FF FF FF FF FF FF FF 00)
    uint8_t         minute;                 // Sector minute, in terms of audio units (Binary coded decimal, or 'BCD')
    uint8_t         second;                 // Sector second, in terms of audio units (BCD)
    uint8_t         frame;                  // Sector 'frame' within a second, in terms of audio units (BCD)
    uint8_t         mode;                   // Should be '2' for PSX games
    CDXAHeader      header;                 // Header for a sector in a STR (stream) file - CD-XA format
    CDXAHeader      header2;                // Duplicate of 'header' (possibly for error detection)
    uint8_t         data[2324];             // The sector data itself
    uint32_t        _unused;                // Unused/padding
};

static_assert(sizeof(CDXASector) == 2352);

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads and buffers raw CDXA sectors sequentially from a file on a disc.
// Allows sectors of interest (video, audio etc.) to be picked out while other sectors get buffered for later use.
//------------------------------------------------------------------------------------------------------------------------------------------
class CDXAFileStreamer {
public:
    CDXAFileStreamer() noexcept;
    ~CDXAFileStreamer() noexcept;

    bool open(
        const DiscInfo& disc,
        const IsoFileSys& fileSys,
        const char* const fileName,
        const uint32_t bufferSize       // How many sectors can be held in the buffer before some are dropped
    ) noexcept;

    bool isOpen() const noexcept;
    void close() noexcept;
    const CDXASector* readSector() noexcept;
    void freeSector(const CDXASector& sector) noexcept;

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Try to find a sector matching the specified condition without reading one.
    // Returns 'nullptr' if no such sector exists.
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <class PredT>
    const CDXASector* peekSectorType(const PredT& sectorPredicate) noexcept {
        for (uint32_t slotIdx : mUsedSectorBufferSlots) {
            const CDXASector& sector = mSectorBuffer[slotIdx];

            if (sectorPredicate(sector))
                return &sector;
        }

        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Tries to find an existing sector matching the given condition.
    // If none exists tries to read and buffer sectors until a match is found, potentially dropping old sectors if the buffer is full.
    // If reading fails to find a suitable sector or an error is encountered, then 'nullptr' is returned.
    // May also close up the stream if a read error occurs.
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <class PredT>
    const CDXASector* peekOrReadSectorType(const PredT& sectorPredicate) noexcept {
        const CDXASector* const pExistingSector = peekSectorType(sectorPredicate);

        if (pExistingSector)
            return pExistingSector;

        for (const CDXASector* pSector = readSector(); pSector; pSector = readSector()) {
            if (sectorPredicate(*pSector))
                return pSector;
        }

        return nullptr;
    }

private:
    CDXAFileStreamer(const CDXAFileStreamer& other) = delete;
    CDXAFileStreamer& operator = (const CDXAFileStreamer& other) = delete;

    CDXASector& allocBufferSector() noexcept;

    std::unique_ptr<FileInputStream>    mFile;                      // The file being streamed from
    uint32_t                            mCurSector;                 // Next sector to be read
    uint32_t                            mEndSector;                 // End sector in the file
    std::vector<CDXASector>             mSectorBuffer;              // Buffer of sectors that is potentially sparsely used
    std::vector<uint32_t>               mUsedSectorBufferSlots;     // Which sector buffer slots are in use (in FIFO order)
    std::vector<uint32_t>               mFreeSectorBufferSlots;     // Which sector buffer slots are free
};

END_NAMESPACE(movie)
