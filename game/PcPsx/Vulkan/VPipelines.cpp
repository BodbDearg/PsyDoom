//------------------------------------------------------------------------------------------------------------------------------------------
// Logic for creating the various shaders and pipelines used by the new Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VPipelines.h"

#if PSYDOOM_VULKAN_RENDERER

#include "DescriptorSetLayout.h"
#include "FatalErrors.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "Sampler.h"
#include "ShaderModule.h"

BEGIN_NAMESPACE(VPipelines)

// The raw SPIRV binary code for the shaders
#include "SPIRV_colored_frag.bin.h"
#include "SPIRV_colored_vert.bin.h"
#include "SPIRV_msaa_resolve_frag.bin.h"
#include "SPIRV_msaa_resolve_vert.bin.h"
#include "SPIRV_ui_16bpp_frag.bin.h"
#include "SPIRV_ui_4bpp_frag.bin.h"
#include "SPIRV_ui_8bpp_frag.bin.h"
#include "SPIRV_ui_vert.bin.h"
#include "SPIRV_view_frag.bin.h"
#include "SPIRV_view_vert.bin.h"

// Vertex binding descriptions
const VkVertexInputBindingDescription gVertexBindingDesc_drawing        = { 0, sizeof(VVertex), VK_VERTEX_INPUT_RATE_VERTEX };
const VkVertexInputBindingDescription gVertexBindingDesc_msaaResolve    = { 0, sizeof(VVertex_MsaaResolve), VK_VERTEX_INPUT_RATE_VERTEX };

// Vertex attribute descriptions
const VkVertexInputAttributeDescription gVertexAttribs_drawing[] = {
    { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VVertex, x) },
    { 1, 0, VK_FORMAT_R8G8B8_USCALED,   offsetof(VVertex, r) },
    { 2, 0, VK_FORMAT_R8_UINT,          offsetof(VVertex, lightDimMode) },
    { 3, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(VVertex, u) },
    { 4, 0, VK_FORMAT_R16G16_UINT,      offsetof(VVertex, texWinX) },
    { 5, 0, VK_FORMAT_R16G16_UINT,      offsetof(VVertex, texWinW) },
    { 6, 0, VK_FORMAT_R16G16_UINT,      offsetof(VVertex, clutX) },
    { 7, 0, VK_FORMAT_R8G8B8A8_USCALED, offsetof(VVertex, stmulR) },
};

const VkVertexInputAttributeDescription gVertexAttribs_msaaResolve[] = {
    { 0, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(VVertex_MsaaResolve, x) },
};

// Shaders: see the associated source files for more comments/details
static vgl::ShaderModule    gShader_colored_frag;
static vgl::ShaderModule    gShader_colored_vert;
static vgl::ShaderModule    gShader_ui_16bpp_frag;
static vgl::ShaderModule    gShader_ui_4bpp_frag;
static vgl::ShaderModule    gShader_ui_8bpp_frag;
static vgl::ShaderModule    gShader_ui_vert;
static vgl::ShaderModule    gShader_view_frag;
static vgl::ShaderModule    gShader_view_vert;
static vgl::ShaderModule    gShader_msaa_resolve_frag;
static vgl::ShaderModule    gShader_msaa_resolve_vert;

// Sets of shader modules
vgl::ShaderModule* const gShaders_colored[]       = { &gShader_colored_vert, &gShader_colored_frag };
vgl::ShaderModule* const gShaders_ui_4bpp[]       = { &gShader_ui_vert, &gShader_ui_4bpp_frag };
vgl::ShaderModule* const gShaders_ui_8bpp[]       = { &gShader_ui_vert, &gShader_ui_8bpp_frag };
vgl::ShaderModule* const gShaders_ui_16bpp[]      = { &gShader_ui_vert, &gShader_ui_16bpp_frag };
vgl::ShaderModule* const gShaders_view[]          = { &gShader_view_vert, &gShader_view_frag };
vgl::ShaderModule* const gShaders_msaaResolve[]   = { &gShader_msaa_resolve_vert, &gShader_msaa_resolve_frag };

// Pipeline samplers
vgl::Sampler gSampler_drawing;

// Pipeline descriptor set layouts: for normal drawing and MSAA resolve
vgl::DescriptorSetLayout gDescSetLayout_drawing;
vgl::DescriptorSetLayout gDescSetLayout_msaaResolve;

// Pipeline layouts
vgl::PipelineLayout gPipelineLayout_drawing;        // Used by all the normal drawing pipelines
vgl::PipelineLayout gPipelineLayout_msaaResolve;    // Used for MSAA resolve

