#include "LogicalDevice.h"

#include "CmdBuffer.h"
#include "CmdBufferWaitCond.h"
#include "CmdPool.h"
#include "DeviceMemMgr.h"
#include "DeviceSurfaceCaps.h"
#include "Fence.h"
#include "Finally.h"
#include "PhysicalDevice.h"
#include "RetirementMgr.h"
#include "RingbufferMgr.h"
#include "Semaphore.h"
#include "TransferMgr.h"
#include "VkFuncs.h"
#include "WindowSurface.h"

BEGIN_NAMESPACE(vgl)

// Queue priority constant
static constexpr const float DEFAULT_VK_DEVICE_QUEUE_PRIORITY = 1.0f;

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized Vulkan logical device
//------------------------------------------------------------------------------------------------------------------------------------------
LogicalDevice::LogicalDevice(VkFuncs& vkFuncs) noexcept
    : mVkFuncs(vkFuncs)
    , mbIsValid(false)
    , mbIsHeadless(false)
    , mpVulkanInstance(nullptr)
    , mpPhysicalDevice(nullptr)
    , mpWindowSurface(nullptr)
    , mVkDevice(VK_NULL_HANDLE)
    , mWorkQueueFamilyIdx(INVALID_QUEUE_FAMILY_IDX)
    , mWorkQueue(VK_NULL_HANDLE)
    , mPresentationQueueFamilyIdx(INVALID_QUEUE_FAMILY_IDX)
    , mPresentationQueue(VK_NULL_HANDLE)
    , mCmdPool()
    , mDeviceMemMgr(vkFuncs)
    , mRingbufferMgr()
    , mRetirementMgr()
    , mTransferMgr()
    , mCmdBufferSubmitHelpers()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the logical device
//------------------------------------------------------------------------------------------------------------------------------------------
LogicalDevice::~LogicalDevice() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the logical device for the given physical device and window surface
//------------------------------------------------------------------------------------------------------------------------------------------
bool LogicalDevice::init(const PhysicalDevice& physicalDevice, const WindowSurface* const pWindowSurface) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT((!pWindowSurface) || pWindowSurface->isValid());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // Save these for later reference
    mbIsHeadless = (pWindowSurface == nullptr);
    mpVulkanInstance = &physicalDevice.getVulkanInstance();
    mpPhysicalDevice = &physicalDevice;
    mpWindowSurface = pWindowSurface;
    
    // Basic device setup steps:
    //
    //  1 - Create the Vulkan device and all of the queues associated with it
    //  2 - Create the device memory manager
    //  3 - Create the command pools used to create command buffers for the graphics and transfer queues
    //
    if (!createDeviceAndQueues())
        return false;

    mDeviceMemMgr.init(*this);

    if (!mCmdPool.init(*this, mWorkQueueFamilyIdx, true))
        return false;

    // Initializing other managers that are contained on the device
    if (!mRingbufferMgr.init(*this))
        return false;

    if (!mRetirementMgr.init(*this))
        return false;

    if (!mTransferMgr.init(*this))
        return false;
    
    // All went well!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the logical device and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void LogicalDevice::destroy(const bool bForceIfInvalid) noexcept {
    // Only cleanup if we have to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Now invalid (kind of)
    mbIsValid = false;

    // Ensure the device is idle before proceeding.
    // Note that this will also release any retired resources and finish pending transfers.
    waitUntilDeviceIdle();
    
    // Cleanup main objects
    mTransferMgr.destroy();
    mRetirementMgr.destroy();
    mRingbufferMgr.destroy();
    mDeviceMemMgr.destroy();
    mCmdPool.destroy();
    destroyDeviceAndQueues();

    // Cleanup everything else
    mCmdBufferSubmitHelpers.waitSemaphores.clear();
    mCmdBufferSubmitHelpers.waitSemaphores.shrink_to_fit();
    mCmdBufferSubmitHelpers.waitStageFlags.clear();
    mCmdBufferSubmitHelpers.waitStageFlags.shrink_to_fit();
    mpWindowSurface = nullptr;
    mpPhysicalDevice = nullptr;
    mpVulkanInstance = nullptr;
    mbIsHeadless = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submit a single command buffer for execution to the device. 
