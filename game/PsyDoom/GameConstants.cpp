//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing constants for various different game types
//------------------------------------------------------------------------------------------------------------------------------------------
#include "GameConstants.h"

#include "BuiltInPaletteData.h"
#include "Doom/cdmaptbl.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Renderer/r_data.h"
#include "FatalErrors.h"
#include "Game.h"
#include "PlayerPrefs.h"

#include <cstdio>

// Game ids for networking
static constexpr uint32_t NET_GAMEID_DOOM                       = 0xAA11AA22;
static constexpr uint32_t NET_GAMEID_FINAL_DOOM                 = 0xAB11AB22;
static constexpr uint32_t NET_GAMEID_GEC_ME_BETA3               = 0xAB00AB22;
static constexpr uint32_t NET_GAMEID_GEC_ME_TESTMAP_DOOM        = 0xBB00BB22;
static constexpr uint32_t NET_GAMEID_GEC_ME_TESTMAP_FINAL_DOOM  = 0xBB00BB23;
static constexpr uint32_t NET_GAMEID_GEC_ME_BETA4               = 0xAB00AB23;

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_Doom(GameConstants& consts, const bool bIsDemoVersion) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
    consts.introMovies[0] = "PSXDOOM/ABIN/MOVIE.STR";

    if (!bIsDemoVersion) {
        consts.demos[0] = BuiltInDemoDef{ "DEMO1.LMP", false, (Game::gGameVariant == GameVariant::PAL), true  };
        consts.demos[1] = BuiltInDemoDef{ "DEMO2.LMP", false, (Game::gGameVariant == GameVariant::PAL), false };
    } else {
        consts.demos[0] = BuiltInDemoDef{ "DEMO2.LMP", false, (Game::gGameVariant == GameVariant::PAL), true  };    // Demo only uses 'DEMO2.LMP'!
    }

    consts.saveFilePrefix = "Doom_";
    consts.pLastPasswordField = &PlayerPrefs::gLastPassword_Doom;
    consts.netGameId = NET_GAMEID_DOOM;
    consts.baseNumAnims = BASE_NUM_ANIMS_DOOM;
    consts.texPalette_BUTTONS = MAINPAL;
    consts.numPalettesRequired = NUMPALETTES_DOOM;
    consts.bUseFinalDoomSkyPalettes = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'Final Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_FinalDoom(GameConstants& consts) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
    consts.introMovies[0] = "PSXDOOM/ABIN/MOVIE.STR";
    consts.demos[0] = BuiltInDemoDef{ "DEMO1.LMP", true, (Game::gGameVariant == GameVariant::PAL), true  };
    consts.demos[1] = BuiltInDemoDef{ "DEMO2.LMP", true, (Game::gGameVariant == GameVariant::PAL), false };
    consts.saveFilePrefix = "FDoom_";
    consts.pLastPasswordField = &PlayerPrefs::gLastPassword_FDoom;
    consts.netGameId = NET_GAMEID_FINAL_DOOM;
    consts.baseNumAnims = BASE_NUM_ANIMS_FDOOM;
    consts.texPalette_BUTTONS = UIPAL2;
    consts.numPalettesRequired = NUMPALETTES_FINAL_DOOM;
    consts.bUseFinalDoomSkyPalettes = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'GEC Master Edition (Beta 3)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_Beta3(GameConstants& consts) noexcept {
    // Default to the settings used by Final Doom for any unspecified fields
    populateConsts_FinalDoom(consts);

    // Quick hack to support the Master Edition:
    // (1) Load the Final Doom WAD first so that the .ROM format maps reference the correct lump numbers for floor and wall textures.
    // (2) Load the regular Doom WAD on top so we get extra/missing assets from that WAD.
    // (3) Load the GEC WAD so we can get a few resources for the credits screen.
    // (4) But we want Final Doom assets to take precedence over everything, so load that WAD on top of everything again.
    // 
    // This method is a bit hacky, but does the job to merge the contents of the 2 separate IWADs.
    consts.mainWads[0] = CdFile::PSXFINAL_WAD;
    consts.mainWads[1] = CdFile::MEDOOM_WAD;
    consts.mainWads[2] = CdFile::PSXDOOM_WAD;
    consts.mainWads[3] = CdFile::PSXFINAL_WAD;

    // Lump name remapping to support this game:
    consts.mainWadLumpRemappers[0] = [](const WadLumpName& oldName) noexcept -> WadLumpName {
        // Use the main menu background from 'MEDOOM.WAD' instead of 'PSXFINAL.WAD'
        if (oldName == "BACK")      { return "_UNUSED1";  }
        // Use the 'DOOM' logo from 'MEDOOM.WAD' instead of 'PSXFINAL.WAD'
        if (oldName == "DOOM")      { return "_UNUSED2";  }

        // Leave this lump name alone!
        return oldName;
    };

    consts.mainWadLumpRemappers[1] = [](const WadLumpName& oldName) noexcept -> WadLumpName {
        // Need to access this palette in 'MEDOOM.WAD' separately to the Final Doom 'PLAYPAL' for decoding intro logos
        if (oldName == "PLAYPAL")   { return "GECINPAL";  }

        // Leave this lump name alone!
        return oldName;
    };

    consts.mainWadLumpRemappers[2] = [](const WadLumpName& oldName) noexcept -> WadLumpName {
        // Rename the 'Doom' version of this texture so we can use it instead of the 'Final Doom' version in 'Doom' episode maps
        if (oldName == "REDROK01")  { return "REDROKX1";  }

        // Leave this lump name alone!
        return oldName;
    };

    // Demo files: note that we have to remap the map numbers for the 'Final Doom' episodes!
    {
        const BuiltInDemoDef::MapNumOverrideFn remapMasterLevelsMapNum = [](const int32_t mapNum) noexcept { return mapNum + 30; };
        const BuiltInDemoDef::MapNumOverrideFn remapTntMapNum          = [](const int32_t mapNum) noexcept { return mapNum + 50; };
        const BuiltInDemoDef::MapNumOverrideFn remapPlutoniaMapNum     = [](const int32_t mapNum) noexcept { return mapNum + 70; };

        consts.demos[0] = BuiltInDemoDef{ "DEMO1.LMP", false, false, true                           };
        consts.demos[1] = BuiltInDemoDef{ "DEMO2.LMP", false, false, true                           };
        consts.demos[2] = BuiltInDemoDef{ "DEMO3.LMP", true,  false, true,  remapMasterLevelsMapNum };
        consts.demos[3] = BuiltInDemoDef{ "DEMO4.LMP", true,  false, true,  remapMasterLevelsMapNum };
        consts.demos[4] = BuiltInDemoDef{ "DEMO5.LMP", true,  false, true,  remapTntMapNum          };
        consts.demos[5] = BuiltInDemoDef{ "DEMO6.LMP", true,  false, true,  remapTntMapNum          };
        consts.demos[6] = BuiltInDemoDef{ "DEMO7.LMP", true,  false, true,  remapPlutoniaMapNum     };
        consts.demos[7] = BuiltInDemoDef{ "DEMO8.LMP", true,  false, false, remapPlutoniaMapNum     };
    }

    // All other differing constants
    consts.introMovies[0] = "DATA/MOVIE.STR";
    consts.introMovies[1] = "DATA/GEC.STR";
    consts.introMovies[2] = "DATA/DWORLD.STR";
    consts.saveFilePrefix = "GecMe_";
    consts.pLastPasswordField = &PlayerPrefs::gLastPassword_GecMe;
    consts.netGameId = NET_GAMEID_GEC_ME_BETA3;
    consts.pExtraPalettes = BuiltInPaletteData::GEC_ME_BETA3_EXTRA_PALETTES;
    consts.numExtraPalettes = BuiltInPaletteData::NUM_GEC_ME_BETA3_EXTRA_PALETTES;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for '[GEC] Master Edition tools: single map test disc (Doom format)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_TestMap_Doom(GameConstants& consts) noexcept {
    populateConsts_Doom(consts, false);
    consts.introMovies[0] = "";
    consts.demos[0] = {};
    consts.demos[1] = {};
    consts.netGameId = NET_GAMEID_GEC_ME_TESTMAP_DOOM;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for '[GEC] Master Edition tools: single map test disc (Final Doom format)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_TestMap_FinalDoom(GameConstants& consts) noexcept {
    populateConsts_FinalDoom(consts);
    consts.introMovies[0] = "";
    consts.demos[0] = {};
    consts.demos[1] = {};
    consts.netGameId = NET_GAMEID_GEC_ME_TESTMAP_FINAL_DOOM;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'GEC Master Edition (Beta 4)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_Beta4(GameConstants& consts) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
    consts.mainWads[1] = CdFile::LEGALS_WAD;

    consts.mainWadLumpRemappers[1] = [](const WadLumpName& oldName) noexcept -> WadLumpName {
        // Need to access this palette in 'LEGALS.WAD' separately to 'PLAYPAL' in 'PSXDOOM.WAD' for decoding intro logos
        return (oldName == "PLAYPAL") ? "GECINPAL" : oldName;
    };

    consts.introMovies[0] = "PSXDOOM/ABIN/MOVIE_N.STR";
    consts.introMovies[1] = "PSXDOOM/ABIN/GEC_N.STR";
    consts.introMovies[2] = "PSXDOOM/ABIN/DWORLD_N.STR";
    consts.SetNumDemos_GecMe_Beta4OrLater(7);               // Assume '7' until we read the GEC MAPINFO field for this
    consts.saveFilePrefix = "GecMe_";
    consts.pLastPasswordField = &PlayerPrefs::gLastPassword_GecMe;
    consts.pExtraPalettes = BuiltInPaletteData::GEC_ME_DYNAMIC_PALETTE_PLACEHOLDERS;
    consts.numExtraPalettes = BuiltInPaletteData::NUM_GEC_ME_DYNAMIC_PALETTE_PLACEHOLDERS;
    consts.numPalettesRequired = 31;
    consts.netGameId = NET_GAMEID_GEC_ME_BETA4;
    consts.baseNumAnims = 0;                        // We don't use built-in hardcoded anims for this mod (they're loaded via GEC MAPINFO instead)
    consts.texPalette_BUTTONS = 17;
    consts.bRemove_LOADING_progressBar = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the set of game constants for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
