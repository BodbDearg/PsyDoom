#pragma once

#include "Asserts.h"
#include "Utils.h"

#include <cstdint>
#include <cstring>
#include <type_traits>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility functionality related to the 'Abstract Fixed Quad Tree Memory Manager' class
//------------------------------------------------------------------------------------------------------------------------------------------
namespace AbstractFixedQTreeMemMgrUtils {
    // The result of an allocation request
    struct AllocResult {
        // The size of the allocated region in the address space managed by the memory manager.
        // May be bigger than what was requested. 0 if allocation failed.
        uint64_t size;

        // The offset of the allocated region within the address space managed by the memory manager.
        // Undefined if allocation failed. Note that the address space always starts at offset '0', so this
        // does not mean a null pointer or bad allocation; check the 'size' field for a failed allocation instead.
        uint64_t offset;
    };

    // Utility: compute the number of blocks for a given tier
    constexpr inline uint64_t getNumBlocksForTier(const uint64_t tier) noexcept {
        return uint64_t(1) << tier * 2;
    }

    // Utility: compute the total number of blocks among all tiers for the given tier count
    constexpr inline uint64_t getNumBlocksForAllTiers(const uint64_t numTiers) noexcept {
        return (numTiers > 0) ? getNumBlocksForTier(numTiers) + getNumBlocksForAllTiers(numTiers - 1) : 1;
    }

    // Computes the data type used to hold block indices or counts/sizes for a manager with the given number of tiers.
    // Chooses 16-bit indices to save memory where possible!
    template <uint64_t NumTiers>
    using BlockSizeTForNumTiers = std::conditional_t<
        (AbstractFixedQTreeMemMgrUtils::getNumBlocksForAllTiers(NumTiers) <= UINT16_MAX),
        uint16_t,
        uint32_t
    >;

