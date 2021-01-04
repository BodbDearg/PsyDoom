#pragma once

#include "Macros.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

typedef uint8_t AttachmentUsageFlags;

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a subpass within a renderpass.
//
// N.B: The number of color resolve attachments *MUST* match the number of color attachments if the resolve attachments list is not empty.
// Failure to ensure this property means the definition is not valid; this is so color indexes can map directly to resolve indexes.
// If resolve is not desired for an attachment then the 'VK_ATTACHMENT_UNUSED' index can be used for the resolve entry.
// See:
//  https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSubpassDescription.html
//------------------------------------------------------------------------------------------------------------------------------------------
struct SubpassDef {
    // Which attachments are used as color buffers to render to
    std::vector<VkAttachmentReference> colorAttachments;
    
    // Which of the color attachments are resolved to a single sample if multi sampled.
    // Note that this list must NOT reference an attachment that is not in 'colorAttachments'.
    std::vector<VkAttachmentReference> colorResolveAttachments;
    
    // Attachment for depth/stencil buffer if specified
    std::vector<VkAttachmentReference> depthStencilAttachments;
    
    // Which attachments are used as inputs into shaders
    std::vector<VkAttachmentReference> inputAttachments;

    // Which attachments indexes are resolved are preserved in their current form to the next stage
    std::vector<uint32_t> preserveAttachments;

    bool isAttachmentInUseForDepthStencil(const uint32_t attachmentIdx) const noexcept;

    AttachmentUsageFlags getAttachmentUsageFlags(
        const uint32_t attachmentIndex,
        const AttachmentUsageFlags usageMask
    ) const noexcept;

    bool hasValidResolveAttachmentList() const noexcept;
};

END_NAMESPACE(vgl)
