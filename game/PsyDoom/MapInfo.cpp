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
#include "MapInfo_Parse.h"

#include <algorithm>
#include <memory>

BEGIN_NAMESPACE(MapInfo)

// A saved linear search result. Used to speed up looking up the same MAPINFO item over and over again.
// The integer key of the item being looked up and array index where it was found (past the end if not found).
struct CachedSearchResult {
    int32_t     key;
    uint32_t    itemIdx;
};

static std::vector<MusicTrack>  gMusicTracks;

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a music track from the specicified value block
//------------------------------------------------------------------------------------------------------------------------------------------
static void readMusicTrack(const Block& block) noexcept {
    // Read and validate the track header
    block.ensureHeaderTokenCount(1);
    const int32_t trackNum = (int32_t) block.getRequiredHeaderNumber(0);

    if ((trackNum < 1) || (trackNum > 1024)) {
        error(block, "MusicTrack: track number must be between 1 and 1024!");
    }

    // Get the existing track or make a new one
    MusicTrack* const pExistingTrack = const_cast<MusicTrack*>(getMusicTrack(trackNum));
    MusicTrack& track = (pExistingTrack) ? *pExistingTrack : gMusicTracks.emplace_back(trackNum);

    // Read and validate track properties
    track.sequenceNum = (int32_t) block.getSingleNumberValue("Sequence", (float) track.sequenceNum);

    if ((track.sequenceNum < 0) || (track.sequenceNum > 16384)) {
        error(block, "MusicTrack: 'Sequence' must be specified and be between 0 and 16384!");
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
// Initializes MAPINFO using built-in MAPINFO appropriate to the current game ('Doom' or 'Final Doom')
//------------------------------------------------------------------------------------------------------------------------------------------
static void initDefaultMapInfo() noexcept {
    // Default music tracks, including track '0' (the invalid/null track).
    // There are 30 of these in Final Doom (a superset of Doom's music) and they begin at sequence '90' for track '1'.
    gMusicTracks.push_back(MusicTrack{ 0, 0 });

    for (int32_t trackNum = 1; trackNum <= 30; ++trackNum) {
        gMusicTracks.push_back(MusicTrack{ trackNum, 89 + trackNum });
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the MAPINFO lump, if present from one of the main IWAD files.
// Otherwise initializes MAPINFO to the appropriate settings for the current game (Doom or Final Doom).
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    initDefaultMapInfo();
    readMapInfoFromIWAD();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup of MAPINFO data and releasing resources
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gMusicTracks.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Finds a 'MusicTrack' structure by track number
//------------------------------------------------------------------------------------------------------------------------------------------
const MusicTrack* getMusicTrack(const int32_t trackNum) noexcept {
    const uint32_t numTracks = (uint32_t) gMusicTracks.size();
    const MusicTrack* const pTracks = gMusicTracks.data();

    for (uint32_t i = 0; i < numTracks; ++i) {
        if (pTracks[i].trackNum == trackNum)
            return &pTracks[i];
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns a list of all music tracks in the game
//------------------------------------------------------------------------------------------------------------------------------------------
const std::vector<MusicTrack>& allMusicTracks() noexcept {
    return gMusicTracks;
}

END_NAMESPACE(MapInfo)
