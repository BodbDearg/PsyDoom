//------------------------------------------------------------------------------------------------------------------------------------------
// MapInfo defaults for '[GEC] Master Edition tools: single map test disc (Doom & Final Doom formats)'
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapInfo_Defaults_GEC_ME_TestMap.h"

#include "MapInfo.h"
#include "MapInfo_Defaults.h"
#include "MapInfo_Defaults_Doom.h"
#include "MapInfo_Defaults_FinalDoom.h"
#include "PsyQ/LIBSPU.h"

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a 'GameInfo' struct for '[GEC] Master Edition tools: single map test disc (Doom format)'
//------------------------------------------------------------------------------------------------------------------------------------------
void initGameInfo_GEC_ME_TestMap_Doom(GameInfo& gameInfo) noexcept {
    initGameInfo_Doom(gameInfo);
    gameInfo.numMaps = 1;
    gameInfo.numRegularMaps = 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a 'GameInfo' struct for '[GEC] Master Edition tools: single map test disc (Final Doom format)'
//------------------------------------------------------------------------------------------------------------------------------------------
void initGameInfo_GEC_ME_TestMap_FinalDoom(GameInfo& gameInfo) noexcept {
    initGameInfo_FinalDoom(gameInfo);
    gameInfo.numMaps = 1;
    gameInfo.numRegularMaps = 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default episodes for '[GEC] Master Edition tools: single map test disc' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addEpisodes_GEC_ME_TestMap(std::vector<Episode>& episodes) noexcept {
    addEpisode(episodes, 1,  1,  "Test Map");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default clusters for '[GEC] Master Edition tools: single map test disc' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addClusters_GEC_ME_TestMap(std::vector<Cluster>& clusters) noexcept {
    Cluster& clus = clusters.emplace_back();
    clus.clusterNum = 1;
    clus.bSkipFinale = true;
    clus.bHideNextMapForFinale = true;
    clus.bEnableCast = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds the default maps for '[GEC] Master Edition tools: single map test disc' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addMaps_GEC_ME_TestMap(std::vector<Map>& maps) noexcept {
    addMap(maps, 1 , 1, "Test Map", 1, SPU_REV_MODE_SPACE, 0x0FFF);
}

END_NAMESPACE(MapInfo)
