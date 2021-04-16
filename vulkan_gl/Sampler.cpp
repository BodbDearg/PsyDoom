#include "Sampler.h"

#include "Finally.h"
#include "LogicalDevice.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized sampler
//------------------------------------------------------------------------------------------------------------------------------------------
Sampler::Sampler() noexcept
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mVkSampler(VK_NULL_HANDLE)
    , mSettings()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a sampler to this object
//------------------------------------------------------------------------------------------------------------------------------------------
Sampler::Sampler(Sampler&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mpDevice(other.mpDevice)
    , mVkSampler(other.mVkSampler)
    , mSettings(other.mSettings)
{
    other.mbIsValid = false;
    other.mpDevice = nullptr;
    other.mVkSampler = VK_NULL_HANDLE;
    other.mSettings = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the sampler
//------------------------------------------------------------------------------------------------------------------------------------------
Sampler::~Sampler() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the sampler with the specified settings
//-----------------------------------------------------------------------------------------------------------------------------------------
bool Sampler::init(LogicalDevice& device, const SamplerSettings& settings) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // Save for future reference
    mpDevice = &device;
    mSettings = settings;

    // Fill in the sampler settings and create the sampler
    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.flags = settings.flags;
    createInfo.magFilter = settings.magFilter;
    createInfo.minFilter = settings.minFilter;
    createInfo.mipmapMode = settings.mipmapMode;
    createInfo.addressModeU = settings.addressModeU;
    createInfo.addressModeV = settings.addressModeV;
    createInfo.addressModeW = settings.addressModeW;
    createInfo.mipLodBias = settings.mipLodBias;
    createInfo.anisotropyEnable = settings.anisotropyEnable;
    createInfo.maxAnisotropy = settings.maxAnisotropy;
    createInfo.minLod = settings.minLod;
    createInfo.maxLod = settings.maxLod;
    createInfo.borderColor = settings.borderColor;
    createInfo.unnormalizedCoordinates = settings.unnormalizedCoordinates;

    const VkFuncs& vkFuncs = device.getVkFuncs();

    if (vkFuncs.vkCreateSampler(mpDevice->getVkDevice(), &createInfo, nullptr, &mVkSampler) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a Vulkan sampler object!");
        return false;
    }

    // Success!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the sampler and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void Sampler::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Cleanup and destroy the Vulkan sampler
    mbIsValid = false;

    if (mVkSampler) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroySampler(mpDevice->getVkDevice(), mVkSampler, nullptr);
        mVkSampler = VK_NULL_HANDLE;
    }

    mSettings = {};
    mpDevice = nullptr;
}

END_NAMESPACE(vgl)
