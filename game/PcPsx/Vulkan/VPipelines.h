#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"
#include "VTypes.h"

namespace vgl {
    class BaseRenderPass;
    class DescriptorSetLayout;
    class LogicalDevice;
    class Pipeline;
    class PipelineLayout;
    class Sampler;
}

BEGIN_NAMESPACE(VPipelines)

extern vgl::Sampler                 gSampler_draw;
extern vgl::DescriptorSetLayout     gDescSetLayout_draw;
extern vgl::DescriptorSetLayout     gDescSetLayout_msaaResolve;
extern vgl::PipelineLayout          gPipelineLayout_draw;
extern vgl::PipelineLayout          gPipelineLayout_msaaResolve;
extern vgl::Pipeline                gPipelines[(size_t) VPipelineType::NUM_TYPES];

void init(vgl::LogicalDevice& device, vgl::BaseRenderPass& renderPass, const uint32_t numSamples) noexcept;
void shutdown() noexcept;

END_NAMESPACE(VPipelines)

#endif  // #if PSYDOOM_VULKAN_RENDERER
