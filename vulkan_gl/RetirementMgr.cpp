#include "RetirementMgr.h"

#include "CmdBuffer.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "Framebuffer.h"
#include "IRetirementProvider.h"
#include "LogicalDevice.h"
#include "MutableTexture.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "RawBuffer.h"
#include "RenderTexture.h"
#include "RingbufferMgr.h"
#include "ShaderModule.h"
#include "Texture.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a default non-initialized retirement manager
//------------------------------------------------------------------------------------------------------------------------------------------
RetirementMgr::RetirementMgr() noexcept
    : mbIsValid(false)
#if ASSERTS_ENABLED == 1
    , mbDebugIsDestroyingResources(false)
#endif
    , mpDevice(nullptr)
    , mpRingbufferMgr(nullptr)
    , mRetiredResourceSets()
    , mExternalRetirementProviders()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroy/cleanup the retirement manager
//------------------------------------------------------------------------------------------------------------------------------------------
RetirementMgr::~RetirementMgr() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializess the retirement manager for the specified device
//------------------------------------------------------------------------------------------------------------------------------------------
bool RetirementMgr::init(LogicalDevice& device) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");

    // Note that this never fails (assuming preconditions are met)
    mpDevice = &device;
    mpRingbufferMgr = &device.getRingbufferMgr();
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the retirement manager and immediately retires all resources due to be retired
//------------------------------------------------------------------------------------------------------------------------------------------
void RetirementMgr::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Cleanup everything
    mbIsValid = false;

    #if ASSERTS_ENABLED == 1
        mbDebugIsDestroyingResources = true;
    #endif

    for (IRetirementProvider* pRetirementProvider : mExternalRetirementProviders) {
        pRetirementProvider->freeRetiredResourcesForAllRingbufferSlots();
    }
    
    mExternalRetirementProviders.clear();
    
    for (RetiredResourceSet& set : mRetiredResourceSets) {
        set.clear();
        set.compact();
    }

    #if ASSERTS_ENABLED == 1
        mbDebugIsDestroyingResources = false;
    #endif
    
    mpRingbufferMgr = nullptr;
    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Registers an external retirement provider with the retirement manager. This provider will be receive the same 'free resources' events
