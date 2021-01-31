//------------------------------------------------------------------------------------------------------------------------------------------
// A module that handles the submission of draw commands to the Vulkan renderer.
// Allows various primitives to be submitted.
// Records the vertex data into a buffer and the commands into the current command buffer used by the renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VDrawing.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
#include "BaseRenderPass.h"
#include "Buffer.h"
#include "CmdBufferRecorder.h"
#include "Defines.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Doom/doomdef.h"
#include "FatalErrors.h"
#include "Framebuffer.h"
#include "Gpu.h"
#include "LogicalDevice.h"
#include "Pipeline.h"
#include "VPipelines.h"
#include "VRenderer.h"
#include "VTypes.h"
#include "VVertexBufferSet.h"

BEGIN_NAMESPACE(VDrawing)

// Ringbuffer index for the current frame being generated
static uint32_t gCurRingbufferIdx;

// Descriptor set and a descriptor pool used for all 'draw' subpass operations.
// Binds the PSX VRAM texture to it's combined image sampler in binding 0.
static vgl::DescriptorPool  gDescriptorPool;
static vgl::DescriptorSet*  gpDescriptorSet;

// Command buffers for recording 'draw occluder plane' subpass and regular 'draw' subpass commands.
// One for each ringbuffer slot, so we don't disturb a frame still being processed.
static vgl::CmdBuffer gCmdBuffers_OccPlanePass[vgl::Defines::RINGBUFFER_SIZE];
static vgl::CmdBuffer gCmdBuffers_DrawPass[vgl::Defines::RINGBUFFER_SIZE];

// Command buffer recorders for each subpass or command buffer set
static vgl::CmdBufferRecorder gCmdBufRec_DepthPass(VRenderer::gVkFuncs);
static vgl::CmdBufferRecorder gCmdBufRec_DrawPass(VRenderer::gVkFuncs);

// Vertex buffers: for the 'depth draw' subpass (VVertex_Depth) and regular 'draw' subpass (VVertex_Draw)
static VVertexBufferSet gVertexBuffers_Depth;
static VVertexBufferSet gVertexBuffers_Draw;

