#pragma once

#include "CmdPool.h"
#include "DeviceMemMgr.h"
#include "RetirementMgr.h"
#include "RingbufferMgr.h"
#include "TransferMgr.h"

BEGIN_NAMESPACE(vgl)

class CmdBuffer;
class Semaphore;
class PhysicalDevice;
class VulkanInstance;
class WindowSurface;
struct CmdBufferWaitCond;
struct VkFuncs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan logical device.
// The logical device is created from the physical device and is basically a usable instance of the physical device.
//------------------------------------------------------------------------------------------------------------------------------------------
class LogicalDevice {
public:
    // Constant for an invalid queue family index.
    // Used to signify that a queue family index is not set to anything valid.
    static constexpr const uint32_t INVALID_QUEUE_FAMILY_IDX = UINT32_MAX;

    LogicalDevice(VkFuncs& vkFuncs) noexcept;
    ~LogicalDevice() noexcept;

    // Note: window surface can be omitted for headless mode!
    bool init(const PhysicalDevice& physicalDevice, const WindowSurface* const pWindowSurface) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline bool isHeadless() const noexcept { return mbIsHeadless; }
    inline const VulkanInstance* getVulkanInstance() const noexcept { return mpVulkanInstance; }
    inline const PhysicalDevice* getPhysicalDevice() const noexcept { return mpPhysicalDevice; }
    inline const WindowSurface* getWindowSurface() const noexcept { return mpWindowSurface; }
    inline VkDevice getVkDevice() const noexcept { return mVkDevice; }
    inline const VkFuncs& getVkFuncs() const noexcept { return mVkFuncs; }
    inline uint32_t getWorkQueueFamilyIdx() const noexcept { return mWorkQueueFamilyIdx; }
    inline VkQueue getWorkQueue() const noexcept { return mWorkQueue; }
    inline uint32_t getPresentationQueueFamilyIdx() const noexcept { return mPresentationQueueFamilyIdx; }
    inline VkQueue getPresentationQueue() const noexcept { return mPresentationQueue; }
    inline DeviceMemMgr& getDeviceMemMgr() noexcept { return mDeviceMemMgr; }
    inline RingbufferMgr& getRingbufferMgr() noexcept { return mRingbufferMgr; }
    inline RetirementMgr& getRetirementMgr() noexcept { return mRetirementMgr; }
    inline TransferMgr& getTransferMgr() noexcept { return mTransferMgr; }
    inline CmdPool& getCmdPool() noexcept { return mCmdPool; }

    bool submitCmdBuffer(
        const CmdBuffer& cmdBuffer,
        const std::vector<CmdBufferWaitCond>& waitConditions,
        const Semaphore* const pSignalSemaphore,
        const Fence* const pSignalFence
    ) noexcept;

    void waitUntilDeviceIdle() noexcept;

private:
    // Copy and move disallowed
    LogicalDevice(const LogicalDevice& other) = delete;
    LogicalDevice(LogicalDevice&& other) = delete;
    LogicalDevice& operator = (const LogicalDevice& other) = delete;
    LogicalDevice& operator = (LogicalDevice&& other) = delete;

    bool createDeviceAndQueues() noexcept;
    void destroyDeviceAndQueues() noexcept;
    bool chooseOptimalWorkQueueFamily(const std::vector<uint32_t>& validPresentationQueueFamilies) noexcept;
    bool chooseOptimalPresentationQueueFamily(const std::vector<uint32_t>& validPresentationQueueFamilies) noexcept;

    bool gatherCreatedQueueHandles(
        const VkDeviceQueueCreateInfo* const pQueueCreateInfos, 
        const uint32_t numQueueCreateInfos
    ) noexcept;

    VkFuncs&                mVkFuncs;                       // Pointers to Vulkan API functions
    bool                    mbIsValid;                      // True if the logical device was created & initialized successfully
    bool                    mbIsHeadless;                   // If true we are not presenting to a window surface, the device is in 'headless' mode
    VulkanInstance*         mpVulkanInstance;               // Vulkan API instance used by the device
    const PhysicalDevice*   mpPhysicalDevice;               // The physical Vulkan device this logical device is based on
    const WindowSurface*    mpWindowSurface;                // The window surface to use for the logical device
    VkDevice                mVkDevice;                      // The Vulkan handle for the logical device
    uint32_t                mWorkQueueFamilyIdx;            // The index of the queue family that supports graphics + compute + transfer operations
    VkQueue                 mWorkQueue;                     // Handle to the queue that supports graphics + compute + transfer operations
    uint32_t                mPresentationQueueFamilyIdx;    // The index of the queue family the presentation queue belongs to
    VkQueue                 mPresentationQueue;             // Handle to the presentation queue for the device
    CmdPool                 mCmdPool;                       // A command pool used by the transfer manager and possibly application code too
    DeviceMemMgr            mDeviceMemMgr;                  // Used for allocating Vulkan device memory
    RingbufferMgr           mRingbufferMgr;                 // Keeps track of which ringbuffer we are on and holds sync primitives for ringbuffers
    RetirementMgr           mRetirementMgr;                 // Manages the retirement of resources that may be in used by currently processing frames
    TransferMgr             mTransferMgr;                   // Used for managing transfers to GPU memory

    // Temporary buffers used during command buffer submit.
    // These are re-used for each submit.
    struct {
        std::vector<VkSemaphore>            waitSemaphores;
        std::vector<VkPipelineStageFlags>   waitStageFlags;
    } mCmdBufferSubmitHelpers;
};

END_NAMESPACE(vgl)
