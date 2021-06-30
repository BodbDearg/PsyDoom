#pragma once

#include "Macros.h"

#include <cstdint>

struct DiscInfo;
struct DiscTrack;

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides access to the data in CD image
//------------------------------------------------------------------------------------------------------------------------------------------
class DiscReader {
public:
    DiscReader(DiscInfo& discInfo) noexcept;
    ~DiscReader() noexcept;

    int32_t getTrackNum() const noexcept;
    bool setTrackNum(int32_t trackNum) noexcept;

    bool isTrackOpen() noexcept;
    void closeTrack() noexcept;
    inline const DiscTrack* getOpenTrack() const { return mpCurTrack; }

    bool trackSeekAbs(const int32_t offsetAbs) noexcept;
    bool trackSeekRel(const int32_t offsetRel) noexcept;
    bool read(void* const pBuffer, const int32_t numBytes) noexcept;
    int32_t tell() const noexcept;

private:
    int32_t dataOffsetToPhysical(const int32_t dataOffset) const noexcept;

    DiscInfo&           mDiscInfo;      // Information for the disc being read from
    const DiscTrack*    mpCurTrack;     // Pointer to the current track open for the disc reader
    int32_t             mCurTrackIdx;   // Current track index in the disc that is open for reading or '-1' if none
    int32_t             mCurOffset;     // Current byte offset in the actual track data we are at (NOT physical offset in the file)
    void*               mpOpenFile;     // Handle to the open file for the current track
};
