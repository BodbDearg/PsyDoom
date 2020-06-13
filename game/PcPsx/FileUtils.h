#pragma once

#include "Macros.h"

#include <cstddef>
#include <memory>

BEGIN_NAMESPACE(FileUtils)

bool getContentsOfFile(
    const char* const filePath,
    std::byte*& pOutputMem,
    size_t& outputSize,
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

END_NAMESPACE(FileUtils)
