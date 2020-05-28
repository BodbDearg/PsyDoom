//------------------------------------------------------------------------------------------------------------------------------------------
// Mod manager: provides modding functionality for PSX DOOM
//------------------------------------------------------------------------------------------------------------------------------------------
#include "ModMgr.h"

#include "ProgArgs.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <map>
#include <vector>

BEGIN_NAMESPACE(ModMgr)

// Maximum number of files that can be open at once by the mod manager
static constexpr uint8_t MAX_OPEN_FILES = 16;

// A list of booleans indicating whether each file in the game can be overriden by a file in the user given 'datadir'.
// The list is indexed by a 'CdMapTbl_File' value.
static std::vector<bool> gbFileHasOverrides;

// A list of currently open files.
// Only a certain amount are allowed at a time:
static std::FILE* gOpenFileSlots[MAX_OPEN_FILES] = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines what files in the game are overriden with files in a user supplied 'data directory', if any.
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineFileOverridesInUserDataDir() noexcept {
// FIXME: get working with MacOS 10.14
#ifdef WIN32
    // If there is no data dir then there is no overrides
    gbFileHasOverrides.clear();
    gbFileHasOverrides.resize((uint32_t) CdMapTbl_File::END);

    if (!ProgArgs::gDataDirPath[0])
        return;
    
    // Build a map from filename to file index
    std::map<std::string, uint32_t> nameToFileIndex;

    for (uint32_t i = 0; i < (uint32_t) CdMapTbl_File::END; ++i) {
        nameToFileIndex[CD_MAP_FILENAMES[i]] = i;
    }

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
        FATAL_ERROR_F("Failed to search the given data/file overrides directory '%s'! Does this directory exist?", ProgArgs::gDataDirPath);
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

    FATAL_ERROR("findFreeOpenFileSlotIndex: out of available open file slots, too many files open!");
    return UINT8_MAX;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if the given file is validly open through the user file overrides system
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isValidOverridenFile(const PsxCd_File& file) noexcept {
    return (
        (file.file.pos.track == 255) &&
        (file.file.pos.minute < MAX_OPEN_FILES) &&
        (gOpenFileSlots[file.file.pos.minute] != nullptr)
    );
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

bool areOverridesAvailableForFile(const CdMapTbl_File discFile) noexcept {
    const uint32_t fileIdx = (uint32_t) discFile;
    return (fileIdx < gbFileHasOverrides.size() && gbFileHasOverrides[fileIdx]);
}

bool isFileOverriden(const PsxCd_File& file) noexcept {
    // The opened file is overriden if the track is set to 255.
    // This is how we mark the file as overriden.
    return (file.file.pos.track == 255);
}

bool openOverridenFile(const CdMapTbl_File discFile, PsxCd_File& fileOut) noexcept {
    // Grab a free open file slot index
    const uint8_t fileSlotIdx = findFreeOpenFileSlotIndex();

    // Figure out the path for the file
    if (discFile >= CdMapTbl_File::END) {
        FATAL_ERROR("ModMgr::openOverridenFile: invalid file specified!");
    }

    const char* const filename = CD_MAP_FILENAMES[(uint32_t) discFile];

    std::string filePath = ProgArgs::gDataDirPath;
    filePath.push_back('/');
    filePath += filename;
    
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
    fileOut.file.pos.minute = fileSlotIdx;      // Save the index of the open file in this field
    fileOut.file.pos.track = 255u;              // Special marker to indicate that the file is overriden
    fileOut.file.size = fileSize;

    std::strncpy(fileOut.file.name, filename, C_ARRAY_SIZE(fileOut.file.name) - 1);
    fileOut.file.name[C_ARRAY_SIZE(fileOut.file.name) - 1] = 0;

    return true;
}

void closeOverridenFile(PsxCd_File& file) noexcept {
    ASSERT(isValidOverridenFile(file));
    std::FILE* const pFile = gOpenFileSlots[file.file.pos.minute];
    std::fclose(pFile);
    gOpenFileSlots[file.file.pos.minute] = nullptr;
    file = {};
}

int32_t readFromOverridenFile(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept {
    ASSERT(isValidOverridenFile(file));
    std::FILE* const pFile = gOpenFileSlots[file.file.pos.minute];
    return (std::fread(pDest, (size_t) numBytes, 1, pFile) == 1) ? numBytes : -1;
}

int32_t seekForOverridenFile(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept {
    ASSERT(isValidOverridenFile(file));
    std::FILE* const pFile = gOpenFileSlots[file.file.pos.minute];
    
    if (mode == PsxCd_SeekMode::SET) {
        return (std::fseek(pFile, offset, SEEK_SET) == 0) ? 0 : -1;
    } else if (mode == PsxCd_SeekMode::CUR) {
        return (std::fseek(pFile, offset, SEEK_CUR) == 0) ? 0 : -1;
    } else {
        return (std::fseek(pFile, -offset, SEEK_END) == 0) ? 0 : -1;
    }
}

int32_t tellForOverridenFile(const PsxCd_File& file) noexcept {
    ASSERT(isValidOverridenFile(file));
    std::FILE* const pFile = gOpenFileSlots[file.file.pos.minute];
    return (int32_t) std::ftell(pFile);
}

END_NAMESPACE(ModMgr)
