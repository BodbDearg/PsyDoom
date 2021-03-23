#pragma once

#include "AbstractFixedQTreeMemMgr.h"
#include "DeviceMemAlloc.h"

#include <map>
#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;
struct VkFuncs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Manages vulkan device memory. Provides an interface to allocate both host visible/coherent memory (normally just RAM) as well as device
// local memory (typically on-card GDDR RAM etc.) via 'vkAllocateMemory'. Allocates memory in large pools/chunks but also supports
// allocations outside of the pool for large and irregular sized objects like framebuffers. Unpooled allocations should be used sparingly
// however since Vulkan only requires that 4096 of these can be made, depending on the implementation.
//
// If on-device memory is requested it most likely will NOT be directly visible to the client unless the host device uses some sort
// of shared memory system, but you can check in the returned allocation details whether this is the case or not.
//
// Note that if on-device memory allocation fails, the memory manager will fall back to to using slower regular RAM where it can instead.
// If that subsequent allocation fails then the app will terminate with an out of memory error.
//------------------------------------------------------------------------------------------------------------------------------------------
class DeviceMemMgr {
public:
    DeviceMemMgr(const VkFuncs& vkFuncs) noexcept;
    ~DeviceMemMgr() noexcept;

    void init(LogicalDevice& device) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    uint64_t estimateAllocSize(const uint64_t numBytes) const noexcept;
    
    void alloc(
        const uint64_t numBytes,
        const uint32_t allowedVkMemTypeBits,
        const uint32_t alignment,
        const DeviceMemAllocMode allocMode,
        DeviceMemAlloc& allocInfoOut
    ) noexcept;
    
    void dealloc(DeviceMemAlloc& allocInfo) noexcept;
    void freeUnusedPools() noexcept;

private:
    // Copy and move are disallowed
    DeviceMemMgr(const DeviceMemMgr& other) = delete;
    DeviceMemMgr(DeviceMemMgr&& other) = delete;
    DeviceMemMgr& operator = (const DeviceMemMgr& other) = delete;
    DeviceMemMgr& operator = (DeviceMemMgr&& other) = delete;

    // This shortens things a bit
    using MemMgrAllocResult = AbstractFixedQTreeMemMgrUtils::AllocResult;

    // Size constants for the main and sub pools
    static constexpr uint32_t MAIN_POOL_UNIT_SIZE = 256 * 1024;                 // 256KB smallest block size
    static constexpr uint32_t MAIN_POOL_NUM_TIERS = 4;                          // Results in 256KB, 1MB, 4MB, 16MB, 64MB tiers
    static constexpr uint32_t SUB_POOL_UNIT_SIZE = 4 * 1024;                    // 4KB smallest block size
    static constexpr uint32_t SUB_POOL_NUM_TIERS = 5;                           // Results in 4KB, 16KB, 64KB, 256KB, 1MB, 4MB tiers
    static constexpr uint32_t SUB_POOL_ALLOC_THRESHOLD = MAIN_POOL_UNIT_SIZE;   // Allocs smaller than this go into a sub pool

    // Sizes for the main and sub pools
    static constexpr uint64_t MAIN_POOL_SIZE = Utils::powRecursive((uint64_t) 4, (uint64_t) MAIN_POOL_NUM_TIERS) * MAIN_POOL_UNIT_SIZE;
    static constexpr uint64_t SUB_POOL_SIZE = Utils::powRecursive((uint64_t) 4, (uint64_t) SUB_POOL_NUM_TIERS) * SUB_POOL_UNIT_SIZE;

    // Type for a memory manager id: can be used for a pool, a subpool or an unpooled allocation
    typedef uint32_t MemMgrIdT;

    // Represents an invalid mememory manager id that should never be used
    static constexpr MemMgrIdT INVALID_MEM_MGR_ID = 0;

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Type for a smaller pool which a parent pool to alloc/manage memory in smaller amounts
    //--------------------------------------------------------------------------------------------------------------------------------------
    struct SubPool {
        // Default initialize the sub-pool to an invalid pool
        inline SubPool() noexcept
            : offset(0)
            , subPoolId(INVALID_MEM_MGR_ID)
            , memMgr()
        {
        }

        uint64_t    offset;         // The offset of this sub-pool (in bytes) in the parent pool
        MemMgrIdT   subPoolId;      // The id of the sub-pool

        // The memory manager for this pool
        AbstractFixedQTreeMemMgr<SUB_POOL_UNIT_SIZE, SUB_POOL_NUM_TIERS> memMgr;
    };
    
