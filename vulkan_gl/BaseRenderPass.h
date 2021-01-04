#pragma once

#include "Macros.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;
struct RenderPassDef;

//------------------------------------------------------------------------------------------------------------------------------------------
// Base definition for all renderpass types.
// Handles the actual work of creating the Vulkan renderpass object, and whatever specific logic the derived renderpass type needs.
// For example a derived renderpass might manage graphics pipeline caches, if required...
//------------------------------------------------------------------------------------------------------------------------------------------
class BaseRenderPass {
public:
    inline bool isValid() const noexcept { return mbIsValid; }
    inline uint32_t getNumSubpasses() const noexcept { return mNumSubpasses; }
    inline uint32_t getNumAttachments() const noexcept { return mNumAttachments; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkRenderPass getVkRenderPass() const noexcept { return mVkRenderPass; }

    uint32_t getNumSubpassColorAttachments(const uint32_t subpassIndex) const noexcept;
    
protected:
    // Copy and move disallowed
    BaseRenderPass(const BaseRenderPass& other) = delete;
    BaseRenderPass(BaseRenderPass&& other) = delete;
    BaseRenderPass& operator = (const BaseRenderPass& other) = delete;
    BaseRenderPass& operator = (BaseRenderPass&& other) = delete;
    
    BaseRenderPass() noexcept;
    ~BaseRenderPass() noexcept;

    bool init(LogicalDevice& device, const RenderPassDef& renderPassDef) noexcept;
    void destroy() noexcept;
    
    bool                    mbIsValid;
    uint32_t                mNumSubpasses;
    uint32_t                mNumAttachments;
    std::vector<uint32_t>   mNumSubpassColorAttachments;
    LogicalDevice*          mpDevice;
    VkRenderPass            mVkRenderPass;
};

END_NAMESPACE(vgl)
