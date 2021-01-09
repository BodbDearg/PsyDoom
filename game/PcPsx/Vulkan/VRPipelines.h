#include "Macros.h"
#include "VRTypes.h"

namespace vgl {
    class BaseRenderPass;
    class DescriptorSetLayout;
    class LogicalDevice;
    class Pipeline;
    class PipelineLayout;
    class Sampler;
}

BEGIN_NAMESPACE(VRPipelines)

extern vgl::Sampler                 gSampler;
extern vgl::DescriptorSetLayout     gDescriptorSetLayout;
extern vgl::PipelineLayout          gPipelineLayout;
extern vgl::Pipeline                gPipelines[(size_t) VRPipelineType::NUM_TYPES];

void init(vgl::LogicalDevice& device, vgl::BaseRenderPass& renderPass) noexcept;
void shutdown() noexcept;

END_NAMESPACE(VRPipelines)
