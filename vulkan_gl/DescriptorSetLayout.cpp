#include "DescriptorSetLayout.h"

#include "Finally.h"
#include "LogicalDevice.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized descriptor set layout
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorSetLayout::DescriptorSetLayout() noexcept
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mVkLayout(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a descriptor set layout to this object
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mpDevice(other.mpDevice)
    , mVkLayout(other.mVkLayout)
{
    other.mbIsValid = false;
    other.mpDevice = nullptr;
    other.mVkLayout = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the descriptor set layout
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorSetLayout::~DescriptorSetLayout() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the descriptor set layout with the given entries
//------------------------------------------------------------------------------------------------------------------------------------------
bool DescriptorSetLayout::init(
    LogicalDevice& device,
    const VkDescriptorSetLayoutBinding* const pLayoutBindings,
    const uint32_t numLayoutBindings
) noexcept {
    // Sanity checks
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.isValid());
    ASSERT(pLayoutBindings || (numLayoutBindings == 0));

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&] {
        if (!mbIsValid) {
            destroy(true, true);
        }
    });

    // Required for later cleanup
    mpDevice = &device;

    // Create the actual descriptor set layout itself
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = numLayoutBindings;
    createInfo.pBindings = pLayoutBindings;

    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    if (vkFuncs.vkCreateDescriptorSetLayout(mpDevice->getVkDevice(), &createInfo, nullptr, &mVkLayout) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a descriptor set layout!");
        return false;
    }

    // All good if we get to here
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the descriptor set layout and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorSetLayout::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
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

    // Normal cleanup logic
    mbIsValid = false;

    if (mVkLayout) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyDescriptorSetLayout(mpDevice->getVkDevice(), mVkLayout, nullptr);
        mVkLayout = VK_NULL_HANDLE;
    }

    mpDevice = nullptr;
}

END_NAMESPACE(vgl)
