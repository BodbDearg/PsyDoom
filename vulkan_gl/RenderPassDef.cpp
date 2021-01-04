#include "RenderPassDef.h"

#include "Asserts.h"
#include "AttachmentUsageFlags.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility that determines the first usage of a specified attachment matching the specified usage mask starting BEFORE the given
// subpass index. Returns 'false' if no such usage was found or 'true' otherwise. If a usage is found within a subpass, both the usage
// flags and the index of the referencing subpass are returned.
//
// This function is used to determine the dependencies of attachments on their previous subpasses.
//------------------------------------------------------------------------------------------------------------------------------------------
bool RenderPassDef::findPrevSubpassAttachmentUsage(
    const uint32_t currentSubpassIdx,
    const uint32_t attachmentIdx,
    const AttachmentUsageFlags usageMask,
    uint32_t& referencingSubpassIdx,
    AttachmentUsageFlags& referencingUsageFlags
) const noexcept {
    ASSERT(currentSubpassIdx < subpasses.size());
    referencingSubpassIdx = UINT32_MAX;
    referencingUsageFlags = 0;

    const SubpassDef* const pStartSubpass = subpasses.data();
    const SubpassDef* const pEndSubpass = pStartSubpass + currentSubpassIdx;

    {
        const SubpassDef* pCurSubpass = pEndSubpass - 1;

        while (pCurSubpass >= pStartSubpass) {
            const AttachmentUsageFlags usageFlagsInSubpass = pCurSubpass->getAttachmentUsageFlags(attachmentIdx, usageMask);

            if (usageFlagsInSubpass != 0) {
                referencingSubpassIdx = (uint32_t)(pCurSubpass - pStartSubpass);
                referencingUsageFlags = usageFlagsInSubpass;
                return true;
            }

            --pCurSubpass;
        }
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the resolve attachments list for all subpasses are valid
//------------------------------------------------------------------------------------------------------------------------------------------
bool RenderPassDef::hasValidResolveAttachmentLists() const noexcept {
    for (const SubpassDef& subpass : subpasses) {
        if (!subpass.hasValidResolveAttachmentList())
            return false;
    }

    return true;
}

END_NAMESPACE(vgl)
