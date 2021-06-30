//------------------------------------------------------------------------------------------------------------------------------------------
// A module that handles the submission of draw commands to the Vulkan renderer.
// Allows various primitives to be submitted.
// Records the vertex data into a buffer and the commands into the current command buffer used by the renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VDrawing.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
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
#include "PsyDoom/Config.h"
#include "PsyDoom/Video.h"
#include "VPipelines.h"
#include "VRenderer.h"
#include "VTypes.h"
#include "VVertexBufferSet.h"

BEGIN_NAMESPACE(VDrawing)

//------------------------------------------------------------------------------------------------------------------------------------------
// Enum representing a type of draw command. We store submitted draw commands in a list and record the Vulkan command buffer at the end
// of the frame, once the vertex buffers are finalized and have known sizes.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class DrawCmdType : uint32_t {
    SetPipeline,        // Set the graphics pipeline to use: 1st arg is pipeline type, 2nd arg unused
    SetUniforms,        // Set the uniforms to use: 1st arg is index in the uniforms list
    Draw                // A command to draw primitives: 1st arg is vertex count, 2nd arg is vertex offset
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Stores a single drawing command: the meaning of the arguments depends on the command type
//------------------------------------------------------------------------------------------------------------------------------------------
struct DrawCmd {
    DrawCmdType     type;
    uint32_t        arg1;
    uint32_t        arg2;
};

// Ringbuffer index for the current frame being generated
static uint32_t gCurRingbufferIdx;

// Descriptor set and a descriptor pool used for all 'draw' subpass operations.
// Binds the PSX VRAM texture to it's combined image sampler in binding 0.
static vgl::DescriptorPool  gDescriptorPool;
static vgl::DescriptorSet*  gpDescriptorSet;

// Vertex buffers: for the 'draw' subpass (VVertex_Draw)
static VVertexBufferSet gVertexBuffers_Draw;

// The current pipeline being used by the 'draw' subpass; used to help avoid unneccessary pipeline switches
static VPipelineType gCurDrawPipelineType;

// Sets of uniforms for the current frame
static std::vector<VShaderUniforms_Draw> gFrameUniforms;

// Drawing commands for the current frame
static std::vector<DrawCmd> gFrameDrawCmds;

