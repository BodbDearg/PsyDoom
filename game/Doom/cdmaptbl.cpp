#include "cdmaptbl.h"

#if PSYDOOM_MODS

#include "FatalErrors.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/IsoFileSys.h"
#include "PsyDoom/PsxVm.h"

#include <cstdio>
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
// Manually adds a disc file to the list of disc files
//------------------------------------------------------------------------------------------------------------------------------------------
static void AddDiscFile(const CdFileId id, const char* const path) noexcept {
    const IsoFileSysEntry* const pFsEntry = PsxVm::gIsoFileSys.getEntry(path);

    if (!pFsEntry)
        FatalErrors::raiseF("Game disc is missing required file '%s'!", path);

    DiscFile& discFile = gDiscFiles.emplace_back();
    discFile.name = id;
    discFile.info.startSector = pFsEntry->startLba;
    discFile.info.size = pFsEntry->size;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the cd map table for [GEC] Master Edition PSX Doom for the PlayStation (Beta 3).
// This needs to be done manually due to conflicting file names.
//------------------------------------------------------------------------------------------------------------------------------------------
static void CdMapTbl_Init_GEC_ME_Beta3() noexcept {
    // Buffers to use to make up temporary strings
    char srcFileName[256];
    char dstFileName[32];

    // Add the main resource wads
    AddDiscFile("MEDOOM.WAD",   "DATA/MEDOOM.WAD");
    AddDiscFile("PSXDOOM.WAD",  "PSXDOOM/ABIN/PSXDOOM.WAD");
    AddDiscFile("PSXFINAL.WAD", "PSXDOOM/ABIN/PSXFINAL.WAD");

    // Define demo lumps
    const auto defineDemoLump = [&](const int32_t srcNum, const int32_t dstNum) noexcept {
        std::snprintf(srcFileName, C_ARRAY_SIZE(srcFileName), "PSXDOOM/ABIN/DEMO%d.LMP", srcNum);
        std::snprintf(dstFileName, C_ARRAY_SIZE(dstFileName), "DEMO%d.LMP", dstNum);
        AddDiscFile(dstFileName, srcFileName);
    };

    for (int32_t lumpNum = 1; lumpNum <= 8; ++lumpNum) {
        defineDemoLump(lumpNum, lumpNum);
    }

    // Define maps
    const auto defineMap = [&](const char* const srcBaseDir, const char* const srcFmt, const int32_t srcNum, const int32_t dstNum) noexcept {
        const int32_t srcSubDirNum = (srcNum - 1) / 8;

        // Define the .WAD file
        std::snprintf(srcFileName, C_ARRAY_SIZE(srcFileName), "%s/MAPDIR%d/MAP%02d.%s", srcBaseDir, srcSubDirNum, srcNum, srcFmt);
        std::snprintf(dstFileName, C_ARRAY_SIZE(dstFileName), "MAP%02d.%s", dstNum, srcFmt);
        AddDiscFile(dstFileName, srcFileName);

        // Define the .LCD file
        std::snprintf(srcFileName, C_ARRAY_SIZE(srcFileName), "%s/SNDMAPS/MAP%02d.LCD", srcBaseDir, srcNum);
        std::snprintf(dstFileName, C_ARRAY_SIZE(dstFileName), "MAP%02d.LCD", dstNum);
        AddDiscFile(dstFileName, srcFileName);
    };

    for (int32_t srcMapNum = 1; srcMapNum <= 30; ++srcMapNum) {
        defineMap("PSXDOOM/MAPS/DOOM", "WAD", srcMapNum, srcMapNum);
    }

    for (int32_t srcMapNum = 1; srcMapNum <= 20; ++srcMapNum) {
        defineMap("PSXDOOM/MAPS/FINAL1", "ROM", srcMapNum, 30 + srcMapNum);
    }

    for (int32_t srcMapNum = 1; srcMapNum <= 20; ++srcMapNum) {
        defineMap("PSXDOOM/MAPS/FINAL2", "ROM", srcMapNum, 50 + srcMapNum);
    }

    for (int32_t srcMapNum = 1; srcMapNum <= 24; ++srcMapNum) {
        defineMap("PSXDOOM/MAPS/FINAL3", "ROM", srcMapNum, 70 + srcMapNum);
    }

    // Define music and common sfx
    AddDiscFile("DOOMSFX.LCD", "PSXDOOM/MUSIC/DOOMSFX.LCD");
    AddDiscFile("DOOMSND.WMD", "PSXDOOM/MUSIC/DOOMSNDF.WMD");

    for (int32_t musicNum = 1; musicNum <= 30; ++musicNum) {
        std::snprintf(srcFileName, C_ARRAY_SIZE(srcFileName), "PSXDOOM/MUSIC/MUSLEV%d.LCD", musicNum);
        std::snprintf(dstFileName, C_ARRAY_SIZE(dstFileName), "MUSLEV%d.LCD", musicNum);
        AddDiscFile(dstFileName, srcFileName);
    }
}

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
    // Reserve room for all the entries
    gDiscFiles.clear();
    gDiscFiles.reserve(512);

    // Special case the GEC Master Edition Beta 3
    if (Game::gGameType == GameType::GEC_ME_Beta3) {
        CdMapTbl_Init_GEC_ME_Beta3();
        return;
    }

    // Build up the list of filenames on the game disc.
    // Note that we completely ignore the folder structure and flatten the disc.
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
