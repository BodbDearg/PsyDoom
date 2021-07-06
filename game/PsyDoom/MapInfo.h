#pragma once

#include "Macros.h"
#include "SmallString.h"

#include <cstdint>
#include <vector>

enum SpuReverbMode : int32_t;

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a music track played the WESS sequencer system
//------------------------------------------------------------------------------------------------------------------------------------------
struct MusicTrack {
    int32_t     trackNum;      // Music track number. Also determines which LCD file is used (MUSLEV<trackNum>.LCD).
    int32_t     sequenceNum;   // Which sequence number from the module file to play for this music track.

    // Default constructor: marks all non-optional fields as undefined in some way
    inline constexpr MusicTrack() noexcept
        : trackNum(-1)
        , sequenceNum(-1)
    {
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines the name and start map for an episode
//------------------------------------------------------------------------------------------------------------------------------------------
struct Episode {
    int32_t     episodeNum;     // Which episode number this is
    int32_t     startMap;       // Starting map for the episode
    String32    name;           // Name of the episode shown in the main menu

    // Default constructor: marks all non-optional fields as undefined in some way
    inline constexpr Episode() noexcept
        : episodeNum(-1)
        , startMap(-1)
        , name("Unnamed Episode")
    {
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a cluster/group of maps.
// Just specifies how the finale is handled when the cluster end is reached.
//------------------------------------------------------------------------------------------------------------------------------------------
struct Cluster {
    int32_t     clusterNum;         // The cluster number/identifier
    String16    castLcdFile;        // Which LCD file to load for the cast call
    String8     pic;                // Background image displayed for the finale
    int32_t     picPal;             // Which palette to use for the finale background
    int16_t     cdMusicA;           // Which CD music track to play initially for the finale
    int16_t     cdMusicB;           // Which CD music track to play looped after the initial finale track is done
    int16_t     textX;              // Position for the finale text: x; ignored if the text is centered.
    int16_t     textY;              // Position for the finale text: y
    bool        bSkipFinale;        // Show no finale?
    bool        bEnableCast;        // Show the cast call for this finale?
    bool        bNoCenterText;      // Avoid centering the finale text?
    bool        bSmallFont;         // Use a small font for the finale instead of a big one?
    String32    text[30];           // Text lines for the finale text (max 32x30 characters)

    Cluster() noexcept;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines info for a map
//------------------------------------------------------------------------------------------------------------------------------------------
struct Map {
    int32_t         mapNum;                 // Which map number it is (1-255)
    int32_t         music;                  // Which music track to play, or which CD track to play if using CD audio
    String32        name;                   // Name for the map
    int32_t         cluster;                // Which cluster the map belongs to
    int32_t         skyPaletteOverride;     // Overrides the sky palette to use for the map ('-1' if no override)
    bool            bPlayCdMusic;           // If 'true' then 'music' indicates a CDDA track to play, instead of a sequencer track
    SpuReverbMode   reverbMode;             // Which reverb mode to use
    int16_t         reverbDepthL;           // Reverb effect depth for most reverb modes (left)
    int16_t         reverbDepthR;           // Reverb effect depth for most reverb modes (right)
    int16_t         reverbDelay;            // Only used for the 'ECHO' and 'DELAY' reverb modes. Specifies how long the delay is.
    int16_t         reverbFeedback;         // Only used for the 'ECHO' and 'DELAY' reverb modes. Affects the strength of the effect.

    // Default constructor: marks all non-optional fields as undefined in some way
    inline constexpr Map() noexcept
        : mapNum(-1)
        , music(-1)
        , name("Unnamed Map")
        , cluster(-1)
        , skyPaletteOverride(-1)
        , bPlayCdMusic(false)
        , reverbMode(SpuReverbMode{})
        , reverbDepthL(0)
        , reverbDepthR(0)
        , reverbDelay(0)
        , reverbFeedback(0)
    {
    }
};

void init() noexcept;
void shutdown() noexcept;

const MusicTrack* getMusicTrack(const int32_t trackNum) noexcept;
const std::vector<MusicTrack>& allMusicTracks() noexcept;

const Episode* getEpisode(const int32_t episodeNum) noexcept;
const std::vector<Episode>& allEpisodes() noexcept;
int32_t getNumEpisodes() noexcept;

const Map* getMap(const int32_t mapNum) noexcept;
const std::vector<Map>& allMaps() noexcept;

const Cluster* getCluster(const int32_t clusterNum) noexcept;
const std::vector<Cluster>& allClusters() noexcept;

END_NAMESPACE(MapInfo)
