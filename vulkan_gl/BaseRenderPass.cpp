#include "BaseRenderPass.h"

#include "AttachmentUsageFlags.h"
#include "LogicalDevice.h"
#include "RenderPassDef.h"
#include "VkFuncs.h"

#include <cstring>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// A hack to make ComparableVkSubpassDependency comparable using the equality operator
//------------------------------------------------------------------------------------------------------------------------------------------
struct ComparableVkSubpassDependency : public VkSubpassDependency {
    bool operator == (const VkSubpassDependency& other) const noexcept {
        return (std::memcmp(this, &other, sizeof(VkSubpassDependency)) == 0);
    }
};

static_assert(sizeof(ComparableVkSubpassDependency) == sizeof(VkSubpassDependency));
static_assert(alignof(ComparableVkSubpassDependency) == alignof(VkSubpassDependency));

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function that computes the dependencies on other subpasses for a particular subpass in a render pass.
// Adds the dependencies to the given output vector and returns the number of dependencies written by the function.
//
// Note:
//  This function does NOT generate dependendencies for external subpasses: i.e VK_SUBPASS_EXTERNAL.
//  The reason for this is because zero information exists within the render pass to make any sort of intelligent decision
//  about how to block with respect to stuff outside of it. Rather than blocking on everything (which is the only real option available)
//  and potentially affecting the ability of render passes to process in parallel, it is instead up to external code to handle the
//  synchronization of all attachments added as input to the render pass.
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t determineSubpassDependencies(
    const uint32_t subpassIdx,
    const RenderPassDef& renderPassDef,
    std::vector<VkSubpassDependency>& dependenciesOut
) noexcept {
    // Get the subpass involved
    ASSERT(subpassIdx < renderPassDef.subpasses.size());
    const SubpassDef& subpass = renderPassDef.subpasses[subpassIdx];

    // Store the dependencies here temporarily
    std::vector<ComparableVkSubpassDependency> subpassDependencies;
    subpassDependencies.reserve(subpass.colorAttachments.size() + subpass.depthStencilAttachments.size());

    //-----------------------------------------------------------------------------------------------------------------
    // Generate dependencies for color attachments.
    // Color attachments are written to so they must wait for both reads and writes to finish.
    //-----------------------------------------------------------------------------------------------------------------
    for (const VkAttachmentReference& attachment : subpass.colorAttachments) {
        const uint32_t attachmentIdx = attachment.attachment;

        // Fill in destination subpass portions of the dependency.
        // The stage that is blocked and operations that are blocked:
        ComparableVkSubpassDependency dependency = {};
        dependency.dstSubpass = subpassIdx;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = (
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        );
        
        // Now find what other subpass it depends on and fill in the operations we must wait on
        uint32_t referencingSubpassIdx = {};
        AttachmentUsageFlags referencingSubpassUsageFlags = 0;

        const bool inUseByPrevSubpass = renderPassDef.findPrevSubpassAttachmentUsage(
            subpassIdx,
            attachmentIdx,
            AttachmentUsageFlagBits::READ | AttachmentUsageFlagBits::WRITE,
            referencingSubpassIdx,
            referencingSubpassUsageFlags
        );
        
        if (inUseByPrevSubpass) {
            dependency.srcSubpass = referencingSubpassIdx;

            if (referencingSubpassUsageFlags & AttachmentUsageFlagBits::WRITE) {
                // Previous referencing subpass is writing - must wait for color output to finish
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = (
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                );
            }
            else {
                // Previous referencing subpass is only reading - just wait for shader use to finish
                dependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                dependency.srcAccessMask = (
                    VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                    VK_ACCESS_SHADER_READ_BIT
                );
            }

            if (!Utils::containerContains(subpassDependencies, dependency)) {
                subpassDependencies.push_back(dependency);
            }
        }
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Generate dependencies for depth stencil attachments.
    // Depth stencil attachments are written to so they must wait for both reads and writes to finish.
    //-----------------------------------------------------------------------------------------------------------------
    for (const VkAttachmentReference& attachment : subpass.depthStencilAttachments) {
        const uint32_t attachmentIdx = attachment.attachment;

        // Fill in destination subpass portions of the dependency.
        // The stage that is blocked and operations that are blocked:
        ComparableVkSubpassDependency dependency = {};
        dependency.dstSubpass = subpassIdx;
        dependency.dstStageMask = (
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
        );
        dependency.dstAccessMask = (
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        );
        
        // Now find what other subpass it depends on and fill in the operations we must wait on
        uint32_t referencingSubpassIdx = {};
        AttachmentUsageFlags referencingSubpassUsageFlags = 0;

        const bool inUseByPrevSubpass = renderPassDef.findPrevSubpassAttachmentUsage(
            subpassIdx,
            attachmentIdx,
            AttachmentUsageFlagBits::READ | AttachmentUsageFlagBits::WRITE,
            referencingSubpassIdx,
            referencingSubpassUsageFlags
        );
        
        if (inUseByPrevSubpass) {
            dependency.srcSubpass = referencingSubpassIdx;

            if (referencingSubpassUsageFlags & AttachmentUsageFlagBits::WRITE) {
                // Previous referencing subpass is writing - must wait for depth stencil output and tests to finish
                dependency.srcStageMask = (
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
                );
                dependency.srcAccessMask = (
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                );
            }
            else {
                // Previous referencing subpass is only reading - just wait for shader use to finish
                dependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                dependency.srcAccessMask = (
                    VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                    VK_ACCESS_SHADER_READ_BIT
                );
            }

            if (!Utils::containerContains(subpassDependencies, dependency)) {
                subpassDependencies.push_back(dependency);
            }
        }
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Generate dependencies for input attachments.
    // Input attachments are only read from, so only block on writes.
    //-----------------------------------------------------------------------------------------------------------------
    for (const VkAttachmentReference& attachment : subpass.inputAttachments) {
        const uint32_t attachmentIdx = attachment.attachment;

        // Fill in destination subpass portions of the dependency.
        // The stage that is blocked and operations that are blocked:
        ComparableVkSubpassDependency dependency = {};
        dependency.dstSubpass = subpassIdx;
        dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;    // Input attachments are only readable in the frag shader
        dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

        // Now find what other subpass it depends on and fill in the operations we must wait on
        uint32_t referencingSubpassIdx = {};
        AttachmentUsageFlags referencingSubpassUsageFlags = 0;

        const bool inUseByPrevSubpass = renderPassDef.findPrevSubpassAttachmentUsage(
            subpassIdx,
            attachmentIdx,
            AttachmentUsageFlagBits::READ | AttachmentUsageFlagBits::WRITE,
            referencingSubpassIdx,
            referencingSubpassUsageFlags
        );
        
        if (inUseByPrevSubpass) {
            dependency.srcSubpass = referencingSubpassIdx;

            // Previous referencing subpass is writing - must wait for all color or depth-stencil access to finish
            if (renderPassDef.subpasses[referencingSubpassIdx].isAttachmentInUseForDepthStencil(attachmentIdx)) {
                // In use as a depth stencil buffer, block until depth stencil read/writes are done
                dependency.srcStageMask = (
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
                );
                dependency.srcAccessMask = (
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                );
            }
            else {
                // Must be in use as a normal color attachment, block until color attachment read/writes are done
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = (
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                );
            }

            if (!Utils::containerContains(subpassDependencies, dependency)) {
                subpassDependencies.push_back(dependency);
            }
        }
    }

    //-----------------------------------------------------------------------------------------------------------------
    // Save final output to the input list and return the number of dependencies found
    //-----------------------------------------------------------------------------------------------------------------
    dependenciesOut.insert(dependenciesOut.end(), subpassDependencies.begin(), subpassDependencies.end());
    return static_cast<uint32_t>(subpassDependencies.size());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells how many color attachments there are for a subpass. Index MUST be valid!
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t BaseRenderPass::getNumSubpassColorAttachments(const uint32_t subpassIndex) const noexcept {
    ASSERT(subpassIndex < mNumSubpasses);
    return mNumSubpassColorAttachments[subpassIndex];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Constructs the renderpass base to be un-initialized and defaulted
//------------------------------------------------------------------------------------------------------------------------------------------
BaseRenderPass::BaseRenderPass() noexcept
    : mbIsValid(false)
    , mNumSubpasses(0)
    , mNumAttachments(0)
    , mNumSubpassColorAttachments{}
    , mpDevice(nullptr)
    , mVkRenderPass(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Base renderpass destructor does nothing, must be implemented by derived classes
//------------------------------------------------------------------------------------------------------------------------------------------
BaseRenderPass::~BaseRenderPass() noexcept {
    // We don't call destroy() here because it is redundant.
    // It's expected that derived classes do this in their own destructors.
    ASSERT_LOG(!mbIsValid, "Derived classes must call destroy() in their destructor!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes/creates the base render pass
//------------------------------------------------------------------------------------------------------------------------------------------
bool BaseRenderPass::init(LogicalDevice& device, const RenderPassDef& renderPassDef) noexcept {
    // Preconditions
    ASSERT_LOG(!mbIsValid, "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());

    // Sanity check: all subpasses specified must have valid resolve attachment lists
    if (!renderPassDef.hasValidResolveAttachmentLists()) {
        ASSERT_FAIL("Invalid resolve attachments list specified for one or more subpasses in a renderpass!");
        return false;
    }

    // Save basic info
    mNumSubpasses = (uint32_t) renderPassDef.subpasses.size();
    mNumAttachments = (uint32_t) renderPassDef.attachments.size();
    mpDevice = &device;
    
    // Fill in the Vulkan subpass descriptions and the attachment references for those subpasses
    mNumSubpassColorAttachments.clear();

    std::vector<VkSubpassDescription> subpassDescs;
    subpassDescs.reserve(renderPassDef.subpasses.size());
        
    for (const SubpassDef& inSubpass : renderPassDef.subpasses) {
        // Sanity checks: these  preconditions are required by Vulkan
        ASSERT((inSubpass.depthStencilAttachments.size() == 0) || (inSubpass.depthStencilAttachments.size() == 1));
        ASSERT((inSubpass.colorResolveAttachments.size() == 0) || (inSubpass.colorResolveAttachments.size() == inSubpass.colorAttachments.size()));

        // Notes:
        //  (1) Pipeline bind point MUST be sepecified: there may be compute subpasses in future!
        //  (2) Flags is currently unused, right not it's just for vendor specific extensions
        VkSubpassDescription& outSubpass = subpassDescs.emplace_back();
        outSubpass.flags = 0;
        outSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        outSubpass.inputAttachmentCount = (uint32_t) inSubpass.inputAttachments.size();
        outSubpass.pInputAttachments = inSubpass.inputAttachments.data();
        outSubpass.colorAttachmentCount = (uint32_t) inSubpass.colorAttachments.size();
        outSubpass.pColorAttachments = inSubpass.colorAttachments.data();
        outSubpass.pResolveAttachments = (!inSubpass.colorResolveAttachments.empty()) ? inSubpass.colorResolveAttachments.data() : nullptr;
        outSubpass.pDepthStencilAttachment = inSubpass.depthStencilAttachments.data();
        outSubpass.preserveAttachmentCount = (uint32_t) inSubpass.preserveAttachments.size();
        outSubpass.pPreserveAttachments = inSubpass.preserveAttachments.data();
    }
    
    // Figure out the dependencies for all subpasses (excluding external subpass dependencies, which must be synchronized externally)
    std::vector<VkSubpassDependency> subpassDependencies;
    subpassDependencies.reserve(renderPassDef.subpasses.size() * 4);

    for (uint32_t subpassIdx = 0; subpassIdx < mNumSubpasses; ++subpassIdx) {
        determineSubpassDependencies(subpassIdx, renderPassDef, subpassDependencies);
    }

    // Create info for the actual render pass object
    const VkFuncs& vkFuncs = device.getVkFuncs();

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t) renderPassDef.attachments.size();
    renderPassInfo.pAttachments = renderPassDef.attachments.data();
    renderPassInfo.subpassCount = (uint32_t) subpassDescs.size();
    renderPassInfo.pSubpasses = subpassDescs.data();
    renderPassInfo.dependencyCount = (uint32_t) subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();

    if (vkFuncs.vkCreateRenderPass(device.getVkDevice(), &renderPassInfo, nullptr, &mVkRenderPass) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a renderpass!");
        return false;
    }

    ASSERT(mVkRenderPass);
    
    // Note: derived classes must set the 'isValid' flag to true
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup and destroy the render pass
//------------------------------------------------------------------------------------------------------------------------------------------
void BaseRenderPass::destroy() noexcept {
    // Preconditions
    ASSERT_LOG(!mbIsValid, "Should be marked as not valid prior to this base 'destroy()' call by derived classes!");
    ASSERT_LOG(((!mpDevice) || mpDevice->getVkDevice()), "Parent device must still be valid if defined!");

    // Do the cleanup
    mNumSubpasses = 0;
    mNumAttachments = 0;
    mNumSubpassColorAttachments.clear();

    if (mVkRenderPass) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyRenderPass(mpDevice->getVkDevice(), mVkRenderPass, nullptr);
        mVkRenderPass = VK_NULL_HANDLE;
    }

    mpDevice = nullptr;
}

END_NAMESPACE(vgl)
