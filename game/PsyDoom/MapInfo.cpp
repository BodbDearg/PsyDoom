//------------------------------------------------------------------------------------------------------------------------------------------
// A module response for parsing and providing access to the new 'MAPINFO' lump which PsyDoom supports.
// PsyDoom's MAPINFO lump is in the 'new' ZDoom format and a subset of the fields (plus some new PSX specific ones) are supported.
// 
// For more on the new ZDoom format, see:
//      https://zdoom.org/wiki/MAPINFO
//      https://doomwiki.org/wiki/MAPINFO#ZDoom_.28new.29
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapInfo.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Base/w_wad.h"
#include "Game.h"
#include "MapInfo_Defaults.h"
#include "MapInfo_Parse.h"
#include "PsyQ/LIBSPU.h"

#include <algorithm>
#include <cstring>
#include <memory>

BEGIN_NAMESPACE(MapInfo)

// All of the main MAPINFO data structures
static std::vector<MusicTrack>  gMusicTracks;
static GameInfo                 gGameInfo;
static std::vector<Episode>     gEpisodes;
static std::vector<Cluster>     gClusters;
static std::vector<Map>         gMaps;

// Used to speed up repeated searches for the same 'Map' data structure.
// Cache which map was queried and where it is in the maps array, so we can do O(1) lookup for repeat queries.
static int32_t  gLastGetMap         = -1;
static int32_t  gLastGetMapIndex    = -1;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a cluster with default settings
//------------------------------------------------------------------------------------------------------------------------------------------
Cluster::Cluster() noexcept
    : castLcdFile(S_GetSoundLcdFileId(60))
    , pic("BACK")
    , picPal(Game::getTexPalette_BACK())
    , cdMusicA((int16_t) gCDTrackNum[cdmusic_finale_doom1_final_doom])
    , cdMusicB((int16_t) gCDTrackNum[cdmusic_credits_demo])
    , textX(0)
    , textY(15)
    , bSkipFinale(true)
    , bHideNextMapForFinale(false)
    , bEnableCast(false)
    , bNoCenterText(false)
    , bSmallFont(false)
    , text{}
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Zero initializes a game info struct
//------------------------------------------------------------------------------------------------------------------------------------------
GameInfo::GameInfo() noexcept
    : numMaps(0)
    , numRegularMaps(0)
    , bFinalDoomGameRules(false)
    , bDisableMultiplayer(false)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find a specified data structure in a list by an integer field
