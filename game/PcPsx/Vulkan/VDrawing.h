#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"

#include <cstdint>

namespace vgl {
    class BaseTexture;
    class CmdBufferRecorder;
    class LogicalDevice;
}

namespace Gpu {
    enum class BlendMode : uint8_t;
    enum class TexFmt : uint8_t;
}

enum class VPipelineType : uint8_t;

BEGIN_NAMESPACE(VDrawing)

void init(vgl::LogicalDevice& device, vgl::BaseTexture& vramTex) noexcept;
void shutdown() noexcept;
void beginFrame(vgl::LogicalDevice& device, vgl::CmdBufferRecorder& cmdRec) noexcept;
void endFrame() noexcept;
void setPipeline(const VPipelineType type) noexcept;
void setTransformMatrix(const float mvpMatrix[4][4]) noexcept;
void setTransformMatrixForUI() noexcept;

void addAlphaBlendedUILine(
    const float x1,
    const float y1,
    const float x2,
    const float y2,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const uint8_t a,
    const bool bBlend
) noexcept;

void addAlphaBlendedUISprite(
    const float x,
    const float y,
    const float w,
    const float h,
    const float u,
    const float v,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const uint8_t a,
    const uint16_t clutX,
    const uint16_t clutY,
    const uint16_t texPageX,
    const uint16_t texPageY,
    const Gpu::TexFmt texFmt,
    const bool bBlend
) noexcept;

END_NAMESPACE(VDrawing)

#endif  // #if PSYDOOM_VULKAN_RENDERER
