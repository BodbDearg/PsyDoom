#pragma once

#include "AlphaMode.h"
#include "BaseTexture.h"

BEGIN_NAMESPACE(vgl)

class MutableTexture;
class TransferTask;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a 2d texture that can be rendered to.
// This is either a normal texture or a depth stencil buffer.
// Note that for depth stencil textures the 'stencil' portion is optional and depends on the requested depth stencil format.
//------------------------------------------------------------------------------------------------------------------------------------------
class RenderTexture : public BaseTexture {
public:
    RenderTexture() noexcept;
    RenderTexture(RenderTexture&& other) noexcept;
    ~RenderTexture() noexcept;

    bool initAsRenderTexture(
        LogicalDevice& device,
        const VkFormat textureFormat,
        const uint32_t width,
        const uint32_t height,
        const AlphaMode alphaMode = AlphaMode::PREMULTIPLIED
    ) noexcept;

    bool initAsDepthStencilBuffer(
        LogicalDevice& device,
        const VkFormat textureFormat,
        const uint32_t width,
        const uint32_t height
    ) noexcept;

    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;

    void scheduleCopyTo(MutableTexture& mutableTex, TransferTask& transferTask) noexcept;
    
private:
    // Copy and move assign are disallowed
    RenderTexture(const RenderTexture& other) = delete;
    RenderTexture& operator = (const RenderTexture& other) = delete;
    RenderTexture& operator = (RenderTexture&& other) = delete;
};

END_NAMESPACE(vgl)
