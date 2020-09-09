#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(AudioUtils)

float getNoteSampleRate(const float baseNote, const float baseNoteSampleRate, const float note) noexcept;
double getNoteSampleRate(const double baseNote, const double baseNoteSampleRate, const double note) noexcept;

END_NAMESPACE(AudioUtils)
END_NAMESPACE(AudioTools)
