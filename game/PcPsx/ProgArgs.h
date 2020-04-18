#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(ProgArgs)

extern bool         gbUseHighFpsHack;
extern const char*  gDataDirPath;

void init(const int argc, const char** const argv) noexcept;
void shutdown() noexcept;

END_NAMESPACE(ProgArgs)
