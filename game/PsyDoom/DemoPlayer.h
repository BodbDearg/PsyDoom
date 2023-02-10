#pragma once

#include "Macros.h"

#include <cstdint>

// Which format a playing demo has
enum class DemoFormat : uint8_t {
    None,           // No demo is currently being played
    Classic,        // The original PSX DOOM demo format (note: this is slightly extended for 'Final Doom' with extra mouse functionality)
    PsyDoom,        // PsyDoom's extended demo format which supports higher precision gameplay and multiplayer
    GecMe           // An extended demo format used by the 'GEC Master Edition' (Beta 4 and later)
};

BEGIN_NAMESPACE(DemoPlayer)

bool onBeforeMapLoad() noexcept;
bool onAfterMapLoad() noexcept;
bool hasReachedDemoEnd() noexcept;
bool wasLevelCompleted() noexcept;
DemoFormat getPlayingDemoFormat() noexcept;
bool shouldOverrideMapMusicForDemo() noexcept;
bool isPlayerTurning30HzCapped() noexcept;
bool readTickInputs() noexcept;
void onPlaybackDone() noexcept;

END_NAMESPACE(DemoPlayer)
