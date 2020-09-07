//------------------------------------------------------------------------------------------------------------------------------------------
// Mod manager: provides modding functionality for PSX DOOM
//------------------------------------------------------------------------------------------------------------------------------------------
#include "ModMgr.h"

#include "Asserts.h"
#include "FileUtils.h"
#include "ProgArgs.h"

#include <filesystem>
#include <map>
#include <vector>

// MacOS: some POSIX stuff needed due to <filesystem> workaround
#if __APPLE__
    #include <dirent.h>
#endif

BEGIN_NAMESPACE(ModMgr)

// Maximum number of files that can be open at once by the mod manager
static constexpr uint8_t MAX_OPEN_FILES = 16;

// A list of booleans indicating whether each file in the game can be overriden by a file in the user given 'datadir'.
// The list is indexed by a 'CdFileId' value.
static std::vector<bool> gbFileHasOverrides;

// A list of currently open files.
// Only a certain amount are allowed at a time:
static std::FILE* gOpenFileSlots[MAX_OPEN_FILES] = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines what files in the game are overriden with files in a user supplied 'data directory', if any.
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineFileOverridesInUserDataDir() noexcept {
    // If there is no data dir then there is no overrides
    gbFileHasOverrides.clear();
    gbFileHasOverrides.resize((uint32_t) CdFileId::END);

    if (!ProgArgs::gDataDirPath[0])
        return;
    
    // Build a map from filename to file index
    std::map<std::string, uint32_t> nameToFileIndex;

    for (uint32_t i = 0; i < (uint32_t) CdFileId::END; ++i) {
        nameToFileIndex[gCdMapTblFileNames[i]] = i;
    }

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
                // Try to match this file against game files and flag as an override if matching
                auto nameToFileIndexIter = nameToFileIndex.find(pDirEnt->d_name);
                
                if (nameToFileIndexIter != nameToFileIndex.end()) {
                    const uint32_t fileIdx = nameToFileIndexIter->second;
                    gbFileHasOverrides[fileIdx] = true;
                }
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
                // Try to match this file against game files and flag as an override if matching
                std::string filename = dirIter->path().filename().string();
                auto nameToFileIndexIter = nameToFileIndex.find(filename);

                if (nameToFileIndexIter != nameToFileIndex.end()) {
                    const uint32_t fileIdx = nameToFileIndexIter->second;
                    gbFileHasOverrides[fileIdx] = true;
                }

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
    const char* const filename = gCdMapTblFileNames[(uint32_t) discFile];

    std::string filePath;
    filePath.reserve(255);
    filePath = ProgArgs::gDataDirPath;
    filePath.push_back('/');
    filePath += filename;

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

    // Clear all data
    gbFileHasOverrides.clear();
    gbFileHasOverrides.shrink_to_fit();
}

bool areOverridesAvailableForFile(const CdFileId discFile) noexcept {
    const uint32_t fileIdx = (uint32_t) discFile;
    return (fileIdx < gbFileHasOverrides.size() && gbFileHasOverrides[fileIdx]);
}

bool isFileOverriden(const PsxCd_File& file) noexcept {
    return (file.overrideFileHandle != 0);
}

bool openOverridenFile(const CdFileId discFile, PsxCd_File& fileOut) noexcept {
    // Grab a free open file slot index
    const uint8_t fileSlotIdx = findFreeOpenFileSlotIndex();

    // Figure out the path for the file
    if (discFile >= CdFileId::END) {
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
    return (std::fread(pDest, (size_t) numBytes, 1, pFile) == 1) ? numBytes : -1;
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
    if (discFile >= CdFileId::END) {
        FatalErrors::raise("ModMgr::getOverridenFileSize: invalid file specified!");
    }

    std::string filePath = getOverridenFilePath(discFile);
    return (int32_t) FileUtils::getFileSize(filePath.c_str());
}

END_NAMESPACE(ModMgr)
