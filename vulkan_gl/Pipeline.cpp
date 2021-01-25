#include "Pipeline.h"

#include "BaseRenderPass.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "PipelineLayout.h"
#include "RetirementMgr.h"
#include "ShaderModule.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized pipeline
//------------------------------------------------------------------------------------------------------------------------------------------
Pipeline::Pipeline() noexcept
    : mbIsValid(false)
    , mbIsComputePipeline(false)
    , mpDevice(nullptr)
    , mVkPipeline(VK_NULL_HANDLE)
    , mVkPipelineLayout(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a pipeline to this object
//------------------------------------------------------------------------------------------------------------------------------------------
Pipeline::Pipeline(Pipeline&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mbIsComputePipeline(other.mbIsComputePipeline)
    , mpDevice(other.mpDevice)
    , mVkPipeline(other.mVkPipeline)
    , mVkPipelineLayout(other.mVkPipelineLayout)
{
    other.mbIsValid = false;
    other.mbIsComputePipeline = false;
    other.mpDevice = nullptr;
    other.mVkPipeline = VK_NULL_HANDLE;
    other.mVkPipelineLayout = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the pipeline
//------------------------------------------------------------------------------------------------------------------------------------------
Pipeline::~Pipeline() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a graphics pipeline with the specified settings and state
//------------------------------------------------------------------------------------------------------------------------------------------
bool Pipeline::initGraphicsPipeline(
    const PipelineLayout& pipelineLayout,
    const BaseRenderPass& renderPass,
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
) noexcept {
    //------------------------------------------------------------------------------------------------------------------
    // The basics
    //------------------------------------------------------------------------------------------------------------------

    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(pipelineLayout.isValid());
    ASSERT(renderPass.isValid());
    ASSERT(subpassIndex < renderPass.getNumSubpasses());
    ASSERT(numShaderModules >= Defines::MIN_ACTIVE_GRAPHICS_SHADERS);
    ASSERT(numShaderModules <= Defines::MAX_ACTIVE_GRAPHICS_SHADERS);
    ASSERT(ppShaderModules);
    ASSERT(pVertexInputBindingDescs);
    ASSERT(numVertexInputBindingDescs > 0);
    ASSERT(pVertexInputAttribDescs);
    ASSERT(numVertexInputAttribDescs > 0);

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true, true);
        }
    });

    // Saving some of the basics
    LogicalDevice& device = *renderPass.getDevice();
    ASSERT(device.isValid());

    mbIsComputePipeline = false;
    mpDevice = &device;
    mVkPipelineLayout = pipelineLayout.getVkPipelineLayout();

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline settings: shaders
    //------------------------------------------------------------------------------------------------------------------
    VkPipelineShaderStageCreateInfo shaderStageCIs[Defines::MAX_ACTIVE_GRAPHICS_SHADERS];

    if (numShaderModules > Defines::MAX_ACTIVE_GRAPHICS_SHADERS) {
        ASSERT_FAIL("Too many shaders specified!");
        return false;
    }

    {
        const ShaderModule* const* ppCurShaderModule = ppShaderModules;
        const ShaderModule* const* const ppEndShaderModule = ppShaderModules + numShaderModules;

        VkPipelineShaderStageCreateInfo* pCurShaderStageCI = shaderStageCIs;

        while (ppCurShaderModule < ppEndShaderModule) {
            // Note: eventually we might want a mechanism to override the shader entry point from 'main()' here!
            const ShaderModule* const pCurShaderModule = *ppCurShaderModule;
            ASSERT(pCurShaderModule->isValid());

            *pCurShaderStageCI = {};
            pCurShaderStageCI->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            pCurShaderStageCI->stage = pCurShaderModule->getVkShaderStageFlagBits();
            pCurShaderStageCI->module = pCurShaderModule->getVkShaderModule();
            pCurShaderStageCI->pName = "main";
            pCurShaderStageCI->pSpecializationInfo = pShaderSpecializationInfo;     // Note: forcing the same specialization info to be used by all shader stages

            ++ppCurShaderModule;
            ++pCurShaderStageCI;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline settings: vertex input stage
    //------------------------------------------------------------------------------------------------------------------
    VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {};
    vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCI.vertexBindingDescriptionCount = numVertexInputBindingDescs;
    vertexInputStateCI.pVertexBindingDescriptions = pVertexInputBindingDescs;
    vertexInputStateCI.vertexAttributeDescriptionCount = numVertexInputAttribDescs;
    vertexInputStateCI.pVertexAttributeDescriptions = pVertexInputAttribDescs;

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline settings: input assembly
    //------------------------------------------------------------------------------------------------------------------
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = {};
    inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCI.topology = inputAssemblyState.topology;
    inputAssemblyStateCI.primitiveRestartEnable = inputAssemblyState.bPrimitiveRestartEnable;

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline settings: viewport
    //------------------------------------------------------------------------------------------------------------------

    // Most of this state is specified dynamically, apart from viewport and scissor count.
    // Hardcoding those settings to '1' for now; if multiple viewports are ever needed then this will need to be revisited!
    VkPipelineViewportStateCreateInfo viewportStateCI = {};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCI.viewportCount = 1;
    viewportStateCI.pViewports = nullptr;
    viewportStateCI.scissorCount = 1;
    viewportStateCI.pScissors = nullptr;

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline settings: Rasterizer
    //------------------------------------------------------------------------------------------------------------------
    VkPipelineRasterizationStateCreateInfo rasterizerStateCI = {};
    rasterizerStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateCI.depthClampEnable = rasterizationState.bDepthClampEnable;
    rasterizerStateCI.rasterizerDiscardEnable = rasterizationState.bRasterizerDiscardEnable;
    rasterizerStateCI.polygonMode = rasterizationState.polygonMode;
    rasterizerStateCI.cullMode = rasterizationState.cullMode;
    rasterizerStateCI.frontFace = rasterizationState.frontFace;
    rasterizerStateCI.depthBiasEnable = rasterizationState.bDepthBiasEnable;
    rasterizerStateCI.depthBiasConstantFactor = rasterizationState.depthBiasConstantFactor;
    rasterizerStateCI.depthBiasClamp = rasterizationState.depthBiasClamp;
    rasterizerStateCI.depthBiasSlopeFactor = rasterizationState.depthBiasSlopeFactor;
    rasterizerStateCI.lineWidth = rasterizationState.lineWidth;

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline settings: Multisampling
    //------------------------------------------------------------------------------------------------------------------
    VkPipelineMultisampleStateCreateInfo multisampleStateCI = {};
    multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCI.rasterizationSamples = multisampleState.rasterizationSamples;
    multisampleStateCI.sampleShadingEnable =  multisampleState.sampleShadingEnable;
    multisampleStateCI.minSampleShading = multisampleState.minSampleShading;
    multisampleStateCI.pSampleMask = multisampleState.pSampleMask;
    multisampleStateCI.alphaToCoverageEnable = multisampleState.alphaToCoverageEnable;
    multisampleStateCI.alphaToOneEnable = multisampleState.alphaToOneEnable;

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline Settings: Color blend state
    //------------------------------------------------------------------------------------------------------------------
    VkPipelineColorBlendStateCreateInfo colorBlendStateCI = {};
    colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCI.logicOpEnable = colorBlendState.bLogicOpEnable;
    colorBlendStateCI.logicOp = colorBlendState.logicOp;
    colorBlendStateCI.attachmentCount = colorBlendState.attachmentCount;
    colorBlendStateCI.pAttachments = colorBlendState.pAttachments;
    colorBlendStateCI.blendConstants[0] = colorBlendState.blendConstants[0];
    colorBlendStateCI.blendConstants[1] = colorBlendState.blendConstants[1];
    colorBlendStateCI.blendConstants[2] = colorBlendState.blendConstants[2];
    colorBlendStateCI.blendConstants[3] = colorBlendState.blendConstants[3];

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline settings: depth and stencil testing
    //------------------------------------------------------------------------------------------------------------------

    // Note: for now not using or setting any stencil test state.
    // Also not exposing the depth bounds test; can revisit and add later if required!
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = {};
    depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateCI.depthTestEnable = depthStencilState.bDepthTestEnable;
    depthStencilStateCI.depthWriteEnable = depthStencilState.bDepthWriteEnable;
    depthStencilStateCI.depthCompareOp = depthStencilState.depthCompareOp;
    depthStencilStateCI.depthBoundsTestEnable = depthStencilState.bDepthBoundsTestEnable;
    depthStencilStateCI.stencilTestEnable = depthStencilState.bStencilTestEnable;
    depthStencilStateCI.front = depthStencilState.front;
    depthStencilStateCI.back = depthStencilState.back;
    depthStencilStateCI.minDepthBounds = depthStencilState.minDepthBounds;
    depthStencilStateCI.maxDepthBounds = depthStencilState.maxDepthBounds;

    //------------------------------------------------------------------------------------------------------------------
    // Pipeline settings: states which are specified dynamically
    //------------------------------------------------------------------------------------------------------------------
    constexpr const VkDynamicState DYNAMIC_STATES[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCI = {};
    dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCI.dynamicStateCount = C_ARRAY_SIZE(DYNAMIC_STATES);
    dynamicStateCI.pDynamicStates = DYNAMIC_STATES;
    
    //------------------------------------------------------------------------------------------------------------------
    // Create the pipeline itself
    //------------------------------------------------------------------------------------------------------------------
    VkGraphicsPipelineCreateInfo pipelineCI = {};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.stageCount = numShaderModules;
    pipelineCI.pStages = shaderStageCIs;
    pipelineCI.pVertexInputState = &vertexInputStateCI;
    pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pRasterizationState = &rasterizerStateCI;
    pipelineCI.pMultisampleState = &multisampleStateCI;
    pipelineCI.pDepthStencilState = &depthStencilStateCI;
    pipelineCI.pColorBlendState = &colorBlendStateCI;
    pipelineCI.pDynamicState = &dynamicStateCI;
    pipelineCI.layout = pipelineLayout.getVkPipelineLayout();
    pipelineCI.renderPass = renderPass.getVkRenderPass();
    pipelineCI.subpass = subpassIndex;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;     // Used when creating derived pipelines
    pipelineCI.basePipelineIndex = -1;                  // Used when creating derived pipelines

    const VkFuncs& vkFuncs = device.getVkFuncs();

    if (vkFuncs.vkCreateGraphicsPipelines(device.getVkDevice(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &mVkPipeline) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a graphics pipeline!");
        return false;
    }

    ASSERT(mVkPipeline);

    // If we got to here then all is well
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes a compute pipeline with the specified settings and state
//------------------------------------------------------------------------------------------------------------------------------------------
bool Pipeline::initComputePipeline(
    const VkPipelineLayout& pipelineLayout,
    const ShaderModule& shaderModule,
    const VkSpecializationInfo* const pShaderSpecializationInfo
) noexcept {
    //------------------------------------------------------------------------------------------------------------------
    // The basics
    //------------------------------------------------------------------------------------------------------------------

    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(pipelineLayout != VK_NULL_HANDLE);
    ASSERT(shaderModule.isValid());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true, true);
        }
    });

    // Saving some of the basics
    LogicalDevice& device = *shaderModule.getDevice();

    mbIsComputePipeline = true;
    mpDevice = &device;
    mVkPipelineLayout = pipelineLayout;

    //------------------------------------------------------------------------------------------------------------------
    // Create the pipeline itself
    //------------------------------------------------------------------------------------------------------------------

    // Note: eventually we might want a mechanism to override the shader entry point here!
    VkComputePipelineCreateInfo pipelineCI = {};
    pipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCI.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineCI.stage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineCI.stage.module = shaderModule.getVkShaderModule();
    pipelineCI.stage.pName = "main";
    pipelineCI.stage.pSpecializationInfo = pShaderSpecializationInfo;
    pipelineCI.layout = pipelineLayout;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;     // Used when creating derived pipelines
    pipelineCI.basePipelineIndex = -1;                  // Used when creating derived pipelines

    const VkFuncs& vkFuncs = device.getVkFuncs();

    if (vkFuncs.vkCreateComputePipelines(device.getVkDevice(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &mVkPipeline) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a compute pipeline!");
        return false;
    }

    ASSERT(mVkPipeline);

    // If we got to here then all is well
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the pipeline and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void Pipeline::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    const bool bIsValid = mbIsValid;

    if ((!bIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Gradual 'retirement' logic if specified and possible
    if ((!bImmediately) && bIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Normal cleanup logic
    mbIsValid = false;
    
    if (mVkPipeline) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyPipeline(mpDevice->getVkDevice(), mVkPipeline, nullptr);
        mVkPipeline = VK_NULL_HANDLE;
    }

    mpDevice = nullptr;
    mVkPipelineLayout = VK_NULL_HANDLE;
    mbIsComputePipeline = false;
}

END_NAMESPACE(vgl)
