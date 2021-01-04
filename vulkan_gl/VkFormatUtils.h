#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility functions & queries for texture formats.
// Note that not all formats included in the Vulkan standard will be covered here; just the main desktop ones.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)
BEGIN_NAMESPACE(VkFormatUtils)

bool hasColorAspect(const VkFormat format) noexcept;
bool hasDepthAspect(const VkFormat format) noexcept;
bool hasStencilAspect(const VkFormat format) noexcept;
VkImageAspectFlags getVkImageAspectFlags(const VkFormat format) noexcept;
bool isCompressed(const VkFormat format) noexcept;
bool hasAlpha(const VkFormat format) noexcept;

bool getMinBlockSizeForFormat(
    const VkFormat format,
    uint32_t& minBlockWidth,
    uint32_t& minBlockHeight,
    uint32_t& minBlockDepth
) noexcept;

bool roundDimensionsToMinBlockSizeForFormat(
    const VkFormat format,
    uint32_t& width,
    uint32_t& height,
    uint32_t& depth
) noexcept;

uint32_t getNumBitsPerPixelForFormat(const VkFormat format) noexcept;

END_NAMESPACE(VkFormatUtils)
END_NAMESPACE(vgl)
