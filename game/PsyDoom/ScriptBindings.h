#pragma once

#include "Macros.h"

namespace sol {
    class state;
}

BEGIN_NAMESPACE(ScriptBindings)

void registerAll(sol::state& lua) noexcept;

END_NAMESPACE(ScriptBindings)
