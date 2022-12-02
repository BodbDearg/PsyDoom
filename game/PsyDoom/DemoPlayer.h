#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(DemoPlayer)

bool onBeforeMapLoad() noexcept;
bool onAfterMapLoad() noexcept;
bool hasReachedDemoEnd() noexcept;
bool wasLevelCompleted() noexcept;
bool isUsingNewDemoFormat() noexcept;
bool isPlayingAClassicDemo() noexcept;
bool readTickInputs() noexcept;
void onPlaybackDone() noexcept;

END_NAMESPACE(DemoPlayer)
