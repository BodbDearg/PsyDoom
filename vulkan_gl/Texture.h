#pragma once

#include "BaseTexture.h"
#include "AlphaMode.h"

BEGIN_NAMESPACE(vgl)

class TransferTask;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan texture used primarily as a read-only texture for rendering.
//
// Notes:
//  (1) The texture can be locked and have it's contents updated if required
//  (2) The texture upload is done asynchronously
//------------------------------------------------------------------------------------------------------------------------------------------
class Texture : public BaseTexture {
public:
    struct Concepts final {
        static constexpr bool IS_RELOCATABLE = true;
    };

    Texture() noexcept;
    Texture(Texture&& other) noexcept;
    ~Texture() noexcept;

    bool initAs1dTexture(
        LogicalDevice& device,
        const VkFormat textureFormat,
        const uint32_t width,
        const uint32_t numLayers = 1,
        const uint32_t numMipLevels = 1,
        const AlphaMode alphaMode = AlphaMode::UNSPECIFIED
    ) noexcept;

    bool initAs2dTexture(
        LogicalDevice& device,
        const VkFormat textureFormat,
        const uint32_t width,
        const uint32_t height,
        const uint32_t numLayers = 1,
        const uint32_t numMipLevels = 1,
        const AlphaMode alphaMode = AlphaMode::UNSPECIFIED
    ) noexcept;

    bool initAs3dTexture(
        LogicalDevice& device,
        const VkFormat textureFormat,
        const uint32_t width,
        const uint32_t height,
        const uint32_t depth,
        const uint32_t numMipLevels = 1,
        const AlphaMode alphaMode = AlphaMode::UNSPECIFIED
    ) noexcept;

    bool initAsCubeTexture(
        LogicalDevice& device,
        const VkFormat textureFormat,
        const uint32_t width,
        const uint32_t height,
        const uint32_t numLayers = 1,
        const uint32_t numMipLevels = 1,
        const AlphaMode alphaMode = AlphaMode::UNSPECIFIED
    ) noexcept;

    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;
    
    bool getIsLocked() const noexcept;
    std::byte* lock() noexcept;
    void unlock(TransferTask* const pTransferTaskOverride = nullptr) noexcept;
    
private:
    // Copy and move assign disallowed
    Texture(const Texture& other) = delete;
    Texture& operator = (const Texture& other) = delete;
    Texture& operator = (Texture&& other) = delete;
    
    /* If the texture is locked this is the staging buffer being used for transfer */
    VkBuffer mLockedVkStagingBuffer;
};

END_NAMESPACE(vgl)
