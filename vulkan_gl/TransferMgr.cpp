#include "TransferMgr.h"

#include "CmdBuffer.h"
#include "CmdBufferRecorder.h"
#include "CmdBufferWaitCond.h"
#include "CmdPool.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "RawBuffer.h"
#include "RingbufferMgr.h"
#include "TransferTask.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized transfer manager
//------------------------------------------------------------------------------------------------------------------------------------------
TransferMgr::TransferMgr() noexcept
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mpRingbufferMgr(nullptr)
    , mPreFrameTransferTask()
    , mRingbufferSlots()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the transfer manager
//------------------------------------------------------------------------------------------------------------------------------------------
TransferMgr::~TransferMgr() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the transfer manager for the given device
//------------------------------------------------------------------------------------------------------------------------------------------
bool TransferMgr::init(LogicalDevice& device) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getCmdPool().isValid());
    ASSERT(device.getRingbufferMgr().isValid());

    // Save for later use
    mpDevice = &device;
    mpRingbufferMgr = &device.getRingbufferMgr();

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // Initialize each of the ringbuffer slots
    for (RingbufferSlot& slot : mRingbufferSlots) {
        if (!slot.cmdBuffer.init(device.getCmdPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY))
            return false;

        slot.tmpStagingBuffers.reserve(32);
    }

    // Successful at this point!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the transfer manager and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void TransferMgr::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    mbIsValid = false;
    ASSERT((!mpDevice) || mpDevice->getVkDevice());

    for (RingbufferSlot& slot : mRingbufferSlots) {
        slot.cmdBuffer.destroy();
        slot.tmpStagingBuffers.clear();
        slot.tmpStagingBuffers.shrink_to_fit();
    }

    mPreFrameTransferTask.clearCmds(true);
    mpRingbufferMgr = nullptr;
    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to allocate a temporary raw staging buffer that can be used to schedule transfers.
// The buffer is automatically destroyed the next time we start a frame on the current ringbuffer index.
//------------------------------------------------------------------------------------------------------------------------------------------
TransferMgr::StagingBuffer TransferMgr::allocTempStagingBuffer(const uint32_t numBytes) noexcept {
    // Preconditions
    ASSERT(mbIsValid);
    ASSERT(mpDevice && mpDevice->getVkDevice());

    // Alloc the raw staging buffer
    RingbufferSlot& ringbufferSlot = getCurrentRingbufferSlot();
    std::vector<RawBuffer>& tmpBuffers = ringbufferSlot.tmpStagingBuffers;
    RawBuffer& tmpBuffer = tmpBuffers.emplace_back();
    
    if (!tmpBuffer.init(
            *mpDevice,
            numBytes,
            DeviceMemAllocMode::REQUIRE_HOST_VISIBLE,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT            // Used as a transfer source only
        )
    )
    {
        ASSERT_FAIL("Failed to allocate a buffer!");
        tmpBuffers.pop_back();
        return {};
    }

    // Return the allocation result
    ASSERT(tmpBuffer.getBytes());
    ASSERT(tmpBuffer.getSize() > 0);
    ASSERT(tmpBuffer.getVkBuffer());

    return StagingBuffer {
        tmpBuffer.getBytes(),
        tmpBuffer.getVkBuffer(),
        tmpBuffer.getSize()
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the pre-frame transfer task by recording it into a command buffer for the current
// ringbuffer index and submitting to the GPU. Returns true if the task successfully executed.
//
// Notes:
//  (1) This should only be called ONCE per frame.
//  (2) Clears the pre-frame transfer task upon submitting to the GPU.
//  (3) If there are no transfers to execute then this call is a no-op.
//------------------------------------------------------------------------------------------------------------------------------------------
bool TransferMgr::executePreFrameTransferTask() noexcept {
    // Preconditions: must be valid and device must be valid
    ASSERT(mbIsValid);
    ASSERT(mpDevice && mpDevice->getVkDevice());

    // If there is no work to be done then just bail
    if (mPreFrameTransferTask.isEmpty())
        return true;

    // Otherwise record the transfer task to the command buffer for this ringbuffer index
    RingbufferSlot& ringbufferSlot = getCurrentRingbufferSlot();
    CmdBuffer& cmdBuffer = ringbufferSlot.cmdBuffer;
    CmdBufferRecorder cmdRecorder(mpDevice->getVkFuncs());

    if (!cmdRecorder.beginPrimaryCmdBuffer(cmdBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT))
        return false;

    mPreFrameTransferTask.submitToCmdBuffer(cmdBuffer);

    if (!cmdRecorder.endCmdBuffer())
        return false;

    // Finally begin execution of the command buffer against the device
    return mpDevice->submitCmdBuffer(cmdBuffer, {}, nullptr, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleans up resources in the transfer manager associated with the given ringbuffer index.
// These resources include temporary staging buffers allocated for the purposes of doing transfers.
// This function should be called when the specified ringbuffer index is acquired or when the device
// has gone completely idle and no pending GPU operations are in flight.
//------------------------------------------------------------------------------------------------------------------------------------------
void TransferMgr::doCleanupForRingbufferIndex(const uint8_t ringbufferIndex) noexcept {
    ASSERT(mbIsValid);
    ASSERT(ringbufferIndex < Defines::RINGBUFFER_SIZE);

    RingbufferSlot& ringbufferSlot = mRingbufferSlots[ringbufferIndex];
    ringbufferSlot.tmpStagingBuffers.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current ringbuffer slot
//------------------------------------------------------------------------------------------------------------------------------------------
TransferMgr::RingbufferSlot& TransferMgr::getCurrentRingbufferSlot() noexcept {
    ASSERT(mbIsValid);
    const uint8_t ringbufferIdx = mpRingbufferMgr->getBufferIndex();
    return mRingbufferSlots[ringbufferIdx];
}

END_NAMESPACE(vgl)
