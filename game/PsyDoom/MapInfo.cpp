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
    // Read the MAPINFO string and parse (if it exists)
    std::unique_ptr<char[]> mapInfoStr = readMapInfoLumpAsCString();

    if (mapInfoStr) {
        MapInfo mapInfo = parseMapInfo(mapInfoStr.get());
        // TODO: Handle the map info
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes MAPINFO using built-in MAPINFO appropriate to the current game ('Doom' or 'Final Doom')
//------------------------------------------------------------------------------------------------------------------------------------------
static void setDefaultMapInfo() noexcept {
    // TODO: MapInfo::setDefaultMapInfo
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the MAPINFO lump, if present from one of the main IWAD files.
// Otherwise initializes MAPINFO to the appropriate settings for the current game (Doom or Final Doom).
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    setDefaultMapInfo();
    readMapInfoFromIWAD();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup of MAPINFO data and releasing resources
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    // TODO: MapInfo::shutdown
}

END_NAMESPACE(MapInfo)
