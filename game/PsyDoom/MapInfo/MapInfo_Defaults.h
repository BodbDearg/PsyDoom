#pragma once

#include "Macros.h"

#include <cstdint>
#include <vector>

enum SpuReverbMode : int32_t;
struct String8;
struct String32;

BEGIN_NAMESPACE(MapInfo)

struct Cluster;
struct CreditsPage;
struct Episode;
struct GameInfo;
struct Map;
struct MusicTrack;

void addEpisode(
    std::vector<Episode>& episodes,
    const int32_t episodeNum,
    const int32_t startMap,
    const String32& name
) noexcept;

void addMap(
    std::vector<Map>& maps,
    const int32_t mapNum,
    const int32_t cluster,
    const String32& name,
    const int32_t musicTrack,
    const SpuReverbMode reverbMode,
    const int16_t reverbDepth
) noexcept;

void addCreditsPage(
    std::vector<CreditsPage>& credits,
    const String8& bgPic,
    const String8& fgPic,
    const uint8_t bgPal,
    const uint8_t fgPal,
    const int16_t fgXPos,
    const int16_t maxScroll
) noexcept;

void setMapInfoToDefaults(
    GameInfo& gameInfo,
    std::vector<Episode>& episodes,
    std::vector<Cluster>& clusters,
    std::vector<Map>& maps,
    std::vector<CreditsPage>& credits,
    std::vector<MusicTrack>& musicTracks
) noexcept;

END_NAMESPACE(MapInfo)
