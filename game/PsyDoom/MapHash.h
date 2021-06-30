#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>

BEGIN_NAMESPACE(MapHash)

extern int32_t      gDataSize;
extern uint64_t     gWord1;
extern uint64_t     gWord2;

void clear() noexcept;
void addData(const void* const pData, const int32_t dataSize) noexcept;
void finalize() noexcept;

END_NAMESPACE(MapHash)
