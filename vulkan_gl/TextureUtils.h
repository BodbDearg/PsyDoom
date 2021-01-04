#pragma once

#include "Macros.h"

#include <cstdint>
#include <vulkan/vulkan.h>

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility functions dealing with textures
//------------------------------------------------------------------------------------------------------------------------------------------
BEGIN_NAMESPACE(vgl)
BEGIN_NAMESPACE(TextureUtils)

bool getMipLevelDimensions(
    const uint32_t baseWidth,
    const uint32_t baseHeight,
    const uint32_t baseDepth,
    const uint32_t mipLevel,
    uint32_t& mipLevelWidth,
    uint32_t& mipLevelHeight,
    uint32_t& mipLevelDepth
) noexcept;

uint32_t getNumMipLevels(
    const uint32_t baseWidth,
    const uint32_t baseHeight,
    const uint32_t baseDepth
) noexcept;

unsigned getNumTexImages(const uint32_t numArrayLayers, const bool bIsCubeMap) noexcept;
uint64_t getPitchInBytes(const VkFormat textureFormat, const uint32_t imageWidth) noexcept;

uint64_t getMipLevelByteSize(
    const VkFormat textureFormat,
    const uint32_t mipLevelWidth,
    const uint32_t mipLevelHeight,
    const uint32_t mipLevelDepth
) noexcept;

uint64_t getMipLevelsByteSize(
    const VkFormat textureFormat,
    const uint32_t baseMipLevelWidth,
    const uint32_t baseMipLevelHeight,
    const uint32_t baseMipLevelDepth,
    const uint32_t numMipLevels,
    const uint32_t mipLevelAlignment
) noexcept;

uint64_t getTotalTextureByteSize(
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t depth,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const bool bIsCubemap,
    const uint32_t imageAlignment
) noexcept;

END_NAMESPACE(TextureUtils)
END_NAMESPACE(vgl)
