#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"
#include "VTypes.h"

namespace vgl {
    class DescriptorSetLayout;
    class LogicalDevice;
    class Pipeline;
    class PipelineLayout;
    class RenderPass;
    class Sampler;
}

class VRenderPath_FadeLoad;
class VRenderPath_Main;

BEGIN_NAMESPACE(VPipelines)

extern vgl::Sampler                 gSampler_draw;
extern vgl::Sampler                 gSampler_crossfade;
extern vgl::DescriptorSetLayout     gDescSetLayout_draw;
extern vgl::DescriptorSetLayout     gDescSetLayout_msaaResolve;
extern vgl::DescriptorSetLayout     gDescSetLayout_crossfade;
extern vgl::PipelineLayout          gPipelineLayout_draw;
extern vgl::PipelineLayout          gPipelineLayout_msaaResolve;
extern vgl::PipelineLayout          gPipelineLayout_crossfade;
extern vgl::Pipeline                gPipelines[(size_t) VPipelineType::NUM_TYPES];

void initPipelineComponents(vgl::LogicalDevice& device, const uint32_t numSamples) noexcept;

void initPipelines(
    VRenderPath_Main& mainRPath,
    VRenderPath_FadeLoad& fadeLoadRPath,
    const uint32_t numSamples
) noexcept;

void shutdown() noexcept;

END_NAMESPACE(VPipelines)

#endif  // #if PSYDOOM_VULKAN_RENDERER
