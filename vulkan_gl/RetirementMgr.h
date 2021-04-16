#pragma once

#include "Asserts.h"
#include "Defines.h"

#include <set>
#include <vector>

BEGIN_NAMESPACE(vgl)

class CmdBuffer;
class DescriptorPool;
class DescriptorSet;
class DescriptorSetLayout;
class Framebuffer;
class IRetirementProvider;
class LogicalDevice;
class MutableTexture;
class Pipeline;
class PipelineLayout;
class RawBuffer;
class RenderTexture;
class RingbufferMgr;
class ShaderModule;
class Texture;

//------------------------------------------------------------------------------------------------------------------------------------------
// Manages the process of 'retiring' resources like buffers, textures etc. after they have been destroyed.
//
// Because resources may be used in a frame that is still rendering, we can't immediately delete them. Instead we transfer ownership to
// the retirement manager and allow it to clean them up, but only after the frames which are potentially using the resources have completed.
//------------------------------------------------------------------------------------------------------------------------------------------
class RetirementMgr {
public:
    RetirementMgr() noexcept;
    ~RetirementMgr() noexcept;

    bool init(LogicalDevice& device) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;
    void registerRetirementProvider(IRetirementProvider& provider) noexcept;
    void unregisterRetirementProvider(IRetirementProvider& provider) noexcept;
    void freeRetiredResourcesForAllRingbufferSlots() noexcept;
    void freeRetiredResourcesForRingbufferIndex(const uint8_t ringbufferIndex) noexcept;

    void retire(CmdBuffer& cmdBuffer) noexcept;
    void retire(DescriptorPool& descriptorPool) noexcept;
    void retire(DescriptorSet& descriptorSet) noexcept;
    void retire(DescriptorSetLayout& descriptorSetLayout) noexcept;
    void retire(PipelineLayout& pipelineLayout) noexcept;
    void retire(Pipeline& pipeline) noexcept;
    void retire(MutableTexture& mutableTexture) noexcept;
    void retire(RawBuffer& rawBuffer) noexcept;
    void retire(RenderTexture& renderTexture) noexcept;
    void retire(ShaderModule& shaderModule) noexcept;
    void retire(Texture& texture) noexcept;
    void retire(Framebuffer& framebuffer) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }

private:
    // Copy and move disallowed
    RetirementMgr(const RetirementMgr& other) = delete;
    RetirementMgr(RetirementMgr&& other) = delete;
    RetirementMgr& operator = (const RetirementMgr& other) = delete;
    RetirementMgr& operator = (RetirementMgr&& other) = delete;

    // Holds lists of resources to be retired for a particular ringbuffer index
    struct RetiredResourceSet {
        std::vector<CmdBuffer>              mCmdBuffers;
        std::vector<DescriptorPool>         mDescriptorPools;
        std::vector<DescriptorSet*>         mDescriptorSets;
        std::vector<DescriptorSetLayout>    mDescriptorSetLayouts;
        std::vector<PipelineLayout>         mPipelineLayouts;
        std::vector<Pipeline>               mPipelines;
        std::vector<MutableTexture>         mMutableTextures;
        std::vector<RawBuffer>              mRawBuffers;
        std::vector<RenderTexture>          mRenderTextures;
        std::vector<ShaderModule>           mShaderModules;
        std::vector<Texture>                mTextures;
        std::vector<Framebuffer>            mFramebuffers;

        void clear() noexcept;
        void compact() noexcept;

        template <class ResourceType>
        auto& getListForResourceType() noexcept;

        template <class ResourceType>
        void clearListForResourceType() noexcept;

        template <class ResourceType>
        void retireResourceToSet(ResourceType& resource) noexcept;
    };

    RetiredResourceSet& getCurrentRetiredResourceSet() noexcept;

    bool mbIsValid;

    #if ASSERTS_ENABLED == 1
        // A Flag set to true in debug if we are destroying resources.
        // Note that retiring resources is DISALLOWED while resources are being destroyed.
        // A destroy operation must result in complete destruction of an object and all its children.
        bool mbDebugIsDestroyingResources;
    #endif

    LogicalDevice*                  mpDevice;
    RingbufferMgr*                  mpRingbufferMgr;
    RetiredResourceSet              mRetiredResourceSets[Defines::RINGBUFFER_SIZE];
    std::set<IRetirementProvider*>  mExternalRetirementProviders;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Specializations of RetiredResourceSet functions.
// Have to declare here (as opposed to inside the 'RetiredResourceSet' class) because of the following GCC bug:
// 
//      https://stackoverflow.com/questions/49707184/explicit-specialization-in-non-namespace-scope-does-not-compile-in-gcc
//      https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
// 
// This bug does not exist in MSVC and works fine in Clang too (according to Stack Overflow)
//------------------------------------------------------------------------------------------------------------------------------------------
template <>
void RetirementMgr::RetiredResourceSet::clearListForResourceType<DescriptorSet>() noexcept;

template <>
void RetirementMgr::RetiredResourceSet::retireResourceToSet<DescriptorSet>(DescriptorSet& resource) noexcept;

END_NAMESPACE(vgl)
