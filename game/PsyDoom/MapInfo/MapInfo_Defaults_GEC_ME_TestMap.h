#pragma once

#include "Macros.h"

#include <vector>

BEGIN_NAMESPACE(MapInfo)

struct Cluster;
struct CreditsPage;
struct Episode;
struct GameInfo;
struct Map;

void initGameInfo_GEC_ME_TestMap_Doom(GameInfo& gameInfo) noexcept;
void initGameInfo_GEC_ME_TestMap_FinalDoom(GameInfo& gameInfo) noexcept;
void addEpisodes_GEC_ME_TestMap(std::vector<Episode>& episodes) noexcept;
void addClusters_GEC_ME_TestMap(std::vector<Cluster>& clusters) noexcept;
void addMaps_GEC_ME_TestMap(std::vector<Map>& maps) noexcept;
void addCredits_GEC_ME_TestMap_Doom(std::vector<CreditsPage>& credits) noexcept;
void addCredits_GEC_ME_TestMap_FinalDoom(std::vector<CreditsPage>& credits) noexcept;

END_NAMESPACE(MapInfo)
