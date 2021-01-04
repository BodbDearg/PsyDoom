#pragma once

#include "CmdBuffer.h"
#include "Defines.h"
#include "TransferTask.h"

#include <cstddef>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;
class RawBuffer;
class RingbufferMgr;

//------------------------------------------------------------------------------------------------------------------------------------------
// A manager that allows scheduling of data transfer from one buffer to another.
// Used to schedule transferring from an in-RAM staging buffer to on-device/GPU memory.
//------------------------------------------------------------------------------------------------------------------------------------------
class TransferMgr {
public:
    // Contains details for an allocated staging buffer that can be used in transfers
    struct StagingBuffer {
        std::byte*  pBytes;
        VkBuffer    vkBuffer;
        uint64_t    size;
    };

    TransferMgr() noexcept;
    ~TransferMgr() noexcept;

    bool init(LogicalDevice& device) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }

    // Gets the transfer task that is executed prior to rendering operations in the frame.
    // Typically this contains texture and vertex etc. buffer upload operations.
    inline TransferTask& getPreFrameTransferTask() noexcept { return mPreFrameTransferTask; }

    StagingBuffer allocTempStagingBuffer(const uint32_t numBytes) noexcept;
    bool executePreFrameTransferTask() noexcept;   
    void doCleanupForRingbufferIndex(const uint8_t ringbufferIndex) noexcept;

private:
    // Copy and move disallowed
    TransferMgr(const TransferMgr& other) = delete;
    TransferMgr(TransferMgr&& other) = delete;
    TransferMgr& operator = (const TransferMgr& other) = delete;
    TransferMgr& operator = (TransferMgr&& other) = delete;

    // Data structures for one of the ringbuffer slots
    struct RingbufferSlot {
        CmdBuffer               cmdBuffer;              // The command buffer which is submitted to the transfer queue to do transfer related commands
        std::vector<RawBuffer>  tmpStagingBuffers;      // Temporary raw staging buffers that have been allocated by the manager for the purposes of doing transfers
    };

    RingbufferSlot& getCurrentRingbufferSlot() noexcept;

    bool                mbIsValid;
    LogicalDevice*      mpDevice;
    RingbufferMgr*      mpRingbufferMgr;
    TransferTask        mPreFrameTransferTask;
    RingbufferSlot      mRingbufferSlots[Defines::RINGBUFFER_SIZE];
};

END_NAMESPACE(vgl)
