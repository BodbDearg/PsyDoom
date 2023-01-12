//------------------------------------------------------------------------------------------------------------------------------------------
// MapInfo defaults for 'Final Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapInfo_Defaults_FinalDoom.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/cr_main.h"
#include "Doom/UI/ti_main.h"
#include "MapInfo.h"
#include "MapInfo_Defaults.h"
#include "PsyQ/LIBSPU.h"

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a 'GameInfo' struct for 'Final Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
void initGameInfo_FinalDoom(GameInfo& gameInfo) noexcept {
    gameInfo.numMaps = 30;
    gameInfo.numRegularMaps = 30;
    gameInfo.bDisableMultiplayer = false;
    gameInfo.bFinalDoomGameRules = true;
    gameInfo.bAllowWideTitleScreenFire = false;
    gameInfo.bAllowWideOptionsBg = false;
    gameInfo.titleScreenStyle = TitleScreenStyle::FinalDoom;
    gameInfo.creditsScreenStyle = CreditsScreenStyle::FinalDoom;
    gameInfo.texPalette_STATUS = UIPAL;
    gameInfo.texPalette_TITLE = TITLEPAL;
    gameInfo.texPalette_BACK = TITLEPAL;
    gameInfo.texLumpName_BACK = "BACK";
    gameInfo.texPalette_Inter_BACK = {};    // Default: use the same 'BACK' graphic as the main menu
    gameInfo.texLumpName_Inter_BACK = {};
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
    gameInfo.creditsXPos_IDCRED2 = 9;
    gameInfo.creditsXPos_WMSCRED2 = 5;
    gameInfo.creditsXPos_LEVCRED2 = 11;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default episodes for 'Final Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addEpisodes_FinalDoom(std::vector<Episode>& episodes) noexcept {
    addEpisode(episodes, 1, 1,  "Master Levels");
    addEpisode(episodes, 2, 14, "TNT");
    addEpisode(episodes, 3, 25, "Plutonia");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds all the default clusters for 'Final Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addClusters_FinalDoom(std::vector<Cluster>& clusters) noexcept {
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
// Adds the default maps for 'Final Doom' to the given list
//------------------------------------------------------------------------------------------------------------------------------------------
void addMaps_FinalDoom(std::vector<Map>& maps) noexcept {
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

END_NAMESPACE(MapInfo)
