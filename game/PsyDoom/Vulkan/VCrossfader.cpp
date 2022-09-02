//------------------------------------------------------------------------------------------------------------------------------------------
// This module handles doing crossfades between the final color image of two framebuffers and renders them directly to the swapchain image.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VCrossfader.h"

#if PSYDOOM_VULKAN_RENDERER

#include "CmdBufferRecorder.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Doom/Base/i_main.h"
#include "FatalErrors.h"
#include "LogicalDevice.h"
#include "Pipeline.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/Utils.h"
#include "RenderTexture.h"
#include "Sampler.h"
#include "Swapchain.h"
#include "VPipelines.h"
#include "VRenderer.h"
#include "VRenderPath_Crossfade.h"
#include "VRenderPath_Main.h"
#include "VScreenQuad.h"

BEGIN_NAMESPACE(VCrossfader)

// The screen quad used for rendering the cross fade
static VScreenQuad gScreenQuad;

// A descriptor pool and the descriptor set allocated from it.
// The descriptor set contains 2 bindings, one for each of the rendered framebuffers that the crossfade is happening between.
static vgl::DescriptorPool gDescriptorPool;
static vgl::DescriptorSet* gpDescriptorSet;

// The textures used for crossfading
static vgl::RenderTexture* gpCrossfadeTex1;
static vgl::RenderTexture* gpCrossfadeTex2;

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the descriptor pool and descriptor set used for crossfading
//-----------------------------------------------------------------------------------------------------------------------------------------
static void initDescriptorPoolAndSet(vgl::LogicalDevice& device) noexcept {
    // Make the pool
    VkDescriptorPoolSize poolResourceCount = {};
    poolResourceCount.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolResourceCount.descriptorCount = 2;

    if (!gDescriptorPool.init(device, { poolResourceCount }, 1))
        FatalErrors::raise("Failed to create a descriptor pool used for crossfading!");

    // Alloc the single descriptor set
    gpDescriptorSet = gDescriptorPool.allocDescriptorSet(VPipelines::gDescSetLayout_crossfade);

    if (!gpDescriptorSet)
        FatalErrors::raise("Failed to allocate the descriptor set used for crossfading!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines which framebuffer textures to crossfade.
// Which textures are used will vary depending on whether MSAA is enabled or not.
// This should be called at a point in time where we are still drawing the image to be cross faded INTO.
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineCrossfadeTextures(vgl::LogicalDevice& device) noexcept {
    // Note that this logic assumes a ringbuffer size of '2'!
    static_assert(vgl::Defines::RINGBUFFER_SIZE == 2);

    VRenderPath_Main& mainRPath = VRenderer::gRenderPath_Main;
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();

    if (mainRPath.getNumDrawSamples() > 1) {
        VMsaaResolver& msaaResolver = mainRPath.getMsaaResolver();
        gpCrossfadeTex1 = &msaaResolver.getResolveAttachment(ringbufferIdx ^ 1);
        gpCrossfadeTex2 = &msaaResolver.getResolveAttachment(ringbufferIdx);
    } else {
        gpCrossfadeTex1 = &mainRPath.getFramebufferAttachment(ringbufferIdx ^ 1);
        gpCrossfadeTex2 = &mainRPath.getFramebufferAttachment(ringbufferIdx);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Binds the textures used for crossfading to the descriptor set used for drawing
//------------------------------------------------------------------------------------------------------------------------------------------
static void bindCrossfadeTextures() noexcept {
    ASSERT(gpDescriptorSet);
    ASSERT(gpCrossfadeTex1);
    ASSERT(gpCrossfadeTex2);

    const VkSampler vkSampler = VPipelines::gSampler_normClampNearest.getVkSampler();

    // Note: used to use an array of 2 textures, but MoltenVK didn't like that on MacOS.
    // Use 2 separate texture bindings instead to work around the issue...
    VkDescriptorImageInfo imageInfos[2] = {};
    imageInfos[0].sampler = vkSampler;
    imageInfos[0].imageView = gpCrossfadeTex1->getVkImageView();
    imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[1].sampler = vkSampler;
    imageInfos[1].imageView = gpCrossfadeTex2->getVkImageView();
    imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    gpDescriptorSet->bindTextures(0, &imageInfos[0], 1);
    gpDescriptorSet->bindTextures(1, &imageInfos[1], 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws one frame for the crossfader
//------------------------------------------------------------------------------------------------------------------------------------------
static void drawCrossfadeFrame(const float fadePercentComplete) noexcept {
    // What size is the view being rendered to?
    vgl::Swapchain& swapchain = VRenderer::gSwapchain;
    const uint32_t viewportW = swapchain.getSwapExtentWidth();
    const uint32_t viewportH = swapchain.getSwapExtentHeight();

    // Record the commands to draw the crossfade
    vgl::CmdBufferRecorder& cmdRec = VRenderer::gCmdBufferRec;
    vgl::Pipeline& pipeline = VPipelines::gPipelines[(uint32_t) VPipelineType::Crossfade];

    cmdRec.setViewport(0.0f, 0.0f, (float) viewportW, (float) viewportH, 0.0f, 1.0f);
    cmdRec.setScissors(0, 0, viewportW, viewportH);
    cmdRec.bindPipeline(pipeline);
    cmdRec.bindDescriptorSet(*gpDescriptorSet, pipeline, 0);
    cmdRec.pushConstants(VPipelines::gPipelineLayout_crossfade, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &fadePercentComplete);

    gScreenQuad.bindVertexBuffer(cmdRec, 0);
    gScreenQuad.draw(cmdRec);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Setup all the resources needed for crossfading
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device) noexcept {
    gScreenQuad.init(device);
    initDescriptorPoolAndSet(device);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup all of the crossfading resources
//------------------------------------------------------------------------------------------------------------------------------------------
void destroy() noexcept {
    gpCrossfadeTex1 = nullptr;
    gpCrossfadeTex2 = nullptr;

    if (gpDescriptorSet) {
        gDescriptorPool.freeDescriptorSet(*gpDescriptorSet);
        gpDescriptorSet = nullptr;
    }

    gDescriptorPool.destroy(true);
    gScreenQuad.destroy(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does some setup prior to doing a crossfade
//------------------------------------------------------------------------------------------------------------------------------------------
void doPreCrossfadeSetup() noexcept {
    // Get the device to crossfade with
    ASSERT(gScreenQuad.isValid());
    vgl::LogicalDevice& device = *gDescriptorPool.getDevice();

    // Determine which textures/framebuffers to do the crossfade with and tell the crossfader to use them
    determineCrossfadeTextures(device);
    ASSERT(gpCrossfadeTex1);
    ASSERT(gpCrossfadeTex2);

    VRenderer::gRenderPath_Crossfade.setOldFramebufferTextures(gpCrossfadeTex1, gpCrossfadeTex2);

    // Schedule a transition to the crossfade render path next frame and request that the next frame rendered not be presented.
    // This will be the frame we fade INTO and we don't want it shown onscreen after it is drawn.
    VRenderer::setNextRenderPath(VRenderer::gRenderPath_Crossfade);
    VRenderer::skipNextFramePresent();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does a crossfade for the specified duration given in original PSX vblanks
//------------------------------------------------------------------------------------------------------------------------------------------
void doCrossfade(const int32_t vblanksDuration) noexcept {
    // Get the device to crossfade with
    ASSERT(gScreenQuad.isValid());
    vgl::LogicalDevice& device = *gDescriptorPool.getDevice();

    // Prior to this being called the renderer should already be put into the crossfade render path, and crossfade textures determined
    ASSERT(&VRenderer::getActiveRenderPath() == &VRenderer::gRenderPath_Crossfade);
    ASSERT(gpCrossfadeTex1);
    ASSERT(gpCrossfadeTex2);

    // Sample the begin time (in vblanks) for the crossfade
    const int32_t fadeBeginTimeVbl = I_GetTotalVBlanks();
    float percentComplete = 0.0f;

    // Continue fading until enough time has elapsed
    bool bDidBindCrossfadeTextures = false;

    while (true) {
        // Draw a fade frame if rendering and bind the crossfade textures to the descriptor set if required
        I_IncDrawnFrameCount();

        if (VRenderer::isRendering()) {
            if (!bDidBindCrossfadeTextures) {
                bindCrossfadeTextures();
                bDidBindCrossfadeTextures = true;
            }

            drawCrossfadeFrame(percentComplete);
        }

        // Do platform updates and yield some CPU time in case vsync is not capping us
        Utils::doPlatformUpdates();
        Utils::threadYield();

        // Is it time to end the fade?
        const int32_t nowTimeVbl = I_GetTotalVBlanks();
        const int32_t elapsedVbl = nowTimeVbl - fadeBeginTimeVbl;

        if ((elapsedVbl >= vblanksDuration) || Input::isQuitRequested())
            break;

        // Otherwise begin a new frame and update the percent complete
        percentComplete = (float) elapsedVbl / (float) vblanksDuration;
        VRenderer::endFrame();
        VRenderer::beginFrame();
    }

    // Return back to the main render path after doing the crossfade and wait for all drawing to end.
    // We have to do this in case the framebuffers need to be resized after the crossfade is done, so the crossfade needs to be done with them at this point.
    VRenderer::setNextRenderPath(VRenderer::gRenderPath_Main);
    VRenderer::endFrame();
    device.waitUntilDeviceIdle();

    // Go back to doing normal rendering
    VRenderer::beginFrame();

    // Clear these out for good measure
    VRenderer::gRenderPath_Crossfade.setOldFramebufferTextures(nullptr, nullptr);
}

END_NAMESPACE(VCrossfader)

#endif  // #if PSYDOOM_VULKAN_RENDERER
