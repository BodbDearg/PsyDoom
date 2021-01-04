#pragma once

#include "AlphaMode.h"
#include "BaseTexture.h"

BEGIN_NAMESPACE(vgl)

class TransferTask;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan texture that is stored in CPU visible memory that can be modified at will at any time.
// Can be used as a source to copy texture data to regular on-GPU read only texture, or as a target to copy
// texture data from a render target.
//
// Notes:
//  (1) This texture type may NOT be used in rendering or GPU access of any sort.
//      It is merely a way to get texture data to and from the GPU.
//  (2) This class does not handle any synchronization around transfers!
//      Thus the class should be used as follows:
//          (a) If used as a transfer target then ensure that the 'end of frame' fence for the ringbuffer
//              that the transfer was scheduled in is signalled before assuming completion.
//          (b) If used as a transfer source then be sure to double/triple buffer etc. by the number of
//              ringbuffer slots. This will ensure modifications for the current frame will not affect the
//              previous (potentially in-flight) GPU frame.
//------------------------------------------------------------------------------------------------------------------------------------------
class MutableTexture : public BaseTexture {
public:
    struct Concepts final {
        static constexpr bool IS_RELOCATABLE = true;
    };

    MutableTexture() noexcept;
    MutableTexture(MutableTexture&& other) noexcept;
    ~MutableTexture() noexcept;

    // Get the actual bytes of the texture
    inline std::byte* getBytes() const noexcept { return mDeviceMemAlloc.pBytes; }

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

    // Cleanup/destroy the texture
    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;
      
private:
    // Copy and move assign disallowed
    MutableTexture(const MutableTexture& other) = delete;
    MutableTexture& operator = (const MutableTexture& other) = delete;
    MutableTexture& operator = (MutableTexture&& other) = delete;
};

END_NAMESPACE(vgl)
