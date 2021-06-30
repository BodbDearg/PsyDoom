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
#include "PsyDoom/Config.h"
#include "Sampler.h"
#include "ShaderModule.h"
#include "VRenderPath_Crossfade.h"
#include "VRenderPath_Main.h"

BEGIN_NAMESPACE(VPipelines)

// The raw SPIRV binary code for the shaders
#include "SPIRV_colored_frag.bin.h"
#include "SPIRV_colored_vert.bin.h"
#include "SPIRV_crossfade_frag.bin.h"
#include "SPIRV_msaa_resolve_frag.bin.h"
#include "SPIRV_msaa_resolve_vert.bin.h"
#include "SPIRV_sky_frag.bin.h"
#include "SPIRV_sky_vert.bin.h"
#include "SPIRV_ndc_textured_frag.bin.h"
#include "SPIRV_ndc_textured_vert.bin.h"
#include "SPIRV_ui_16bpp_frag.bin.h"
#include "SPIRV_ui_4bpp_frag.bin.h"
#include "SPIRV_ui_8bpp_frag.bin.h"
#include "SPIRV_ui_vert.bin.h"
#include "SPIRV_world_frag.bin.h"
#include "SPIRV_world_vert.bin.h"

// Vertex binding descriptions
const VkVertexInputBindingDescription gVertexBindingDesc_draw           = { 0, sizeof(VVertex_Draw), VK_VERTEX_INPUT_RATE_VERTEX };
const VkVertexInputBindingDescription gVertexBindingDesc_msaaResolve    = { 0, sizeof(VVertex_MsaaResolve), VK_VERTEX_INPUT_RATE_VERTEX };
const VkVertexInputBindingDescription gVertexBindingDesc_xyUv           = { 0, sizeof(VVertex_XyUv), VK_VERTEX_INPUT_RATE_VERTEX };

// Vertex attribute descriptions
const VkVertexInputAttributeDescription gVertexAttribs_draw[] = {
    { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VVertex_Draw, x) },
    { 1, 0, VK_FORMAT_R8G8B8A8_UINT,    offsetof(VVertex_Draw, r) },
    { 2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(VVertex_Draw, u) },
    { 3, 0, VK_FORMAT_R16G16_UINT,      offsetof(VVertex_Draw, texWinX) },
    { 4, 0, VK_FORMAT_R16G16_UINT,      offsetof(VVertex_Draw, texWinW) },
    { 5, 0, VK_FORMAT_R16G16_UINT,      offsetof(VVertex_Draw, clutX) },
    { 6, 0, VK_FORMAT_R8G8B8A8_UINT,    offsetof(VVertex_Draw, stmulR) },
};

const VkVertexInputAttributeDescription gVertexAttribs_msaaResolve[] = {
    { 0, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(VVertex_MsaaResolve, x) },
};

const VkVertexInputAttributeDescription gVertexAttribs_xyUv[] = {
    { 0, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(VVertex_XyUv, x) },
    { 1, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(VVertex_XyUv, u) },
};

// Shaders: see the associated source files for more comments/details
static vgl::ShaderModule    gShader_colored_vert;
static vgl::ShaderModule    gShader_colored_frag;
static vgl::ShaderModule    gShader_ui_vert;
static vgl::ShaderModule    gShader_ui_16bpp_frag;
static vgl::ShaderModule    gShader_ui_4bpp_frag;
static vgl::ShaderModule    gShader_ui_8bpp_frag;
static vgl::ShaderModule    gShader_world_vert;
static vgl::ShaderModule    gShader_world_frag;
static vgl::ShaderModule    gShader_sky_vert;
static vgl::ShaderModule    gShader_sky_frag;
static vgl::ShaderModule    gShader_ndc_textured_vert;
static vgl::ShaderModule    gShader_ndc_textured_frag;
static vgl::ShaderModule    gShader_crossfade_frag;
static vgl::ShaderModule    gShader_msaa_resolve_vert;
static vgl::ShaderModule    gShader_msaa_resolve_frag;

