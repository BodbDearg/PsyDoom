//------------------------------------------------------------------------------------------------------------------------------------------
// Logic for creating the various shaders and pipelines used by the new Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VPipelines.h"

#include "DescriptorSetLayout.h"
#include "FatalErrors.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "Sampler.h"
#include "ShaderModule.h"

BEGIN_NAMESPACE(VPipelines)

// The raw SPIRV binary code for the shaders
#include "SPIRV_colored_frag.bin.h"
#include "SPIRV_colored_vert.bin.h"
#include "SPIRV_ui_16bpp_frag.bin.h"
#include "SPIRV_ui_4bpp_frag.bin.h"
#include "SPIRV_ui_8bpp_frag.bin.h"
#include "SPIRV_ui_vert.bin.h"
#include "SPIRV_view_frag.bin.h"
#include "SPIRV_view_vert.bin.h"

// Shaders: see the associated source files for more comments/details
static vgl::ShaderModule    gShader_colored_frag;
static vgl::ShaderModule    gShader_colored_vert;
static vgl::ShaderModule    gShader_ui_16bpp_frag;
static vgl::ShaderModule    gShader_ui_4bpp_frag;
static vgl::ShaderModule    gShader_ui_8bpp_frag;
static vgl::ShaderModule    gShader_ui_vert;
static vgl::ShaderModule    gShader_view_frag;
static vgl::ShaderModule    gShader_view_vert;

vgl::Sampler                gSampler;               // The single sampler used: uses nearest neighbor filtering
vgl::DescriptorSetLayout    gDescriptorSetLayout;   // The single descriptor set layout used for the single pipeline
vgl::PipelineLayout         gPipelineLayout;        // The pipeline layout used by all pipelines

