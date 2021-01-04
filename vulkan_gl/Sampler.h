#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Settings controlling how a sampler is created and behaves.
// Basically the same as 'VkSamplerCreateInfo' but minus some unneeded stuff and with the option to default init settings.
//------------------------------------------------------------------------------------------------------------------------------------------
struct SamplerSettings {
    VkSamplerCreateFlags    flags;
    VkFilter                magFilter;
    VkFilter                minFilter;
    VkSamplerMipmapMode     mipmapMode;
    VkSamplerAddressMode    addressModeU;
    VkSamplerAddressMode    addressModeV;
    VkSamplerAddressMode    addressModeW;
    float                   mipLodBias;
    VkBool32                anisotropyEnable;
    float                   maxAnisotropy;
    float                   minLod;
    float                   maxLod;
    VkBorderColor           borderColor;
    VkBool32                unnormalizedCoordinates;

    SamplerSettings& setToDefault() noexcept {
        flags = 0;
        magFilter = VK_FILTER_NEAREST;
        minFilter = VK_FILTER_NEAREST;
        mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        mipLodBias = 0;
        anisotropyEnable = false;
        maxAnisotropy = 1.0f;
        minLod = 0.0f;
        maxLod = 32.0f;
        borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        unnormalizedCoordinates = false;
        return *this;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a vulkan sampler
//------------------------------------------------------------------------------------------------------------------------------------------
class Sampler {
public:
    Sampler() noexcept;
    Sampler(Sampler&& other) noexcept;
    ~Sampler() noexcept;

    bool init(LogicalDevice& device, const SamplerSettings& settings) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkSampler getVkSampler() const noexcept { return mVkSampler; }
    inline const SamplerSettings& getSettings() const noexcept { return mSettings; }

private:
    // Copy and move assign are disallowed
    Sampler(const Sampler& other) = delete;
    Sampler& operator = (const Sampler& other) = delete;
    Sampler& operator = (Sampler&& other) = delete;

    bool                mbIsValid;
    LogicalDevice*      mpDevice;
    VkSampler           mVkSampler;
    SamplerSettings     mSettings;
};

END_NAMESPACE(vgl)
