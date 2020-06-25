#pragma once

#include "Assert.h"
#include "Network.h"

#include <asio.hpp>
#include <chrono>
#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Class that maintains a circular buffer of network packets of a certain type and which can asynchronously write them.
// Useful for buffering output network packets and writing them in the background.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PacketT, int32_t BufferSize>
class NetPacketWriter {
public:
    typedef asio::ip::tcp::socket SocketT;

    NetPacketWriter(SockeT& socket) noexcept 
        : mTcpSocket()
        , mBufBegIdx(0)
        , mBufEndIdx(0)
        , mBuffer{}
        , mbError(false)
    {
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Did an error occur with any write operation? If this flag is set then the socket and packet writer should NOT be used any further.
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline bool hasError() const noexcept {
        return mError;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // How many packets are currently outgoing or being written?
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline int32_t numOutgoingPackets() const noexcept {
        if (mBufEndIdx >= mBufBegIdx) {
            return (mBufEndIdx - mBufBegIdx);
        } else {
            return (BufferSize - mBufBegIdx) + mBufEndIdx;
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Tells if any packets are outgoing
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline bool arePacketsOutgoing() const noexcept {
        return (mBufEndIdx != mBufBegIdx);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Tells if there is a free outgoing packet slot; if this is the case a write operation will not block
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline bool hasFreeOutgoingPacketSlot() const noexcept {
        return (numOutgoingPackets() < BufferSize);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Start writing an outgoing packet.
    // If there are no free outgoing packet slots, then the call will block until one becomes free.
    // While waiting for a free packet slot, platform updates such as the window and sound etc. are peformed to keep the app responsive.
    // A callback can be specified that is invoked periodically to do logic and if it returns 'false' then the request is cancelled.
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <class UpdateCancelCB>
    bool writePacket(PacketT& packet, const UpdateCancelCB& callback) noexcept {
        // If the stream is in error then this fails immediately
        if (mbError)
            return false;

        // Must have a free outgoing packet slot before we begin the write
        if (!waitForFreePacketSlot(callback))
            return false;

        // Do the write
        asyncWritePacket(packet);
        return (!mbError);
    }

private:
    //--------------------------------------------------------------------------------------------------------------------------------------
    // Kicks off the write of specified packet.
    // Assumes there is a free slot to accomodate the packet until it is written and that there is no stream error.
    //--------------------------------------------------------------------------------------------------------------------------------------
    void asyncWritePacket(const PacketT& packet) noexcept {
        ASSERT(!mbError);
        ASSERT(hasFreeOutgoingPacketSlot());

        try {
            // Kick off the async write
            asio::async_write(
                mTcpSocket,
                asio::buffer(&mBuffer[mBufEndIdx], sizeof(PacketT)),
                [=](const asio::error_code error, const std::size_t bytesWritten) noexcept {
                    if ((!error) && (bytesWritten == sizeof(PacketT)) {
                        // Note: this logic assumes an in-order network protocol, which in this case is TCP.
                        // When the write is done simply move forward the beginning of the queue marker and wraparound.
                        mBufBegIdx = (mBufBegIdx + 1) % BufferSize;
                    } else {
                        // Something went wrong...
                        handleStreamError();
                    }
                }
            );

            // Consume the buffer slot and wraparound if required
            mBufEndIdx++;

            if (mBufEndIdx >= BufferSize) {
                mBufEndIdx = 0;
            }
        }
        catch (...) {
            handleStreamError();
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Wait for an outgoing packet slot to become ready and return 'true' for success when one is free.
    // The given update/cancel callback can be invoked periodically to do logic and cancel the operation by returning 'false'.
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <class UpdateCancelCB>
    bool waitForFreePacketSlot(const UpdateCancelCB& callback) noexcept {
        // If there is already a packet slot free then we are done
        if (hasFreeOutgoingPacketSlot())
            return true;

        while (true) {
            // Update networking and do platform updates periodically to keep the app responsive: this might cause a write operation to complete.
            // Note that platform updates also include networking, but they are restricted to run only so often to keep CPU usage down.
            // Adding an explicit call here means we do updates on every single loop iteration for networking, while we are waiting.
            Network::doUpdates();
            Utils::doPlatformUpdates();

            // Did something go wrong?
            if (mbError)
                return false;
            
            // If there is a free packet slot then we are done
            if (hasFreeOutgoingPacketSlot())
                return true;

            // Call the cancel/update callback (if given) and abort if it asks
            if (callback) {
                const bool bContinueWaiting = callback();

                if (!bContinueWaiting) {
                    handleStreamError();
                    return false;
                }
            }

            // Yield some CPU time before trying this all over again
            Utils::threadYield();
        }

        ASSERT_FAIL("Shouldn't ever reach here!");
        return false;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Handles a stream error
    //--------------------------------------------------------------------------------------------------------------------------------------
    void handleStreamError() noexcept {
        mbError = true;
        mBufBegIdx = 0;
        mBufEndIdx = 0;
    }

    SocketT&    mTcpSocket;             // The socket to use: must be valid for the lifetime of this object
    int32_t     mBufBegIdx;             // The first packet currently being written in the circular packet buffer
    int32_t     mBufEndIdx;             // The packet slot which will be used for the next write in the circular packet buffer
    PacketT     mBuffer[BufferSize];    // The output packet buffer
    bool        mbError;                // If true there was an error reading
};
