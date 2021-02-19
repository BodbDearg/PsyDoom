#include "VRenderPath_Crossfade.h"

#if PSYDOOM_VULKAN_RENDERER

#include "CmdBufferRecorder.h"
#include "LogicalDevice.h"
#include "RenderPassDef.h"
#include "Swapchain.h"
#include "VDrawing.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the render path to a default uninitialized state
//------------------------------------------------------------------------------------------------------------------------------------------
VRenderPath_Crossfade::VRenderPath_Crossfade() noexcept 
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mpSwapchain(nullptr)
    , mPresentSurfaceFormat()
    , mpMainRenderPath(nullptr)
    , mRenderPass()
    , mFramebuffers()
    , mpOldFbImagesToLayout{}
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the render path if not already destroyed
//------------------------------------------------------------------------------------------------------------------------------------------
VRenderPath_Crossfade::~VRenderPath_Crossfade() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the render path - this must always succeed
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Crossfade::init(
    vgl::LogicalDevice& device,
    vgl::Swapchain& swapchain,
    const VkFormat presentSurfaceFormat,
    VRenderPath_Main& mainRenderPath
) noexcept {
    // Sanity checks
    ASSERT_LOG(!mbIsValid, "Can't initialize twice!");
    ASSERT(device.isValid());

    // Remember all this info
    mpDevice = &device;
    mpSwapchain = &swapchain;
    mpMainRenderPath = &mainRenderPath;
    mPresentSurfaceFormat = presentSurfaceFormat;

    // Create the renderpass
    if (!initRenderPass())
        FatalErrors::raise("Failed to create the crossfade Vulkan renderpass!");

    // Now initialized
    mbIsValid = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Crossfade::destroy() noexcept {
    if (!mbIsValid)
        return;

    mbIsValid = false;

    mpOldFbImagesToLayout[0] = nullptr;
    mpOldFbImagesToLayout[1] = nullptr;
    
    for (vgl::Framebuffer& framebuffer : mFramebuffers) {
        framebuffer.destroy(true);
    }

    mFramebuffers.clear();
    mRenderPass.destroy();

    mPresentSurfaceFormat = {};
    mpMainRenderPath = nullptr;
    mpSwapchain = nullptr;
    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates or recreates the framebuffers for this render path
//------------------------------------------------------------------------------------------------------------------------------------------
bool VRenderPath_Crossfade::ensureValidFramebuffers([[maybe_unused]] const uint32_t fbWidth, [[maybe_unused]] const uint32_t fbHeight) noexcept {
    ASSERT(mbIsValid);

    // Only do this if we need to actually create/re-create framebuffers
    if (!doFramebuffersNeedRecreate())
        return true;

    // Recreate all framebuffers
    vgl::Swapchain& swapchain = *mpSwapchain;
    const uint32_t swapchainLen = swapchain.getLength();
    mFramebuffers.resize(swapchainLen);

    for (uint32_t swapImgIdx = 0; swapImgIdx < swapchainLen; ++swapImgIdx) {
        vgl::Framebuffer& framebuffer = mFramebuffers[swapImgIdx];
        framebuffer.destroy(true);

        if (!framebuffer.init(mRenderPass, swapchain, swapImgIdx, {}))
            return false;
    }

    // Nothing to do for this renderpath...
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begins the frame for the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Crossfade::beginFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Sanity checks
    ASSERT(mbIsValid);
    ASSERT(mpDevice);
    ASSERT(swapchain.isValid());
    ASSERT(swapchain.getAcquiredImageIdx() < swapchain.getVkImages().size());

    // Transition the image layout for old framebuffer images, if required; this will happen on the first render path frame...
    if (mpOldFbImagesToLayout[0]) {
        transitionOldFramebufferTexLayout(*mpOldFbImagesToLayout[0], cmdRec);
        mpOldFbImagesToLayout[0] = nullptr;
    }

    if (mpOldFbImagesToLayout[1]) {
        transitionOldFramebufferTexLayout(*mpOldFbImagesToLayout[1], cmdRec);
        mpOldFbImagesToLayout[1] = nullptr;
    }

    // Begin the render pass
    const uint32_t swapchainIdx = swapchain.getAcquiredImageIdx();
    vgl::Framebuffer& framebuffer = mFramebuffers[swapchainIdx];

    cmdRec.beginRenderPass(
        mRenderPass,
        framebuffer,
        VK_SUBPASS_CONTENTS_INLINE,
        0,
        0,
        framebuffer.getWidth(),
        framebuffer.getHeight(),
        nullptr,
        0
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ends the frame for the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Crossfade::endFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Sanity checks and end the current render pass
    ASSERT(mbIsValid);
    ASSERT(swapchain.isValid());
    cmdRec.endRenderPass();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the Vulkan renderpass used by the render path
//------------------------------------------------------------------------------------------------------------------------------------------
bool VRenderPath_Crossfade::initRenderPass() noexcept {
    // Sanity checks and getting the device
    ASSERT(mpDevice);
    vgl::LogicalDevice& device = *mpDevice;

    // Define the single color attachment
    vgl::RenderPassDef renderPassDef;

    VkAttachmentDescription& colorAttach = renderPassDef.attachments.emplace_back();
    colorAttach.format = mPresentSurfaceFormat;
    colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;               // No need to clear, will be filling the screen anyway
    colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;          // Ready for presentation

    // Define the single subpass and it's attachments
    {
        vgl::SubpassDef& subpassDef = renderPassDef.subpasses.emplace_back();

        VkAttachmentReference& colorAttachRef = subpassDef.colorAttachments.emplace_back();
        colorAttachRef.attachment = 0;
        colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    // Finally, create the renderpass
    return mRenderPass.init(device, renderPassDef);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the framebuffers need recreation
//------------------------------------------------------------------------------------------------------------------------------------------
bool VRenderPath_Crossfade::doFramebuffersNeedRecreate() noexcept {
    // Firstly ensure we have the right amount of framebuffers
    ASSERT(mbIsValid);
    vgl::Swapchain& swapchain = *mpSwapchain;
    const uint32_t swapchainLen = swapchain.getLength();

    if (swapchainLen != mFramebuffers.size())
        return true;

    // Make sure each framebuffer is valid and references the correct swapchain image
    for (uint32_t swapImgIdx = 0; swapImgIdx < swapchainLen; ++swapImgIdx) {
        vgl::Framebuffer& framebuffer = mFramebuffers[swapImgIdx];

        if (!framebuffer.isValid())
            return true;

        if (framebuffer.getAttachmentImages().size() != 1)
            return true;

        if (framebuffer.getAttachmentImages()[0] != swapchain.getVkImages()[swapImgIdx])
            return true;
    }

    // If we get to here then the framebuffers don't need recreation
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Crossfade drawing setup.
// These functions schedule specified framebuffer color attachment textures to be transitioned from an expected 'transfer source' image
// layout to a 'shader read only' image layout at the beginning of the next render path frame. This transition is required to get the
// framebuffers into the correct layout so they can be sampled in shaders.
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Crossfade::scheduleOldFramebufferLayoutTransitions(vgl::RenderTexture& fb1ColorAttach, vgl::RenderTexture& fb2ColorAttach) noexcept {
    mpOldFbImagesToLayout[0] = &fb1ColorAttach;
    mpOldFbImagesToLayout[1] = &fb2ColorAttach;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Transition the specified framebuffer texture/color-attachment (if valid) from an expected 'transfer source' image layout to the
// 'shader read only' image layout. This prepares it for reading in shader code.
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Crossfade::transitionOldFramebufferTexLayout(vgl::RenderTexture& fbColorAttach, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Do nothing if invalid
    if (!fbColorAttach.isValid())
        return;

    // Schedule the image layout transition
    VkImageMemoryBarrier imgBarrier = {};
    imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgBarrier.image = fbColorAttach.getVkImage();
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

#endif  // #if PSYDOOM_VULKAN_RENDERER
