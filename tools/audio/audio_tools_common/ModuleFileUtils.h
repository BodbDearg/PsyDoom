//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities for reading and writing to module files (.WMD files) and their human readable .json equivalents
//------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

#include <string>

namespace AudioTools {
    struct Module;

    namespace ModuleFileUtils {
        bool writeJsonFile(const char* const jsonFilePath, const Module& moduleIn, std::string& errorMsgOut) noexcept;
        bool readWmdFile(const char* const wmdFilePath, Module& moduleOut, std::string& errorMsgOut) noexcept;
        bool writeWmdFile(const char* const wmdFilePath, const Module& moduleIn, std::string& errorMsgOut) noexcept;
    }
}
