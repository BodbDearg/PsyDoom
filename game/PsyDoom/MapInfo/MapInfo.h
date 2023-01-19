#pragma once

#include "Macros.h"
#include "SmallString.h"

#include <cstdint>
#include <vector>

enum class CreditsScreenStyle : uint8_t;
enum class TitleScreenStyle : uint8_t;
enum SpuReverbMode : int32_t;

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a music track played the WESS sequencer system
//------------------------------------------------------------------------------------------------------------------------------------------
struct MusicTrack {
    int32_t     trackNum;       // Music track number. Also determines which LCD file is used (MUSLEV<trackNum>.LCD).
    int32_t     sequenceNum;    // Which sequence number from the module file to play for this music track.

    inline constexpr MusicTrack() noexcept
        : trackNum(-1)      // Not yet defined
        , sequenceNum(-1)   // Not yet defined
    {
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// General game settings
//------------------------------------------------------------------------------------------------------------------------------------------
struct GameInfo {
    int32_t                 numMaps;                        // The total number of maps in the game, including secret ones
    int32_t                 numRegularMaps;                 // The total number of regular non-secret maps in the game
    bool                    bDisableMultiplayer;            // If 'true' then the game does not support multiplayer. Multiplayer can be disabled for mods that use tricks which would break in MP game modes.
    bool                    bFinalDoomGameRules;            // If 'true' then the game should use all Final Doom style game rules by default
    bool                    bAllowWideTitleScreenFire;      // Vulkan renderer: if 'true' then the fire sky on the title screen can tile/repeat taking widescreen into account (beyond the original UI area)
    bool                    bAllowWideOptionsBg;            // Vulkan renderer: if 'true' then the options menu background can tile/repeat taking widescreen into account (beyond the original UI area)
    TitleScreenStyle        titleScreenStyle;               // What style of title screen to use
    CreditsScreenStyle      creditsScreenStyle;             // What style of credits screen to use
    int8_t                  titleScreenCdTrackOverride;     // If >= 2 then play this cd track on the title screen instead of the regular one
    uint8_t                 texPalette_titleScreenFire;     // Palette index to use for the fire on the title screen
    uint8_t                 texPalette_STATUS;              // Palette index to use for the 'STATUS' image lump
    uint8_t                 texPalette_TITLE;               // Palette index to use for the 'TITLE' image lump
    uint8_t                 texPalette_TITLE2;              // Palette index to use for the 'TITLE2' image lump (GEC ME specific)
    uint8_t                 texPalette_BACK;                // Palette index to use for the 'BACK' image lump
    uint8_t                 texPalette_Inter_BACK;          // Palette index to use for the 'BACK' image lump on the intermission screen specifically (if using a specific graphic for this screen)
    uint8_t                 texPalette_LOADING;             // Palette index to use for the 'LOADING' image lump
    uint8_t                 texPalette_PAUSE;               // Palette index to use for the 'PAUSE' image lump
    uint8_t                 texPalette_NETERR;              // Palette index to use for the 'NETERR' image lump
    uint8_t                 texPalette_DOOM;                // Palette index to use for the 'DOOM' image lump
    uint8_t                 texPalette_CONNECT;             // Palette index to use for the 'CONNECT' image lump
    uint8_t                 texPalette_IDCRED1;             // Palette index to use for the 'IDCRED1' image lump
    uint8_t                 texPalette_IDCRED2;             // Palette index to use for the 'IDCRED2' image lump
    uint8_t                 texPalette_WMSCRED1;            // Palette index to use for the 'WMSCRED1' image lump
    uint8_t                 texPalette_WMSCRED2;            // Palette index to use for the 'WMSCRED2' image lump
    uint8_t                 texPalette_LEVCRED2;            // Palette index to use for the 'LEVCRED2' image lump
    uint8_t                 texPalette_GEC;                 // Palette index to use for the 'GEC' image lump (GEC Master Edition addition)
    uint8_t                 texPalette_GECCRED;             // Palette index to use for the 'GECCRED' image lump (GEC Master Edition addition)
    uint8_t                 texPalette_DWOLRD;              // Palette index to use for the 'DWOLRD' image lump (GEC Master Edition addition)
    uint8_t                 texPalette_DWCRED;              // Palette index to use for the 'DWCRED' image lump (GEC Master Edition addition)
    uint8_t                 texPalette_DATA;                // Palette index to use for the 'DATA' image lump (GEC Master Edition addition)
    uint8_t                 texPalette_FINAL;               // Palette index to use for the 'FINAL' image lump (GEC Master Edition addition)
    uint8_t                 texPalette_OptionsBG;           // Palette index to use for the options menu tiled background
    String8                 texLumpName_TITLE;              // Which texture lump name to load wherever the 'TITLE' texture is used
    String8                 texLumpName_TITLE2;             // Which texture lump name to load wherever the 'TITLE2' texture is used (GEC ME specific)
    String8                 texLumpName_BACK;               // Which texture lump name to load wherever the 'BACK' texture is used
    String8                 texLumpName_Inter_BACK;         // Which texture lump name to load for the 'BACK' on the intermission screen specifically (if empty, use 'texLumpName_BACK')
    String8                 texLumpName_OptionsBG;          // Which texture lump name to load for the options menu tiled background
    int16_t                 creditsXPos_IDCRED2;            // X position of the 'IDCRED2' graphic on the credits screen
    int16_t                 creditsXPos_WMSCRED2;           // X position of the 'WMSCRED2' graphic on the credits screen
    int16_t                 creditsXPos_LEVCRED2;           // X position of the 'LEVCRED2' graphic on the credits screen
    int16_t                 creditsXPos_GECCRED;            // X position of the 'GECCRED' graphic on the credits screen (GEC Master Edition addition)
    int16_t                 creditsXPos_DWCRED;             // X position of the 'DWCRED' graphic on the credits screen (GEC Master Edition addition)

    GameInfo() noexcept;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Settings for a menu sprite
//------------------------------------------------------------------------------------------------------------------------------------------
struct MenuSprite {
    String8     lumpName;
    uint8_t     paletteIdx;
    int16_t     xPos;
    int16_t     yPos;
    
    inline MenuSprite() noexcept
        : lumpName()
        , paletteIdx()
        , xPos()
        , yPos()
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
    String8     logoPic;        // Episode logo lump name (shown on the main menu)
    int16_t     logoPal;        // Which palette to use for 'logoPic' (if < 0 then use the global palette for the 'DOOM' lump)
    int16_t     logoX;          // Horizontal position for the episode logo
    int16_t     logoYOffset;    // Additional y offset that can be added to the episode logo position
    bool        bIsHidden;      // If 'true' then the episode is not a selectable single player episode (used to define an episode logo for secret maps in multiplayer only)

    inline constexpr Episode() noexcept
        : episodeNum(-1)            // Not yet defined
        , startMap(-1)              // Not yet defined
        , name("Unnamed Episode")   // Not yet defined
        , logoPic("DOOM")           // This is what displayed on the main menu for 'Doom' and 'Final Doom' originally
        , logoPal(-1)               // Use whatever palette is used for the 'DOOM' lump
        , logoX(75)                 // This was the original x position the 'DOOM' image was shown at
        , logoYOffset(0)
        , bIsHidden(false)
    {
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a cluster/group of maps.
// Just specifies how the finale is handled when the cluster end is reached.
//------------------------------------------------------------------------------------------------------------------------------------------
struct Cluster {
    int32_t     clusterNum;             // The cluster number/identifier
    String16    castLcdFile;            // Which LCD file to load for the cast call
    String8     pic;                    // Background image displayed for the finale
    int32_t     picPal;                 // Which palette to use for the finale background
    int16_t     cdMusicA;               // Which CD music track to play initially for the finale
    int16_t     cdMusicB;               // Which CD music track to play looped after the initial finale track is done
    int16_t     textX;                  // Position for the finale text: x; ignored if the text is centered.
    int16_t     textY;                  // Position for the finale text: y
    bool        bSkipFinale;            // Show no finale?
    bool        bHideNextMapForFinale;  // If the conditions to show a finale arise (ignoring the skip flag), hide the 'Entering <MAP_NAME>' message and password on the intermission screen?
    bool        bEnableCast;            // Show the cast call for this finale?
    bool        bNoCenterText;          // Avoid centering the finale text?
    bool        bSmallFont;             // Use a small font for the finale instead of a big one?
    String32    text[30];               // Text lines for the finale text (max 32x30 characters)

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
    bool            bNoIntermission;        // If 'true' then skip the intermission for this map
    SpuReverbMode   reverbMode;             // Which reverb mode to use
    int16_t         reverbDepthL;           // Reverb effect depth for most reverb modes (left)
    int16_t         reverbDepthR;           // Reverb effect depth for most reverb modes (right)
    int16_t         reverbDelay;            // Only used for the 'ECHO' and 'DELAY' reverb modes. Specifies how long the delay is.
    int16_t         reverbFeedback;         // Only used for the 'ECHO' and 'DELAY' reverb modes. Affects the strength of the effect.

    // Default constructor: marks all non-optional fields as undefined in some way
    inline constexpr Map() noexcept
        : mapNum(-1)                    // Not yet defined
        , music(-1)                     // Not yet defined
        , name("Unnamed Map")           // Not yet defined
        , cluster(-1)                   // Not yet defined
        , skyPaletteOverride(-1)        // No override
        , bPlayCdMusic(false)
        , bNoIntermission(false)
        , reverbMode(SpuReverbMode{})
        , reverbDepthL(0)
        , reverbDepthR(0)
        , reverbDelay(0)
        , reverbFeedback(0)
    {
    }
};

bool shouldUse() noexcept;
void init() noexcept;
void shutdown() noexcept;

const MusicTrack* getMusicTrack(const int32_t trackNum) noexcept;
const std::vector<MusicTrack>& allMusicTracks() noexcept;

const GameInfo& getGameInfo() noexcept;

const Episode* getEpisode(const int32_t episodeNum) noexcept;
const std::vector<Episode>& allEpisodes() noexcept;
int32_t getNumEpisodes() noexcept;
const Episode* getNextEpisode(const Episode& episode) noexcept;
const Episode* getPrevEpisode(const Episode& episode) noexcept;

const Cluster* getCluster(const int32_t clusterNum) noexcept;
const std::vector<Cluster>& allClusters() noexcept;

const Map* getMap(const int32_t mapNum) noexcept;
const std::vector<Map>& allMaps() noexcept;

END_NAMESPACE(MapInfo)
