#pragma once

#include "SubpassDef.h"

#include <vector>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Definition of a render pass: specifies render pass attachments and which attachments are used in each subpass
//------------------------------------------------------------------------------------------------------------------------------------------
struct RenderPassDef {
    // All of the color and depth/stencil attachments and inputs for the renderpass
    std::vector<VkAttachmentDescription> attachments;

    // All of the subpasses within the renderpass
    std::vector<SubpassDef> subpasses;

    bool findPrevSubpassAttachmentUsage(
        const uint32_t currentSubpassIdx,
        const uint32_t attachmentIdx,
        const AttachmentUsageFlags usageMask,
        uint32_t& referencingSubpassIdx,
        AttachmentUsageFlags& referencingUsageFlags
    ) const noexcept;

    bool hasValidResolveAttachmentLists() const noexcept;
};

END_NAMESPACE(vgl)
