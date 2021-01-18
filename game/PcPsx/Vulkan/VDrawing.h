#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"
#include "Matrix4.h"

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

enum class VLightDimMode : uint8_t;
enum class VPipelineType : uint8_t;
enum class VPipelineType : uint8_t;

BEGIN_NAMESPACE(VDrawing)

void init(vgl::LogicalDevice& device, vgl::BaseTexture& vramTex) noexcept;
void shutdown() noexcept;
void beginFrame(vgl::LogicalDevice& device, vgl::CmdBufferRecorder& cmdRec) noexcept;
void endFrame() noexcept;
void setPipeline(const VPipelineType type) noexcept;
void setTransformMatrix(const Matrix4f& matrix) noexcept;
Matrix4f computeTransformMatrixForUI() noexcept;
Matrix4f computeTransformMatrixFor3D(const float viewX, const float viewY, const float viewZ, const float viewAngle) noexcept;
void endCurrentDrawBatch() noexcept;

void addAlphaBlendedUILine(
    const float x1,
    const float y1,
    const float x2,
    const float y2,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const uint8_t a
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
    const uint16_t texWinW,
    const uint16_t texWinH,
    const Gpu::TexFmt texFmt
) noexcept;

// TODO: add quad variant to submit two triangles at once
void add3dViewTriangle(
    const float x1,
    const float y1,
    const float z1,
    const float u1,
    const float v1,
    const float x2,
    const float y2,
    const float z2,
    const float u2,
    const float v2,
    const float x3,
    const float y3,
    const float z3,
    const float u3,
    const float v3,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const uint16_t clutX,
    const uint16_t clutY,
    const uint16_t texWinX,
    const uint16_t texWinY,
    const uint16_t texWinW,
    const uint16_t texWinH,
    const VLightDimMode lightDimMode,
    const VPipelineType drawPipeline,
    const uint8_t stMulR,
    const uint8_t stMulG,
    const uint8_t stMulB,
    const uint8_t stMulA
) noexcept;

END_NAMESPACE(VDrawing)

#endif  // #if PSYDOOM_VULKAN_RENDERER