    // Compute the total address space size for a memory manager with a given number of tiers and a given unit size
    constexpr inline uint64_t getByteCapacityForMgr(const uint64_t unitSize, const uint64_t numTiers) noexcept {
        return getNumBlocksForTier(numTiers) * unitSize;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Abstract Fixed Quad Tree Memory Manager.
//
// An 'abstract' memory manager that keeps track of which regions in a fixed size address space are allocated
// and which provides functionality to query current allocations, as well as to allocate and deallocate.
//
// The memory manager is 'abstract' in the sense that it does not allocate any memory or resources on it's own.
// Instead it just manages an address space which could potentially be tied to anything, including RAM, EPROM, video or
// device specific memory etc. It is a building block for a memory manager to a specific resource essentially.
//
// The manager works by subdividing the entire address space recursively into four sub-divisions, 'NumTiers' times.
// The amount of storage provided is determined by the number of tiers in the manager and the memory 'UnitSize' provided.
// The memory 'UnitSize' determines the size in bytes of the smallest blocks at the lowest level tier in the manager.
//
// Overall there will be '4 ^ NumTiers' units of memory in the manager and the memory overhead of the manager
// depends on the total number of units, therefore keeping the tier count as low as possible is recommended.
//
// ASCII illustration:
//
//  Tier 0  |                     (Entire address space)                    |           |   Higher tiers
//  Tier 1  |               |               |               |               |           |
//  Tier 2  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |           |
//  Tier 3  |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||          \ /
//  ...                                                                                 .   Lower tiers
//
// Provided an allocation does not fall below the 'UnitSize' provided, the absolute maximum wastage of memory per allocation will be 50%.
// This is an absolute worst case scenario however, and under typical usage scenarios wastage would likely be considerably less.
// For most allocation sizes the maximum wastage is closer to 33%. This wastage is a tradeoff to help fight memory fragmentation
// and simplify the manager. If the sizes requested are nice even powers of two or multiples of powers of two numbers then wastage
// should be very small or non-existent.
//
// Overall this manager is designed/optimized to allocate larger blocks of more regular sized (power of two) allocations typically used
// in graphics, though it could easily be used for other purposes too. It may not be suitable if tons of smaller allocations are required or
// very arbitrary allocation sizes, though perhaps alternative hybrid strategies could be employed to help handle those cases also.
//------------------------------------------------------------------------------------------------------------------------------------------
template <uint64_t UnitSize, uint64_t NumTiers>
class AbstractFixedQTreeMemMgr {
public:
    // The data type used to hold indexes of blocks or block sizes within the memory manager
    typedef AbstractFixedQTreeMemMgrUtils::BlockSizeTForNumTiers<NumTiers> BlockSizeT;

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Structure holding information for a block at a particular tier
    //--------------------------------------------------------------------------------------------------------------------------------------
    struct Block {
        // True if the block is in use, either because it is directly part of an allocation on the tier that the block belongs
        // to or because it's the parent or ancestor of a block that is part of an allocation.
        bool bIsInUse;

        // If this block represents the head block in a series of blocks belonging to an allocation, tells how many blocks (on this tier)
        // are part of that same allocation. This will be 1-3 for head blocks of an allocation and 0 for blocks that are not head blocks
        // in an allocation or which are not allocated.
        uint8_t allocSizeInBlocks;

        // The maximum number memory units that can be allocated in one contiguous allocation at this node and it's children at this time.
        // If '0' then no allocation is possible and all blocks are in use.
        BlockSizeT maxNewAllocInUnits;
    };

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Initializes the memory manager
    //--------------------------------------------------------------------------------------------------------------------------------------
    AbstractFixedQTreeMemMgr() noexcept {
        // Sanity checks
        static_assert(NumTiers > 0, "There must be at least 1 tier!");
        static_assert(NumTiers <= 15, "Too many tiers in memory manager!");
        static_assert(UnitSize > 0, "Unit size must be greater than zero!");

        // Initialize the offsets into the block info's array for each tier.
        // This only needs to be done once and will be a pattern like:
        //
        //      0, 1, 5, 21, 85
        //
        // and so on...
        {
            BlockSizeT curOffset = 0;
            BlockSizeT* pCurTierOffset = mTierOffsets;
            BlockSizeT* const pEndTierOffsets = mTierOffsets + NumTiers + 1;
            BlockSizeT numBlocksInTier = 1;

            while (pCurTierOffset < pEndTierOffsets) {
                *pCurTierOffset = curOffset;
                curOffset += numBlocksInTier;
                numBlocksInTier <<= 2;
                ++pCurTierOffset;
            }
        }

        // Reset the manager to it's default state of all blocks free
        reset();
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Just some sanity checks on shutdown in debug
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline ~AbstractFixedQTreeMemMgr() noexcept {
        ASSERT_LOG(mNumUnitsFree == getCapacityInUnits(), "All deallocations should be cleaned up before the memory manager is destroyed!");
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Resets the state of all allocations in the memory manager and frees up the entire memory range
    //--------------------------------------------------------------------------------------------------------------------------------------
    void reset() noexcept {
        // The number of memory units free is back to the maximum
        mNumUnitsFree = getCapacityInUnits();

        // Initialize the blocks array
        constexpr uint64_t NUM_BLOCKS = AbstractFixedQTreeMemMgrUtils::getNumBlocksForAllTiers(NumTiers);
        std::memset(mBlocks, 0, sizeof(Block) * NUM_BLOCKS);

        {
            // Loop vars
            uint64_t numTierBlocks = 1;
            BlockSizeT maxAllocInUnits = (1 << NumTiers * 2);
            Block* pCurBlock = mBlocks;
            Block* pEndTierBlocks = mBlocks + 1;
            Block* const pEndBlocks = mBlocks + NUM_BLOCKS;

            // Keep going until we initialize all blocks
            while (pCurBlock < pEndBlocks) {
                // Initializing this tier
                while (pCurBlock < pEndTierBlocks) {
                    pCurBlock->maxNewAllocInUnits = maxAllocInUnits;
                    ++pCurBlock;
                }

                // Move onto the next tier
                numTierBlocks <<= 2;
                maxAllocInUnits >>= 2;
                pEndTierBlocks = pCurBlock + numTierBlocks;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Try to allocate the given number of bytes and return the result
    //--------------------------------------------------------------------------------------------------------------------------------------
    AbstractFixedQTreeMemMgrUtils::AllocResult alloc(const uint64_t numAllocBytes) noexcept {
        // A zero sized alloc always fails
        if (numAllocBytes <= 0)
            return AbstractFixedQTreeMemMgrUtils::AllocResult{ 0, 0 };

        // Get the number of units that we need to allocate for this request
        const uint64_t numUnitsReqForAlloc = getAllocSizeInBytesAsUnits(numAllocBytes);

        // See if there is enough room to satisfy this request by checking the maximum number of units that
        // can be allocated according to the root node. At the root this count will be the absolute biggest
        // thing that can be allocated so if this check fails then we can't do anything else...
        if (numUnitsReqForAlloc > mBlocks[0].maxNewAllocInUnits)
            return AbstractFixedQTreeMemMgrUtils::AllocResult{ 0, 0 };

        // Figure out the tier that we will allocate on:
        const uint64_t allocTier = getAllocTierForSupportedUnitAlloc(numUnitsReqForAlloc);

        // Okay, find where to slot in this allocation.
        // Keep moving down the tiers until we reach the tier to allocate at
        uint64_t curTier = 0;
        uint64_t curTierBlocksOffset = 0;
        uint64_t curTierBlockIdx = 0;
        uint64_t curTierUnitsPerBlock = AbstractFixedQTreeMemMgrUtils::getNumBlocksForTier(NumTiers);

        if (curTier < allocTier) {
            while (true) {
                // Need to move down another tier, do that now
                ++curTier;
                curTierBlocksOffset = mTierOffsets[curTier];
                curTierUnitsPerBlock >>= 2;
                curTierBlockIdx <<= 2;

                // Now at the desired allocation tier, stop here
                if (curTier == allocTier)
                    break;

                // Going to move down another tier in the next loop iteration.
                // See which of the 4 child blocks can accomodate the allocation we want to make.
                // There MUST be at least 1 because of what our 'max new alloc in units' check determined against the root:
                Block* const pChildBlocks = mBlocks + (curTierBlocksOffset + curTierBlockIdx);

                if (pChildBlocks[0].maxNewAllocInUnits >= numUnitsReqForAlloc) {
                    continue;
                }

                // Couldn't use child block 1, try 2 instead
                if (pChildBlocks[1].maxNewAllocInUnits >= numUnitsReqForAlloc) {
                    curTierBlockIdx += 1;
                    continue;
                }

                // Couldn't use child block 2, try 3 instead
                if (pChildBlocks[2].maxNewAllocInUnits >= numUnitsReqForAlloc) {
                    curTierBlockIdx += 2;
                    continue;
                }

                // Failing all else we *MUST* be able to use child block 4
                ASSERT(pChildBlocks[3].maxNewAllocInUnits >= numUnitsReqForAlloc);
                curTierBlockIdx += 3;
            }
        }

        // Figure out how many blocks to allocate on this tier
        ASSERT(curTierUnitsPerBlock > 0);
        const uint64_t numTierBlocksToAlloc = Utils::udivCeil(numUnitsReqForAlloc, curTierUnitsPerBlock);
        const uint64_t actualNumUnitsToAlloc = numTierBlocksToAlloc * curTierUnitsPerBlock;

        // Expect the number of tier blocks to alloc to be between 1-3, or 1 if we are at the root tier (0).
        // We never allocate 4 blocks in a tier, the allocator will always chose a tier above to keep the tier block count between 1 and 3.
        ASSERT((numTierBlocksToAlloc >= 1) && (numTierBlocksToAlloc <= 3));
        ASSERT((curTier != 0) || (numTierBlocksToAlloc == 1));

        // Figure out the head block in this allocation from the next (up to 4) blocks.
        // Also compute it's index in the tier:
        Block* const pTierBlocks = mBlocks + curTierBlocksOffset;
        Block* const pHeadAllocBlock = getHeadBlockForAlloc(pTierBlocks + curTierBlockIdx, numTierBlocksToAlloc);
        BlockSizeT allocTierBlockIdx = static_cast<BlockSizeT>(pHeadAllocBlock - pTierBlocks);

        // Now fill in all the allocated blocks on this tier
        {
            // Fill in the head block first
            Block* pBlock = pHeadAllocBlock;
            pBlock->bIsInUse = true;
            pBlock->allocSizeInBlocks = static_cast<uint8_t>(numTierBlocksToAlloc);
            pBlock->maxNewAllocInUnits = 0;

            // Fill in any other blocks
            if (numTierBlocksToAlloc > 1) {
                ++pBlock;
                ASSERT_LOG(pBlock->allocSizeInBlocks == 0, "Expect alloc size in blocks not to need clearing to zero!");
                pBlock->bIsInUse = true;
                pBlock->maxNewAllocInUnits = 0;

                if (numTierBlocksToAlloc > 2) {
                    ++pBlock;
                    ASSERT_LOG(pBlock->allocSizeInBlocks == 0, "Expect alloc size in blocks not to need clearing to zero!");
                    pBlock->bIsInUse = true;
                    pBlock->maxNewAllocInUnits = 0;
                }
            }
        }

        // Update the number of units free in the manager
        mNumUnitsFree -= static_cast<BlockSizeT>(actualNumUnitsToAlloc);

        // Save the info on the allocation, will return later
        AbstractFixedQTreeMemMgrUtils::AllocResult allocResult{
            actualNumUnitsToAlloc * UnitSize,
            allocTierBlockIdx * curTierUnitsPerBlock * UnitSize
        };

        // Update the 'max new alloc in units' field on all parent blocks in the tree
        while (curTier > 0) {
            // Save some child info before we move up to the next tier.
            //
            // Note that we save the index to the 1st child block in the group of 4, as we will be examining all 4.
            // We might not neccesarily be on this, so we have to adjust here...
            const uint64_t childTierBlocksOffset = curTierBlocksOffset;
            const uint64_t childTierBlockIdx = curTierBlockIdx - (curTierBlockIdx % 4);

            // Move up to the parent tier
            --curTier;
            curTierBlocksOffset = mTierOffsets[curTier];
            curTierBlockIdx >>= 2;

            // Get the first and end child block pointer
            Block* const pFirstChildBlock = mBlocks + (childTierBlocksOffset + childTierBlockIdx);
            Block* const pEndChildBlocks = pFirstChildBlock + 4;

            // Get the max contiguous alloc on the child blocks
            BlockSizeT maxContiguousAllocInUnits = getMaxContiguousAllocInUnits(pFirstChildBlock, pEndChildBlocks);

            // Save that max contiguous alloc on the parent block and mark the parent as in use
            Block* const pParentBlock = mBlocks + (curTierBlocksOffset + curTierBlockIdx);
            pParentBlock->maxNewAllocInUnits = maxContiguousAllocInUnits;
            pParentBlock->bIsInUse = true;
        }

        // Return info on the allocation made
        return allocResult;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Dealloc the allocation at the given offset.
    // Expects that the given offset points to a valid allocation, undefined behavior will result otherwise.
    //--------------------------------------------------------------------------------------------------------------------------------------
    void dealloc(const uint64_t allocOffsetInBytes) noexcept {
        // Sanity check this is an actual valid alloc & get the alloc offset in units
        ASSERT(isAlloc(allocOffsetInBytes));
        const BlockSizeT allocOffsetInUnits = static_cast<BlockSizeT>(allocOffsetInBytes / UnitSize);

        // Start at this offset in the lowest tier in the manager and move up the tiers until we find the tier where this allocation was
        // actually made. We can tell that by seeing if the block has an alloc size in blocks greater than '0'.
        BlockSizeT curTier = NumTiers;
        BlockSizeT curTierBlockIdx = allocOffsetInUnits;
        BlockSizeT curTierUnitsPerBlock = 1;
        BlockSizeT curTierBlocksOffset;

        while (true) {
            // Get the current block we are on
            curTierBlocksOffset = mTierOffsets[curTier];
            Block* pBlock = mBlocks + (curTierBlocksOffset + curTierBlockIdx);

            // See if this is the alloc we are looking for, by checking to see if there is an allocation size. The tier where the
            // allocation was actually made will be the only tier to have the alloc size, all blocks within child tiers of an 
            // allocation will have the alloc size set to '0'.
            const uint8_t allocSizeInBlocks = pBlock->allocSizeInBlocks;
            ASSERT((allocSizeInBlocks >= 0) && (allocSizeInBlocks <= 3));

            if (allocSizeInBlocks > 0) {
                // This is the alloc, figure out the range of blocks to be dealloced and zero out the alloc size
                ASSERT_LOG(
                    (curTierBlockIdx * curTierUnitsPerBlock == allocOffsetInUnits),
                    "An invalid alloc offset was specified, and now that will result in something else being freed!"
                );

                Block* pEndBlock = pBlock + allocSizeInBlocks;
                pBlock->allocSizeInBlocks = 0;

                // Now dealloc all blocks in this allocation
                while (pBlock < pEndBlock) {
                    // Mark the block as not in use, set the max alloc that can be done and move on.
                    ASSERT_LOG((pBlock->allocSizeInBlocks == 0), "Expect alloc size in blocks to be zeroed out!");
                    pBlock->bIsInUse = false;
                    pBlock->maxNewAllocInUnits = curTierUnitsPerBlock;
                    ++pBlock;
                }

                // Update the free units count on the manager and break out of the loop since we are now done
                mNumUnitsFree += allocSizeInBlocks * curTierUnitsPerBlock;
                break;
            }

            // Move up to the next tier if there is one
            if (curTier <= 0)
                break;

            --curTier;
            curTierBlockIdx >>= 2;
            curTierUnitsPerBlock <<= 2;
        }

        // Update the maximum sized alloc in all parent nodes
        while (curTier > 0) {
            // Save child tier offset and move up to the next tier
            const BlockSizeT childTierBlocksOffset = curTierBlocksOffset;
            --curTier;
            curTierUnitsPerBlock <<= 2;
            curTierBlocksOffset = mTierOffsets[curTier];

            // Compute parent block index and first child block index (of the 4 children)
            curTierBlockIdx >>= 2;
            const BlockSizeT childTierBlockIdx = curTierBlockIdx << 2;

            // Get the parent block and first/end child pointers
            Block* const pParent = mBlocks + (curTierBlocksOffset + curTierBlockIdx);
            Block* const pFirstChild = mBlocks + (childTierBlocksOffset + childTierBlockIdx);
            Block* const pEndChild = pFirstChild + 4;

            // Update the maximum alloc possible on the parent
            BlockSizeT maxContiguousAlloc = getMaxContiguousAllocInUnits(pFirstChild, pEndChild);
            pParent->maxNewAllocInUnits = maxContiguousAlloc;

            // If all child blocks are now free then the parent is no longer in use
            if (maxContiguousAlloc >= curTierUnitsPerBlock) {
                ASSERT(maxContiguousAlloc == curTierUnitsPerBlock);
                pParent->bIsInUse = false;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Tells if there is a valid allocation at the given address
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline bool isAlloc(const uint64_t allocOffsetInBytes) noexcept {
        return (getAllocSizeInUnits(allocOffsetInBytes) > 0);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Gives the size in bytes of the allocation at the given address; returns 0 if there is no valid allocation at the given address
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline uint64_t getAllocSizeInBytes(const uint64_t allocOffsetInBytes) noexcept {
        getAllocSizeInUnits(allocOffsetInBytes) * UnitSize;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Gives the size in units of the allocation at the given address; returns 0 if there is no valid allocation at the given address
    //--------------------------------------------------------------------------------------------------------------------------------------
    BlockSizeT getAllocSizeInUnits(const uint64_t allocOffsetInBytes) noexcept {
        // If beyond the address space then it's not a valid alloc
        if (allocOffsetInBytes >= getCapacityInBytes())
            return 0;

        // If the bytes are misaligned then it's not a valid alloc
        if (allocOffsetInBytes % UnitSize != 0)
            return 0;

        // Get the alloc offset in units
        const BlockSizeT allocOffsetInUnits = static_cast<BlockSizeT>(allocOffsetInBytes / UnitSize);

        // Start at this offset in the lowest tier in the manager and move up the tiers until we
        // find where this allocation was actually made or find there is no allocation
        BlockSizeT curTier = NumTiers;
        BlockSizeT curTierBlockIdx = allocOffsetInUnits;
        BlockSizeT curTierUnitsPerBlock = 1;

        while (true) {
            // Get the current block we are on
            const BlockSizeT curTierBlocksOffset = mTierOffsets[curTier];
            Block* const pBlock = mBlocks + (curTierBlocksOffset + curTierBlockIdx);

            // See if this is the alloc we are looking for
            const uint8_t allocSizeInBlocks = pBlock->allocSizeInBlocks;
            ASSERT((allocSizeInBlocks >= 0) && (allocSizeInBlocks <= 3));

            if (allocSizeInBlocks > 0) {
                // This is the alloc we are looking for, return it's size in units
                return allocSizeInBlocks * curTierUnitsPerBlock;
            }

            // Move up to the next tier if there is one
            if (curTier <= 0)
                break;

            --curTier;
            curTierBlockIdx >>= 2;
            curTierUnitsPerBlock <<= 2;

            // Compute the offset in bytes we are currently at and if we've moved before the given alloc offset
            // in bytes then this is not a valid alloc so return 0
            const uint64_t curByteOffset = static_cast<uint64_t>(curTierBlockIdx) * curTierUnitsPerBlock * UnitSize;

            if (curByteOffset < allocOffsetInBytes)
                return 0;
        }

        // Invalid alloc, couldn't find the alloc we were looking for!
        return 0; 
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Tells if there are any allocations in the memory manager
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline bool hasAllocations() const noexcept {
        // There are allocations if the root block is flagged in use
        return mBlocks[0].bIsInUse;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Return the number memory units and bytes allocated and free
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline BlockSizeT getNumUnitsFree() const noexcept { return mNumUnitsFree; }
    inline uint64_t getNumBytesFree() const noexcept { return mNumUnitsFree * UnitSize; }
    inline BlockSizeT getNumUnitsAllocated() const noexcept { return getCapacityInUnits() - mNumUnitsFree; }
    inline uint64_t getNumBytesAllocated() const noexcept { return getNumUnitsAllocated() * UnitSize; }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Returns the total number of memory units and bytes that can be allocated in the memory manager
    //--------------------------------------------------------------------------------------------------------------------------------------
    constexpr inline BlockSizeT getCapacityInUnits() const noexcept {
        return static_cast<BlockSizeT>(AbstractFixedQTreeMemMgrUtils::getNumBlocksForTier(NumTiers));
    }

    constexpr inline uint64_t getCapacityInBytes() const noexcept {
        return AbstractFixedQTreeMemMgrUtils::getByteCapacityForMgr(UnitSize, NumTiers);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Return the maximum number of memory units and bytes that can currently be allocated without failure
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline BlockSizeT getMaxGuaranteedAllocInUnits() const noexcept {
        return mBlocks[0].maxNewAllocInUnits;
    }

    inline uint64_t getMaxGuaranteedAllocInBytes() const noexcept {
        return mBlocks[0].maxNewAllocInUnits * UnitSize;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Returns the total number of blocks and tiers of blocks in the memory manager
    //--------------------------------------------------------------------------------------------------------------------------------------
    constexpr inline BlockSizeT getNumBlocks() const noexcept {
        return static_cast<BlockSizeT>(AbstractFixedQTreeMemMgrUtils::getNumBlocksForAllTiers(NumTiers));
    }

    constexpr inline uint64_t getNumTiers() const noexcept {
        return NumTiers;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Get the array holding the offsets of each tier in the blocks array and the blocks array itself
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline const BlockSizeT* getTierOffsets() const noexcept {
        return mTierOffsets;
    }

    inline const Block* getBlocks() const noexcept {
        return mBlocks;
    }

private:
    // Copy and move are disallowed
    AbstractFixedQTreeMemMgr(const AbstractFixedQTreeMemMgr& other) = delete;
    AbstractFixedQTreeMemMgr(AbstractFixedQTreeMemMgr&& other) = delete;
    AbstractFixedQTreeMemMgr& operator = (const AbstractFixedQTreeMemMgr& other) = delete;
    AbstractFixedQTreeMemMgr& operator = (AbstractFixedQTreeMemMgr&& other) = delete;

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Figure out how many memory units (at a minimum) would be required to service an allocation of a given amount of bytes.
    // Note that due to the way the quad tree works, we might use a lot more units for the allocation than this.
    //--------------------------------------------------------------------------------------------------------------------------------------
    constexpr inline uint64_t getAllocSizeInBytesAsUnits(const uint64_t allocSizeInBytes) noexcept {
        return Utils::udivCeil(allocSizeInBytes, UnitSize);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Figures out which tier to allocate the given allocation in units. The allocation in units is also expected to be supported by this
    // manager potentially, I.E. it MUST not exceed the total amount of memory managed by the manager.
    //--------------------------------------------------------------------------------------------------------------------------------------
    constexpr inline uint64_t getAllocTierForSupportedUnitAlloc(const uint64_t allocSizeInUnits) noexcept {
        // Sanity checks: the alloc size in units MUST be valid for this call
        ASSERT(allocSizeInUnits > 0);
        ASSERT(allocSizeInUnits <= getCapacityInUnits());

        // Method to determine the tier to allocate on:
        // 
        //  (1) Get the highest bit in the number (a)
        //  (2) Get the number with only this bit set and all other bits zeroed (b)
        //  (3) Get half of b (c)
        //  (4) If the alloc size in units is greater than b + c then we need to round up by 1 bit, return:
        //          NumTiers - ((a + 1) / 2)
        //      Otherwise return:
        //          NumTiers - (a / 2)
        //
        // Basically the idea is that every 2 bits represents a tier (since this is a QTree, and 2 bits means 4 possible values)
        // and we are first finding what tier to tentatively use by finding the highest bit. We then consider the remainder with
        // a number where only the highest bit is set and if the remainder is more than half of the number with only the highest
        // bit set then we must round up our calculation by 1 bit.
        //
        // It's hard to explain this without working it out on paper but all that this calculation does is to ensure that we can
        // accomodate the specified allocation in 1-3 blocks (inclusive) on the chosen tier.
        //
        // Note that we never allow 4 or more blocks to be allocated contiguously on a given tier since that will either be wasteful
        // (if 4 blocks are required we should move up a tier!) or will overun/use the child blocks managed by another parent block
        // on a higher tier (5+ blocks case).
        const uint64_t a = Utils::highestSetBit(allocSizeInUnits);
        const uint64_t b = uint64_t(1) << a;
        const uint64_t c = b >> uint64_t(1);

        if (allocSizeInUnits > b + c)
            return NumTiers - ((a + 1) >> uint64_t(1));
        
        return NumTiers - (a >> uint64_t(1));
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Gives the first block in a group of up to 4 consequtive blocks, starting at the given pointer, which can service the
    // requested allocation. The allocation is specified in terms of the number of blocks to allocate on this tier.
    //
    // N.B! EXPECTS that the allocation can be serviced, this function is called a point where this should always be the case.
    //--------------------------------------------------------------------------------------------------------------------------------------
    Block* getHeadBlockForAlloc(Block* const pFirstBlock, const uint64_t numBlocksToAlloc) const noexcept {
        // Begin searching for the block we are looking for
        ASSERT((numBlocksToAlloc >= 1) && (numBlocksToAlloc <= 3));
        Block* pBlock = pFirstBlock;

        while (true) {
            // Find the next free block
            while (pBlock->bIsInUse) {
                ++pBlock;
                ASSERT(pBlock - pFirstBlock <= 3);
            }

            // If we are just allocing 1 block then we are done now, just return the free block we found
            if (numBlocksToAlloc <= 1) {
                ASSERT(numBlocksToAlloc == 1);
                return pBlock;
            }

            // Make sure there are enough contiguous blocks free following that block if we are doing a multi block allocation instead:
            uint64_t numContiguousFreeBlocks = 1;
            Block* pEndBlock = pBlock;

            do {
                ++pEndBlock;
                ASSERT(pEndBlock - pFirstBlock <= 3);

                if (pEndBlock->bIsInUse) {
                    // If this follow on block is in use then we have to start over :(
                    pBlock = pEndBlock + 1;
                    break;
                }

                ++numContiguousFreeBlocks;
            } while (numContiguousFreeBlocks < numBlocksToAlloc);

            // See if we succeeded
            if (numContiguousFreeBlocks >= numBlocksToAlloc) {
                ASSERT(numContiguousFreeBlocks == numBlocksToAlloc);
                return pBlock;
            }
        }

        ASSERT_FAIL("Should never reach here!");
        return nullptr;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Get the maximum contiguous alloc in units in the given list of blocks
    //--------------------------------------------------------------------------------------------------------------------------------------
    BlockSizeT getMaxContiguousAllocInUnits(const Block* const pStartBlock, const Block* const pEndBlock) const noexcept {
        BlockSizeT maxContigAllocInUnits = 0;
        BlockSizeT curContigAllocInUnits = 0;

        for (const Block* pCurBlock = pStartBlock; pCurBlock < pEndBlock; ++pCurBlock) {
            // Terminate this contiguous block of mem if the current block is in use
            if (pCurBlock->bIsInUse) {
                maxContigAllocInUnits = std::max(maxContigAllocInUnits, curContigAllocInUnits);
                maxContigAllocInUnits = std::max(maxContigAllocInUnits, pCurBlock->maxNewAllocInUnits);
                curContigAllocInUnits = 0;
            }
            else {
                ASSERT(pCurBlock->maxNewAllocInUnits > 0);
                curContigAllocInUnits += pCurBlock->maxNewAllocInUnits;
            }
        }

        maxContigAllocInUnits = std::max(maxContigAllocInUnits, curContigAllocInUnits);
        return maxContigAllocInUnits;
    }

    // The total number of memory units free in the memory manager
    BlockSizeT mNumUnitsFree;

    // Gives the offset into the blocks array where the blocks for each tier begin
    BlockSizeT mTierOffsets[NumTiers + 1];

    // Holds the blocks for all tiers in the memory manager.
    //
    // Note that if a block in a particular tier is involved in allocation, then all child blocks in lower tiers are ignored and
    // do not have their fields assigned to. This way large allocations don't need to update large amounts of blocks, they can
    // just update 1 parent block (and all of its parents) and be done with it...
    Block mBlocks[AbstractFixedQTreeMemMgrUtils::getNumBlocksForAllTiers(NumTiers)];
};

END_NAMESPACE(vgl)
