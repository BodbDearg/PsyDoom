#pragma once

#include "Macros.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class CmdBuffer;
class MutableTexture;
class RenderTexture;
class Texture;

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds and collects transfer commands which can be submitted later to a command buffer.
//
// Notes:
//  (1) You do not add commands directly to this object, instead you request via other objects (buffer, texture etc.)
//  (2) All buffers and objects referenced by the transfer commands are assumed to be valid until the transfer command
//      completes, so they should be retired via the retirement manager in the usual way.
//------------------------------------------------------------------------------------------------------------------------------------------
class TransferTask {
public:
    TransferTask() noexcept;
    ~TransferTask() noexcept;

    void reserveCmds(const size_t numCmds) noexcept;
    void clearCmds(const bool bCompactCmdList = true) noexcept;
    bool isEmpty() const noexcept;
    size_t getNumCmds() const noexcept;
    void submitToCmdBuffer(CmdBuffer& cmdBuffer) noexcept;

private:
    // Copy and move is disallowed
    TransferTask(const TransferTask& other) = delete;
    TransferTask(TransferTask&& other) = delete;
    TransferTask& operator = (const TransferTask& other) = delete;
    TransferTask& operator = (TransferTask&& other) = delete;
    
    // Only these classes have access
    friend class Buffer;
    friend class RenderTexture;
    friend class Texture;

    void addBufferCopy(
        const VkBuffer srcVkBuffer,
        const VkBuffer dstVkBuffer,
        const uint64_t srcOffset,
        const uint64_t dstOffset,
        const uint64_t numBytes
    ) noexcept;

    void addTextureUpload(const VkBuffer srcVkBuffer, const Texture& dstTexture) noexcept;
    void addRenderTextureDownload(RenderTexture& src, MutableTexture& dst) noexcept;

    // The list of transfer commands to execute
    struct TransferCmd;
    std::vector<TransferCmd> mCmds;
};

END_NAMESPACE(vgl)
