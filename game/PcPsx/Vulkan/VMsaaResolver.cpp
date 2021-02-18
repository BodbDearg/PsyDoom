#include "VMsaaResolver.h"

#if PSYDOOM_VULKAN_RENDERER

#include "CmdBufferRecorder.h"
#include "DescriptorSet.h"
#include "LogicalDevice.h"
#include "Pipeline.h"
#include "VPipelines.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Defaults the MSAA resolver to an uninitalized state
//------------------------------------------------------------------------------------------------------------------------------------------
VMsaaResolver::VMsaaResolver() noexcept
    : mbIsValid(false)
    , mVertexBuffer()
    , mResolveAttachments{}
    , mDescriptorPool()
    , mpDescriptorSets{}
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroy the MSAA resolver and it's resources
//------------------------------------------------------------------------------------------------------------------------------------------
VMsaaResolver::~VMsaaResolver() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the MSAA resolver and sets up its main resources.
// Note that this init does NOT create the resolve color attachments, that must be done in a separate call.
//------------------------------------------------------------------------------------------------------------------------------------------
void VMsaaResolver::init(vgl::LogicalDevice& device) noexcept {
    ASSERT_LOG((!mbIsValid), "VMsaaResolver is already initialized!");

    initVertexBuffer(device);
    initDescriptorPoolAndSets(device);
    mbIsValid = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroy the resolver and cleanup all resources
//------------------------------------------------------------------------------------------------------------------------------------------
void VMsaaResolver::destroy() noexcept {
    mbIsValid = false;

    for (vgl::DescriptorSet*& pDescriptorSet : mpDescriptorSets) {
        if (pDescriptorSet) {
            mDescriptorPool.freeDescriptorSet(*pDescriptorSet);
            pDescriptorSet = nullptr;
        }
    }

    mDescriptorPool.destroy(true);
    destroyResolveAttachments();
    mVertexBuffer.destroy(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create all the color attachments used for MSAA resolve for the specified framebuffer size and return 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool VMsaaResolver::createResolveAttachments(const VkFormat format, const uint32_t fbWidth, const uint32_t fbHeight) noexcept {
    // Get the render device used
    ASSERT(mbIsValid);
    vgl::LogicalDevice& device = *mVertexBuffer.getDevice();

    // Create or re-create the attachements
    for (vgl::RenderTexture& attachment : mResolveAttachments) {
        attachment.destroy(true);
        const VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        if (!attachment.initAsRenderTexture(device, format, imageUsageFlags, fbWidth, fbHeight, 1))
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroy all the color attachments used for MSAA resolve
//------------------------------------------------------------------------------------------------------------------------------------------
void VMsaaResolver::destroyResolveAttachments() noexcept {
    for (vgl::RenderTexture& attachment : mResolveAttachments) {
        attachment.destroy(true);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the input multi-sampled attachments to be resolved.
// This function should be called after these external attachments have been created/recreated.
//------------------------------------------------------------------------------------------------------------------------------------------
void VMsaaResolver::setInputAttachments(const vgl::BaseTexture inputAttachments[vgl::Defines::RINGBUFFER_SIZE]) noexcept {
    ASSERT(mbIsValid);

    for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
        ASSERT(mpDescriptorSets[i]);
        mpDescriptorSets[i]->bindInputAttachment(0, inputAttachments[i]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedules an MSAA resolve to happen.
// Assumes we have already transitioned to the subpass for MSAA resolve.
//------------------------------------------------------------------------------------------------------------------------------------------
void VMsaaResolver::resolve(vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Get the render device
    ASSERT(mbIsValid);
    vgl::LogicalDevice& device = *mVertexBuffer.getDevice();

    // Set the viewport and scissors rect dimensions
    const uint32_t fbWidth = mResolveAttachments[0].getWidth();
    const uint32_t fbHeight = mResolveAttachments[0].getHeight();

    cmdRec.setViewport(0, 0, (float) fbWidth, (float) fbHeight, 0.0f, 1.0f);
    cmdRec.setScissors(0, 0, fbWidth, fbHeight);

    // Switch to the correct pipeline and bind the vertex buffer to use
    vgl::Pipeline& pipeline = VPipelines::gPipelines[(uint32_t) VPipelineType::Msaa_Resolve];
    cmdRec.bindPipeline(pipeline);
    cmdRec.bindVertexBuffer(mVertexBuffer, 0, 0);

    // Bind the correct descriptor set
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();
    cmdRec.bindDescriptorSet(*mpDescriptorSets[ringbufferIdx], pipeline, 0);

    // Issue a draw call to draw the full screen quad to do the MSAA resolve
    cmdRec.draw(6, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells whether all resolve attachments are valid for the specified framebuffer width and height
//------------------------------------------------------------------------------------------------------------------------------------------
bool VMsaaResolver::areAllResolveAttachmentsValid(const uint32_t fbWidth, const uint32_t fbHeight) noexcept {
    for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
        vgl::RenderTexture& tex = mResolveAttachments[i];

        if ((!tex.isValid()) || (tex.getWidth() != fbWidth) || (tex.getHeight() != fbHeight))
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create the vertex buffer used for MSAA resolve and populate it
//------------------------------------------------------------------------------------------------------------------------------------------
void VMsaaResolver::initVertexBuffer(vgl::LogicalDevice& device) noexcept {
    // Create the buffer
    const bool bCreatedBufferOk = mVertexBuffer.initWithElementCount<VVertex_MsaaResolve>(
        device,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        vgl::BufferUsageMode::STATIC,
        6
    );

    if (!bCreatedBufferOk)
        FatalErrors::raise("Failed to initialize a vertex buffer used for MSAA resolve!");

    // Lock the buffer and populate it
    VVertex_MsaaResolve* const pVerts = mVertexBuffer.lockElements<VVertex_MsaaResolve>(0, 6);

    if (!pVerts)
        FatalErrors::raise("Failed to lock a vertex buffer used for MSAA resolve!");

    pVerts[0] = { -1.0f, -1.0f };
    pVerts[1] = { +1.0f, -1.0f };
    pVerts[2] = { +1.0f, +1.0f };
    pVerts[3] = { +1.0f, +1.0f };
    pVerts[4] = { -1.0f, +1.0f };
    pVerts[5] = { -1.0f, -1.0f };

    mVertexBuffer.unlockElements<VVertex_MsaaResolve>(6);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the descriptor pool and descriptor sets used for msaa resolve
//-----------------------------------------------------------------------------------------------------------------------------------------
void VMsaaResolver::initDescriptorPoolAndSets(vgl::LogicalDevice& device) noexcept {
    // Make the pool
    VkDescriptorPoolSize poolResourceCount = {};
    poolResourceCount.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolResourceCount.descriptorCount = 2;

    if (!mDescriptorPool.init(device, { poolResourceCount }, 2))
        FatalErrors::raise("Failed to create a descriptor pool for MSAA resolve!");

    // Make the sets, but don't bind the input msaa attachment yet - do that at the behest of the user of this class
    for (vgl::DescriptorSet*& pDescriptorSet : mpDescriptorSets) {
        pDescriptorSet = mDescriptorPool.allocDescriptorSet(VPipelines::gDescSetLayout_msaaResolve);

        if (!pDescriptorSet)
            FatalErrors::raise("Failed to allocate a required descriptor set for MSAA resolve!");
    }
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