// that the retirement manager itself receives and is thus synchronized to retire resources at the same time.
//------------------------------------------------------------------------------------------------------------------------------------------
void RetirementMgr::registerRetirementProvider(IRetirementProvider& provider) noexcept {
    ASSERT(!mbDebugIsDestroyingResources);
    mExternalRetirementProviders.insert(&provider);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// De-register a previously registered external retirement provider
//------------------------------------------------------------------------------------------------------------------------------------------
void RetirementMgr::unregisterRetirementProvider(IRetirementProvider& provider) noexcept {
    ASSERT(!mbDebugIsDestroyingResources);
    mExternalRetirementProviders.erase(&provider);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free all retired resources for all ringbuffer slots.
// Note that you should only call this if you know that all rendering operations have finished!
//------------------------------------------------------------------------------------------------------------------------------------------
void RetirementMgr::freeRetiredResourcesForAllRingbufferSlots() noexcept {
    if (!mbIsValid)
        return;
    
    #if ASSERTS_ENABLED == 1
        mbDebugIsDestroyingResources = true;
    #endif
    
    for (IRetirementProvider* pRetirementProvider : mExternalRetirementProviders) {
        pRetirementProvider->freeRetiredResourcesForAllRingbufferSlots();
    }
    
    for (RetiredResourceSet& set : mRetiredResourceSets) {
        set.clear();
    }

    #if ASSERTS_ENABLED == 1
        mbDebugIsDestroyingResources = false;
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free retired resources for a particular ringbuffer index.
// This should be called once the frame the ringbuffer slot is for has completely finished.
//------------------------------------------------------------------------------------------------------------------------------------------
void RetirementMgr::freeRetiredResourcesForRingbufferIndex(const uint8_t ringbufferIndex) noexcept {
    ASSERT(ringbufferIndex < Defines::RINGBUFFER_SIZE);

    #if ASSERTS_ENABLED == 1
        mbDebugIsDestroyingResources = true;
    #endif
    
    for (IRetirementProvider* pRetirementProvider : mExternalRetirementProviders) {
        pRetirementProvider->freeRetiredResourcesForRingbufferIndex(ringbufferIndex);
    }

    RetiredResourceSet& set = mRetiredResourceSets[ringbufferIndex];
    set.clear();

    #if ASSERTS_ENABLED == 1
        mbDebugIsDestroyingResources = false;
    #endif
}


//------------------------------------------------------------------------------------------------------------------------------------------
// Retire resources of different types. The retired resources are placed in the retired resource list
// for the current ringbuffer index defined by the ringbuffer manager.
//
// Important:
//      Retire is DISALLOWED while 'destroyAllResources' or 'destroyResourcesForRingbufferIndex' is being called!
//      The reason for this is because it is unsafe to modify the retirement lists while they are being traversed.
//      The destruction of an object should also be atomic and result in it's complete destruction, as well as the
//      destruction of all it's children.
//------------------------------------------------------------------------------------------------------------------------------------------

// This reduces some repetition
#define IMPL_RETIRE_FUNC_FOR_TYPE(Type)\
    void RetirementMgr::retire(Type& objToRetire) noexcept {\
        ASSERT(mbIsValid);\
        ASSERT(!mbDebugIsDestroyingResources);\
        getCurrentRetiredResourceSet().retireResourceToSet(objToRetire);\
    }

IMPL_RETIRE_FUNC_FOR_TYPE(CmdBuffer)
IMPL_RETIRE_FUNC_FOR_TYPE(DescriptorPool)
IMPL_RETIRE_FUNC_FOR_TYPE(DescriptorSet)
IMPL_RETIRE_FUNC_FOR_TYPE(DescriptorSetLayout)
IMPL_RETIRE_FUNC_FOR_TYPE(PipelineLayout)
IMPL_RETIRE_FUNC_FOR_TYPE(Pipeline)
IMPL_RETIRE_FUNC_FOR_TYPE(MutableTexture)
IMPL_RETIRE_FUNC_FOR_TYPE(RawBuffer)
IMPL_RETIRE_FUNC_FOR_TYPE(RenderTexture)
IMPL_RETIRE_FUNC_FOR_TYPE(ShaderModule)
IMPL_RETIRE_FUNC_FOR_TYPE(Texture)
IMPL_RETIRE_FUNC_FOR_TYPE(Framebuffer)

#undef IMPL_RETIRE_FUNC_FOR_TYPE

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears all entries in the retired resource set
//------------------------------------------------------------------------------------------------------------------------------------------
void RetirementMgr::RetiredResourceSet::clear() noexcept {
    // N.B: Reverse of declaration order
    clearListForResourceType<Framebuffer>();
    clearListForResourceType<Texture>();
    clearListForResourceType<ShaderModule>();
    clearListForResourceType<RenderTexture>();
    clearListForResourceType<RawBuffer>();
    clearListForResourceType<MutableTexture>();
    clearListForResourceType<Pipeline>();
    clearListForResourceType<PipelineLayout>();
    clearListForResourceType<DescriptorSetLayout>();
    clearListForResourceType<DescriptorSet>();
    clearListForResourceType<DescriptorPool>();
    clearListForResourceType<CmdBuffer>();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Compacts the resource lists in the retired resource set to consume as little memory as possible
//------------------------------------------------------------------------------------------------------------------------------------------
void RetirementMgr::RetiredResourceSet::compact() noexcept {
    // N.B: Reverse of declaration order
    mFramebuffers.shrink_to_fit();
    mTextures.shrink_to_fit();
    mShaderModules.shrink_to_fit();
    mRenderTextures.shrink_to_fit();
    mRawBuffers.shrink_to_fit();
    mMutableTextures.shrink_to_fit();
    mPipelines.shrink_to_fit();
    mPipelineLayouts.shrink_to_fit();
    mDescriptorSetLayouts.shrink_to_fit();
    mDescriptorSets.shrink_to_fit();
    mDescriptorPools.shrink_to_fit();
    mCmdBuffers.shrink_to_fit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the retirement list for a particular resource type in the retired resource set
//------------------------------------------------------------------------------------------------------------------------------------------

// This reduces some repetition
#define IMPL_GET_LIST_FOR_RESOURCE_TYPE(Type, ListVar)\
    template <>\
    auto& RetirementMgr::RetiredResourceSet::getListForResourceType<Type>() noexcept {\
        return ListVar;\
    }

IMPL_GET_LIST_FOR_RESOURCE_TYPE(CmdBuffer,              mCmdBuffers)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(DescriptorPool,         mDescriptorPools)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(DescriptorSet,          mDescriptorSets)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(DescriptorSetLayout,    mDescriptorSetLayouts)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(PipelineLayout,         mPipelineLayouts)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(Pipeline,               mPipelines);
IMPL_GET_LIST_FOR_RESOURCE_TYPE(MutableTexture,         mMutableTextures)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(RawBuffer,              mRawBuffers)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(RenderTexture,          mRenderTextures)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(ShaderModule,           mShaderModules)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(Texture,                mTextures)
IMPL_GET_LIST_FOR_RESOURCE_TYPE(Framebuffer,            mFramebuffers)

#undef IMPL_GET_LIST_FOR_RESOURCE_TYPE

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroy and clear the resources in the list of resources for a particular resource type.
// This should be called instead of a direct vector 'clear' to ensure proper cleanup and that we don't inadvertently trigger
// retirement to this retirement manager WHILE the list is being cleared itself.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ResourceType>
void RetirementMgr::RetiredResourceSet::clearListForResourceType() noexcept {
    auto& retiredResourceList = getListForResourceType<ResourceType>();

    for (auto& resource : retiredResourceList) {
        // Note: 'true' = destroy immediate.
        // Don't want the resource adding itself back into the retirement manager again!
        resource.destroy(true);
    }

    retiredResourceList.clear();
}

template <>
void RetirementMgr::RetiredResourceSet::clearListForResourceType<DescriptorSet>() noexcept {
    std::vector<DescriptorSet*>& retiredDescriptorSets = getListForResourceType<DescriptorSet>();

    for (DescriptorSet* pDescriptorSet : retiredDescriptorSets) {
        if (pDescriptorSet->isValid()) {
            pDescriptorSet->free(true);
        }
    }

    retiredDescriptorSets.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a generic resource type to the retired source set
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ResourceType>
void RetirementMgr::RetiredResourceSet::retireResourceToSet(ResourceType& resource) noexcept {
    auto& retiredResourceList = getListForResourceType<ResourceType>();
    retiredResourceList.emplace_back(std::move(resource));
}

template <>
void RetirementMgr::RetiredResourceSet::retireResourceToSet<DescriptorSet>(DescriptorSet& resource) noexcept {
    // Descriptor sets are treated a bit differently, hence specialization!
    auto& retiredResourceList = getListForResourceType<DescriptorSet>();
    retiredResourceList.push_back(&resource);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current retired resource set
//------------------------------------------------------------------------------------------------------------------------------------------
RetirementMgr::RetiredResourceSet& RetirementMgr::getCurrentRetiredResourceSet() noexcept {
    const uint8_t ringbufferIdx = mpRingbufferMgr->getBufferIndex();
    return mRetiredResourceSets[ringbufferIdx];
}

END_NAMESPACE(vgl)
