#include "TransferTask.h"

#include "Buffer.h"
#include "CmdBuffer.h"
#include "CmdPool.h"
#include "Defines.h"
#include "LogicalDevice.h"
#include "MutableTexture.h"
#include "RenderTexture.h"
#include "Texture.h"
#include "TextureUtils.h"
#include "Utils.h"
#include "VkFormatUtils.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

// What type of transfer command we are dealing with
enum class TransferCmdType {
    BUFFER_TO_BUFFER_TRANSFER,
    BUFFER_TO_TEXTURE_TRANSFER,
    RENDER_TEXTURE_DOWNLOAD
};

// A buffer to buffer transfer command
struct BufToBufTransCmd {
    VkBuffer srcVkBuffer;
    VkBuffer dstVkBuffer;
    uint64_t srcOffset;
    uint64_t dstOffset;
    uint64_t numBytes;
};

// A buffer to texture transfer command
struct BufToTexTransCmd {
    VkBuffer        srcVkBuffer;
    VkImage         dstVkImage;
    VkImageLayout   dstOldVkImageLayout;
    VkFormat        texFormat;
    uint64_t        srcBufferOffset;
    uint32_t        dstOffsetX;
    uint32_t        dstOffsetY;
    uint32_t        dstOffsetZ;
    uint32_t        dstStartLayer;
    uint32_t        dstSizeX;
    uint32_t        dstSizeY;
    uint32_t        dstSizeZ;
    uint32_t        dstNumLayers;
    uint32_t        dstNumMipLevels;
    bool            bTexIsCubemap;
};

// A render texture download command
struct RenderTexDownloadCmd {
    VkImage     srcVkImage;
    VkImage     dstVkImage;
    uint32_t    texWidth;
    uint32_t    texHeight;
    uint32_t    texDepth;
    uint32_t    texNumLayers;
    VkFormat    texFormat;
    uint32_t    texNumMipLevels;
    bool        bTexIsCubemap;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Structure for an individual transfer command.
// The contents are interpreted differently depending on the command type.
//------------------------------------------------------------------------------------------------------------------------------------------
struct TransferTask::TransferCmd {
    TransferCmdType type;

