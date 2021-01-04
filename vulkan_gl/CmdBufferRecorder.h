#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class BaseRenderPass;
class Buffer;
class CmdBuffer;
class DescriptorSet;
class Framebuffer;
class Pipeline;
class PipelineLayout;
struct VkFuncs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper that handles the recording of commands for a command buffer.
// Note that it is an error for this to be destroyed without explicitly ending the recording!
//------------------------------------------------------------------------------------------------------------------------------------------
class CmdBufferRecorder {
public:
    CmdBufferRecorder(const VkFuncs& vkFuncs) noexcept;
    ~CmdBufferRecorder() noexcept;

    bool beginPrimaryCmdBuffer(CmdBuffer& cmdBuffer, const VkCommandBufferUsageFlags usageFlags) noexcept;

    bool beginSecondaryCmdBuffer(
        CmdBuffer& cmdBuffer,
        const VkCommandBufferUsageFlags usageFlags,
        const VkRenderPass containingRenderPass,
        const uint32_t containingSubpass,
        const VkFramebuffer targetFramebuffer
    ) noexcept;

    bool endCmdBuffer() noexcept;
    bool isRecording() const noexcept;
    inline VkCommandBuffer getVkCommandBuffer() const noexcept { return mVkCommandBuffer; }

    //-----------------------------------------------------------------------------------------------------------------
    // ~~ Recorded commands ~~
    //-----------------------------------------------------------------------------------------------------------------    
    void beginRenderPass(
        const BaseRenderPass& renderPass,
        const Framebuffer& framebuffer,
        const VkSubpassContents subpassContents,
        const int32_t renderAreaX,
        const int32_t renderAreaY,
        const uint32_t renderAreaW,
        const uint32_t renderAreaH,
        const VkClearValue* const pClearValues,
        const uint32_t numClearValues
    ) noexcept;

    void nextSubpass(const VkSubpassContents nextSubpassContents) noexcept;
    void endRenderPass() noexcept;

    void setViewport(
        const float viewportX,
        const float viewportY,
        const float viewportW,
        const float viewportH,
        const float minDepth,
        const float maxDepth
    ) noexcept;

    void setScissors(
        const int32_t scissorsRectX,
        const int32_t scissorsRectY,
        const uint32_t scissorsRectW,
        const uint32_t scissorsRectH
    ) noexcept;
    
    void bindPipeline(const Pipeline& pipeline) noexcept;
    void bindVertexBuffer(const Buffer& buffer, const uint32_t bindingIndex, const uint64_t offset) noexcept;
    void bindIndexBufferUint16(const Buffer& buffer, const uint64_t offset) noexcept;
    void bindIndexBufferUint32(const Buffer& buffer, const uint64_t offset) noexcept;

    void bindDescriptorSet(
        const DescriptorSet& descriptorSet,
        const Pipeline& pipeline,
        const uint32_t descriptorSetIndex,
        const uint32_t dynamicOffsetCount = 0,
        const uint32_t* const pDynamicOffsets = nullptr
    ) noexcept;

    void draw(const uint32_t vertexCount, const uint32_t firstVertex) noexcept;

    void drawIndexed(
        const uint32_t indexCount,
        const uint32_t firstIndex,
        const uint32_t vertexNumberOffset = 0
    ) noexcept;

    void pushConstants(
        const PipelineLayout& pipelineLayout,
        const VkShaderStageFlags affectedShaderStageFlags,
        const uint32_t offset,
        const uint32_t size,
        const void* const pConstantData
    ) noexcept;

    void exec(const VkCommandBuffer secondaryCmdBuffer) noexcept;

    void dispatchWorkgroups(
        const uint32_t numWorkgroupsX,
        const uint32_t numWorkgroupsY = 1,
        const uint32_t numWorkgroupsZ = 1
    ) noexcept;

private:
    // Copy and move are disallowed
    CmdBufferRecorder(const CmdBufferRecorder& other) = delete;
    CmdBufferRecorder(CmdBufferRecorder&& other) = delete;
    CmdBufferRecorder& operator = (const CmdBufferRecorder& other) = delete;
    CmdBufferRecorder& operator = (CmdBufferRecorder&& other) = delete;

    const VkFuncs&      mVkFuncs;           // Vulkan API functions
    VkCommandBuffer     mVkCommandBuffer;   // The current Vulkan command buffer we are recording to
};

END_NAMESPACE(vgl)
