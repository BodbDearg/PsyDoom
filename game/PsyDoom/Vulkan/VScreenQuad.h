#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Buffer.h"

namespace vgl {
    class CmdBufferRecorder;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Class that creates a Vulkan vertex buffer containing a screen sized quad in NDC coordinates.
// The vertex type used is 'VVertex_XyUv' and the XY coordinates range from -1 to +1.
// The UV coordinates range from 0 to 1.
//------------------------------------------------------------------------------------------------------------------------------------------
class VScreenQuad {
public:
    VScreenQuad() noexcept;
    ~VScreenQuad() noexcept;

    void init(vgl::LogicalDevice& device) noexcept;
    void destroy(const bool bImmediateCleanup = false) noexcept;
    bool isValid() const noexcept;

    void bindVertexBuffer(vgl::CmdBufferRecorder& cmdRec, const uint32_t bindingIdx) noexcept;
    void draw(vgl::CmdBufferRecorder& cmdRec) noexcept;

private:
    vgl::Buffer     mVertexBuffer;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
