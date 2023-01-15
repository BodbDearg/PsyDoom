#pragma once

#include "Macros.h"
#include "SmallString.h"

#include <vector>

namespace MapInfo {
    struct Cluster;
    struct Episode;
    struct GameInfo;
    struct Map;
    struct MenuSprite;
}

BEGIN_NAMESPACE(GecMapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// GEC specific: defines settings for a sky
//------------------------------------------------------------------------------------------------------------------------------------------
struct Sky {
    // The lump name for the Sky, such as 'SKY04'.
    // Note: if it starts with the 'F_SKY' prefix then the 'F_' part of the prefix is  removed.
    String8 lumpName;

    // Which palette to use for the SKY (0-31)
    uint8_t paletteIdx;

    // Is the sky a special animated fire sky?
    bool bIsFireSky;

    inline constexpr Sky() noexcept
        : lumpName()
        , paletteIdx(0)
        , bIsFireSky(false)
    {
    }
};

bool shouldUseGecMapInfo() noexcept;
void init() noexcept;
void shutdown() noexcept;

const MapInfo::GameInfo& getGameInfo() noexcept;
const MapInfo::MenuSprite& getTitleLogo() noexcept;
const std::vector<MapInfo::Episode>& allEpisodes() noexcept;
const std::vector<MapInfo::Cluster>& allClusters() noexcept;
const std::vector<MapInfo::Map>& allMaps() noexcept;
const std::vector<Sky>& allSkies() noexcept;

const Sky* getSky(const String8& lumpName) noexcept;

END_NAMESPACE(GecMapInfo)
