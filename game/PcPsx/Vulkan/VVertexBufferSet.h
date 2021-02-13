#pragma once

#if PSYDOOM_MODS

#include "Asserts.h"
#include "Buffer.h"
#include "Defines.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds a collection of vertex buffers (one per ringbuffer slot) for a specified vertex type.
// Keeps track of where we are in the vertex buffer, the current draw batch location and size and so on.
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertexBufferSet {
    vgl::Buffer     buffers[vgl::Defines::RINGBUFFER_SIZE];     // The vertex buffers for each ringbuffer slot
    uint32_t        vertexSize;                                 // The size of each vertex
    vgl::Buffer*    pCurBuffer;                                 // The the vertex buffer to use from the current ringbuffer slot
    uint32_t        curOffset;                                  // How many vertices have been placed in the vertex buffer
    uint32_t        curSize;                                    // Size of the vertex buffer in vertexes
    std::byte*      pCurVerts;                                  // Pointer to the vertex data which can be written to
    uint32_t        curBatchStart;                              // Where the current draw batch in the vertex buffer starts (which vertex number)
    uint32_t        curBatchSize;                               // Size of the current draw batch in the vertedx buffer (in vertices)

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Initializes the vertex buffer set for the given vertex type and capacity
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <class VertT>
    void init(vgl::LogicalDevice& device, const uint32_t numVerts) noexcept {
        // Init basic fields
        vertexSize = sizeof(VertT);
        pCurBuffer = nullptr;
        curOffset = 0;
        curSize = numVerts;
        pCurVerts = nullptr;
        curBatchStart = 0;
        curBatchSize = 0;

        // Create the vertex buffers for each ringbuffer slot
        for (vgl::Buffer& buffer : buffers) {
            const bool bWasSuccessful = buffer.initWithElementCount<VertT>(
                device,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                vgl::BufferUsageMode::DYNAMIC,
                numVerts,
                true
            );

            if (!bWasSuccessful)
                FatalErrors::raise("Failed to create a required Vulkan vertex buffer! May be out of memory!");
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Tears down the vertex buffer set
    //--------------------------------------------------------------------------------------------------------------------------------------
    void destroy() noexcept {
        vertexSize = 0;
        pCurBuffer = nullptr;
        curOffset = 0;
        curSize = 0;
        pCurVerts = nullptr;
        curBatchStart = 0;
        curBatchSize = 0;

        for (vgl::Buffer& buffer : buffers) {
            buffer.destroy(true);
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Should be called at the beginning of a frame.
    // Decides which vertex buffer to use based on the ringbuffer index, and locks it for writing.
    //--------------------------------------------------------------------------------------------------------------------------------------
    void beginFrame(const uint32_t ringbufferIdx) noexcept {
        // Get what vertex buffer to use and lock its entire range for writing
        pCurBuffer = &buffers[ringbufferIdx];
        curOffset = 0;
        curSize = (uint32_t)(pCurBuffer->getSizeInBytes() / vertexSize);
        pCurVerts = pCurBuffer->lockBytes(0, pCurBuffer->getSizeInBytes());
        ASSERT(pCurVerts);

        // These should already be zeroed
        ASSERT(curBatchStart == 0);
        ASSERT(curBatchSize == 0);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Should be called at the end of a frame.
    // Schedules uploads for any vertices that need to be uploaded to the GPU.
    //--------------------------------------------------------------------------------------------------------------------------------------
    void endFrame() noexcept {
        // Unlock the vertex buffer used to schedule the transfer of vertex data to the GPU
        ASSERT(pCurBuffer);
        pCurBuffer->unlockBytes(curOffset * vertexSize);
        pCurVerts = nullptr;

        // Clear everything else
        pCurBuffer = nullptr;
        curOffset = 0;
        curSize = 0;
        curBatchStart = 0;
        curBatchSize = 0;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Ensures the specified number of vertices are free for use, and if they are not resizes the current vertex buffer in use.
    // Note: must only be called after 'beginFrame', i.e a ringbuffer slot MUST be decided.
    //--------------------------------------------------------------------------------------------------------------------------------------
    void ensureNumVerts(const uint32_t numVerts) noexcept {
        // The vertex buffer must be locked by 'beginFrame' and we must have a valid vertex size set
        ASSERT(vertexSize > 0);
        ASSERT(pCurBuffer);

        // What size would be good to hold this amount of vertices?
        uint32_t newSize = curSize;

        while (curOffset + numVerts > newSize) {
            newSize *= 2;
        }

        // Do we need to do a resize?
        if (newSize != curSize) {
            const bool bResizeOk = pCurBuffer->resizeToByteCount(
                (uint64_t) newSize * vertexSize,
                vgl::Buffer::ResizeFlagBits::KEEP_LOCKED_DATA,
                0,
                UINT64_MAX      // Re-lock the entire buffer
            );

            if (!bResizeOk)
                FatalErrors::raise("Failed to resize a Vulkan vertex buffer used for rendering! May be out of memory!");

            // Need to update the locked pointer after resizing and the new vertex buffer size
            curSize = (uint32_t)(pCurBuffer->getSizeInBytes() / vertexSize);
            pCurVerts = pCurBuffer->getLockedBytes();
            ASSERT(pCurVerts);
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Allocate the specified number of vertices from the buffer for use and move on.
    // 
    // Notes:
    //  (1) If the buffer is not big enough then it is resized.
    //  (2) Must only be called after 'beginFrame', i.e a ringbuffer slot MUST be decided.
    //  (3) May invalidate previously allocated vertex pointers, be careful!
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <class VertT>
    VertT* allocVerts(const uint32_t numVerts) noexcept {
        // Make sure the vertex is big enough, then get the vertex pointer
        ASSERT_LOG(sizeof(VertT) == vertexSize, "Vertex size inconsistency!");
        ensureNumVerts(numVerts);
        VertT* const pVerts = (VertT*) pCurVerts + curOffset;

        // Mark these vertices as consumed
        curOffset += numVerts;
        curBatchSize += numVerts;
        return pVerts;
    }

    //------------------------------------------------------------------------------------------------------------------------------------------
    // Marks the current draw batch as ended
    //------------------------------------------------------------------------------------------------------------------------------------------
    void endCurrentDrawBatch() noexcept {
        if (curBatchSize > 0) {
            curBatchStart = curOffset;
            curBatchSize = 0;
        }
    }
};

#endif  // #if PSYDOOM_MODS
