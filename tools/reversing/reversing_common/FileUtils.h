#pragma once

#include <string>

namespace FileUtils {
    // Read the file at the given path into the given string and return false on failure
    bool readFileAsString(const char* const filePath, std::string& out) noexcept;
}
