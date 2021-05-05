#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>

BEGIN_NAMESPACE(MapPatcher)

void applyPatches(const int32_t mapLumpIndex, const std::byte* const pLumpBytes, const int32_t lumpSize) noexcept;

END_NAMESPACE(MapPatcher)
