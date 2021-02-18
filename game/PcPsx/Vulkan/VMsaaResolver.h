#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
#include "Buffer.h"
#include "Defines.h"
#include "DescriptorPool.h"
#include "RenderTexture.h"

namespace vgl {
    class CmdBufferRecorder;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handles the resolving of MSAA and the resources required for that is MSAA is enabled for the game
//------------------------------------------------------------------------------------------------------------------------------------------
class VMsaaResolver {
public:
    VMsaaResolver() noexcept;
    ~VMsaaResolver() noexcept;

    void init(vgl::LogicalDevice& device) noexcept;
    void destroy() noexcept;

    bool createResolveAttachments(const VkFormat format, const uint32_t fbWidth, const uint32_t fbHeight) noexcept;
    void destroyResolveAttachments() noexcept;
    void setInputAttachments(const vgl::BaseTexture inputAttachments[vgl::Defines::RINGBUFFER_SIZE]) noexcept;
    void resolve(vgl::CmdBufferRecorder& cmdRec) noexcept;

    vgl::RenderTexture& getResolveAttachment(const uint32_t index) noexcept {
        ASSERT(index < vgl::Defines::RINGBUFFER_SIZE);
        return mResolveAttachments[index];
    }

    bool areAllResolveAttachmentsValid(const uint32_t fbWidth, const uint32_t fbHeight) noexcept;

private:
    void initVertexBuffer(vgl::LogicalDevice& device) noexcept;
    void initDescriptorPoolAndSets(vgl::LogicalDevice& device) noexcept;

    // Set to true once the resolver is validly initialized
    bool mbIsValid;

    // A vertex buffer containing a single quad (two triangles) of vertex type 'VVertex_MsaaResolve' covering the entire screen.
    // This is used to draw a screen quad during MSAA resolve.
    vgl::Buffer mVertexBuffer;

    // Destination color attachments for resolving MSAA samples to 1 sample
    vgl::RenderTexture mResolveAttachments[vgl::Defines::RINGBUFFER_SIZE];

    // A descriptor pool and the descriptor sets allocated from it.
    // The descriptor sets just contain a binding for an input multi-sampled attachment to resolve - one for each ringbuffer index so we don't have to update constantly.
    vgl::DescriptorPool mDescriptorPool;
    vgl::DescriptorSet* mpDescriptorSets[vgl::Defines::RINGBUFFER_SIZE];
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