// Sets of shader modules
vgl::ShaderModule* const gShaders_colored[]     = { &gShader_colored_vert, &gShader_colored_frag };
vgl::ShaderModule* const gShaders_ui_4bpp[]     = { &gShader_ui_vert, &gShader_ui_4bpp_frag };
vgl::ShaderModule* const gShaders_ui_8bpp[]     = { &gShader_ui_vert, &gShader_ui_8bpp_frag };
vgl::ShaderModule* const gShaders_ui_16bpp[]    = { &gShader_ui_vert, &gShader_ui_16bpp_frag };
vgl::ShaderModule* const gShaders_world[]       = { &gShader_world_vert, &gShader_world_frag };
vgl::ShaderModule* const gShaders_sky[]         = { &gShader_sky_vert, &gShader_sky_frag };
vgl::ShaderModule* const gShaders_ndcTextured[] = { &gShader_ndc_textured_vert, &gShader_ndc_textured_frag };
vgl::ShaderModule* const gShaders_crossfade[]   = { &gShader_ndc_textured_vert, &gShader_crossfade_frag };
vgl::ShaderModule* const gShaders_msaaResolve[] = { &gShader_msaa_resolve_vert, &gShader_msaa_resolve_frag };

// Pipeline samplers
vgl::Sampler gSampler_draw;
vgl::Sampler gSampler_normClampNearest;

// Pipeline descriptor set layouts
vgl::DescriptorSetLayout gDescSetLayout_draw;           // Used by all the normal drawing pipelines
vgl::DescriptorSetLayout gDescSetLayout_msaaResolve;    // Used for MSAA resolve
vgl::DescriptorSetLayout gDescSetLayout_crossfade;      // For drawing crossfades
vgl::DescriptorSetLayout gDescSetLayout_loadingPlaque;  // For drawing loading plaques

// Pipeline layouts
vgl::PipelineLayout gPipelineLayout_draw;               // Used by all the normal drawing pipelines
vgl::PipelineLayout gPipelineLayout_msaaResolve;        // Used for MSAA resolve
vgl::PipelineLayout gPipelineLayout_crossfade;          // For drawing crossfades
vgl::PipelineLayout gPipelineLayout_loadingPlaque;      // For drawing loading plaques

// Pipeline input assembly states
vgl::PipelineInputAssemblyState gInputAS_lineList;      // A list of lines
vgl::PipelineInputAssemblyState gInputAS_triList;       // A list of triangles

// Pipeline rasterization states
vgl::PipelineRasterizationState gRasterState_noCull;
vgl::PipelineRasterizationState gRasterState_backFaceCull;

// Pipeline multisample states
vgl::PipelineMultisampleState gMultisampleState_noMultisample;
vgl::PipelineMultisampleState gMultisampleState_perSettings;
vgl::PipelineMultisampleState gMultisampleState_perSettingsEdgeOnly;    // Same as 'per settings' but don't multi-sample interior triangle texels

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

// Pipeline depth/stencil states
vgl::PipelineDepthStencilState gDepthState_disabled;        // No depth/stencil buffer: Depth write, test and all stencil operations disabled

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
    initShader(device, gShader_world_vert, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_world_vert, sizeof(gSPIRV_world_vert), "world_vert");
    initShader(device, gShader_world_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_world_frag, sizeof(gSPIRV_world_frag), "world_frag");
    initShader(device, gShader_sky_vert, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_sky_vert, sizeof(gSPIRV_sky_vert), "sky_vert");
    initShader(device, gShader_sky_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_sky_frag, sizeof(gSPIRV_sky_frag), "sky_frag");
    initShader(device, gShader_ndc_textured_vert, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_ndc_textured_vert, sizeof(gSPIRV_ndc_textured_vert), "ndc_textured_vert");
    initShader(device, gShader_ndc_textured_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_ndc_textured_frag, sizeof(gSPIRV_ndc_textured_frag), "ndc_textured_frag");
    initShader(device, gShader_crossfade_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_crossfade_frag, sizeof(gSPIRV_crossfade_frag), "crossfade_frag");
    initShader(device, gShader_msaa_resolve_vert, VK_SHADER_STAGE_VERTEX_BIT, gSPIRV_msaa_resolve_vert, sizeof(gSPIRV_msaa_resolve_vert), "msaa_resolve_vert");
    initShader(device, gShader_msaa_resolve_frag, VK_SHADER_STAGE_FRAGMENT_BIT, gSPIRV_msaa_resolve_frag, sizeof(gSPIRV_msaa_resolve_frag), "msaa_resolve_frag");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all samplers
