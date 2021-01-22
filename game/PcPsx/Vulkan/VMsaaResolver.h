#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Defines.h"

#include <vulkan/vulkan.h>

namespace vgl {
    class BaseTexture;
    class CmdBufferRecorder;
    class LogicalDevice;
    class RenderTexture;
}

BEGIN_NAMESPACE(VMsaaResolver)

extern vgl::RenderTexture gResolveColorAttachments[vgl::Defines::RINGBUFFER_SIZE];

void init(vgl::LogicalDevice& device) noexcept;
void destroy() noexcept;

bool createResolveColorAttachments(
    vgl::LogicalDevice& device,
    const VkFormat colorFormat,
    const uint32_t fbWidth,
    const uint32_t fbHeight
) noexcept;

void destroyResolveColorAttachments() noexcept;
void setMsaaResolveInputAttachments(const vgl::BaseTexture inputAttachments[vgl::Defines::RINGBUFFER_SIZE]) noexcept;

void doMsaaResolve(vgl::LogicalDevice& device, vgl::CmdBufferRecorder& cmdRec) noexcept;

END_NAMESPACE(VMsaaResolver)

#endif  // #if PSYDOOM_VULKAN_RENDERER
