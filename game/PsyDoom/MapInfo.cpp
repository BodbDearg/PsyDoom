//------------------------------------------------------------------------------------------------------------------------------------------
// A module response for parsing and providing access to the new 'MAPINFO' lump which PsyDoom supports.
// PsyDoom's MAPINFO lump is in the 'new' ZDoom format and a subset of the fields (plus some new PSX specific ones) are supported.
// 
// For more on the new ZDoom format, see:
//      https://zdoom.org/wiki/MAPINFO
//      https://doomwiki.org/wiki/MAPINFO#ZDoom_.28new.29
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapInfo.h"

#include "Doom/Base/w_wad.h"
#include "Game.h"
#include "MapInfo_Parse.h"

#include <algorithm>
#include <memory>

BEGIN_NAMESPACE(MapInfo)

static std::vector<MusicTrack>  gMusicTracks;
static std::vector<Episode>     gEpisodes;

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
    block.ensureHeaderTokenCount(1);
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
// Reads the 'clear episodes' command: just clears the episode list, does nothing else
//------------------------------------------------------------------------------------------------------------------------------------------
static void readClearEpisodes(const Block& block) noexcept {
    gEpisodes.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads an episode from the specicified value block
//------------------------------------------------------------------------------------------------------------------------------------------
static void readEpisode(const Block& block) noexcept {
    // Read and validate the episode header
    block.ensureHeaderTokenCount(1);
    const int32_t episodeNum = block.getRequiredHeaderInt(0);

    if ((episodeNum < 1) || (episodeNum > 255)) {
        error(block, "Episode: episode number must be between 1 and 255!");
    }

    // Read and validate the episode properties
    Episode& episode = getOrCreateStructWithIntKey(episodeNum, gEpisodes, &Episode::episodeNum);
    episode.name = block.getSingleString32Value("Name", episode.name);
    episode.startMap = block.getSingleIntValue("StartMap", episode.startMap);

    if ((episode.startMap < 1) || (episode.startMap > 255)) {
        error(block, "Episode: 'StartMap' must be specified and be between 1 and 255!");
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
        { "ClearEpisodes", readClearEpisodes},
        { "Episode", readEpisode },
        { "MusicTrack", readMusicTrack },
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
// Initializes MAPINFO to the defaults appropriate for the current game ('Doom' or 'Final Doom')
//------------------------------------------------------------------------------------------------------------------------------------------
static void setMapInfoToDefaults() noexcept {
    // Default music tracks, including track '0' (the invalid/null track).
    // There are 30 of these in Final Doom (a superset of Doom's music) and they begin at sequence '90' for track '1'.
    gMusicTracks.clear();
    gMusicTracks.push_back(MusicTrack{ 0, 0 });

    for (int32_t trackNum = 1; trackNum <= 30; ++trackNum) {
        gMusicTracks.push_back(MusicTrack{ trackNum, 89 + trackNum });
    }

    // Default episodes
    gEpisodes.clear();

    if (Game::isFinalDoom()) {
        gEpisodes.push_back(Episode{ 1, 1,  "Master Levels" });
        gEpisodes.push_back(Episode{ 2, 14, "TNT" });
        gEpisodes.push_back(Episode{ 3, 25, "Plutonia" });
    } else {
        gEpisodes.push_back(Episode{ 1, 1,  "Ultimate Doom" });
        gEpisodes.push_back(Episode{ 2, 31, "Doom II" });
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the MAPINFO lump, if present from one of the main IWAD files.
// Otherwise initializes MAPINFO to the appropriate settings for the current game (Doom or Final Doom).
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    setMapInfoToDefaults();
    readMapInfoFromIWAD();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup of MAPINFO data and releasing resources
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
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

END_NAMESPACE(MapInfo)
