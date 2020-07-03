//------------------------------------------------------------------------------------------------------------------------------------------
// Various game related utility functions
//------------------------------------------------------------------------------------------------------------------------------------------
#include "GameUtils.h"

#include "FatalErrors.h"
#include "IsoFileSys.h"
#include "PsxVm.h"

BEGIN_NAMESPACE(GameUtils)

GameType        gGameType;
GameVariant     gGameVariant;

void determineGameTypeAndVariant() noexcept {
    if (PsxVm::gIsoFileSys.getEntry("SLUS_000.77")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::NTSC_U;
    } else if (PsxVm::gIsoFileSys.getEntry("SLPS_003.08")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::NTSC_J;
    } else if (PsxVm::gIsoFileSys.getEntry("SLES_001.32")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::PAL;
    } else if (PsxVm::gIsoFileSys.getEntry("SLUS_003.31")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::NTSC_U;
    } else if (PsxVm::gIsoFileSys.getEntry("SLPS_007.27")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::NTSC_J;
    } else if (PsxVm::gIsoFileSys.getEntry("SLES_004.87")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::PAL;
    } else {
        FatalErrors::raise(
            "Unknown/unrecognized PSX Doom game disc provided!\n"
            "The disc given must be either 'Doom' or 'Final Doom' (NTSC-U, NTSC-J or PAL version)."
        );
    }
}

END_NAMESPACE(GameUtils)
