#pragma once

#include "Macros.h"

#include <cstdint>

namespace Launcher {
    struct Tab_Launcher;
}

BEGIN_NAMESPACE(LauncherPrefs)

using namespace Launcher;

void load(Tab_Launcher& tab) noexcept;
void save(Tab_Launcher& tab) noexcept;
bool shouldSave(Tab_Launcher& tab) noexcept;

END_NAMESPACE(LauncherPrefs)
