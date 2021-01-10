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
#include "FatalErrors.h"
#include "LogicalDevice.h"
#include "Pipeline.h"
#include "VPipelines.h"
#include "VTypes.h"

BEGIN_NAMESPACE(VDrawing)

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
// Initializes the drawing module and allocates draw vertex buffers etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device) noexcept {
    // Create the vertex buffers
    for (vgl::Buffer& vertexBuffer : gVertexBuffers) {
        vertexBuffer.initWithElementCount<VVertex>(
            device,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            vgl::BufferUsageMode::DYNAMIC,
            // Around 48K verts - should be absolutely PLENTY, even for very demanding user maps!
            // This comes to about 2 MiB per buffer if the vertex size is '40' bytes.
            // If we happen to exceed this limit then the buffer can always be resized later.
            1024 * 48,
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

    // Only switch pipelines if we need to and end the current draw batch before we do it
    if (gCurPipelineType != type) {
        endDrawBatch();
        gCurPipelineType = type;
        gpCurCmdBufRec->bindPipeline(VPipelines::gPipelines[(uint32_t) type]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the model/view/projection transform matrix used
//------------------------------------------------------------------------------------------------------------------------------------------
void setTransformMatirx(const float mvpMatrix[4][4]) noexcept {
    // Note: currently the transform matrix is the only uniform and it's only used by vertex shaders
    ASSERT(gpCurCmdBufRec);
    gpCurCmdBufRec->pushConstants(VPipelines::gPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VShaderUniforms), mvpMatrix);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a UI sprite to be drawn
//------------------------------------------------------------------------------------------------------------------------------------------
void addUISprite(
    const int16_t x,
    const int16_t y,
    const uint16_t w,
    const uint16_t h,
    const uint16_t u,
    const uint16_t v,
    const uint16_t clutId,
    const uint16_t texPageId
) noexcept {
    // TODO...
}

END_NAMESPACE(VDrawing)

#endif  // #if PSYDOOM_VULKAN_RENDERER
