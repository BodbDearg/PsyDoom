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

    // Additional subpass dependencies; these can be used to specify dependencies on external subpasses.
    // These dependencies cannot be automatically inferred based on the information in this data structure.
    std::vector<VkSubpassDependency> extraSubpassDeps;

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
