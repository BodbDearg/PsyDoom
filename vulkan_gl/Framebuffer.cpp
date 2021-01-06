#include "Framebuffer.h"

#include "Asserts.h"
#include "BaseRenderPass.h"
#include "BaseTexture.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "RetirementMgr.h"
#include "Swapchain.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized framebuffer
//------------------------------------------------------------------------------------------------------------------------------------------
Framebuffer::Framebuffer() noexcept
    : mbIsValid(false)
    , mWidth()
    , mHeight()
    , mpDevice(nullptr)
    , mVkFramebuffer(VK_NULL_HANDLE)
    , mAttachmentImages()
    , mAttachmentImageViews()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a framebuffer to this object
//------------------------------------------------------------------------------------------------------------------------------------------
Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mpDevice(other.mpDevice)
    , mVkFramebuffer(other.mVkFramebuffer)
    , mAttachmentImages(std::move(other.mAttachmentImages))
    , mAttachmentImageViews(std::move(other.mAttachmentImageViews))
{
    other.mbIsValid = false;
    other.mWidth = {};
    other.mHeight = {};
    other.mpDevice = nullptr;
    other.mVkFramebuffer = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the framebuffer
//------------------------------------------------------------------------------------------------------------------------------------------
Framebuffer::~Framebuffer() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a framebuffer using the specified swapchain image as attachment 0, plus a list of other textures as additional attachments.
// For instance a second attachment can be specified for the depth buffer and so on.
// This is used for the main display framebuffer.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Framebuffer::init(
    const BaseRenderPass& renderPass,
    const Swapchain& swapchain,
    const uint32_t swapchainImageIdx,
    const std::vector<const BaseTexture*>& otherAttachments
) noexcept {
    ASSERT(renderPass.isValid());
    ASSERT(swapchain.isValid());
    ASSERT(swapchainImageIdx < swapchain.getVkImageViews().size());

    // Get the swap extent since this determines the framebuffer width and height.
    // Expect all attachments to match this size!
    const uint32_t fbWidth = swapchain.getSwapExtentWidth();
    const uint32_t fbHeight = swapchain.getSwapExtentHeight();

    // Initialize with all these attachment image views
    const uint32_t numAttachments = 1 + (uint32_t) otherAttachments.size();

    mAttachmentImages.reserve(numAttachments);
    mAttachmentImageViews.reserve(numAttachments);
    mAttachmentImages.push_back(swapchain.getVkImages()[swapchainImageIdx]);
    mAttachmentImageViews.push_back(swapchain.getVkImageViews()[swapchainImageIdx]);

    for (const BaseTexture* attachment : otherAttachments) {
        ASSERT(attachment->isValid());
        ASSERT(attachment->getWidth() == fbWidth);
        ASSERT(attachment->getHeight() == fbHeight);

        mAttachmentImages.push_back(attachment->getVkImage());
        mAttachmentImageViews.push_back(attachment->getVkImageView());
    }

    return initInternal(renderPass, fbWidth, fbHeight);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a framebuffer using the specified list of attachments.
// This is used for render to texture framebuffers.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Framebuffer::init(
    const BaseRenderPass& renderPass,
    const std::vector<const BaseTexture*>& attachments
) noexcept {
    ASSERT(renderPass.isValid());
    ASSERT(attachments.size() > 0);

    // Determine the framebuffer width and height from the first attachment.
    // Expect all framebuffer attachments after this to be equally sized!
    const BaseTexture& attachment0 = *attachments.front();
    const uint32_t fbWidth = attachment0.getWidth();
    const uint32_t fbHeight = attachment0.getHeight();

    // Initialize with all these attachment image views
    const uint32_t numAttachments = (uint32_t) attachments.size();

    mAttachmentImages.reserve(numAttachments);
    mAttachmentImageViews.reserve(numAttachments);

    for (const BaseTexture* attachment : attachments) {
        ASSERT(attachment->isValid());
        ASSERT(attachment->getWidth() == fbWidth);
        ASSERT(attachment->getHeight() == fbHeight);

        mAttachmentImages.push_back(attachment->getVkImage());
        mAttachmentImageViews.push_back(attachment->getVkImageView());
    }

    return initInternal(renderPass, fbWidth, fbHeight);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the framebuffer and releases it's resources
//------------------------------------------------------------------------------------------------------------------------------------------
void Framebuffer::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Gradual 'retirement' logic if specified and possible:
    if ((!bImmediately) && mbIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Regular destruction logic:
    mbIsValid = false;
    mAttachmentImages.clear();
    mAttachmentImageViews.clear();

    if (mVkFramebuffer) {
        ASSERT(mpDevice);
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyFramebuffer(mpDevice->getVkDevice(), mVkFramebuffer, nullptr);
        mVkFramebuffer = VK_NULL_HANDLE;
    }

    mpDevice = nullptr;
    mHeight = {};
    mWidth = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// This actually does most of the init work for the framebuffer for all use cases
//------------------------------------------------------------------------------------------------------------------------------------------
bool Framebuffer::initInternal(
    const BaseRenderPass& renderPass,
    const uint32_t fbWidth,
    const uint32_t fbHeight
) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(renderPass.isValid());
    ASSERT(renderPass.getDevice()->getVkDevice());
    ASSERT(fbWidth > 0);
    ASSERT(fbHeight > 0);
    ASSERT(mAttachmentImages.size() > 0);
    ASSERT(mAttachmentImageViews.size() > 0);
    ASSERT(mAttachmentImages.size() == mAttachmentImageViews.size());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true, true);
        }
    });

    // Save basic fields
    LogicalDevice& device = *renderPass.getDevice();
    mWidth = fbWidth;
    mHeight = fbHeight;
    mpDevice = &device;
    
    // Create the framebuffer
    VkFramebufferCreateInfo fbCreateInfo = {};
    fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbCreateInfo.renderPass = renderPass.getVkRenderPass();
    fbCreateInfo.attachmentCount = static_cast<uint32_t>(mAttachmentImages.size());
    fbCreateInfo.pAttachments = mAttachmentImageViews.data();
    fbCreateInfo.width = fbWidth;
    fbCreateInfo.height = fbHeight;
    fbCreateInfo.layers = 1;

    const VkFuncs& vkFuncs = device.getVkFuncs();

    if (vkFuncs.vkCreateFramebuffer(device.getVkDevice(), &fbCreateInfo, nullptr, &mVkFramebuffer) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a framebuffer!");
        return false;
    }

    ASSERT(mVkFramebuffer);

    // All went well!
    mbIsValid = true;
    return true;
}

END_NAMESPACE(vgl)