//------------------------------------------------------------------------------------------------------------------------------------------
template <class DataT, class ListT>
static const DataT* findStructByIntKey(const int32_t key, const ListT& list, int32_t DataT::* const keyField) noexcept {
    for (const DataT& obj : list) {
        if (obj.*keyField == key)
            return &obj;
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get an existing struct with the specfied integer key or create a new one
//------------------------------------------------------------------------------------------------------------------------------------------
template <class DataT, class ListT>
static DataT& getOrCreateStructWithIntKey(const int32_t key, ListT& list, int32_t DataT::* const keyField) noexcept {
    // Try to find an existing structure with this key
    for (DataT& obj : list) {
        if (obj.*keyField == key)
            return obj;
    }

    // If not found then create a new one and assign it the key
    DataT& obj = list.emplace_back();
    obj.*keyField = key;
    return obj;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a music track from the specicified value block
//------------------------------------------------------------------------------------------------------------------------------------------
static void readMusicTrack(const Block& block) noexcept {
    // Read and validate the track header
    block.ensureMinHeaderTokenCount(1);
    const int32_t trackNum = block.getRequiredHeaderInt(0);

    if ((trackNum < 1) || (trackNum > 1024)) {
        error(block, "MusicTrack: track number must be between 1 and 1024!");
    }

    // Read and validate track properties
    MusicTrack& track = getOrCreateStructWithIntKey(trackNum, gMusicTracks, &MusicTrack::trackNum);
    track.sequenceNum = block.getSingleIntValue("Sequence", track.sequenceNum);

    if ((track.sequenceNum < 0) || (track.sequenceNum > 16384)) {
        error(block, "MusicTrack: 'Sequence' must be specified and be between 0 and 16384!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads general game info from the specicified value block
//------------------------------------------------------------------------------------------------------------------------------------------
static void readGameInfo(const Block& block) noexcept {
    // Read and validate all properties
    GameInfo& gameInfo = gGameInfo;
    gameInfo.numMaps = block.getSingleIntValue("NumMaps", gameInfo.numMaps);
    gameInfo.numRegularMaps = block.getSingleIntValue("NumRegularMaps", gameInfo.numRegularMaps);
    gameInfo.bFinalDoomGameRules = (block.getSingleIntValue("FinalDoomGameRules", gameInfo.bFinalDoomGameRules) > 0);
    gameInfo.bDisableMultiplayer = (block.getSingleIntValue("DisableMultiplayer", gameInfo.bDisableMultiplayer) > 0);

    if ((gameInfo.numMaps < 1) || (gameInfo.numMaps > 255)) {
        error(block, "GameInfo: 'NumMaps' must be between 1 and 255!");
    }

    if ((gameInfo.numRegularMaps < 1) || (gameInfo.numRegularMaps > 255)) {
        error(block, "GameInfo: 'NumRegularMaps' must be between 1 and 255!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the 'clear episodes' command: just clears the episode list, does nothing else
//------------------------------------------------------------------------------------------------------------------------------------------
static void readClearEpisodes([[maybe_unused]] const Block& block) noexcept {
    gEpisodes.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads an episode from the specicified value block
//------------------------------------------------------------------------------------------------------------------------------------------
static void readEpisode(const Block& block) noexcept {
    // Read and validate the episode header
    block.ensureMinHeaderTokenCount(1);
    const int32_t episodeNum = block.getRequiredHeaderInt(0);

    if ((episodeNum < 1) || (episodeNum > 255)) {
        error(block, "Episode: episode number must be between 1 and 255!");
    }

    // Read and validate the episode properties
    Episode& episode = getOrCreateStructWithIntKey(episodeNum, gEpisodes, &Episode::episodeNum);
    episode.name = block.getSingleSmallStringValue("Name", episode.name);
    episode.startMap = block.getSingleIntValue("StartMap", episode.startMap);

    if ((episode.startMap < 1) || (episode.startMap > 255)) {
        error(block, "Episode: 'StartMap' must be specified and be between 1 and 255!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a cluster definition from the specicified value block
//------------------------------------------------------------------------------------------------------------------------------------------
static void readCluster(const Block& block) noexcept {
    // Read and validate the cluster header
    block.ensureMinHeaderTokenCount(1);
    const int32_t clusterNum = block.getRequiredHeaderInt(0);

    if ((clusterNum < 1) || (clusterNum > 255)) {
        error(block, "Cluster: cluster number must be between 1 and 255!");
    }

    // Read and validate all other map properties
    Cluster& cluster = getOrCreateStructWithIntKey(clusterNum, gClusters, &Cluster::clusterNum);
    cluster.castLcdFile = block.getSingleSmallStringValue("CastLcdFile", cluster.castLcdFile);
    cluster.pic = block.getSingleSmallStringValue("Pic", cluster.pic);
    cluster.picPal = block.getSingleIntValue("PicPal", cluster.picPal);
    cluster.cdMusicA = (int16_t) block.getSingleIntValue("CdMusicA", cluster.cdMusicA);
    cluster.cdMusicB = (int16_t) block.getSingleIntValue("CdMusicB", cluster.cdMusicB);
    cluster.textX = (int16_t) block.getSingleIntValue("TextX", cluster.textX);
    cluster.textY = (int16_t) block.getSingleIntValue("TextY", cluster.textY);
    cluster.bSkipFinale = (block.getSingleIntValue("SkipFinale", cluster.bSkipFinale) > 0);
    cluster.bHideNextMapForFinale = (block.getSingleIntValue("HideNextMapForFinale", cluster.bHideNextMapForFinale) > 0);
    cluster.bEnableCast = (block.getSingleIntValue("EnableCast", cluster.bEnableCast) > 0);
    cluster.bNoCenterText = (block.getSingleIntValue("NoCenterText", cluster.bNoCenterText) > 0);
    cluster.bSmallFont = (block.getSingleIntValue("SmallFont", cluster.bSmallFont) > 0);

    {
        const LinkedToken* const pTextValue = block.getValue("Text");
        const LinkedToken* pTextData = (pTextValue) ? pTextValue->pNextData : nullptr;

        if (pTextValue) {
            // If text is specified then it overwrites the text for the cluster
            for (String32& line : cluster.text) {
                line = {};
            }
        }

        for (uint32_t lineIdx = 0; (lineIdx < C_ARRAY_SIZE(Cluster::text)) && pTextData; ++lineIdx, pTextData = pTextData->pNextData) {
            const std::string_view text = pTextData->token.text();
            cluster.text[lineIdx].assign(text.data(), (uint32_t) text.length());
        }
    }

    if ((cluster.picPal < 0) || (cluster.picPal > 31)) {
        error(block, "Cluster: 'PicPal' must be between 0 and 31!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a map definition from the specicified value block
//------------------------------------------------------------------------------------------------------------------------------------------
static void readMap(const Block& block) noexcept {
    // Read and validate the map header
    block.ensureMinHeaderTokenCount(1);
    const int32_t mapNum = block.getRequiredHeaderInt(0);
    
    if ((mapNum < 1) || (mapNum > 255)) {
        error(block, "Map: map number must be between 1 and 255!");
    }

    Map& map = getOrCreateStructWithIntKey(mapNum, gMaps, &Map::mapNum);

    if (block.getHeaderTokenCount() > 1) {
        map.name = block.getRequiredHeaderSmallString<String32>(1);
    }

    // Read and validate all other map properties
    map.music = block.getSingleIntValue("Music", map.music);
    map.cluster = block.getSingleIntValue("Cluster", map.cluster);
    map.skyPaletteOverride = block.getSingleIntValue("SkyPal", map.skyPaletteOverride);
    map.bPlayCdMusic = (block.getSingleIntValue("PlayCdMusic", map.bPlayCdMusic) > 0);
    map.reverbMode = (SpuReverbMode) block.getSingleIntValue("ReverbMode", map.reverbMode);
    map.reverbDepthL = (int16_t) std::clamp<int32_t>(block.getSingleIntValue("ReverbDepthL", map.reverbDepthL), INT16_MIN, INT16_MAX);
    map.reverbDepthR = (int16_t) std::clamp<int32_t>(block.getSingleIntValue("ReverbDepthR", map.reverbDepthR), INT16_MIN, INT16_MAX);
    map.reverbDelay = (int16_t) std::clamp<int32_t>(block.getSingleIntValue("ReverbDelay", map.reverbDelay), 0, INT16_MAX);
    map.reverbFeedback = (int16_t) std::clamp<int32_t>(block.getSingleIntValue("ReverbFeedback", map.reverbFeedback), INT16_MIN, INT16_MAX);

    if ((map.music < 0) || (map.music > 1024)) {
        error(block, "Map: 'Music' must be specified and be between 0 and 1024!");
    }

    if ((map.cluster < 1) || (map.cluster > 255)) {
        error(block, "Map: 'Cluster' must be specified and be between 1 and 255!");
    }

    if ((map.skyPaletteOverride < -1) || (map.skyPaletteOverride > 32)) {
        error(block, "Map: 'SkyPaletteOverride' must be between -1 and 32!");
    }

    if ((map.reverbMode < SpuReverbMode::SPU_REV_MODE_OFF) || (map.reverbMode > SpuReverbMode::SPU_REV_MODE_PIPE)) {
        error(block, "Map: 'ReverbMode' must be between 0 and 9!");
    }

    // Allow a single 'ReverbDepth' value to set the depth for both channels, if present
    {
        const int32_t depth = block.getSingleIntValue("ReverbDepth", INT32_MAX);

        if (depth != INT32_MAX) {
            map.reverbDepthL = (int16_t) std::clamp<int32_t>(depth, INT16_MIN, INT16_MAX);
            map.reverbDepthR = (int16_t) std::clamp<int32_t>(depth, INT16_MIN, INT16_MAX);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the MAPINFO lump data from the main IWAD (if available) as a C string
//------------------------------------------------------------------------------------------------------------------------------------------
static std::unique_ptr<char[]> readMapInfoLumpAsCString() noexcept {
    const int32_t lumpNum = W_CheckNumForName("MAPINFO");

    if (lumpNum < 0)
        return {};

    const int32_t lumpSize = std::max(W_LumpLength(lumpNum), 0);
    std::unique_ptr<char[]> lumpCString(new char[lumpSize + 1]);
    W_ReadLump(lumpNum, lumpCString.get(), true);
    lumpCString[lumpSize] = 0;

    return lumpCString;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to read MAPINFO from the main IWAD, if available
//------------------------------------------------------------------------------------------------------------------------------------------
static void readMapInfoFromIWAD() noexcept {
    // Read the MAPINFO string (if it exists) and parse it if it's valid
    std::unique_ptr<char[]> mapInfoStr = readMapInfoLumpAsCString();

    if (!mapInfoStr)
        return;

    MapInfo mapInfo = parseMapInfo(mapInfoStr.get());

    // Define the list of block type readers
    struct BlockReader {
        const char* blockType;
        void (*ReadFunc)(const Block& block) noexcept;
    };

    constexpr BlockReader BLOCK_READERS[] = {
        { "ClearEpisodes",  readClearEpisodes   },
        { "Cluster",        readCluster         },
        { "Episode",        readEpisode         },
        { "GameInfo",       readGameInfo        },
        { "Map",            readMap             },
        { "MusicTrack",     readMusicTrack      },
    };

    // Read all supported block types, ignore unsupported ones
    for (const Block& block : mapInfo.blocks) {
        for (const BlockReader& reader : BLOCK_READERS) {
            if (block.pType->token.textEqualsIgnoreCase(reader.blockType)) {
                reader.ReadFunc(block);
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the MAPINFO lump, if present from one of the main IWAD files.
// Otherwise initializes MAPINFO to the appropriate settings for the current game (Doom or Final Doom).
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Set MAPINFO to the default values then read overrides from one of the IWADs
    setMapInfoToDefaults(gGameInfo, gEpisodes, gClusters, gMaps, gMusicTracks);
    readMapInfoFromIWAD();

    // Clear the cached search result for 'getMap'
    gLastGetMap = -1;
    gLastGetMapIndex = -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup of MAPINFO data and releasing resources
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gLastGetMapIndex = -1;
    gLastGetMap = -1;
    gMaps.clear();
    gClusters.clear();
    gEpisodes.clear();
    gMusicTracks.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find a 'MusicTrack' structure by track number or return all music tracks
//------------------------------------------------------------------------------------------------------------------------------------------
const MusicTrack* getMusicTrack(const int32_t trackNum) noexcept {
    return findStructByIntKey(trackNum, gMusicTracks, &MusicTrack::trackNum);
}

const std::vector<MusicTrack>& allMusicTracks() noexcept {
    return gMusicTracks;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get general game info
//------------------------------------------------------------------------------------------------------------------------------------------
const GameInfo& getGameInfo() noexcept {
    return gGameInfo;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find an 'Episode' structure by episode number or return all episodes
//------------------------------------------------------------------------------------------------------------------------------------------
const Episode* getEpisode(const int32_t episodeNum) noexcept {
    return findStructByIntKey(episodeNum, gEpisodes, &Episode::episodeNum);
}

const std::vector<Episode>& allEpisodes() noexcept {
    return gEpisodes;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the maximum episode number that can be played
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getNumEpisodes() noexcept {
    // Determine the maximum episode number in the episode list
    int32_t maxEpisodeNum = 1;

    for (const Episode& episode : gEpisodes) {
        maxEpisodeNum = std::max(maxEpisodeNum, episode.episodeNum);
    }

    return maxEpisodeNum;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find a 'Cluster' structure by cluster number or return all clusters
//------------------------------------------------------------------------------------------------------------------------------------------
const Cluster* getCluster(const int32_t clusterNum) noexcept {
    return findStructByIntKey(clusterNum, gClusters, &Cluster::clusterNum);
}

const std::vector<Cluster>& allClusters() noexcept {
    return gClusters;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find a 'Map' structure by map number or return all maps
//------------------------------------------------------------------------------------------------------------------------------------------
const Map* getMap(const int32_t mapNum) noexcept {
    // Did we search for this last? If so then re-use the search result:
    if (gLastGetMap == mapNum)
        return (gLastGetMapIndex >= 0) ? &gMaps[gLastGetMapIndex] : nullptr;

    // Querying a new map: do a linear search through the list of maps and cache the result for next time
    const Map* const pMap = findStructByIntKey(mapNum, gMaps, &Map::mapNum);

    gLastGetMap = mapNum;
    gLastGetMapIndex = (pMap) ? (int32_t)(pMap - gMaps.data()) : -1;

    return pMap;
}

const std::vector<Map>& allMaps() noexcept {
    return gMaps;
}

END_NAMESPACE(MapInfo)
