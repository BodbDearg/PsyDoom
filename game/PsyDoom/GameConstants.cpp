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

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_Doom(GameConstants& consts) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
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
    consts.netGameId = NET_GAMEID_FINAL_DOOM;
    consts.baseNumAnims = BASE_NUM_ANIMS_FDOOM;
    consts.texPalette_BUTTONS = UIPAL2;
    consts.numPalettesRequired = NUMPALETTES_FINAL_DOOM;
    consts.bUseFinalDoomClassicDemoFormat = true;
    consts.bUseFinalDoomSkyPalettes = true;
    consts.bUseFinalDoomPasswordStorage = true;
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

        default:
            FatalErrors::raise("GameConstants::populate(): unhandled game type!");
    }
}
