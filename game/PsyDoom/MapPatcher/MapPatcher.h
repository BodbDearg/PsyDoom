#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>

BEGIN_NAMESPACE(MapPatcher)

void clearMapHash() noexcept;
void addToMapHash(const std::byte* const pMapData, const int32_t dataSize) noexcept;
void applyPatches() noexcept;

END_NAMESPACE(MapPatcher)
