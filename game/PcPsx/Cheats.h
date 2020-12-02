#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(Cheats)

extern bool gbJustExecutedACheat;

void init() noexcept;
void shutdown() noexcept;
void update() noexcept;

END_NAMESPACE(Cheats)
