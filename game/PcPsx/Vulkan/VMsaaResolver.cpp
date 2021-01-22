//------------------------------------------------------------------------------------------------------------------------------------------
// Handles the resolving of MSAA and the resources required for that is MSAA is enabled for the game
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VMsaaResolver.h"

#include "Asserts.h"
#include "Buffer.h"
#include "CmdBufferRecorder.h"
#include "Defines.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "FatalErrors.h"
#include "LogicalDevice.h"
#include "Pipeline.h"
#include "RenderTexture.h"
#include "VPipelines.h"
#include "VTypes.h"

BEGIN_NAMESPACE(VMsaaResolver)

// A vertex buffer containing a single quad of vertex type 'VVertex_MsaaResolve' covering the entire screen.
// This is used to draw a screen quad during MSAA resolve.
static vgl::Buffer gResolveVertexBuffer;

// Destination color attachments for resolving MSAA samples to 1 sample
vgl::RenderTexture gResolveColorAttachments[vgl::Defines::RINGBUFFER_SIZE];

// A descriptor pool and the descriptor sets allocated from it.
// The descriptor sets just contain a binding for an input msaa attachment to sample, one for each ringbuffer index so we don't have to update constantly.
vgl::DescriptorPool gDescriptorPool;
vgl::DescriptorSet* gpDescriptorSets[vgl::Defines::RINGBUFFER_SIZE];

//------------------------------------------------------------------------------------------------------------------------------------------
// Create the vertex buffer used for MSAA resolve and populate it
//------------------------------------------------------------------------------------------------------------------------------------------
static void initResolveVertexBuffer(vgl::LogicalDevice& device) noexcept {
    // Create the buffer
    const bool bCreatedBufferOk = gResolveVertexBuffer.initWithElementCount<VVertex_MsaaResolve>(
        device,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        vgl::BufferUsageMode::STATIC,
        6
    );

    if (!bCreatedBufferOk)
        FatalErrors::raise("Failed to initialize a vertex buffer used for MSAA resolve!");

    // Lock the buffer and populate it
    VVertex_MsaaResolve* const pVerts = gResolveVertexBuffer.lockElements<VVertex_MsaaResolve>(0, 6);

    if (!pVerts)
        FatalErrors::raise("Failed to lock a vertex buffer used for MSAA resolve!");

    pVerts[0] = { -1.0f, -1.0f };
    pVerts[1] = { +1.0f, -1.0f };
    pVerts[2] = { +1.0f, +1.0f };
    pVerts[3] = { +1.0f, +1.0f };
    pVerts[4] = { -1.0f, +1.0f };
    pVerts[5] = { -1.0f, -1.0f };

    gResolveVertexBuffer.unlockElements<VVertex_MsaaResolve>(6);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the descriptor pool and descriptor set used for msaa  resolve.
// Only using a single descriptor set which is bound to input multisampled attachment.
//-----------------------------------------------------------------------------------------------------------------------------------------
static void initDescriptorPoolAndSets(vgl::LogicalDevice& device) noexcept {
    // Make the pool
    VkDescriptorPoolSize poolResourceCount = {};
    poolResourceCount.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolResourceCount.descriptorCount = 2;

    if (!gDescriptorPool.init(device, { poolResourceCount }, 2))
        FatalErrors::raise("VDrawing: Failed to create a Vulkan descriptor pool for MSAA resolve!");

    // Make the sets, but don't bind the input msaa attachment yet, need framebuffers to be created for that
    for (vgl::DescriptorSet*& pDescriptorSet : gpDescriptorSets) {
        pDescriptorSet = gDescriptorPool.allocDescriptorSet(VPipelines::gDescSetLayout_msaaResolve);

        if (!pDescriptorSet)
            FatalErrors::raise("VDrawing: Failed to allocate a required Vulkan descriptor set for MSAA resolve!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the MSAA resolver and sets up its main resources.
// Note that this init does NOT create the resolve color attachment, that must be done in a separate call.
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device) noexcept {
    initResolveVertexBuffer(device);
    initDescriptorPoolAndSets(device);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroy the resolver and cleanup all resources
//------------------------------------------------------------------------------------------------------------------------------------------
void destroy() noexcept {
    for (vgl::DescriptorSet*& pDescriptorSet : gpDescriptorSets) {
        if (pDescriptorSet) {
            gDescriptorPool.freeDescriptorSet(*pDescriptorSet);
            pDescriptorSet = nullptr;
        }
    }

    gDescriptorPool.destroy(true);
    destroyResolveColorAttachments();
    gResolveVertexBuffer.destroy(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create all the color attachments used for MSAA resolve with the specified framebuffer size and return 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool createResolveColorAttachments(
    vgl::LogicalDevice& device,
    const VkFormat colorFormat,
    const uint32_t fbWidth,
    const uint32_t fbHeight
) noexcept {
    // Create the attachements
    for (vgl::RenderTexture& attachment : gResolveColorAttachments) {
        if (!attachment.initAsRenderTexture(device, colorFormat, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, fbWidth, fbHeight, 1))
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroy all the color attachments used for MSAA resolve
//------------------------------------------------------------------------------------------------------------------------------------------
void destroyResolveColorAttachments() noexcept {
    for (vgl::RenderTexture& attachment : gResolveColorAttachments) {
        attachment.destroy(true);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the input attachments for MSAA resolve.
// Should be done after these attachments have been created/recreated.
//------------------------------------------------------------------------------------------------------------------------------------------
void setMsaaResolveInputAttachments(const vgl::BaseTexture inputAttachments[vgl::Defines::RINGBUFFER_SIZE]) noexcept {
    for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
        ASSERT(gpDescriptorSets[i]);
        gpDescriptorSets[i]->bindInputAttachment(0, inputAttachments[i]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedules an MSAA resolve to happen.
// Assumes we have already transitioned to the subpass for MSAA resolve.
//------------------------------------------------------------------------------------------------------------------------------------------
void doMsaaResolve(vgl::LogicalDevice& device, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Switch to the correct pipeline and bind the vertex buffer to use
    vgl::Pipeline& pipeline = VPipelines::gPipelines[(uint32_t) VPipelineType::Msaa_Resolve];
    cmdRec.bindPipeline(pipeline);
    cmdRec.bindVertexBuffer(gResolveVertexBuffer, 0, 0);

    // Bind the correct descriptor set
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();
    cmdRec.bindDescriptorSet(*gpDescriptorSets[ringbufferIdx], pipeline, 0);

    // Issue a draw call to draw the full screen quad to do the MSAA resolve
    cmdRec.draw(6, 0);
}

END_NAMESPACE(VMsaaResolver)
