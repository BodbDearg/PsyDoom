#include "Texture.h"

#include "LogicalDevice.h"
#include "RawBuffer.h"
#include "RetirementMgr.h"
#include "TextureUtils.h"
#include "TransferMgr.h"
#include "TransferTask.h"
#include "VkFormatUtils.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized texture
//------------------------------------------------------------------------------------------------------------------------------------------
Texture::Texture() noexcept
    : BaseTexture()
    , mbDidATextureUpload(false)
    , mLockedVkStagingBuffer(VK_NULL_HANDLE)
    , mpLockedBytes(nullptr)
    , mLockedSizeInBytes(0)
    , mLockedOffsetX(0)
    , mLockedOffsetY(0)
    , mLockedOffsetZ(0)
    , mLockedStartLayer(0)
    , mLockedSizeX(0)
    , mLockedSizeY(0)
    , mLockedSizeZ(0)
    , mLockedNumLayers(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a texture to this object
//------------------------------------------------------------------------------------------------------------------------------------------
Texture::Texture(Texture&& other) noexcept
    : BaseTexture(std::move(other))
    , mbDidATextureUpload(other.mbDidATextureUpload)
    , mLockedVkStagingBuffer(other.mLockedVkStagingBuffer)
    , mpLockedBytes(other.mpLockedBytes)
    , mLockedSizeInBytes(other.mLockedSizeInBytes)
    , mLockedOffsetX(other.mLockedOffsetX)
    , mLockedOffsetY(other.mLockedOffsetY)
    , mLockedOffsetZ(other.mLockedOffsetZ)
    , mLockedStartLayer(other.mLockedStartLayer)
    , mLockedSizeX(other.mLockedSizeX)
    , mLockedSizeY(other.mLockedSizeY)
    , mLockedSizeZ(other.mLockedSizeZ)
    , mLockedNumLayers(other.mLockedNumLayers)
{
    other.mbDidATextureUpload = false;
    other.mLockedVkStagingBuffer = VK_NULL_HANDLE;
    other.mpLockedBytes = nullptr;
    other.mLockedSizeInBytes = 0;
    other.mLockedOffsetX = 0;
    other.mLockedOffsetY = 0;
    other.mLockedOffsetZ = 0;
    other.mLockedStartLayer = 0;
    other.mLockedSizeX = 0;
    other.mLockedSizeY = 0;
    other.mLockedSizeZ = 0;
    other.mLockedNumLayers = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the texture
//------------------------------------------------------------------------------------------------------------------------------------------
Texture::~Texture() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a 1d texture.
// Note: does not populate/initialize texture data, just allocates memory, creates the Vulkan image object and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
bool Texture::initAs1dTexture(
    LogicalDevice& device,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_1D,
        (numLayers > 1) ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        DeviceMemAllocMode::PREFER_DEVICE_LOCAL,
        textureFormat,
        width,
        1,              // Height
        1,              // Depth
        numLayers,
        numMipLevels,
        1,              // Num samples
        false,          // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a 2d texture.
// Note: does not populate/initialize texture data, just allocates memory, creates the Vulkan image object and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
bool Texture::initAs2dTexture(
    LogicalDevice& device,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_2D,
        (numLayers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        DeviceMemAllocMode::PREFER_DEVICE_LOCAL,
        textureFormat,
        width,
        height,
        1,              // Depth
        numLayers,
        numMipLevels,
        1,              // Num samples
        false,          // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a 3d texture.
// Note: does not populate/initialize texture data, just allocates memory, creates the Vulkan image object and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
bool Texture::initAs3dTexture(
    LogicalDevice& device,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t depth,
    const uint32_t numMipLevels,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_3D,
        VK_IMAGE_VIEW_TYPE_3D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        DeviceMemAllocMode::PREFER_DEVICE_LOCAL,
        textureFormat,
        width,
        height,
        depth,
        1,              // Num layers
        numMipLevels,
        1,              // Num samples
        false,          // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a cube texture.
// Note: does not populate/initialize texture data, just allocates memory, creates the Vulkan image object and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
bool Texture::initAsCubeTexture(
    LogicalDevice& device,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_2D,
        (numLayers > 1) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        DeviceMemAllocMode::PREFER_DEVICE_LOCAL,
        textureFormat,
        width,
        height,
        1,              // Depth
        numLayers,
        numMipLevels,
        1,              // Num samples
        true,           // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the texture and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void Texture::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions: note not clearing out lock details since they should already be cleared (due to no lock) at this point!
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");
    ASSERT_LOG(!isLocked(), "Destroying an image that has not been unlocked!");

    // Gradual 'retirement' logic if specified and possible
    if ((!bImmediately) && mbIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Cleanup logic: most of this is in the base class
    mbDidATextureUpload = false;
    BaseTexture::destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells whether the texture has been locked for image transfer or not
//------------------------------------------------------------------------------------------------------------------------------------------
bool Texture::isLocked() const noexcept {
    return (mLockedVkStagingBuffer != VK_NULL_HANDLE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convenience overload: locks the entire texture
//------------------------------------------------------------------------------------------------------------------------------------------
std::byte* Texture::lock() noexcept {
    return lock(0, 0, 0, 0, mWidth, mHeight, mDepth, mNumLayers);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Locks the data for the specified region of the texture for all mipmap levels and returns a pointer that can be used to populate
// the new contents of the texture.
//
// Notes:
//  (1) If locking the requested region fails then a null pointer is returned.
//  (2) The memory given is completely undefined upon locking, and may not contain any previous data.
//  (3) The texture must be unlocked in order for it's contents to be uploaded to GPU memory.
//  (4) Each mipmap level in the data MUST start on 4-byte (32-bit) boundaries.
//      Data must be padded between mipmap levels in order to satisfy this requirement.
//  (5) The locked region should be aligned along the block size defined by the texture format.
//      For example a texture format with a 4x4 pixel block size should only be locked in multiples of this block size.
//------------------------------------------------------------------------------------------------------------------------------------------
std::byte* Texture::lock(
    const uint32_t offsetX,
    const uint32_t offsetY,
    const uint32_t offsetZ,
    const uint32_t startLayer,
    const uint32_t sizeX,
    const uint32_t sizeY,
    const uint32_t sizeZ,
    const uint32_t numLayers
) noexcept {
    // Preconditions: must be valid and device must still be valid
    ASSERT(mbIsValid);
    ASSERT(mpDevice && mpDevice->getVkDevice());

    // Preconditions: can't double lock
    ASSERT_LOG(!isLocked(), "Can't lock when already locked!");

    // Preconditions: can't lock a zero sized area
    ASSERT(sizeX > 0);
    ASSERT(sizeY > 0);
    ASSERT(sizeZ > 0);
    ASSERT(numLayers > 0);

    // Preconditions: verify locked area is in range
    ASSERT(sizeX <= mWidth);
    ASSERT(sizeY <= mHeight);
    ASSERT(sizeZ <= mDepth);
    ASSERT(numLayers <= mNumLayers);
    ASSERT(offsetX + sizeX <= mWidth);
    ASSERT(offsetY + sizeY <= mHeight);
    ASSERT(offsetZ + sizeZ <= mDepth);
    ASSERT(startLayer + numLayers <= mNumLayers);

    // Preconditions: verify locked area is aligned for the block size of the texture format
    #if ASSERTS_ENABLED
    {
        uint32_t texBlockSizeX = 1;
        uint32_t texBlockSizeY = 1;
        uint32_t texBlockSizeZ = 1;
        VkFormatUtils::getMinBlockSizeForFormat(mFormat, texBlockSizeX, texBlockSizeY, texBlockSizeZ);

        ASSERT(offsetX % texBlockSizeX == 0);
        ASSERT(offsetY % texBlockSizeY == 0);
        ASSERT(offsetZ % texBlockSizeZ == 0);
        ASSERT(sizeX % texBlockSizeX == 0);
        ASSERT(sizeY % texBlockSizeY == 0);
        ASSERT(sizeZ % texBlockSizeZ == 0);
    }
    #endif  // #if ASSERTS_ENABLED

    // Get the size of the region to lock
    mLockedSizeInBytes = TextureUtils::getTotalTextureByteSize(
        mFormat,
        sizeX,
        sizeY,
        sizeZ,
        numLayers,
        mNumMipLevels,
        mbIsCubemap,
        Defines::MIN_IMAGE_ALIGNMENT
    );

    // Create a temporary staging buffer via the transfer manager to handle the transfer.
    // Note: for shared memory architectures (consoles, phones) this step could be skipped, we might just need to do layout transitions in that case.
    TransferMgr& transferMgr = mpDevice->getTransferMgr();
    const TransferMgr::StagingBuffer buffer = transferMgr.allocTempStagingBuffer(mLockedSizeInBytes);

    if (!buffer.pBytes) {
        ASSERT_FAIL("Failed to allocate a temporary staging buffer for a transfer!");
        mLockedSizeInBytes = 0;
        return nullptr;
    }

    // All good, save the details of the buffer, locked region and return the locked memory
    mLockedVkStagingBuffer = buffer.vkBuffer;
    mpLockedBytes = buffer.pBytes;
    mLockedOffsetX = offsetX;
    mLockedOffsetY = offsetY;
    mLockedOffsetZ = offsetZ;
    mLockedStartLayer = startLayer;
    mLockedSizeX = sizeX;
    mLockedSizeY = sizeY;
    mLockedSizeZ = sizeZ;
    mLockedNumLayers = numLayers;

    return buffer.pBytes;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlock previously locked memory and schedule an upload of the data to texture memory.
//
// Optionally the upload can be scheduled to happen against the specified transfer task.
// If not specified, then the global 'pre-frame' transfer task is used by default.
//------------------------------------------------------------------------------------------------------------------------------------------
void Texture::unlock(TransferTask* const pTransferTaskOverride) noexcept {
    // Preconditions
    ASSERT(mbIsValid);
    ASSERT(mpDevice && mpDevice->getVkDevice());
    ASSERT_LOG(isLocked(), "Buffer must first be locked to unlock!");

    // Determine the old texture image layout.
    // If we have done an upload previously then it's shader read only optimal, otherwise it's undefined.
    const VkImageLayout oldVkImageLayout = (mbDidATextureUpload) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

    // Schedule the data transfer for the image and image layout transitions
    TransferTask* pDstTask;

    if (!pTransferTaskOverride) {
        pDstTask = &mpDevice->getTransferMgr().getPreFrameTransferTask();
    } else {
        pDstTask = pTransferTaskOverride;
    }

    pDstTask->addTextureUpload(
        mLockedVkStagingBuffer,
        *this,
        oldVkImageLayout,
        0,
        mLockedOffsetX,
        mLockedOffsetY,
        mLockedOffsetZ,
        mLockedStartLayer,
        mLockedSizeX,
        mLockedSizeY,
        mLockedSizeZ,
        mLockedNumLayers
    );

    // Clear the lock details
    mLockedVkStagingBuffer = VK_NULL_HANDLE;
    mpLockedBytes = nullptr;
    mLockedSizeInBytes = 0;
    mLockedOffsetX = 0;
    mLockedOffsetY = 0;
    mLockedOffsetZ = 0;
    mLockedStartLayer = 0;
    mLockedSizeX = 0;
    mLockedSizeY = 0;
    mLockedSizeZ = 0;
    mLockedNumLayers = 0;

    // We now did a texture upload
    mbDidATextureUpload = true;
}

END_NAMESPACE(vgl)
