#if PSYDOOM_VULKAN_RENDERER

#include "VPresentRenderPass.h"

#include "LogicalDevice.h"
#include "RenderPassDef.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized render pass
//------------------------------------------------------------------------------------------------------------------------------------------
VPresentRenderPass::VPresentRenderPass() noexcept
    : vgl::BaseRenderPass()
    , mColorFormat(VK_FORMAT_UNDEFINED)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys and cleans up the render pass
//------------------------------------------------------------------------------------------------------------------------------------------
VPresentRenderPass::~VPresentRenderPass() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the render pass with the specified color format
//------------------------------------------------------------------------------------------------------------------------------------------
bool VPresentRenderPass::init(vgl::LogicalDevice& device, const VkFormat colorFormat) noexcept {
    // Basic initialization and sanity checks
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.isValid());

    mColorFormat = colorFormat;

    // Define color attachment
    vgl::RenderPassDef renderPassDef;

    VkAttachmentDescription& colorAttach = renderPassDef.attachments.emplace_back();
    colorAttach.format = colorFormat;
    colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttach.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    colorAttach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Define the single subpass and its attachment references
    vgl::SubpassDef& subpassDef = renderPassDef.subpasses.emplace_back();

    VkAttachmentReference& colorAttachRef = subpassDef.colorAttachments.emplace_back();
    colorAttachRef.attachment = 0;
    colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Create the actual render pass 
    if (!BaseRenderPass::init(device, renderPassDef)) {
        destroy(true);
        return false;
    }

    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the render pass and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void VPresentRenderPass::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    mbIsValid = false;
    mColorFormat = VK_FORMAT_UNDEFINED;

    BaseRenderPass::destroy();
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
