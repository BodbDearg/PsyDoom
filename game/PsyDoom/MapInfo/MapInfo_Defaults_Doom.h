#pragma once

#include "Macros.h"

#include <vector>

BEGIN_NAMESPACE(MapInfo)

struct Cluster;
struct Episode;
struct GameInfo;
struct Map;

void initGameInfo_Doom(GameInfo& gameInfo) noexcept;
void addEpisodes_Doom(std::vector<Episode>& episodes) noexcept;
void addClusters_Doom(std::vector<Cluster>& clusters) noexcept;
void addMaps_Doom(std::vector<Map>& maps) noexcept;

END_NAMESPACE(MapInfo)
