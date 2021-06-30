#include "cdmaptbl.h"

#if PSYDOOM_MODS

#include "PsyDoom/Game.h"
#include "PsyDoom/IsoFileSys.h"
#include "PsyDoom/PsxVm.h"

#include <vector>

// The id of a filename on disc (up to 16 characters) and where it starts on the CD-ROM image
struct DiscFile {
    String16            name;
    PsxCd_MapTblEntry   info;
};

// The start sector index and size of each file on the game's CD-ROM.
//
// These values were originally harcoded but are now populated from the disc's actual ISO 9960 filesystem, which allows flexibility for
// modding and different game versions. For the original harcoded values, refer to these tables in the 'Old' folder.
//
// Note for purists:
// The start sector here DOES NOT include the 150 sector 'lead-in/toc' track on the CD-ROM.
// Therefore you must add 150 sectors to this count to get the actual real physical sector number.
// This doesn't matter for us however since .cue/.bin pair CD-ROM images don't include this 150 sector TOC chunk.
// Therefore we can just specify the start sector without including that amount.
//
static std::vector<DiscFile> gDiscFiles;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the cd map table for the "PSX Doom Forever" ROM hack, which is a re-skin of Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
static void CdMapTbl_Init_PsxDoomForever() noexcept {
    // Rename the 'ZONE3D.WAD' file to 'PSXDOOM.WAD' so PsyDoom recognizes it.
    // Everything else can just stay the same.
    const CdFileId ZONE3D_WAD = "ZONE3D.WAD";

    for (DiscFile& discFile : gDiscFiles) {
        if (discFile.name == ZONE3D_WAD) {
            discFile.name = CdFile::PSXDOOM_WAD;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the cd map table and figure out where each file is on disc and file sizes
//------------------------------------------------------------------------------------------------------------------------------------------
void CdMapTbl_Init() noexcept {
    // Build up the list of filenames on the game disc.
    // Note that we completely ignore the folder structure and flatten the disc.
    gDiscFiles.clear();
    gDiscFiles.reserve(512);
    const IsoFileSys& fileSys = PsxVm::gIsoFileSys;

    for (const IsoFileSysEntry& fsEntry : fileSys.entries) {
        // Filter out directories or invalid names
        if (fsEntry.bIsDirectory || (fsEntry.nameLen <= 0))
            continue;

        DiscFile& discFile = gDiscFiles.emplace_back();
        discFile.name = fsEntry.name;
        discFile.info.startSector = fsEntry.startLba;
        discFile.info.size = fsEntry.size;
    }

    // Special case the PSX Doom Forever ROM Hack
    if (Game::gbIsPsxDoomForever) {
        CdMapTbl_Init_PsxDoomForever();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the location of the specified filename on disc, or a zero sized file if not found
//------------------------------------------------------------------------------------------------------------------------------------------
PsxCd_MapTblEntry CdMapTbl_GetEntry(const CdFileId fileId) noexcept {
    for (DiscFile& discFile : gDiscFiles) {
        if (discFile.name == fileId)
            return discFile.info;
    }

    return {};
}

#endif  // #if PSYDOOM_MODS
