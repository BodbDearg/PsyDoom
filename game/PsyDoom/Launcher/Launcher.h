#pragma once

#if PSYDOOM_LAUNCHER

#include "Macros.h"

#include <cstdint>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Launcher)

int launcherMain(const int argc, const char* const* const argv) noexcept;

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
