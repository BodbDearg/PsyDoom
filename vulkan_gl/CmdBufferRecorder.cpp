#include "CmdBufferRecorder.h"

#include "BaseRenderPass.h"
#include "Buffer.h"
#include "CmdBuffer.h"
#include "CmdPool.h"
#include "DescriptorSet.h"
#include "Framebuffer.h"
#include "LogicalDevice.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the recorder with no associated command buffer (i.e not recording)
//------------------------------------------------------------------------------------------------------------------------------------------
CmdBufferRecorder::CmdBufferRecorder(const VkFuncs& vkFuncs) noexcept
    : mVkFuncs(vkFuncs)
    , mVkCommandBuffer(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destructor: doesn't need to do anything since the command buffer only has weak references to all it's resources
//------------------------------------------------------------------------------------------------------------------------------------------
CmdBufferRecorder::~CmdBufferRecorder() noexcept {
    ASSERT_LOG(!isRecording(), "Failed to end recording before recorder destroyed!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begins recording a primary command buffer and returns 'true' on success
//------------------------------------------------------------------------------------------------------------------------------------------
bool CmdBufferRecorder::beginPrimaryCmdBuffer(CmdBuffer& cmdBuffer, const VkCommandBufferUsageFlags usageFlags) noexcept {
    ASSERT_LOG((!isRecording()), "Can't record while already recording, stop recording first!");
    ASSERT(cmdBuffer.isValid());

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags;
    
    if (mVkFuncs.vkBeginCommandBuffer(cmdBuffer.getVkCommandBuffer(), &beginInfo) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to begin command buffer recording!");
        return false;
    }

    mVkCommandBuffer = cmdBuffer.getVkCommandBuffer();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin recording a secondary command buffer that is executed entirely within a renderpass for the given framebuffer
//------------------------------------------------------------------------------------------------------------------------------------------
bool CmdBufferRecorder::beginSecondaryCmdBuffer(
    CmdBuffer& cmdBuffer,
    const VkCommandBufferUsageFlags usageFlags,
    const VkRenderPass containingRenderPass,
    const uint32_t containingSubpass,
    const VkFramebuffer targetFramebuffer
) noexcept {
    ASSERT_LOG((!isRecording()), "Can't record while already recording, stop recording first!");
    ASSERT(cmdBuffer.isValid());
    ASSERT(containingRenderPass);
    ASSERT(targetFramebuffer);

    VkCommandBufferInheritanceInfo inheritInfo = {};
    inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritInfo.renderPass = containingRenderPass;
    inheritInfo.subpass = containingSubpass;
    inheritInfo.framebuffer = targetFramebuffer;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;    // RENDER_PASS_CONTINUE_BIT: Secondary command buffer is assumed to be entirely within a renderpass
    beginInfo.pInheritanceInfo = &inheritInfo;

    if (mVkFuncs.vkBeginCommandBuffer(cmdBuffer.getVkCommandBuffer(), &beginInfo) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to begin command buffer recording!");
        return false;
    }

    mVkCommandBuffer = cmdBuffer.getVkCommandBuffer();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// End recording a primary or secondary command buffer; returns 'true' on success
//------------------------------------------------------------------------------------------------------------------------------------------
bool CmdBufferRecorder::endCmdBuffer() noexcept {
    ASSERT_LOG(isRecording(), "Must be recording to stop recording!");

    const bool bEndedOk = (mVkFuncs.vkEndCommandBuffer(mVkCommandBuffer) == VK_SUCCESS);
    ASSERT_LOG(bEndedOk, "Failed to end command buffer recording!");
    mVkCommandBuffer = nullptr;
    return bEndedOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the command buffer is currently being recorded to
//------------------------------------------------------------------------------------------------------------------------------------------
bool CmdBufferRecorder::isRecording() const noexcept {
    return (mVkCommandBuffer != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: begin a renderpass
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::beginRenderPass(
    const BaseRenderPass& renderPass,
    const Framebuffer& framebuffer,
    const VkSubpassContents subpassContents,
    const int32_t renderAreaX,
    const int32_t renderAreaY,
    const uint32_t renderAreaW,
    const uint32_t renderAreaH,
    const VkClearValue* const pClearValues,
    const uint32_t numClearValues
) noexcept {
    ASSERT(isRecording());
    ASSERT(renderPass.isValid());
    ASSERT(framebuffer.isValid());
    ASSERT((pClearValues != nullptr) || (numClearValues <= 0));
    
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.getVkRenderPass();
    renderPassInfo.framebuffer = framebuffer.getVkFramebuffer();
    renderPassInfo.renderArea.offset = { renderAreaX, renderAreaY };
    renderPassInfo.renderArea.extent = { renderAreaW, renderAreaH };
    renderPassInfo.clearValueCount = numClearValues;
    renderPassInfo.pClearValues = pClearValues;

    mVkFuncs.vkCmdBeginRenderPass(mVkCommandBuffer, &renderPassInfo, subpassContents);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: transition onto the next subpass within a renderpass
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::nextSubpass(const VkSubpassContents nextSubpassContents) noexcept {
    ASSERT(isRecording());
    mVkFuncs.vkCmdNextSubpass(mVkCommandBuffer, nextSubpassContents);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: end the current renderpass
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::endRenderPass() noexcept {
    ASSERT(isRecording());
    mVkFuncs.vkCmdEndRenderPass(mVkCommandBuffer);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: set the viewport dimensions.
// Note that we are only supporting one viewport here!
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::setViewport(
    const float viewportX,
    const float viewportY,
    const float viewportW,
    const float viewportH,
    const float minDepth,
    const float maxDepth
) noexcept {
    ASSERT(isRecording());

    VkViewport viewport = {};
    viewport.x = viewportX;
    viewport.y = viewportY;
    viewport.width = viewportW;
    viewport.height = viewportH;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    mVkFuncs.vkCmdSetViewport(mVkCommandBuffer, 0, 1, &viewport);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: set the scissors rectangle area
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::setScissors(
    const int32_t scissorsRectX,
    const int32_t scissorsRectY,
    const uint32_t scissorsRectW,
    const uint32_t scissorsRectH
) noexcept {
    ASSERT(isRecording());

    VkRect2D scissorRect = {};
    scissorRect.offset.x = scissorsRectX;
    scissorRect.offset.y = scissorsRectY;
    scissorRect.extent.width = scissorsRectW;
    scissorRect.extent.height = scissorsRectH;
    mVkFuncs.vkCmdSetScissor(mVkCommandBuffer, 0, 1, &scissorRect);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: bind a pipeline
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::bindPipeline(const Pipeline& pipeline) noexcept {
    ASSERT(isRecording());
    ASSERT(pipeline.isValid());

    const VkPipelineBindPoint bindPoint = (pipeline.isComputePipeline()) ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
    mVkFuncs.vkCmdBindPipeline(mVkCommandBuffer, bindPoint,  pipeline.getVkPipeline());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: bind a vertex buffer
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::bindVertexBuffer(const Buffer& buffer, const uint32_t bindingIndex, const uint64_t offset) noexcept {
    ASSERT(isRecording());
    ASSERT(buffer.isValid());
    ASSERT(offset < buffer.getSizeInBytes());

    const VkBuffer vkBuffer = buffer.getVkBuffer();
    mVkFuncs.vkCmdBindVertexBuffers(mVkCommandBuffer, bindingIndex, 1, &vkBuffer, &offset);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: bind an index buffer
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::bindIndexBufferUint16(const Buffer& buffer, const uint64_t offset) noexcept {
    ASSERT(isRecording());
    ASSERT(buffer.isValid());
    ASSERT(offset < buffer.getSizeInBytes());

    const VkBuffer vkBuffer = buffer.getVkBuffer();
    mVkFuncs.vkCmdBindIndexBuffer(mVkCommandBuffer, buffer.getVkBuffer(), offset, VkIndexType::VK_INDEX_TYPE_UINT16);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: bind an index buffer
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::bindIndexBufferUint32(const Buffer& buffer, const uint64_t offset) noexcept {
    ASSERT(isRecording());
    ASSERT(buffer.isValid());
    ASSERT(offset < buffer.getSizeInBytes());

    mVkFuncs.vkCmdBindIndexBuffer(mVkCommandBuffer, buffer.getVkBuffer(), offset, VkIndexType::VK_INDEX_TYPE_UINT32);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: bind a descriptor set
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::bindDescriptorSet(
    const DescriptorSet& descriptorSet,
    const Pipeline& pipeline,
    const uint32_t descriptorSetIndex,
    const uint32_t dynamicOffsetCount,
    const uint32_t* const pDynamicOffsets
) noexcept {
    ASSERT(isRecording());
    ASSERT(descriptorSet.isValid());
    ASSERT(pipeline.isValid());
    ASSERT((pDynamicOffsets != nullptr) || (dynamicOffsetCount == 0));

    const VkPipelineBindPoint bindPoint = (pipeline.isComputePipeline()) ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
    const VkDescriptorSet vkDescriptorSet = descriptorSet.getVkDescriptorSet();

    mVkFuncs.vkCmdBindDescriptorSets(
        mVkCommandBuffer,
        bindPoint,
        pipeline.getVkPipelineLayout(),
        descriptorSetIndex,
        1,                      // Descriptor set count
        &vkDescriptorSet,
        dynamicOffsetCount,     // Dynamic offset count
        pDynamicOffsets         // Dynamic offsets
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: draw geometry (non indexed)
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::draw(const uint32_t vertexCount, const uint32_t firstVertex) noexcept {
    ASSERT(isRecording());
    mVkFuncs.vkCmdDraw(mVkCommandBuffer, vertexCount, 1, firstVertex, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: draw geometry (indexed)
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::drawIndexed(
    const uint32_t indexCount,
    const uint32_t firstIndex,
    const uint32_t vertexNumberOffset
) noexcept {
    ASSERT(isRecording());
    mVkFuncs.vkCmdDrawIndexed(mVkCommandBuffer, indexCount, 1, firstIndex, vertexNumberOffset, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: push and update to the specified range of push constant memory for the specified shader stages
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::pushConstants(
    const PipelineLayout& pipelineLayout,
    const VkShaderStageFlags affectedShaderStageFlags,
    const uint32_t offset,
    const uint32_t size,
    const void* const pConstantData
) noexcept {
    ASSERT(isRecording());
    ASSERT(pipelineLayout.isValid());
    ASSERT(affectedShaderStageFlags != 0);
    ASSERT(size > 0);
    ASSERT(pConstantData);
    ASSERT(offset % Defines::MIN_PUSH_CONSTANT_ALIGNMENT == 0);
    ASSERT(size % Defines::MIN_PUSH_CONSTANT_ALIGNMENT == 0);

    mVkFuncs.vkCmdPushConstants(
        mVkCommandBuffer,
        pipelineLayout.getVkPipelineLayout(),
        affectedShaderStageFlags,
        offset,
        size,
        pConstantData
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: execute a secondary command buffer
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::exec(const VkCommandBuffer secondaryCmdBuffer) noexcept {
    ASSERT(isRecording());
    mVkFuncs.vkCmdExecuteCommands(mVkCommandBuffer, 1, &secondaryCmdBuffer);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recorded command: Dispatch a compute workload consisting of a specified number of workgroups.
// Note that the size of the workgroup is defined by the shader itself, and this affects the total number of shader invocations too.
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBufferRecorder::dispatchWorkgroups(
    const uint32_t numWorkgroupsX,
    const uint32_t numWorkgroupsY,
    const uint32_t numWorkgroupsZ
) noexcept {
    ASSERT(isRecording());
    mVkFuncs.vkCmdDispatch(mVkCommandBuffer, numWorkgroupsX, numWorkgroupsY, numWorkgroupsZ);
}

END_NAMESPACE(vgl)
