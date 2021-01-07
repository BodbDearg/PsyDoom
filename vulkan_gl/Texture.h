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
    bool isLocked() const noexcept;

    std::byte* lock() noexcept;
    std::byte* lock(
        const uint32_t offsetX,
        const uint32_t offsetY,
        const uint32_t offsetZ,
        const uint32_t startLayer,
        const uint32_t sizeX,
        const uint32_t sizeY,
        const uint32_t sizeZ,
        const uint32_t numLayers
    ) noexcept;

    void unlock(TransferTask* const pTransferTaskOverride = nullptr) noexcept;

    inline bool didATextureUpload() const noexcept { return mbDidATextureUpload; }
    inline std::byte* getLockedBytes() const noexcept { return mpLockedBytes; }
    inline uint64_t getLockedSizeInBytes() const noexcept { return mLockedSizeInBytes; }
    inline uint32_t getLockedOffsetX() const noexcept { return mLockedOffsetX; }
    inline uint32_t getLockedOffsetY() const noexcept { return mLockedOffsetY; }
    inline uint32_t getLockedOffsetZ() const noexcept { return mLockedOffsetZ; }
    inline uint32_t getLockedSizeX() const noexcept { return mLockedSizeX; }
    inline uint32_t getLockedSizeY() const noexcept { return mLockedSizeY; }
    inline uint32_t getLockedSizeZ() const noexcept { return mLockedSizeZ; }
    inline uint32_t getLockedStartLayer() const noexcept { return mLockedStartLayer; }
    inline uint32_t getLockedNumLayers() const noexcept { return mLockedNumLayers; }

private:
    // Copy and move assign disallowed
    Texture(const Texture& other) = delete;
    Texture& operator = (const Texture& other) = delete;
    Texture& operator = (Texture&& other) = delete;
    
    bool            mbDidATextureUpload;        // Flag set to true after we schedule the first texture upload (might not have EXECUTED yet!)
    VkBuffer        mLockedVkStagingBuffer;     // If the texture is locked this is the staging buffer being used for transfer
    std::byte*      mpLockedBytes;              // If the texture is locked then these are the data bytes locked
    uint64_t        mLockedSizeInBytes;         // Size of the locked area in bytes including padding between mipmap levels, minimum block size alignment etc.
    uint32_t        mLockedOffsetX;             // Currently locked area: offset x
    uint32_t        mLockedOffsetY;             // Currently locked area: offset y
    uint32_t        mLockedOffsetZ;             // Currently locked area: offset z
    uint32_t        mLockedStartLayer;          // Currently locked area: start layer
    uint32_t        mLockedSizeX;               // Currently locked area: size x
    uint32_t        mLockedSizeY;               // Currently locked area: size y
    uint32_t        mLockedSizeZ;               // Currently locked area: size z
    uint32_t        mLockedNumLayers;           // Currently locked area: number of layers
};

END_NAMESPACE(vgl)
