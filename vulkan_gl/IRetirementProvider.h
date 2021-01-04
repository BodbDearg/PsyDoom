#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Interface for an object which provides resource retirement services externally to the retirement manager.
// Can be used to trigger the deferred destruction of various external things upon completion of a frame.
// Preforms its retirement during the same trigger as the retirement manager itself.
//------------------------------------------------------------------------------------------------------------------------------------------
class IRetirementProvider {
public:
    virtual ~IRetirementProvider() noexcept {}

    // Free all retired resources for all ringbuffer slots
    virtual void freeRetiredResourcesForAllRingbufferSlots() noexcept = 0;

    // Free retired resources for a particular ringbuffer slot index
    virtual void freeRetiredResourcesForRingbufferIndex(const uint8_t ringbufferIndex) noexcept = 0;
};

END_NAMESPACE(vgl)