// Pipeline input assembly states
vgl::PipelineInputAssemblyState gInputAS_lineList;      // A list of lines
vgl::PipelineInputAssemblyState gInputAS_triList;       // A list of triangles

// Pipeline rasterization states
vgl::PipelineRasterizationState gRasterState_noCull;
vgl::PipelineRasterizationState gRasterState_backFaceCull;

// Pipeline multisample states
vgl::PipelineMultisampleState gMultisampleState_noMultisample;
vgl::PipelineMultisampleState gMultisampleState_drawing;

// Pipeline color blend per-attachment states
VkPipelineColorBlendAttachmentState gBlendAS_noBlend;           // No blending
VkPipelineColorBlendAttachmentState gBlendAS_alpha;             // Regular alpha blending
VkPipelineColorBlendAttachmentState gBlendAS_additive;          // Additive blending
VkPipelineColorBlendAttachmentState gBlendAS_subtractive;       // Subtractive blending

// Pipeline color blend state for all attachments: note that all these assume blending ONLY on attachment at index '0'!
vgl::PipelineColorBlendState gBlendState_noBlend;
vgl::PipelineColorBlendState gBlendState_alpha;
vgl::PipelineColorBlendState gBlendState_additive;
vgl::PipelineColorBlendState gBlendState_subtractive;

// Pipeline depth stencil states
vgl::PipelineDepthStencilState gDepthState_noTestNoWrite;       // No depth test and no depth write, stencil disabled
vgl::PipelineDepthStencilState gDepthState_testNoWrite;         // No depth write but do a depth test, stencil disabled
vgl::PipelineDepthStencilState gDepthState_testAndWrite;        // Both depth test and depth write, stencil disabled