//------------------------------------------------------------------------------------------------------------------------------------------
// Records all drawing commands for the current frame to a Vulkan command buffer
//------------------------------------------------------------------------------------------------------------------------------------------
static void recordCmdBuffer(vgl::CmdBufferRecorder& cmdRec) noexcept {
    // First command in the drawing pass is to setup the viewport. Note that while the view is allowed to extend horizontally if widescreen
    // is enabled, no extension is allowed vertically; instead, letterboxing will happen. I considered allowing a vertically long display
    // but it won't work with the UI assets & design that Doom uses. I'm also not sure why someone want to play that way anyway...
    const bool bAllowWidescreen = Config::gbVulkanWidescreenEnabled;
    const float viewportX = (bAllowWidescreen) ? 0 : VRenderer::gPsxCoordsFbX;
    const float viewportY = VRenderer::gPsxCoordsFbY;
    const float viewportW = (bAllowWidescreen) ? (float) VRenderer::gFramebufferW : VRenderer::gPsxCoordsFbW;
    const float viewportH = VRenderer::gPsxCoordsFbH;

    const int32_t viewportXInt = (int32_t)(viewportX);
    const int32_t viewportYInt = (int32_t)(viewportY);
    const int32_t viewportWInt = (int32_t)(viewportX + viewportW) - viewportXInt;
    const int32_t viewportHInt = (int32_t)(viewportY + viewportH) - viewportYInt;

    cmdRec.setViewport((float) viewportXInt, (float) viewportYInt, (float) viewportWInt, (float) viewportHInt, 0.0f, 1.0f);
    cmdRec.setScissors(viewportXInt, viewportYInt, viewportWInt, viewportHInt);

    // Bind the correct vertex buffer for drawing
    cmdRec.bindVertexBuffer(*gVertexBuffers_Draw.pCurBuffer, 0, 0);

    // Clear this flag once we bind the drawing descriptor set - it only needs to be done once since all draw pipeline layouts are compatible
    bool bNeedToBindDescriptorSet = true;

    // Handle each draw command
    for (const DrawCmd& drawCmd : gFrameDrawCmds) {
        switch (drawCmd.type) {
            case DrawCmdType::SetPipeline: {
                // Bind the pipeline
                ASSERT(drawCmd.arg1 < (uint32_t) VPipelineType::NUM_TYPES);
                vgl::Pipeline& pipeline = VPipelines::gPipelines[drawCmd.arg1];
                cmdRec.bindPipeline(pipeline);

                // Do we need to bind the draw descriptor set as well, after setting the pipeline?
                if (bNeedToBindDescriptorSet) {
                    cmdRec.bindDescriptorSet(*gpDescriptorSet, pipeline, 0, 0, nullptr);
                    bNeedToBindDescriptorSet = false;
                }
            }   break;

            case DrawCmdType::SetUniforms: {
                cmdRec.pushConstants(
                    VPipelines::gPipelineLayout_draw,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(VShaderUniforms_Draw),
                    &gFrameUniforms[drawCmd.arg1]
                );
            }   break;

            case DrawCmdType::Draw: {
                cmdRec.draw(drawCmd.arg1, drawCmd.arg2);
            }   break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the drawing module and allocates draw vertex buffers etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void init(vgl::LogicalDevice& device, vgl::BaseTexture& vramTex) noexcept {
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

    // Create the vertex buffers
    constexpr uint32_t DRAW_VB_SIZE = 4 * 1024 * 1024;
    gVertexBuffers_Draw.init<VVertex_Draw>(device, DRAW_VB_SIZE / sizeof(VVertex_Draw));

    // Current draw pipeline in use is undefined initially
    gCurDrawPipelineType = (VPipelineType) -1;

    // Prealloc draw buffer memory
    gFrameUniforms.reserve(16);
    gFrameDrawCmds.reserve(4196);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the drawing module and frees up resources
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gFrameDrawCmds.clear();
    gFrameUniforms.clear();
    gCurDrawPipelineType = {};
    gVertexBuffers_Draw.destroy();

    if (gpDescriptorSet) {
        gpDescriptorSet->free(true);
        gpDescriptorSet = nullptr;
    }

    gDescriptorPool.destroy(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs start of frame logic for the drawing module
//------------------------------------------------------------------------------------------------------------------------------------------
void beginFrame(const uint32_t ringbufferIdx) noexcept {
    // Remember which ringbuffer slot we are on and setup vertex buffers for the frame
    gCurRingbufferIdx = ringbufferIdx;
    gVertexBuffers_Draw.beginFrame(ringbufferIdx);

    // Expect all this to already have the following state
    ASSERT(gCurDrawPipelineType == (VPipelineType) -1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs end of frame logic for the drawing module
//------------------------------------------------------------------------------------------------------------------------------------------
void endFrame(vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Finish the current draw batch then record all drawing commands in the Vulkan command buffer
    endCurrentDrawBatch();
    recordCmdBuffer(cmdRec);

    // Upload vertices generated during drawing, so the draw commands can use them
    gVertexBuffers_Draw.endFrame();

    // Post frame cleanup: clear buffers, the current draw pipeline and ringbuffer index
    gFrameDrawCmds.clear();
    gFrameUniforms.clear();
    gCurDrawPipelineType = (VPipelineType) -1;
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

    // Do the pipeline switch and record the switch draw command.
    // Note that we must end the current draw batch before switching!
    endCurrentDrawBatch();
    gCurDrawPipelineType = type;

    DrawCmd& drawCmd = gFrameDrawCmds.emplace_back();
    drawCmd.type = DrawCmdType::SetPipeline;
    drawCmd.arg1 = (uint32_t) type;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set uniforms used by draw shaders, including the model/view/projection transform matrix
//------------------------------------------------------------------------------------------------------------------------------------------
void setDrawUniforms(const VShaderUniforms_Draw& uniforms) noexcept {
    // Record the command to set the uniforms and save them for later
    DrawCmd& drawCmd = gFrameDrawCmds.emplace_back();
    drawCmd.type = DrawCmdType::SetUniforms;
    drawCmd.arg1 = (uint32_t) gFrameUniforms.size();

    gFrameUniforms.emplace_back(uniforms);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Compute the transform matrix for the UI/2D elements in Doom.
// Scales and positions everything such that the original UI coordinates can be used for the same result.
//------------------------------------------------------------------------------------------------------------------------------------------
Matrix4f computeTransformMatrixForUI(const bool bAllowWidescreen) noexcept {
    // The UI is centered horizontally in the viewport (if we are in widescreen mode) and occupies it's full vertical range.
    // Compute how much leftover space there would be at the left and right due to widescreen.
    const float xPadding = (VRenderer::gPsxCoordsFbX / VRenderer::gPsxCoordsFbW) * (float) SCREEN_W;

    // These are the projection parameters.
    // Note: need to reverse by/ty to get the view the right way round (normally y is up with projection matrices).
    const float viewLx = (bAllowWidescreen) ? -xPadding : 0.0f;
    const float viewRx = (bAllowWidescreen) ? (float) SCREEN_W + xPadding : (float) SCREEN_W;
    const float viewTy = (float) Video::gTopOverscan;
    const float viewBy = (float) SCREEN_H - (float) Video::gBotOverscan;
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
    // This matches the projection that the original PSX Doom used, with allowance for widescreen:
    constexpr float STATUS_BAR_H = SCREEN_H - VIEW_3D_H;

    const bool bAllowWidescreen = Config::gbVulkanWidescreenEnabled;
    const float widescreenScale = std::max((float) VRenderer::gFramebufferW / (float) VRenderer::gPsxCoordsFbW, 1.0f);
    const float viewLx = (bAllowWidescreen) ? -widescreenScale : -1.0f;
    const float viewRx = (bAllowWidescreen) ? +widescreenScale : +1.0f;
    const float viewTy = (HALF_VIEW_3D_H - (float) Video::gTopOverscan) / (float) HALF_SCREEN_W;
    const float viewBy = (-HALF_VIEW_3D_H - STATUS_BAR_H + (float) Video::gBotOverscan) / (float) HALF_SCREEN_W;
    const float zNear = 1.0f;
    const float zFar = 65536.0f;

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
    // Ignore if there are no vertices in the current batch
    if (gVertexBuffers_Draw.curBatchSize <= 0)
        return;

    // Record the draw command
    DrawCmd& drawCmd = gFrameDrawCmds.emplace_back();
    drawCmd.type = DrawCmdType::Draw;
    drawCmd.arg1 = gVertexBuffers_Draw.curBatchSize;
    drawCmd.arg2 = gVertexBuffers_Draw.curBatchStart;
    gVertexBuffers_Draw.endCurrentDrawBatch();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a 2D/UI line to the 'draw' subpass
//------------------------------------------------------------------------------------------------------------------------------------------
void addUILine(
    const float x1,
    const float y1,
    const float x2,
    const float y2,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b
) noexcept {
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
        vert.stmulA = 128;
    }

    // Fill in xy positions
    pVerts[0].x = x1;   pVerts[0].y = y1;
    pVerts[1].x = x2;   pVerts[1].y = y2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a flat colored triangle to the 'draw' subpass
//------------------------------------------------------------------------------------------------------------------------------------------
void addFlatColoredTriangle(
    const float x1,
    const float y1,
    const float z1,
    const float x2,
    const float y2,
    const float z2,
    const float x3,
    const float y3,
    const float z3,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b
) noexcept {
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(3);

    for (uint32_t i = 0; i < 3; ++i) {
        VVertex_Draw& vert = pVerts[i];
        vert = {};
        vert.r = r;
        vert.g = g;
        vert.b = b;
        vert.stmulR = 128;
        vert.stmulG = 128;
        vert.stmulB = 128;
        vert.stmulA = 128;
    }

    pVerts[0].x = x1;   pVerts[0].y = y1;   pVerts[0].z = z1;
    pVerts[1].x = x2;   pVerts[1].y = y2;   pVerts[1].z = z2;
    pVerts[2].x = x3;   pVerts[2].y = y3;   pVerts[2].z = z3;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a flat colored quad to the 'draw' subpass
//------------------------------------------------------------------------------------------------------------------------------------------
void addFlatColoredQuad(
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
    const float z4,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b
) noexcept {
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(6);

    for (uint32_t i = 0; i < 6; ++i) {
        VVertex_Draw& vert = pVerts[i];
        vert = {};
        vert.r = r;
        vert.g = g;
        vert.b = b;
        vert.stmulR = 128;
        vert.stmulG = 128;
        vert.stmulB = 128;
        vert.stmulA = 128;
    }

    pVerts[0].x = x1;   pVerts[0].y = y1;   pVerts[0].z = z1;
    pVerts[1].x = x2;   pVerts[1].y = y2;   pVerts[1].z = z2;
    pVerts[2].x = x3;   pVerts[2].y = y3;   pVerts[2].z = z3;
    pVerts[3].x = x3;   pVerts[3].y = y3;   pVerts[3].z = z3;
    pVerts[4].x = x4;   pVerts[4].y = y4;   pVerts[4].z = z4;
    pVerts[5].x = x1;   pVerts[5].y = y1;   pVerts[5].z = z1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a UI sprite to the 'draw' subpass
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
    const uint16_t clutX,
    const uint16_t clutY,
    const uint16_t texPageX,
    const uint16_t texPageY,
    const uint16_t texWinW,
    const uint16_t texWinH
) noexcept {
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
    const float xl = x;
    const float xr = x + w;
    const float yt = y;
    const float yb = y + h;

    const float ul = u;
    const float ur = u + w;
    const float vt = v;
    const float vb = v + h;

    pVerts[0].x = xl;   pVerts[0].y = yt;
    pVerts[1].x = xr;   pVerts[1].y = yt;
    pVerts[2].x = xr;   pVerts[2].y = yb;
    pVerts[3].x = xr;   pVerts[3].y = yb;
    pVerts[4].x = xl;   pVerts[4].y = yb;
    pVerts[5].x = xl;   pVerts[5].y = yt;

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
//  (2) All texture coordinates and texture sizes are in terms of 8-bit pixels (not VRAM 16-bit pixels).
//  (3) The alpha component is only used if alpha blending is being used.
//------------------------------------------------------------------------------------------------------------------------------------------
void addWorldTriangle(
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
//  (2) All texture coordinates and texture sizes are in terms of 8-bit pixels (not VRAM 16-bit pixels).
//  (3) The alpha component is only used if alpha blending is being used.
//------------------------------------------------------------------------------------------------------------------------------------------
void addWorldQuad(
    const AddWorldQuadVert& v1,
    const AddWorldQuadVert& v2,
    const AddWorldQuadVert& v3,
    const AddWorldQuadVert& v4,
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
    // Fill in the vertices, starting first with the parameters that are the same for all vertices
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(6);

    for (uint32_t i = 0; i < 6; ++i) {
        VVertex_Draw& vert = pVerts[i];
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

    // Fill in the unique vertex attributes
    constexpr auto assignVertexUniqueAttribs = [](VVertex_Draw& dst, const AddWorldQuadVert& src) noexcept {
        dst.x = src.x;
        dst.y = src.y;
        dst.z = src.z;
        dst.u = src.u;
        dst.v = src.v;
        dst.r = src.r;
        dst.g = src.g;
        dst.b = src.b;
    };

    assignVertexUniqueAttribs(pVerts[0], v1);
    assignVertexUniqueAttribs(pVerts[1], v2);
    assignVertexUniqueAttribs(pVerts[2], v3);
    assignVertexUniqueAttribs(pVerts[3], v3);
    assignVertexUniqueAttribs(pVerts[4], v4);
    assignVertexUniqueAttribs(pVerts[5], v1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a vertical quad for the sky to the 'draw' subpass.
// The y coordinate where the sky starts and the 2 endpoints are specified only, along with whether it is an upper or lower sky wall.
// The sky wall is stretched past the top or bottom of the screen.
//------------------------------------------------------------------------------------------------------------------------------------------
void addWorldInfiniteSkyWall(
    const float x1,
    const float z1,
    const float x2,
    const float z2,
    const float y,
    const bool bIsUpperSkyWall,
    const float skyUOffset,
    const uint16_t clutX,
    const uint16_t clutY,
    const uint16_t texWinX,
    const uint16_t texWinY,
    const uint16_t texWinW,
    const uint16_t texWinH
) noexcept {
    // Fill in the vertices, starting first with common parameters.
    // Note: we store the sky U offset based on player rotation in the U coordinate.
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(6);

    for (uint32_t i = 0; i < 6; ++i) {
        VVertex_Draw& vert = pVerts[i];
        vert.y = y;
        vert.r = 128;
        vert.g = 128;
        vert.b = 128;
        vert.lightDimMode = VLightDimMode::None;    // Unused for sky rendering
        vert.u = skyUOffset;
        vert.texWinX = texWinX;
        vert.texWinY = texWinY;
        vert.texWinW = texWinW;
        vert.texWinH = texWinH;
        vert.clutX = clutX;
        vert.clutY = clutY;
        vert.stmulR = 128;
        vert.stmulG = 128;
        vert.stmulB = 128;
        vert.stmulA = 128;
    }

    // Note: the 'v' coordinate is used to determine whether the vertex is for the bottom or the top of the sky wall.
    // If the 'v' coord is '-1' or '+1' then it means the vertex should be stretched past the top (+1) or bottom (-1) of the screen.
    pVerts[0].x = x1;   pVerts[0].z = z1;
    pVerts[1].x = x2;   pVerts[1].z = z2;
    pVerts[2].x = x2;   pVerts[2].z = z2;
    pVerts[3].x = x2;   pVerts[3].z = z2;
    pVerts[4].x = x1;   pVerts[4].z = z1;
    pVerts[5].x = x1;   pVerts[5].z = z1;

    if (bIsUpperSkyWall) {
        pVerts[0].v = 0.0f;
        pVerts[1].v = 1.0f;
        pVerts[2].v = 0.0f;
        pVerts[3].v = 1.0f;
        pVerts[4].v = 0.0f;
        pVerts[5].v = 1.0f;
    } else {
        pVerts[0].v = -1.0f;
        pVerts[1].v = 0.0f;
        pVerts[2].v = -1.0f;
        pVerts[3].v = 0.0f;
        pVerts[4].v = -1.0f;
        pVerts[5].v = 0.0f;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a quad for the sky to the 'draw' subpass. Unlike the infinite sky wall, this quad is not stretched in any way
//------------------------------------------------------------------------------------------------------------------------------------------
void addWorldSkyQuad(
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
    const float z4,
    const float skyUOffset,
    const uint16_t clutX,
    const uint16_t clutY,
    const uint16_t texWinX,
    const uint16_t texWinY,
    const uint16_t texWinW,
    const uint16_t texWinH
) noexcept {
    // Fill in the vertices, starting first with common parameters.
    // Note: we store the sky U offset based on player rotation in the U coordinate.
    VVertex_Draw* const pVerts = gVertexBuffers_Draw.allocVerts<VVertex_Draw>(6);

    for (uint32_t i = 0; i < 6; ++i) {
        VVertex_Draw& vert = pVerts[i];
        vert.r = 128;
        vert.g = 128;
        vert.b = 128;
        vert.lightDimMode = VLightDimMode::None;    // Unused for sky rendering
        vert.u = skyUOffset;
        vert.v = 0.0f;
        vert.texWinX = texWinX;
        vert.texWinY = texWinY;
        vert.texWinW = texWinW;
        vert.texWinH = texWinH;
        vert.clutX = clutX;
        vert.clutY = clutY;
        vert.stmulR = 128;
        vert.stmulG = 128;
        vert.stmulB = 128;
        vert.stmulA = 128;
    }

    // Fill the rest of the coords
    pVerts[0].x = x1;   pVerts[0].y = y1;   pVerts[0].z = z1;
    pVerts[1].x = x2;   pVerts[1].y = y2;   pVerts[1].z = z2;
    pVerts[2].x = x3;   pVerts[2].y = y3;   pVerts[2].z = z3;
    pVerts[3].x = x3;   pVerts[3].y = y3;   pVerts[3].z = z3;
    pVerts[4].x = x4;   pVerts[4].y = y4;   pVerts[4].z = z4;
    pVerts[5].x = x1;   pVerts[5].y = y1;   pVerts[5].z = z1;
}

END_NAMESPACE(VDrawing)

#endif  // #if PSYDOOM_VULKAN_RENDERER
