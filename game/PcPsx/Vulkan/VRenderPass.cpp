#if PSYDOOM_VULKAN_RENDERER

#include "VRenderPass.h"

#include "LogicalDevice.h"
#include "RenderPassDef.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized render pass
//------------------------------------------------------------------------------------------------------------------------------------------
VRenderPass::VRenderPass() noexcept
    : vgl::BaseRenderPass()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys and cleans up the render pass
//------------------------------------------------------------------------------------------------------------------------------------------
VRenderPass::~VRenderPass() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the render pass with the specified color and depth formats
//------------------------------------------------------------------------------------------------------------------------------------------
bool VRenderPass::init(
    vgl::LogicalDevice& device,
    const VkFormat colorFormat,
    const VkFormat depthFormat,
    const VkFormat colorMsaaResolveFormat,
    const uint32_t sampleCount
) noexcept {
    // Basic sanity checks
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.isValid());
    ASSERT(vgl::Utils::isPowerOf2(sampleCount));

    // Define the color attachment
    vgl::RenderPassDef renderPassDef;

    VkAttachmentDescription& colorAttach = renderPassDef.attachments.emplace_back();
    colorAttach.format = colorFormat;
    colorAttach.samples = (VkSampleCountFlagBits) sampleCount;          // Can just cast for the correct conversion
    colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (sampleCount > 1) {
        colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                 // Can save on bandwidth on tiled GPU architectures: don't need to write data to VRAM at the end of the renderpass
        colorAttach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;     // Don't care: would say undefined but that is not allowed
    } else {
        colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttach.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;         // Ready for blitting to the swapchain image
    }

    // Define the depth attachment
    VkAttachmentDescription& depthAttach = renderPassDef.attachments.emplace_back();
    depthAttach.format = depthFormat;
    depthAttach.samples = (VkSampleCountFlagBits) sampleCount;           // Can just cast for the correct conversion
    depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // Only needed during rendering, don't store at end of the pass to save on bandwidth
    depthAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    // If doing MSAA, define the MSAA color resolve attachment
    if (sampleCount > 1) {
        VkAttachmentDescription& resolveAttach = renderPassDef.attachments.emplace_back();
        resolveAttach.format = colorMsaaResolveFormat;
        resolveAttach.samples = VK_SAMPLE_COUNT_1_BIT;
        resolveAttach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveAttach.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;     // Ready for blitting to the swapchain image
    }

    // Define the 'draw depth' subpass for drawing occluder planes
    {
        vgl::SubpassDef& subpassDef = renderPassDef.subpasses.emplace_back();

        VkAttachmentReference& depthAttachRef = subpassDef.depthStencilAttachments.emplace_back();
        depthAttachRef.attachment = 1;
        depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    // Define the main 'draw' subpass and it's attachments
    {
        vgl::SubpassDef& subpassDef = renderPassDef.subpasses.emplace_back();

        VkAttachmentReference& colorAttachRef = subpassDef.colorAttachments.emplace_back();
        colorAttachRef.attachment = 0;
        colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference& depthAttachRef = subpassDef.depthStencilAttachments.emplace_back();
        depthAttachRef.attachment = 1;
        depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }

    // If doing MSAA, define the MSAA color resolve subpass and the attachment resolved to as well as the input MSAA color attachment
    if (sampleCount > 1) {
        vgl::SubpassDef& subpassDef = renderPassDef.subpasses.emplace_back();

        VkAttachmentReference& resolveAttachRef = subpassDef.colorAttachments.emplace_back();
        resolveAttachRef.attachment = 2;
        resolveAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference& msaaColorAttachRef = subpassDef.inputAttachments.emplace_back();
        msaaColorAttachRef.attachment = 0;
        msaaColorAttachRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

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
void VRenderPass::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    mbIsValid = false;
    BaseRenderPass::destroy();
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