// The pipelines themselves
vgl::Pipeline gPipelines[(size_t) VPipelineType::NUM_TYPES];

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the pipelines module
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device, vgl::BaseRenderPass& renderPass) noexcept {
    // Load all of the required shaders
    if (!gShader_colored_vert.init(device, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_colored_vert, sizeof(gSPIRV_colored_vert)))
        FatalErrors::raise("Failed to init Vulkan shader 'colored_vert'!");

    if (!gShader_colored_frag.init(device, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_colored_frag, sizeof(gSPIRV_colored_frag)))
        FatalErrors::raise("Failed to init Vulkan shader 'colored_frag'!");

    if (!gShader_ui_vert.init(device, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_ui_vert, sizeof(gSPIRV_ui_vert)))
        FatalErrors::raise("Failed to init Vulkan shader 'ui_vert'!");

    if (!gShader_ui_4bpp_frag.init(device, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_ui_4bpp_frag, sizeof(gSPIRV_ui_4bpp_frag)))
        FatalErrors::raise("Failed to init Vulkan shader 'ui_4bpp_frag'!");

    if (!gShader_ui_8bpp_frag.init(device, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_ui_8bpp_frag, sizeof(gSPIRV_ui_8bpp_frag)))
        FatalErrors::raise("Failed to init Vulkan shader 'ui_8bpp_frag'!");

    if (!gShader_ui_16bpp_frag.init(device, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_ui_16bpp_frag, sizeof(gSPIRV_ui_16bpp_frag)))
        FatalErrors::raise("Failed to init Vulkan shader 'ui_16bpp_frag'!");

    if (!gShader_view_vert.init(device, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_view_vert, sizeof(gSPIRV_view_vert)))
        FatalErrors::raise("Failed to init Vulkan shader 'view_vert'!");

    if (!gShader_view_frag.init(device, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_view_frag, sizeof(gSPIRV_view_frag)))
        FatalErrors::raise("Failed to init Vulkan shader 'view_frag'!");

    // Create the Vulkan sampler
    {
        vgl::SamplerSettings settings = vgl::SamplerSettings().setToDefault();
        settings.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.minLod = 0;
        settings.maxLod = 0;
        settings.unnormalizedCoordinates = true;    // Note: Vulkan requires min/max lod to be '0' and clamp to edge to be 'true' if using un-normalized coords

        if (!gSampler.init(device, settings))
            FatalErrors::raise("Failed to init a Vulkan sampler!");
    }

    // Create the descriptor set layout.
    // Use an immutable sampler baked into the layout for better performance.
    {
        const VkSampler vkSampler = gSampler.getVkSampler();

        VkDescriptorSetLayoutBinding vramImgSampBinding = {};
        vramImgSampBinding.binding = 0;
        vramImgSampBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        vramImgSampBinding.descriptorCount = 1;
        vramImgSampBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;   // Only reading PSX VRAM in the fragment shader
        vramImgSampBinding.pImmutableSamplers = &vkSampler;

        if (!gDescriptorSetLayout.init(device, &vramImgSampBinding, 1))
            FatalErrors::raise("Failed to init a Vulkan descriptor set layout!");
    }

    // Create the pipeline layout
    {
        VkDescriptorSetLayout vkDescSetLayout = gDescriptorSetLayout.getVkLayout();

        VkPushConstantRange uniformPushConstants = {};
        uniformPushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;   // Just needed in the vertex shader
        uniformPushConstants.offset = 0;
        uniformPushConstants.size = sizeof(VShaderUniforms);

        if (!gPipelineLayout.init(device, &vkDescSetLayout, 1, &uniformPushConstants, 1))
            FatalErrors::raise("Failed to init a Vulkan pipeline layout!");
    }

    // Pipeline state: vertex bindings and attributes. Just using the same bindings and attributes for all pipelines.
    VkVertexInputBindingDescription vertexBindingDesc = {};
    vertexBindingDesc.binding = 0;
    vertexBindingDesc.stride = sizeof(VVertex);
    vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexAttribs[] = {
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT,     offsetof(VVertex, x) },
        { 1, 0, VK_FORMAT_R8G8B8_USCALED,       offsetof(VVertex, r) },
        { 2, 0, VK_FORMAT_R8_UINT,              offsetof(VVertex, lightDimMode) },
        { 3, 0, VK_FORMAT_R32G32_SFLOAT,        offsetof(VVertex, u) },
        { 4, 0, VK_FORMAT_R16G16_UINT,          offsetof(VVertex, texWinX) },
        { 5, 0, VK_FORMAT_R16G16_UINT,          offsetof(VVertex, texWinW) },
        { 6, 0, VK_FORMAT_R16G16_UINT,          offsetof(VVertex, clutX) },
        { 7, 0, VK_FORMAT_R8G8B8A8_USCALED,     offsetof(VVertex, stmulR) },
    };

    // Pipeline state: sets of shader modules
    vgl::ShaderModule* const shaderModules_colored[] = { &gShader_colored_vert, &gShader_colored_frag };
    vgl::ShaderModule* const shaderModules_ui_4bpp[] = { &gShader_ui_vert, &gShader_ui_4bpp_frag };
    vgl::ShaderModule* const shaderModules_ui_8bpp[] = { &gShader_ui_vert, &gShader_ui_8bpp_frag };
    vgl::ShaderModule* const shaderModules_ui_16bpp[] = { &gShader_ui_vert, &gShader_ui_16bpp_frag };
    vgl::ShaderModule* const shaderModules_view[] = { &gShader_view_vert, &gShader_view_frag };

    // Pipeline state: input primitive types
    vgl::PipelineInputAssemblyState lineListInput = vgl::PipelineInputAssemblyState().setToDefault();
    lineListInput.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    vgl::PipelineInputAssemblyState triangleListInput = vgl::PipelineInputAssemblyState().setToDefault();
    triangleListInput.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Pipelines state: rasterization and depth stencil
    vgl::PipelineRasterizationState rasterizerState_noCull = vgl::PipelineRasterizationState().setToDefault();

    vgl::PipelineRasterizationState rasterizerState_backFaceCull = vgl::PipelineRasterizationState().setToDefault();
    rasterizerState_backFaceCull.cullMode = VK_CULL_MODE_BACK_BIT;
    
    // Pipeline states: color blending
    VkPipelineColorBlendAttachmentState attachBlend_common = {};
    attachBlend_common.blendEnable = VK_TRUE;
    attachBlend_common.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachBlend_common.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachBlend_common.alphaBlendOp = VK_BLEND_OP_ADD;
    attachBlend_common.colorWriteMask = (
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT    // Don't need alpha writes but its probably more efficient to write 32-bits at a time
    );

    VkPipelineColorBlendAttachmentState attachBlend_alpha = attachBlend_common;
    attachBlend_alpha.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachBlend_alpha.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachBlend_alpha.colorBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendAttachmentState attachBlend_additive = attachBlend_common;
    attachBlend_additive.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachBlend_additive.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachBlend_additive.colorBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendAttachmentState attachBlend_subtractive = attachBlend_common;
    attachBlend_subtractive.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachBlend_subtractive.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachBlend_subtractive.colorBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;

    vgl::PipelineColorBlendState blendState_alpha = vgl::PipelineColorBlendState().setToDefault();
    blendState_alpha.pAttachments = &attachBlend_alpha;
    blendState_alpha.attachmentCount = 1;

    vgl::PipelineColorBlendState blendState_additive = vgl::PipelineColorBlendState().setToDefault();
    blendState_additive.pAttachments = &attachBlend_additive;
    blendState_additive.attachmentCount = 1;

    vgl::PipelineColorBlendState blendState_subtractive = vgl::PipelineColorBlendState().setToDefault();
    blendState_subtractive.pAttachments = &attachBlend_subtractive;
    blendState_subtractive.attachmentCount = 1;

    // Depth stencil pipeline states
    vgl::PipelineDepthStencilState depthState_noTestNoWrite = vgl::PipelineDepthStencilState().setToDefault();
    depthState_noTestNoWrite.bDepthTestEnable = false;
    depthState_noTestNoWrite.bDepthWriteEnable = false;

    vgl::PipelineDepthStencilState depthState_testNoWrite = vgl::PipelineDepthStencilState().setToDefault();
    depthState_testNoWrite.bDepthTestEnable = true;
    depthState_testNoWrite.bDepthWriteEnable = false;
    depthState_testNoWrite.depthCompareOp = VK_COMPARE_OP_LESS;

    // Create the pipelines
    auto createPipeline = [&](
        const VPipelineType pipelineType,
        const vgl::ShaderModule* const * const pShaderModules,
        const vgl::PipelineInputAssemblyState& inputAssemblyState,
        const vgl::PipelineRasterizationState& rasterizerState,
        const vgl::PipelineColorBlendState& colorBlendState,
        const vgl::PipelineDepthStencilState& depthStencilState
    ) noexcept {
        const bool bSuccess = gPipelines[(size_t) pipelineType].initGraphicsPipeline(
            gPipelineLayout,
            renderPass,
            0,
            pShaderModules,
            2,                          // Always just a vertex and fragment shader
            &vertexBindingDesc,
            1,
            vertexAttribs,
            C_ARRAY_SIZE(vertexAttribs),
            inputAssemblyState,
            rasterizerState,
            colorBlendState,
            depthStencilState
        );

        if (!bSuccess)
            FatalErrors::raise("Failed to create a Vulkan graphics pipeline used for rendering!");
    };

    createPipeline(VPipelineType::Lines, shaderModules_colored, lineListInput, rasterizerState_noCull, blendState_alpha, depthState_noTestNoWrite);
    createPipeline(VPipelineType::UI_4bpp, shaderModules_ui_4bpp, triangleListInput, rasterizerState_noCull, blendState_alpha, depthState_noTestNoWrite);
    createPipeline(VPipelineType::UI_8bpp, shaderModules_ui_8bpp, triangleListInput, rasterizerState_noCull, blendState_alpha, depthState_noTestNoWrite);
    createPipeline(VPipelineType::UI_8bpp_Add, shaderModules_ui_8bpp, triangleListInput, rasterizerState_noCull, blendState_additive, depthState_noTestNoWrite);
    createPipeline(VPipelineType::UI_16bpp, shaderModules_ui_16bpp, triangleListInput, rasterizerState_noCull, blendState_alpha, depthState_noTestNoWrite);
    createPipeline(VPipelineType::View_Alpha, shaderModules_view, triangleListInput, rasterizerState_backFaceCull, blendState_alpha, depthState_testNoWrite);
    createPipeline(VPipelineType::View_Additive, shaderModules_view, triangleListInput, rasterizerState_backFaceCull, blendState_additive, depthState_testNoWrite);
    createPipeline(VPipelineType::View_Subtractive, shaderModules_view, triangleListInput, rasterizerState_backFaceCull, blendState_subtractive, depthState_testNoWrite);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down the pipelines module
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    for (vgl::Pipeline& pipeline : gPipelines) {
        pipeline.destroy(true);
    }

    gPipelineLayout.destroy(true);
    gDescriptorSetLayout.destroy(true);
    gSampler.destroy();
    gShader_view_frag.destroy(true);
    gShader_view_vert.destroy(true);
    gShader_ui_16bpp_frag.destroy(true);
    gShader_ui_8bpp_frag.destroy(true);
    gShader_ui_4bpp_frag.destroy(true);
    gShader_ui_vert.destroy(true);
    gShader_colored_frag.destroy(true);
    gShader_colored_vert.destroy(true);
}

END_NAMESPACE(VRPipelines)
