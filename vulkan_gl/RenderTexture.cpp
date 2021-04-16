#include "RenderTexture.h"

#include "Asserts.h"
#include "DeviceMemMgr.h"
#include "LogicalDevice.h"
#include "MutableTexture.h"
#include "RetirementMgr.h"
#include "TransferTask.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized render texture
//------------------------------------------------------------------------------------------------------------------------------------------
RenderTexture::RenderTexture() noexcept
    : BaseTexture()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a render texture to this object
//------------------------------------------------------------------------------------------------------------------------------------------
RenderTexture::RenderTexture(RenderTexture&& other) noexcept
    : BaseTexture(std::move(other))
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the render texture
//------------------------------------------------------------------------------------------------------------------------------------------
RenderTexture::~RenderTexture() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a renderable texture that is a normal 2d texture.
// By default the texture is given the 'VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT' usage flag.
//------------------------------------------------------------------------------------------------------------------------------------------
bool RenderTexture::initAsRenderTexture(
    LogicalDevice& device,
    const bool bUseUnpooledAlloc,
    const VkFormat textureFormat,
    const VkImageUsageFlags usageFlags,
    const uint32_t width,
    const uint32_t height,
    const uint32_t sampleCount,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_2D,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | usageFlags,
        VK_IMAGE_LAYOUT_UNDEFINED,
        (bUseUnpooledAlloc) ? DeviceMemAllocMode::UNPOOLED_PREFER_DEVICE_LOCAL : DeviceMemAllocMode::PREFER_DEVICE_LOCAL,
        textureFormat,
        width,
        height,
        1,              // Depth
        1,              // Num layers
        1,              // Num mip levels
        sampleCount,
        false,          // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a renderable texture that is a depth stencil buffer.
// By default the texture is given the 'VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT' usage flag.
//------------------------------------------------------------------------------------------------------------------------------------------
bool RenderTexture::initAsDepthStencilBuffer(
    LogicalDevice& device,
    const bool bUseUnpooledAlloc,
    const VkFormat textureFormat,
    const VkImageUsageFlags usageFlags,
    const uint32_t width,
    const uint32_t height,
    const uint32_t sampleCount
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_2D,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | usageFlags,
        VK_IMAGE_LAYOUT_UNDEFINED,
        (bUseUnpooledAlloc) ?  DeviceMemAllocMode::UNPOOLED_PREFER_DEVICE_LOCAL : DeviceMemAllocMode::PREFER_DEVICE_LOCAL,
        textureFormat,
        width,
        height,
        1,              // Depth
        1,              // Num layers
        1,              // Num mip levels
        sampleCount,    
        false,          // Is cubemap?
        AlphaMode::UNSPECIFIED
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the render texture and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void RenderTexture::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Gradual 'retirement' logic if specified and possible
    if ((!bImmediately) && mbIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Cleanup logic: this class has no real logic of it's own so defers to base
    BaseTexture::destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedule a transfer of the render texture's contents to the given mutable texture as part  of the given transfer task.
//
// Notes:
//  (1) The size, parameters and format of the the two textures MUST match EXACTLY.
//  (2) It is up to external code to ensure that the transfer is properly synchronized and  will not occur at a time when the render
//      texture might potentially be updating. No guarantees about synchronization are made by this call.
//------------------------------------------------------------------------------------------------------------------------------------------
void RenderTexture::scheduleCopyTo(MutableTexture& mutableTex, TransferTask& transferTask) noexcept {
    // Note: transfer task will verify formats match so don't verify here
    ASSERT(mbIsValid);
    ASSERT(mutableTex.isValid());
    transferTask.addRenderTextureDownload(*this, mutableTex);
}

END_NAMESPACE(vgl)
