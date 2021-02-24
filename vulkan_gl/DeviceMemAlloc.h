#pragma once

#include "Macros.h"

#include <cstddef>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds information about an allocation made through the device memory manager
//------------------------------------------------------------------------------------------------------------------------------------------
struct DeviceMemAlloc {
    // Handle to the Vulkan device memory object that this block of memory is allocated from.
    // Needed to bind this allocation to a buffer in Vulkan.
    // Note that this memory object will likely be shared with many other allocations.
    VkDeviceMemory vkDeviceMemory;

    // The offset of this allocation within the device memory pool.
    // This will be needed when binding the memory to a buffer.
    uint64_t offset;

    // The size of the memory region allocated.
    // This may be bigger than what was requested.
    uint64_t size;

    // Pointer to the start of the memory region allocated.
    // Note that this is only provided for memory allocations that are host/CPU visible.
    std::byte* pBytes;

    // True if the alloc is pooled.
    // If 'false' then the device memory is allocated directly from Vulkan.
    // Only a small number of these allocations can be made (depending on the implementation) so they should be used sparingly.
    bool bIsPooled;

    // True if the alloc lives on the device
    bool bIsDeviceLocal;

    // True if the alloc can be mapped into the address space of this process (host/CPU visible)
    bool bIsHostVisible;

    // Memory allocator internal info: which pool (and optionally sub pool) the alloc is in, if pooled.
    // If the alloc is not pooled, which alloc id is used.
    union {
        uint32_t poolId;
        uint32_t allocId;
    };

    uint32_t subPoolId;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// The required mode of behavior when allocating device memory
//------------------------------------------------------------------------------------------------------------------------------------------
enum class DeviceMemAllocMode : uint8_t {
    // Prefer to use on-device memory (GDDR5 etc.) where possible but fallback to using system RAM when GPU memory is not available.
    // Note that when the returned memory is on-device it *may* also be host/CPU visible on some systems that have a shared
    // memory architecture. When these situations arise, texture uploads etc. can be optimized in some cases.
    PREFER_DEVICE_LOCAL,

    // Absolutely require host/CPU visible memory at the very least.
    // This mode should be used when mapping memory is a requirement (e.g for upload to GPU ram).
    // Note that in shared memory architecture systems the returned memory may also be on-device.
    REQUIRE_HOST_VISIBLE,

    // Same as 'PREFER_DEVICE_LOCAL' except the allocation should be unpooled
    UNPOOLED_PREFER_DEVICE_LOCAL,

    // Same as 'REQUIRE_HOST_VISIBLE' except the allocation should be unpooled
    UNPOOLED_REQUIRE_HOST_VISIBLE
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if the specified device mem alloc mode is unpooled
//------------------------------------------------------------------------------------------------------------------------------------------
inline bool isUnpooledDeviceMemAllocMode(const DeviceMemAllocMode mode) noexcept {
    switch (mode) {
        case DeviceMemAllocMode::PREFER_DEVICE_LOCAL:
        case DeviceMemAllocMode::REQUIRE_HOST_VISIBLE:
            return false;

        case DeviceMemAllocMode::UNPOOLED_PREFER_DEVICE_LOCAL:
        case DeviceMemAllocMode::UNPOOLED_REQUIRE_HOST_VISIBLE:
            return true;
    }

    return false;
}

END_NAMESPACE(vgl)
