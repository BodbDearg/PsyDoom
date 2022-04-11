#include "VScreenQuad.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
#include "CmdBufferRecorder.h"
#include "VTypes.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Create an uninitialized screen quad and destroy it completely
//------------------------------------------------------------------------------------------------------------------------------------------
VScreenQuad::VScreenQuad() noexcept : mVertexBuffer() {}
VScreenQuad::~VScreenQuad() noexcept = default;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the screen quad.
// This operation is only allowed to succeed: issues a fatal error if not.
//------------------------------------------------------------------------------------------------------------------------------------------
void VScreenQuad::init(vgl::LogicalDevice& device) noexcept {
    ASSERT_LOG(!isValid(), "Can't initialize twice!");

    // Create the buffer
    const bool bCreatedBufferOk = mVertexBuffer.initWithElementCount<VVertex_XyUv>(
        device,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        vgl::BufferUsageMode::STATIC,
        6
    );

    if (!bCreatedBufferOk)
        FatalErrors::raise("VScreenQuad::init: failed to initialize a required vertex buffer!");

    // Lock the buffer and populate it
    VVertex_XyUv* const pv = mVertexBuffer.lockElements<VVertex_XyUv>(0, 6);

    if (!pv)
        FatalErrors::raise("VScreenQuad::init: failed to lock a required vertex buffer!");

    pv[0].x = -1.0f;    pv[0].y = -1.0f;    pv[0].u = 0.0f;     pv[0].v = 0.0f;
    pv[1].x = +1.0f;    pv[1].y = -1.0f;    pv[1].u = 1.0f;     pv[1].v = 0.0f;
    pv[2].x = +1.0f;    pv[2].y = +1.0f;    pv[2].u = 1.0f;     pv[2].v = 1.0f;
    pv[3].x = +1.0f;    pv[3].y = +1.0f;    pv[3].u = 1.0f;     pv[3].v = 1.0f;
    pv[4].x = -1.0f;    pv[4].y = +1.0f;    pv[4].u = 0.0f;     pv[4].v = 1.0f;
    pv[5].x = -1.0f;    pv[5].y = -1.0f;    pv[5].u = 0.0f;     pv[5].v = 0.0f;

    mVertexBuffer.unlockElements<VVertex_XyUv>(6);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the screen quad's vertex buffer and releases all its resources.
// Optionally, the vertex buffer can be cleaned up immediately.
//------------------------------------------------------------------------------------------------------------------------------------------
void VScreenQuad::destroy(const bool bImmediateCleanup) noexcept {
    mVertexBuffer.destroy(bImmediateCleanup);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the screen quad has been validly initialized
//------------------------------------------------------------------------------------------------------------------------------------------
bool VScreenQuad::isValid() const noexcept {
    return mVertexBuffer.isValid();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Binds the vertex buffer for the screen quad using the specified command recorder.
// It's bound to the specified vertex buffer binding index.
// This must be called prior to drawing!
//------------------------------------------------------------------------------------------------------------------------------------------
void VScreenQuad::bindVertexBuffer(vgl::CmdBufferRecorder& cmdRec, const uint32_t bindingIdx) noexcept {
    ASSERT(isValid());
    cmdRec.bindVertexBuffer(mVertexBuffer, bindingIdx, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Issues the command to draw the screen quad.
// Note that 'bindVertexBuffer()' must first be called beforehand to bind the vertex buffer for the quad.
//------------------------------------------------------------------------------------------------------------------------------------------
void VScreenQuad::draw(vgl::CmdBufferRecorder& cmdRec) noexcept {
    ASSERT(isValid());
    cmdRec.draw(6, 0);
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
