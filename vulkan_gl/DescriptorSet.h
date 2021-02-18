#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class BaseTexture;
class Buffer;
class DescriptorPool;
class Sampler;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan descriptor set. A descriptor set defines the inputs into a shader.
// Note that descriptor sets must be initialized indirectly via a pool.
//------------------------------------------------------------------------------------------------------------------------------------------
class DescriptorSet {
public:
    DescriptorSet() noexcept;
    DescriptorSet(DescriptorSet&& other) noexcept;
    ~DescriptorSet() noexcept;

    bool isValid() const noexcept;

    inline DescriptorPool* getParentPool() const noexcept { return mpParentPool; }
    inline VkDescriptorSet getVkDescriptorSet() const noexcept { return mVkDescriptorSet; }
    
    void free(const bool bImmediately = false) noexcept;
    
    void bindBufferBytes(
        const uint32_t bindingNum,
        const Buffer& buffer,
        const VkDescriptorType vkDescriptorType,
        const uint64_t bufferOffsetInBytes,
        const uint64_t bufferRangeInBytes
    ) noexcept;

    // Convenience helper: bind the buffer in terms of elements, not bytes
    template <class T>
    inline void bindBufferElements(
        const uint32_t bindingNum,
        const Buffer& buffer,
        const VkDescriptorType vkDescriptorType,
        const uint64_t bufferStartElement,
        const uint64_t bufferNumElements
    ) noexcept {
        bindBufferBytes(bindingNum, buffer, vkDescriptorType, bufferStartElement * sizeof(T), bufferNumElements * sizeof(T));
    }

    void bindTextureAndSampler(
        const uint32_t bindingNum,
        const BaseTexture& texture,
        const Sampler& sampler
    ) noexcept;

    void bindTextures(const uint32_t bindingNum, const VkDescriptorImageInfo* const pImageInfos, const uint32_t numImages) noexcept;
    void bindInputAttachment(const uint32_t bindingNum, const BaseTexture& attachTex) noexcept;

private:
    // Copy and move assign are disallowed
    DescriptorSet(const DescriptorSet& other) = delete;
    DescriptorSet& operator = (const DescriptorSet& other) = delete;
    DescriptorSet& operator = (DescriptorSet&& other) = delete;
    
    // Descriptor pool and retirement manager get intrusive access
    friend class DescriptorPool;
    friend class RetirementMgr;

    void init(DescriptorPool& parentPool, const VkDescriptorSet vkDescriptorSet) noexcept;  
    void release() noexcept;
    
    DescriptorPool*     mpParentPool;
    VkDescriptorSet     mVkDescriptorSet;
};

END_NAMESPACE(vgl)
