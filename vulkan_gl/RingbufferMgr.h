#pragma once

#include "Asserts.h"
#include "Defines.h"
#include "Fence.h"

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Keeps track of which ringbuffer slot we are using for this frame of rendering.
// One ringbuffer slot is used per frame of rendering and the app cycles through the ring buffer slots in order to allow for
// CPU/GPU parallelism between the current and previous frames. In this way command buffers and so forth submitted for the previous frame
// can still be processed while the next frame's command buffers are being prepared.
//
// This manager also holds fences that are used to signal the end each ringbuffer use, which should be triggered by the renderer once
// it has completed it's rendering operations for the frame.
//
// Note that a ringbuffer doesn't exist as one discrete thing per say, instead it is a series of resources scattered about the application
// that are stored in arrays of the same length as the ring buffer. Each slot in these various resource arrays corresponds to one slot in
// the ringbuffer. Example of things that are ring buffered include the main view depth buffers, per frame uniform and vertex buffers,
// command buffers, lists of resources to be retired and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
class RingbufferMgr {
public:
    RingbufferMgr() noexcept;
    ~RingbufferMgr() noexcept;

    bool init(LogicalDevice& device) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }

    // Gets the current ringbuffer index to use
    inline uint8_t getBufferIndex() const noexcept {
        ASSERT(mbIsValid);
        return mBufferIndex;
    }

    Fence& getCurrentBufferFence() noexcept;
    uint8_t acquireNextBuffer() noexcept;
    void doCleanupForAllBufferSlots() noexcept;

private:
    // Copy and move are disallowed
    RingbufferMgr(const RingbufferMgr& other) = delete;
    RingbufferMgr(RingbufferMgr&& other) = delete;
    RingbufferMgr& operator = (const RingbufferMgr& other) = delete;
    RingbufferMgr& operator = (RingbufferMgr&& other) = delete;

    void doCleanupForBufferIndex(const uint8_t ringbufferIndex) noexcept;

    bool            mbIsValid;
    uint8_t         mBufferIndex;                           // Which ringbuffer we are currently on
    LogicalDevice*  mpDevice;
    Fence           mFences[Defines::RINGBUFFER_SIZE];      // Fences used to signal the end of rendering for each ringbuffer
};

END_NAMESPACE(vgl)
