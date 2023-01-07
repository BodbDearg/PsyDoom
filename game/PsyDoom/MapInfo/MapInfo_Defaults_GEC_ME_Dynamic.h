#pragma once

#include "Macros.h"

#include <vector>

BEGIN_NAMESPACE(MapInfo)

struct Cluster;
struct Episode;
struct GameInfo;
struct Map;

void loadDefaults_GEC_ME_Dynamic() noexcept;
void freeDefaults_GEC_ME_Dynamic() noexcept;

void initGameInfo_GEC_ME_Dynamic(GameInfo& gameInfo) noexcept;
void addEpisodes_GEC_ME_Dynamic(std::vector<Episode>& episodes) noexcept;
void addClusters_GEC_ME_Dynamic(std::vector<Cluster>& clusters) noexcept;
void addMaps_GEC_ME_Dynamic(std::vector<Map>& maps) noexcept;

END_NAMESPACE(MapInfo)
