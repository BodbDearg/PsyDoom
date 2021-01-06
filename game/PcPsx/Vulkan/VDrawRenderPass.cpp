#if PSYDOOM_VULKAN_RENDERER

#include "VDrawRenderPass.h"

#include "LogicalDevice.h"
#include "RenderPassDef.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized render pass
//------------------------------------------------------------------------------------------------------------------------------------------
VDrawRenderPass::VDrawRenderPass() noexcept
    : vgl::BaseRenderPass()
    , mColorFormat(VK_FORMAT_UNDEFINED)
    , mDepthStencilFormat(VK_FORMAT_UNDEFINED)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys and cleans up the render pass
//------------------------------------------------------------------------------------------------------------------------------------------
VDrawRenderPass::~VDrawRenderPass() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the render pass with the specified color and depth/stencil formats
//------------------------------------------------------------------------------------------------------------------------------------------
bool VDrawRenderPass::init(vgl::LogicalDevice& device, const VkFormat colorFormat, const VkFormat depthStencilFormat) noexcept {
    // Basic initialization and sanity checks
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.isValid());

    mColorFormat = colorFormat;
    mDepthStencilFormat = depthStencilFormat;

    // Define color attachment
    vgl::RenderPassDef renderPassDef;

    VkAttachmentDescription& colorAttach = renderPassDef.attachments.emplace_back();
    colorAttach.format = colorFormat;
    colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Define depth attachment
    VkAttachmentDescription& depthAttach = renderPassDef.attachments.emplace_back();
    depthAttach.format = depthStencilFormat;
    depthAttach.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;            // Not using stencil buffers
    depthAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;          // Not using stencil buffers
    depthAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Define the single subpass and its attachment references
    vgl::SubpassDef& subpassDef = renderPassDef.subpasses.emplace_back();

    VkAttachmentReference& colorAttachRef = subpassDef.colorAttachments.emplace_back();
    colorAttachRef.attachment = 0;
    colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference& depthAttachRef = subpassDef.depthStencilAttachments.emplace_back();
    depthAttachRef.attachment = 1;
    depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
void VDrawRenderPass::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    mbIsValid = false;
    mDepthStencilFormat = VK_FORMAT_UNDEFINED;
    mColorFormat = VK_FORMAT_UNDEFINED;

    BaseRenderPass::destroy();
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
