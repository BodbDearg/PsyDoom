#pragma once

#include "RawBuffer.h"

BEGIN_NAMESPACE(vgl)

class LogicalDevice;
class TransferTask;

//------------------------------------------------------------------------------------------------------------------------------------------
// Enum representing how a buffer is expected to be used.
// Specified on creation of the buffer in order to optimize it for different usage patterns.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class BufferUsageMode : uint8_t {
    // The buffer is populated once or very infrequently.
    // Prefer to transfer the buffer into on-device memory before being used (if possible!) but fallback to
    // regular RAM if we are out of GPU memory. 
    //
    // With this usage mode the buffer will not keep a temporary CPU-visible staging buffer around (if staging
    // buffers are required to do transfers) because it is not expected to be updated often. Instead a temporary
    // buffer will be allocated once via the transfer manager for the purposes of a once-off transfer.
    STATIC,
    
    // The buffer is populated very frequently or once per frame.
    // Prefer to transfer the buffer into on-device memory before being used (if possible!) but fallback to
    // regular RAM if we are out of GPU memory.
    //
    // With this usage mode the buffer maintains it's own staging buffer (if staging buffers are required for transfers)
    // permanently so as to save the overhead of frequent allocations. This results in higher overall memory usage but
    // means that the buffer always has a staging buffer ready if a transfer is required.
    DYNAMIC,

    // Immediate mode: only attempt to put the buffer in CPU visible memory (a.k.a RAM) and don't bother attempting
    // to transfer the contents of the buffer to GPU memory. All writes are guaranteed to take effect immediately
    // since the memory allocated is CPU visible and no transfer operations are required to transfer to GPU memory.
    //
    // This usage mode can be appropriate in some cases where we have small buffers that are only used once,
    // such as uniform buffers used in shaders. In those cases the cost of arranging a transfer to GPU side memory for a small,
    // one-shot uniform buffer may outweigh the potential benefits of having all the data in GPU rather than CPU memory.
    IMMEDIATE
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Class representing a buffer (Vertex, Index, Uniform etc.) that can perferably be placed in on-GPU memory if required.
// The buffer is lockable and arranges transfers through the transfer manager if required once the buffer has been unlocked.
// 
// Notes:
//  (1) The format of the actual data within the buffer (16 vs 32 bit indices, vertex format etc.) is completely
//      determined externally. This buffer just provides an interface to update the raw bytes of the buffer data.
//  (2) Read backs of buffer data are NOT supported. If you lock a buffer range the data is undefined until written to.
//  (3) The actual size of the buffer will most likely be bigger than what was requested, due to the way the allocator works.
//      In some cases calling code can take advantage of that and leverage the extra memory to some end.
//      If you want to know what the original requested size of the buffer was for book-keeping purposes, query the
//      'requested' buffer size instead of regular size.
//  (4) Even though some of the usage modes might request on-GPU memory, due to memory restraints the allocator
//      may need to fall back to using system RAM if on-device memory has been exhausted.
//  (5) On a system with shared memory architecture system RAM may also be GPU memory too, so internally only
//      one buffer is maintained and no staging/transfer operations are needed. This is not usual of most desktop
//      PC systems however and is instead more typical to mobile or consoles.
//------------------------------------------------------------------------------------------------------------------------------------------
class Buffer {
public:
    Buffer() noexcept;
    Buffer(Buffer&& other) noexcept;
    ~Buffer() noexcept;

    bool initWithByteCount(
        LogicalDevice& device,
        const VkBufferUsageFlags baseUsageFlags,
        const BufferUsageMode usageMode,
        const uint64_t sizeInBytes,
        const bool bIsResizable = false
    ) noexcept;

    // Convenience call: initialize the buffer with a specified number of typed elements
    template <class T>
    inline bool initWithElementCount(
        LogicalDevice& device,
        const VkBufferUsageFlags baseUsageFlags,
        const BufferUsageMode usageMode,
        const uint64_t numElements,
        const bool bIsResizable = false
    ) noexcept {
        return initWithByteCount(device, baseUsageFlags, usageMode, numElements * sizeof(T), bIsResizable);
    }

    void destroy(const bool bImmediateCleanup = false, const bool bForceIfInvalid = false) noexcept;

    // Get the device used to create the buffer
    inline LogicalDevice* getDevice() const noexcept {
        return mBuffer.getDevice();
    }

    // General properties of the buffer
    inline bool isValid() const noexcept { return mbIsValid; }
    inline VkImageUsageFlags getUsageFlags() const noexcept { return mUsageFlags; }
    inline BufferUsageMode getUsageMode() const noexcept { return mUsageMode; }
    inline bool isResizable() const noexcept { return mbIsResizable; }
    
    uint64_t getSizeInBytes() const noexcept;
    uint64_t getOrigRequestedSizeInBytes() const noexcept;

