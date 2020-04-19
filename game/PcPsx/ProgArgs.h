#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(ProgArgs)

extern bool         gbUseHighFpsHack;
extern const char*  gDataDirPath;
extern const char*  gPlayDemoFilePath;
extern const char*  gSaveDemoResultFilePath;
extern const char*  gCheckDemoResultFilePath;

void init(const int argc, const char** const argv) noexcept;
void shutdown() noexcept;

END_NAMESPACE(ProgArgs)