// Can optionally be made to wait on the given command buffer wait conditions.
// Can optionally signal a semaphore or fence, or both.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LogicalDevice::submitCmdBuffer(
    const CmdBuffer& cmdBuffer,
    const std::vector<CmdBufferWaitCond>& waitConditions,
    const Semaphore* const pSignalSemaphore,
    const Fence* const pSignalFence
) noexcept {
    // Sanity checks
    ASSERT(mbIsValid);
    ASSERT(cmdBuffer.isValid());

    // Prepare the list of semaphores that we will wait on:
    const uint32_t numWaitConditions = static_cast<uint32_t>(waitConditions.size());
    mCmdBufferSubmitHelpers.waitSemaphores.clear();
    mCmdBufferSubmitHelpers.waitStageFlags.clear();
    mCmdBufferSubmitHelpers.waitSemaphores.resize(numWaitConditions);
    mCmdBufferSubmitHelpers.waitStageFlags.resize(numWaitConditions);

    {
        VkSemaphore* pCurVkSemaphore = mCmdBufferSubmitHelpers.waitSemaphores.data();
        VkPipelineStageFlags* pCurPipelineStageFlags = mCmdBufferSubmitHelpers.waitStageFlags.data();

        for (const CmdBufferWaitCond& waitCond : waitConditions) {
            ASSERT(waitCond.pSemaphore->isValid());
            *pCurVkSemaphore = waitCond.pSemaphore->getVkSemaphore();
            *pCurPipelineStageFlags = waitCond.blockedStageFlags;
            ++pCurVkSemaphore;
            ++pCurPipelineStageFlags; 
        }
    }

    // The semaphore that will be signalled when done (if any)
    VkSemaphore signalVkSemaphore;
    uint32_t numSignalSemaphores;

    if (pSignalSemaphore) {
        ASSERT(pSignalSemaphore->isValid());
        signalVkSemaphore = pSignalSemaphore->getVkSemaphore();
        numSignalSemaphores = 1;
    } else {
        signalVkSemaphore = VK_NULL_HANDLE;
        numSignalSemaphores = 0;
    }

    // The fence that will be signalled when done (if any) and the command buffer to submit
    const VkFence signalVkFence = (pSignalFence) ? pSignalFence->getVkFence() : VK_NULL_HANDLE;
    const VkCommandBuffer vkCommandBuffer = cmdBuffer.getVkCommandBuffer();

    // Do the submit
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = numWaitConditions;
    submitInfo.pWaitSemaphores = mCmdBufferSubmitHelpers.waitSemaphores.data();
    submitInfo.pWaitDstStageMask = mCmdBufferSubmitHelpers.waitStageFlags.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCommandBuffer;
    submitInfo.signalSemaphoreCount = numSignalSemaphores;
    submitInfo.pSignalSemaphores = &signalVkSemaphore;

    if (mVkFuncs.vkQueueSubmit(mWorkQueue, 1, &submitInfo, signalVkFence) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to submit the command buffer to the graphics queue for rendering!");
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Waits for the device to be idle. All operations will complete after this.
//------------------------------------------------------------------------------------------------------------------------------------------
void LogicalDevice::waitUntilDeviceIdle() noexcept {
    // Note: no check for 'mbIsValid' since this function is invoked during the destructor.
    // It's called when the device is not 100% in a valid state:
    ASSERT(mVkDevice);

    if (mTransferMgr.isValid()) {
        // Finish up all transfers before we do the cleanup below
        mTransferMgr.executePreFrameTransferTask();
    }

    mVkFuncs.vkDeviceWaitIdle(mVkDevice);

    // Do cleanup for all ringbuffer slots if we can, since the device is now completely idle
    if (mRingbufferMgr.isValid()) {
        mRingbufferMgr.doCleanupForAllBufferSlots();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the Vulkan device and all of it's work queues
//------------------------------------------------------------------------------------------------------------------------------------------
bool LogicalDevice::createDeviceAndQueues() noexcept {
    // Sanity checks
    ASSERT(mpPhysicalDevice);
    
    // The physical device must have at least 1 valid graphics and transfer queue family
    if (mpPhysicalDevice->getGraphicsQueueFamilyIndexes().empty()) {
        ASSERT_FAIL("Device does not have any valid graphics queues for rendering!");
        return false;
    }

    if (mpPhysicalDevice->getTransferQueueFamilyIndexes().empty()) {
        ASSERT_FAIL("Device does not have any valid transfer queues for transfering data!");
        return false;
    }

    // Pick queue families to use; logic differs depending on whether we are in headless mode or not:
    if (!mbIsHeadless) {
        // Not in headless mode: there must be at least 1 valid presentation queue family in the device for the given surface!
        ASSERT(mpWindowSurface);

        DeviceSurfaceCaps deviceSurfaceCaps;
        deviceSurfaceCaps.query(*mpPhysicalDevice, *mpWindowSurface);
        const std::vector<uint32_t>& validPresentationQueueFamilies = deviceSurfaceCaps.getPresentCapableQueueFamilies();

        if (validPresentationQueueFamilies.empty()) {
            ASSERT_FAIL("Device does not have any valid presentation queues for the surface!");
            return false;
        }

        // Select the optimal queue families to use for graphics + compute (and by extension, transfer) as well as presentation
        if (!chooseOptimalWorkQueueFamily(validPresentationQueueFamilies)) {
            ASSERT_FAIL("Failed to choose an optimal work queue family!");
            return false;
        }

        if (!chooseOptimalPresentationQueueFamily(validPresentationQueueFamilies)) {
            ASSERT_FAIL("Failed to choose an optimal presentation queue family!");
            return false;
        }
    }
    else {
        // Headless mode: just create choose a queue family for general graphics + compute work but not for presentation
        if (!chooseOptimalWorkQueueFamily({})) {
            ASSERT_FAIL("Failed to choose an optimal work queue family!");
            return false;
        }
    }

    // Decide how many queues we need to create.
    // We create 1 for each unique queue family that we require.
    constexpr const uint32_t MAX_QUEUE_FAMILIES = 2;
    uint32_t requiredQueueFamilyIdxs[MAX_QUEUE_FAMILIES] = { mWorkQueueFamilyIdx, UINT32_MAX };
    uint32_t numQueuesToCreate = 1;

    if (!mbIsHeadless) {
        if (mPresentationQueueFamilyIdx != mWorkQueueFamilyIdx) {
            requiredQueueFamilyIdxs[1] = mPresentationQueueFamilyIdx;
            numQueuesToCreate = 2;
        }
    }

    // Fill in the info for each queue that we must create
    VkDeviceQueueCreateInfo requiredQueueCreateInfos[MAX_QUEUE_FAMILIES];

    {
        VkDeviceQueueCreateInfo* pCurCreateInfo = requiredQueueCreateInfos;
        VkDeviceQueueCreateInfo* const pEndCreateInfo = pCurCreateInfo + numQueuesToCreate;
        const uint32_t* pCurQueueFamilyIdx = requiredQueueFamilyIdxs;

        while (pCurCreateInfo < pEndCreateInfo) {
            *pCurCreateInfo = {};
            pCurCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            pCurCreateInfo->queueCount = 1;
            pCurCreateInfo->queueFamilyIndex = *pCurQueueFamilyIdx;
            pCurCreateInfo->pQueuePriorities = &DEFAULT_VK_DEVICE_QUEUE_PRIORITY;

            ++pCurCreateInfo;
            ++pCurQueueFamilyIdx;
        }
    }

    // Device features required: don't need anything specific currently
    VkPhysicalDeviceFeatures deviceFeatures = {};

    // Create the device itself using all of these settings
    {
        // Note: need the swapchain extension for creating the swapchain
        const char* const DEVICE_EXTENSIONS[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = requiredQueueCreateInfos;
        createInfo.queueCreateInfoCount = numQueuesToCreate;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
        createInfo.enabledExtensionCount = C_ARRAY_SIZE(DEVICE_EXTENSIONS);

        if (mVkFuncs.vkCreateDevice(mpPhysicalDevice->getVkPhysicalDevice(), &createInfo, nullptr, &mVkDevice) != VK_SUCCESS) {
            ASSERT_FAIL("Failed to create a logical device!");
            return false;
        }
    }

    // Load device level functions
    mVkFuncs.loadDeviceFuncs(mVkDevice);

    // Get the handle to all the queues created with the device
    if (!gatherCreatedQueueHandles(requiredQueueCreateInfos, numQueuesToCreate))
        return false;

    // All went well if we have got to here
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the Vulkan device and all of it's work queues
//------------------------------------------------------------------------------------------------------------------------------------------
void LogicalDevice::destroyDeviceAndQueues() noexcept {
    if (mVkDevice) {
        mVkFuncs.vkDestroyDevice(mVkDevice, nullptr);
        mVkDevice = VK_NULL_HANDLE;
    }

    // Note: queues get destroyed automatically with the device, so we only null out to cleanup the references
    mPresentationQueue = VK_NULL_HANDLE;
    mPresentationQueueFamilyIdx = INVALID_QUEUE_FAMILY_IDX;
    mWorkQueue = VK_NULL_HANDLE;
    mWorkQueueFamilyIdx = INVALID_QUEUE_FAMILY_IDX;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Chooses the optimal queue family to use that supports graphics + compute + transfer based on the given queue families that
// can be used for presentation. Tries to chose a work queue family that is the same as a present queue family.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LogicalDevice::chooseOptimalWorkQueueFamily(const std::vector<uint32_t>& validPresentationQueueFamilies) noexcept {
    // Get the queue families available
    ASSERT(mpPhysicalDevice);
    const std::vector<uint32_t>& graphicsQueueFamilies = mpPhysicalDevice->getGraphicsQueueFamilyIndexes();
    const std::vector<uint32_t>& computeQueueFamilies = mpPhysicalDevice->getComputeQueueFamilyIndexes();

    // Try finding the first graphics queue family that supports compute and is also a valid presentation queue family
    if (!validPresentationQueueFamilies.empty()) {
        for (uint32_t graphicsQueueFamily : graphicsQueueFamilies) {
            if (!Utils::containerContains(computeQueueFamilies, graphicsQueueFamily))
                continue;

            if (!Utils::containerContains(validPresentationQueueFamilies, graphicsQueueFamily))
                continue;

            mWorkQueueFamilyIdx = graphicsQueueFamily;
            return true;
        }
    }

    // No graphics queue families that are also valid presentation + compute queue families.
    // Instead use the first graphics + compute queue family we have!
    for (uint32_t graphicsQueueFamily : graphicsQueueFamilies) {
        if (!Utils::containerContains(computeQueueFamilies, graphicsQueueFamily))
            continue;

        mWorkQueueFamilyIdx = graphicsQueueFamily;
        return true;
    }

    // Selection failed if we got to here
    mWorkQueueFamilyIdx = INVALID_QUEUE_FAMILY_IDX;
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Choses the optimal presentation queue family to use based on the chosen graphics queue family.
// Tries to chose a presentation queue family that is the same as the graphics queue family.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LogicalDevice::chooseOptimalPresentationQueueFamily(const std::vector<uint32_t>& validPresentationQueueFamilies) noexcept {
    // Preconditions
    ASSERT(!mbIsHeadless);
    ASSERT(mpPhysicalDevice);

    // Get the queue families available
    const std::vector<uint32_t>& graphicsQueueFamilies = mpPhysicalDevice->getGraphicsQueueFamilyIndexes();
    const std::vector<uint32_t>& computeQueueFamilies = mpPhysicalDevice->getComputeQueueFamilyIndexes();

    // Try finding the first presentation queue family that is also a valid graphics + compute queue family
    for (const uint32_t queueFamilyIdx : validPresentationQueueFamilies) {
        const bool bIsGraphicsAndComputeQueue = (
            Utils::containerContains(graphicsQueueFamilies, queueFamilyIdx) &&
            Utils::containerContains(computeQueueFamilies, queueFamilyIdx)
        );

        if (bIsGraphicsAndComputeQueue) {
            mPresentationQueueFamilyIdx = queueFamilyIdx;
            return true;
        }
    }

    // No presentation queue families that are also graphics + compute queue families! Just choose any other presentation queue:
    if (!validPresentationQueueFamilies.empty()) {
        mPresentationQueueFamilyIdx = validPresentationQueueFamilies.front();
        return true;
    }

    // Selection failed if we got to here
    mPresentationQueueFamilyIdx = INVALID_QUEUE_FAMILY_IDX;
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gather all queue handles for created queues after device creation. Returns false on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LogicalDevice::gatherCreatedQueueHandles(
    const VkDeviceQueueCreateInfo* const pQueueCreateInfos,
    const uint32_t numQueueCreateInfos
) noexcept {
    const VkDeviceQueueCreateInfo* pCurCreateInfo = pQueueCreateInfos;
    const VkDeviceQueueCreateInfo* const pEndCreateInfo = pCurCreateInfo + numQueueCreateInfos;

    while (pCurCreateInfo < pEndCreateInfo) {
        const uint32_t queueFamilyIdx = pCurCreateInfo->queueFamilyIndex;
        const uint32_t numQueuesInFamily = pCurCreateInfo->queueCount;

        for (uint32_t queueIdxInFamily = 0; queueIdxInFamily < numQueuesInFamily; ++queueIdxInFamily) {
            VkQueue queue = VK_NULL_HANDLE;
            mVkFuncs.vkGetDeviceQueue(mVkDevice, queueFamilyIdx, queueIdxInFamily, &queue);

            if (queue) {
                if (queueFamilyIdx == mWorkQueueFamilyIdx) {
                    mWorkQueue = queue;
                }

                if (!mbIsHeadless) {
                    if (queueFamilyIdx == mPresentationQueueFamilyIdx) {
                        mPresentationQueue = queue;
                    }
                }
            }
        }

        ++pCurCreateInfo;
    }

    // Must have a valid work queue (and presentation queue, if not headless) to succeed
    if (!mWorkQueue) {
        ASSERT_FAIL("Failed to find a valid work queue!");
        return false;
    }

    if (!mbIsHeadless) {
        if (!mPresentationQueue) {
            ASSERT_FAIL("Failed to find a valid presentation queue!");
            return false;
        }
    }

    // If we got to here then this all went well
    return true;
}

END_NAMESPACE(vgl)
