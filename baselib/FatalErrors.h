#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(FatalErrors)

extern void (*gFatalErrorHandler)(const char* const msg) noexcept;

[[noreturn]] void outOfMemory() noexcept;
[[noreturn]] void raise(const char* const msgStr) noexcept;
[[noreturn]] void raiseF(const char* const msgFormatStr, ...) noexcept;

END_NAMESPACE(FatalErrors)
