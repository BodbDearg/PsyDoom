#include "RawBuffer.h"

#include "DeviceMemMgr.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "RetirementMgr.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized raw buffer
//------------------------------------------------------------------------------------------------------------------------------------------
RawBuffer::RawBuffer() noexcept
    : mSize(0)
    , mOrigRequestedSize(0)
    , mpDevice(nullptr)
    , mVkBuffer(VK_NULL_HANDLE)
    , mDeviceMemAlloc()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a raw buffer to this object
//------------------------------------------------------------------------------------------------------------------------------------------
RawBuffer::RawBuffer(RawBuffer&& other) noexcept
    : mSize(other.mSize)
    , mOrigRequestedSize(other.mOrigRequestedSize)
    , mpDevice(other.mpDevice)
    , mVkBuffer(other.mVkBuffer)
    , mDeviceMemAlloc(other.mDeviceMemAlloc)
{
    other.mSize = 0;
    other.mOrigRequestedSize = 0;
    other.mpDevice = nullptr;
    other.mVkBuffer = VK_NULL_HANDLE;
    other.mDeviceMemAlloc = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the raw buffer
//------------------------------------------------------------------------------------------------------------------------------------------
RawBuffer::~RawBuffer() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to initialize the raw buffer and returns 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool RawBuffer::init(
    LogicalDevice& device,
    const uint64_t size,
    const DeviceMemAllocMode allocMode,
    const VkBufferUsageFlags bufferUsageFlags
) noexcept {
    // Preconditions
    ASSERT_LOG((!isValid()), "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());

    // Creating a zero sized buffer always fails!
    if (size <= 0) {
        ASSERT_FAIL("Can't create a zero sized buffer!");
        return false;
    }

    // Save for later
    mpDevice = &device;
    mOrigRequestedSize = size;

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!isValid()) {
            destroy(true, true);
        }
    });

    // Get the device memory allocator and estimate out how many bytes would actually be allocated for this request after rounding.
    // We'll size the buffer accordingly, so client code can make use of the extra bytes if it wants:
    DeviceMemMgr& deviceMemMgr = mpDevice->getDeviceMemMgr();
    const uint64_t roundedAllocSize = deviceMemMgr.estimateAllocSize(size);

    if (roundedAllocSize <= 0) {
        ASSERT_FAIL("Alloc will not succeed, too big or 0 bytes!");
        return false;
    }

    ASSERT_LOG(roundedAllocSize >= size, "Rounded size should be at least as big as requested size!");

    // Create the Vulkan buffer object:
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = roundedAllocSize;
    bufferCreateInfo.usage = bufferUsageFlags;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;   // Only used in one queue family at a time

    const VkDevice vkDevice = mpDevice->getVkDevice();
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    if (vkFuncs.vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &mVkBuffer) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to allocate a vulkan buffer!");
        return false;
    }

    ASSERT(mVkBuffer);

    // Nab the memory requirements of the buffer, which memory types it is allowed to use and
    // what the exact memory requirements including size & alignment etc.
    VkMemoryRequirements memReqs;
    vkFuncs.vkGetBufferMemoryRequirements(vkDevice, mVkBuffer, &memReqs);

    // Try to do a device memory alloc
    deviceMemMgr.alloc(memReqs.size, memReqs.memoryTypeBits, memReqs.alignment, allocMode, mDeviceMemAlloc);

    // If that fails then bail out and return false for failure
    if (mDeviceMemAlloc.size <= 0)
        return false;

    // Okay, now that we have the buffer bind it to it's memory
    const VkResult bindResult = vkFuncs.vkBindBufferMemory(vkDevice, mVkBuffer, mDeviceMemAlloc.vkDeviceMemory, mDeviceMemAlloc.offset);

    if (bindResult != VK_SUCCESS) {
        ASSERT_FAIL("Failed to associate the Vulkan buffer with it's allocated memory!");
        return false;
    }

    // All good! Save the size of the buffer to mark it as valid:
    mSize = roundedAllocSize;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the raw buffer and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void RawBuffer::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only do if we need to cleanup
    const bool bIsValid = isValid();

    if ((!bIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Gradual 'retirement' logic if specified and possible:
    if ((!bImmediately) && bIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Regular cleanup logic: cleanup the Vulkan buffer.
    // Note: clear size field first since it determines validity!
    mSize = 0;

    if (mVkBuffer) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyBuffer(mpDevice->getVkDevice(), mVkBuffer, nullptr);
        mVkBuffer = VK_NULL_HANDLE;
    }

    // Cleanup the device memory alloc
    if (mDeviceMemAlloc.size > 0) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        DeviceMemMgr& deviceMemMgr = mpDevice->getDeviceMemMgr();
        deviceMemMgr.dealloc(mDeviceMemAlloc);
    }

    // Cleanup everything else
    mpDevice = nullptr;
    mOrigRequestedSize = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the raw buffer has been successfully initialized
//------------------------------------------------------------------------------------------------------------------------------------------
bool RawBuffer::isValid() const noexcept {
    return (mSize != 0);
}

END_NAMESPACE(vgl)
