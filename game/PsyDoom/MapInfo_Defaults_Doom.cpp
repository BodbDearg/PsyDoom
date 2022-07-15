//------------------------------------------------------------------------------------------------------------------------------------------
// MapInfo defaults for 'Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapInfo_Defaults_Doom.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/cr_main.h"
#include "Doom/UI/ti_main.h"
#include "Game.h"
#include "MapInfo.h"
#include "MapInfo_Defaults.h"
#include "PsyQ/LIBSPU.h"

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a 'GameInfo' struct for 'Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
void initGameInfo_Doom(GameInfo& gameInfo) noexcept {
    gameInfo.numMaps = 59;
    gameInfo.numRegularMaps = 54;
    gameInfo.bDisableMultiplayer = false;
    gameInfo.bFinalDoomGameRules = false;
    gameInfo.titleScreenStyle = TitleScreenStyle::Doom;
    gameInfo.creditsScreenStyle = CreditsScreenStyle::Doom;
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
    gameInfo.creditsXPos_IDCRED2 = 9;
    gameInfo.creditsXPos_WMSCRED2 = 7;

    // Doom one level demo: don't allow going past 'The Gantlet'
    if (Game::gbIsDemoVersion) {
        gameInfo.numMaps = 33;
        gameInfo.numRegularMaps = 33;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default episodes for 'Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addEpisodes_Doom(std::vector<Episode>& episodes) noexcept {
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
// Adds all the default clusters for 'Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addClusters_Doom(std::vector<Cluster>& clusters) noexcept {
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
// Adds the default maps for 'Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addMaps_Doom(std::vector<Map>& maps) noexcept {
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

END_NAMESPACE(MapInfo)
