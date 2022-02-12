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
// Initializes a 'GameInfo' struct for 'Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void initGameInfo_Doom(GameInfo& gameInfo) noexcept {
    gameInfo.numMaps = 59;
    gameInfo.numRegularMaps = 54;
    gameInfo.bDisableMultiplayer = false;
    gameInfo.bFinalDoomGameRules = false;
    gameInfo.bFinalDoomTitleScreen = false;
    gameInfo.bFinalDoomCredits = false;
    gameInfo.texPalette_STATUS = UIPAL;
    gameInfo.texPalette_TITLE = TITLEPAL;
    gameInfo.texPalette_BACK = MAINPAL;
    gameInfo.texPalette_LOADING = UIPAL;
    gameInfo.texPalette_PAUSE = MAINPAL;
    gameInfo.texPalette_NETERR = UIPAL;
    gameInfo.texPalette_DOOM = TITLEPAL;
    gameInfo.texPalette_CONNECT = MAINPAL;
    gameInfo.texPalette_IDCRED1 = IDCREDITS1PAL;
    gameInfo.texPalette_IDCRED2 = UIPAL;
    gameInfo.texPalette_WMSCRED1 = WCREDITS1PAL;
    gameInfo.texPalette_WMSCRED2 = UIPAL;
    gameInfo.texPalette_LEVCRED2 = WCREDITS1PAL;
    gameInfo.texPalette_OptionsBG = MAINPAL;
    gameInfo.texLumpName_OptionsBG = "MARB01";

    // Doom one level demo: don't allow going past 'The Gantlet'
    if (Game::gbIsDemoVersion) {
        gameInfo.numMaps = 33;
        gameInfo.numRegularMaps = 33;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a 'GameInfo' struct for 'Final Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void initGameInfo_FinalDoom(GameInfo& gameInfo) noexcept {
    gameInfo.numMaps = 30;
    gameInfo.numRegularMaps = 30;
    gameInfo.bDisableMultiplayer = false;
    gameInfo.bFinalDoomGameRules = true;
    gameInfo.bFinalDoomTitleScreen = true;
    gameInfo.bFinalDoomCredits = true;
    gameInfo.texPalette_STATUS = UIPAL;
    gameInfo.texPalette_TITLE = TITLEPAL;
    gameInfo.texPalette_BACK = TITLEPAL;
    gameInfo.texPalette_LOADING = UIPAL2;
    gameInfo.texPalette_PAUSE = UIPAL2;
    gameInfo.texPalette_NETERR = UIPAL2;
    gameInfo.texPalette_DOOM = UIPAL;
    gameInfo.texPalette_CONNECT = UIPAL2;
    gameInfo.texPalette_IDCRED1 = IDCREDITS1PAL;
    gameInfo.texPalette_IDCRED2 = WCREDITS1PAL;
    gameInfo.texPalette_WMSCRED1 = WCREDITS1PAL;
    gameInfo.texPalette_WMSCRED2 = WCREDITS1PAL;
    gameInfo.texPalette_LEVCRED2 = WCREDITS1PAL;
    gameInfo.texPalette_OptionsBG = UIPAL2;
    gameInfo.texLumpName_OptionsBG = "TILE";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a 'GameInfo' struct for 'GEC Master Edition (Beta 3)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void initGameInfo_GEC_ME_Beta3(GameInfo& gameInfo) noexcept {
    initGameInfo_FinalDoom(gameInfo);       // Uses Final Doom values for most settings
    gameInfo.numMaps = 94;
    gameInfo.numRegularMaps = 94;
    gameInfo.bFinalDoomGameRules = false;   // Some maps might rely on the extra forward speed of 'Doom'
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the 'GameInfo' struct for the current game to the default set of values
//------------------------------------------------------------------------------------------------------------------------------------------
static void initGameInfo(GameInfo& gameInfo) noexcept {
    gameInfo = GameInfo();

    switch (Game::gGameType) {
        case GameType::Doom:            initGameInfo_Doom(gameInfo);            break;
        case GameType::FinalDoom:       initGameInfo_FinalDoom(gameInfo);       break;
        case GameType::GECMasterBeta3:  initGameInfo_GEC_ME_Beta3(gameInfo);    break;

        default:
            FatalErrors::raise("MapInfo::initGameInfo(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default episodes for 'Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addEpisodes_Doom(std::vector<Episode>& episodes) noexcept {
    if (!Game::gbIsDemoVersion) {
        // Doom: full game
        addEpisode(episodes, 1, 1,  "Ultimate Doom");
        addEpisode(episodes, 2, 31, "Doom II");
    } else {
        // Doom: one level demo version
        addEpisode(episodes, 1, 33, "Level 33");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default episodes for 'Final Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addEpisodes_FinalDoom(std::vector<Episode>& episodes) noexcept {
    addEpisode(episodes, 1, 1,  "Master Levels");
    addEpisode(episodes, 2, 14, "TNT");
    addEpisode(episodes, 3, 25, "Plutonia");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default episodes for 'GEC Master Edition (Beta 3)' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addEpisodes_GEC_ME_Beta3(std::vector<Episode>& episodes) noexcept {
    addEpisode(episodes, 1,  1,  "Doom");
    addEpisode(episodes, 2, 31,  "Master Levels");
    addEpisode(episodes, 3, 51,  "TNT");
    addEpisode(episodes, 4, 71,  "Plutonia");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of episodes for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initEpisodes(std::vector<Episode>& episodes) noexcept {
    episodes.clear();

    switch (Game::gGameType) {
        case GameType::Doom:            addEpisodes_Doom(episodes);             break;
        case GameType::FinalDoom:       addEpisodes_FinalDoom(episodes);        break;
        case GameType::GECMasterBeta3:  addEpisodes_GEC_ME_Beta3(episodes);     break;

        default:
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
// Adds all the default clusters for 'GEC Master Edition (Beta 3)' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addClusters_GEC_ME_Beta3(std::vector<Cluster>& clusters) noexcept {
    // No finale for the GEC ME!
    // TODO: GEC ME BETA 3: figure out end cast
    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 1;
        clus.bSkipFinale = true;
        clus.bHideNextMapForFinale = true;
        clus.bEnableCast = false;
    }
    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 2;
        clus.bSkipFinale = true;
        clus.bHideNextMapForFinale = true;
        clus.bEnableCast = false;
    }
    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 3;
        clus.bSkipFinale = true;
        clus.bHideNextMapForFinale = true;
        clus.bEnableCast = false;
    }
    {
        Cluster& clus = clusters.emplace_back();
        clus.clusterNum = 4;
        clus.bSkipFinale = true;
        clus.bHideNextMapForFinale = true;
        clus.bEnableCast = false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of clusters for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initClusters(std::vector<Cluster>& clusters) noexcept {
    clusters.clear();

    switch (Game::gGameType) {
        case GameType::Doom:            addClusters_Doom(clusters);             break;
        case GameType::FinalDoom:       addClusters_FinalDoom(clusters);        break;
        case GameType::GECMasterBeta3:  addClusters_GEC_ME_Beta3(clusters);     break;

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
// Adds the default maps for 'GEC Master Edition (Beta 3)' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
static void addMaps_GEC_ME_Beta3(std::vector<Map>& maps) noexcept {
    // Doom
    addMap(maps, 1 , 1, "Phobos Mission Control",   6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 2 , 1, "Forgotten Sewers",         1,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 3 , 1, "Altar Of Extraction",      15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 4 , 1, "Hell Keep",                16,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 5 , 1, "Slough Of Despair",        12,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 6 , 1, "They Will Repent",         2,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 7 , 1, "Against Thee Wickedly",    4,    SPU_REV_MODE_HALL,      0x17FF);
    addMap(maps, 8 , 1, "And Hell Followed",        14,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 9 , 1, "Dis",                      9,    SPU_REV_MODE_STUDIO_C,  0x2FFF);
    addMap(maps, 10, 1, "Industrial Zone",          7,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 11, 1, "Betray",                   2,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 12, 1, "Gotcha",                   12,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 13, 1, "The Chasm",                15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 14, 1, "The Spirit World",         19,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 15, 1, "The Living End",           10,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 16, 1, "Icon Of Sin",              13,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 17, 1, "The Earth Base",           18,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 18, 1, "The Pain Labs",            8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
    addMap(maps, 19, 1, "Canyon of the Dead",       6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 20, 1, "Hell Mountain",            12,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 21, 1, "Vivisection",              17,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 22, 1, "Inferno of Blood",         18,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 23, 1, "Barons Banquet",           10,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 24, 1, "Tomb Of Malevolence",      17,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 25, 1, "Warrens",                  7,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 26, 1, "Fear",                     11,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 27, 1, "Wolfenstein",              3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 28, 1, "Grosse",                   10,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 29, 1, "March Of The Demons",      12,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 30, 1, "Baphomet Demense",         20,   SPU_REV_MODE_STUDIO_C,  0x2DFF);

    // Master Levels
    addMap(maps, 31, 2, "Titan Manor",              22,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 32, 2, "Trapped On Titan",         5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
    addMap(maps, 33, 2, "The Garrison",             17,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 34, 2, "Black Tower",              8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
    addMap(maps, 35, 2, "Bloodsea Keep",            26,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 36, 2, "TEETH",                    28,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 37, 2, "The Titan Anomaly",        5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
    addMap(maps, 38, 2, "The Farside Of Titan",     28,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 39, 2, "Dantes Gate",              22,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 40, 2, "Crossing Acheron",         3,    SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 41, 2, "The Hive!",                29,   SPU_REV_MODE_STUDIO_C,  0x27FF);
    addMap(maps, 42, 2, "The D.M.Z.",               21,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 43, 2, "The C.P.U",                30,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 44, 2, "The Fury",                 27,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 45, 2, "The Enemy Inside",         28,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 46, 2, "Device One",               2,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 47, 2, "Bloodflood",               24,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 48, 2, "Derelict Station",         26,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 49, 2, "The Image of Evil",        13,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 50, 2, "Bad Dream",                21,   SPU_REV_MODE_SPACE,     0x1FFF);
    // TNT
    addMap(maps, 51, 3, "Power Control",            24,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 52, 3, "Hanger",                   25,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 53, 3, "Open Season",              28,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 54, 3, "Prison",                   6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 55, 3, "Metal",                    12,   SPU_REV_MODE_STUDIO_C,  0x2DFF);
    addMap(maps, 56, 3, "Stronghold",               30,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 57, 3, "Redemption",               17,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 58, 3, "Storage Facility",         15,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    addMap(maps, 59, 3, "Steel Works",              1,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 60, 3, "Dead Zone",                5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
    addMap(maps, 61, 3, "Mill",                     22,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 62, 3, "Shipping Respawning",      2,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 63, 3, "Central Processing",       9,    SPU_REV_MODE_STUDIO_C,  0x2DFF);
    addMap(maps, 64, 3, "Administration Center",    11,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 65, 3, "Habitat",                  9,    SPU_REV_MODE_STUDIO_C,  0x2DFF);
    addMap(maps, 66, 3, "Barons Den",               26,   SPU_REV_MODE_STUDIO_C,  0x2DFF);
    addMap(maps, 67, 3, "Mount Pain",               18,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 68, 3, "River Styx",               12,   SPU_REV_MODE_STUDIO_C,  0x2DFF);
    addMap(maps, 69, 3, "Pharaoh",                  21,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 70, 3, "Caribbean",                10,   SPU_REV_MODE_STUDIO_B,  0x27FF);
    // Plutonia
    addMap(maps, 71, 4, "Well Of Souls",            8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
    addMap(maps, 72, 4, "Caged",                    23,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 73, 4, "Caughtyard",               26,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 74, 4, "Abattoire",                14,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 75, 4, "Hunted",                   21,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 76, 4, "Speed",                    4,    SPU_REV_MODE_HALL,      0x17FF);
    addMap(maps, 77, 4, "The Crypt",                27,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 78, 4, "Genesis",                  25,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 79, 4, "The Twilight",             22,   SPU_REV_MODE_SPACE,     0x1FFF);
    addMap(maps, 80, 4, "The Omen",                 18,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 81, 4, "Compound",                 2,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 82, 4, "Neurosphere",              5,    SPU_REV_MODE_STUDIO_A,  0x23FF);
    addMap(maps, 83, 4, "N-M-E",                    17,   SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 84, 4, "Slayer",                   2,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 85, 4, "Impossible Mission",       7,    SPU_REV_MODE_STUDIO_C,  0x26FF);
    addMap(maps, 86, 4, "Tombstone",                13,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 87, 4, "The Final Frontier",       25,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 88, 4, "The Temple Of Darkness",   6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 89, 4, "Bunker",                   2,    SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 90, 4, "Anti-Christ",              7,    SPU_REV_MODE_STUDIO_C,  0x26FF);
    addMap(maps, 91, 4, "The Sewers",               6,    SPU_REV_MODE_HALL,      0x1FFF);
    addMap(maps, 92, 4, "Odyssey of Noises",        26,   SPU_REV_MODE_SPACE,     0x0FFF);
    addMap(maps, 93, 4, "Cyberden",                 8,    SPU_REV_MODE_STUDIO_B,  0x2DFF);
    addMap(maps, 94, 4, "Go 2 It",                  17,   SPU_REV_MODE_HALL,      0x1FFF);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of maps for the game to the defaults for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
static void initMaps(std::vector<Map>& maps) noexcept {
    maps.clear();

    switch (Game::gGameType) {
        case GameType::Doom:            addMaps_Doom(maps);             break;
        case GameType::FinalDoom:       addMaps_FinalDoom(maps);        break;
        case GameType::GECMasterBeta3:  addMaps_GEC_ME_Beta3(maps);     break;

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
