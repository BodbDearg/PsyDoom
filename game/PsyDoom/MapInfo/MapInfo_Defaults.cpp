#include "MapInfo_Defaults.h"

#include "GecMapInfo.h"
#include "MapInfo.h"
#include "MapInfo_Defaults_Doom.h"
#include "MapInfo_Defaults_FinalDoom.h"
#include "MapInfo_Defaults_GEC_ME_Beta3.h"
#include "MapInfo_Defaults_GEC_ME_TestMap.h"
#include "PsyDoom/Game.h"

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: defines an episode and adds it to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addEpisode(
    std::vector<Episode>& episodes,
    const int32_t episodeNum,
    const int32_t startMap,
    const String32& name
) noexcept {
    Episode& episode = episodes.emplace_back();
    episode.episodeNum = episodeNum;
    episode.startMap = startMap;
    episode.name = name;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: defines a built-in map and adds it to the given list.
// Defaults fields that aren't relevant to standard retail maps and assumes the map doesn't exist already.
//------------------------------------------------------------------------------------------------------------------------------------------
void addMap(
    std::vector<Map>& maps,
    const int32_t mapNum,
    const int32_t cluster,
    const String32& name,
    const int32_t musicTrack,
    const SpuReverbMode reverbMode,
    const int16_t reverbDepth
) noexcept {
    Map& map = maps.emplace_back();
    map.mapNum = mapNum;
    map.music = musicTrack;
    map.name = name;
    map.cluster = cluster;
    map.skyPaletteOverride = -1;
    map.reverbMode = reverbMode;
    map.reverbDepthL = reverbDepth;
    map.reverbDepthR = reverbDepth;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: defines a music track and adds it to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addMusicTrack(std::vector<MusicTrack>& musicTracks, const int32_t trackNum, const int32_t sequenceNum) noexcept {
    MusicTrack& musicTrack = musicTracks.emplace_back();
    musicTrack.trackNum = trackNum;
    musicTrack.sequenceNum = sequenceNum;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the 'GameInfo' struct for the current game to the default set of values
//------------------------------------------------------------------------------------------------------------------------------------------
static void initGameInfo(GameInfo& gameInfo) noexcept {
    gameInfo = GameInfo();

    switch (Game::gGameType) {
        case GameType::Doom:                        initGameInfo_Doom(gameInfo);                        break;
        case GameType::FinalDoom:                   initGameInfo_FinalDoom(gameInfo);                   break;
        case GameType::GEC_ME_Beta3:                initGameInfo_GEC_ME_Beta3(gameInfo);                break;
        case GameType::GEC_ME_TestMap_Doom:         initGameInfo_GEC_ME_TestMap_Doom(gameInfo);         break;
        case GameType::GEC_ME_TestMap_FinalDoom:    initGameInfo_GEC_ME_TestMap_FinalDoom(gameInfo);    break;

        case GameType::GEC_ME_Beta4:
            gameInfo = GecMapInfo::getGameInfo();
            break;

        default:
            FatalErrors::raise("MapInfo::initGameInfo(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of episodes for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initEpisodes(std::vector<Episode>& episodes) noexcept {
    episodes.clear();

    switch (Game::gGameType) {
        case GameType::Doom:                        addEpisodes_Doom(episodes);             break;
        case GameType::FinalDoom:                   addEpisodes_FinalDoom(episodes);        break;
        case GameType::GEC_ME_Beta3:                addEpisodes_GEC_ME_Beta3(episodes);     break;
        case GameType::GEC_ME_TestMap_Doom:         addEpisodes_GEC_ME_TestMap(episodes);   break;
        case GameType::GEC_ME_TestMap_FinalDoom:    addEpisodes_GEC_ME_TestMap(episodes);   break;

        case GameType::GEC_ME_Beta4:
            episodes = GecMapInfo::allEpisodes();
            break;

        default:
            FatalErrors::raise("MapInfo::initEpisodes(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of clusters for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initClusters(std::vector<Cluster>& clusters) noexcept {
    clusters.clear();

    switch (Game::gGameType) {
        case GameType::Doom:                        addClusters_Doom(clusters);             break;
        case GameType::FinalDoom:                   addClusters_FinalDoom(clusters);        break;
        case GameType::GEC_ME_Beta3:                addClusters_GEC_ME_Beta3(clusters);     break;
        case GameType::GEC_ME_TestMap_Doom:         addClusters_GEC_ME_TestMap(clusters);   break;
        case GameType::GEC_ME_TestMap_FinalDoom:    addClusters_GEC_ME_TestMap(clusters);   break;

        case GameType::GEC_ME_Beta4:
            clusters = GecMapInfo::allClusters();
            break;

        default:
            FatalErrors::raise("MapInfo::initClusters(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of maps for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initMaps(std::vector<Map>& maps) noexcept {
    maps.clear();

    switch (Game::gGameType) {
        case GameType::Doom:                        addMaps_Doom(maps);             break;
        case GameType::FinalDoom:                   addMaps_FinalDoom(maps);        break;
        case GameType::GEC_ME_Beta3:                addMaps_GEC_ME_Beta3(maps);     break;
        case GameType::GEC_ME_TestMap_Doom:         addMaps_GEC_ME_TestMap(maps);   break;
        case GameType::GEC_ME_TestMap_FinalDoom:    addMaps_GEC_ME_TestMap(maps);   break;

        case GameType::GEC_ME_Beta4:
            maps = GecMapInfo::allMaps();
            break;

        default:
            FatalErrors::raise("MapInfo::initMaps(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of music tracks for the game to the default list
//------------------------------------------------------------------------------------------------------------------------------------------
static void initMusicTracks(std::vector<MusicTrack>& musicTracks) noexcept {
    // Default music tracks, including track '0' (the invalid/null track).
    // There are 30 of these in Final Doom (a superset of Doom's music) and they begin at sequence '90' for track '1'.
    musicTracks.clear();
    addMusicTrack(musicTracks, 0, 0);

    for (int32_t trackNum = 1; trackNum <= 30; ++trackNum) {
        addMusicTrack(musicTracks, trackNum, 89 + trackNum);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes MAPINFO to the defaults appropriate for the current game ('Doom' or 'Final Doom')
//------------------------------------------------------------------------------------------------------------------------------------------
void setMapInfoToDefaults(
    GameInfo& gameInfo,
    std::vector<Episode>& episodes,
    std::vector<Cluster>& clusters,
    std::vector<Map>& maps,
    std::vector<MusicTrack>& musicTracks
) noexcept {
    // Init the defaults
    initGameInfo(gameInfo);
    initEpisodes(episodes);
    initClusters(clusters);
    initMaps(maps);
    initMusicTracks(musicTracks);
}

END_NAMESPACE(MapInfo)
