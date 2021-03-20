#include "DeviceMemMgr.h"

#include "Asserts.h"
#include "DeviceMemAlloc.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates a non-initialized memory manager
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceMemMgr::DeviceMemMgr(const VkFuncs& vkFuncs) noexcept
    : mbIsValid(false)
    , mVkFuncs(vkFuncs)
    , mpDevice(nullptr)
    , mPools()
    , mUnpooledAllocs()
    , mDeviceLocalPools()
    , mHostVisiblePools()
    , mNextNewPoolId(1)
    , mNextUnpooledAllocId(1)
    , mbDeviceMemExhausted(false)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically cleans up after the memory manager
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceMemMgr::~DeviceMemMgr() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the memory manager
//------------------------------------------------------------------------------------------------------------------------------------------
void DeviceMemMgr::init(LogicalDevice& device) noexcept {
    // Preconditions
    ASSERT_LOG(!mbIsValid, "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());

    // Not much else to do here; just save the device for later reference...
    mpDevice = &device;
    mbIsValid = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the memory manager
//------------------------------------------------------------------------------------------------------------------------------------------
void DeviceMemMgr::destroy(const bool bForceIfInvalid) noexcept {
    // Only proceed if we have to...
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Free anything that is unused firstly and mark the memory manager as invalid
    freeUnusedPools();
    mbIsValid = false;

    // If any other stuff is leftover then it is a leak
    #if ASSERTS_ENABLED == 1
        for (std::pair<const MemMgrIdT, Pool>& poolKvp : mPools) {
            Pool& pool = poolKvp.second;
            ASSERT_LOG(pool.memMgr.getNumBytesAllocated() == 0, "Expect all resources to be freed at this point!");

            for (std::pair<const MemMgrIdT, SubPool>& subPoolKvp : pool.subPools) {
                SubPool& subPool = subPoolKvp.second;
                ASSERT_LOG(subPool.memMgr.getNumBytesAllocated() == 0, "Expect all resources to be freed at this point!");
            }
        }

        ASSERT_LOG(mUnpooledAllocs.empty(), "Expect all resources to be freed at this point!");
    #endif

    // Free all Vulkan memory allocated
    if (mpDevice) {
        // Free up the memory for all pools
        const VkDevice vkDevice = mpDevice->getVkDevice();
        
        for (std::pair<const MemMgrIdT, Pool>& poolKvp : mPools) {
            // Unmap any mapped memory in this pool and free the device memory
            Pool& pool = poolKvp.second;
            const VkDeviceMemory vkDeviceMemory = pool.vkDeviceMemory;
            ASSERT(vkDeviceMemory);

            if (pool.pMappedMemory) {
                mVkFuncs.vkUnmapMemory(vkDevice, vkDeviceMemory);
            }

            mVkFuncs.vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);
        }

        // Free all unpooled allocations
        for (std::pair<const MemMgrIdT, DeviceMemAlloc>& allocKvp : mUnpooledAllocs) {
            freeUnpooledAlloc(allocKvp.second);
        }
    }

    // Cleanup all these
    mbDeviceMemExhausted = false;
    mNextUnpooledAllocId = 1;
    mNextNewPoolId = 1;
    mHostVisiblePools.clear();
    mDeviceLocalPools.clear();
    mUnpooledAllocs.clear();
    mPools.clear();
    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the expected allocation size for a given input size.
// This is the amount that is GUARANTEED to be allocated if an allocation will succeed.
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t DeviceMemMgr::simulateAllocSize(const uint64_t numBytes) const noexcept {
    // Figure out the the unit size for the tier to start the search on
    uint64_t thisTierUS;
    
    if (numBytes < SUB_POOL_ALLOC_THRESHOLD) {
        if (numBytes > 0) {
            thisTierUS = SUB_POOL_UNIT_SIZE;
        } else {
            return 0;       // Asked for zero bytes so the alloc will be zero bytes
        }
    }
    else {
        if (numBytes <= MAIN_POOL_SIZE) {
            thisTierUS = MAIN_POOL_UNIT_SIZE;
        } else {
            return numBytes;    // Alloc is too big for pooling - must be allocated directly from Vulkan!
        }
    }

    // Keep going up tiers until we get to the right one
    while (numBytes > thisTierUS * 3) {
        thisTierUS = thisTierUS * 4;
    }

    // Figure out how many units to allocate on this tier - 1, 2 or 3
    if (numBytes > thisTierUS) {
        const uint64_t doubleThisTierUS = thisTierUS * 2;

        if (numBytes > doubleThisTierUS) {
            return thisTierUS * 3;
        } else {
            return doubleThisTierUS;
        }
    } 
    else {
        return thisTierUS;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to alloc the given amount of memory and saves the details in the given output struct.
// The allowable vulkan memory type bits are also specified, or if '0' is given then any Vulkan memory type is allowable.
//
// Notes:
//  (1) The app will terminate if allocation fails due to it being too big or if we are out of memory.
//  (2) If a zero sized alloc is requested the call will succeed and not terminate with an out of memory
//      error but the given alloc info out will be all zeroed out.
//------------------------------------------------------------------------------------------------------------------------------------------
void DeviceMemMgr::alloc(
    const uint64_t numBytes,
    const uint32_t allowedVkMemTypeBits,
    const DeviceMemAllocMode allocMode,
    DeviceMemAlloc& allocInfoOut
) noexcept {
    // Clear out the output info. Note that if there are no bytes to alloc then we are done
    allocInfoOut = {};
    
    if (numBytes <= 0)
        return;

    // Should we attempt a pooled or unpooled alloc?
    // Do an unpooled alloc if explicitly requested, or if the allocation is too big.
    const bool bAllocUnpooled = (isUnpooledDeviceMemAllocMode(allocMode) || (numBytes > MAIN_POOL_SIZE));

    // Try to do the alloc and die with an out of memory error if it fails
    if (bAllocUnpooled) {
        if (!allocUnpooled(numBytes, allowedVkMemTypeBits, allocMode, allocInfoOut))
            FatalErrors::outOfMemory();
    } 
    else {
        if (!allocFromAllPools(numBytes, allowedVkMemTypeBits, allocMode, allocInfoOut))
            FatalErrors::outOfMemory();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Dealloc the given memory.
//
// Notes:
//  (1) The given allocation is expected to be valid.
//  (2) The given struct is automatically mem-zeroed when the function ends to proactively help
//      prevent bugs where we try to free the same thing twice.
//------------------------------------------------------------------------------------------------------------------------------------------
void DeviceMemMgr::dealloc(DeviceMemAlloc& allocInfo) noexcept {
    // If it's an unpooled allocation then just dealloc directly
    if (!allocInfo.bIsPooled) {
        freeUnpooledAlloc(allocInfo);
        return;
    }

    // Get the pool the allocation is in: expect it to exist
    Pool& pool = mPools.at(allocInfo.poolId);

    // See if the alloc is in a sub pool or not
    if (allocInfo.subPoolId != INVALID_MEM_MGR_ID) {
        // Alloc is in a sub-pool, get the reference to that and expect it to exist
        SubPool& subPool = pool.subPools.at(allocInfo.subPoolId);

        // Get the offset of the alloc in the subpool and free it up there
        const uint64_t allocSubPoolOffset = allocInfo.offset - subPool.offset;
        subPool.memMgr.dealloc(allocSubPoolOffset);
    }
    else {
        // Alloc is not in a sub-pool, just dealloc it directly instead
        pool.memMgr.dealloc(allocInfo.offset);
    }

    // Clear out the alloc once we're done
    allocInfo = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the memory manager free up any pools that are no longer in use
//------------------------------------------------------------------------------------------------------------------------------------------
void DeviceMemMgr::freeUnusedPools() noexcept {
    // This can only be done if there is a device.
    // If we have no device then there should be no allocations, hence nothing to do.
    if (!mpDevice)
        return;

    const VkDevice vkDevice = mpDevice->getVkDevice();
    ASSERT(vkDevice);

    // Run through all pools and free any that are unused
    auto poolIter = mPools.begin();

    while (poolIter != mPools.end()) {
        // Run through all sub pools in the pool and free any that are unused
        Pool& pool = poolIter->second;
        auto& subPools = pool.subPools;
        auto subPoolIter = subPools.begin();

        while (subPoolIter != subPools.end()) {
            SubPool& subPool = subPoolIter->second;

            if (!subPool.memMgr.hasAllocations()) {
                // This sub pool is finished, deallocate it from the main pool
                pool.memMgr.dealloc(subPool.offset);
                subPoolIter = subPools.erase(subPoolIter);
            } else {
                // This sub pool is still in use
                ++subPoolIter;
            }
        }

        // Free the pool itself if it is no longer in use
        if (!pool.memMgr.hasAllocations()) {
            // Unmap any mapped memory with Vulkan
            if (pool.pMappedMemory) {
                mVkFuncs.vkUnmapMemory(vkDevice, pool.vkDeviceMemory);
            }

            // Free the device memory with Vulkan
            const VkDeviceMemory vkDeviceMemory = pool.vkDeviceMemory;
            mVkFuncs.vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);

            // Remove this pool from any lists that it is in
            const MemMgrIdT poolId = pool.poolId;

            for (size_t i = 0; i < mDeviceLocalPools.size();) {
                if (mDeviceLocalPools[i] == poolId) {
                    mDeviceLocalPools[i] = mDeviceLocalPools.back();    // Swap and pop for a faster erase - order is not important
                    mDeviceLocalPools.pop_back();
                } else {
                    ++i;
                }
            }

            for (size_t i = 0; i < mHostVisiblePools.size();) {
                if (mHostVisiblePools[i] == poolId) {
                    mHostVisiblePools[i] = mHostVisiblePools.back();    // Swap and pop for a faster erase - order is not important
                    mHostVisiblePools.pop_back();
                } else {
                    ++i;
                }
            }

            // If this pool being removed was a device local pool then clear this flag
            if (pool.bIsDeviceLocal) {
                mbDeviceMemExhausted = false;
            }

            // Remove from the map of pools
            poolIter = mPools.erase(poolIter);
        }
        else {
            // This pool is still in use
            ++poolIter;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to alloc from all pools using the given memory type.
// If existing pools cannot satisfy the allocation request then a new one may be made.
// Note that device local memory is preferred where possible.
//------------------------------------------------------------------------------------------------------------------------------------------
bool DeviceMemMgr::allocFromAllPools(
    const uint64_t numBytes,
    const uint32_t allowedVkMemTypeBits,
    const DeviceMemAllocMode allocMode,
    DeviceMemAlloc& allocInfoOut
) noexcept {
    // Sanity check: alloc info should be cleared at this point
    ASSERT(!allocInfoOut.vkDeviceMemory);
    ASSERT(allocInfoOut.offset == 0);
    ASSERT(allocInfoOut.size == 0);
    ASSERT(!allocInfoOut.pBytes);

    // Sanity check: this is for pooled allocations only
    ASSERT(!isUnpooledDeviceMemAllocMode(allocMode));

    // If the alloc is bigger than then our pool size then it can't be done!
    if (numBytes > MAIN_POOL_SIZE)
        return false;

    // If we want device local memory then try to allocate that first
    if (allocMode == DeviceMemAllocMode::PREFER_DEVICE_LOCAL) {
        // Try the existing pools we have first
        for (MemMgrIdT poolId : mDeviceLocalPools) {
            Pool& pool = mPools.at(poolId);

            // See if this pool is allowed according to the Vulkan memory type bits
            if ((allowedVkMemTypeBits & pool.vkMemoryTypeBit) != 0) {
                // This pool is allowed, try to allocate from it
                if (allocFromPool(numBytes, pool, allocInfoOut))
                    return true;
            }
        }

        // Failing that try to make a new device local pool and allocate from
        // that if we haven't already exhausted all our device memory:
        if (!mbDeviceMemExhausted) {
            if (Pool* const pPool = allocNewPool(allowedVkMemTypeBits, true, false)) {
                // Note: this SHOULD succeed at this point!
                ASSERT((pPool->vkMemoryTypeBit & allowedVkMemTypeBits) != 0);

                if (allocFromPool(numBytes, *pPool, allocInfoOut)) {
                    return true;
                } else {
                    ASSERT_FAIL("Expected allocation to succeed, and it didn't?!");
                }
            } else {
                // Gah! Out of device memory - we'll fall back to RAM instead :(
                mbDeviceMemExhausted = true;
            }
        }
    }

    // If we require a host visible ram type or have exhaused on-device memory fallback to here
    for (MemMgrIdT poolId : mHostVisiblePools) {
        Pool& pool = mPools.at(poolId);

        if (allocFromPool(numBytes, pool, allocInfoOut))
            return true;
    }

    // If that failed then try allocating a new host visible pool
    if (Pool* const pPool = allocNewPool(allowedVkMemTypeBits, false, true)) {
        // Note: this SHOULD succeed at this point
        ASSERT((pPool->vkMemoryTypeBit & allowedVkMemTypeBits) != 0);

        if (allocFromPool(numBytes, *pPool, allocInfoOut)) {
            return true;
        } else {
            ASSERT_FAIL("Expected allocation to succeed, and it didn't?!");
        }
    }

    // Allocation failed! :(
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to service an allocation using the given pool. Returns false on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
bool DeviceMemMgr::allocFromPool(
    const uint64_t numBytes,
    Pool& pool,
    DeviceMemAlloc& allocInfoOut
) noexcept {
    // Sanity check: alloc info should be cleared at this point
    ASSERT(!allocInfoOut.vkDeviceMemory);
    ASSERT(allocInfoOut.offset == 0);
    ASSERT(allocInfoOut.size == 0);
    ASSERT(!allocInfoOut.pBytes);

    // See if the alloc is under the 'small alloc threshold'.
    // If that is the case then alloc it from a small sub pool instead of this main pool.
    static_assert(SUB_POOL_ALLOC_THRESHOLD <= SUB_POOL_SIZE, "Sub pool not big enough to accomodate sub pool alloc?!!");

    if (numBytes < SUB_POOL_ALLOC_THRESHOLD) {
        // Try to alloc from all existing sub pools
        for (std::pair<const MemMgrIdT, SubPool>& subPoolKvp : pool.subPools) {
            SubPool& subPool = subPoolKvp.second;

            if (subPool.memMgr.getMaxGuaranteedAllocInBytes() < numBytes)
                continue;

            if (allocFromSubPool(numBytes, pool, subPool, allocInfoOut))
                return true;
        }

        // If that fails try to make a new sub pool and alloc from that
        if (SubPool* subPool = allocNewSubPool(pool))
            return allocFromSubPool(numBytes, pool, *subPool, allocInfoOut);

        // Failed to alloc a new sub pool from this parent pool; can't alloc from this parent pool
        return false;
    }

    // Try to alloc and if that fails then return false
    MemMgrAllocResult alloc = pool.memMgr.alloc(numBytes);

    if (alloc.size <= 0)
        return false;

    // Otherwise fill in the result details and return true for success
    allocInfoOut.vkDeviceMemory = pool.vkDeviceMemory;
    allocInfoOut.offset = alloc.offset;
    allocInfoOut.size = alloc.size;
    allocInfoOut.bIsPooled = true;
    allocInfoOut.bIsDeviceLocal = pool.bIsDeviceLocal;
    allocInfoOut.bIsHostVisible = pool.bIsHostVisible;
    allocInfoOut.poolId = pool.poolId;

    if (pool.pMappedMemory) {
        allocInfoOut.pBytes = pool.pMappedMemory + alloc.offset;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to service an allocation using the given sub pool. Returns false on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
bool DeviceMemMgr::allocFromSubPool(
    const uint64_t numBytes,
    Pool& parentPool,
    SubPool& subPool,
    DeviceMemAlloc& allocInfoOut
) noexcept {
    // Sanity check: alloc info should be cleared at this point
    ASSERT(!allocInfoOut.vkDeviceMemory);
    ASSERT(allocInfoOut.offset == 0);
    ASSERT(allocInfoOut.size == 0);
    ASSERT(!allocInfoOut.pBytes);

    // Try to alloc and if that fails then return false
    MemMgrAllocResult alloc = subPool.memMgr.alloc(numBytes);

    if (alloc.size <= 0)
        return false;
    
    // Otherwise fill in the result details and return true for success
    allocInfoOut.vkDeviceMemory = parentPool.vkDeviceMemory;
    allocInfoOut.offset = subPool.offset + alloc.offset;
    allocInfoOut.size = alloc.size;
    allocInfoOut.bIsPooled = true;
    allocInfoOut.bIsDeviceLocal = parentPool.bIsDeviceLocal;
    allocInfoOut.bIsHostVisible = parentPool.bIsHostVisible;
    allocInfoOut.poolId = parentPool.poolId;
    allocInfoOut.subPoolId = subPool.subPoolId;

    if (parentPool.pMappedMemory) {
        allocInfoOut.pBytes = parentPool.pMappedMemory + subPool.offset + alloc.offset;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to create a new pool of memory in the memory manager with specific memory properties.
// If 'bRequireDeviceLocalMem' is true then device local memory is explicitly required.
// If 'bRequireHostVisibleMem' is true then host visible memory is explicitly required.
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceMemMgr::Pool* DeviceMemMgr::allocNewPool(
    const uint32_t allowedVulkanMemTypeBits,
    bool bRequireDeviceLocalMem,
    bool bRequireHostVisibleMem
) noexcept {
    // Sanity checks
    ASSERT(mbIsValid);
    ASSERT(mpDevice && mpDevice->getVkDevice());
    ASSERT_LOG(bRequireDeviceLocalMem || bRequireHostVisibleMem, "Must require either device local or host visible memory!");
    static_assert(MAIN_POOL_SIZE >= SUB_POOL_UNIT_SIZE, "Invalid sizes!");

    // Figure out the vulkan memory property flags we want
    const VkMemoryPropertyFlags vkMemPropertyFlagsRequired = (
        (bRequireDeviceLocalMem ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0) |
        (bRequireHostVisibleMem ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0)
    );

    // Try to find a memory type that satisfies both the required memory property flags we want as well as being
    // one of the allowed memory types specified via the memory type bits:
    const PhysicalDevice* pPhysicalDevice = mpDevice->getPhysicalDevice();
    ASSERT(pPhysicalDevice);
    const uint32_t vkMemoryTypeIndex = pPhysicalDevice->findSuitableMemTypeIndex(allowedVulkanMemTypeBits, vkMemPropertyFlagsRequired);

    // If no suitable memory type can be found then bail out
    if (vkMemoryTypeIndex >= VK_MAX_MEMORY_TYPES)
        return nullptr;

    // Try to alloc a new id for this pool, if that fails then pool alloc fails
    const MemMgrIdT poolId = pickNewPoolId();

    if (poolId == INVALID_MEM_MGR_ID)
        return nullptr;

    // Fill in the details of the memory alloc for vulkan
    VkMemoryAllocateInfo memAllocInfo = {};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = MAIN_POOL_SIZE;
    memAllocInfo.memoryTypeIndex = vkMemoryTypeIndex;

    // Do the memory allocation
    const VkDevice vkDevice = mpDevice->getVkDevice();
    VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;

    if (mVkFuncs.vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &vkDeviceMemory) != VK_SUCCESS)
        return nullptr;

    ASSERT(vkDeviceMemory);

    // Get the memory type we picked to allocate and see whether it is device local and host visible
    const VkMemoryType& vkMemoryType = pPhysicalDevice->getMemProps().memoryTypes[vkMemoryTypeIndex];

    const bool bIsMemTypeDeviceLocal = (
        (vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0
    );

    const bool bIsMemTypeHostVisible = (
        ((vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) &&
        ((vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0)
    );

    // If the memory allocation is CPU mappable then do the mapping now
    void* pMappedMemory = nullptr;

    if (bIsMemTypeHostVisible) {
        const VkResult mapMemResult = mVkFuncs.vkMapMemory(vkDevice, vkDeviceMemory, 0, VK_WHOLE_SIZE, 0, &pMappedMemory);

        if (mapMemResult != VK_SUCCESS) {
            // Failed to do a mapping - cleanup the buffer and return null!
            ASSERT_FAIL("Failed to map device memory to a host visible buffer!");
            mVkFuncs.vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);
            return nullptr;
        }
    }

    // Okay, all good! Make a new pool and initialize it:
    Pool& pool = mPools[poolId];
    pool.bIsDeviceLocal = bIsMemTypeDeviceLocal;
    pool.bIsHostVisible = bIsMemTypeHostVisible;
    pool.poolId = poolId;
    pool.vkMemoryTypeBit = 1 << vkMemoryTypeIndex;
    pool.vkDeviceMemory = vkDeviceMemory;
    pool.pMappedMemory = reinterpret_cast<std::byte*>(pMappedMemory);

    // Add the pool id to the appropriate category lists
    if (bIsMemTypeDeviceLocal) {
        mDeviceLocalPools.push_back(poolId);
    }

    if (bIsMemTypeHostVisible) {
        mHostVisiblePools.push_back(poolId);
    }

    // Return the newly created pool
    return &pool;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to create a sub pool under the given pool and return it's pointer; returns null on failure
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceMemMgr::SubPool* DeviceMemMgr::allocNewSubPool(Pool& parentPool) noexcept {
    // Don't try anything if the parent pool can't service what we are looking for
    auto& parentPoolMemMgr = parentPool.memMgr;

    if (parentPoolMemMgr.getMaxGuaranteedAllocInBytes() < SUB_POOL_SIZE)
        return nullptr;

    // Firstly try to alloc a new sub pool id, if that fails then sub pool alloc fails
    const MemMgrIdT subPoolId = pickNewSubPoolId(parentPool);
    
    if (subPoolId == INVALID_MEM_MGR_ID)
        return nullptr;

    // Try to alloc a new sub pool, if that fails then just return a null pointer
    MemMgrAllocResult allocResult = parentPool.memMgr.alloc(SUB_POOL_SIZE);

    if (allocResult.size <= 0)
        return nullptr;

    // Okay, all good! Make a new sub pool, initialize and return it:
    SubPool& subPool = parentPool.subPools[subPoolId];
    subPool.offset = allocResult.offset;
    subPool.subPoolId = subPoolId;
    return &subPool;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to make an unpooled memory allocation and save the details in the output structure.
// May fallback to using system RAM if possible, if device memory is requested and not possible.
// Returns 'true' on successful allocation.
//------------------------------------------------------------------------------------------------------------------------------------------
bool DeviceMemMgr::allocUnpooled(
    const uint64_t numBytes,
    const uint32_t allowedVkMemTypeBits,
    const DeviceMemAllocMode allocMode,
    DeviceMemAlloc& allocInfoOut
) noexcept {
    // Note: this can be called even when a pooled allocation is requested, if the alloc is too big.
    // Therefore the alloc mode can be either of these two things:
    if ((allocMode == DeviceMemAllocMode::PREFER_DEVICE_LOCAL) || (allocMode == DeviceMemAllocMode::UNPOOLED_PREFER_DEVICE_LOCAL)) {
        if (allocUnpooled(numBytes, allowedVkMemTypeBits, true, false, allocInfoOut))
            return true;
    }

    // Fallback to using non-device (host visible) memory if that is possible, or if that was requested in the first place
    return allocUnpooled(numBytes, allowedVkMemTypeBits, false, true, allocInfoOut);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to make an unpooled memory allocation of the exact specified type and save the details in the output structure.
// Returns 'true' on successful allocation.
//------------------------------------------------------------------------------------------------------------------------------------------
bool DeviceMemMgr::allocUnpooled(
    const uint64_t numBytes,
    const uint32_t allowedVkMemTypeBits,
    const bool bRequireDeviceLocalMem,
    const bool bRequireHostVisibleMem,
    DeviceMemAlloc& allocInfoOut
) noexcept {
    // Sanity checks
    ASSERT(mbIsValid);
    ASSERT(mpDevice && mpDevice->getVkDevice());
    ASSERT_LOG(bRequireDeviceLocalMem || bRequireHostVisibleMem, "Must require either device local or host visible memory!");

    // Figure out the vulkan memory property flags we want
    const VkMemoryPropertyFlags vkMemPropertyFlagsRequired = (
        (bRequireDeviceLocalMem ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0) |
        (bRequireHostVisibleMem ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0)
    );

    // Try to find a memory type that satisfies both the required memory property flags we want as well as being
    // one of the allowed memory types specified via the memory type bits:
    const PhysicalDevice* pPhysicalDevice = mpDevice->getPhysicalDevice();
    ASSERT(pPhysicalDevice);
    const uint32_t vkMemoryTypeIndex = pPhysicalDevice->findSuitableMemTypeIndex(allowedVkMemTypeBits, vkMemPropertyFlagsRequired);

    // If no suitable memory type can be found then bail out
    if (vkMemoryTypeIndex >= VK_MAX_MEMORY_TYPES)
        return false;

    // Allocate an id for the allocation and if that fails then the alloc itself fails
    const MemMgrIdT allocId = pickNewUnpooledAllocId();

    if (allocId == INVALID_MEM_MGR_ID)
        return nullptr;

    // Fill in the details of the memory alloc for vulkan
    VkMemoryAllocateInfo memAllocInfo = {};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = numBytes;
    memAllocInfo.memoryTypeIndex = vkMemoryTypeIndex;

    // Do the memory allocation
    const VkDevice vkDevice = mpDevice->getVkDevice();
    VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;

    if (mVkFuncs.vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &vkDeviceMemory) != VK_SUCCESS)
        return nullptr;

    ASSERT(vkDeviceMemory);

    // Get the memory type we picked to allocate and see whether it is device local and host visible
    const VkMemoryType& vkMemoryType = pPhysicalDevice->getMemProps().memoryTypes[vkMemoryTypeIndex];

    const bool bIsMemTypeDeviceLocal = (
        (vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0
    );

    const bool bIsMemTypeHostVisible = (
        ((vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) &&
        ((vkMemoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0)
    );

    // If the memory allocation is CPU mappable then do the mapping now
    void* pMappedMemory = nullptr;

    if (bIsMemTypeHostVisible) {
        const VkResult mapMemResult = mVkFuncs.vkMapMemory(vkDevice, vkDeviceMemory, 0, VK_WHOLE_SIZE, 0, &pMappedMemory);

        if (mapMemResult != VK_SUCCESS) {
            // Failed to do a mapping - cleanup the buffer and return null!
            ASSERT_FAIL("Failed to map device memory to a host visible buffer!");
            mVkFuncs.vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);
            return nullptr;
        }
    }

    // Okay, all good! Save the allocation details:
    DeviceMemAlloc& alloc = mUnpooledAllocs[allocId];
    alloc.vkDeviceMemory = vkDeviceMemory;
    alloc.offset = 0;
    alloc.size = numBytes;
    alloc.pBytes = (std::byte*) pMappedMemory;
    alloc.bIsPooled = false;
    alloc.bIsDeviceLocal = bIsMemTypeDeviceLocal;
    alloc.bIsHostVisible = bIsMemTypeHostVisible;
    alloc.allocId = allocId;
    alloc.subPoolId = INVALID_MEM_MGR_ID;

    // Copy the allocation details to output to finish up
    allocInfoOut = alloc;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frees the memory for an unpooled allocation which should be valid; also unmaps any memory mapped
//------------------------------------------------------------------------------------------------------------------------------------------
void DeviceMemMgr::freeUnpooledAlloc(DeviceMemAlloc& alloc) noexcept {
    // Sanity checks
    ASSERT(alloc.vkDeviceMemory != VK_NULL_HANDLE);
    ASSERT(!alloc.bIsPooled);

    // Unmap any host visible memory (if mapped) then free the allocation
    const VkDevice vkDevice = mpDevice->getVkDevice();
    const VkDeviceMemory vkDeviceMemory = alloc.vkDeviceMemory;

    if (alloc.pBytes) {
        mVkFuncs.vkUnmapMemory(vkDevice, vkDeviceMemory);
    }

    mVkFuncs.vkFreeMemory(vkDevice, vkDeviceMemory, nullptr);

    // Clear the alloc details and remove from the map
    mUnpooledAllocs.erase(alloc.allocId);
    alloc = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to choose a new pool id within the memory manager
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceMemMgr::MemMgrIdT DeviceMemMgr::pickNewPoolId() noexcept {
    // Start our search using the next new pool id field in the memory manager
    const MemMgrIdT origNextNewPoolId = mNextNewPoolId;
    MemMgrIdT poolId = origNextNewPoolId;

    do {
        // Make sure this is not the invalid pool id, and that there isn't already a pool with this id
        if ((poolId != INVALID_MEM_MGR_ID) && (mPools.find(poolId) == mPools.end())) {
            // Found a valid pool id to use. Increment the next new pool id and return the id chosen!
            mNextNewPoolId = poolId + 1;
            return poolId;
        }

        // This is not a valid pool id, try another
        ++poolId;

    } while (poolId != origNextNewPoolId);      // Continue until we wrap around

    // We should never reach here in practice, but if we do we couldn't alloc a pool id!
    return INVALID_MEM_MGR_ID;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to choose a new sub pool id within the the given parent pool
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceMemMgr::MemMgrIdT DeviceMemMgr::pickNewSubPoolId(Pool& parentPool) noexcept {
    // Start our search on the next new pool id on the parent
    const MemMgrIdT origNextNewPoolId = parentPool.nextNewSubPoolId;
    MemMgrIdT poolId = origNextNewPoolId;

    do {
        // Make sure this is not the invalid pool id, and that there isn't already a sub pool with this id
        if ((poolId != INVALID_MEM_MGR_ID) && (parentPool.subPools.find(poolId) ==  parentPool.subPools.end())) {
            // Found a valid pool id to use. Increment the next new pool id and return the id chosen!
            parentPool.nextNewSubPoolId = poolId + 1;
            return poolId;
        }

        // This is not a valid pool id, try another
        ++poolId;

    } while (poolId != origNextNewPoolId);      // Continue until we wrap around

    // We should never reach here in practice, but if we do we couldn't alloc a pool id!
    return INVALID_MEM_MGR_ID;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to choose a new unpooled alloc id within the memory manager
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceMemMgr::MemMgrIdT DeviceMemMgr::pickNewUnpooledAllocId() noexcept {
    // Start our search using the next new unpooled alloc id field in the memory manager
    const MemMgrIdT origNextNewAllocId = mNextUnpooledAllocId;
    MemMgrIdT allocId = origNextNewAllocId;

    do {
        // Make sure this is not the invalid alloc id, and that there isn't already an alloc with this id
        if ((allocId != INVALID_MEM_MGR_ID) && (mUnpooledAllocs.find(allocId) == mUnpooledAllocs.end())) {
            // Found a valid alloc id to use. Increment the next new alloc id and return the id chosen!
            mNextNewPoolId = allocId + 1;
            return allocId;
        }

        // This is not a valid alloc id, try another
        ++allocId;

    } while (allocId != origNextNewAllocId);     // Continue until we wrap around

    // We should never reach here in practice, but if we do we couldn't alloc a pool id!
    return INVALID_MEM_MGR_ID;
}

END_NAMESPACE(vgl)
