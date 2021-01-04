#include "SubpassDef.h"

#include "AttachmentUsageFlags.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given attachment is in use as a depth stencil attachment
//------------------------------------------------------------------------------------------------------------------------------------------
bool SubpassDef::isAttachmentInUseForDepthStencil(const uint32_t attachmentIdx) const noexcept {
    for (const VkAttachmentReference& attachment : depthStencilAttachments) {
        if (attachment.attachment == attachmentIdx)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility which returns the flags for how a particular attachment is referenced in this subpass.
// Returns '0' if the attachment is not read from or written to during this subpass.
// A mask is also supplied to filter the result to only certain usages.
//------------------------------------------------------------------------------------------------------------------------------------------
AttachmentUsageFlags SubpassDef::getAttachmentUsageFlags(
    const uint32_t attachmentIndex,
    const AttachmentUsageFlags usageMask
) const noexcept {
    AttachmentUsageFlags usageFlagsOut = 0;

    // Search for write references
    if (usageMask & AttachmentUsageFlagBits::WRITE) {
        for (const VkAttachmentReference& attachment : colorAttachments) {
            if (attachment.attachment == attachmentIndex) {
                usageFlagsOut |= AttachmentUsageFlagBits::WRITE;
                break;
            }
        }

        if ((usageFlagsOut & AttachmentUsageFlagBits::WRITE) == 0) {
            for (const VkAttachmentReference& attachment : colorResolveAttachments) {
                if (attachment.attachment == attachmentIndex) {
                    usageFlagsOut |= AttachmentUsageFlagBits::WRITE;
                    break;
                }
            }

            if ((usageFlagsOut & AttachmentUsageFlagBits::WRITE) == 0) {
                for (const VkAttachmentReference& attachment : depthStencilAttachments) {
                    if (attachment.attachment == attachmentIndex) {
                        usageFlagsOut |= AttachmentUsageFlagBits::WRITE;
                        break;
                    }
                }
            }
        }
    }

    // Search for read references
    if (usageMask & AttachmentUsageFlagBits::READ) {
        for (const VkAttachmentReference& attachment : inputAttachments) {
            if (attachment.attachment == attachmentIndex) {
                usageFlagsOut |= AttachmentUsageFlagBits::READ;
                break;
            }
        }
    }

    return usageFlagsOut;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the number of entries in the resolve attachment list is valid
//------------------------------------------------------------------------------------------------------------------------------------------
bool SubpassDef::hasValidResolveAttachmentList() const noexcept {
    return (colorResolveAttachments.empty() || (colorResolveAttachments.size() == colorAttachments.size()));
}

END_NAMESPACE(vgl)
