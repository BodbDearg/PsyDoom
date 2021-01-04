#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class Semaphore;

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a condition that a command buffer must wait on before it is allowed to execute.
// Holds a semaphore to wait on (until signalled) and the pipeline stages that are blocked waiting for that semaphore to be signalled.
//------------------------------------------------------------------------------------------------------------------------------------------
struct CmdBufferWaitCond {
    const Semaphore*        pSemaphore;             // The semaphore that is waited on
    VkPipelineStageFlags    blockedStageFlags;      // Which pipeline stages are blocked waiting for the semaphore

    inline constexpr CmdBufferWaitCond() noexcept
        : pSemaphore(nullptr)
        , blockedStageFlags()
    {
    }

    inline constexpr CmdBufferWaitCond(const Semaphore* const pSemaphore, const VkPipelineStageFlags blockedStageFlags) noexcept
        : pSemaphore(pSemaphore)
        , blockedStageFlags(blockedStageFlags)
    {
    }
};

END_NAMESPACE(vgl)
