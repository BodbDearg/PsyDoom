#include "Buffer.h"

#include "RawBuffer.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "TransferMgr.h"
#include "TransferTask.h"

#include <cstring>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: returns the 'VkBufferUsageFlags' for a given buffer usage mode
//------------------------------------------------------------------------------------------------------------------------------------------
static inline VkBufferUsageFlags getVkBufferUsageFlagsForBufferUsageMode(const BufferUsageMode usageMode, const bool bIsResizable) noexcept {
    switch (usageMode) {
        case BufferUsageMode::STATIC:
        case BufferUsageMode::DYNAMIC: {
            // Note: if the buffer is resizable then we might need to copy back out from it to a new buffer.
            // Otherwise we can hint to the driver that we will only ever transfer TO the buffer.
            if (bIsResizable) {
                return VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            } else {
                return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
        }
        
        case BufferUsageMode::IMMEDIATE:
            return 0;
    }

    ASSERT_FAIL("Unhandled buffer usage mode!");
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized buffer
//------------------------------------------------------------------------------------------------------------------------------------------
Buffer::Buffer() noexcept
    : mbIsValid(false)
    , mUsageFlags()
    , mUsageMode()
    , mbIsResizable(false)
    , mbIsLocked(false)
    , mLockedOffset(0)
    , mLockedSize(0)
    , mLockedVkBuffer(VK_NULL_HANDLE)
    , mpLockedBytes(nullptr)
    , mBuffer()
    , mStagingBuffer()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a buffer to this object
//------------------------------------------------------------------------------------------------------------------------------------------
Buffer::Buffer(Buffer&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mUsageFlags(other.mUsageFlags)
    , mUsageMode(other.mUsageMode)
    , mbIsResizable(other.mbIsResizable)
    , mbIsLocked(other.mbIsLocked)
    , mLockedOffset(other.mLockedOffset)
    , mLockedSize(other.mLockedSize)
    , mLockedVkBuffer(other.mLockedVkBuffer)
    , mpLockedBytes(other.mpLockedBytes)
    , mBuffer(std::move(other.mBuffer))
    , mStagingBuffer(std::move(other.mStagingBuffer))
{
    other.mbIsValid = false;
    other.mUsageFlags = {};
    other.mUsageMode = {};
    other.mbIsResizable = false;
    other.mbIsLocked = false;
    other.mLockedOffset = 0;
    other.mLockedSize = 0;
    other.mLockedVkBuffer = VK_NULL_HANDLE;
    other.mpLockedBytes = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the buffer
//------------------------------------------------------------------------------------------------------------------------------------------
Buffer::~Buffer() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the buffer with the specified byte size and other usage parameters
//------------------------------------------------------------------------------------------------------------------------------------------
bool Buffer::initWithByteCount(
    LogicalDevice& device,
    const VkBufferUsageFlags baseUsageFlags,
    const BufferUsageMode usageMode,
    const uint64_t sizeInBytes,
    const bool bIsResizable
) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());
    ASSERT(sizeInBytes > 0);

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // Save for later reference
    mUsageFlags = (baseUsageFlags | getVkBufferUsageFlagsForBufferUsageMode(usageMode, mbIsResizable));
    mUsageMode = usageMode;
    mbIsResizable = bIsResizable;

    // Create the main raw buffer for this buffer and prefer to make a device local buffer in most
    // cases unless the usage mode is set to 'immediate':
    const DeviceMemAllocMode mainBufferMemAllocMode = (usageMode == BufferUsageMode::IMMEDIATE) 
        ? DeviceMemAllocMode::REQUIRE_HOST_VISIBLE
        : DeviceMemAllocMode::PREFER_DEVICE_LOCAL;
    
    if (!mBuffer.init(device, sizeInBytes, mainBufferMemAllocMode, mUsageFlags)) {
        ASSERT_FAIL("Creating the main raw buffer for a buffer failed!");
        return false;
    }

    // Create a staging buffer if the usage mode is dynamic and if the main buffer is not memory mappable.
    // With dynamic usage mode we keep a staging buffer always persistent and allocated since it's constantly needed.
    // If the usage mode is static however we allocate a temporary buffer through the transfer manager and discard
    // immediately once it's no longer in use.
    if (usageMode == BufferUsageMode::DYNAMIC) {
        if (!mBuffer.getBytes()) {
            if (!mStagingBuffer.init(
                    device,
                    mBuffer.getSize(),  // Make sure this is at least as big as the main buffers allocated size
                    DeviceMemAllocMode::REQUIRE_HOST_VISIBLE,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                )
            ) {
                ASSERT_FAIL("Creating the raw staging buffer for a buffer failed!");
                return false;
            }
        }
    }
    
    // Success!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the buffer and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void Buffer::destroy(const bool bImmediateCleanup, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;
    
    // Preconditions: note not clearing out lock details since they should already be cleared (due to no lock) at this point
    ASSERT_LOG((!getDevice()) || getDevice()->getVkDevice(), "Parent device must still be valid if defined!");
    ASSERT_LOG((!mbIsLocked), "Destroying a buffer that has not been unlocked!");
    
    // Destroy the buffer
    mbIsValid = false;
    mStagingBuffer.destroy(bImmediateCleanup);
    mBuffer.destroy(bImmediateCleanup);
    mbIsResizable = false;
    mUsageMode = {};
    mUsageFlags = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the actual allocated size of the buffer in terms of bytes.
// This may be larger than the requested size due to the way memory allocation works.
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t Buffer::getSizeInBytes() const noexcept {
    return mBuffer.getSize();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the original requested size of the buffer in terms of bytes
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t Buffer::getOrigRequestedSizeInBytes() const noexcept {
    return mBuffer.getOrigRequestedSize();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Lock a specified byte range of the buffer for filling and return a pointer to the locked region of the buffer.
//
// Notes:
//  (1) If locking the requested region fails then a null pointer is returned.
//  (2) The buffer region given must be in range.
//  (3) The locked region return is completely undefined upon locking, and may not contain any previous data.
//  (4) The buffer must be unlocked in order for it's contents to be uploaded to GPU memory.
//  (5) The unlock must happen before the draw is commenced, undefined behavior or crashes may result otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
std::byte* Buffer::lockBytes(const uint64_t offsetInBytes, const uint64_t sizeInBytes) noexcept {
    // Preconditions: must be valid and device must still be valid
    ASSERT(mbIsValid);
    LogicalDevice* const pDevice = getDevice();
    ASSERT(pDevice && pDevice->getVkDevice());

    // Preconditions: range to lock must be in bounds
    ASSERT(offsetInBytes < mBuffer.getSize());
    ASSERT(offsetInBytes + sizeInBytes <= mBuffer.getSize());

    // Preconditions: can't double lock
    ASSERT_LOG((!mbIsLocked), "Can't lock when already locked!");

    // Locking a zero sized region yields a null pointer
    if (sizeInBytes <= 0)
        return nullptr;

    // If the main buffer is mapped to a memory address (e.g shared memory architecture, or immediate mode) 
    // then we can just return a pointer to that instead...
    if (std::byte* const pBytes = mBuffer.getBytes()) {
        mbIsLocked = true;
        mLockedOffset = offsetInBytes;
        mLockedSize = sizeInBytes;
        mpLockedBytes = pBytes;
        return pBytes + offsetInBytes;
    }

    // If we use a persistent staging buffer then return a pointer to within that
    if (std::byte* const pBytes = mStagingBuffer.getBytes()) {
        mbIsLocked = true;
        mLockedOffset = offsetInBytes;
        mLockedSize = sizeInBytes;
        mLockedVkBuffer = mStagingBuffer.getVkBuffer();
        mpLockedBytes = pBytes;
        return pBytes + offsetInBytes;
    }

    // Otherwise try and create a temporary staging buffer via the transfer manager to handle the transfer
    TransferMgr& transferMgr = pDevice->getTransferMgr();
    const TransferMgr::StagingBuffer buffer = transferMgr.allocTempStagingBuffer(sizeInBytes);

    if (!buffer.pBytes) {
        ASSERT_FAIL("Failed to allocate a temporary staging buffer for a transfer!");
        return nullptr;
    }

    // All good, save the details of the buffer and return the locked memory
    mbIsLocked = true;
    mLockedOffset = offsetInBytes;
    mLockedSize = sizeInBytes;
    mLockedVkBuffer = buffer.vkBuffer;
    mpLockedBytes = buffer.pBytes;
    return mpLockedBytes;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlocks the previously locked range of the buffer.
//
// Optionally the upload can be scheduled to happen against the specified transfer task.
// If not specified, then the global 'pre-frame' transfer task is used by default.
//
// Notes:
//  (1) The buffer must have previously been locked. It is invalid to call this on an unlocked buffer.
//  (2) Unlock must be performed prior to destruction. It is invalid to destroy a locked buffer.
//  (3) The written data may or may not immediately be uploaded to the GPU. The transfer manager may need to be
//      invoked once per frame in order to ensure the data is actually uploaded onto a GPU side buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
void Buffer::unlock(TransferTask* const pTransferTaskOverride) noexcept {
    // Preconditions: must be valid and device must still be valid
    ASSERT(mbIsValid);
    LogicalDevice* const pDevice = getDevice();
    ASSERT(pDevice && pDevice->getVkDevice());

    // Preconditions: can't unlock if not locked
    ASSERT_LOG(mbIsLocked, "Buffer must first be locked to unlock!");

    // Schedule the data transfer for the locked region and clear the lock.
    // Note that we only need to do the transfer if we're actually using a staging buffer - if we're on
    // a shared memory architecture then the 'unlock()' call might basically be a no-op...
    const VkBuffer lockedVkBuffer = mLockedVkBuffer;

    if (lockedVkBuffer) {
        // Note: we only use an staging buffer offset if we are using a persistent buffer.
        // Otherwise we always use offset '0' if it was temporarily allocated just for a single transfer:
        const uint64_t stagingBufferOffset = (lockedVkBuffer == mStagingBuffer.getVkBuffer()) ? mLockedOffset : 0;
        TransferTask* pDstTask;

        if (!pTransferTaskOverride) {
            pDstTask = &pDevice->getTransferMgr().getPreFrameTransferTask();
        } else {
            pDstTask = pTransferTaskOverride;
        }

        pDstTask->addBufferCopy(
            lockedVkBuffer,
            mBuffer.getVkBuffer(),
            stagingBufferOffset,
            mLockedOffset,
            mLockedSize
        );

        mLockedVkBuffer = VK_NULL_HANDLE;
    }

    mbIsLocked = false;
    mLockedOffset = 0;
    mLockedSize = 0;
    mpLockedBytes = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Similar to unlock() but no attempt is made to transfer locked data to the GPU
//------------------------------------------------------------------------------------------------------------------------------------------
void Buffer::abandonLock() noexcept {
    // Preconditions: must be valid and device must still be valid
    ASSERT(mbIsValid);
    ASSERT(getDevice() && getDevice()->getVkDevice());

    // Preconditions: can't unlock if not locked
    ASSERT_LOG(mbIsLocked, "Buffer must first be locked to abandon lock!");

    // Abandon the lock
    mbIsLocked = false;
    mLockedOffset = 0;
    mLockedSize = 0;
    mLockedVkBuffer = VK_NULL_HANDLE;
    mpLockedBytes = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Resizes the buffer memory.
// Note: the buffer must be specified as being resizable in order for this to be allowed.
//
// Notes:
//  (1) The size of the locked area is clamped to be within the bounds of the new buffer.
//      Therefore you can specify UINT64_MAX for instance to lock the entire buffer.
//  (2) Discard the old data when not needed for higher performance and to avoid expensive copies.
//      This can be performed by omitting certain resize flags.
//  (3) If the buffer is made bigger then new regions of memory are undefined, in spite of whether data is preserved or not.
//  (4) Note that if resizing fails or re-locking fails then the buffer may become invalidated and destroyed as a result of this operation.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Buffer::resizeToByteCount(
    const uint64_t newSizeInBytes,
    const ResizeFlags resizeFlags,
    const uint64_t newLockOffsetInBytes,
    const uint64_t newLockSizeInBytes
) noexcept {
    // Preconditions: must be valid and device must still be valid
    ASSERT(mbIsValid);
    LogicalDevice* const pDevice = getDevice();
    ASSERT(pDevice && pDevice->getVkDevice());

    // Precondition: the buffer must have been created as resizable
    ASSERT(mbIsResizable);

    // If new size is the same as current then there is nothing to do
    if (newSizeInBytes == getSizeInBytes())
        return true;

    // Tear down this buffer and preserve what we need temporarily
    const VkBufferUsageFlags    usageFlags          = mUsageFlags;
    const BufferUsageMode       usageMode           = mUsageMode;
    const bool                  bIsResizable        = mbIsResizable;
    const bool                  bWasLocked          = mbIsLocked;
    const uint64_t              oldLockedOffset     = mLockedOffset;
    const uint64_t              oldLockedSize       = mLockedSize;
    std::byte* const            pOldLockedBytes     = mpLockedBytes;

    mbIsValid       = false;
    mUsageFlags     = {};
    mUsageMode      = {};
    mbIsResizable   = false;
    mbIsLocked      = false;
    mLockedOffset   = 0;
    mLockedSize     = 0;
    mLockedVkBuffer = VK_NULL_HANDLE;
    mpLockedBytes   = nullptr;
    
    RawBuffer oldBuffer(std::move(mBuffer));
    RawBuffer oldStagingBuffer(std::move(mStagingBuffer));

    // Destroy the old buffers immediately if we can:
    const bool bKeepData = ((resizeFlags & ResizeFlagBits::KEEP_DATA) != 0);
    const bool bKeepLockedData = ((resizeFlags & ResizeFlagBits::KEEP_LOCKED_DATA) != 0);
    const bool bMainBufferIsStagingBuffer = (oldBuffer.getBytes() != nullptr);
    const bool bDestroyStagingBufferImmediately = ((!bKeepLockedData) || (!bWasLocked));
    
    if (bDestroyStagingBufferImmediately) {
        oldStagingBuffer.destroy(true);
    }

    if ((!bKeepData) && ((!bMainBufferIsStagingBuffer) || bDestroyStagingBufferImmediately)) {
        oldBuffer.destroy(true);
    }

    // Recreate the buffer with the new size, bail out if that fails
    if (!initWithByteCount(*pDevice, mUsageFlags, usageMode, newSizeInBytes, bIsResizable))
        return false;

    // Re-lock the buffer if a non zero lock size is specified
    const uint64_t actualNewSize = getSizeInBytes();

    if (newLockSizeInBytes > 0 && newLockOffsetInBytes < actualNewSize) {
        const uint64_t maxLockSize = actualNewSize - newLockOffsetInBytes;
        const uint64_t actualNewLockSize = std::min(maxLockSize, newLockSizeInBytes);
        
        if (!lockBytes(newLockOffsetInBytes, actualNewLockSize)) {
            ASSERT_FAIL("Re-lock after buffer resize failed!");
            destroy(true);
            return false;
        }
    }

    // Preserving the old buffer contents if specified; arrange a transfer if required:
    bool bDidCopyRamImmediate = false;

    if (bKeepData) {
        // See if we do an immediate memory copy or schedule a transfer instead
        const uint64_t sizeToPreserve = std::min(oldBuffer.getSize(), newSizeInBytes);
        const bool bCopyRamImmediate = ((resizeFlags & ResizeFlagBits::COPY_RAM_IMMEDIATE) != 0);

        if (bCopyRamImmediate && oldBuffer.getBytes() && mBuffer.getBytes()) {
            bDidCopyRamImmediate = true;
            std::memcpy(mBuffer.getBytes(), oldBuffer.getBytes(), sizeToPreserve);
        }
        else {
            TransferTask& transferTask = pDevice->getTransferMgr().getPreFrameTransferTask();
            transferTask.addBufferCopy(
                oldBuffer.getVkBuffer(),
                mBuffer.getVkBuffer(),
                0,
                0,
                sizeToPreserve
            );
        }
    }

    // Preserving the locked region contents if specified.
    //
    // Notes: 
    //  (1) That this operation is always performed immediately
    //  (2) We can skip if we already copied the entire contents of the buffer (copy ram immediate)
    //
    if (bKeepLockedData && bWasLocked && mbIsLocked && !bDidCopyRamImmediate) {
        const uint64_t overlapMin = std::max(oldLockedOffset, mLockedOffset);
        const uint64_t overlapMax = std::min(oldLockedOffset + oldLockedSize, mLockedOffset + mLockedSize);

        // Only do if there is an actual overlap
        if (overlapMax > overlapMin) {
            const uint64_t overlapSize = overlapMax - overlapMin;
            const std::byte* const pSrc = (pOldLockedBytes - oldLockedOffset + overlapMin);
            std::byte* const pDst = (mpLockedBytes - mLockedOffset + overlapMin);

            std::memcpy(pDst, pSrc, overlapSize);
        }
    }

    // If we got to here then the resize operation was a success
    return true;
}

END_NAMESPACE(vgl)
