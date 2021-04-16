#pragma once

#include "Macros.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;
struct RenderPassDef;

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a Vulkan render pass, which is a description of the framebuffer, it's inputs, outputs and rendering stages.
// Initializes the render pass using the specified input definition.
//------------------------------------------------------------------------------------------------------------------------------------------
class RenderPass {
public:
    RenderPass() noexcept;
    ~RenderPass() noexcept;

    bool init(LogicalDevice& device, const RenderPassDef& renderPassDef) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline uint32_t getNumSubpasses() const noexcept { return mNumSubpasses; }
    inline uint32_t getNumAttachments() const noexcept { return mNumAttachments; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkRenderPass getVkRenderPass() const noexcept { return mVkRenderPass; }

    uint32_t getNumSubpassColorAttachments(const uint32_t subpassIndex) const noexcept;

private:
    // Copy and move disallowed
    RenderPass(const RenderPass& other) = delete;
    RenderPass(RenderPass&& other) = delete;
    RenderPass& operator = (const RenderPass& other) = delete;
    RenderPass& operator = (RenderPass&& other) = delete;

    bool                    mbIsValid;
    uint32_t                mNumSubpasses;
    uint32_t                mNumAttachments;
    std::vector<uint32_t>   mNumSubpassColorAttachments;
    LogicalDevice*          mpDevice;
    VkRenderPass            mVkRenderPass;
};

END_NAMESPACE(vgl)
