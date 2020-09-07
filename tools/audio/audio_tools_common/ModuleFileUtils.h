//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities for reading and writing to module files (.WMD files) and their human readable .json equivalents
//------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

#include "Macros.h"

#include <string>

BEGIN_NAMESPACE(AudioTools)

struct Module;

BEGIN_NAMESPACE(ModuleFileUtils)

bool readJsonFile(const char* const jsonFilePath, Module& moduleOut, std::string& errorMsgOut) noexcept;
bool writeJsonFile(const char* const jsonFilePath, const Module& moduleIn, std::string& errorMsgOut) noexcept;
bool readWmdFile(const char* const wmdFilePath, Module& moduleOut, std::string& errorMsgOut) noexcept;
bool writeWmdFile(const char* const wmdFilePath, const Module& moduleIn, std::string& errorMsgOut) noexcept;

END_NAMESPACE(ModuleFileUtils)
END_NAMESPACE(AudioTools)
