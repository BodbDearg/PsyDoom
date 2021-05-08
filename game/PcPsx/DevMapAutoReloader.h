#pragma once

#include "Macros.h"

#include <cstdint>

enum class CdFileId : int32_t;

BEGIN_NAMESPACE(DevMapAutoReloader)

void init(const CdFileId mapWadFile) noexcept;
void shutdown() noexcept;
void update() noexcept;

END_NAMESPACE(DevMapAutoReloader)