//------------------------------------------------------------------------------------------------------------------------------------------
static void initSamplers(vgl::LogicalDevice& device) noexcept {
    // Sampler used for most drawing operations
    {
        vgl::SamplerSettings settings = vgl::SamplerSettings().setToDefault();
        settings.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.minLod = 0;
        settings.maxLod = 0;
        settings.unnormalizedCoordinates = true;    // Note: Vulkan requires min/max lod to be '0' and clamp to edge to be 'true' if using un-normalized coords

        if (!gSampler_draw.init(device, settings))
            FatalErrors::raise("Failed to init a Vulkan sampler!");
    }

    // A sampler with nearest neighbor filtering, clamp to edge and using normalized coordinates
    {
        vgl::SamplerSettings settings = vgl::SamplerSettings().setToDefault();
        settings.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        settings.minLod = 0;
        settings.maxLod = 0;

        if (!gSampler_normClampNearest.init(device, settings))
            FatalErrors::raise("Failed to init a Vulkan sampler!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all descriptor set layouts.
// Requires all samplers to have been created first and uses immutable samplers baked into the layout for better performance.
//------------------------------------------------------------------------------------------------------------------------------------------
static void initDescriptorSetLayouts(vgl::LogicalDevice& device) noexcept {
    // Regular drawing
    {
         const VkSampler vkSamplers[] = { gSampler_draw.getVkSampler() };

        VkDescriptorSetLayoutBinding bindings[1] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = C_ARRAY_SIZE(vkSamplers);
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0].pImmutableSamplers = vkSamplers;

        if (!gDescSetLayout_draw.init(device, bindings, C_ARRAY_SIZE(bindings)))
            FatalErrors::raise("Failed to init the 'draw' Vulkan descriptor set layout!");
    }

    // MSAA resolve
    {
        VkDescriptorSetLayoutBinding bindings[1] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        if (!gDescSetLayout_msaaResolve.init(device, bindings, C_ARRAY_SIZE(bindings)))
            FatalErrors::raise("Failed to init the 'MSAA resolve' Vulkan descriptor set layout!");
    }

    // Crossfading
    {
        const VkSampler vkSampler = gSampler_normClampNearest.getVkSampler();

        // Note: used to use an array of 2 textures, but MoltenVK didn't like that on MacOS.
        // Use 2 separate texture bindings instead to work around the issue...
        VkDescriptorSetLayoutBinding bindings[2] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0].pImmutableSamplers = &vkSampler;

        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[1].pImmutableSamplers = &vkSampler;

        if (!gDescSetLayout_crossfade.init(device, bindings, C_ARRAY_SIZE(bindings)))
            FatalErrors::raise("Failed to init the 'crossfade' Vulkan descriptor set layout!");
    }

    // Loading plaque drawing
    {
        const VkSampler vkSamplers[] = { gSampler_normClampNearest.getVkSampler() };

        VkDescriptorSetLayoutBinding bindings[1] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = C_ARRAY_SIZE(vkSamplers);
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0].pImmutableSamplers = vkSamplers;

        if (!gDescSetLayout_loadingPlaque.init(device, bindings, C_ARRAY_SIZE(bindings)))
            FatalErrors::raise("Failed to init the 'loading plaque' Vulkan descriptor set layout!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipeline layouts.
// Requires descriptor set layouts to have been created first.
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipelineLayouts(vgl::LogicalDevice& device) noexcept {
    // Regular drawing pipeline layout: uses push constants to set the MVP matrix.
    {
        const VkDescriptorSetLayout vkDescSetLayouts[] = { gDescSetLayout_draw.getVkLayout() };

        VkPushConstantRange uniformPushConstants = {};
        uniformPushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        uniformPushConstants.offset = 0;
        uniformPushConstants.size = sizeof(VShaderUniforms_Draw);

        if (!gPipelineLayout_draw.init(device, vkDescSetLayouts, C_ARRAY_SIZE(vkDescSetLayouts), &uniformPushConstants, 1))
            FatalErrors::raise("Failed to init the 'draw' Vulkan pipeline layout!");
    }

    // MSAA resolve pipeline layout: no push constants needed for this one!
    {
        const VkDescriptorSetLayout vkDescSetLayouts[] = { gDescSetLayout_msaaResolve.getVkLayout() };

        if (!gPipelineLayout_msaaResolve.init(device, vkDescSetLayouts, C_ARRAY_SIZE(vkDescSetLayouts), nullptr, 0))
            FatalErrors::raise("Failed to init the 'msaa resolve' Vulkan pipeline layout!");
    }

    // Draw crossfade pipeline layout: uses push constants to set the lerp factor
    {
        const VkDescriptorSetLayout vkDescSetLayouts[] = { gDescSetLayout_crossfade.getVkLayout() };

        VkPushConstantRange uniformPushConstants = {};
        uniformPushConstants.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        uniformPushConstants.offset = 0;
        uniformPushConstants.size = sizeof(VShaderUniforms_Crossfade);

        if (!gPipelineLayout_crossfade.init(device, vkDescSetLayouts, C_ARRAY_SIZE(vkDescSetLayouts), &uniformPushConstants, 1))
            FatalErrors::raise("Failed to init the 'crossfade' Vulkan pipeline layout!");
    }

    // Draw loading plaque layout: no push constants for this
    {
        const VkDescriptorSetLayout vkDescSetLayouts[] = { gDescSetLayout_loadingPlaque.getVkLayout() };

        if (!gPipelineLayout_loadingPlaque.init(device, vkDescSetLayouts, C_ARRAY_SIZE(vkDescSetLayouts), nullptr, 0))
            FatalErrors::raise("Failed to init the 'loading plaque' Vulkan pipeline layout!");
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
    // This state always has multisampling disabled
    gMultisampleState_noMultisample = vgl::PipelineMultisampleState().setToDefault();

    // This state *may* have a varying amount of multisampling depending on current graphic settings
    gMultisampleState_perSettings = vgl::PipelineMultisampleState().setToDefault();
    gMultisampleState_perSettings.rasterizationSamples = (VkSampleCountFlagBits) numSamples;

    if (device.getPhysicalDevice()->getFeatures().sampleRateShading) {
        // Enable sample rate shading if we can get it for nicer MSAA.
        // Force it to do MSAA for every single fragment to help eliminate texture shimmer and shader aliasing.
        gMultisampleState_perSettings.sampleShadingEnable = true;
        gMultisampleState_perSettings.minSampleShading = 1.0f;
    }

    // This is a variant of the 'per settings' state but without sample rate shading (edge only MSAA).
    // This mode is more appropriate for UI elements since MSAA can blur them slightly.
    gMultisampleState_perSettingsEdgeOnly = vgl::PipelineMultisampleState().setToDefault();
    gMultisampleState_perSettingsEdgeOnly.rasterizationSamples = (VkSampleCountFlagBits) numSamples;
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
    blendAS_blendCommon.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
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
    gDepthState_disabled = vgl::PipelineDepthStencilState().setToDefault();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a single pipeline and raise a fatal error on failure
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPipeline(
    const VPipelineType pipelineType,
    const vgl::RenderPass& renderPass,
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
    VRenderPath_Main& mainRPath,
    const vgl::ShaderModule* const pShaderModules[2],
    const vgl::PipelineInputAssemblyState& inputAssemblyState,
    const vgl::PipelineRasterizationState& rasterizerState,
    const vgl::PipelineColorBlendState& colorBlendState,
    const vgl::PipelineDepthStencilState& depthStencilState,
    const bool bWrapTexture,
    const bool bEdgeOnlyMsaa
) noexcept {
    // Shader specialization constants
    struct ShaderSpecConsts {
        VkBool32 bWrapTexture;          // Whether to do wrapping (true) or clamping (false) when texturing
        VkBool32 bUse16BitShading;      // Whether to use the original PSX 16-bit shading or not
    } shaderSpecConsts;

    shaderSpecConsts.bWrapTexture = bWrapTexture;
    shaderSpecConsts.bUse16BitShading = (!Config::gbUseVulkan32BitShading);

    const VkSpecializationMapEntry specializationMapEntries[] = {
        { 0, offsetof(ShaderSpecConsts, bWrapTexture), sizeof(shaderSpecConsts.bWrapTexture) },
        { 1, offsetof(ShaderSpecConsts, bUse16BitShading), sizeof(shaderSpecConsts.bUse16BitShading) },
    };

    VkSpecializationInfo specializationInfo = {};
    specializationInfo.mapEntryCount = C_ARRAY_SIZE(specializationMapEntries);
    specializationInfo.pMapEntries = specializationMapEntries;
    specializationInfo.dataSize = sizeof(ShaderSpecConsts);
    specializationInfo.pData = &shaderSpecConsts;

    // Create the pipeline
    initPipeline(
        pipelineType,
        mainRPath.getRenderPass(),
        0,
        pShaderModules,
        &specializationInfo,
        gPipelineLayout_draw,
        gVertexBindingDesc_draw,
        gVertexAttribs_draw,
        C_ARRAY_SIZE(gVertexAttribs_draw),
        inputAssemblyState,
        rasterizerState,
        colorBlendState,
        depthStencilState,
        (bEdgeOnlyMsaa) ? gMultisampleState_perSettingsEdgeOnly : gMultisampleState_perSettings
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all of the building blocks that make up pipelines, but not the pipelines themselves.
// These elements have very few dependencies and can be initialized early in order to solve bootstrap dependency issues.
//------------------------------------------------------------------------------------------------------------------------------------------
void initPipelineComponents(vgl::LogicalDevice& device, const uint32_t numSamples) noexcept {
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all pipelines.
// This step must be done after all pipeline components have been created.
//------------------------------------------------------------------------------------------------------------------------------------------
void initPipelines(
    VRenderPath_Main& mainRPath,
    VRenderPath_Crossfade& crossfadeRPath,
    const uint32_t numSamples
) noexcept {
    // Create all of the main drawing pipelines
    initDrawPipeline(VPipelineType::Lines, mainRPath, gShaders_colored, gInputAS_lineList, gRasterState_noCull, gBlendState_noBlend, gDepthState_disabled, false, true);
    initDrawPipeline(VPipelineType::Colored, mainRPath, gShaders_colored, gInputAS_triList, gRasterState_noCull, gBlendState_noBlend, gDepthState_disabled, false, true);
    initDrawPipeline(VPipelineType::UI_4bpp, mainRPath, gShaders_ui_4bpp, gInputAS_triList, gRasterState_noCull, gBlendState_noBlend, gDepthState_disabled, false, true);
    initDrawPipeline(VPipelineType::UI_8bpp, mainRPath, gShaders_ui_8bpp, gInputAS_triList, gRasterState_noCull, gBlendState_noBlend, gDepthState_disabled, false, true);
    initDrawPipeline(VPipelineType::UI_8bpp_Add, mainRPath, gShaders_ui_8bpp, gInputAS_triList, gRasterState_noCull, gBlendState_additive, gDepthState_disabled, false, true);
    initDrawPipeline(VPipelineType::UI_16bpp, mainRPath, gShaders_ui_16bpp, gInputAS_triList, gRasterState_noCull, gBlendState_noBlend, gDepthState_disabled, false, true);
    initDrawPipeline(VPipelineType::World_GeomMasked, mainRPath, gShaders_world, gInputAS_triList, gRasterState_backFaceCull, gBlendState_noBlend, gDepthState_disabled, true, false);
    initDrawPipeline(VPipelineType::World_GeomAlpha, mainRPath, gShaders_world, gInputAS_triList, gRasterState_backFaceCull, gBlendState_alpha, gDepthState_disabled, true, false);
    initDrawPipeline(VPipelineType::World_SpriteMasked, mainRPath, gShaders_world, gInputAS_triList, gRasterState_noCull, gBlendState_noBlend, gDepthState_disabled, false, false);
    initDrawPipeline(VPipelineType::World_SpriteAlpha, mainRPath, gShaders_world, gInputAS_triList, gRasterState_noCull, gBlendState_alpha, gDepthState_disabled, false, false);
    initDrawPipeline(VPipelineType::World_SpriteAdditive, mainRPath, gShaders_world, gInputAS_triList, gRasterState_noCull, gBlendState_additive, gDepthState_disabled, false, false);
    initDrawPipeline(VPipelineType::World_SpriteSubtractive, mainRPath, gShaders_world, gInputAS_triList, gRasterState_noCull, gBlendState_subtractive, gDepthState_disabled, false, false);
    initDrawPipeline(VPipelineType::World_Sky, mainRPath, gShaders_sky, gInputAS_triList, gRasterState_backFaceCull, gBlendState_noBlend, gDepthState_disabled, true, true);

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
            VPipelineType::Msaa_Resolve, mainRPath.getRenderPass(), 1,
            gShaders_msaaResolve, &specializationInfo, gPipelineLayout_msaaResolve,
            gVertexBindingDesc_msaaResolve, gVertexAttribs_msaaResolve, C_ARRAY_SIZE(gVertexAttribs_msaaResolve),
            gInputAS_triList, gRasterState_noCull,
            gBlendState_noBlend, gDepthState_disabled, gMultisampleState_noMultisample
        );
    }

    // A pipeline to use during crossfading
    {
        const VkSpecializationMapEntry specializationMapEntries[] = {
            { 0, 0, sizeof(VkBool32) }
        };

        const VkBool32 bShade16Bit = (!Config::gbUseVulkan32BitShading);

        VkSpecializationInfo specializationInfo = {};
        specializationInfo.mapEntryCount = C_ARRAY_SIZE(specializationMapEntries);
        specializationInfo.pMapEntries = specializationMapEntries;
        specializationInfo.dataSize = sizeof(VkBool32);
        specializationInfo.pData = &bShade16Bit;

        initPipeline(
            VPipelineType::Crossfade, crossfadeRPath.getRenderPass(), 0,
            gShaders_crossfade, &specializationInfo, gPipelineLayout_crossfade,
            gVertexBindingDesc_xyUv, gVertexAttribs_xyUv, C_ARRAY_SIZE(gVertexAttribs_xyUv),
            gInputAS_triList, gRasterState_noCull,
            gBlendState_noBlend, gDepthState_disabled, gMultisampleState_noMultisample
        );
    }

    // Used to draw loading plaques, both the background and the plaque itself
    initPipeline(
        VPipelineType::LoadingPlaque, mainRPath.getRenderPass(), 0,
        gShaders_ndcTextured, nullptr, gPipelineLayout_loadingPlaque,
        gVertexBindingDesc_xyUv, gVertexAttribs_xyUv, C_ARRAY_SIZE(gVertexAttribs_xyUv),
        gInputAS_triList, gRasterState_noCull,
        gBlendState_noBlend, gDepthState_disabled, gMultisampleState_perSettingsEdgeOnly
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys all pipelines and their components
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    for (vgl::Pipeline& pipeline : gPipelines) {
        pipeline.destroy(true);
    }

    gPipelineLayout_loadingPlaque.destroy(true);
    gPipelineLayout_crossfade.destroy(true);
    gPipelineLayout_msaaResolve.destroy(true);
    gPipelineLayout_draw.destroy(true);

    gDescSetLayout_loadingPlaque.destroy(true);
    gDescSetLayout_crossfade.destroy(true);
    gDescSetLayout_msaaResolve.destroy(true);
    gDescSetLayout_draw.destroy(true);

    gSampler_normClampNearest.destroy();
    gSampler_draw.destroy();

    gShader_msaa_resolve_frag.destroy(true);
    gShader_msaa_resolve_vert.destroy(true);
    gShader_crossfade_frag.destroy(true);
    gShader_ndc_textured_frag.destroy(true);
    gShader_ndc_textured_vert.destroy(true);
    gShader_sky_frag.destroy(true);
    gShader_sky_vert.destroy(true);
    gShader_world_frag.destroy(true);
    gShader_world_vert.destroy(true);
    gShader_ui_16bpp_frag.destroy(true);
    gShader_ui_8bpp_frag.destroy(true);
    gShader_ui_4bpp_frag.destroy(true);
    gShader_ui_vert.destroy(true);
    gShader_colored_frag.destroy(true);
    gShader_colored_vert.destroy(true);
}

END_NAMESPACE(VRPipelines)

#endif  // #if PSYDOOM_VULKAN_RENDERER
