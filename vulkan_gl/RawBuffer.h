#pragma once

#include "DeviceMemAlloc.h"

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a 'raw' Vulkan buffer that either resides in on-device memory, system RAM or shared memory on systems that
// have a shared memory architecture. Allocates it's own memory from the device memory manager and manages the lifetime
// of the Vulkan buffer object.
//
// Unlike the regular 'Buffer' class that wraps this 'RawBuffer' type, this object does NOT support locking the buffer.
// If the buffer resides in on-device memory that is not CPU visisble, transfers must be arranged via Vulkan's supported
// transfer mechanisms using the transfer manager or through some other means.
//
// Note: the buffer size will typically be bigger than what is requested due to the way the allocator works.
// Use this to your advantage where possible!
//------------------------------------------------------------------------------------------------------------------------------------------
class RawBuffer {
public:
    RawBuffer() noexcept;
    RawBuffer(RawBuffer&& other) noexcept;
    ~RawBuffer() noexcept;

    bool init(
        LogicalDevice& device,
        const uint64_t size,
        const DeviceMemAllocMode allocMode,
        const VkBufferUsageFlags bufferUsageFlags
    ) noexcept;
    
    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;
    bool isValid() const noexcept;

    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkBuffer getVkBuffer() const noexcept { return mVkBuffer; }
    inline uint64_t getSize() const noexcept { return mSize; }
    inline uint64_t getOrigRequestedSize() const noexcept { return mOrigRequestedSize; }
    inline bool isDeviceLocal() const noexcept { return mDeviceMemAlloc.bIsDeviceLocal; }
    inline bool isHostVisible() const noexcept { return mDeviceMemAlloc.bIsHostVisible; }
    inline std::byte* getBytes() const noexcept { return mDeviceMemAlloc.pBytes; }
    
private:
    // Copy and move assign disallowed
    RawBuffer(const RawBuffer& other) = delete;
    RawBuffer& operator = (const RawBuffer& other) = delete;
    RawBuffer& operator = (RawBuffer&& other) = delete;

    // Retirement manager has intrusive access (for modifying retirement state)
    friend class RetirementMgr;
    
    uint64_t            mSize;                  // Note: this controls 'validity' of the buffer to save memory - no 'valid' flag
    uint64_t            mOrigRequestedSize;
    LogicalDevice*      mpDevice;
    VkBuffer            mVkBuffer;
    DeviceMemAlloc      mDeviceMemAlloc;
};

END_NAMESPACE(vgl)
