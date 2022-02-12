//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing constants for various different game types
//------------------------------------------------------------------------------------------------------------------------------------------
#include "GameConstants.h"

#include "Doom/cdmaptbl.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Renderer/r_data.h"
#include "FatalErrors.h"
#include "Game.h"

// Game ids for networking
static constexpr uint32_t NET_GAMEID_DOOM           = 0xAA11AA22;
static constexpr uint32_t NET_GAMEID_FINAL_DOOM     = 0xAB11AB22;
static constexpr uint32_t NET_GAMEID_GEC_ME_BETA3   = 0xAB00AB22;

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_Doom(GameConstants& consts) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
    consts.introMovies[0] = "PSXDOOM/ABIN/MOVIE.STR";
    consts.netGameId = NET_GAMEID_DOOM;
    consts.baseNumAnims = BASE_NUM_ANIMS_DOOM;
    consts.texPalette_BUTTONS = MAINPAL;
    consts.numPalettesRequired = NUMPALETTES_DOOM;
    consts.bUseFinalDoomClassicDemoFormat = false;
    consts.bUseFinalDoomSkyPalettes = false;
    consts.bUseFinalDoomPasswordStorage = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'Final Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_FinalDoom(GameConstants& consts) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
    consts.introMovies[0] = "PSXDOOM/ABIN/MOVIE.STR";
    consts.netGameId = NET_GAMEID_FINAL_DOOM;
    consts.baseNumAnims = BASE_NUM_ANIMS_FDOOM;
    consts.texPalette_BUTTONS = UIPAL2;
    consts.numPalettesRequired = NUMPALETTES_FINAL_DOOM;
    consts.bUseFinalDoomClassicDemoFormat = true;
    consts.bUseFinalDoomSkyPalettes = true;
    consts.bUseFinalDoomPasswordStorage = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'GEC Master Edition (Beta 3)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_Beta3(GameConstants& consts) noexcept {
    // Use the settings for Final Doom in most cases
    populateConsts_FinalDoom(consts);

    // Quick hack to support the Master Edition:
    // (1) Load the Final Doom WAD first so that the .ROM format maps reference the correct lump numbers for floor and wall textures.
    // (2) Load the regular Doom WAD on top so we get extra/missing assets from that WAD.
    // (3) But we want Final Doom assets to take precedence, so load that WAD on top of Doom again.
    // 
    // This method is a bit hacky, but does the job to merge the contents of the 2 separate IWADs.
    consts.mainWads[0] = CdFile::PSXFINAL_WAD;
    consts.mainWads[1] = CdFile::PSXDOOM_WAD;
    consts.mainWads[2] = CdFile::PSXFINAL_WAD;

    // All other differing constants
    consts.introMovies[0] = "DATA/MOVIE.STR";
    consts.introMovies[1] = "DATA/GEC.STR";
    consts.introMovies[2] = "DATA/DWORLD.STR";
    consts.netGameId = NET_GAMEID_GEC_ME_BETA3;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the set of game constants for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
void GameConstants::populate(const GameType gameType) noexcept {
    // Default init first
    *this = {};

    // Then populate all fields explicitly
    switch (gameType) {
        case GameType::Doom:            populateConsts_Doom(*this);             break;
        case GameType::FinalDoom:       populateConsts_FinalDoom(*this);        break;
        case GameType::GECMasterBeta3:  populateConsts_GEC_ME_Beta3(*this);     break;

        default:
            FatalErrors::raise("GameConstants::populate(): unhandled game type!");
    }
}
