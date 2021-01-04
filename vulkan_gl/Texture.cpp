#include "Texture.h"

#include "LogicalDevice.h"
#include "RawBuffer.h"
#include "RetirementMgr.h"
#include "TransferMgr.h"
#include "TransferTask.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized texture
//------------------------------------------------------------------------------------------------------------------------------------------
Texture::Texture() noexcept
    : BaseTexture()
    , mLockedVkStagingBuffer(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a texture to this object
//------------------------------------------------------------------------------------------------------------------------------------------
Texture::Texture(Texture&& other) noexcept
    : BaseTexture(std::move(other))
    , mLockedVkStagingBuffer(other.mLockedVkStagingBuffer)
{
    other.mLockedVkStagingBuffer = VK_NULL_HANDLE;
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
    ASSERT_LOG(!getIsLocked(), "Destroying an image that has not been unlocked!");

    // Gradual 'retirement' logic if specified and possible
    if ((!bImmediately) && mbIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Cleanup logic: this class has no real logic of it's own so defers to base
    BaseTexture::destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells whether the texture has been locked for image transfer or not
//------------------------------------------------------------------------------------------------------------------------------------------
bool Texture::getIsLocked() const noexcept {
    return (mLockedVkStagingBuffer != VK_NULL_HANDLE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Locks the data for the entire texture and returns a pointer that can be used to populate the new contents of the texture.
//
// Notes:
//  (1) If locking the requested region fails then a null pointer is returned
//  (2) The memory given is completely undefined upon locking, and may not contain any previous data
//  (3) The texture must be unlocked in order for it's contents to be uploaded to GPU memory
//  (4) Unlock must happen before the draw is commenced, undefined behavior or crashes may result otherwise
//  (5) Each mipmap level in the data MUST start on 4-byte (32-bit) boundaries.
//      Data must be padded between mipmap levels in order to satisfy this requirement.
//------------------------------------------------------------------------------------------------------------------------------------------
std::byte* Texture::lock() noexcept {
    // Preconditions: must be valid and device must still be valid
    ASSERT(mbIsValid);
    ASSERT(mpDevice && mpDevice->getVkDevice());

    // Preconditions: can't double lock
    ASSERT_LOG(!getIsLocked(), "Can't lock when already locked!");

    // Create a temporary staging buffer via the transfer manager to handle the transfer.
    // Note: for shared memory architectures (consoles, phones) this step could be skipped, we might just need to do layout transitions in that case.
    TransferMgr& transferMgr = mpDevice->getTransferMgr();
    const TransferMgr::StagingBuffer buffer = transferMgr.allocTempStagingBuffer(mSizeInBytes);

    if (!buffer.pBytes) {
        ASSERT_FAIL("Failed to allocate a temporary staging buffer for a transfer!");
        return nullptr;
    }
    
    // All good, save the details of the buffer and return the locked memory
    mLockedVkStagingBuffer = buffer.vkBuffer;
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
    ASSERT_LOG(getIsLocked(), "Buffer must first be locked to unlock!");

    // Schedule the data transfer for the image and image layout transitions
    TransferTask* pDstTask;

    if (!pTransferTaskOverride) {
        pDstTask = &mpDevice->getTransferMgr().getPreFrameTransferTask();
    } else {
        pDstTask = pTransferTaskOverride;
    }

    pDstTask->addTextureUpload(mLockedVkStagingBuffer, *this);

    // Clear the lock details
    mLockedVkStagingBuffer = VK_NULL_HANDLE;
}

END_NAMESPACE(vgl)
