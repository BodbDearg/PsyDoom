//
// Created by Dawid Drozd aka Gelldur on 18/10/16.
//

#pragma once

#include <memory>

#include "EventBus.h"
#include "TokenHolder.h"

namespace Dexode
{

// [[deprecated("Deprecating EventCollector. Try move to: TokenHolder<>")]]
using EventCollector = TokenHolder<EventBus>;

} // namespace Dexode
