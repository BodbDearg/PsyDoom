#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(DemoResult)

bool saveToJsonFile(const char* const jsonFilePath) noexcept;
bool verifyMatchesJsonFileResult(const char* const jsonFilePath) noexcept;

END_NAMESPACE(DemoResult)
