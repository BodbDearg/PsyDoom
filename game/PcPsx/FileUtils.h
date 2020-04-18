#pragma once

#include "Macros.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <cstddef>
END_THIRD_PARTY_INCLUDES

BEGIN_NAMESPACE(FileUtils)

bool getContentsOfFile(
    const char* const pFilePath,
    std::byte*& pOutputMem,
    size_t& outputSize,
    const size_t numExtraBytes = 0,
    const std::byte extraBytesValue = std::byte(0)
) noexcept;

bool writeDataToFile(
    const char* const pFilePath,
    const std::byte* const pData,
    const size_t dataSize,
    const bool bAppend = false
) noexcept;

END_NAMESPACE(FileUtils)
