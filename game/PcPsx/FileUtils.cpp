//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities for dealing with files
//------------------------------------------------------------------------------------------------------------------------------------------
#include "FileUtils.h"

#include "Finally.h"
#include <cstdint>
#include <cstring>

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
bool getContentsOfFile(
    const char* const pFilePath,
    std::byte*& pOutputMem,
    size_t& outputSize,
    const size_t numExtraBytes,
    const std::byte extraBytesValue
) noexcept {
    ASSERT(pFilePath);

    // Clear output firstly
    pOutputMem = nullptr;
    outputSize = 0;

    // Open the file firstly and ensure it will be closed on exit    
    FILE* pFile = std::fopen(pFilePath, "rb");

    if (!pFile) {
        return false;
    }

    auto closeFile = finally([&]() noexcept {
        std::fclose(pFile);
    });

    // Figure out what size it is and rewind back to the start
    if (std::fseek(pFile, 0, SEEK_END) != 0)
        return false;
    
    const long fileSize = std::ftell(pFile);

    if (fileSize <= 0 || fileSize >= INT32_MAX)
        return false;
    
    if (std::fseek(pFile, 0, SEEK_SET) != 0)
        return false;
    
    // Try to read the file contents
    std::byte* const pTmpBuffer = new std::byte[(size_t) fileSize + numExtraBytes];

    if (std::fread(pTmpBuffer, (uint32_t) fileSize, 1, pFile) != 1) {
        delete[] pTmpBuffer;
        return false;
    }

    // Success! Set the value of the extra bytes (if specified)
    if (numExtraBytes > 0) {
        std::memset(pTmpBuffer + fileSize, (int) extraBytesValue, numExtraBytes);
    }

    // Save the result and return 'true' for success
    pOutputMem = pTmpBuffer;
    outputSize = (size_t) fileSize;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the specified block of bytes to the given file.
// Returns 'true' on success.
//------------------------------------------------------------------------------------------------------------------------------------------
bool writeDataToFile(
    const char* const pFilePath,
    const std::byte* const pData,
    const size_t dataSize,
    const bool bAppend
) noexcept {
    ASSERT(pFilePath);

    // Open the file firstly and ensure it will be closed on exit    
    FILE* pFile = std::fopen(pFilePath, bAppend ? "ab" : "wb");

    if (!pFile) {
        return false;
    }

    auto closeFile = finally([&]() noexcept {
        std::fclose(pFile);
    });    

    // Do the write and return the result
    return (std::fwrite(pData, dataSize, 1, pFile) == 1);
}

END_NAMESPACE(FileUtils)
