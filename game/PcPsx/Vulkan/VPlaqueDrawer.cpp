//------------------------------------------------------------------------------------------------------------------------------------------
// This module handles drawing loading plaques for the new Vulkan renderer.
// It uses the previously rendered frame as the background and then draws a plaque on top of that.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VPlaqueDrawer.h"

#if PSYDOOM_VULKAN_RENDERER

#include "CmdBufferRecorder.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Doom/Base/i_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/RendererVk/rv_utils.h"
#include "Gpu.h"
#include "LogicalDevice.h"
#include "PcPsx/PsxVm.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Texture.h"
#include "VDrawing.h"
#include "VPipelines.h"
#include "VRenderer.h"
#include "VRenderPath_FadeLoad.h"
#include "VRenderPath_Main.h"

BEGIN_NAMESPACE(VPlaqueDrawer)

// A vertex buffer containing a two quads (four triangles) of vertex type 'VVertex_XyUv'.
// One quad draws the background or previous frame, the other draws the loading plaque.
static vgl::Buffer gVertexBuffer;

// A descriptor pool and the descriptor sets allocated from it.
// One descriptor set binds the background or previous frame texture, the other binds the plaque texture.
static vgl::DescriptorPool gDescriptorPool;
static vgl::DescriptorSet* gpDescSet_Background;
static vgl::DescriptorSet* gpDescSet_Plaque;

// This is the texture from which the loading plaque is drawn
static vgl::Texture gPlaqueTex;

