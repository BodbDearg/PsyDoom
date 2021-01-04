#include "PipelineLayout.h"

#include "Defines.h"
#include "DescriptorSetLayout.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "RetirementMgr.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized pipeline layout
//------------------------------------------------------------------------------------------------------------------------------------------
PipelineLayout::PipelineLayout() noexcept
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mVkPipelineLayout(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a pipeline layout to this object
//------------------------------------------------------------------------------------------------------------------------------------------
PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mpDevice(other.mpDevice)
    , mVkPipelineLayout(other.mVkPipelineLayout)
{
    other.mbIsValid = false;
    other.mpDevice = nullptr;
    other.mVkPipelineLayout = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the pipeline layout
//------------------------------------------------------------------------------------------------------------------------------------------
PipelineLayout::~PipelineLayout() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the pipeline layout from the given descriptor set layouts and push constant ranges
//------------------------------------------------------------------------------------------------------------------------------------------
bool PipelineLayout::init(
    LogicalDevice& device,
    VkDescriptorSetLayout* const pDescriptorSetLayouts,
    const uint32_t numDescriptorSetLayouts,
    VkPushConstantRange* const pPushConstantRanges,
    const uint32_t numPushConstantRanges
) noexcept {
    // Sanity checks
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.isValid());
    ASSERT(pDescriptorSetLayouts || (numDescriptorSetLayouts == 0));
    ASSERT(pPushConstantRanges || (numPushConstantRanges == 0));

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&] {
        if (!mbIsValid) {
            destroy(true, true);
        }
    });

    // Save for later use
    mpDevice = &device;

    // Create the pipeline layout using those
    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = numDescriptorSetLayouts;
    layoutCreateInfo.pSetLayouts = pDescriptorSetLayouts;
    layoutCreateInfo.pushConstantRangeCount = numPushConstantRanges;
    layoutCreateInfo.pPushConstantRanges = pPushConstantRanges;

    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    if (vkFuncs.vkCreatePipelineLayout(mpDevice->getVkDevice(), &layoutCreateInfo, nullptr, &mVkPipelineLayout) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a pipeline layout!");
        return nullptr;
    }

    // All good if we get to here!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the pipeline layout and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void PipelineLayout::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
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

    if (mVkPipelineLayout) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyPipelineLayout(mpDevice->getVkDevice(), mVkPipelineLayout, nullptr);
        mVkPipelineLayout = VK_NULL_HANDLE;
    }

    mpDevice = nullptr;
}

END_NAMESPACE(vgl)
