#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"

#include <cstdint>

namespace vgl {
    class CmdBufferRecorder;
    class LogicalDevice;
}

enum class VPipelineType : uint8_t;

BEGIN_NAMESPACE(VDrawing)

void init(vgl::LogicalDevice& device) noexcept;
void shutdown() noexcept;
void beginFrame(vgl::LogicalDevice& device, vgl::CmdBufferRecorder& cmdRec) noexcept;
void endFrame() noexcept;
void setPipeline(const VPipelineType type) noexcept;
void setTransformMatirx(const float mvpMatrix[4][4]) noexcept;

void addUISprite(
    const int16_t x,
    const int16_t y,
    const uint16_t w,
    const uint16_t h,
    const uint16_t u,
    const uint16_t v,
    const uint16_t clutId,
    const uint16_t texPageId
) noexcept;

END_NAMESPACE(VDrawing)

#endif  // #if PSYDOOM_VULKAN_RENDERER
