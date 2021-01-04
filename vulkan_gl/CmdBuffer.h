#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class CmdPool;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents an individual Vulkan command buffer that is used to record rendering commands.
// This structure is allocated from a command pool and the command pool is expected to be valid for the lifetime of the command buffer.
// The command buffer can either be a primary or secondary command buffer; secondary buffers are nested inside primary command buffers.
//------------------------------------------------------------------------------------------------------------------------------------------
class CmdBuffer {
public:
    CmdBuffer() noexcept;
    CmdBuffer(CmdBuffer&& other) noexcept;
    ~CmdBuffer() noexcept;

    bool init(CmdPool& cmdPool, const VkCommandBufferLevel level) noexcept;
    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline VkCommandBufferLevel getLevel() const noexcept { return mLevel; }
    inline CmdPool* getCmdPool() const noexcept { return mpCmdPool; }
    inline VkCommandBuffer getVkCommandBuffer() const noexcept { return mVkCommandBuffer; }

private:
    // Copy construct and assignment disallowed
    CmdBuffer(const CmdBuffer& other) = delete;
    CmdBuffer& operator = (const CmdBuffer& other) = delete;
    CmdBuffer& operator = (CmdBuffer&& other) = delete;

    bool                    mbIsValid;
    VkCommandBufferLevel    mLevel;
    CmdPool*                mpCmdPool;
    VkCommandBuffer         mVkCommandBuffer;
};

END_NAMESPACE(vgl)
