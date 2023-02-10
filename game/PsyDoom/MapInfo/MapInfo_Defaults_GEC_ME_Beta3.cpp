//------------------------------------------------------------------------------------------------------------------------------------------
// MapInfo defaults for 'GEC Master Edition (Beta 3)'
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapInfo_Defaults_GEC_ME_Beta3.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/cr_main.h"
#include "Doom/UI/ti_main.h"
#include "MapInfo.h"
#include "MapInfo_Defaults.h"
#include "MapInfo_Defaults_FinalDoom.h"
#include "PsyQ/LIBSPU.h"

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a 'GameInfo' struct for 'GEC Master Edition (Beta 3)'
//------------------------------------------------------------------------------------------------------------------------------------------
void initGameInfo_GEC_ME_Beta3(GameInfo& gameInfo) noexcept {
    // Use Final Doom values for most settings
    initGameInfo_FinalDoom(gameInfo);

    gameInfo.numMaps = 94;
    gameInfo.numRegularMaps = 92;           // Last two maps are secret: stops the game ending on completing 'Go 2 It'
    gameInfo.bFinalDoomGameRules = false;   // Some maps might rely on the extra forward speed of 'Doom'
    gameInfo.titleScreenStyle = TitleScreenStyle::GEC_ME_BETA3;
    gameInfo.creditsScreenStyle = CreditsScreenStyle::GEC_ME;
    gameInfo.texPalette_titleScreenFire = FIRESKYPAL;
    gameInfo.texPalette_BACK = 29;
    gameInfo.texLumpName_BACK = "BACK";
    gameInfo.texPalette_Inter_BACK = {};    // Default: use the same 'BACK' graphic as the main menu
    gameInfo.texLumpName_Inter_BACK = {};
    gameInfo.texPalette_DOOM = 29;
    gameInfo.texPalette_DATA = 30;
    gameInfo.texPalette_FINAL = 29;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default episodes for 'GEC Master Edition (Beta 3)' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addEpisodes_GEC_ME_Beta3(std::vector<Episode>& episodes) noexcept {
    addEpisode(episodes, 1,  1,  "Doom");
    addEpisode(episodes, 2, 31,  "Master Levels");
    addEpisode(episodes, 3, 51,  "TNT");
    addEpisode(episodes, 4, 71,  "Plutonia");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default clusters for 'GEC Master Edition (Beta 3)' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addClusters_GEC_ME_Beta3(std::vector<Cluster>& clusters) noexcept {
    // Use this template for all clusters and customize as required
    Cluster clusTemplate = {};
    clusTemplate.pic = "BACK";
    clusTemplate.picPal = 29;
    clusTemplate.cdMusicA = 9;
    clusTemplate.cdMusicB = (int16_t) gCDTrackNum[cdmusic_credits_demo];
    clusTemplate.textY = 45;
    clusTemplate.bSkipFinale = false;
    clusTemplate.bHideNextMapForFinale = false;
    clusTemplate.bEnableCast = false;
    clusTemplate.text[0] = "We hope you";
    clusTemplate.text[1] = "have enjoyed our master";
    clusTemplate.text[2] = "edition finaldoom beta3";
    clusTemplate.text[3] = "more coming soon";
    clusTemplate.text[4] = "be aware for the";
    clusTemplate.text[5] = "next beta release";
    clusTemplate.text[6] = "thanks";

    // Fill in the Doom and Final Doom episode clusters
    {
        Cluster& clus = clusters.emplace_back(clusTemplate);
        clus.clusterNum = 1;
        clus.text[2] = "edition doom beta 3";
    }
    {
        Cluster& clus = clusters.emplace_back(clusTemplate);
        clus.clusterNum = 2;
    }
    {
        Cluster& clus = clusters.emplace_back(clusTemplate);
        clus.clusterNum = 3;
    }
    {
        Cluster& clus = clusters.emplace_back(clusTemplate);
        clus.clusterNum = 4;
        clusTemplate.bHideNextMapForFinale = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds the default maps for 'GEC Master Edition (Beta 3)' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addMaps_GEC_ME_Beta3(std::vector<Map>& maps) noexcept {
    // Doom, Doom II and others
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
// Adds the credit pages for 'GEC Master Edition (Beta 3)' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addCredits_GEC_ME_Beta3(std::vector<CreditsPage>& credits) noexcept {
    addCreditsPage(credits, "GEC",       "GECCRED",   26,             28,            0,   256);
    addCreditsPage(credits, "DWOLRD",    "DWCRED",    27,             28,            0,   256);
    addCreditsPage(credits, "TITLE",     "LEVCRED2",  TITLEPAL,       WCREDITS1PAL,  11,  256);
    addCreditsPage(credits, "WMSCRED1",  "WMSCRED2",  WCREDITS1PAL,   WCREDITS1PAL,  5,   256);
    addCreditsPage(credits, "IDCRED1",   "IDCRED2",   IDCREDITS1PAL,  WCREDITS1PAL,  9,   256);
}

END_NAMESPACE(MapInfo)