    union {
        BufToBufTransCmd        bufToBufTransCmd;
        BufToTexTransCmd        bufToTexTransCmd;
        RenderTexDownloadCmd    renderTexDownloadCmd;
    };
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a buffer to buffer transfer command into the given command buffer
//------------------------------------------------------------------------------------------------------------------------------------------
static void submitToCmdBufferImpl(CmdBuffer& cmdBuffer, const BufToBufTransCmd& cmd) noexcept {
    VkBufferCopy copyInfo = {
        cmd.srcOffset,
        cmd.dstOffset,
        cmd.numBytes
    };

    const VkFuncs& vkFuncs = cmdBuffer.getCmdPool()->getDevice()->getVkFuncs();
    vkFuncs.vkCmdCopyBuffer(cmdBuffer.getVkCommandBuffer(), cmd.srcVkBuffer, cmd.dstVkBuffer, 1, &copyInfo);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a buffer to texture transfer command into the given command buffer
//------------------------------------------------------------------------------------------------------------------------------------------
static void submitToCmdBufferImpl(CmdBuffer& cmdBuffer, const BufToTexTransCmd& cmd) noexcept {
    // Buffer offset for command should be 32-bit aligned!
    ASSERT(cmd.srcBufferOffset % Defines::MIN_IMAGE_ALIGNMENT == 0);

    // Get the queue that we use for submitting work in general to graphics device
    LogicalDevice& device = *cmdBuffer.getCmdPool()->getDevice();
    const uint32_t workQueueFamilyIdx = device.getWorkQueueFamilyIdx();

    // Need to insert an image barrier to get the image in a format that is optimal as a transfer destination:
    const VkCommandBuffer vkCmdBuffer = cmdBuffer.getVkCommandBuffer();
    const VkFuncs& vkFuncs = device.getVkFuncs();

    const uint32_t startTexImage = TextureUtils::getNumTexImages(cmd.dstStartLayer, cmd.bTexIsCubemap);
    const uint32_t numTexImages = TextureUtils::getNumTexImages(cmd.dstNumLayers, cmd.bTexIsCubemap);

    {
        VkImageMemoryBarrier barrier = {};

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;     // Wait for other access to finish
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;     // Reads and writes are blocked on waiting for the other transfers to finish
        barrier.oldLayout = cmd.dstOldVkImageLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;           // Make the image be optimal as a transfer destination
        barrier.srcQueueFamilyIndex = workQueueFamilyIdx;
        barrier.dstQueueFamilyIndex = workQueueFamilyIdx;
        barrier.image = cmd.dstVkImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;    // Only dealing with color buffers and not depth
        barrier.subresourceRange.baseMipLevel = 0;                          // Include all mip levels
        barrier.subresourceRange.levelCount = cmd.dstNumMipLevels;          // Include all mip levels
        barrier.subresourceRange.baseArrayLayer = startTexImage;
        barrier.subresourceRange.layerCount = numTexImages;

        vkFuncs.vkCmdPipelineBarrier(
            vkCmdBuffer,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,     // Src pipeline stage mask: wait for other stages to finish accessing
            VK_PIPELINE_STAGE_TRANSFER_BIT,         // Dst pipeline stage mask: transfers waiting on transfers
            0,                                      // Dependency flags
            0,                                      // Memory barrier count
            nullptr,                                // Memory barriers
            0,                                      // Buffer memory barrier count
            nullptr,                                // Buffer memory barriers
            1,                                      // Image memory barrier count
            &barrier                                // Image memory barrier
        );
    }

    // Next record the actual copy operations that will copy the data into the image.
    {
        // Use this buffer to submit the copy operations
        // TODO: preallocate this temporary buffer once and re-use
        uint64_t bufferOffset = cmd.srcBufferOffset;
        std::vector<VkBufferImageCopy> bufferImageCopyCmds;
        bufferImageCopyCmds.reserve(16);

        // Process the copy operations for every image and mipmap level in the texture
        for (uint32_t curImage = 0; curImage < numTexImages; ++curImage) {
            for (uint32_t curMipLevel = 0; curMipLevel < cmd.dstNumMipLevels; ++curMipLevel) {
                // Figure out the dimensions for this mip level.
                // Note that we do NOT round to the nearest block size for the copy command!
                uint32_t mipLevelWidth = 0;
                uint32_t mipLevelHeight = 0;
                uint32_t mipLevelDepth = 0;

                TextureUtils::getMipLevelDimensions(
                    cmd.dstSizeX,
                    cmd.dstSizeY,
                    cmd.dstSizeZ,
                    curMipLevel,
                    mipLevelWidth,
                    mipLevelHeight,
                    mipLevelDepth
                );

                ASSERT((mipLevelWidth > 0) && (mipLevelHeight > 0) && (mipLevelDepth > 0));

                // Figure out the byte size for this mip level.
                // Note that this WILL round to the nearest block size!
                const uint64_t mipLevelByteSize = TextureUtils::getMipLevelByteSize(
                    cmd.texFormat,
                    mipLevelWidth,
                    mipLevelHeight,
                    mipLevelDepth
                );

                // Get the destination offset within this mipmap level
                const uint32_t mipOffsetX = (cmd.dstOffsetX >> curMipLevel);
                const uint32_t mipOffsetY = (cmd.dstOffsetY >> curMipLevel);
                const uint32_t mipOffsetZ = (cmd.dstOffsetZ >> curMipLevel);

                // Schedule the copy operation for this mip level
                VkBufferImageCopy copyOp = {};
                copyOp.bufferOffset = bufferOffset;
                copyOp.bufferRowLength = 0;                                         // Tightly packed
                copyOp.bufferImageHeight = 0;                                       // Tightly packed
                copyOp.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;     // Just dealing with color buffer
                copyOp.imageSubresource.mipLevel = curMipLevel;
                copyOp.imageSubresource.baseArrayLayer = startTexImage + curImage;
                copyOp.imageSubresource.layerCount = 1;
                copyOp.imageOffset = { (int32_t) mipOffsetX, (int32_t) mipOffsetY, (int32_t) mipOffsetZ };
                copyOp.imageExtent = { mipLevelWidth, mipLevelHeight, mipLevelDepth };

                bufferImageCopyCmds.push_back(copyOp);

                // Move onto the next mip level in the buffer.
                // Note that as per the docs in 'Texture' this offset must be 32-bit (4 byte) aligned so if it's not aligned then align it now.
                bufferOffset += mipLevelByteSize;
                bufferOffset = Utils::ualignUp(bufferOffset, (uint64_t) Defines::MIN_IMAGE_ALIGNMENT);
            }
        }

        // Issue the buffer image copy commands
        vkFuncs.vkCmdCopyBufferToImage(
            vkCmdBuffer,
            cmd.srcVkBuffer,
            cmd.dstVkImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,                       // Dest image is in this format
            static_cast<uint32_t>(bufferImageCopyCmds.size()),
            bufferImageCopyCmds.data()
        );
    }

    // Need to insert an image barrier to get the image into a format that is optimal for use in shaders:
    {
        VkImageMemoryBarrier barrier = {};

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;   // Waiting on transfer reads and writes to finish
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;     // All types of reads and writes are blocked waiting for the writes
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;                           // The old layout was transfer optimal
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;                       // The new layout will be shader use optimal
        barrier.srcQueueFamilyIndex = workQueueFamilyIdx;
        barrier.dstQueueFamilyIndex = workQueueFamilyIdx;
        barrier.image = cmd.dstVkImage;
        barrier.subresourceRange.aspectMask = VkFormatUtils::getVkImageAspectFlags(cmd.texFormat);
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = cmd.dstNumMipLevels;              // Include all mip levels
        barrier.subresourceRange.baseArrayLayer = startTexImage;
        barrier.subresourceRange.layerCount = numTexImages;

        vkFuncs.vkCmdPipelineBarrier(
            vkCmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,         // Wait for the transfer stage to finish executing
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,     // All stages are blocked waiting for the transfer to finish
            0,                                      // Dependency flags
            0,                                      // Memory barrier count
            nullptr,                                // Memory barriers
            0,                                      // Buffer memory barrier count
            nullptr,                                // Buffer memory barriers
            1,                                      // Image memory barrier count
            &barrier                                // Image memory barrier
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a render texture download command into the given command buffer
//------------------------------------------------------------------------------------------------------------------------------------------
static void submitToCmdBufferImpl(CmdBuffer& cmdBuffer, const RenderTexDownloadCmd& cmd) noexcept {
    // Some useful stuff; the work queue & command buffer we will submit to, Vulkan API functions and the number of images in the texture:
    LogicalDevice& device = *cmdBuffer.getCmdPool()->getDevice();
    const VkFuncs& vkFuncs = device.getVkFuncs();

    const uint32_t workQueueFamilyIdx = device.getWorkQueueFamilyIdx();
    const VkCommandBuffer vkCmdBuffer = cmdBuffer.getVkCommandBuffer();
    const uint32_t numTexImages = TextureUtils::getNumTexImages(cmd.texNumLayers, cmd.bTexIsCubemap);

    // Figure out the image aspect flags
    const VkImageAspectFlags vkImageAspectFlags = (
        (VkFormatUtils::hasColorAspect(cmd.texFormat) ? VK_IMAGE_ASPECT_COLOR_BIT : 0) |
        (VkFormatUtils::hasDepthAspect(cmd.texFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0) |
        (VkFormatUtils::hasStencilAspect(cmd.texFormat) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0)
    );

    // Need to insert an image barrier to get the render target in a format that is optimal as a transfer destination:
    {
        VkImageMemoryBarrier barrier = {};

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;                                          // Waiting on top of pipe so no access mask
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;                // Transfer read blocked on layout change
        barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;       // Render target expected to be in shader read only optimal format when done rendering!
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;           // Make the image be optimal as a transfer source
        barrier.srcQueueFamilyIndex = workQueueFamilyIdx;
        barrier.dstQueueFamilyIndex = workQueueFamilyIdx;
        barrier.image = cmd.srcVkImage;
        barrier.subresourceRange.aspectMask = vkImageAspectFlags;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = cmd.texNumMipLevels;          // Include all mip levels
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = numTexImages;                 // Include all layers

        vkFuncs.vkCmdPipelineBarrier(
            vkCmdBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,      // Src pipeline stage mask
            VK_PIPELINE_STAGE_TRANSFER_BIT,         // Dst pipeline stage mask
            0,                                      // Dependency flags
            0,                                      // Memory barrier count
            nullptr,                                // Memory barriers
            0,                                      // Buffer memory barrier count
            nullptr,                                // Buffer memory barriers
            1,                                      // Image memory barrier count
            &barrier                                // Image memory barrier
        );
    }

    // Need to insert an image barrier to get the mutable texture in the general layout:
    {
        VkImageMemoryBarrier barrier = {};

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;                                          // Waiting on top of pipe so no access mask
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;               // Transfer write blocked on layout change
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // Don't care about previous layout
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;                        // Use the all purpose 'general' layout
        barrier.srcQueueFamilyIndex = workQueueFamilyIdx;
        barrier.dstQueueFamilyIndex = workQueueFamilyIdx;
        barrier.image = cmd.dstVkImage;
        barrier.subresourceRange.aspectMask = vkImageAspectFlags;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = cmd.texNumMipLevels;          // Include all mip levels
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = numTexImages;                 // Include all layers

        vkFuncs.vkCmdPipelineBarrier(
            vkCmdBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,      // Src pipeline stage mask
            VK_PIPELINE_STAGE_TRANSFER_BIT,         // Dst pipeline stage mask
            0,                                      // Dependency flags
            0,                                      // Memory barrier count
            nullptr,                                // Memory barriers
            0,                                      // Buffer memory barrier count
            nullptr,                                // Buffer memory barriers
            1,                                      // Image memory barrier count
            &barrier                                // Image memory barrier
        );
    }

    // Next record the actual copy operations that will copy the data into the image.
    {
        // Use this buffer to submit the copy operations
        // TODO: preallocate this temporary buffer once and re-use
        std::vector<VkImageCopy> imageCopyCmds;
        imageCopyCmds.reserve(16);

        // Process the copy operations for every image and mipmap level in the texture
        for (uint32_t curMipLevel = 0; curMipLevel < cmd.texNumMipLevels; ++curMipLevel) {
            // Figure out the dimensions for this mip level.
            // Note that we do NOT round to the nearest block size for the copy command!
            uint32_t mipLevelWidth = 0;
            uint32_t mipLevelHeight = 0;
            uint32_t mipLevelDepth = 0;

            TextureUtils::getMipLevelDimensions(
                cmd.texWidth,
                cmd.texHeight,
                cmd.texDepth,
                curMipLevel,
                mipLevelWidth,
                mipLevelHeight,
                mipLevelDepth
            );

            ASSERT((mipLevelWidth > 0) && (mipLevelHeight > 0) && (mipLevelDepth > 0));

            // Schedule the copy operation for this mip level
            VkImageCopy copyOp = {};
            copyOp.srcSubresource.aspectMask = vkImageAspectFlags;
            copyOp.srcSubresource.mipLevel = curMipLevel;
            copyOp.srcSubresource.baseArrayLayer = 0;
            copyOp.srcSubresource.layerCount = numTexImages;
            copyOp.srcOffset = { 0, 0, 0 };
            copyOp.dstSubresource.aspectMask = vkImageAspectFlags;
            copyOp.dstSubresource.mipLevel = curMipLevel;
            copyOp.dstSubresource.baseArrayLayer = 0;
            copyOp.dstSubresource.layerCount = numTexImages;
            copyOp.dstOffset = { 0, 0, 0 };
            copyOp.extent = { mipLevelWidth, mipLevelHeight, mipLevelDepth };

            imageCopyCmds.push_back(copyOp);
        }

        // Submit all the copy operations
        vkFuncs.vkCmdCopyImage(
            vkCmdBuffer,
            cmd.srcVkImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            cmd.dstVkImage,
            VK_IMAGE_LAYOUT_GENERAL,
            static_cast<uint32_t>(imageCopyCmds.size()),
            imageCopyCmds.data()
        );
    }

    // Transfer the source image back into normal layout that can be used for shader reads
    {
        VkImageMemoryBarrier barrier = {};

        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;                                          // Waiting on top of pipe so no access mask
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;                  // Shader read blocked on layout change
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;           // Expect render target to be in a readable state
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;       // Make the image be optimal as a transfer source
        barrier.srcQueueFamilyIndex = workQueueFamilyIdx;
        barrier.dstQueueFamilyIndex = workQueueFamilyIdx;
        barrier.image = cmd.srcVkImage;
        barrier.subresourceRange.aspectMask = vkImageAspectFlags;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = cmd.texNumMipLevels;          // Include all mip levels
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = numTexImages;                 // Include all layers

        vkFuncs.vkCmdPipelineBarrier(
            vkCmdBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,      // Src pipeline stage mask
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,    // Dst pipeline stage mask
            0,                                      // Dependency flags
            0,                                      // Memory barrier count
            nullptr,                                // Memory barriers
            0,                                      // Buffer memory barrier count
            nullptr,                                // Buffer memory barriers
            1,                                      // Image memory barrier count
            &barrier                                // Image memory barrier
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Default initializes the transfer task
//------------------------------------------------------------------------------------------------------------------------------------------
TransferTask::TransferTask() noexcept
    : mCmds(false)
{
}

TransferTask::~TransferTask() noexcept {
    // Just here so outside code doesn't need to know size of 'TransferCmd'...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reserve a number of commands of room in the internal command buffer
//------------------------------------------------------------------------------------------------------------------------------------------
void TransferTask::reserveCmds(const size_t numCmds) noexcept {
    mCmds.reserve(numCmds);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clear the list of commands and optionally relinquish command list memory
//------------------------------------------------------------------------------------------------------------------------------------------
void TransferTask::clearCmds(const bool bCompactCmdList) noexcept {
    mCmds.clear();

    if (bCompactCmdList) {
        mCmds.shrink_to_fit();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Query if there are any commands in the task
//------------------------------------------------------------------------------------------------------------------------------------------
bool TransferTask::isEmpty() const noexcept {
    return mCmds.empty();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current number of commands added to the transfer task
//------------------------------------------------------------------------------------------------------------------------------------------
size_t TransferTask::getNumCmds() const noexcept {
    return mCmds.size();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Records the transfer task into the given command buffer for execution.
// After the call the list of commands in the task is empty again.
//------------------------------------------------------------------------------------------------------------------------------------------
void TransferTask::submitToCmdBuffer(CmdBuffer& cmdBuffer) noexcept {
    ASSERT(cmdBuffer.isValid());

    for (const TransferCmd& cmd : mCmds) {
        switch (cmd.type) {
            case TransferCmdType::BUFFER_TO_BUFFER_TRANSFER:
                submitToCmdBufferImpl(cmdBuffer, cmd.bufToBufTransCmd);
                break;

            case TransferCmdType::BUFFER_TO_TEXTURE_TRANSFER:
                submitToCmdBufferImpl(cmdBuffer, cmd.bufToTexTransCmd);
                break;

            case TransferCmdType::RENDER_TEXTURE_DOWNLOAD:
                submitToCmdBufferImpl(cmdBuffer, cmd.renderTexDownloadCmd);
                break;

            default:
                ASSERT_FAIL("Unknown command type!");
                break;
        }
    }

    mCmds.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a command to schedule a transfer between two buffers.
// Note that the source and destination buffers are assumed valid as transfer source and destinations respectively.
//------------------------------------------------------------------------------------------------------------------------------------------
void TransferTask::addBufferCopy(
    const VkBuffer srcVkBuffer,
    const VkBuffer dstVkBuffer,
    const uint64_t srcOffset,
    const uint64_t dstOffset,
    const uint64_t numBytes
) noexcept {
    ASSERT(srcVkBuffer);
    ASSERT(dstVkBuffer);

    TransferCmd& cmd = mCmds.emplace_back();
    cmd.type = TransferCmdType::BUFFER_TO_BUFFER_TRANSFER;

    BufToBufTransCmd& cmdDetails = cmd.bufToBufTransCmd;
    cmdDetails.srcVkBuffer = srcVkBuffer;
    cmdDetails.dstVkBuffer = dstVkBuffer;
    cmdDetails.srcOffset = srcOffset;
    cmdDetails.dstOffset = dstOffset;
    cmdDetails.numBytes = numBytes;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedules a texture data upload to occur from the given buffer to the given texture.
// The specified region is uploaded, for all mipmap levels.
//
// Notes:
//  (1) The buffer is assumed to be sized large enough to accomodate all of the image data and valid as a transfer source.
//  (2) The image is put in a shader read optimal state after completion.
//------------------------------------------------------------------------------------------------------------------------------------------
void TransferTask::addTextureUpload(
    const VkBuffer srcVkBuffer,
    const Texture& dstTexture,
    const VkImageLayout dstOldVkImageLayout,
    const uint64_t srcBufferOffset,
    const uint32_t dstOffsetX,
    const uint32_t dstOffsetY,
    const uint32_t dstOffsetZ,
    const uint32_t dstStartLayer,
    const uint32_t dstSizeX,
    const uint32_t dstSizeY,
    const uint32_t dstSizeZ,
    const uint32_t dstNumLayers
) noexcept {
    ASSERT(srcVkBuffer);
    ASSERT(dstTexture.isValid());

    TransferCmd& cmd = mCmds.emplace_back();
    cmd.type = TransferCmdType::BUFFER_TO_TEXTURE_TRANSFER;

    BufToTexTransCmd& cmdDetails = cmd.bufToTexTransCmd;
    cmdDetails.srcVkBuffer = srcVkBuffer;
    cmdDetails.dstVkImage = dstTexture.getVkImage();
    cmdDetails.dstOldVkImageLayout = dstOldVkImageLayout;
    cmdDetails.texFormat = dstTexture.getFormat();
    cmdDetails.srcBufferOffset = srcBufferOffset;
    cmdDetails.dstOffsetX = dstOffsetX;
    cmdDetails.dstOffsetY = dstOffsetY;
    cmdDetails.dstOffsetZ = dstOffsetZ;
    cmdDetails.dstStartLayer = dstStartLayer;
    cmdDetails.dstSizeX = dstSizeX;
    cmdDetails.dstSizeY = dstSizeY;
    cmdDetails.dstSizeZ = dstSizeZ;
    cmdDetails.dstNumLayers = dstNumLayers;
    cmdDetails.dstNumMipLevels = dstTexture.getNumMipLevels();
    cmdDetails.bTexIsCubemap = dstTexture.isCubemap();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedules the contents of a render texture to be transferred into the given mutable texture.
// The entire texture data is transferred, including all the mipmap levels.
//
// Notes: 
//  (1) The target texture format, size etc. must match exactly the render texture.
//  (2) The render target must not be multisampled since mutable textures do not support multi sampling.
//------------------------------------------------------------------------------------------------------------------------------------------
void TransferTask::addRenderTextureDownload(RenderTexture& src, MutableTexture& dst) noexcept {
    // Preconditions: both source and destination must be valid
    ASSERT(src.isValid());
    ASSERT(dst.isValid());

    // Preconditions: render texture cannot be multi sampled
    ASSERT(src.getNumSamples() == 1);

    // Preconditions: source and destination textures must be identical
    ASSERT(src.getFormat() == dst.getFormat());
    ASSERT(src.getWidth() == dst.getWidth());
    ASSERT(src.getHeight() == dst.getHeight());
    ASSERT(src.getDepth() == dst.getDepth());
    ASSERT(src.getNumLayers() == dst.getNumLayers());
    ASSERT(src.getNumMipLevels() == dst.getNumMipLevels());
    ASSERT(src.isCubemap() == dst.isCubemap());

    // Schedule the transfer
    TransferCmd& cmd = mCmds.emplace_back();
    cmd.type = TransferCmdType::RENDER_TEXTURE_DOWNLOAD;

    RenderTexDownloadCmd& cmdDetails = cmd.renderTexDownloadCmd;
    cmdDetails.srcVkImage = src.getVkImage();
    cmdDetails.dstVkImage = dst.getVkImage();
    cmdDetails.texWidth = src.getWidth();
    cmdDetails.texHeight = src.getHeight();
    cmdDetails.texDepth = src.getDepth();
    cmdDetails.texNumLayers = src.getNumLayers();
    cmdDetails.texFormat = src.getFormat();
    cmdDetails.texNumMipLevels = src.getNumMipLevels();
    cmdDetails.bTexIsCubemap = src.isCubemap();
}

END_NAMESPACE(vgl)
