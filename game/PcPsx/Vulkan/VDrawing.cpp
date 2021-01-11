#if PSYDOOM_VULKAN_RENDERER

//------------------------------------------------------------------------------------------------------------------------------------------
// A module that handles the submission of draw commands to the Vulkan renderer.
// Allows various primitives to be submitted.
// Records the vertex data into a buffer and the commands into the current command buffer used by the renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VDrawing.h"

#include "Asserts.h"
#include "Buffer.h"
#include "CmdBufferRecorder.h"
#include "Defines.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "FatalErrors.h"
#include "Framebuffer.h"
#include "Gpu.h"
#include "LogicalDevice.h"
#include "Pipeline.h"
#include "VPipelines.h"
#include "VTypes.h"

BEGIN_NAMESPACE(VDrawing)

// A descriptor set and descriptor pool used for all drawing operations.
// Just binds the PSX VRAM texture to it's combined image sampler.
static vgl::DescriptorPool  gDescriptorPool;
static vgl::DescriptorSet*  gpDescriptorSet;

// Hardware vertex buffers used to host the vertex data
static vgl::Buffer gVertexBuffers[vgl::Defines::RINGBUFFER_SIZE];

static vgl::Buffer*     gpCurVtxBuffer;         // The current vertex buffer being used
static VVertex*         gpCurVtxBufferVerts;    // Pointer to the locked vertices in the vertex buffer
static uint32_t         gCurVtxBufferOffset;    // How many vertices have been written to the current vertex buffer
static uint32_t         gCurVtxBufferSize;      // The current capacity of the vertex buffer
static uint32_t         gCurDrawBatchStart;     // Which vertex the current draw batch starts on
static uint32_t         gCurDrawBatchSize;      // How many vertices are in the current draw batch

// The command buffer recorder being used for the current frame
static vgl::CmdBufferRecorder* gpCurCmdBufRec;

// The current pipeline being used: used to avoid pipeline switches where possible
static VPipelineType gCurPipelineType;

