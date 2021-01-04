#include "RingbufferMgr.h"

#include "Finally.h"
#include "Fence.h"
#include "LogicalDevice.h"
#include "RetirementMgr.h"
#include "TransferMgr.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an invalid/uninitialized ringbuffer manager
//------------------------------------------------------------------------------------------------------------------------------------------
RingbufferMgr::RingbufferMgr() noexcept
    : mbIsValid(false)
    , mBufferIndex(0)
    , mpDevice(nullptr)
    , mFences{}
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the ringbuffer manager
//------------------------------------------------------------------------------------------------------------------------------------------
RingbufferMgr::~RingbufferMgr() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the ringbuffer manager for the given device
//------------------------------------------------------------------------------------------------------------------------------------------
bool RingbufferMgr::init(LogicalDevice& device) noexcept {
    // Preconditions
    ASSERT_LOG(!mbIsValid, "Must call destroy() before re-initializing!");
    
    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    mpDevice = &device;

    // Create the first fence unsignalled since it will be used first and every other fence signalled
    // so they will be marked as 'ready' when we go to flip over to the next ringbuffer:
    static_assert(Defines::RINGBUFFER_SIZE >= 1);

    {
        Fence* pCurFence = mFences;

        if (!pCurFence->init(device, Fence::InitMode::UNSIGNALLED))
            return false;
        
        Fence* const pEndFence = pCurFence + Defines::RINGBUFFER_SIZE;
        ++pCurFence;
        
        while (pCurFence < pEndFence) {
            if (!pCurFence->init(device, Fence::InitMode::SIGNALLED))
                return false;

            ++pCurFence;
        }
    }

    // All went well if we got to here!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroy the ringbuffer manager
//------------------------------------------------------------------------------------------------------------------------------------------
void RingbufferMgr::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Cleanup everything
    mbIsValid = false;

    {
        Fence* pCurFence = mFences;
        Fence* const pEndFence = pCurFence + Defines::RINGBUFFER_SIZE;
        
        while (pCurFence < pEndFence) {
            pCurFence->destroy();
            ++pCurFence;
        }
    }
    
    mpDevice = nullptr;
    mBufferIndex = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the fence used to signal the end of rendering for the current ring buffer.
// The renderer should make sure this gets signalled as part of the final draw submission.
//------------------------------------------------------------------------------------------------------------------------------------------
Fence& RingbufferMgr::getCurrentBufferFence() noexcept {
    ASSERT(mbIsValid);
    return mFences[mBufferIndex];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move onto the next ring buffer and return it's index.
// Acquires the fence for the next ring buffer and waits until it is signalled if neccessary.
// Also cleans up dead resources in the retirement manager for the outgoing ringbuffer after the switch.
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t RingbufferMgr::acquireNextBuffer() noexcept {
    ASSERT(mbIsValid);

    // Wait for the fence for the next buffer to become signalled
    const uint8_t nextBufferIdx = (mBufferIndex + 1) % Defines::RINGBUFFER_SIZE;
    Fence& nextRingbufferFence = mFences[nextBufferIdx];
    nextRingbufferFence.waitUntilSignalled();

    // Cleanup all resources that we can for this ringbuffer index
    doCleanupForBufferIndex(nextBufferIdx);
    
    // Reset the fence for this ringbuffer slot.
    // It will be signalled again once all operations have completed for this ringbuffer slot:
    nextRingbufferFence.resetSignal();

    // Move onto the next ringbuffer slot
    mBufferIndex = nextBufferIdx;
    return nextBufferIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does cleanup for all ringbuffer slots.
// Releases all retired resources for all ringbuffer indices, temporary memory used in transfers etc.
// This should ONLY be called when the device is fully idle.
//------------------------------------------------------------------------------------------------------------------------------------------
void RingbufferMgr::doCleanupForAllBufferSlots() noexcept {
    ASSERT(mbIsValid);

    for (uint8_t bufferIdx = 0; bufferIdx < Defines::RINGBUFFER_SIZE; ++bufferIdx) {
        doCleanupForBufferIndex(bufferIdx);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do cleanup for a particular ringbuffer slot
//------------------------------------------------------------------------------------------------------------------------------------------
void RingbufferMgr::doCleanupForBufferIndex(const uint8_t ringbufferIndex) noexcept {
    ASSERT(mbIsValid);

    // Cleanup temporary buffers used for transfers for this slot
    TransferMgr& transferMgr = mpDevice->getTransferMgr();

    if (transferMgr.isValid()) {
        transferMgr.doCleanupForRingbufferIndex(ringbufferIndex);
    }

    // Release retired resources for this slot.
    // N.B: Do AFTER temporary transfer cleanup just in case that retires stuff.
    RetirementMgr& retirementMgr = mpDevice->getRetirementMgr();

    if (retirementMgr.isValid()) {
        retirementMgr.freeRetiredResourcesForRingbufferIndex(ringbufferIndex);
    }
}

END_NAMESPACE(vgl)
