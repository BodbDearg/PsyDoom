#pragma once

#include "Macros.h"

#include <cstdint>

union String16;
typedef String16 CdFileId;

BEGIN_NAMESPACE(DevMapAutoReloader)

void init(const CdFileId mapWadFile) noexcept;
void shutdown() noexcept;
void update() noexcept;

END_NAMESPACE(DevMapAutoReloader)
