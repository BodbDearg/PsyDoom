#pragma once

#include "Macros.h"

#include <vector>

BEGIN_NAMESPACE(MapInfo)

struct Cluster;
struct Episode;
struct GameInfo;
struct Map;
struct MusicTrack;

void setMapInfoToDefaults(
    GameInfo& gameInfo,
    std::vector<Episode>& episodes,
    std::vector<Cluster>& clusters,
    std::vector<Map>& maps,
    std::vector<MusicTrack>& musicTracks
) noexcept;

END_NAMESPACE(MapInfo)
