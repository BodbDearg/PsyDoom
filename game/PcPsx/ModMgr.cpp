//------------------------------------------------------------------------------------------------------------------------------------------
// Mod manager: provides modding functionality for PSX DOOM
//------------------------------------------------------------------------------------------------------------------------------------------
#include "ModMgr.h"

#include "Asserts.h"
#include "FileUtils.h"
#include "IsoFileSys.h"
#include "ProgArgs.h"
#include "PsxVm.h"
#include "WadList.h"

#include <algorithm>
#include <functional>
#include <map>
#include <unordered_set>
#include <vector>

// MacOS: some POSIX stuff needed due to <filesystem> workaround
#if __APPLE__
    #include <dirent.h>
#else
    #include <filesystem>
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides a hash for a CD file ID, required so we can put it in a set
//------------------------------------------------------------------------------------------------------------------------------------------
template<> struct std::hash<CdFileId> {
    inline std::size_t operator()(const CdFileId& fileId) const noexcept {
        return fileId.words[0] ^ fileId.words[1];
    }
};

BEGIN_NAMESPACE(ModMgr)

// Maximum number of files that can be open at once by the mod manager
static constexpr uint8_t MAX_OPEN_FILES = 16;

// A set of filenames in the game that are overriden by a file in the user specified 'datadir'.
// The names in this set are uppercased for case insensitive comparison.
static std::unordered_set<CdFileId> gOverridenFileNames;

