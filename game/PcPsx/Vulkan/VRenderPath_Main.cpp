#include "VRenderPath_Main.h"

#if PSYDOOM_VULKAN_RENDERER

#include "CmdBufferRecorder.h"
#include "LogicalDevice.h"
#include "RenderPassDef.h"
#include "Swapchain.h"
#include "VDrawing.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the render path to a default uninitialized state
//------------------------------------------------------------------------------------------------------------------------------------------
VRenderPath_Main::VRenderPath_Main() noexcept 
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mNumDrawSamples(0)
    , mColorFormat{}
    , mResolveFormat{}
    , mRenderPass()
    , mMsaaResolver()
    , mColorAttachments{}
    , mFramebuffers{}
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the render path if not already destroyed
//------------------------------------------------------------------------------------------------------------------------------------------
VRenderPath_Main::~VRenderPath_Main() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the render path - this must always succeed
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Main::init(
    vgl::LogicalDevice& device,
    const uint32_t numDrawSamples,
    const VkFormat colorFormat,
    const VkFormat resolveFormat
) noexcept {
    // Sanity checks
    ASSERT_LOG(!mbIsValid, "Can't initialize twice!");
    ASSERT(device.isValid());

    // Remember all this info
    mpDevice = &device;
    mNumDrawSamples = numDrawSamples;
    mColorFormat = colorFormat;
    mResolveFormat = resolveFormat;

    // Create the renderpass and the MSAA resolver if we are doing multi-sampling
    if (!initRenderPass())
        FatalErrors::raise("Failed to create the main Vulkan renderpass!");

    if (mNumDrawSamples > 1) {
        mMsaaResolver.init(device);
    }

    // Now initialized
    mbIsValid = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Main::destroy() noexcept {
    if (!mbIsValid)
        return;

    mbIsValid = false;
    
    for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
        mFramebuffers[i].destroy(true);
        mColorAttachments[i].destroy(true);
    }

    mMsaaResolver.destroy();
    mRenderPass.destroy();
    mResolveFormat = {};
    mColorFormat = {};
    mNumDrawSamples = 0;
    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates or recreates the framebuffers for this render path
//------------------------------------------------------------------------------------------------------------------------------------------
bool VRenderPath_Main::ensureValidFramebuffers(const uint32_t fbWidth, const uint32_t fbHeight) noexcept {
    // Sanity checks and getting the device
    ASSERT(mbIsValid);
    ASSERT(mpDevice);
    vgl::LogicalDevice& device = *mpDevice;

    // Recreate MSAA resolve attachments if needed
    const bool bDoingMsaa = (mNumDrawSamples > 1);

    if (bDoingMsaa && (!mMsaaResolver.areAllResolveAttachmentsValid(fbWidth, fbHeight))) {
        if (!mMsaaResolver.createResolveAttachments(mResolveFormat, fbWidth, fbHeight))
            return false;
    }

    // Ensure all the framebuffers are created and valid
    bool bCreatedColorAttachments = false;

    for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
        // Do we need to create or re-create this framebuffer?
        vgl::Framebuffer& framebuffer = mFramebuffers[i];
        const bool bNeedNewFramebuffer = (
            (!framebuffer.isValid()) ||
            (fbWidth != framebuffer.getWidth()) ||
            (fbHeight != framebuffer.getHeight())
        );

        if (!bNeedNewFramebuffer)
            continue;

        // Cleanup any previous framebuffer and attachments
        mFramebuffers[i].destroy(true);
        mColorAttachments[i].destroy(true);

        // Color attachment can either be used as a transfer source (for blits, no MSAA) or an input attachment for MSAA resolve
        const VkImageUsageFlags colorAttachUsage = (mNumDrawSamples > 1) ?
            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT :
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        if (!mColorAttachments[i].initAsRenderTexture(device, mColorFormat, colorAttachUsage, fbWidth, fbHeight, mNumDrawSamples))
            return false;

        std::vector<const vgl::BaseTexture*> fbAttachments;
        fbAttachments.reserve(3);
        fbAttachments.push_back(&mColorAttachments[i]);

        if (mNumDrawSamples > 1) {
            fbAttachments.push_back(&mMsaaResolver.getResolveAttachment(i));
        }

        if (!mFramebuffers[i].init(mRenderPass, fbAttachments))
            return false;

        bCreatedColorAttachments = true;
    }

    // Need to set the input attachments for the MSAA resolver after creating framebuffer color attachments
    if (bDoingMsaa && bCreatedColorAttachments) {
        mMsaaResolver.setInputAttachments(mColorAttachments);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begins the frame for the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Main::beginFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Sanity checks and getting the device
    ASSERT(mbIsValid);
    ASSERT(mpDevice);
    ASSERT(swapchain.isValid());

    vgl::LogicalDevice& device = *swapchain.getDevice();

    // Transition the swapchain image to transfer destination optimal in preparation for blitting
    const uint32_t swapchainIdx = swapchain.getAcquiredImageIdx();
    ASSERT(swapchainIdx < swapchain.getLength());

    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.image = swapchain.getVkImages()[swapchainIdx];
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;

        cmdRec.addPipelineBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            nullptr,
            1,
            &imgBarrier
        );
    }

    // Begin the render pass and clear all attachments
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();
    vgl::Framebuffer& framebuffer = mFramebuffers[ringbufferIdx];
    VkClearValue framebufferClearValues[1] = {};

    cmdRec.beginRenderPass(
        mRenderPass,
        framebuffer,
        VK_SUBPASS_CONTENTS_INLINE,
        0,
        0,
        framebuffer.getWidth(),
        framebuffer.getHeight(),
        framebufferClearValues,
        2
    );

    // Begin a frame for the drawing module
    VDrawing::beginFrame(ringbufferIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ends the frame for the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Main::endFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Sanity checks and getting the device
    ASSERT(mbIsValid);
    ASSERT(mpDevice);
    ASSERT(swapchain.isValid());

    vgl::LogicalDevice& device = *mpDevice;

    // Finish up drawing for the 'occluder plane' and regular 'draw' subpasses
    VDrawing::endFrame(cmdRec);

    // Do an MSAA resolve subpass if MSAA is enabled
    if (mNumDrawSamples > 1) {
        cmdRec.nextSubpass(VK_SUBPASS_CONTENTS_INLINE);
        mMsaaResolver.resolve(cmdRec);
    }

    // Done with the render pass now
    cmdRec.endRenderPass();
    
    // Blit the drawing color attachment (or MSAA resolve target, if MSAA is active) to the swapchain image
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();
    const vgl::Framebuffer& framebuffer = mFramebuffers[ringbufferIdx];

    const VkImage blitSrcImage = (mNumDrawSamples > 1) ?
        mMsaaResolver.getResolveAttachment(ringbufferIdx).getVkImage() :    // Blit from the MSAA resolve color buffer
        framebuffer.getAttachmentImages()[0];                               // No MSAA: blit directly from the color buffer that was drawn to

    const uint32_t swapchainIdx = swapchain.getAcquiredImageIdx();
    const VkImage swapchainImage = swapchain.getVkImages()[swapchainIdx];

    {
        VkImageBlit blitRegion = {};
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.srcOffsets[1].x = framebuffer.getWidth();
        blitRegion.srcOffsets[1].y = framebuffer.getHeight();
        blitRegion.srcOffsets[1].z = 1;
        blitRegion.dstOffsets[1].x = swapchain.getSwapExtentWidth();
        blitRegion.dstOffsets[1].y = swapchain.getSwapExtentHeight();
        blitRegion.dstOffsets[1].z = 1;

        cmdRec.blitImage(
            blitSrcImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapchainImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blitRegion,
            VK_FILTER_NEAREST
        );
    }

    // Transition the swapchain image back to presentation optimal in preparation for presentation
    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imgBarrier.image = swapchainImage;
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;

        cmdRec.addPipelineBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            nullptr,
            1,
            &imgBarrier
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the Vulkan renderpass used by the render path
//------------------------------------------------------------------------------------------------------------------------------------------
bool VRenderPath_Main::initRenderPass() noexcept {
    // Sanity checks and getting the device
    ASSERT(mpDevice);
    vgl::LogicalDevice& device = *mpDevice;

    // Define the color attachment
    vgl::RenderPassDef renderPassDef;

    VkAttachmentDescription& colorAttach = renderPassDef.attachments.emplace_back();
    colorAttach.format = mColorFormat;
    colorAttach.samples = (VkSampleCountFlagBits) mNumDrawSamples;      // Can just cast for the correct conversion
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (mNumDrawSamples > 1) {
        colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                 // Can save on bandwidth on tiled GPU architectures: don't need to write data to VRAM at the end of the renderpass
        colorAttach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;     // Don't care: would say undefined but that is not allowed
    } else {
        colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttach.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;         // Ready for blitting to the swapchain image
    }

    // If doing MSAA, define the MSAA color resolve attachment
    if (mNumDrawSamples > 1) {
        VkAttachmentDescription& resolveAttach = renderPassDef.attachments.emplace_back();
        resolveAttach.format = mResolveFormat;
        resolveAttach.samples = VK_SAMPLE_COUNT_1_BIT;
        resolveAttach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveAttach.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;     // Ready for blitting to the swapchain image
    }

    // Define the main 'draw' subpass and it's attachments
    {
        vgl::SubpassDef& subpassDef = renderPassDef.subpasses.emplace_back();

        VkAttachmentReference& colorAttachRef = subpassDef.colorAttachments.emplace_back();
        colorAttachRef.attachment = 0;
        colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    // If doing MSAA, define the MSAA color resolve subpass and the attachment resolved to as well as the input MSAA color attachment
    if (mNumDrawSamples > 1) {
        vgl::SubpassDef& subpassDef = renderPassDef.subpasses.emplace_back();

        VkAttachmentReference& resolveAttachRef = subpassDef.colorAttachments.emplace_back();
        resolveAttachRef.attachment = 1;
        resolveAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference& msaaColorAttachRef = subpassDef.inputAttachments.emplace_back();
        msaaColorAttachRef.attachment = 0;
        msaaColorAttachRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    // Finally, create the renderpass
    return mRenderPass.init(device, renderPassDef);
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
