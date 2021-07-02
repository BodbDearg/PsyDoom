#pragma once

#include "Macros.h"

#include <cstdint>
#include <vector>

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a music track played the WESS sequencer system
//------------------------------------------------------------------------------------------------------------------------------------------
struct MusicTrack {
    int32_t trackNum;       // Music track number. Also determines which LCD file is used (MUSLEV<trackNum>.LCD).
    int32_t sequenceNum;    // Which sequence number from the module file to play for this music track.

    MusicTrack(const int32_t trackNum, const int32_t sequenceNum = -1) noexcept
        : trackNum(trackNum)
        , sequenceNum(sequenceNum) 
    {
    }
};

void init() noexcept;
void shutdown() noexcept;
const MusicTrack* getMusicTrack(const int32_t trackNum) noexcept;
const std::vector<MusicTrack>& allMusicTracks() noexcept;

END_NAMESPACE(MapInfo)
