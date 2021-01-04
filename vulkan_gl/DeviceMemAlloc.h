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

    // True if the alloc lives on the device
    bool bIsDeviceLocal;

    // True if the alloc can be mapped into the address space of this process (host/CPU visible)
    bool bIsHostVisible;

    // Memory allocator internal info: which pool (and optionally sub pool) the alloc is in
    uint32_t poolId;
    uint32_t subPoolId;

    // TODO: add support for LARGE direct allocs outside of pools
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
    REQUIRE_HOST_VISIBLE
};

END_NAMESPACE(vgl)