// The pipelines themselves
vgl::Pipeline gPipelines[(size_t) VPipelineType::NUM_TYPES];

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a single shader and raise a fatal error if it fails
//-----------------------------------------------------------------------------------------------------------------------------------------
static void initShader(
    vgl::LogicalDevice& device,
    vgl::ShaderModule& shader,
    const VkShaderStageFlagBits stageFlags,
    const uint32_t* const pCode,
    const uint32_t codeSize,
    const char* const shaderName
) noexcept {
    if (!shader.init(device, stageFlags, pCode, codeSize))
        FatalErrors::raiseF("Failed to init Vulkan shader '%s'!", shaderName);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all shaders
//------------------------------------------------------------------------------------------------------------------------------------------
static void initShaders(vgl::LogicalDevice& device) noexcept {
    initShader(device, gShader_colored_vert, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_colored_vert, sizeof(gSPIRV_colored_vert), "colored_vert");
    initShader(device, gShader_colored_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_colored_frag, sizeof(gSPIRV_colored_frag), "colored_frag");
    initShader(device, gShader_ui_vert, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_ui_vert, sizeof(gSPIRV_ui_vert), "ui_vert");
    initShader(device, gShader_ui_4bpp_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_ui_4bpp_frag, sizeof(gSPIRV_ui_4bpp_frag), "ui_4bpp_frag");
    initShader(device, gShader_ui_8bpp_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_ui_8bpp_frag, sizeof(gSPIRV_ui_8bpp_frag), "ui_8bpp_frag");
    initShader(device, gShader_ui_16bpp_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_ui_16bpp_frag, sizeof(gSPIRV_ui_16bpp_frag), "ui_16bpp_frag");
    initShader(device, gShader_view_vert, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_view_vert, sizeof(gSPIRV_view_vert), "view_vert");
    initShader(device, gShader_view_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_view_frag, sizeof(gSPIRV_view_frag), "view_frag");
    initShader(device, gShader_msaa_resolve_vert, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_msaa_resolve_vert, sizeof(gSPIRV_msaa_resolve_vert), "msaa_resolve_vert");
    initShader(device, gShader_msaa_resolve_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_msaa_resolve_frag, sizeof(gSPIRV_msaa_resolve_frag), "msaa_resolve_frag");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all samplers
//------------------------------------------------------------------------------------------------------------------------------------------
static void initSamplers(vgl::LogicalDevice& device) noexcept {
    // Just using a single sampler for now
    vgl::SamplerSettings settings = vgl::SamplerSettings().setToDefault();
    settings.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    settings.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    settings.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    settings.minLod = 0;
    settings.maxLod = 0;
    settings.unnormalizedCoordinates = true;    // Note: Vulkan requires min/max lod to be '0' and clamp to edge to be 'true' if using un-normalized coords

    if (!gSampler_drawing.init(device, settings))
        FatalErrors::raise("Failed to init a Vulkan sampler!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all descriptor set layouts.
// Requires all samplers to have been created first and uses immutable samplers baked into the layout for better performance.
//------------------------------------------------------------------------------------------------------------------------------------------
static void initDescriptorSetLayouts(vgl::LogicalDevice& device) noexcept {
    // Regular drawing
    {
        const VkSampler vkSampler = gSampler_drawing.getVkSampler();

        VkDescriptorSetLayoutBinding samplerBinding = {};
        samplerBinding.binding = 0;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;    // Only reading the texture in fragment shaders
        samplerBinding.pImmutableSamplers = &vkSampler;

        if (!gDescSetLayout_drawing.init(device, &samplerBinding, 1))
            FatalErrors::raise("Failed to init a Vulkan descriptor set layout!");
    }

    // MSAA resolve
    {
        VkDescriptorSetLayoutBinding samplerBinding = {};
        samplerBinding.binding = 0;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        samplerBinding.descriptorCount = 1;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;    // This descriptor type can only be used in fragment shaders

        if (!gDescSetLayout_msaaResolve.init(device, &samplerBinding, 1))
            FatalErrors::raise("Failed to init a Vulkan descriptor set layout!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipeline layouts.
// Requires descriptor set layouts to have been created first.
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipelineLayouts(vgl::LogicalDevice& device) noexcept {
    // Drawing pipeline layout: uses push constants to set the MVP matrix.
    {
        VkDescriptorSetLayout vkDescSetLayout = gDescSetLayout_drawing.getVkLayout();

        VkPushConstantRange uniformPushConstants = {};
        uniformPushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;   // Just needed in the vertex shader
        uniformPushConstants.offset = 0;
        uniformPushConstants.size = sizeof(VShaderUniforms);

        if (!gPipelineLayout_drawing.init(device, &vkDescSetLayout, 1, &uniformPushConstants, 1))
            FatalErrors::raise("Failed to init the 'drawing' Vulkan pipeline layout!");
    }

    // MSAA resolve pipeline layout: no push constants needed for this one!
    {
        VkDescriptorSetLayout vkDescSetLayout = gDescSetLayout_msaaResolve.getVkLayout();

        if (!gPipelineLayout_msaaResolve.init(device, &vkDescSetLayout, 1, nullptr, 0))
            FatalErrors::raise("Failed to init the 'msaa resolve' Vulkan pipeline layout!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipeline input assembly states
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipelineInputAssemblyStates() noexcept {
    gInputAS_lineList = vgl::PipelineInputAssemblyState().setToDefault();
    gInputAS_lineList.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    gInputAS_triList = vgl::PipelineInputAssemblyState().setToDefault();
    gInputAS_triList.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipeline rasterization states
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipelineRasterizationStates() noexcept {
    gRasterState_noCull = vgl::PipelineRasterizationState().setToDefault();

    gRasterState_backFaceCull = vgl::PipelineRasterizationState().setToDefault();
    gRasterState_backFaceCull.cullMode = VK_CULL_MODE_BACK_BIT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipeline mulitsample states
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipelineMultisampleStates(vgl::LogicalDevice& device, const uint32_t numSamples) noexcept {
    gMultisampleState_noMultisample = vgl::PipelineMultisampleState().setToDefault();

    // Drawing can use whatever amount of multisamples have been configured
    gMultisampleState_drawing = vgl::PipelineMultisampleState().setToDefault();
    gMultisampleState_drawing.rasterizationSamples = (VkSampleCountFlagBits) numSamples;

    if (device.getPhysicalDevice()->getFeatures().sampleRateShading) {
        // Enable sample rate shading if we can get it for nicer MSAA.
        // Force it to do MSAA for every single fragment to help eliminate texture shimmer and shader aliasing.
        gMultisampleState_drawing.sampleShadingEnable = true;
        gMultisampleState_drawing.minSampleShading = 1.0f;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipeline color blend attachment states
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipelineColorBlendAttachmentStates() noexcept {
    // No blending
    gBlendAS_noBlend = {};
    gBlendAS_noBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    gBlendAS_noBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    gBlendAS_noBlend.colorWriteMask = (
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT    // Don't need alpha writes but it's probably more efficient to write 32-bits at a time
    );

    // Common state for when we are blending
    VkPipelineColorBlendAttachmentState blendAS_blendCommon = {};
    blendAS_blendCommon.blendEnable = VK_TRUE;
    blendAS_blendCommon.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAS_blendCommon.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAS_blendCommon.alphaBlendOp = VK_BLEND_OP_ADD;
    blendAS_blendCommon.colorWriteMask = (
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT    // Don't need alpha writes but it's probably more efficient to write 32-bits at a time
    );

    // Blending modes
    gBlendAS_alpha = blendAS_blendCommon;
    gBlendAS_alpha.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    gBlendAS_alpha.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    gBlendAS_alpha.colorBlendOp = VK_BLEND_OP_ADD;

    gBlendAS_additive = blendAS_blendCommon;
    gBlendAS_additive.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    gBlendAS_additive.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    gBlendAS_additive.colorBlendOp = VK_BLEND_OP_ADD;

    gBlendAS_subtractive = blendAS_blendCommon;
    gBlendAS_subtractive.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    gBlendAS_subtractive.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    gBlendAS_subtractive.colorBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipeline color blend states
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipelineColorBlendStates() noexcept {
    // Note that all these assume blending ONLY on attachment at index '0'!
    gBlendState_noBlend = vgl::PipelineColorBlendState().setToDefault();
    gBlendState_noBlend.pAttachments = &gBlendAS_noBlend;
    gBlendState_noBlend.attachmentCount = 1;

    gBlendState_alpha = vgl::PipelineColorBlendState().setToDefault();
    gBlendState_alpha.pAttachments = &gBlendAS_alpha;
    gBlendState_alpha.attachmentCount = 1;

    gBlendState_additive = vgl::PipelineColorBlendState().setToDefault();
    gBlendState_additive.pAttachments = &gBlendAS_additive;
    gBlendState_additive.attachmentCount = 1;

    gBlendState_subtractive = vgl::PipelineColorBlendState().setToDefault();
    gBlendState_subtractive.pAttachments = &gBlendAS_subtractive;
    gBlendState_subtractive.attachmentCount = 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipeline depth stencil states
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipelineDepthStencilStates() noexcept {
    gDepthState_noTestNoWrite = vgl::PipelineDepthStencilState().setToDefault();
    gDepthState_noTestNoWrite.bDepthTestEnable = false;
    gDepthState_noTestNoWrite.bDepthWriteEnable = false;

    gDepthState_testNoWrite = vgl::PipelineDepthStencilState().setToDefault();
    gDepthState_testNoWrite.bDepthTestEnable = true;
    gDepthState_testNoWrite.bDepthWriteEnable = false;
    gDepthState_testNoWrite.depthCompareOp = VK_COMPARE_OP_LESS;

    gDepthState_testAndWrite = vgl::PipelineDepthStencilState().setToDefault();
    gDepthState_testAndWrite.bDepthTestEnable = true;
    gDepthState_testAndWrite.bDepthWriteEnable = true;
    gDepthState_testAndWrite.depthCompareOp = VK_COMPARE_OP_LESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a single pipeline and raise a fatal error on failure
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipeline(
    const VPipelineType pipelineType,
    const vgl::BaseRenderPass& renderPass,
    const uint32_t subpassIdx,
    const vgl::ShaderModule* const pShaderModules[2],
    const VkSpecializationInfo* const pShaderSpecializationInfo,
    const vgl::PipelineLayout& pipelineLayout,
    const VkVertexInputBindingDescription& vertexBindingDesc,
    const VkVertexInputAttributeDescription* const pVertexAttribs,
    const uint32_t numVertexAttribs,
    const vgl::PipelineInputAssemblyState& inputAssemblyState,
    const vgl::PipelineRasterizationState& rasterizerState,
    const vgl::PipelineColorBlendState& colorBlendState,
    const vgl::PipelineDepthStencilState& depthStencilState,
    const vgl::PipelineMultisampleState& multisampleState
) noexcept {
    const bool bSuccess = gPipelines[(size_t) pipelineType].initGraphicsPipeline(
        pipelineLayout,
        renderPass,
        subpassIdx,
        pShaderModules,
        2,                          // For this project always using just a vertex and fragment shader
        pShaderSpecializationInfo,
        &vertexBindingDesc,
        1,                          // For this project always a single input stream of vertices
        pVertexAttribs,
        numVertexAttribs,
        inputAssemblyState,
        rasterizerState,
        multisampleState,
        colorBlendState,
        depthStencilState
    );

    if (!bSuccess)
        FatalErrors::raise("Failed to create a Vulkan graphics pipeline used for rendering!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convenience function: initialize a pipeline used for normal drawing and fill in a few parameters that are common to all draw pipelines
//------------------------------------------------------------------------------------------------------------------------------------------
static void initDrawPipeline(
    const VPipelineType pipelineType,
    const vgl::BaseRenderPass& renderPass,
    const vgl::ShaderModule* const pShaderModules[2],
    const vgl::PipelineInputAssemblyState& inputAssemblyState,
    const vgl::PipelineRasterizationState& rasterizerState,
    const vgl::PipelineColorBlendState& colorBlendState,
    const vgl::PipelineDepthStencilState& depthStencilState
) noexcept {
    initPipeline(
        pipelineType,
        renderPass,
        0,
        pShaderModules,
        nullptr,
        gPipelineLayout_drawing,
        gVertexBindingDesc_drawing,
        gVertexAttribs_drawing,
        C_ARRAY_SIZE(gVertexAttribs_drawing),
        inputAssemblyState,
        rasterizerState,
        colorBlendState,
        depthStencilState,
        gMultisampleState_drawing
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the pipelines module
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device, vgl::BaseRenderPass& renderPass, const uint32_t numSamples) noexcept {
    // Create all pipeline creation inputs and states
    initShaders(device);
    initSamplers(device);
    initDescriptorSetLayouts(device);
    initPipelineLayouts(device);
    initPipelineInputAssemblyStates();
    initPipelineRasterizationStates();
    initPipelineMultisampleStates(device, numSamples);
    initPipelineColorBlendAttachmentStates();
    initPipelineColorBlendStates();
    initPipelineDepthStencilStates();

    // Create all of the main drawing pipelines
    initDrawPipeline(VPipelineType::Lines, renderPass, gShaders_colored, gInputAS_lineList, gRasterState_noCull, gBlendState_alpha, gDepthState_noTestNoWrite);
    initDrawPipeline(VPipelineType::UI_4bpp, renderPass, gShaders_ui_4bpp, gInputAS_triList, gRasterState_noCull, gBlendState_alpha, gDepthState_noTestNoWrite);
    initDrawPipeline(VPipelineType::UI_8bpp, renderPass, gShaders_ui_8bpp, gInputAS_triList, gRasterState_noCull, gBlendState_alpha, gDepthState_noTestNoWrite);
    initDrawPipeline(VPipelineType::UI_8bpp_Add, renderPass, gShaders_ui_8bpp, gInputAS_triList, gRasterState_noCull, gBlendState_additive, gDepthState_noTestNoWrite);
    initDrawPipeline(VPipelineType::UI_16bpp, renderPass, gShaders_ui_16bpp, gInputAS_triList, gRasterState_noCull, gBlendState_alpha, gDepthState_noTestNoWrite);
    initDrawPipeline(VPipelineType::View_Alpha, renderPass, gShaders_view, gInputAS_triList, gRasterState_backFaceCull, gBlendState_alpha, gDepthState_testNoWrite);
    initDrawPipeline(VPipelineType::View_Additive, renderPass, gShaders_view, gInputAS_triList, gRasterState_backFaceCull, gBlendState_additive, gDepthState_testNoWrite);
    initDrawPipeline(VPipelineType::View_Subtractive, renderPass, gShaders_view, gInputAS_triList, gRasterState_backFaceCull, gBlendState_subtractive, gDepthState_testNoWrite);

    // The pipeline to resolve MSAA: only bother creating this if we are doing MSAA.
    // Specialize the shader to the number of samples also, so that loops can be unrolled.
    if (numSamples > 1) {
        const VkSpecializationMapEntry specializationMapEntries[] = {
            { 0, 0, sizeof(uint32_t) }
        };

        VkSpecializationInfo specializationInfo = {};
        specializationInfo.mapEntryCount = C_ARRAY_SIZE(specializationMapEntries);
        specializationInfo.pMapEntries = specializationMapEntries;
        specializationInfo.dataSize = sizeof(uint32_t);
        specializationInfo.pData = &numSamples;

        initPipeline(
            VPipelineType::Msaa_Resolve, renderPass, 1,
            gShaders_msaaResolve, &specializationInfo, gPipelineLayout_msaaResolve,
            gVertexBindingDesc_msaaResolve, gVertexAttribs_msaaResolve, C_ARRAY_SIZE(gVertexAttribs_msaaResolve),
            gInputAS_triList, gRasterState_noCull,
            gBlendState_noBlend, gDepthState_noTestNoWrite, gMultisampleState_noMultisample
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down the pipelines module
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    for (vgl::Pipeline& pipeline : gPipelines) {
        pipeline.destroy(true);
    }

    gPipelineLayout_msaaResolve.destroy(true);
    gPipelineLayout_drawing.destroy(true);

    gDescSetLayout_msaaResolve.destroy(true);
    gDescSetLayout_drawing.destroy(true);

    gSampler_drawing.destroy();

    gShader_msaa_resolve_frag.destroy(true);
    gShader_msaa_resolve_vert.destroy(true);
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

#endif  // #if PSYDOOM_VULKAN_RENDERER