void GameConstants::populate(const GameType gameType, const bool bIsDemoVersion) noexcept {
    // Default init first
    *this = {};

    // Then populate all fields explicitly
    switch (gameType) {
        case GameType::Doom:                        populateConsts_Doom(*this, bIsDemoVersion);         break;
        case GameType::FinalDoom:                   populateConsts_FinalDoom(*this);                    break;
        case GameType::GEC_ME_Beta3:                populateConsts_GEC_ME_Beta3(*this);                 break;
        case GameType::GEC_ME_TestMap_Doom:         populateConsts_GEC_ME_TestMap_Doom(*this);          break;
        case GameType::GEC_ME_TestMap_FinalDoom:    populateConsts_GEC_ME_TestMap_FinalDoom(*this);     break;
        case GameType::GEC_ME_Beta4:                populateConsts_GEC_ME_Beta4(*this);                 break;

        default:
            FatalErrors::raise("GameConstants::populate(): unhandled game type!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the number of demos for 'GEC Master Edition' (Beta 4 or later).
// Populates the 'demos' field according to the specified number of demos.
//------------------------------------------------------------------------------------------------------------------------------------------
void GameConstants::SetNumDemos_GecMe_Beta4OrLater(const int32_t numDemos) noexcept {
    for (int32_t i = 0; i < (int) C_ARRAY_SIZE(demos); ++i) {
        // Default init this demo slot to begin with
        BuiltInDemoDef& demo = demos[i];
        demo = {};

        // Reached the end of the populated demo slots?
        if (i + 1 > numDemos)
            continue;

        // Populate this demo slot
        const bool bIsLastDemo = (i + 1 == numDemos);
        const bool bIsOnlyDemo = (numDemos == 1);

        std::snprintf(demo.filename.chars, C_ARRAY_SIZE(demo.filename.chars), "NDEMO%d.LMP", (int)(i + 1));
        demo.bFinalDoomDemo = true;
        demo.bPalDemo = false;
        demo.bShowCreditsAfter = ((!bIsLastDemo) || bIsOnlyDemo);
    }
}
