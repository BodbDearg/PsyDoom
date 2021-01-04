#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan command pool.
// A command pool manages the memory for and is used to create one or more command buffers.
//------------------------------------------------------------------------------------------------------------------------------------------
class CmdPool {
public:
    CmdPool() noexcept;
    CmdPool(CmdPool&& other) noexcept;
    ~CmdPool() noexcept;
    
    bool init(LogicalDevice& device, const uint32_t queueFamilyIdx, const bool bIsTransient) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;
    
    inline bool isValid() const noexcept { return mbIsValid; }
    inline bool isTransient() const noexcept { return mbIsTransient; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkCommandPool getVkCommandPool() const noexcept { return mVkCommandPool; }

    void reset(const VkCommandBufferResetFlags resetFlags) noexcept;
    
private:
    // Copy and move assign disallowed
    CmdPool(const CmdPool& other) = delete;
    CmdPool& operator = (const CmdPool& other) = delete;
    CmdPool& operator = (CmdPool&& other) = delete;
    
    bool                mbIsValid;          // True if the pool has been validly initialized/created
    bool                mbIsTransient;      // True if command buffers allocated from the pool are very short lived
    uint32_t            mQueueFamilyIdx;    // The index of the device queue family this pool belongs to
    LogicalDevice*      mpDevice;           // The device the pool belongs to
    VkCommandPool       mVkCommandPool;     // The underlying Vulkan command pool
};

END_NAMESPACE(vgl)
