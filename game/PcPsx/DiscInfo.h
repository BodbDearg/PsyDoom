#pragma once

#include <cstdint>
#include <string>
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a physical (2,352 byte) sector location in a CD-ROM.
// The sector is expressed in terms of minutes, seconds and frames within a second (MSF format).
//------------------------------------------------------------------------------------------------------------------------------------------
struct DiscPos {
    // How many 2,352 byte sectors there are in a second and minute of CD audio
    static constexpr int32_t FRAMES_PER_SEC = 75;
    static constexpr int32_t FRAMES_PER_MIN = FRAMES_PER_SEC * 60;

    int32_t min;
    int32_t sec;
    int32_t frame;

    // Convert the disc position to a logical block address (physical 2,352 byte sector index)
    int32_t toLba() const noexcept {
        return (min * FRAMES_PER_MIN) + (sec * FRAMES_PER_SEC) + frame;
    }

    // Construct the disc position from a logical block address (physical 2,352 byte sector index)
    static DiscPos fromLba(const int32_t lba) noexcept {
        const int32_t min = (lba / FRAMES_PER_MIN);
        const int32_t sec = (lba % FRAMES_PER_MIN) / FRAMES_PER_SEC;
        const int32_t frame = lba % FRAMES_PER_MIN;
        return DiscPos{ min, sec, frame };
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Info for a track in a disc
//------------------------------------------------------------------------------------------------------------------------------------------
struct DiscTrack {
    std::string     sourceFilePath;         // Which file the track data comes from
    int32_t         sourceFileTotalSize;    // The total size of the file containing the track data
    int32_t         trackNum;               // The track number
    int32_t         fileOffset;             // Where the track is located in the file
    int32_t         blockSize;              // Size of each block/sector in the track
    int32_t         blockCount;             // The number of blocks in the track
    int32_t         trackPhysicalSize;      // Block count x block size
    int32_t         trackPayloadSize;       // Block count x block payload size
    int32_t         blockPayloadOffset;     // Where the actual data is located in each block/sector
    int32_t         blockPayloadSize;       // The size of the actual data in each block/sector
    bool            bIsData;                // Audio or data track?
    int32_t         index0;                 // Raw CD-ROM sector (2,352 byte) where the pre-gap for the track starts, as read from the .cue file
    int32_t         index1;                 // Raw CD-ROM sector (2,352 byte) where the actual track data starts, as read from the .cue file
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Info for all tracks in a disc
//------------------------------------------------------------------------------------------------------------------------------------------
struct DiscInfo {
    std::vector<DiscTrack> tracks;

    DiscTrack* getTrack(int32_t trackNum) noexcept;
    const DiscTrack* getTrack(int32_t trackNum) const noexcept;

    bool parseFromCueStr(const char* const str, const char* const cueBasePath, std::string& errorMsg) noexcept;
    bool parseFromCueFile(const char* const filePath, std::string& errorMsg) noexcept;
};
