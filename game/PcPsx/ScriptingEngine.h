#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(ScriptingEngine)

void init() noexcept;
void shutdown() noexcept;
void doAction(const int32_t actionNum) noexcept;

END_NAMESPACE(ScriptingEngine)
