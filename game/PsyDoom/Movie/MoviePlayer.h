#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(movie)
BEGIN_NAMESPACE(MoviePlayer)

bool play(const char* const cdFilePath, const float fps) noexcept;
bool isPlaying() noexcept;

END_NAMESPACE(MoviePlayer)
END_NAMESPACE(movie)
