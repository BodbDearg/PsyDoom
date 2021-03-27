#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"

#include <cstdint>

namespace vgl {
    class CmdBufferRecorder;
    class DeviceSurfaceCaps;
    class LogicalDevice;
    class PhysicalDevice;
    class Swapchain;
    struct VkFuncs;
}

class IVRendererPath;
class VRenderPath_Crossfade;
class VRenderPath_Main;
class VRenderPath_Psx;
struct VShaderUniforms_Draw;

BEGIN_NAMESPACE(VRenderer)

// Min/max supported depth ranges for the renderer
static constexpr float MIN_DEPTH = 1.0f;
static constexpr float MAX_DEPTH = 32768.0f;

extern vgl::VkFuncs             gVkFuncs;
extern VRenderPath_Psx          gRenderPath_Psx;
extern VRenderPath_Main         gRenderPath_Main;
extern VRenderPath_Crossfade    gRenderPath_Crossfade;
extern uint32_t                 gPresentSurfaceW;
extern uint32_t                 gPresentSurfaceH;
extern uint32_t                 gFramebufferW;
extern uint32_t                 gFramebufferH;
extern float                    gInvFramebufferW;
extern float                    gInvFramebufferH;
extern float                    gCurLogicalDisplayW;
extern float                    gPsxCoordsFbX;
extern float                    gPsxCoordsFbY;
extern float                    gPsxCoordsFbW;
extern float                    gPsxCoordsFbH;
extern float                    gNdcToPsxScaleX, gNdcToPsxScaleY;
extern float                    gPsxNdcOffsetX,  gPsxNdcOffsetY;
extern vgl::LogicalDevice       gDevice;
extern vgl::Swapchain           gSwapchain;
extern vgl::CmdBufferRecorder   gCmdBufferRec;

bool isHeadlessPhysicalDeviceSuitable(const vgl::PhysicalDevice& device) noexcept;
bool isPhysicalDeviceSuitable(const vgl::PhysicalDevice& device, const vgl::DeviceSurfaceCaps& surfaceCaps) noexcept;
void init() noexcept;
void destroy() noexcept;
bool beginFrame() noexcept;
bool isRendering() noexcept;
void endFrame() noexcept;
void pushPsxVramUpdates(const uint16_t rectLx, const uint16_t rectRx, const uint16_t rectTy, const uint16_t rectBy) noexcept;
void initRendererUniformFields(VShaderUniforms_Draw& uniforms) noexcept;
IVRendererPath& getActiveRenderPath() noexcept;
IVRendererPath& getNextRenderPath() noexcept;
void setNextRenderPath(IVRendererPath& renderPath) noexcept;
bool isUsingPsxRenderPath() noexcept;
void switchToPsxRenderPath() noexcept;
void switchToMainVulkanRenderPath() noexcept;
void skipNextFramePresent() noexcept;
bool willSkipNextFramePresent() noexcept;
bool isSwapchainOutOfDate() noexcept;

END_NAMESPACE(VRenderer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
