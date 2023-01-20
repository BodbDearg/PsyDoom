#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: this module has been reworked for greater flexibility, to allow for map files outside of the 1-59 range and for values to be
// more easily defined in external data files via strings.
// 
// For the orginal version of this see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS

#include "SmallString.h"
#include "Wess/psxcd.h"

// Allow a file name on disc to be up to 16-characters.
// The original game disc had 8.3 format files so this is more than generous enough to allow for map numbers beyond '100'.
typedef String16 CdFileId;

// Some predefined ids of files on the game disc
namespace CdFile {
    constexpr CdFileId MOVIE_STR    = "MOVIE.STR";
    constexpr CdFileId PSXDOOM_WAD  = "PSXDOOM.WAD";
    constexpr CdFileId PSXFINAL_WAD = "PSXFINAL.WAD";   // GEC ME (Beta 3)
    constexpr CdFileId MEDOOM_WAD   = "MEDOOM.WAD";     // GEC ME (Beta 3)
    constexpr CdFileId LEGALS_WAD   = "LEGALS.WAD";     // GEC ME (Beta 4)
    constexpr CdFileId DOOMSFX_LCD  = "DOOMSFX.LCD";
    constexpr CdFileId DOOMSND_WMD  = "DOOMSND.WMD";
    constexpr CdFileId DEMO1_LMP    = "DEMO1.LMP";
    constexpr CdFileId DEMO2_LMP    = "DEMO2.LMP";
}

void CdMapTbl_Init() noexcept;
PsxCd_MapTblEntry CdMapTbl_GetEntry(const CdFileId fileId) noexcept;

#endif  // #if PSYDOOM_MODS
