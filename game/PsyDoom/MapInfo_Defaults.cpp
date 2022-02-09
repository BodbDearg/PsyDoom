#include "MapInfo.h"

#include "Asserts.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Renderer/r_data.h"
#include "FatalErrors.h"
#include "Game.h"
#include "PsyQ/LIBSPU.h"

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: defines an episode and adds it to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addEpisode(
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
static void addMap(
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

    if (Game::gGameType == GameType::FinalDoom) {
        gameInfo.numMaps = 30;
        gameInfo.numRegularMaps = 30;
        gameInfo.bFinalDoomGameRules = true;
    }
    else if (Game::gGameType == GameType::Doom) {
        gameInfo.numMaps = 59;
        gameInfo.numRegularMaps = 54;
        gameInfo.bFinalDoomGameRules = false;

        // Doom one level demo: don't allow going past 'The Gantlet'
        if (Game::gbIsDemoVersion) {
            gameInfo.numMaps = 33;
            gameInfo.numRegularMaps = 33;
        }
    }
    else {
        FatalErrors::raise("MapInfo::initGameInfo(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of episodes for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initEpisodes(std::vector<Episode>& episodes) noexcept {
    episodes.clear();

    if (Game::gGameType == GameType::FinalDoom) {
        addEpisode(episodes, 1, 1,  "Master Levels");
        addEpisode(episodes, 2, 14, "TNT");
        addEpisode(episodes, 3, 25, "Plutonia");
    }
    else if (Game::gGameType == GameType::Doom) {
        if (!Game::gbIsDemoVersion) {
            // Doom: full game
            addEpisode(episodes, 1, 1,  "Ultimate Doom");
            addEpisode(episodes, 2, 31, "Doom II");
        } else {
            // Doom: one level demo version
            addEpisode(episodes, 1, 33, "Level 33");
        }
    }
    else {
        FatalErrors::raise("MapInfo::initEpisodes(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default clusters for 'Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addClusters_Doom(std::vector<Cluster>& clusters) noexcept {
    const size_t startClusterIdx = clusters.size();

    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 1;
        clus.castLcdFile = "MAP60.LCD";
        clus.pic = "BACK";
        clus.picPal = MAINPAL;
        clus.cdMusicA = (int16_t) gCDTrackNum[cdmusic_finale_doom1_final_doom];
        clus.cdMusicB = (int16_t) gCDTrackNum[cdmusic_credits_demo];
        clus.textY = 45;
        clus.bSkipFinale = false;
        clus.bHideNextMapForFinale = false;
        clus.bEnableCast = false;
        clus.text[0] = "you have won!";
        clus.text[1] = "your victory enabled";
        clus.text[2] = "humankind to evacuate";
        clus.text[3] = "earth and escape the";
        clus.text[4] = "nightmare.";
        clus.text[5] = "but then earth control";
        clus.text[6] = "pinpoints the source";
        clus.text[7] = "of the alien invasion.";
        clus.text[8] = "you are their only hope.";
        clus.text[9] = "you painfully get up";
        clus.text[10] = "and return to the fray.";
    }
    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 2;
        clus.castLcdFile = "MAP60.LCD";
        clus.pic = "DEMON";
        clus.picPal = MAINPAL;
        clus.cdMusicA = (int16_t) gCDTrackNum[cdmusic_finale_doom2];
        clus.cdMusicB = (int16_t) gCDTrackNum[cdmusic_credits_demo];
        clus.textY = 45;
        clus.bSkipFinale = false;
        clus.bHideNextMapForFinale = false;
        clus.bEnableCast = true;
        clus.text[0] = "you did it!";
        clus.text[1] = "by turning the evil of";
        clus.text[2] = "the horrors of hell in";
        clus.text[3] = "upon itself you have";
        clus.text[4] = "destroyed the power of";
        clus.text[5] = "the demons.";
        clus.text[6] = "their dreadful invasion";
        clus.text[7] = "has been stopped cold!";
        clus.text[8] = "now you can retire to";
        clus.text[9] = "a lifetime of frivolity.";
        clus.text[10] = "congratulations!";
    }

    // Doom one level demo: skip all finales
    if (Game::gbIsDemoVersion) {
        for (size_t clusterIdx = startClusterIdx; clusterIdx < clusters.size(); ++clusterIdx) {
            Cluster& clus = clusters[clusterIdx];
            clus.bSkipFinale = true;
            clus.bHideNextMapForFinale = true;
            clus.bEnableCast = false;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default clusters for 'Final Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addClusters_FinalDoom(std::vector<Cluster>& clusters) noexcept {
    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 1;
        clus.castLcdFile = "MAP60.LCD";
        clus.pic = "BACK";
        clus.picPal = TITLEPAL;
        clus.cdMusicA = (int16_t) gCDTrackNum[cdmusic_finale_doom1_final_doom];
        clus.cdMusicB = (int16_t) gCDTrackNum[cdmusic_credits_demo];
        clus.textY = 22;
        clus.bSkipFinale = false;
        clus.bHideNextMapForFinale = false;
        clus.bEnableCast = false;
        clus.text[0] = "you have assaulted and";
        clus.text[1] = "triumphed over the most";
        clus.text[2] = "vicious realms that the";
        clus.text[3] = "demented minds of our";
        clus.text[4] = "designers could devise.";
        clus.text[5] = "the havoc you left";
        clus.text[6] = "behind you as you";
        clus.text[7] = "smashed your way";
        clus.text[8] = "through the master";
        clus.text[9] = "levels is mute tribute";
        clus.text[10] = "to your prowess.";
        clus.text[11] = "you have earned the";
        clus.text[12] = "title of";
        clus.text[13] = "Master of Destruction.";
    }
    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 2;
        clus.castLcdFile = "MAP60.LCD";
        clus.pic = "BACK";
        clus.picPal = TITLEPAL;
        clus.cdMusicA = (int16_t) gCDTrackNum[cdmusic_finale_doom1_final_doom];
        clus.cdMusicB = (int16_t) gCDTrackNum[cdmusic_credits_demo];
        clus.textY = 29;
        clus.bSkipFinale = false;
        clus.bHideNextMapForFinale = false;
        clus.bEnableCast = false;
        clus.text[0] = "suddenly all is silent";
        clus.text[1] = "from one horizon to the";
        clus.text[2] = "other.";
        clus.text[3] = "the agonizing echo of";
        clus.text[4] = "hell fades away.";
        clus.text[5] = "the nightmare sky";
        clus.text[6] = "turns blue.";
        clus.text[7] = "the heaps of monster";
        clus.text[8] = "corpses begin to dissolve";
        clus.text[9] = "along with the evil stench";
        clus.text[10] = "that filled the air.";
        clus.text[11] = "maybe you_have done it.";
        clus.text[12] = "Have you really won...";
    }
    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 3;
        clus.castLcdFile = "MAP60.LCD";
        clus.pic = "DEMON";
        clus.picPal = MAINPAL;
        clus.cdMusicA = (int16_t) gCDTrackNum[cdmusic_finale_doom1_final_doom];
        clus.cdMusicB = (int16_t) gCDTrackNum[cdmusic_credits_demo];
        clus.textY = 15;
        clus.bSkipFinale = false;
        clus.bHideNextMapForFinale = false;
        clus.bEnableCast = true;
        clus.text[0] = "you_gloat_over_the";
        clus.text[1] = "carcass_of_the_guardian.";
        clus.text[2] = "with_its_death_you_have";
        clus.text[3] = "wrested_the_accelerator";
        clus.text[4] = "from_the_stinking_claws";
        clus.text[5] = "of_hell._you_are_done.";
        clus.text[6] = "hell_has_returned_to";
        clus.text[7] = "pounding_dead_folks";
        clus.text[8] = "instead_of_good_live_ones.";
        clus.text[9] = "remember_to_tell_your";
        clus.text[10] = "grandkids_to_put_a_rocket";
        clus.text[11] = "launcher_in_your_coffin.";
        clus.text[12] = "If_you_go_to_hell_when";
        clus.text[13] = "you_die_you_will_need_it";
        clus.text[14] = "for_some_cleaning_up.";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of clusters for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initClusters(std::vector<Cluster>& clusters) noexcept {
    clusters.clear();

    switch (Game::gGameType) {
        case GameType::Doom:        addClusters_Doom(clusters);         break;
        case GameType::FinalDoom:   addClusters_FinalDoom(clusters);    break;

        default:
            FatalErrors::raise("MapInfo::initClusters(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds the default maps for 'Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addMaps_Doom(std::vector<Map>& maps) noexcept {
    const size_t startMapIdx = maps.size();

    // Doom 1
    addMap(maps, 1 , 1, "Hangar",                   1,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 2 , 1, "Plant",                    2,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 3 , 1, "Toxin Refinery",           3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 4 , 1, "Command Control",          4,    SPU_REV_MODE_HALL,      0x17FF);
    addMap(maps, 5 , 1, "Phobos Lab",               5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
    addMap(maps, 6 , 1, "Central Processing",       6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 7 , 1, "Computer Station",         7,    SPU_REV_MODE_STUDIO_C,  0x26FF);
    addMap(maps, 8 , 1, "Phobos Anomaly",           8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
    addMap(maps, 9 , 1, "Deimos Anomaly",           11,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 10, 1, "Containment Area",         9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 11, 1, "Refinery",                 15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 12, 1, "Deimos Lab",               10,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 13, 1, "Command Center",           19,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 14, 1, "Halls of the Damned",      2,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 15, 1, "Spawning Vats",            1,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 16, 1, "Hell Gate",                12,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 17, 1, "Hell Keep",                16,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 18, 1, "Pandemonium",              17,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 19, 1, "House of Pain",            6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 20, 1, "Unholy Cathedral",         18,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 21, 1, "Mt. Erebus",               13,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 22, 1, "Limbo",                    14,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 23, 1, "Tower Of Babel",           3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 24, 1, "Hell Beneath",             20,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 25, 1, "Perfect Hatred",           11,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 26, 1, "Sever The Wicked",         7,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 27, 1, "Unruly Evil",              4,    SPU_REV_MODE_HALL,      0x17FF);
    addMap(maps, 28, 1, "Unto The Cruel",           5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
    addMap(maps, 29, 1, "Twilight Descends",        10,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 30, 1, "Threshold of Pain",        19,   SPU_REV_MODE_HALL,      0x1FFF);
    // Doom 2
    addMap(maps, 31, 2, "Entryway",                 1,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 32, 2, "Underhalls",               9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 33, 2, "The Gantlet",              14,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 34, 2, "The Focus",                12,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 35, 2, "The Waste Tunnels",        8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
    addMap(maps, 36, 2, "The Crusher",              13,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 37, 2, "Dead Simple",              18,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 38, 2, "Tricks And Traps",         20,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 39, 2, "The Pit",                  15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 40, 2, "Refueling Base",           19,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 41, 2, "O of Destruction!",        11,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 42, 2, "The Factory",              16,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 43, 2, "The Inmost Dens",          12,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 44, 2, "Suburbs",                  17,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 45, 2, "Tenements",                6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 46, 2, "The Courtyard",            5,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 47, 2, "The Citadel",              9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 48, 2, "Nirvana",                  2,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 49, 2, "The Catacombs",            3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 50, 2, "Barrels of Fun",           1,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 51, 2, "Bloodfalls",               7,    SPU_REV_MODE_STUDIO_C,  0x26FF);
    addMap(maps, 52, 2, "The Abandoned Mines",      8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
    addMap(maps, 53, 2, "Monster Condo",            15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 54, 2, "Redemption Denied",        4,    SPU_REV_MODE_HALL,      0x17FF);
    addMap(maps, 55, 1, "Fortress of Mystery",      17,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 56, 1, "The Military Base",        18,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 57, 1, "The Marshes",              10,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 58, 2, "The Mansion",              16,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 59, 2, "Club Doom",                13,   SPU_REV_MODE_SPACE,     0x0FFF);

    // Doom one level demo: play the credits music for the demo map
    if (Game::gbIsDemoVersion) {
        Map& map = maps[startMapIdx + 32];
        map.bPlayCdMusic = true;
        map.music = gCDTrackNum[cdmusic_credits_demo];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds the default maps for 'Final Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addMaps_FinalDoom(std::vector<Map>& maps) noexcept {
    // Master Levels
    addMap(maps, 1,  1, "Attack",                   23,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 2,  1, "Virgil",                   29,   SPU_REV_MODE_STUDIO_C,  0x26FF);
    addMap(maps, 3,  1, "Canyon",                   24,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 4,  1, "Combine",                  30,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 5,  1, "Catwalk",                  21,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 6,  1, "Fistula",                  27,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 7,  1, "Geryon",                   25,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 8,  1, "Minos",                    28,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 9,  1, "Nessus",                   22,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 10, 1, "Paradox",                  26,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 11, 1, "Subspace",                 1,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 12, 1, "Subterra",                 2,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 13, 1, "Vesperas",                 3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    // TNT
    addMap(maps, 14, 2, "System Control",           4,    SPU_REV_MODE_HALL,      0x17FF);
    addMap(maps, 15, 2, "Human Barbeque",           5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
    addMap(maps, 16, 2, "Wormhole",                 6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 17, 2, "Crater",                   7,    SPU_REV_MODE_STUDIO_C,  0x26FF);
    addMap(maps, 18, 2, "Nukage Processing",        8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
    addMap(maps, 19, 2, "Deepest Reaches",          9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 20, 2, "Processing Area",          10,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 21, 2, "Lunar Mining Project",     11,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 22, 2, "Quarry",                   12,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 23, 2, "Ballistyx",                13,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 24, 2, "Heck",                     14,   SPU_REV_MODE_HALL,      0x1FFF);
    // Plutonia
    addMap(maps, 25, 3, "Congo",                    15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 26, 3, "Aztec",                    16,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 27, 3, "Ghost Town",               17,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 28, 3, "Baron's Lair",             18,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 29, 3, "The Death Domain",         22,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 30, 3, "Onslaught",                26,   SPU_REV_MODE_STUDIO_C,  0x2FFF);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of maps for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initMaps(std::vector<Map>& maps) noexcept {
    maps.clear();

    switch (Game::gGameType) {
        case GameType::Doom:        addMaps_Doom(maps);         break;
        case GameType::FinalDoom:   addMaps_FinalDoom(maps);    break;

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
    initGameInfo(gameInfo);
    initEpisodes(episodes);
    initClusters(clusters);
    initMaps(maps);
    initMusicTracks(musicTracks);
}

END_NAMESPACE(MapInfo)
