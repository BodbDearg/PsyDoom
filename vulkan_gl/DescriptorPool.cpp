#include "DescriptorPool.h"

#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "RetirementMgr.h"
#include "VkFuncs.h"

#include <algorithm>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized descriptor pool
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorPool::DescriptorPool() noexcept
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mVkPool(VK_NULL_HANDLE)
    , mDescriptorSets()
    , mFreeSetIndexes()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a descriptor pool to this object
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mpDevice(other.mpDevice)
    , mVkPool(other.mVkPool)
    , mDescriptorSets(std::move(other.mDescriptorSets))
    , mFreeSetIndexes(std::move(other.mFreeSetIndexes))
{
    other.mbIsValid = false;
    other.mpDevice = nullptr;
    other.mVkPool = VK_NULL_HANDLE;

    // Reparent all sets that are allocated:
    for (DescriptorSet& descriptorSet : mDescriptorSets) {
        if (descriptorSet.mpParentPool) {
            descriptorSet.mpParentPool = this;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the descriptor pool
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorPool::~DescriptorPool() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the pool with the given number of maximum allocations and with the given maximum pool sizes for each descriptor type
//------------------------------------------------------------------------------------------------------------------------------------------
bool DescriptorPool::init(
    LogicalDevice& device,
    const std::vector<VkDescriptorPoolSize>& descriptorPoolSizes,
    const uint32_t maxSetAllocs
) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());
    ASSERT(maxSetAllocs > 0);
    
    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true, true);
        }
    });

    // Save for later reference
    mpDevice = &device;

    // Alloc space for the descriptor sets and init the list of free set indexes
    mDescriptorSets.resize(maxSetAllocs);
    mFreeSetIndexes.resize(maxSetAllocs);

    {
        uint32_t index = maxSetAllocs;

        for (uint32_t& freeIndex : mFreeSetIndexes) {
            --index;
            freeIndex = index;
        }
    }

    // Create the pool
    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;   // Allow freeing individual descriptor sets
    createInfo.maxSets = maxSetAllocs;
    createInfo.poolSizeCount = (uint32_t) descriptorPoolSizes.size();
    createInfo.pPoolSizes = descriptorPoolSizes.data();

    const VkFuncs& vkFuncs = device.getVkFuncs();

    if (vkFuncs.vkCreateDescriptorPool(device.getVkDevice(), &createInfo, nullptr, &mVkPool) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a descriptor pool!");
        return false;
    }

    ASSERT(mVkPool);

    // All went well!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the descriptor pool and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorPool::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Gradual 'retirement' logic if specified and possible
    if ((!bImmediately) && mbIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Regular destruction logic
    ASSERT_LOG(
        (mFreeSetIndexes.size() == mDescriptorSets.size()),
        "Destroying a descriptor pool that has descriptor sets allocated against it!"
    );

    mbIsValid = false;

    if (mVkPool) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyDescriptorPool(mpDevice->getVkDevice(), mVkPool, nullptr);
        mVkPool = VK_NULL_HANDLE;
    }
    
    mFreeSetIndexes.clear();
    mDescriptorSets.clear();
    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells the maximum number of sets that can be allocated.
// Note that the number of sets which can actually be allocated in practice may be lower than the maximum number of sets supported,
// depending on what way pool resources are setup and which sets of which layouts have been allocated from the pool.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t DescriptorPool::getMaxSetAllocs() const noexcept {
    return static_cast<uint32_t>(mDescriptorSets.size());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell how many sets are currently allocated from the pool
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t DescriptorPool::getNumAllocedSets() const noexcept {
    return static_cast<uint32_t>(mDescriptorSets.size() - mFreeSetIndexes.size());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the maximum number of sets that could be possibly allocated.
// Note that the actual amount could be much lower in reality!
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t DescriptorPool::getMaxSetsFree() const noexcept {
    return static_cast<uint32_t>(mFreeSetIndexes.size());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Alloc a descriptor set of the given layout and return the descriptor set object.
// Returns null if the allocation failed.
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorSet* DescriptorPool::allocDescriptorSet(const DescriptorSetLayout& layout) noexcept {
    // Preconditions
    ASSERT(mbIsValid);
    ASSERT(mpDevice->getVkDevice());
    ASSERT(layout.isValid());

    // If we have reached the maximum number of allocations then we're done
    if (mFreeSetIndexes.empty())
        return nullptr;

    // Attempt the vulkan descriptor set alloc. If that fails then bail out...
    const VkDescriptorSetLayout vkDescriptorSetLayout = layout.getVkLayout();

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mVkPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &vkDescriptorSetLayout;

    VkDescriptorSet vkDescriptorSet = {};
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    if (vkFuncs.vkAllocateDescriptorSets(mpDevice->getVkDevice(), &allocInfo, &vkDescriptorSet) != VK_SUCCESS)
        return nullptr;

    // Init and return the allocated descriptor set object:
    DescriptorSet& descriptorSet = mDescriptorSets[mFreeSetIndexes.back()];
    mFreeSetIndexes.pop_back();
    descriptorSet.init(*this, vkDescriptorSet);

    return &descriptorSet;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free a single Vulkan descriptor set.
// Note: the descriptor set should be valid and have been previously allocated from this pool.
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorPool::freeDescriptorSet(DescriptorSet& descriptorSet) noexcept {
    // Preconditions
    ASSERT(mbIsValid);
    ASSERT(mpDevice->getVkDevice() != nullptr);
    ASSERT(descriptorSet.getParentPool() == this);

    // Figure out the set's index in the array of descriptor sets
    uint32_t setIndex = UINT32_MAX;
    const DescriptorSet* const pDescSetsBeg = mDescriptorSets.data();
    const DescriptorSet* const pDescSetsEnd = mDescriptorSets.data() + mDescriptorSets.size();

    if ((&descriptorSet >= pDescSetsBeg) && (&descriptorSet < pDescSetsEnd)) {
        setIndex = (uint32_t)(&descriptorSet - pDescSetsBeg);
    }
    
    ASSERT(setIndex < mDescriptorSets.size());

    // Free the descriptor set
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    if (vkFuncs.vkFreeDescriptorSets(mpDevice->getVkDevice(), mVkPool, 1, &descriptorSet.mVkDescriptorSet) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to free descriptor sets!");
    }

    descriptorSet.release();

    // Put this index back into circulation
    mFreeSetIndexes.push_back(setIndex);
}

END_NAMESPACE(vgl)
