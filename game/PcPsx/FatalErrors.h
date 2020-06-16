#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(FatalErrors)

[[noreturn]] void outOfMemory() noexcept;
[[noreturn]] void raise(const char* const pMsgStr) noexcept;
[[noreturn]] void raiseF(const char* const pMsgFormatStr, ...) noexcept;

END_NAMESPACE(FatalErrors)
