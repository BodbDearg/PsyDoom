#include "Semaphore.h"

#include "Finally.h"
#include "LogicalDevice.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized semaphore
//------------------------------------------------------------------------------------------------------------------------------------------
Semaphore::Semaphore() noexcept
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mVkSemaphore(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate semaphore to this object
//------------------------------------------------------------------------------------------------------------------------------------------
Semaphore::Semaphore(Semaphore&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mpDevice(other.mpDevice)
    , mVkSemaphore(other.mVkSemaphore)
{
    other.mbIsValid = false;
    other.mpDevice = nullptr;
    other.mVkSemaphore = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the semaphore
//------------------------------------------------------------------------------------------------------------------------------------------
Semaphore::~Semaphore() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to initialize the semaphore and returns 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool Semaphore::init(LogicalDevice& device) noexcept {
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

    // Create the semaphore itself; CreateInfo is pretty much useless ATM:
    const VkFuncs& vkFuncs = device.getVkFuncs();

    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkFuncs.vkCreateSemaphore(device.getVkDevice(), &createInfo, nullptr, &mVkSemaphore) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a Vulkan semaphore!");
        return false;
    }

    // Success!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the semaphore and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void Semaphore::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG(((!mpDevice) || mpDevice->getVkDevice()), "Parent device must still be valid if defined!");

    // Destroy the semaphore
    mbIsValid = false;

    if (mVkSemaphore) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroySemaphore(mpDevice->getVkDevice(), mVkSemaphore, nullptr);
        mVkSemaphore = VK_NULL_HANDLE;
    }

    mpDevice = nullptr;
}

END_NAMESPACE(vgl)
