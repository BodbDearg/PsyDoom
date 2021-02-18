#pragma once

#include "Defines.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;
class PipelineLayout;
class RenderPass;
class ShaderModule;

//------------------------------------------------------------------------------------------------------------------------------------------
// Pipeline state: input assembly.
// A simplified and defaultable version of 'VkPipelineInputAssemblyStateCreateInfo'. See docs for that for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PipelineInputAssemblyState {
    VkPrimitiveTopology     topology;
    bool                    bPrimitiveRestartEnable;

    PipelineInputAssemblyState& setToDefault() noexcept {
        topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        bPrimitiveRestartEnable = false;
        return *this;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Pipeline state: rasterization.
// A simplified and defaultable version of 'VkPipelineRasterizationStateCreateInfo'. See docs for that for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PipelineRasterizationState {
    bool                bDepthClampEnable;
    bool                bRasterizerDiscardEnable;
    VkPolygonMode       polygonMode;
    VkCullModeFlags     cullMode;
    VkFrontFace         frontFace;
    bool                bDepthBiasEnable;
    float               depthBiasConstantFactor;
    float               depthBiasClamp;
    float               depthBiasSlopeFactor;
    float               lineWidth;

    PipelineRasterizationState& setToDefault() noexcept {
        bDepthClampEnable = false;
        bRasterizerDiscardEnable = false;
        polygonMode = VK_POLYGON_MODE_FILL;
        cullMode = VK_CULL_MODE_NONE;
        frontFace = VK_FRONT_FACE_CLOCKWISE;
        bDepthBiasEnable = false;
        depthBiasConstantFactor = 0.0f;
        depthBiasClamp = 0.0f;
        depthBiasSlopeFactor = 0.0f;
        lineWidth = 1.0f;
        return *this;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Pipeline state: multisampling.
// A simplified and defaultable version of 'VkPipelineMultisampleStateCreateInfo'. See docs for that for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PipelineMultisampleState {
    VkSampleCountFlagBits   rasterizationSamples;
    VkBool32                sampleShadingEnable;
    float                   minSampleShading;
    const VkSampleMask*     pSampleMask;
    VkBool32                alphaToCoverageEnable;
    VkBool32                alphaToOneEnable;

    PipelineMultisampleState& setToDefault() noexcept {
        rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        sampleShadingEnable = VK_FALSE;
        minSampleShading = 1.0f;
        pSampleMask = nullptr;
        alphaToCoverageEnable = VK_FALSE;
        alphaToOneEnable = VK_FALSE;
        return *this;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Pipeline state: color blend attachment state.
// Just a defaultable version of 'VkPipelineColorBlendAttachmentState'. See docs for that for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PipelineColorBlendAttachmentState : VkPipelineColorBlendAttachmentState {
    PipelineColorBlendAttachmentState& setToDefault() noexcept {
        blendEnable = VK_FALSE;
        srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendOp = VK_BLEND_OP_ADD;
        srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        alphaBlendOp = VK_BLEND_OP_ADD;
        colorWriteMask = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
        return *this;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Pipeline state: color blending.
// A simplified and defaultable version of 'VkPipelineColorBlendStateCreateInfo'. See docs for that for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PipelineColorBlendState {
    bool                                            bLogicOpEnable;
    VkLogicOp                                       logicOp;
    uint32_t                                        attachmentCount;
    const VkPipelineColorBlendAttachmentState*      pAttachments;
    float                                           blendConstants[4];

    PipelineColorBlendState& setToDefault() noexcept {
        bLogicOpEnable = false;
        logicOp = VK_LOGIC_OP_CLEAR;
        attachmentCount = 0;
        pAttachments = nullptr;
        blendConstants[0] = 0.0f;
        blendConstants[1] = 0.0f;
        blendConstants[2] = 0.0f;
        blendConstants[3] = 0.0f;
        return *this;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Pipeline state: depth/stencil.
// A simplified and defaultable version of 'VkPipelineDepthStencilStateCreateFlags'. See docs for that for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PipelineDepthStencilState {
    bool                bDepthTestEnable;
    bool                bDepthWriteEnable;
    VkCompareOp         depthCompareOp;
    bool                bDepthBoundsTestEnable;
    bool                bStencilTestEnable;
    VkStencilOpState    front;
    VkStencilOpState    back;
    float               minDepthBounds;
    float               maxDepthBounds;

    PipelineDepthStencilState& setToDefault() noexcept {
        bDepthTestEnable = false;
        bDepthWriteEnable = false;
        depthCompareOp = VK_COMPARE_OP_ALWAYS;
        bDepthBoundsTestEnable = false;
        bStencilTestEnable = false;
        front = {};
        back = {};
        minDepthBounds = 0.0f;
        maxDepthBounds = 0.0f;
        return *this;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan graphics or compute pipeline.
// 
// A graphics pipeline encapsulates most of the renderer state (shaders, rasterizer settings etc.) except for uniforms and individual
// descriptor bindings like textures and storage buffers. A compute pipeline does similar things for compute, except obviously minus
// anything graphical related.
//------------------------------------------------------------------------------------------------------------------------------------------
class Pipeline {
public:
    Pipeline() noexcept;
    Pipeline(Pipeline&& other) noexcept;
    ~Pipeline() noexcept;

    bool initGraphicsPipeline(
        const PipelineLayout& pipelineLayout,
        const RenderPass& renderPass,
        const uint32_t subpassIndex,
        const ShaderModule* const* const ppShaderModules,
        const uint32_t numShaderModules,
        const VkSpecializationInfo* const pShaderSpecializationInfo,
        const VkVertexInputBindingDescription* pVertexInputBindingDescs,
        const uint32_t numVertexInputBindingDescs,
        const VkVertexInputAttributeDescription* pVertexInputAttribDescs,
        const uint32_t numVertexInputAttribDescs,
        const PipelineInputAssemblyState& inputAssemblyState,
        const PipelineRasterizationState& rasterizationState,
        const PipelineMultisampleState& multisampleState,
        const PipelineColorBlendState& colorBlendState,
        const PipelineDepthStencilState& depthStencilState
    ) noexcept;

    bool initComputePipeline(
        const VkPipelineLayout& pipelineLayout,
        const ShaderModule& shaderModule,
        const VkSpecializationInfo* const pShaderSpecializationInfo
    ) noexcept;

    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline bool isComputePipeline() const noexcept { return mbIsComputePipeline; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkPipeline getVkPipeline() const noexcept { return mVkPipeline; }
    inline VkPipelineLayout getVkPipelineLayout() const noexcept { return mVkPipelineLayout; }

private:
    // Copy and move assign are disallowed
    Pipeline(const Pipeline& other) = delete;
    Pipeline& operator = (const Pipeline& other) = delete;
    Pipeline& operator = (Pipeline&& other) = delete;

    bool                mbIsValid;
    bool                mbIsComputePipeline;
    LogicalDevice*      mpDevice;
    VkPipeline          mVkPipeline;
    VkPipelineLayout    mVkPipelineLayout;      // What pipeline layout this was created with: must exist for the lifetime of the graphics pipeline
};

END_NAMESPACE(vgl)
