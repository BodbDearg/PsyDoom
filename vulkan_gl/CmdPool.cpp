#include "CmdPool.h"

#include "Finally.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates a default invalid command pool
//------------------------------------------------------------------------------------------------------------------------------------------
CmdPool::CmdPool() noexcept
    : mbIsValid(false)
    , mbIsTransient(false)
    , mQueueFamilyIdx(LogicalDevice::INVALID_QUEUE_FAMILY_IDX)
    , mpDevice(nullptr)
    , mVkCommandPool(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Relocate the command pool to this object
//------------------------------------------------------------------------------------------------------------------------------------------
CmdPool::CmdPool(CmdPool&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mbIsTransient(other.mbIsTransient)
    , mQueueFamilyIdx(other.mQueueFamilyIdx)
    , mpDevice(other.mpDevice)
    , mVkCommandPool(other.mVkCommandPool)
{
    other.mbIsValid = false;
    other.mbIsTransient = false;
    other.mQueueFamilyIdx = LogicalDevice::INVALID_QUEUE_FAMILY_IDX;
    other.mpDevice = nullptr;
    other.mVkCommandPool = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically cleanup the command pool
//------------------------------------------------------------------------------------------------------------------------------------------
CmdPool::~CmdPool() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the command pool for the given device
//------------------------------------------------------------------------------------------------------------------------------------------
bool CmdPool::init(LogicalDevice& device, const uint32_t queueFamilyIdx, const bool bIsTransient) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());
    ASSERT(queueFamilyIdx < device.getPhysicalDevice()->getQueueFamilyProps().size());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // Save these for later reference. Will need device handle also for cleanup...
    mpDevice = &device;
    mbIsTransient = bIsTransient;
    mQueueFamilyIdx = queueFamilyIdx;

    // Create the command pool
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    const VkCommandPoolCreateFlags createFlags = (
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |           // Can reset inidivudal buffers in the pool
        (bIsTransient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0)
    );

    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = mQueueFamilyIdx;
    poolCreateInfo.flags = createFlags;

    if (vkFuncs.vkCreateCommandPool(device.getVkDevice(), &poolCreateInfo, nullptr, &mVkCommandPool) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a command pool!");
        return false;
    }

    ASSERT(mVkCommandPool);

    // All went well!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the command pool and frees its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdPool::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Destroy the pool
    mbIsValid = false;

    if (mVkCommandPool) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyCommandPool(mpDevice->getVkDevice(), mVkCommandPool, nullptr);
        mVkCommandPool = VK_NULL_HANDLE;
    }

    mQueueFamilyIdx = LogicalDevice::INVALID_QUEUE_FAMILY_IDX;
    mbIsTransient = false;
    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Resets the command pool.
// This resets any command buffers that were created from the pool back to their initial state.
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdPool::reset(const VkCommandBufferResetFlags resetFlags) noexcept {
    ASSERT(mbIsValid);
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    if (vkFuncs.vkResetCommandPool(mpDevice->getVkDevice(), mVkCommandPool, resetFlags) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to reset the command pool!");
    }
}

END_NAMESPACE(vgl)
