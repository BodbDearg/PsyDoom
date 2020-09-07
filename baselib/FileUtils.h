#pragma once

#include "Macros.h"

#include <cstddef>
#include <memory>
#include <string>

// Holds a reference to the data and size of a file
struct FileData {
    FileData() noexcept = default;
    FileData(const FileData& other) = delete;   // Only move is allowed: if you want a copy, you need to do manually
    FileData(FileData&& other) = default;

    std::unique_ptr<std::byte>  bytes;
    size_t                      size;
};

BEGIN_NAMESPACE(FileUtils)

FileData getContentsOfFile(
    const char* const filePath,
    const size_t numExtraBytes = 0,
    const std::byte extraBytesValue = std::byte(0)
) noexcept;

bool writeDataToFile(
    const char* const filePath,
    const std::byte* const pData,
    const size_t dataSize,
    const bool bAppend = false
) noexcept;

bool fileExists(const char* filePath) noexcept;
int64_t getFileSize(const char* filePath) noexcept;
void getParentPath(const char* path, std::string& parentPath) noexcept;

END_NAMESPACE(FileUtils)
