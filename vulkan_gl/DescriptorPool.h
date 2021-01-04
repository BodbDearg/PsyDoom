#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>
#include <vector>

BEGIN_NAMESPACE(vgl)

class DescriptorSet;
class DescriptorSetLayout;
class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan descriptor pool; allows descriptor sets to be allocated from the pool
//------------------------------------------------------------------------------------------------------------------------------------------
class DescriptorPool {
public:
    DescriptorPool() noexcept;
    DescriptorPool(DescriptorPool&& other) noexcept;
    ~DescriptorPool() noexcept;
    
    bool init(
        LogicalDevice& device,
        const std::vector<VkDescriptorPoolSize>& descriptorPoolSizes,
        const uint32_t maxSetAllocs
    ) noexcept;

    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;
    
    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    
    uint32_t getMaxSetAllocs() const noexcept;
    uint32_t getNumAllocedSets() const noexcept;
    uint32_t getMaxSetsFree() const noexcept;

    DescriptorSet* allocDescriptorSet(const DescriptorSetLayout& layout) noexcept;
    void freeDescriptorSet(DescriptorSet& descriptorSet) noexcept;
    
private:
    // Copy and move assign are disallowed
    DescriptorPool(const DescriptorPool& other) = delete;
    DescriptorPool& operator = (const DescriptorPool& other) = delete;
    DescriptorPool& operator = (DescriptorPool&& other) = delete;
    
    // Retirement manager has intrusive access (for modifying retirement state)
    friend class RetirementMgr;
    
    bool                        mbIsValid;          // True if the pool has been validly initialized/created
    LogicalDevice*              mpDevice;           // The device the pool belongs to
    VkDescriptorPool            mVkPool;            // The Vulkan descriptor pool object
    std::vector<DescriptorSet>  mDescriptorSets;    // The vector of allocated descriptor sets
    std::vector<uint32_t>       mFreeSetIndexes;    // Indices of which descriptor sets are free
};

END_NAMESPACE(vgl)