    // Convenience call: get the buffer actual size in terms of elements
    template <class T>
    inline uint64_t getSizeInElements() const noexcept {
        return getSizeInBytes() / sizeof(T);
    }

    // Convenience call: get the originally requested buffer size in terms of elements
    template <class T>
    inline uint64_t getOrigRequestedSizeInElements() const noexcept {
        return getOrigRequestedSizeInBytes() / sizeof(T);
    }

    // Getting lock and locked region details
    inline bool isLocked() const noexcept { return mbIsLocked; }
    inline uint64_t getLockedOffset() const noexcept { return mLockedOffset; }
    inline uint64_t getLockedSize() const noexcept { return mLockedSize; }
    inline std::byte* getLockedBytes() const noexcept { return mpLockedBytes; }

    template <class T>
    inline T* getLockedElements() const noexcept { return reinterpret_cast<T*>(mpLockedBytes); }
    
    // Get the underlying Vulkan buffer
    inline VkBuffer getVkBuffer() const noexcept { return mBuffer.getVkBuffer(); }

    std::byte* lockBytes(const uint64_t offsetInBytes, const uint64_t sizeInBytes) noexcept;

    // Convenience call: similar to 'lock' but locks a region specified in terms of elements and returns typed data
    template <class T>
    inline T* lockElements(const uint64_t startElement, const uint64_t numElements) noexcept {
        return reinterpret_cast<T*>(lockBytes(startElement * sizeof(T), numElements * sizeof(T)));
    }

    void unlockBytes(const uint64_t writtenSizeInBytes, TransferTask* const pTransferTaskOverride = nullptr) noexcept;

    // Convenience call: similar to 'lock' but unlocks a region and uploads the specified number of elements
    template <class T>
    inline void unlockElements(const uint64_t writtenSizeInElems, TransferTask* const pTransferTaskOverride = nullptr) noexcept {
        unlockBytes(writtenSizeInElems * sizeof(T), pTransferTaskOverride);
    }

    void abandonLock() noexcept;

    // Flags that control buffer resizing behavior
    typedef uint8_t ResizeFlags;

    struct ResizeFlagBits final {
        // No resize flags specified.
        // Specify this if you don't care about preserving data or relocking.
        static constexpr ResizeFlags NONE = 0x00;

        // Keep the data in the buffer after the resize.
        // A transfer is arranged to copy the data to the new buffer.
        // The result of the transfer will not be immediately be available in the resized buffer unless 'COPY_RAM_IMMEDIATE'
        // is specified and both the old and new buffer are stored in CPU visible memory.
        static constexpr ResizeFlags KEEP_DATA = 0x01;
        
        // Keep previously locked data in the new buffer in the region.
        // The region of data preserved is the intersection of the old locked area and new locked area.
        // Any data outside of that region is lost.
        static constexpr ResizeFlags KEEP_LOCKED_DATA = 0x02;

        // Do an immediate copy of the buffer if possible when preserving it's contents.
        // This is only possible if the old and newly resized buffers are CPU visible.
        static constexpr ResizeFlags COPY_RAM_IMMEDIATE = 0x04;
    };

    bool resizeToByteCount(
        const uint64_t newSizeInBytes,
        const ResizeFlags resizeFlags,
        const uint64_t newLockOffsetInBytes = 0,
        const uint64_t newLockSizeInBytes = 0
    ) noexcept;

    // Convenience call: resize the buffer to a specified number of elements
    template <class T>
    inline bool resizeToElementCount(
        const uint64_t newSizeInElements,
        const ResizeFlags resizeFlags,
        const uint64_t newLockOffsetStartElement = 0,
        const uint64_t newLockSizeNumElements = 0
    ) noexcept {
        return resizeToByteCount(
            newSizeInElements * sizeof(T),
            resizeFlags,
            newLockOffsetStartElement * sizeof(T),
            newLockSizeNumElements * sizeof(T)
        );
    }

private:
    // Copy and move assign are disallowed
    Buffer(const Buffer& other) = delete;
    Buffer& operator = (const Buffer& other) = delete;
    Buffer& operator = (Buffer&& other) = delete;
    
    bool                mbIsValid;
    VkBufferUsageFlags  mUsageFlags;
    BufferUsageMode     mUsageMode;
    bool                mbIsResizable;
    bool                mbIsLocked;         // Lock details: is the buffer locked?
    uint64_t            mLockedOffset;      // Lock details: offset of the locked region
    uint64_t            mLockedSize;        // Lock details: size of the locked region
    VkBuffer            mLockedVkBuffer;    // Lock details: the actual Vulkan Buffer locked
    std::byte*          mpLockedBytes;      // Lock details: the actual bytes locked
    RawBuffer           mBuffer;            // Raw backing buffer that stores the actual data for the buffer
    RawBuffer           mStagingBuffer;     // Raw backing buffer used as a persistant staging buffer if the usage mode is dynamic
};

END_NAMESPACE(vgl)
