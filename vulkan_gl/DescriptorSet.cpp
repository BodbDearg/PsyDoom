#include "DescriptorSet.h"

#include "Asserts.h"
#include "BaseTexture.h"
#include "Buffer.h"
#include "DescriptorPool.h"
#include "LogicalDevice.h"
#include "RetirementMgr.h"
#include "Sampler.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized descriptor set
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorSet::DescriptorSet() noexcept
    : mpParentPool(nullptr)
    , mVkDescriptorSet(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a descriptor set to this object
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
    : mpParentPool(other.mpParentPool)
    , mVkDescriptorSet(other.mVkDescriptorSet)
{
    other.mpParentPool = nullptr;
    other.mVkDescriptorSet = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the descriptor set
//------------------------------------------------------------------------------------------------------------------------------------------
DescriptorSet::~DescriptorSet() noexcept {
    ASSERT_LOG((mVkDescriptorSet == VK_NULL_HANDLE), "Destroyed without being released!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if this set is valid/initialized
//------------------------------------------------------------------------------------------------------------------------------------------
bool DescriptorSet::isValid() const noexcept {
    return (mVkDescriptorSet != VK_NULL_HANDLE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Causes the descriptor set to be freed.
// Note that this must ONLY be called on a valid descriptor set!
// Optionally you can choose whether the set is immediately destroyed or whether it is retired gradually.
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorSet::free(const bool bImmediately) noexcept {
    ASSERT(isValid());
    ASSERT(mpParentPool && mpParentPool->isValid());
    ASSERT(mpParentPool->getDevice());
    ASSERT(mpParentPool->getDevice()->getRetirementMgr().isValid());
    
    if (bImmediately) {
        mpParentPool->freeDescriptorSet(*this);
    } else {
        RetirementMgr& retirementMgr = mpParentPool->getDevice()->getRetirementMgr();
        retirementMgr.retire(*this);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bind the specified binding number in the descriptor set to the given range of buffer bytes.
// Note that the buffer must be of the correct type for the descriptor slot it is being bound to.
// For example, a uniform buffer must be assigned to a uniform buffer slot, a storage buffer to a storage buffer slot etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorSet::bindBufferBytes(
    const uint32_t bindingNum,
    const Buffer& buffer,
    const VkDescriptorType vkDescriptorType,
    const uint64_t bufferOffsetInBytes,
    const uint64_t bufferRangeInBytes
) noexcept {
    // Preconditions
    LogicalDevice& device = *mpParentPool->getDevice();

    ASSERT(isValid());
    ASSERT(buffer.isValid());
    ASSERT(device.getVkDevice());
    ASSERT(bufferOffsetInBytes < buffer.getSizeInBytes());
    ASSERT(bufferOffsetInBytes + bufferRangeInBytes <= buffer.getSizeInBytes());
    
    // Fill in the buffer info struct
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer.getVkBuffer();
    bufferInfo.offset = bufferOffsetInBytes;
    bufferInfo.range = bufferRangeInBytes;

    // Fill in a descriptor write struct
    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = mVkDescriptorSet;
    writeInfo.dstBinding = bindingNum;
    writeInfo.descriptorType = vkDescriptorType;
    writeInfo.descriptorCount = 1;
    writeInfo.pBufferInfo = &bufferInfo;

    // Do the descriptor update
    const VkFuncs& vkFuncs = device.getVkFuncs();
    vkFuncs.vkUpdateDescriptorSets(device.getVkDevice(), 1, &writeInfo, 0, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bind the specified binding number in the descriptor set to the given texture and sampler.
// This corresponds to a 'combined image sampler' descriptor type in Vulkan terms.
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorSet::bindTextureAndSampler(
    const uint32_t bindingNum,
    const BaseTexture& texture,
    const Sampler& sampler
) noexcept {
    // Preconditions
    ASSERT(isValid());
    ASSERT(texture.isValid());
    ASSERT(sampler.isValid());

    // Fill in the image info struct
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = sampler.getVkSampler();
    imageInfo.imageView = texture.getVkImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;   // For now assume all textures will be in this layout before they are used

    // Fill in a descriptor write struct
    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = mVkDescriptorSet;
    writeInfo.dstBinding = bindingNum;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.descriptorCount = 1;
    writeInfo.pImageInfo = &imageInfo;

    // Do the descriptor update
    LogicalDevice& device = *mpParentPool->getDevice();
    const VkFuncs& vkFuncs = device.getVkFuncs();
    vkFuncs.vkUpdateDescriptorSets(device.getVkDevice(), 1, &writeInfo, 0, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bind the specified binding number in the descriptor set to the given input attachment, specified by it's texture.
// This corresponds to a 'input attachment' descriptor type in Vulkan terms.
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorSet::bindInputAttachment(const uint32_t bindingNum, const BaseTexture& attachTex) noexcept {
    // Preconditions
    ASSERT(isValid());
    ASSERT(attachTex.isValid());

    // Fill in the image info struct
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = attachTex.getVkImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;   // For now assume all textures will be in this layout before they are used

    // Fill in a descriptor write struct
    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = mVkDescriptorSet;
    writeInfo.dstBinding = bindingNum;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    writeInfo.descriptorCount = 1;
    writeInfo.pImageInfo = &imageInfo;

    // Do the descriptor update
    LogicalDevice& device = *mpParentPool->getDevice();
    const VkFuncs& vkFuncs = device.getVkFuncs();
    vkFuncs.vkUpdateDescriptorSets(device.getVkDevice(), 1, &writeInfo, 0, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the descriptor set: this is invoked by the pool
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorSet::init(DescriptorPool& parentPool, const VkDescriptorSet vkDescriptorSet) noexcept {
    mpParentPool = &parentPool;
    mVkDescriptorSet = vkDescriptorSet;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Causes the descriptor set to release ownership of the underlying Vulkan descriptor set and forget about it's parent pool.
// Essentially like a destroy() but without the resource cleanup.
// Note: this should only be called by the descriptor pool! It's used when the pool wants to do a batch cleanup of descriptor sets.
//------------------------------------------------------------------------------------------------------------------------------------------
void DescriptorSet::release() noexcept {
    mpParentPool = nullptr;
    mVkDescriptorSet = VK_NULL_HANDLE;
}

END_NAMESPACE(vgl)