// A list of currently open files.
// Only a certain amount are allowed at a time:
static std::FILE* gOpenFileSlots[MAX_OPEN_FILES] = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the path to a file in the data dir.
// Assumes that the data dir has actually specified by the user.
//------------------------------------------------------------------------------------------------------------------------------------------
static std::string getDataDirFilePath(const char* const fileName) noexcept {
    std::string filePath;
    filePath.reserve(512);
    filePath += ProgArgs::gDataDirPath;
    
    if ((filePath.back() != '\\') && (filePath.back() != '/')) {
        filePath += '/';
    }

    filePath += fileName;
    return filePath;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Uppercases the specified string
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeUppercase(char* const pStr, const size_t len) noexcept {
    std::transform(pStr, pStr + len, pStr, [](char c) noexcept { return (char) ::toupper(c); });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines what files in the game are overriden with files in a user supplied 'data directory', if any.
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineFileOverridesInUserDataDir() noexcept {
    // If there is no data dir then there are no overrides
    gOverridenFileNames.clear();

    if (!ProgArgs::gDataDirPath[0])
        return;

    // Prealloc memory for the overriden filenames set
    const IsoFileSys& fileSys = PsxVm::gIsoFileSys;
    gOverridenFileNames.reserve(fileSys.entries.size() * 8);

    // MacOS: the C++ 17 '<filesystem>' header requires MacOS Catalina as a minimum target.
    // That's a bit too much for now, so use standard POSIX stuff instead as a workaround.
    // Eventually however this code path can be removed...
    #if __APPLE__
        do {
            DIR* const pDir = opendir(ProgArgs::gDataDirPath);

            if (!pDir) {
                FatalErrors::raiseF("Failed to search the given data/file overrides directory '%s'! Does this directory exist?", ProgArgs::gDataDirPath);
                break;
            }

            errno = 0;

            while (dirent* const pDirEnt = readdir(pDir)) {
                // Mark this file as overridden. Note that we don't bother checking if the file originally existed on the game disc since this
                // allows us to add new files to variants of the game that might not have originally had them. An example of this would be
                // allowing 'MAP01.WAD' (Doom format map) to override 'MAP01.ROM' (Final Doom format map) when the Final Doom game is loaded.
                // This functionality is desirable since the Doom format is more modding friendly and doesn't contain baked-in texture numbers.
                CdFileId fileId = pDirEnt->d_name;
                makeUppercase(fileId.chars, CdFileId::MAX_LEN);
                gOverridenFileNames.insert(fileId);
            }

            closedir(pDir);

            if (errno) {
                FatalErrors::raiseF("Failed to search the given data/file overrides directory '%s'! Does this directory exist?", ProgArgs::gDataDirPath);
            }
        } while (false);
    #else
        // Next search the data dir for overrides.
        // Iterate through all files and try to match them with game files:
        try {
            typedef std::filesystem::directory_iterator DirIter;

            DirIter dirIter(ProgArgs::gDataDirPath);
            const DirIter dirEndIter;

            while (dirIter != dirEndIter) {
                // Mark this file as overridden. Note that we don't bother checking if the file originally existed on the game disc since this
                // allows us to add new files to variants of the game that might not have originally had them. An example of this would be
                // allowing 'MAP01.WAD' (Doom format map) to override 'MAP01.ROM' (Final Doom format map) when the Final Doom game is loaded.
                // This functionality is desirable since the Doom format is more modding friendly and doesn't contain baked-in texture numbers.
                CdFileId fileId = dirIter->path().filename().u8string().c_str();
                makeUppercase(fileId.chars, CdFileId::MAX_LEN);
                gOverridenFileNames.insert(fileId);

                ++dirIter;
            }
        }
        catch (...) {
            FatalErrors::raiseF("Failed to search the given data/file overrides directory '%s'! Does this directory exist?", ProgArgs::gDataDirPath);
        }
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// The index of a free open file slot, or fail with a fatal error if there are no more slots available
//------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t findFreeOpenFileSlotIndex() noexcept {
    for (uint8_t i = 0; i < MAX_OPEN_FILES; ++i) {
        if (!gOpenFileSlots[i])
            return i;
    }

    FatalErrors::raise("findFreeOpenFileSlotIndex: out of available open file slots, too many files open!");
    return UINT8_MAX;
}

#if ASSERTS_ENABLED
//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if the given file is validly open through the user file overrides system
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isValidOverridenFile(const PsxCd_File& file) noexcept {
    return (
        (file.overrideFileHandle > 0) &&
        (file.overrideFileHandle <= MAX_OPEN_FILES) &&
        (gOpenFileSlots[file.overrideFileHandle - 1] != nullptr)
    );
}
#endif  // #if ASSERTS_ENABLED

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the path to an overriden file
//------------------------------------------------------------------------------------------------------------------------------------------
static std::string getOverridenFilePath(const CdFileId discFile) noexcept {
    std::string filePath;
    filePath.reserve(255);
    filePath = ProgArgs::gDataDirPath;
    filePath.push_back('/');
    filePath += discFile.c_str().data();

    return filePath;
}

void init() noexcept {
    determineFileOverridesInUserDataDir();
}

void shutdown() noexcept {
    // Close all open files
    for (std::FILE*& pFile : gOpenFileSlots) {
        if (pFile) {
            std::fclose(pFile);
            pFile = nullptr;
        }
    }

    // Clear all overrides
    gOverridenFileNames.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds the 'PSXDOOM_EXT.WAD' extension IWAD in the user data dir plus other main IWADs specified by the '-file' command to the wad list.
// Note that unlike PC the IWADs are NOT used to load map data and any maps inside them will be ignored.
// Instead, map data must be packaged in special 'map' WADS like 'MAP01.WAD' which are enabled by pointing the game to a mod directory
// via the '-datadir' command line argument.
//------------------------------------------------------------------------------------------------------------------------------------------
void addUserWads(WadList& wadList) noexcept {
    // Add wads specified from the command line first, as they are higher precedence
    ProgArgs::addWadArgsToList(wadList);

    // Load certain special wads from the user data dir (if specified) and if the files exist
    if (ProgArgs::gDataDirPath[0]) {
        // Add 'PSXDOOM_EXT.WAD' if found: this WAD enables user mods to add new resources to the game and expand upon the original 'PSXDOOM.WAD'
        std::string extWadPath = getDataDirFilePath("PSXDOOM_EXT.WAD");

        if (FileUtils::fileExists(extWadPath.c_str())) {
            wadList.add(extWadPath.c_str());
        }

        // Add 'PSX_MISSING_ENEMIES.WAD' if found: this does the same thing as 'PSXDOOM_EXT.WAD' but with lower precedence.
        // It's intended to be redistributable WAD containing a few missing enemies from PC Doom II.
        std::string missingEnemiesWadPath = getDataDirFilePath("PSX_MISSING_ENEMIES.WAD");

        if (FileUtils::fileExists(missingEnemiesWadPath.c_str())) {
            wadList.add(missingEnemiesWadPath.c_str());
        }
    }
}

bool areOverridesAvailableForFile(const CdFileId discFile) noexcept {
    CdFileId ucaseDiscFile = discFile;
    makeUppercase(ucaseDiscFile.chars, CdFileId::MAX_LEN);
    return (gOverridenFileNames.count(ucaseDiscFile) > 0);
}

bool isFileOverriden(const PsxCd_File& file) noexcept {
    return (file.overrideFileHandle != 0);
}

bool openOverridenFile(const CdFileId discFile, PsxCd_File& fileOut) noexcept {
    // Grab a free open file slot index
    const uint8_t fileSlotIdx = findFreeOpenFileSlotIndex();

    // Figure out the path for the file
    if (discFile == CdFileId{}) {
        FatalErrors::raise("ModMgr::openOverridenFile: invalid file specified!");
    }

    std::string filePath = getOverridenFilePath(discFile);

    // Open the file and save it in the file slot index
    std::FILE* const pFile = std::fopen(filePath.c_str(), "rb");

    if (!pFile) {
        return false;
    }

    // Figure out the size of the file:
    if (std::fseek(pFile, 0, SEEK_END) != 0) {
        std::fclose(pFile);
        return false;
    }

    const int32_t fileSize = (int32_t) std::ftell(pFile);

    if (fileSize < 0) {
        std::fclose(pFile);
        return false;
    }

    // Seek to the start of the file by default
    if (std::fseek(pFile, 0, SEEK_SET) != 0) {
        std::fclose(pFile);
        return false;
    }

    // Save this in the open file slot
    gOpenFileSlots[fileSlotIdx] = pFile;

    // Save file details and return 'true' for success
    fileOut = {};
    fileOut.overrideFileHandle = fileSlotIdx + 1;   // Note: handle is the index + 1
    fileOut.size = fileSize;
    return true;
}

void closeOverridenFile(PsxCd_File& file) noexcept {
    ASSERT(isValidOverridenFile(file));
    std::FILE* const pFile = gOpenFileSlots[file.overrideFileHandle - 1];
    std::fclose(pFile);
    gOpenFileSlots[file.overrideFileHandle - 1] = nullptr;
    file = {};
}

int32_t readFromOverridenFile(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept {
    ASSERT(isValidOverridenFile(file));
    std::FILE* const pFile = gOpenFileSlots[file.overrideFileHandle - 1];

    // Note: a read size of '0' always succeeds and negative sizes always result in an error
    if (numBytes > 0) {
        return (std::fread(pDest, (size_t) numBytes, 1, pFile) == 1) ? numBytes : -1;
    } else if (numBytes == 0) {
        return 0;
    } else {
        return -1;
    }
}

int32_t seekForOverridenFile(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept {
    ASSERT(isValidOverridenFile(file));
    std::FILE* const pFile = gOpenFileSlots[file.overrideFileHandle - 1];

    if (mode == PsxCd_SeekMode::SET) {
        return (std::fseek(pFile, offset, SEEK_SET) == 0) ? 0 : -1;
    } else if (mode == PsxCd_SeekMode::CUR) {
        return (std::fseek(pFile, offset, SEEK_CUR) == 0) ? 0 : -1;
    } else if (mode == PsxCd_SeekMode::END) {
        return (std::fseek(pFile, -offset, SEEK_END) == 0) ? 0 : -1;
    } else {
        return -1;  // Bad seek mode!
    }
}

int32_t tellForOverridenFile(const PsxCd_File& file) noexcept {
    ASSERT(isValidOverridenFile(file));
    std::FILE* const pFile = gOpenFileSlots[file.overrideFileHandle - 1];
    return (int32_t) std::ftell(pFile);
}

int32_t getOverridenFileSize(const CdFileId discFile) noexcept {
    if (discFile == CdFileId{}) {
        FatalErrors::raise("ModMgr::getOverridenFileSize: invalid file specified!");
    }

    std::string filePath = getOverridenFilePath(discFile);
    return (int32_t) FileUtils::getFileSize(filePath.c_str());
}

END_NAMESPACE(ModMgr)
