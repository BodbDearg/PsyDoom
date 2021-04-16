#pragma once

#include "Macros.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class BaseTexture;
class LogicalDevice;
class RenderPass;
class Swapchain;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan Framebuffer.
// A framebuffer ties together the image view and render pass.
// Note: All images or textures used in a framebuffer are expected to be the same size!
//------------------------------------------------------------------------------------------------------------------------------------------
class Framebuffer {
public:
    Framebuffer() noexcept;
    Framebuffer(Framebuffer&& other) noexcept;
    ~Framebuffer() noexcept;

    bool init(
        const RenderPass& renderPass,
        const Swapchain& swapchain,
        const uint32_t swapchainImageIdx,
        const std::vector<const BaseTexture*>& otherAttachments
    ) noexcept;

    bool init(
        const RenderPass& renderPass,
        const std::vector<const BaseTexture*>& attachments
    ) noexcept;

    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline uint32_t getWidth() const noexcept { return mWidth; }
    inline uint32_t getHeight() const noexcept { return mHeight; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkFramebuffer getVkFramebuffer() const noexcept { return mVkFramebuffer; }
    inline const std::vector<VkImage> getAttachmentImages() const noexcept { return mAttachmentImages; }
    inline const std::vector<VkImageView> getAttachmentImageViews() const noexcept { return mAttachmentImageViews; }

private:
    // Copy and move assign disallowed
    Framebuffer(const Framebuffer& other) = delete;
    Framebuffer& operator = (const Framebuffer& other) = delete;
    Framebuffer& operator = (Framebuffer&& other) = delete;

    bool initInternal(
        const RenderPass& renderPass,
        const uint32_t fbWidth,
        const uint32_t fbHeight
    ) noexcept;

    bool                        mbIsValid;
    uint32_t                    mWidth;
    uint32_t                    mHeight;
    LogicalDevice*              mpDevice;
    VkFramebuffer               mVkFramebuffer;
    std::vector<VkImage>        mAttachmentImages;
    std::vector<VkImageView>    mAttachmentImageViews;
};

END_NAMESPACE(vgl)
