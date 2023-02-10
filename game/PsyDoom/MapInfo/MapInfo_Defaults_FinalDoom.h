#pragma once

#include "Macros.h"

#include <vector>

BEGIN_NAMESPACE(MapInfo)

struct Cluster;
struct CreditsPage;
struct Episode;
struct GameInfo;
struct Map;

void initGameInfo_FinalDoom(GameInfo& gameInfo) noexcept;
void addEpisodes_FinalDoom(std::vector<Episode>& episodes) noexcept;
void addClusters_FinalDoom(std::vector<Cluster>& clusters) noexcept;
void addMaps_FinalDoom(std::vector<Map>& maps) noexcept;
void addCredits_FinalDoom(std::vector<CreditsPage>& credits) noexcept;

END_NAMESPACE(MapInfo)
