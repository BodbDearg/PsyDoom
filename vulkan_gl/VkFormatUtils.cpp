#include "VkFormatUtils.h"

#include "Asserts.h"

BEGIN_NAMESPACE(vgl)
BEGIN_NAMESPACE(VkFormatUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if the given format has color parts/aspects
//------------------------------------------------------------------------------------------------------------------------------------------
bool hasColorAspect(const VkFormat format) noexcept {
    // The format has a color aspect if it doesn't have a depth or stencil aspect.
    // For all supported formats currently this will be true:
    return ((!hasDepthAspect(format)) && (!hasStencilAspect(format)));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if the given format has depth parts/aspects
//------------------------------------------------------------------------------------------------------------------------------------------
bool hasDepthAspect(const VkFormat format) noexcept {
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;

        default:
            break;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if the given format has stencil parts/aspects
//------------------------------------------------------------------------------------------------------------------------------------------
bool hasStencilAspect(const VkFormat format) noexcept {
    switch (format) {
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;

        default:
            break;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the Vulkan image aspect flags for the given image format
//------------------------------------------------------------------------------------------------------------------------------------------
VkImageAspectFlags getVkImageAspectFlags(const VkFormat format) noexcept {
    return (
        (hasColorAspect(format) ? VK_IMAGE_ASPECT_COLOR_BIT : 0) |
        (hasDepthAspect(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) |
        (hasStencilAspect(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if the given format is compressed
//------------------------------------------------------------------------------------------------------------------------------------------
bool isCompressed(const VkFormat format) noexcept {
    switch (format) {
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
            return true;

        default:
            break;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if the given format has an alpha channel
//------------------------------------------------------------------------------------------------------------------------------------------
bool hasAlpha(const VkFormat format) noexcept {
    switch (format) {
        //==================================================================================================================
        // An unknown or invalid texture format
        //==================================================================================================================
        case VK_FORMAT_UNDEFINED:
            return false;

        //==================================================================================================================
        // Depth/stencil formats
        //==================================================================================================================
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return false;

        //==================================================================================================================
        // 8 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R8_UNORM:            // R
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R4G4_UNORM_PACK8:    // RG
            return false;

        //==================================================================================================================
        // 16 bit formats: 
        //==================================================================================================================
        case VK_FORMAT_R16_UNORM:               // R
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R8G8_UNORM:              // RG
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:     // RGB
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
            return false;

        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:   // RGBA
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            return true;

        //==================================================================================================================
        // 24 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
            return false;

        //==================================================================================================================
        // 32 bit formats: 
        //==================================================================================================================
        case VK_FORMAT_R32_UINT:                    // R
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R16G16_UNORM:                // RG
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:     // RGB
            return false;

        case VK_FORMAT_R8G8B8A8_UNORM:              // RGBA
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            return true;

        //==================================================================================================================
        // 48 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R16G16B16_UNORM:     // RGB
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
            return false;

        //==================================================================================================================
        // 64 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R64_UINT:                    // R
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R32G32_UINT:                 // RG
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
            return false;

        case VK_FORMAT_R16G16B16A16_UNORM:          // RGBA
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return true;

        //==================================================================================================================
        // 96 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R32G32B32_UINT:      // RGB
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
            return false;

        //==================================================================================================================
        // 128 bit formats: 
        //==================================================================================================================
        case VK_FORMAT_R64G64_UINT:             // RG
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
            return false;

        case VK_FORMAT_R32G32B32A32_UINT:       // RGBA
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return true;

        //==================================================================================================================
        // 192 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R64G64B64_UINT:          // RGB
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
            return false;

        //==================================================================================================================
        // 256 bit formats: 
        //==================================================================================================================
        case VK_FORMAT_R64G64B64A64_UINT:       // RGBA
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return true;

        //==================================================================================================================
        // Block compression formats
        //==================================================================================================================
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            return false;

        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
            return true;

        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            return false;

        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
            return true;

        //==================================================================================================================
        // Unhandled/unknown formats
        //==================================================================================================================
        default:
            break;
    }

    ASSERT_FAIL("Unhandled/unsupported format. Need to expand the case list!");
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the minimum block size for the given format, taking into account the compression method used.
// The actual size of textures must be in multiples of this for compressed formats.
//
// For example will return 4,4,1 for BC1 since that requires a block size of 4x4 in 2d.
// Will return 1,1,1 for uncompressed formats.
//
// Returns 'false' on failure to determine the min block size, if the texture format is unknown.
// In this case the output parameters are left undefined.
//------------------------------------------------------------------------------------------------------------------------------------------
bool getMinBlockSizeForFormat(
    const VkFormat format,
    uint32_t& minBlockWidth,
    uint32_t& minBlockHeight,
    uint32_t& minBlockDepth
) noexcept {
    switch (format) {
        //==================================================================================================================
        // An unknown or invalid texture format
        //==================================================================================================================
        case VK_FORMAT_UNDEFINED:
            return false;

        //==================================================================================================================
        // Uncompressed formats (all 1x1x1 block size)
        //==================================================================================================================
        case VK_FORMAT_S8_UINT:                         // Depth/stencil formats
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_R8_UNORM:                        // 8 bit formats: R
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R4G4_UNORM_PACK8:                // 8 bit formats: RG
        case VK_FORMAT_R16_UNORM:                       // 16 bit formats: R
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R8G8_UNORM:                      // 16 bit formats: RG
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:             // 16 bit formats: RGB
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:           // 16 bit formats: RGBA
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_R8G8B8_UNORM:                    // 24 bit formats: RGB
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_R32_UINT:                        // 32 bit formats: R
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R16G16_UNORM:                    // 32 bit formats: RG
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:         // 32 bit formats: RGB
        case VK_FORMAT_R8G8B8A8_UNORM:                  // 32 bit formats: RGBA
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_R16G16B16_UNORM:                 // 48 bit formats: RGB
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R64_UINT:                        // 64 bit formats: R
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R32G32_UINT:                     // 64 bit formats: RG
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UNORM:              // 64 bit formats: RGBA
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:                  // 96 bit formats: RGB
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R64G64_UINT:                     // 128 bit formats: RG
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:               // 128 bit formats: RGBA
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64G64B64_UINT:                  // 192 bit formats: RGB
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_UINT:               // 256 bit formats: RGBA
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            minBlockWidth = 1;
            minBlockHeight = 1;
            minBlockDepth = 1;
            return true;

        //==================================================================================================================
        // Block compression formats (all 4x4x1 block size)
        //==================================================================================================================
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
            minBlockWidth = 4;
            minBlockHeight = 4;
            minBlockDepth = 1;
            return true;

        //==================================================================================================================
        // Unhandled/unknown formats
        //==================================================================================================================
        default:
            break;
    }

    ASSERT_FAIL("Unhandled/unsupported format. Need to expand the case list!");
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility which rounds the given texture dimensions up, taking into account the minimum block size of the specified texture format.
// For example for BC1 will round 1x2x1 up to 4x4x1.
//
// Returns 'false' on failure to determine the min block size, if the texture format is unknown.
// In this case the output parameters are left undefined.
//------------------------------------------------------------------------------------------------------------------------------------------
bool roundDimensionsToMinBlockSizeForFormat(
    const VkFormat format,
    uint32_t& width,
    uint32_t& height,
    uint32_t& depth
) noexcept {
    // Get min block size and bail if that fails:
    uint32_t minBlockWidth = {};
    uint32_t minBlockHeight = {};
    uint32_t minBlockDepth = {};

    if (!getMinBlockSizeForFormat(format, minBlockWidth, minBlockHeight, minBlockDepth))
        return false;

    // Do the rounding
    if (minBlockWidth > 1) {
        const uint32_t rem = width % minBlockWidth;

        if (rem != 0) {
            width = width - rem + minBlockWidth;
        }
    }

    if (minBlockHeight > 1) {
        const uint32_t rem = height % minBlockHeight;

        if (rem != 0) {
            height = height - rem + minBlockHeight;
        }
    }

    if (minBlockDepth > 1) {
        const uint32_t rem = depth % minBlockDepth;

        if (rem != 0) {
            depth = depth - rem + minBlockDepth;
        }
    }

    return true;    // Successfully rounded
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the number of bits per pixel for the given format.
// Note that for block compressed formats, this may be less than a byte.
// Returns '0' on failure to determine, if the texture format is unknown.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t getNumBitsPerPixelForFormat(const VkFormat format) noexcept {
    switch (format) {
        //==================================================================================================================
        // An unknown or invalid texture format
        //==================================================================================================================
        case VK_FORMAT_UNDEFINED:
            return 0;

        //==================================================================================================================
        // Depth/stencil formats
        //==================================================================================================================
        case VK_FORMAT_S8_UINT:               return 8;
        case VK_FORMAT_D16_UNORM:             return 16;
        case VK_FORMAT_D16_UNORM_S8_UINT:     return 24;
        case VK_FORMAT_X8_D24_UNORM_PACK32:   return 32;
        case VK_FORMAT_D24_UNORM_S8_UINT:     return 32;
        case VK_FORMAT_D32_SFLOAT:            return 32;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:    return 40;

        //==================================================================================================================
        // 8 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R8_UNORM:            // R
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R4G4_UNORM_PACK8:    // RG
            return 8;

        //==================================================================================================================
        // 16 bit formats: 
        //==================================================================================================================
        case VK_FORMAT_R16_UNORM:                   // R
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R8G8_UNORM:                  // RG
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:         // RGB
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:       // RGBA
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            return 16;

        //==================================================================================================================
        // 24 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
            return 24;

        //==================================================================================================================
        // 32 bit formats: 
        //==================================================================================================================
        case VK_FORMAT_R32_UINT:                        // R
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R16G16_UNORM:                    // RG
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:         // RGB
        case VK_FORMAT_R8G8B8A8_UNORM:                  // RGBA
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            return 32;

        //==================================================================================================================
        // 48 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R16G16B16_UNORM:         // RGB
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
            return 48;

        //==================================================================================================================
        // 64 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R64_UINT:                // R
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R32G32_UINT:             // RG
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UNORM:      // RGBA
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return 64;

        //==================================================================================================================
        // 96 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R32G32B32_UINT:          // RGB
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
            return 96;

        //==================================================================================================================
        // 128 bit formats: 
        //==================================================================================================================
        case VK_FORMAT_R64G64_UINT:             // RG
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:       // RGBA
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return 128;

        //==================================================================================================================
        // 192 bit formats:
        //==================================================================================================================
        case VK_FORMAT_R64G64B64_UINT:          // RGB
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
            return 192;

        //==================================================================================================================
        // 256 bit formats: 
        //==================================================================================================================
        case VK_FORMAT_R64G64B64A64_UINT:       // RGBA
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return 256;

        //==================================================================================================================
        // Block compression formats
        //==================================================================================================================
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            return 4;

        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
            return 8;

        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
            return 4;

        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
            return 8;

        //==================================================================================================================
        // Unhandled/unknown formats
        //==================================================================================================================
        default:
            break;
    }

    ASSERT_FAIL("Unhandled/unsupported format. Need to expand the case list!");
    return 0;
}

END_NAMESPACE(VkFormatUtils)
END_NAMESPACE(vgl)