// The current pipeline being used by the 'draw' subpass; used to help avoid unneccessary pipeline switches
static VPipelineType gCurDrawPipelineType;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the drawing module and allocates draw vertex buffers etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device, vgl::BaseTexture& vramTex) noexcept {
    // Initialize the command buffers used
    for (vgl::CmdBuffer& buffer : gCmdBuffers_OccPlanePass) {
        if (!buffer.init(device.getCmdPool(), VK_COMMAND_BUFFER_LEVEL_SECONDARY))
            FatalErrors::raise("VDrawing: initialize a secondary command buffer used for drawing!");
    }

    for (vgl::CmdBuffer& buffer : gCmdBuffers_DrawPass) {
        if (!buffer.init(device.getCmdPool(), VK_COMMAND_BUFFER_LEVEL_SECONDARY))
            FatalErrors::raise("VDrawing: initialize a secondary command buffer used for drawing!");
    }

    // Create the descriptor pool and descriptor set used for rendering.
    // Only using a single descriptor set which is bound to the PSX VRAM texture.
    {
        // Make the descriptor pool
        VkDescriptorPoolSize poolResources[1] = {};
        poolResources[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolResources[0].descriptorCount = vgl::Defines::RINGBUFFER_SIZE;

        if (!gDescriptorPool.init(device, { poolResources[0] }, 1))
            FatalErrors::raise("VDrawing: Failed to create a Vulkan descriptor pool!");

        // Make the descriptor set and bind the PSX VRAM texture and sampler to slot 0.
        // This particular binding only needs to be done once at startup!
        gpDescriptorSet = gDescriptorPool.allocDescriptorSet(VPipelines::gDescSetLayout_draw);

        if (!gpDescriptorSet)
            FatalErrors::raise("VDrawing: Failed to allocate a required Vulkan descriptor set!");

        gpDescriptorSet->bindTextureAndSampler(0, vramTex, VPipelines::gSampler_draw);
    }

    // Create the vertex buffer sets
    constexpr uint32_t DEPTH_VB_SIZE = 256 * 1024;  // 256 KiB per buffer
    constexpr uint32_t DRAW_VB_SIZE = 512 * 1024;   // 512 KiB per buffer

    gVertexBuffers_Depth.init<VVertex_Depth>(device, DEPTH_VB_SIZE / sizeof(VVertex_Depth));
    gVertexBuffers_Draw.init<VVertex_Draw>(device, DRAW_VB_SIZE / sizeof(VVertex_Draw));

    // Current draw pipeline in use is undefined initially
    gCurDrawPipelineType = (VPipelineType) -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the drawing module and frees up resources
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gCurDrawPipelineType = {};
    
    gVertexBuffers_Draw.destroy();
    gVertexBuffers_Depth.destroy();

    if (gpDescriptorSet) {
        gpDescriptorSet->free(true);
        gpDescriptorSet = nullptr;
    }

    gDescriptorPool.destroy(true);

    for (vgl::CmdBuffer& buffer : gCmdBuffers_DrawPass) {
        buffer.destroy(true);
    }

    for (vgl::CmdBuffer& buffer : gCmdBuffers_OccPlanePass) {
        buffer.destroy(true);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs start of frame logic for the drawing module
//------------------------------------------------------------------------------------------------------------------------------------------
void beginFrame(
    const uint32_t ringbufferIdx,
    const vgl::BaseRenderPass& renderPass,
    vgl::Framebuffer& framebuffer
) noexcept {
    // Remember which ringbuffer slot we are on
    gCurRingbufferIdx = ringbufferIdx;

    // Begin recording the command buffers
    gCmdBufRec_DepthPass.beginSecondaryCmdBuffer(
        gCmdBuffers_OccPlanePass[ringbufferIdx],
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        renderPass.getVkRenderPass(),
        0,
        framebuffer.getVkFramebuffer()
    );

    gCmdBufRec_DrawPass.beginSecondaryCmdBuffer(
        gCmdBuffers_DrawPass[ringbufferIdx],
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        renderPass.getVkRenderPass(),
        1,
        framebuffer.getVkFramebuffer()
    );

    // Setup the vertex buffers
    gVertexBuffers_Depth.beginFrame(ringbufferIdx);
    gVertexBuffers_Draw.beginFrame(ringbufferIdx);

    // Expect all this to already have the following state
    ASSERT(gCurDrawPipelineType == (VPipelineType) -1);

    // Get the size of the current draw framebuffer
    const uint32_t fbWidth = VRenderer::getVkRendererFbWidth();
    const uint32_t fbHeight = VRenderer::getVkRendererFbHeight();

    // First commands in the renderpass are to set the viewport and scissors dimensions.
    // Add these commands to both subpasses:
    gCmdBufRec_DepthPass.setViewport(0, 0, (float) fbWidth, (float) fbHeight, 0.0f, 1.0f);
    gCmdBufRec_DrawPass.setViewport(0, 0, (float) fbWidth, (float) fbHeight, 0.0f, 1.0f);
    gCmdBufRec_DepthPass.setScissors(0, 0, fbWidth, fbHeight);
    gCmdBufRec_DrawPass.setScissors(0, 0, fbWidth, fbHeight);

    // First command in the 'draw' subpass is to set the vertex buffer used
    gCmdBufRec_DrawPass.bindVertexBuffer(*gVertexBuffers_Draw.pCurBuffer, 0, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs end of frame logic for the drawing module
//------------------------------------------------------------------------------------------------------------------------------------------
void endFrame(vgl::CmdBufferRecorder& primaryCmdRec) noexcept {
    // Do any required drawing commands for the 'draw depth' subpass (if required)
    if (gVertexBuffers_Depth.curOffset > 0) {
        gCmdBufRec_DepthPass.bindVertexBuffer(*gVertexBuffers_Depth.pCurBuffer, 0, 0);
        gCmdBufRec_DepthPass.bindPipeline(VPipelines::gPipelines[(uint32_t) VPipelineType::World_Depth]);
        gVertexBuffers_Depth.endCurrentDrawBatch(gCmdBufRec_DepthPass);
    }

    // Finish the current 'draw' subpass batch
    endCurrentDrawBatch();

    // Upload any vertices generated by invoking the 'end frame' event
    gVertexBuffers_Depth.endFrame();
    gVertexBuffers_Draw.endFrame();

    // Current draw pipeline is undefined on ending a frame
    gCurDrawPipelineType = (VPipelineType) -1;

    // Done recording commands now
    gCmdBufRec_DepthPass.endCmdBuffer();
    gCmdBufRec_DrawPass.endCmdBuffer();

    // Submit the command buffers to the primary command buffer
    primaryCmdRec.exec(gCmdBuffers_OccPlanePass[gCurRingbufferIdx].getVkCommandBuffer());
    primaryCmdRec.nextSubpass(VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    primaryCmdRec.exec(gCmdBuffers_DrawPass[gCurRingbufferIdx].getVkCommandBuffer());

    // No longer on a frame, so default this
    gCurRingbufferIdx = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set which pipeline is being used for the 'draw' subpass with lazy early out if there is no change
//------------------------------------------------------------------------------------------------------------------------------------------
void setDrawPipeline(const VPipelineType type) noexcept {
    // Only switch pipelines if we need to
    ASSERT((uint32_t) type < (uint32_t) VPipelineType::NUM_TYPES);
    const VPipelineType oldPipelineType = gCurDrawPipelineType;

    if (oldPipelineType == type)
        return;

    // Must end the current draw batch before switching
    endCurrentDrawBatch();

    // Note: only need to bind the descriptor set once, can re-use among all draw pipelines
    const bool bNeedToBindDescriptorSet = (oldPipelineType == (VPipelineType) -1);

    // Do the pipeline switch
    gCurDrawPipelineType = type;
    vgl::Pipeline& pipeline = VPipelines::gPipelines[(uint32_t) type];
    gCmdBufRec_DrawPass.bindPipeline(pipeline);

    // Bind the 'draw' subpass descriptor set if we need to
    if (bNeedToBindDescriptorSet) {
        gCmdBufRec_DrawPass.bindDescriptorSet(*gpDescriptorSet, pipeline, 0, 0, nullptr);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the model/view/projection transform matrix used for the 'depth render' subpass
//------------------------------------------------------------------------------------------------------------------------------------------
void setDepthTransformMatrix(const Matrix4f& matrix) noexcept {
    gCmdBufRec_DepthPass.pushConstants(VPipelines::gPipelineLayout_depth, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VShaderUniforms), &matrix);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the model/view/projection transform matrix used for the 'draw' subpass
//------------------------------------------------------------------------------------------------------------------------------------------
void setDrawTransformMatrix(const Matrix4f& matrix) noexcept {
    gCmdBufRec_DrawPass.pushConstants(VPipelines::gPipelineLayout_draw, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VShaderUniforms), &matrix);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Compute the transform matrix for the UI/2D elements in Doom.
// Scales and positions everything such that the original UI coordinates can be used for the same result.
//------------------------------------------------------------------------------------------------------------------------------------------
Matrix4f computeTransformMatrixForUI() noexcept {
    // Projection parameters.
    // TODO: support widescreen modes and don't stretch in widescreen.
    const float viewLx = 0;
    const float viewRx = SCREEN_W;
    const float viewBy = 0;
    const float viewTy = SCREEN_H;      // Note: need to reverse by/ty to get the view the right way round (normally y is up with projection matrices)
    const float zNear = 0.0f;
    const float zFar = 1.0f;

    return Matrix4f::orthographicOffCenter(viewLx, viewRx, viewTy, viewBy, zNear, zFar);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Compute the transform matrix used to setup the 3D view for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
Matrix4f computeTransformMatrixFor3D(const float viewX, const float viewY, const float viewZ, const float viewAngle) noexcept {
    // Projection parameters.
    // Project as if we still had the old 256x200 3D viewport and off center to account for the status bar at the bottom.
    // This matches the projection that the original PSX Doom used:
    constexpr float STATUS_BAR_H = SCREEN_H - VIEW_3D_H;

    const float viewLx = -1.0f;      // TODO: support widescreen modes
    const float viewRx = 1.0f;
    const float viewTy =  HALF_VIEW_3D_H / float(HALF_SCREEN_W);
    const float viewBy = (-HALF_VIEW_3D_H - STATUS_BAR_H) / float(HALF_SCREEN_W);
    const float zNear = 1.0f;
    const float zFar = 65536.0f * 2.0f;

    // Compute the projection, rotation and translation matrices for the view.
    // Rotation is about the Y axis (Doom's Z axis).
    const Matrix4f projection = Matrix4f::perspectiveOffCenter(viewLx, viewRx, viewTy, viewBy, zNear, zFar);
    const Matrix4f rotation = Matrix4f::rotateY(viewAngle);
    const Matrix4f translation = Matrix4f::translate(-viewX, -viewY, -viewZ);

    // Now combine all the matrices and return the result
    const Matrix4f modelViewProj = translation * rotation * projection;
    return modelViewProj;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ends the current primitive batch for the 'draw' subpass and issues the command to draw the primitives.
// The primitives are drawn with whatever pipeline is currently bound.
//------------------------------------------------------------------------------------------------------------------------------------------
void endCurrentDrawBatch() noexcept {
    gVertexBuffers_Draw.endCurrentDrawBatch(gCmdBufRec_DrawPass);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a 2D/UI line to the 'draw' subpass
//------------------------------------------------------------------------------------------------------------------------------------------
void addDrawUILine(
    const float x1,
    const float y1,
    const float x2,
    const float y2,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const uint8_t a
) noexcept {
    // Ensure we are on the right pipeline
    ASSERT(gCurDrawPipelineType == VPipelineType::Lines);

    // Fill in the vertices, starting first with common parameters
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(2);

    for (uint32_t i = 0; i < 2; ++i) {
        VVertex_Draw& vert = pVerts[i];
        vert = {};
        vert.r = r;
        vert.g = g;
        vert.b = b;
        vert.stmulR = 128;
        vert.stmulG = 128;
        vert.stmulB = 128;
        vert.stmulA = a;
    }

    // Fill in xy positions
    pVerts[0].x = x1;   pVerts[0].y = y1;
    pVerts[1].x = x2;   pVerts[1].y = y2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a UI sprite to the 'draw' subpass
//------------------------------------------------------------------------------------------------------------------------------------------
void addDrawUISprite(
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
    const uint16_t clutX,
    const uint16_t clutY,
    const uint16_t texPageX,
    const uint16_t texPageY,
    const uint16_t texWinW,
    const uint16_t texWinH
) noexcept {
    // Figure out the scaling for the 'u' texture coordinate depending on the texture bit rate.
    // Note: UI 8bpp is the most commonly used, so I'm assuming that by default.
    const VPipelineType pipeline = gCurDrawPipelineType;
    float uScale = 1.0f / 2.0f;

    switch (pipeline) {
        case VPipelineType::UI_4bpp:
            uScale = 1.0 / 4.0f;
            break;

        case VPipelineType::UI_8bpp:
        case VPipelineType::UI_8bpp_Add:
        case VPipelineType::World_Sky:
            break;

        case VPipelineType::UI_16bpp:
            uScale = 1.0f;
            break;

        default:
            ASSERT_FAIL("Unhandled pipeline type!");
            break;
    }

    // Fill in the vertices, starting first with common parameters
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(6);

    for (uint32_t i = 0; i < 6; ++i) {
        VVertex_Draw& vert = pVerts[i];
        vert.z = {};                // Unused for UI shaders
        vert.r = r;
        vert.g = g;
        vert.b = b;
        vert.lightDimMode = {};     // Unused for UI shaders
        vert.texWinX = texPageX;
        vert.texWinY = texPageY;
        vert.texWinW = texWinW;
        vert.texWinH = texWinH;
        vert.clutX = clutX;
        vert.clutY = clutY;
        vert.stmulR = 128;          // Fully white (128 = 100% strength)
        vert.stmulG = 128;
        vert.stmulB = 128;
        vert.stmulA = a;
    }

    // Fill in the xy positions
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a triangle for the game's 3D view/world to the 'draw' subpass.
// 
// Notes:
//  (1) The texture format is assumed to be 8 bits per pixel always.
//  (2) All texture coordinates and texture sizes are in terms of 16-bit VRAM pixels.
//  (3) The alpha component is only used if alpha blending is being used.
//------------------------------------------------------------------------------------------------------------------------------------------
void addDrawWorldTriangle(
    const float x1,
    const float y1,
    const float z1,
    const float u1,
    const float v1,
    const float x2,
    const float y2,
    const float z2,
    const float u2,
    const float v2,
    const float x3,
    const float y3,
    const float z3,
    const float u3,
    const float v3,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const uint16_t clutX,
    const uint16_t clutY,
    const uint16_t texWinX,
    const uint16_t texWinY,
    const uint16_t texWinW,
    const uint16_t texWinH,
    const VLightDimMode lightDimMode,
    const uint8_t stMulR,
    const uint8_t stMulG,
    const uint8_t stMulB,
    const uint8_t stMulA
) noexcept {
    // Fill in the vertices, starting first with common parameters
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(3);

    for (uint32_t i = 0; i < 3; ++i) {
        VVertex_Draw& vert = pVerts[i];
        vert.r = r;
        vert.g = g;
        vert.b = b;
        vert.texWinX = texWinX;
        vert.texWinY = texWinY;
        vert.texWinW = texWinW;
        vert.texWinH = texWinH;
        vert.clutX = clutX;
        vert.clutY = clutY;
        vert.stmulR = stMulR;
        vert.stmulG = stMulG;
        vert.stmulB = stMulB;
        vert.stmulA = stMulA;
        vert.lightDimMode = lightDimMode;
    }

    // Fill in verts xy and uv positions
    pVerts[0].x = x1;   pVerts[0].y = y1;   pVerts[0].z = z1;
    pVerts[1].x = x2;   pVerts[1].y = y2;   pVerts[1].z = z2;
    pVerts[2].x = x3;   pVerts[2].y = y3;   pVerts[2].z = z3;

    pVerts[0].u = u1;   pVerts[0].v = v1;
    pVerts[1].u = u2;   pVerts[1].v = v2;
    pVerts[2].u = u3;   pVerts[2].v = v3;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a quadrilateral for the game's 3D view/world to the 'draw' subpass.
// 
// Notes:
//  (1) The texture format is assumed to be 8 bits per pixel always.
//  (2) All texture coordinates and texture sizes are in terms of 16-bit VRAM pixels.
//  (3) The alpha component is only used if alpha blending is being used.
//------------------------------------------------------------------------------------------------------------------------------------------
void addDrawWorldQuad(
    const float x1,
    const float y1,
    const float z1,
    const float u1,
    const float v1,
    const float x2,
    const float y2,
    const float z2,
    const float u2,
    const float v2,
    const float x3,
    const float y3,
    const float z3,
    const float u3,
    const float v3,
    const float x4,
    const float y4,
    const float z4,
    const float u4,
    const float v4,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const uint16_t clutX,
    const uint16_t clutY,
    const uint16_t texWinX,
    const uint16_t texWinY,
    const uint16_t texWinW,
    const uint16_t texWinH,
    const VLightDimMode lightDimMode,
    const uint8_t stMulR,
    const uint8_t stMulG,
    const uint8_t stMulB,
    const uint8_t stMulA
) noexcept {
    // Fill in the vertices, starting first with common parameters
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(6);

    for (uint32_t i = 0; i < 6; ++i) {
        VVertex_Draw& vert = pVerts[i];
        vert.r = r;
        vert.g = g;
        vert.b = b;
        vert.texWinX = texWinX;
        vert.texWinY = texWinY;
        vert.texWinW = texWinW;
        vert.texWinH = texWinH;
        vert.clutX = clutX;
        vert.clutY = clutY;
        vert.stmulR = stMulR;
        vert.stmulG = stMulG;
        vert.stmulB = stMulB;
        vert.stmulA = stMulA;
        vert.lightDimMode = lightDimMode;
    }

    // Fill in verts xy and uv positions
    pVerts[0].x = x1;   pVerts[0].y = y1;   pVerts[0].z = z1;
    pVerts[1].x = x2;   pVerts[1].y = y2;   pVerts[1].z = z2;
    pVerts[2].x = x3;   pVerts[2].y = y3;   pVerts[2].z = z3;
    pVerts[3].x = x3;   pVerts[3].y = y3;   pVerts[3].z = z3;
    pVerts[4].x = x4;   pVerts[4].y = y4;   pVerts[4].z = z4;
    pVerts[5].x = x1;   pVerts[5].y = y1;   pVerts[5].z = z1;

    pVerts[0].u = u1;   pVerts[0].v = v1;
    pVerts[1].u = u2;   pVerts[1].v = v2;
    pVerts[2].u = u3;   pVerts[2].v = v3;
    pVerts[3].u = u3;   pVerts[3].v = v3;
    pVerts[4].u = u4;   pVerts[4].v = v4;
    pVerts[5].u = u1;   pVerts[5].v = v1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a quadrilateral to the depth drawing subpass
//------------------------------------------------------------------------------------------------------------------------------------------
void addDepthWorldQuad(
    const float x1,
    const float y1,
    const float z1,
    const float x2,
    const float y2,
    const float z2,
    const float x3,
    const float y3,
    const float z3,
    const float x4,
    const float y4,
    const float z4
) noexcept {
    // Fill in the vertices
    VVertex_Depth* const pVerts = gVertexBuffers_Depth.allocVerts<VVertex_Depth>(6);

    pVerts[0].x = x1;   pVerts[0].y = y1;   pVerts[0].z = z1;
    pVerts[1].x = x2;   pVerts[1].y = y2;   pVerts[1].z = z2;
    pVerts[2].x = x3;   pVerts[2].y = y3;   pVerts[2].z = z3;

    pVerts[3].x = x3;   pVerts[3].y = y3;   pVerts[3].z = z3;
    pVerts[4].x = x4;   pVerts[4].y = y4;   pVerts[4].z = z4;
    pVerts[5].x = x1;   pVerts[5].y = y1;   pVerts[5].z = z1;
}

END_NAMESPACE(VDrawing)

#endif  // #if PSYDOOM_VULKAN_RENDERER