// This is the background texture to use during plaque drawing.
// It's sourced from the framebuffer of a previously rendered frame.
static vgl::RenderTexture* gpBackgroundTex;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the vertex buffer used by the plaque drawer
//------------------------------------------------------------------------------------------------------------------------------------------
static void initVertexBuffer(vgl::LogicalDevice& device) noexcept {
    // Note: just keep this one in RAM since it's only used once and so small
    const bool bCreatedBufferOk = gVertexBuffer.initWithElementCount<VVertex_XyUv>(
        device,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        vgl::BufferUsageMode::IMMEDIATE,
        12
    );

    if (!bCreatedBufferOk)
        FatalErrors::raise("Failed to initialize a vertex buffer used for plaque drawing!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the descriptor pool and descriptor sets used for plaque drawing
//-----------------------------------------------------------------------------------------------------------------------------------------
static void initDescriptorPoolAndSet(vgl::LogicalDevice& device) noexcept {
    // Make the pool
    VkDescriptorPoolSize poolResourceCount = {};
    poolResourceCount.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolResourceCount.descriptorCount = 2;

    if (!gDescriptorPool.init(device, { poolResourceCount }, 2))
        FatalErrors::raise("Failed to create a descriptor pool used for plaque drawing!");

    // Alloc the descriptor sets
    gpDescSet_Background = gDescriptorPool.allocDescriptorSet(VPipelines::gDescSetLayout_loadingPlaque);
    gpDescSet_Plaque = gDescriptorPool.allocDescriptorSet(VPipelines::gDescSetLayout_loadingPlaque);

    if ((!gpDescSet_Background) || (!gpDescSet_Plaque))
        FatalErrors::raise("Failed to allocate a descriptor set used for plaque drawing!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the loading plaque texture from the given Doom texture.
// The Doom texture is assumed cached in VRAM at this point.
// Note: the Doom texture is also assumed to be in 8bpp mode.
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPlaqueTex(vgl::LogicalDevice& device, const texture_t& tex, const int16_t clutId) noexcept {
    // Sanity checks/pre-conditions
    ASSERT(!gPlaqueTex.isValid());
    ASSERT(tex.ppTexCacheEntries);

    // Get where the texture is in VRAM, note that these are 8bpp coords and not 16bpp
    uint16_t texX, texY, texW, texH;
    RV_GetTexWinXyWh(tex, texX, texY, texW, texH);

    // Get where the CLUT is in VRAM and get the pixel pointer
    uint16_t clutX, clutY;
    RV_ClutIdToClutXy(clutId, clutX, clutY);

    // Create the Vulkan texture and lock its pixels
    if (!gPlaqueTex.initAs2dTexture(device, VK_FORMAT_A1R5G5B5_UNORM_PACK16, texW, texH))
        FatalErrors::raise("Failed to create a Vulkan loading plaque texture!");

    uint16_t* const pPlaquePixels = (uint16_t*) gPlaqueTex.lock();

    // Populate all of those pixels
    const uint16_t* const pVram = PsxVm::gGpu.pRam;
    const uint32_t vramWidth = PsxVm::gGpu.ramPixelW;
    const uint16_t* const pClut = pVram + ((clutY * vramWidth) + clutX);

    for (uint32_t y = 0; y < texH; ++y) {
        for (uint32_t x = 0; x < texW; ++x) {
            // Get the destination and source pixel
            uint16_t& dstPixel = pPlaquePixels[texW * y + x];
            const uint16_t srcVram = pVram[(texY + y) * vramWidth + (texX + x) / 2];   // Note: divide x by '2' to convert to 16bpp pixel coords
            const uint16_t srcColorIdx = (x & 1) ? srcVram >> 8 : srcVram & 0xFFu;
            const uint16_t srcPixel = pClut[srcColorIdx];

            // Convert the pixel components and save
            if (srcPixel != 0) {
                const uint16_t srcR = (srcPixel >>  0) & 0x1F;
                const uint16_t srcG = (srcPixel >>  5) & 0x1F;
                const uint16_t srcB = (srcPixel >> 10) & 0x1F;
                dstPixel = (srcR << 10) | (srcG << 5) | (srcB << 0) | 0x8000;
            } else {
                dstPixel = 0;
            }
        }
    }

    // Unlock the plaque texture to begin the texture upload
    gPlaqueTex.unlock();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines which previous framebuffer texture to use for the background when drawing the loading plaque.
// Which texture used will vary depending on whether MSAA is enabled or not.
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineBackgroundTex(vgl::LogicalDevice& device) noexcept {
    // Note that this logic assumes a ringbuffer size of '2'!
    static_assert(vgl::Defines::RINGBUFFER_SIZE == 2);

    VRenderPath_Main& mainRPath = VRenderer::gRenderPath_Main;
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();

    if (mainRPath.getNumDrawSamples() > 1) {
        VMsaaResolver& msaaResolver = mainRPath.getMsaaResolver();
        gpBackgroundTex = &msaaResolver.getResolveAttachment(ringbufferIdx ^ 1);
    } else {
        gpBackgroundTex = &mainRPath.getFramebufferAttachment(ringbufferIdx ^ 1);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the vertex buffer for drawing the loading plaque and previous frame's background
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateVertexBuffer(texture_t& plaqueTex, const int16_t plaqueX, const int16_t plaqueY) noexcept {
    // Lock the vertices and populate the quad for the background
    VVertex_XyUv* const pVerts = gVertexBuffer.lockElements<VVertex_XyUv>(0, 12);

    pVerts[0] = { -1.0f, -1.0f, 0.0f, 0.0f };
    pVerts[1] = { +1.0f, -1.0f, 1.0f, 0.0f };
    pVerts[2] = { +1.0f, +1.0f, 1.0f, 1.0f };
    pVerts[3] = { +1.0f, +1.0f, 1.0f, 1.0f };
    pVerts[4] = { -1.0f, +1.0f, 0.0f, 1.0f };
    pVerts[5] = { -1.0f, -1.0f, 0.0f, 0.0f };

    // Get the UI transform matrix
    const Matrix4f uiTransforms = VDrawing::computeTransformMatrixForUI();

    // Get the 4 vertices of the plaque quad in original PSX coords and transform to NDC space
    const float lx = (float) plaqueX;
    const float rx = (float) plaqueX + (float) plaqueTex.width;
    const float ty = (float) plaqueY;
    const float by = (float) plaqueY + (float) plaqueTex.height;

    float pv1[3] = { lx, ty, 0.0f };
    float pv2[3] = { rx, ty, 0.0f };
    float pv3[3] = { rx, by, 0.0f };
    float pv4[3] = { lx, by, 0.0f };

    uiTransforms.transform3d(pv1, pv1);
    uiTransforms.transform3d(pv2, pv2);
    uiTransforms.transform3d(pv3, pv3);
    uiTransforms.transform3d(pv4, pv4);

    // Populate the quad for the plaque from these NDC coords and unlock the vertex buffer to finish up
    pVerts[6 ] = { pv1[0], pv1[1], 0.0f, 0.0f };
    pVerts[7 ] = { pv2[0], pv2[1], 1.0f, 0.0f };
    pVerts[8 ] = { pv3[0], pv3[1], 1.0f, 1.0f };
    pVerts[9 ] = { pv3[0], pv3[1], 1.0f, 1.0f };
    pVerts[10] = { pv4[0], pv4[1], 0.0f, 1.0f };
    pVerts[11] = { pv1[0], pv1[1], 0.0f, 0.0f };

    gVertexBuffer.unlockElements<VVertex_XyUv>(12);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Setup all the resources needed for drawing loading plaques
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device) noexcept {
    initVertexBuffer(device);
    initDescriptorPoolAndSet(device);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup all of the loading plaque drawing resources
//------------------------------------------------------------------------------------------------------------------------------------------
void destroy() noexcept {
    gpBackgroundTex = nullptr;
    gPlaqueTex.destroy(true);

    if (gpDescSet_Background) {
        gDescriptorPool.freeDescriptorSet(*gpDescSet_Background);
        gpDescSet_Background = nullptr;
    }

    if (gpDescSet_Plaque) {
        gDescriptorPool.freeDescriptorSet(*gpDescSet_Plaque);
        gpDescSet_Plaque = nullptr;
    }
    
    gDescriptorPool.destroy(true);
    gVertexBuffer.destroy(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the specified loading plaque texture at the given location on screen (UI coords)
//------------------------------------------------------------------------------------------------------------------------------------------
void drawPlaque(texture_t& plaqueTex, const int16_t plaqueX, const int16_t plaqueY, const int16_t clutId) noexcept {
    // Sanity checks and incrementing the drawn frame count
    ASSERT(gVertexBuffer.isValid());
    ASSERT(gpDescSet_Background);
    ASSERT(gpDescSet_Plaque);

    I_IncDrawnFrameCount();

    // Make sure the plaque texture is cached to VRAM firstly and populate the loading plaque texture.
    // Also decide which previous framebuffer texture to use for the background.
    vgl::LogicalDevice& device = VRenderer::gDevice;

    I_CacheTex(plaqueTex);
    initPlaqueTex(device, plaqueTex, clutId);

    determineBackgroundTex(device);
    ASSERT(gpBackgroundTex);

    // Populate the vertex buffer with the vertices to use and bind the background & plaque textures to their descriptor sets
    populateVertexBuffer(plaqueTex, plaqueX, plaqueY);
    gpDescSet_Background->bindTextureAndSampler(0, *gpBackgroundTex, VPipelines::gSampler_normClampNearest);
    gpDescSet_Plaque->bindTextureAndSampler(0, gPlaqueTex, VPipelines::gSampler_normClampNearest);

    // Schedule the background for the plaque to be transitioned to 'shader read only' image layout.
    // The render path will transition this texture as soon as it begins it's frame.
    VRenderer::gRenderPath_FadeLoad.scheduleOldFramebufferLayoutTransitions(*gpBackgroundTex);

    // Make sure we are on the correct render path for drawing the plaque
    VRenderer::setRenderPathImmediate(VRenderer::gRenderPath_FadeLoad);

    // Only issue drawing commands if we can actually render
    if (VRenderer::isRendering()) {
        // What size is the view being rendered to?
        vgl::Swapchain& swapchain = VRenderer::gSwapchain;
        const uint32_t viewportW = swapchain.getSwapExtentWidth();
        const uint32_t viewportH = swapchain.getSwapExtentHeight();

        // Setup the viewport, bind the vertex buffer and pipeline used
        vgl::CmdBufferRecorder& cmdRec = VRenderer::gCmdBufferRec;
        vgl::Pipeline& pipeline = VPipelines::gPipelines[(uint32_t) VPipelineType::LoadingPlaque];

        cmdRec.setViewport(0.0f, 0.0f, (float) viewportW, (float) viewportH, 0.0f, 1.0f);
        cmdRec.setScissors(0, 0, viewportW, viewportH);
        cmdRec.bindVertexBuffer(gVertexBuffer, 0, 0);
        cmdRec.bindPipeline(pipeline);

        // Draw the background
        cmdRec.bindDescriptorSet(*gpDescSet_Background, pipeline, 0);
        cmdRec.draw(6, 0);

        // Draw the plaque
        cmdRec.bindDescriptorSet(*gpDescSet_Plaque, pipeline, 0);
        cmdRec.draw(12, 6);
    }

    // Cleanup when we are done by destroying the plaque tex later
    gPlaqueTex.destroy(false);

    // Return back to the main render path after drawing the plaque and wait for all drawing to end.
    // Have to do this in case external code decides to resize the background framebuffer used for plaque drawing.
    // We must be sure plaque drawing is done with the framebuffer first, before we proceed...
    VRenderer::setNextRenderPath(VRenderer::gRenderPath_Main);
    VRenderer::endFrame();
    device.waitUntilDeviceIdle();

    // Go back to doing normal rendering
    VRenderer::beginFrame();
}

END_NAMESPACE(VPlaqueDrawer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