    //--------------------------------------------------------------------------------------------------------------------------------------
    // Represents one pool of memory in the manager
    //--------------------------------------------------------------------------------------------------------------------------------------
    struct Pool {
        // Default initialize the pool to an invalid pool
        inline Pool() noexcept
            : bIsDeviceLocal(false)
            , bIsHostVisible(false)
            , poolId(INVALID_MEM_MGR_ID)
            , nextNewSubPoolId(1)
            , vkMemoryTypeBit(0)
            , vkDeviceMemory(VK_NULL_HANDLE)
            , pMappedMemory(nullptr)
            , subPools()
            , memMgr()
        {
        }

        // Is this pool hosted on device?
        bool bIsDeviceLocal;

        // Is this pool host visible and modifiable?
        bool bIsHostVisible;

        // The id of this pool
        MemMgrIdT poolId;

        // Id to use for the next sub pool within this pool
        MemMgrIdT nextNewSubPoolId;

        // The Vulkan memory type.
        // One bit is set to indicate which type of Vulkan memory this is (of a possible max of 32 types) so we can
        // do a biwise AND with the supported memory types mask for a vulkan allocation request in order to determine
        // if the allocation can be performed using this pool.
        uint32_t vkMemoryTypeBit;

        // Pointer to the Vulkan Device memory for this pool
        VkDeviceMemory vkDeviceMemory;

        // Pointer to the persistently mapped memory for this pool if the memory is host/CPU visible.
        // Will be null if the device memory is not host visible.
        std::byte* pMappedMemory;

        // Sub pools within this main pool
        std::map<MemMgrIdT, SubPool> subPools;

        // The memory manager for this pool
        AbstractFixedQTreeMemMgr<MAIN_POOL_UNIT_SIZE, MAIN_POOL_NUM_TIERS> memMgr;
    };

    bool allocFromAllPools(
        const uint64_t numBytes,
        const uint32_t allowedVkMemTypeBits,
        const uint32_t alignment,
        const DeviceMemAllocMode allocMode,
        DeviceMemAlloc& allocInfoOut
    ) noexcept;

    bool allocFromPool(
        const uint64_t numBytes,
        const uint32_t alignment,
        Pool& pool,
        DeviceMemAlloc& allocInfoOut
    ) noexcept;

    bool allocFromSubPool(
        const uint64_t numBytes,
        const uint32_t alignment,
        Pool& parentPool,
        SubPool& subPool,
        DeviceMemAlloc& allocInfoOut
    ) noexcept;

    Pool* allocNewPool(
        const uint32_t allowedVulkanMemTypeBits,
        const bool bRequireDeviceLocalMem,
        const bool bRequireHostVisibleMem
    ) noexcept;

    SubPool* allocNewSubPool(Pool& parentPool) noexcept;

    bool allocUnpooled(
        const uint64_t numBytes,
        const uint32_t allowedVkMemTypeBits,
        const DeviceMemAllocMode allocMode,
        DeviceMemAlloc& allocInfoOut
    ) noexcept;

    bool allocUnpooled(
        const uint64_t numBytes,
        const uint32_t allowedVkMemTypeBits,
        const bool bRequireDeviceLocalMem,
        const bool bRequireHostVisibleMem,
        DeviceMemAlloc& allocInfoOut
    ) noexcept;

    void freeUnpooledAlloc(DeviceMemAlloc& alloc) noexcept;

    MemMgrIdT pickNewPoolId() noexcept;
    MemMgrIdT pickNewSubPoolId(Pool& parentPool) noexcept;
    MemMgrIdT pickNewUnpooledAllocId() noexcept;

    bool                                    mbIsValid;              // Is this valid?
    const VkFuncs&                          mVkFuncs;               // Pointer to Vulkan API functions
    LogicalDevice*                          mpDevice;               // Device this belongs to
    std::map<MemMgrIdT, Pool>               mPools;                 // Memory pools in the device memory manager
    std::map<MemMgrIdT, DeviceMemAlloc>     mUnpooledAllocs;        // Unpooled allocations made
    std::vector<MemMgrIdT>                  mDeviceLocalPools;      // A list of memory pools that are guaranteed local to the device. These probably won't be CPU/host visible unless there is a shared memory architecture.
    std::vector<MemMgrIdT>                  mHostVisiblePools;      // A list of memory pools that are guaranteed host visible. These probably won't be on the device unless there is a shared memory architecture.
    MemMgrIdT                               mNextNewPoolId;         // Use this id for the next memory pool to create
    MemMgrIdT                               mNextUnpooledAllocId;   // Use this for the next unpooled alloc id
    bool                                    mbDeviceMemExhausted;   // A flag which is set whenever device memory is exhausted. Used to prevent excessive attempts at reallocation. Only reset once we free one device only pool.
};

END_NAMESPACE(vgl)
