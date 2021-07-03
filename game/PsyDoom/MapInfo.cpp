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
#include "PsyQ/LIBSPU.h"

#include <algorithm>
#include <memory>

BEGIN_NAMESPACE(MapInfo)

// All of the main MAPINFO data structures
static std::vector<MusicTrack>  gMusicTracks;
static std::vector<Episode>     gEpisodes;
static std::vector<Map>         gMaps;

// Used to speed up repeated searches for the same 'Map' data structure.
// Cache which map was queried and where it is in the maps array, so we can do O(1) lookup for repeat queries.
static int32_t  gLastGetMap         = -1;
static int32_t  gLastGetMapIndex    = -1;

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
// Helper: defines a built-in music track
//------------------------------------------------------------------------------------------------------------------------------------------
static void defineBuiltInMusicTrack(const int32_t trackNum, const int32_t sequenceNum) noexcept {
    MusicTrack& musicTrack = gMusicTracks.emplace_back();
    musicTrack.trackNum = trackNum;
    musicTrack.sequenceNum = sequenceNum;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: defines a built-in episode
//------------------------------------------------------------------------------------------------------------------------------------------
static void defineBuiltInEpisode(
    const int32_t episodeNum,
    const int32_t startMap,
    const String32& name
) noexcept {
    Episode& episode = gEpisodes.emplace_back();
    episode.episodeNum = episodeNum;
    episode.startMap = startMap;
    episode.name = name;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: defines a built-in map, defaulting fields that aren't relevant to standard retail maps.
// Assumes the map doesn't already exist.
//------------------------------------------------------------------------------------------------------------------------------------------
static void defineBuiltInMap(
    const int32_t mapNum,
    const int32_t cluster,
    const String32& name,
    const int32_t musicTrack,
    const SpuReverbMode reverbMode,
    const int16_t reverbDepth
) noexcept {
    Map& map = gMaps.emplace_back();
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
    episode.name = block.getSingleString32Value("Name", episode.name);
    episode.startMap = block.getSingleIntValue("StartMap", episode.startMap);

    if ((episode.startMap < 1) || (episode.startMap > 255)) {
        error(block, "Episode: 'StartMap' must be specified and be between 1 and 255!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a map definition from the specicified value block
//------------------------------------------------------------------------------------------------------------------------------------------
static void readMap(const Block& block) noexcept {
    // Read and validate the episode header
    block.ensureMinHeaderTokenCount(1);
    const int32_t mapNum = block.getRequiredHeaderInt(0);
    
    if ((mapNum < 1) || (mapNum > 255)) {
        error(block, "Map: map number must be between 1 and 255!");
    }

    Map& map = getOrCreateStructWithIntKey(mapNum, gMaps, &Map::mapNum);

    if (block.getHeaderTokenCount() > 1) {
        map.name = block.getRequiredHeaderString32(1);
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
        { "ClearEpisodes", readClearEpisodes},
        { "Episode", readEpisode },
        { "Map", readMap},
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
    defineBuiltInMusicTrack(0, 0);

    for (int32_t trackNum = 1; trackNum <= 30; ++trackNum) {
        defineBuiltInMusicTrack(trackNum, 89 + trackNum);
    }

    // Default episodes
    gEpisodes.clear();

    if (Game::isFinalDoom()) {
        defineBuiltInEpisode(1, 1,  "Master Levels");
        defineBuiltInEpisode(2, 14, "TNT");
        defineBuiltInEpisode(3, 25, "Plutonia");
    } else {
        defineBuiltInEpisode(1, 1,  "Ultimate Doom");
        defineBuiltInEpisode(2, 31, "Doom II");
    }

    // Default maps
    gMaps.clear();

    if (Game::isFinalDoom()) {
        // Master Levels
        defineBuiltInMap(1,  1, "Attack",                   23,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(2,  1, "Virgil",                   29,   SPU_REV_MODE_STUDIO_C,  0x26FF);
        defineBuiltInMap(3,  1, "Canyon",                   24,   SPU_REV_MODE_SPACE,     0x1FFF);
        defineBuiltInMap(4,  1, "Combine",                  30,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(5,  1, "Catwalk",                  21,   SPU_REV_MODE_SPACE,     0x1FFF);
        defineBuiltInMap(6,  1, "Fistula",                  27,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(7,  1, "Geryon",                   25,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(8,  1, "Minos",                    28,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(9,  1, "Nessus",                   22,   SPU_REV_MODE_SPACE,     0x1FFF);
        defineBuiltInMap(10, 1, "Paradox",                  26,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(11, 1, "Subspace",                 1,    SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(12, 1, "Subterra",                 2,    SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(13, 1, "Vesperas",                 3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
        // TNT
        defineBuiltInMap(14, 2, "System Control",           4,    SPU_REV_MODE_HALL,      0x17FF);
        defineBuiltInMap(15, 2, "Human Barbeque",           5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
        defineBuiltInMap(16, 2, "Wormhole",                 6,    SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(17, 2, "Crater",                   7,    SPU_REV_MODE_STUDIO_C,  0x26FF);
        defineBuiltInMap(18, 2, "Nukage Processing",        8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
        defineBuiltInMap(19, 2, "Deepest Reaches",          9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(20, 2, "Processing Area",          10,   SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(21, 2, "Lunar Mining Project",     11,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(22, 2, "Quarry",                   12,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(23, 2, "Ballistyx",                13,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(24, 2, "Heck",                     14,   SPU_REV_MODE_HALL,      0x1FFF);
        // Plutonia
        defineBuiltInMap(25, 3, "Congo",                    15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(26, 3, "Aztec",                    16,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(27, 3, "Ghost Town",               17,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(28, 3, "Baron's Lair",             18,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(29, 3, "The Death Domain",         22,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(30, 3, "Onslaught",                26,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
    } else {
        // Doom 1
        defineBuiltInMap(1 , 1, "Hangar",                   1,    SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(2 , 1, "Plant",                    2,    SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(3 , 1, "Toxin Refinery",           3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(4 , 1, "Command Control",          4,    SPU_REV_MODE_HALL,      0x17FF);
        defineBuiltInMap(5 , 1, "Phobos Lab",               5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
        defineBuiltInMap(6 , 1, "Central Processing",       6,    SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(7 , 1, "Computer Station",         7,    SPU_REV_MODE_STUDIO_C,  0x26FF);
        defineBuiltInMap(8 , 1, "Phobos Anomaly",           8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
        defineBuiltInMap(9 , 1, "Deimos Anomaly",           11,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(10, 1, "Containment Area",         9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(11, 1, "Refinery",                 15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(12, 1, "Deimos Lab",               10,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(13, 1, "Command Center",           19,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(14, 1, "Halls of the Damned",      2,    SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(15, 1, "Spawning Vats",            1,    SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(16, 1, "Hell Gate",                12,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(17, 1, "Hell Keep",                16,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(18, 1, "Pandemonium",              17,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(19, 1, "House of Pain",            6,    SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(20, 1, "Unholy Cathedral",         18,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(21, 1, "Mt. Erebus",               13,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(22, 1, "Limbo",                    14,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(23, 1, "Tower Of Babel",           3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(24, 1, "Hell Beneath",             20,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(25, 1, "Perfect Hatred",           11,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(26, 1, "Sever The Wicked",         7,    SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(27, 1, "Unruly Evil",              4,    SPU_REV_MODE_HALL,      0x17FF);
        defineBuiltInMap(28, 1, "Unto The Cruel",           5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
        defineBuiltInMap(29, 1, "Twilight Descends",        10,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(30, 1, "Threshold of Pain",        19,   SPU_REV_MODE_HALL,      0x1FFF);
        // Doom 2
        defineBuiltInMap(31, 2, "Entryway",                 1,    SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(32, 2, "Underhalls",               9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(33, 2, "The Gantlet",              14,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(34, 2, "The Focus",                12,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(35, 2, "The Waste Tunnels",        8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
        defineBuiltInMap(36, 2, "The Crusher",              13,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(37, 2, "Dead Simple",              18,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(38, 2, "Tricks And Traps",         20,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(39, 2, "The Pit",                  15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(40, 2, "Refueling Base",           19,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(41, 2, "O of Destruction!",        11,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(42, 2, "The Factory",              16,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(43, 2, "The Inmost Dens",          12,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(44, 2, "Suburbs",                  17,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(45, 2, "Tenements",                6,    SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(46, 2, "The Courtyard",            5,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(47, 2, "The Citadel",              9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(48, 2, "Nirvana",                  2,    SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(49, 2, "The Catacombs",            3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(50, 2, "Barrels of Fun",           1,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
        defineBuiltInMap(51, 2, "Bloodfalls",               7,    SPU_REV_MODE_STUDIO_C,  0x26FF);
        defineBuiltInMap(52, 2, "The Abandoned Mines",      8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
        defineBuiltInMap(53, 2, "Monster Condo",            15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
        defineBuiltInMap(54, 2, "Redemption Denied",        4,    SPU_REV_MODE_HALL,      0x17FF);
        defineBuiltInMap(55, 1, "Fortress of Mystery",      17,   SPU_REV_MODE_HALL,      0x1FFF);
        defineBuiltInMap(56, 1, "The Military Base",        18,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(57, 1, "The Marshes",              10,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(58, 2, "The Mansion",              16,   SPU_REV_MODE_SPACE,     0x0FFF);
        defineBuiltInMap(59, 2, "Club Doom",                13,   SPU_REV_MODE_SPACE,     0x0FFF);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the MAPINFO lump, if present from one of the main IWAD files.
// Otherwise initializes MAPINFO to the appropriate settings for the current game (Doom or Final Doom).
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Set MAPINFO to the defaults
    setMapInfoToDefaults();
    readMapInfoFromIWAD();

    // Clear the cached search result for 'getMap'
    gLastGetMap = -1;
    gLastGetMapIndex = -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup of MAPINFO data and releasing resources
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gLastGetMap = -1;
    gLastGetMapIndex = -1;
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
