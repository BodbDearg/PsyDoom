//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities for dealing with files
//------------------------------------------------------------------------------------------------------------------------------------------
#include "FileUtils.h"

#include "Asserts.h"
#include "Finally.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

// MacOS: working around missing support for <filesystem> in everything except the latest bleeding edge OS and Xcode.
// Use standard Unix file functions instead for now, but some day this can be removed.
#ifdef __APPLE__
    #include <sys/stat.h>
    #include <unistd.h>
#else
    #include <filesystem>
#endif

BEGIN_NAMESPACE(FileUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read data for the given file path on disk and store it in the given pointer, returning 'true' on success.
// Optionally an additional number of bytes at the end of the data can be allocated and set to the given value.
// This can be useful to null terminate a text file that has been read, for example.
//
// Notes:
//  (1) If the file is not read successfully, the given filepath will be set to null and output size set to '0'.
//  (2) The output memory is allocated with C++ 'new[]' so should be deallocated with C++ 'delete[]'.
//  (3) The output size does NOT include the extra bytes added.
//------------------------------------------------------------------------------------------------------------------------------------------
FileData getContentsOfFile(
    const char* const filePath,
    const size_t numExtraBytes,
    const std::byte extraBytesValue
) noexcept {
    // Open the file firstly and ensure it will be closed on exit
    ASSERT(filePath);
    FILE* pFile = std::fopen(filePath, "rb");

    if (!pFile)
        return FileData();

    auto closeFile = finally([&]() noexcept {
        std::fclose(pFile);
    });

    // Figure out what size it is and rewind back to the start
    if (std::fseek(pFile, 0, SEEK_END) != 0)
        return FileData();
    
    const long fileSize = std::ftell(pFile);

    if (fileSize <= 0 || fileSize >= INT32_MAX)
        return FileData();
    
    if (std::fseek(pFile, 0, SEEK_SET) != 0)
        return FileData();
    
    // Try to read the file contents
    std::byte* const pTmpBuffer = new std::byte[(size_t) fileSize + numExtraBytes];

    if (std::fread(pTmpBuffer, (uint32_t) fileSize, 1, pFile) != 1) {
        delete[] pTmpBuffer;
        return FileData();
    }

    // Success! Set the value of the extra bytes (if specified)
    if (numExtraBytes > 0) {
        std::memset(pTmpBuffer + fileSize, (int) extraBytesValue, numExtraBytes);
    }

    // Save the result and return 'true' for success
    FileData fileData;
    fileData.bytes.reset(pTmpBuffer);
    fileData.size = fileSize;
    return fileData;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the specified block of bytes to the given file.
// Returns 'true' on success.
//------------------------------------------------------------------------------------------------------------------------------------------
bool writeDataToFile(
    const char* const filePath,
    const std::byte* const pData,
    const size_t dataSize,
    const bool bAppend
) noexcept {
    ASSERT(filePath);

    // Open the file firstly and ensure it will be closed on exit
    FILE* pFile = std::fopen(filePath, bAppend ? "ab" : "wb");

    if (!pFile) {
        return false;
    }

    auto closeFile = finally([&]() noexcept {
        std::fclose(pFile);
    });

    // Do the write and return the result
    return (std::fwrite(pData, dataSize, 1, pFile) == 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given file exists; if there is an error determining returns 'false'
//------------------------------------------------------------------------------------------------------------------------------------------
bool fileExists(const char* filePath) noexcept {
    ASSERT(filePath);

    try {
        // MacOS: working around missing support for <filesystem> in everything except the latest bleeding edge OS and Xcode.
        // Use standard Unix file functions instead for now, but some day this can be removed.
        #ifdef __APPLE__
            return (access(filePath, R_OK) == 0);
        #else
            return std::filesystem::exists(filePath);
        #endif
    } catch (...) {
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the size of the given file and return '-1' if there is an error
//------------------------------------------------------------------------------------------------------------------------------------------
int64_t getFileSize(const char* filePath) noexcept {
    ASSERT(filePath);

    try {
        // MacOS: working around missing support for <filesystem> in everything except the latest bleeding edge OS and Xcode.
        // Use standard Unix file functions instead for now, but some day this can be removed.
        #ifdef __APPLE__
            struct stat fileInfo;
            
            if (stat(filePath, &fileInfo) < 0)
                return -1;

            return (fileInfo.st_size >= 0) ? (int64_t) fileInfo.st_size : (int64_t) -1;
        #else
            return (int64_t) std::filesystem::file_size(filePath);
        #endif
    } catch (...) {
        return -1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Given a file or folder path, return the parent folder path or an empty string if there is no parent.
// Accepts POSIX or Windows style separators (forward or backward slash) in the path only.
// Leaves a trailing path separator in the returned path if there is a parent folder, so a file name can be easily appended later.
//------------------------------------------------------------------------------------------------------------------------------------------
void getParentPath(const char* const path, std::string& parentPath) noexcept {
    ASSERT(path);
    parentPath = path;

    while (!parentPath.empty()) {
        const char c = parentPath.back();

        if (c == '\\' || c == '/')
            break;

        parentPath.pop_back();
    }
}

END_NAMESPACE(FileUtils)
