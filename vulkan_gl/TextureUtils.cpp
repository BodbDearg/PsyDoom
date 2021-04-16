#include "TextureUtils.h"

#include "Asserts.h"
#include "Utils.h"
#include "VkFormatUtils.h"

#include <algorithm>

BEGIN_NAMESPACE(vgl)
BEGIN_NAMESPACE(TextureUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility that computes the mipmap level dimensions for a given level given a base image size and also given what mip level we are on.
// Returns true if the given mip level exists, or false if the mipmap level specified is beyond the end of the chain.
// In this case all dimensions returned will be zeroed.
//------------------------------------------------------------------------------------------------------------------------------------------
bool getMipLevelDimensions(
    const uint32_t baseWidth,
    const uint32_t baseHeight,
    const uint32_t baseDepth,
    const uint32_t mipLevel,
    uint32_t& mipLevelWidth,
    uint32_t& mipLevelHeight,
    uint32_t& mipLevelDepth
) noexcept {
    // If any of the base dimensions are zero then we can't figure out a mip level dimension
    if ((baseWidth <= 0) || (baseHeight <= 0) || (baseDepth <= 0)) {
        mipLevelWidth = 0;
        mipLevelHeight = 0;
        mipLevelDepth = 0;
        return false;
    }

    // Okay, figure out the size of this mip level.
    // If a particular dimension is zero then clamp it at 1 unless all dimensions are zero,
    // which indicates that an invalid input mip level has been specified.
    mipLevelWidth = baseWidth >> mipLevel;
    mipLevelHeight = baseHeight >> mipLevel;
    mipLevelDepth = baseDepth >> mipLevel;

    if (mipLevelWidth <= 0) {
        if (mipLevelHeight <= 0) {
            if (mipLevelDepth <= 0) {
                // Invalid mipmap level specified!
                return false;
            }

            mipLevelHeight = 1;
        }
        else {
            mipLevelDepth = std::max(mipLevelDepth, 1u);
        }

        mipLevelWidth = 1;
    }
    else {
        mipLevelHeight = std::max(mipLevelHeight, 1u);
        mipLevelDepth = std::max(mipLevelDepth, 1u);
    }

    return true;    // Valid mipmap level
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes the number of mipmap levels for a given image size.
// Returns '0' if any of the dimensions given are zero.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t getNumMipLevels(
    const uint32_t baseWidth,
    const uint32_t baseHeight,
    const uint32_t baseDepth
) noexcept {
    if ((baseWidth <= 0) || (baseHeight <= 0) || (baseDepth <= 0))
        return 0;

    const unsigned maxDimension = std::max(baseWidth, std::max(baseHeight, baseDepth));
    const unsigned highestBitSet = Utils::highestSetBit(maxDimension);
    return highestBitSet + 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out the number of 2d images or faces in a texture.
// Takes into account the number of layers and whether the texture is a cubemap or not.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t getNumTexImages(const uint32_t numArrayLayers, const bool bIsCubeMap) noexcept {
    return (!bIsCubeMap) ? numArrayLayers : numArrayLayers * 6;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes the number of bytes in a single row of an image for the given format.
// Note that this figure is always padded out to the nearest byte.
//
// Returns '0' if the image format is unrecognized.
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t getPitchInBytes(const VkFormat textureFormat, const uint32_t imageWidth) noexcept {
    // Get the actual image width to use for the calculation, taking into account minimum block size
    uint32_t minBlockSizeWidth = {};
    uint32_t minBlockSizeHeight = {};
    uint32_t minBlockSizeDepth = {};

    if (!VkFormatUtils::getMinBlockSizeForFormat(textureFormat, minBlockSizeWidth, minBlockSizeHeight, minBlockSizeDepth))
        return 0;

    const uint32_t actualWidth = Utils::udivCeil(imageWidth, minBlockSizeWidth) * minBlockSizeWidth;    // Align up to block size

    // Get the number of bits per pixel for this format
    const uint32_t bitsPerPixel = VkFormatUtils::getNumBitsPerPixelForFormat(textureFormat);

    if (bitsPerPixel <= 0)
        return 0;

    // Finally compute and return pitch
    const uint64_t bitsPerRow = bitsPerPixel * (uint64_t) actualWidth;
    const uint64_t pitch = (bitsPerRow + 7) / 8;  // Note: aligns to nearest byte boundary!
    return pitch;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the size in bytes of the given mipmap level with the given dimensions in the given texture format. 
// Will take into account minimum block sizes dictated by texture compression formats when computing the
// actual size of the texture. Returns '0' if the texture format is unknown or if any dimension is 0.
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t getMipLevelByteSize(
    const VkFormat textureFormat,
    const uint32_t mipLevelWidth,
    const uint32_t mipLevelHeight,
    const uint32_t mipLevelDepth
) noexcept {
    // Round the mipmap level size to the actual dimensions allowed when taking into account minimum block size
    uint32_t actualWidth = mipLevelWidth;
    uint32_t actualHeight = mipLevelHeight;
    uint32_t actualDepth = mipLevelDepth;

    if (!VkFormatUtils::roundDimensionsToMinBlockSizeForFormat(textureFormat, actualWidth, actualHeight, actualDepth))
        return 0;

    // Compute the pitch or number of bytes per row for the image and use that to determine actual size
    const uint64_t pitch = getPitchInBytes(textureFormat, actualWidth);
    return pitch * actualHeight * actualDepth;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the size in bytes for the given number of mipmap levels in the specified format texture.
// Will take into account min block sizes dictated by compression formats when computing the actual size.
//
// Can also specify a required alignment for each mip level which factors into the size calculation:
// Vulkan requires '4' byte alignment for each mip level for example when performing buffer to image transfer operations.
// Note: The alignment given MUST be in terms of powers of two!
//
// Returns '0' if the texture format is unknown or if any dimension is 0.
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t getMipLevelsByteSize(
    const VkFormat textureFormat,
    const uint32_t baseMipLevelWidth,
    const uint32_t baseMipLevelHeight,
    const uint32_t baseMipLevelDepth,
    const uint32_t numMipLevels,
    const uint32_t mipLevelAlignment
) noexcept {
    ASSERT(mipLevelAlignment > 0);
    ASSERT(Utils::isPowerOf2(mipLevelAlignment));
    ASSERT(numMipLevels <= getNumMipLevels(baseMipLevelWidth, baseMipLevelHeight, baseMipLevelDepth));

    uint64_t totalSize = 0;
    uint32_t mipLevel = 0;

    uint32_t mipLevelWidth = {};
    uint32_t mipLevelHeight = {};
    uint32_t mipLevelDepth = {};

    while ((mipLevel < numMipLevels) && getMipLevelDimensions(
            baseMipLevelWidth,
            baseMipLevelHeight,
            baseMipLevelDepth,
            mipLevel,
            mipLevelWidth,
            mipLevelHeight,
            mipLevelDepth
        )
    )
    {
        totalSize = Utils::ualignUp(totalSize, (uint64_t) mipLevelAlignment);   // Align up to the mip level alignment amount
        totalSize += getMipLevelByteSize(textureFormat, mipLevelWidth, mipLevelHeight, mipLevelDepth);
        ++mipLevel;
    }

    return totalSize;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the total amount of memory required to store the given texture.
//
// Can also specify a required alignment for each image which factors into the size calculation:
// Vulkan requires '4' byte alignment for each image for example when performing buffer to image transfer operations.
// Note: The alignment given MUST be in terms of powers of two!
//
// Returns '0' if the texture format is unknown or if any dimension is 0.
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t getTotalTextureByteSize(
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t depth,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const bool bIsCubemap,
    const uint32_t mipLevelAlignment
) noexcept {
    ASSERT(mipLevelAlignment > 0);
    ASSERT(Utils::isPowerOf2(mipLevelAlignment));

    const uint32_t totalImages = numLayers * (bIsCubemap ? 6 : 1);
    const uint64_t mipLevelsByteSize = getMipLevelsByteSize(
        textureFormat,
        width,
        height,
        depth,
        numMipLevels,
        mipLevelAlignment
    );

    // Note: every image except the last must be aligned as specified so compute in this way
    uint64_t totalSize = mipLevelsByteSize;

    if (totalImages > 1) {
        const uint64_t mipLevelsByteSizeAligned = Utils::ualignUp(mipLevelsByteSize, (uint64_t) mipLevelAlignment);     // Align up to mip level alignment
        totalSize += mipLevelsByteSizeAligned * (totalImages - 1);
    }

    return totalSize;
}

END_NAMESPACE(TextureUtils)
END_NAMESPACE(vgl)
