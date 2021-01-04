#include "Fence.h"

#include "Asserts.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized fence
//------------------------------------------------------------------------------------------------------------------------------------------
Fence::Fence() noexcept
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mVkFence(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a fence to this object
//------------------------------------------------------------------------------------------------------------------------------------------
Fence::Fence(Fence&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mpDevice(other.mpDevice)
    , mVkFence(other.mVkFence)
{
    other.mbIsValid = false;
    other.mpDevice = nullptr;
    other.mVkFence = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the fence
//------------------------------------------------------------------------------------------------------------------------------------------
Fence::~Fence() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to initialize the fence and returns 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool Fence::init(LogicalDevice& device, const InitMode initMode) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // Save for later cleanup
    mpDevice = &device;

    // Create the fence itself:
    const VkFuncs& vkFuncs = device.getVkFuncs();

    VkFenceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = (initMode == InitMode::SIGNALLED) ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if (vkFuncs.vkCreateFence(device.getVkDevice(), &createInfo, nullptr, &mVkFence) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a Vulkan fence!");
        return false;
    }

    // Success if we get to here:
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the fence and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void Fence::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;
    
    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Destroy the fence
    mbIsValid = false;

    if (mVkFence) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyFence(mpDevice->getVkDevice(), mVkFence, nullptr);
        mVkFence = VK_NULL_HANDLE;
    }

    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check if the fence is in a signalled state
//------------------------------------------------------------------------------------------------------------------------------------------
bool Fence::isSignalled() const noexcept {
    ASSERT(mbIsValid);
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
    const VkResult result = vkFuncs.vkGetFenceStatus(mpDevice->getVkDevice(), mVkFence);
    ASSERT((result == VK_SUCCESS) || (result == VK_NOT_READY));
    return (result == VK_SUCCESS);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reset the fence to the unsignalled state
//------------------------------------------------------------------------------------------------------------------------------------------
void Fence::resetSignal() noexcept {
    ASSERT(mbIsValid);
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
    [[maybe_unused]] const VkResult result = vkFuncs.vkResetFences(mpDevice->getVkDevice(), 1, &mVkFence);
    ASSERT(result == VK_SUCCESS);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for the fence to come into a signalled state
//------------------------------------------------------------------------------------------------------------------------------------------
void Fence::waitUntilSignalled() noexcept {
    ASSERT(mbIsValid);
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
    [[maybe_unused]] const VkResult result = vkFuncs.vkWaitForFences(mpDevice->getVkDevice(), 1, &mVkFence, VK_TRUE, UINT64_MAX);
    ASSERT(result == VK_SUCCESS);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for the specified number of seconds for the fence to become signalled.
// Returns 'true' if the wait happened within the timeout period or 'false' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Fence::waitUntilSignalledWithTimeout(const float seconds) noexcept {
    ASSERT(mbIsValid);
    ASSERT(seconds >= 0);
    const uint64_t timeoutMicroSec = static_cast<uint64_t>(static_cast<double>(seconds) * 1000000.0);
    const uint64_t timeoutNanoSec = timeoutMicroSec * 1000;

    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
    const VkResult result = vkFuncs.vkWaitForFences(mpDevice->getVkDevice(), 1, &mVkFence, VK_TRUE, timeoutNanoSec);
    return (result == VK_SUCCESS);
}

END_NAMESPACE(vgl)