//------------------------------------------------------------------------------------------------------------------------------------------
// Ensures the specified number of vertices can be allocated, and if they can't resizes the current vertex buffer.
// Note: must only be called after 'beginFrame'.
//------------------------------------------------------------------------------------------------------------------------------------------
static void ensureNumVtxBufferVerts(const uint32_t numVerts) noexcept {
    // What size would be good to hold this amount of vertices?
    ASSERT(gpCurVtxBufferVerts);
    uint32_t newVtxBufSize = gCurVtxBufferSize;

    while (gCurVtxBufferOffset + numVerts > newVtxBufSize) {
        newVtxBufSize *= 2;
    }

    // Do we need to do a resize?
    if (newVtxBufSize != gCurVtxBufferSize) {
        const bool bResizeOk = gpCurVtxBuffer->resizeToElementCount<VVertex>(
            newVtxBufSize,
            vgl::Buffer::ResizeFlagBits::KEEP_LOCKED_DATA,
            0,
            newVtxBufSize
        );

        if (!bResizeOk)
            FatalErrors::raise("Failed to resize a Vulkan vertex buffer used for rendering! May be out of memory!");

        // Need to update the locked pointer after resizing and the new vertex buffer size
        gpCurVtxBufferVerts = gpCurVtxBuffer->getLockedElements<VVertex>();
        ASSERT(gpCurVtxBufferVerts);
        gCurVtxBufferSize = newVtxBufSize;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ends the current draw batch and issues the command to draw the primitives using the pipeline
//------------------------------------------------------------------------------------------------------------------------------------------
static void endDrawBatch() noexcept {
    ASSERT(gpCurCmdBufRec);
    
    if (gCurDrawBatchSize > 0) {
        gpCurCmdBufRec->draw(gCurDrawBatchSize, gCurDrawBatchStart);
        gCurDrawBatchStart = gCurVtxBufferOffset;
        gCurDrawBatchSize = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpack a CLUT 'id' into an x, y coordinate for the CLUT
//------------------------------------------------------------------------------------------------------------------------------------------
static void clutIdToClutXy(const uint16_t clutId, uint16_t& clutX, uint16_t& clutY) noexcept {
    clutX = (clutId & 0x3Fu) << 4;      // Max coord: 1023, restricted to being on 16 pixel boundaries on the x-axis
    clutY = (clutId >> 6) & 0x3FFu;     // Max coord: 1023
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpack various texturing parameters from the texture page id.
// Extracts the texture format, texture page position, and blending mode.
//------------------------------------------------------------------------------------------------------------------------------------------
static void texPageIdToTexParams(
    const uint16_t texPageId,
    Gpu::TexFmt& texFmt,
    uint16_t& texPageX,
    uint16_t& texPageY,
    Gpu::BlendMode& blendMode
) noexcept {
    texFmt = (Gpu::TexFmt)((texPageId >> 7) & 0x0003u);
    texPageX = ((texPageId & 0x000Fu) >> 0) * 64u;
    texPageY = ((texPageId & 0x0010u) >> 4) * 256u;
    blendMode = (Gpu::BlendMode)((texPageId >> 5) & 0x0003u);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the drawing module and allocates draw vertex buffers etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device, vgl::BaseTexture& vramTex) noexcept {
    // Create the descriptor pool and descriptor set used for rendering.
    // Only using a single descriptor set which is bound to the PSX VRAM texture.
    {
        // Make the pool
        VkDescriptorPoolSize poolResourceCount = {};
        poolResourceCount.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolResourceCount.descriptorCount = 1;

        if (!gDescriptorPool.init(device, { poolResourceCount }, 1))
            FatalErrors::raise("VDrawing: Failed to create a Vulkan descriptor pool!");

        // Make the set
        gpDescriptorSet = gDescriptorPool.allocDescriptorSet(VPipelines::gDescriptorSetLayout);

        if (!gpDescriptorSet)
            FatalErrors::raise("VDrawing: Failed to allocate a required Vulkan descriptor set!");

        // Bind the PSX VRAM texture and the sampler to it
        gpDescriptorSet->bindTextureAndSampler(0, vramTex, VPipelines::gSampler);
    }

    // Create the vertex buffers
    for (vgl::Buffer& vertexBuffer : gVertexBuffers) {
        vertexBuffer.initWithElementCount<VVertex>(
            device,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            vgl::BufferUsageMode::DYNAMIC,
            // Around 46K verts - should be absolutely PLENTY, even for very demanding user maps!
            // This comes to about 2 MiB per buffer if the vertex size is '44' bytes.
            // If we happen to exceed this limit then the buffer can always be resized later.
            1024 * 46,
            true
        );
    }

    // Current pipeline is undefined initially
    gCurPipelineType = (VPipelineType) -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the drawing module and frees up resources
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gCurPipelineType = {};
    gpCurCmdBufRec = nullptr;
    gCurDrawBatchSize = 0;
    gCurDrawBatchStart = 0;
    gCurVtxBufferSize = 0;
    gCurVtxBufferOffset = 0;
    gpCurVtxBufferVerts = nullptr;
    gpCurVtxBuffer = nullptr;

    for (vgl::Buffer& vertexBuffer : gVertexBuffers) {
        vertexBuffer.destroy(true);
    }

    if (gpDescriptorSet) {
        gpDescriptorSet->free(true);
        gpDescriptorSet = nullptr;
    }

    gDescriptorPool.destroy(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs start of frame logic for the drawing module
//------------------------------------------------------------------------------------------------------------------------------------------
void beginFrame(vgl::LogicalDevice& device, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Save the command buffer recorder used
    gpCurCmdBufRec = &cmdRec;

    // Get what vertex buffer to use and lock its entire range for writing
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();

    gpCurVtxBuffer = &gVertexBuffers[ringbufferIdx];
    gCurVtxBufferOffset = 0;
    gCurVtxBufferSize = (uint32_t) gpCurVtxBuffer->getSizeInElements<VVertex>();
    gpCurVtxBufferVerts = gpCurVtxBuffer->lockElements<VVertex>(0, gCurVtxBufferSize);

    // Expect all these to be already setup correctly
    ASSERT(gCurDrawBatchStart == 0);
    ASSERT(gCurDrawBatchSize == 0);
    ASSERT(gCurPipelineType == (VPipelineType) -1);

    // Get the size of the current draw framebuffer
    const vgl::Framebuffer& framebuffer = device.getScreenFramebufferMgr().getCurrentDrawFramebuffer();
    const uint32_t fbWidth = framebuffer.getWidth();
    const uint32_t fbHeight = framebuffer.getHeight();

    // First draw commands: set viewport and scissors dimensions and bind the vertex buffer
    gpCurCmdBufRec->setViewport(0, 0, (float) fbWidth, (float) fbHeight, 0.0f, 1.0f);
    gpCurCmdBufRec->setScissors(0, 0, fbWidth, fbHeight);
    gpCurCmdBufRec->bindVertexBuffer(*gpCurVtxBuffer, 0, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs end of frame logic for the drawing module
//------------------------------------------------------------------------------------------------------------------------------------------
void endFrame() noexcept {
    // Must have started a frame
    ASSERT(gpCurCmdBufRec);

    // Finish the current draw batch and rewind for the next frame
    endDrawBatch();
    gCurDrawBatchStart = 0;
    gCurDrawBatchSize = 0;

    // Unlock the vertex buffer used to schedule the transfer of vertex data to the GPU
    ASSERT(gpCurVtxBuffer);
    gpCurVtxBuffer->unlockElements<VVertex>(gCurVtxBufferOffset);

    // Clear the details for the locked vertex buffer
    gpCurVtxBuffer = nullptr;
    gpCurVtxBufferVerts = nullptr;
    gCurVtxBufferSize = 0;
    gCurVtxBufferOffset = 0;

    // Current pipeline is undefined on ending a frame
    gCurPipelineType = (VPipelineType) -1;

    // Done with this recorder now
    gpCurCmdBufRec = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set which pipeline is being used for drawing with lazy early out if there is no change
//------------------------------------------------------------------------------------------------------------------------------------------
void setPipeline(const VPipelineType type) noexcept {
    ASSERT(gpCurCmdBufRec);
    ASSERT((uint32_t) type < (uint32_t) VPipelineType::NUM_TYPES);

    // Only switch pipelines if we need to
    if (gCurPipelineType != type) {
        // Must end the current draw batch before switching
        endDrawBatch();

        // Do the switch
        gCurPipelineType = type;
        vgl::Pipeline& pipeline = VPipelines::gPipelines[(uint32_t) type];
        gpCurCmdBufRec->bindPipeline(pipeline);

        // Bind the descriptor set used for all rendering for this pipeline
        gpCurCmdBufRec->bindDescriptorSet(*gpDescriptorSet, pipeline, 0, 0, nullptr);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the model/view/projection transform matrix used
//------------------------------------------------------------------------------------------------------------------------------------------
void setTransformMatrix(const float mvpMatrix[4][4]) noexcept {
    // Note: currently the transform matrix is the only uniform and it's only used by vertex shaders
    ASSERT(gpCurCmdBufRec);
    gpCurCmdBufRec->pushConstants(VPipelines::gPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VShaderUniforms), mvpMatrix);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Setup the transform matrix for UI
//------------------------------------------------------------------------------------------------------------------------------------------
void setTransformMatrixForUI() noexcept {
    // Get the current draw framebuffer being used and its size
    ASSERT(gpCurVtxBuffer);

    vgl::LogicalDevice& device = *gpCurVtxBuffer->getDevice();
    const vgl::Framebuffer& framebuffer = device.getScreenFramebufferMgr().getCurrentDrawFramebuffer();
    const uint32_t fbWidth = framebuffer.getWidth();
    const uint32_t fbHeight = framebuffer.getHeight();

    // Compute the left/right and top/bottom values for the view.
    // 
    // TODO: center the UI instead of stretching.
    const float lx = 0;
    const float rx = (float) 256;   // TODO: don't harcode
    const float ty = 0;
    const float by = (float) 240;   // TODO: don't hardcode
    const float zn = 0.0f;
    const float zf = 1.0f;

    // Note: the calculations here are based on the DirectX 9 utility function 'D3DXMatrixOrthoOffCenterRH()'.
    // I made some adjustments however so the y-axis runs DOWN the screen instead of up.
    const float matrix[4][4] = {
        2 / (rx - lx),          0,                      0,               0,
        0,                      2 / (by - ty),          0,               0,
        0,                      0,                      1  / (zn - zf),  0,
        (lx + rx) / (lx - rx),  (ty + by) / (ty - by),  zn / (zn - zf),  1
    };
    
    setTransformMatrix(matrix);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a UI sprite to be drawn
//------------------------------------------------------------------------------------------------------------------------------------------
void addUISprite(
    const float x,
    const float y,
    const float w,
    const float h,
    const float u,
    const float v,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const uint8_t a,
    const uint16_t clutId,
    const uint16_t texPageId,
    const bool bBlend
) noexcept {
    // Get draw params for the sprite
    uint16_t clutX;
    uint16_t clutY;
    clutIdToClutXy(clutId, clutX, clutY);

    Gpu::TexFmt texFmt;
    uint16_t texPageX;
    uint16_t texPageY;
    Gpu::BlendMode blendMode;
    texPageIdToTexParams(texPageId, texFmt, texPageX, texPageY, blendMode);

    // Switch to the correct pipeline and figure out the scaling for the 'u' texture coordinate depending on the texture bit rate
    float uScale;

    ASSERT_LOG(blendMode == Gpu::BlendMode::Alpha50, "Only alpha blending is supported for UI sprites!");

    if (texFmt == Gpu::TexFmt::Bpp8) {
        setPipeline(VPipelineType::UI_8bpp);
        uScale = 1.0f / 2.0f;
    } else if (texFmt == Gpu::TexFmt::Bpp4) {
        setPipeline(VPipelineType::UI_4bpp);
        uScale = 1.0f / 4.0f;
    } else {
        ASSERT(texFmt == Gpu::TexFmt::Bpp16);
        setPipeline(VPipelineType::UI_16bpp);
        uScale = 1.0f;
    }

    // Decide on the semi-transparent multiply alpha, depending on if blending is enabled.
    // A value of '128' is full alpha and '64' is 50% alpha.
    const uint8_t stmulA = (bBlend) ? 64 : 128;

    // Ensure we have enough vertices to proceed
    ensureNumVtxBufferVerts(6);

    // Fill in the vertices, starting first with common parameters
    VVertex* const pVerts = gpCurVtxBufferVerts + gCurVtxBufferOffset;

    for (uint32_t i = 0; i < 6; ++i) {
        VVertex& vert = pVerts[i];
        vert.z = 0.0f;              // Unused for UI
        vert.s = 0.0f;              // Unused for UI
        vert.r = r;
        vert.g = g;
        vert.b = b;
        vert.a = a;
        vert.texWinX = texPageX;
        vert.texWinY = texPageY;
        vert.texWinW = 256;         // PSX texture pages were 256 px
        vert.texWinH = 256;         // PSX texture pages were 256 px
        vert.clutX = clutX;
        vert.clutY = clutY;
        vert.stmulR = 128;          // Fully white (128 = 100% strength)
        vert.stmulG = 128;
        vert.stmulB = 128;
        vert.stmulA = stmulA;
    }

    // Fill in verts xy and uv positions
    pVerts[0].x = x;                pVerts[0].y = y;
    pVerts[1].x = (float)(x + w);   pVerts[1].y = y;
    pVerts[2].x = (float)(x + w);   pVerts[2].y = (float)(y + h);
    pVerts[3].x = (float)(x + w);   pVerts[3].y = (float)(y + h);
    pVerts[4].x = x;                pVerts[4].y = (float)(y + h);
    pVerts[5].x = x;                pVerts[5].y = y;

    const float ul = u * uScale;
    const float ur = (u + w) * uScale;
    const float vt = v;
    const float vb = v + h;

    pVerts[0].u = ul;   pVerts[0].v = vt;
    pVerts[1].u = ur;   pVerts[1].v = vt;
    pVerts[2].u = ur;   pVerts[2].v = vb;
    pVerts[3].u = ur;   pVerts[3].v = vb;
    pVerts[4].u = ul;   pVerts[4].v = vb;
    pVerts[5].u = ul;   pVerts[5].v = vt;

    // Consumed 6 buffer vertices and add 6 vertices to the current draw batch
    gCurVtxBufferOffset += 6;
    gCurDrawBatchSize += 6;
}

END_NAMESPACE(VDrawing)

#endif  // #if PSYDOOM_VULKAN_RENDERER
