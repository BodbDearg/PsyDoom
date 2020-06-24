#pragma once

#include "Assert.h"
#include "Network.h"

#include <asio.hpp>
#include <chrono>
#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Class that maintains a circular buffer of network packets of a certain type and which can asynchronously read them.
// Useful for buffering network update packets and reading them in the background.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PacketT, int32_t BufferSize>
class NetPacketReader {
public:
    typedef asio::ip::tcp::socket SocketT;
    typedef std::chrono::system_clock::time_point TimePointT;
    
    NetPacketReader(SockeT& socket) noexcept 
        : mTcpSocket()
        , mBufBegIdx(0)
        , mBufEndIdx(0)
        , mBuffer{}
        , mReceiveTime{}
        , mNumPacketsPending(0)
        , mbError(false)
    {
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Did an error occur with any read operation?
    // If this flag is set then no packets from the stream should be used - everything should be treated as invalid.
    // In this situation the connection should be destroyed.
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline bool hasError() const noexcept {
        return mError;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // How many packets are currently requested?
    // This number only goes down once packets are actually consumed, or if a stream error occurs - in which case requests are aborted.
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline int32_t numPacketsRequested() const noexcept {
        if (mBufEndIdx >= mBufBegIdx) {
            return (mBufEndIdx - mBufBegIdx) + mNumPacketsPending;
        } else {
            return (BufferSize - mBufBegIdx) + 1 + mBufEndIdx + mNumPacketsPending;
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Tells if any packets are requested
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline bool arePacketsRequested() const noexcept {
        return (mBufEndIdx != mBufBegIdx);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Tells if a packet is ready to be read
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline bool hasPacketReady() const noexcept {
        return ((mBufEndIdx != mBufBegIdx) && (mReceiveTime[mBufBegIdx] != {}));
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // How many packets are ready to be consumed?
    // Note: this will always return '0' in the case of an error.
    //--------------------------------------------------------------------------------------------------------------------------------------
    int32_t numPacketsReady() const noexcept {
        int32_t numReady = 0;

        // A packet is ready if the receive timestamp is non defaulted
        for (int32_t i = mBufBegIdx; i != mBufEndIdx; i = (i + 1) % BufferSize) {
            if (mReceiveTime[i] != {}) {
                ++numReady;
            }
        }

        return numReady;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Asynchronously read a specified number of packets and return 'false' if that did not start happening.
    // Note that if the buffer is full then packet reading will pause until buffer slots are freed.
    //--------------------------------------------------------------------------------------------------------------------------------------
    bool asyncReadNumPackets(int32_t numPackets) noexcept {
        if ((numPackets <= 0) || mbError)
            return false;

        // Kick off the read if there is a free buffer slot, otherwise wait until later
        const int32_t curNumRequests = numPacketsRequested();

        if (curNumRequests < BufferSize) {
            beginPacketRead();
        } else {
            mNumPacketsPending++;
        }

        return (!mbError);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Asynchronously try to fill up the packet buffer with incoming packets and return 'false' if that did not start happening
    //--------------------------------------------------------------------------------------------------------------------------------------
    bool asyncFillPacketBuffer() noexcept {
        const int32_t curNumRequests = numPacketsRequested();
        return (curNumRequests < BufferSize) ? asyncReadNumPackets(BufferSize - curNumRequests) : true;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Pop a previously requested packet from the packet buffer; if no packets were previously requested, an error occurs.
    // Blocks until a packet is received or an error occurs or the request is canceled.
    // While waiting for a packet, platform updates such as the window and sound etc. are peformed to keep the app responsive.
    // A callback can be specified that is invoked periodically to do logic and if it returns 'false' then the request is cancelled.
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <class UpdateCancelCB>
    bool popRequestedPacket(PacketT& packet, TimePointT& receiveTime, const UpdateCancelCB& callback) noexcept {
        // If the stream is in error, no packets were requested or waiting for a packet fails then issue an error
        if (mbError || (numPacketsRequested() <= 0) || (!waitForPacketReady(callback))) {
            packet = {};
            receiveTime = {};
            return false;
        }

        // Success: return the requested packet and consume it
        packet = mBuffer[mBufBegIdx];
        receiveTime = mReceiveTime[mBufBegIdx];
        consumeCurrentPacket();
        return true;
    }

private:
    //--------------------------------------------------------------------------------------------------------------------------------------
    // Consume the current front of the packet buffer if the stream is not in error.
    // Tries to begin a packet read following that.
    //--------------------------------------------------------------------------------------------------------------------------------------
    void consumeCurrentPacket() noexcept {
        ASSERT(!mbError);
        mBufBegIdx = (mBufBegIdx + 1) % BufferSize;
        tryBeginPendingPacketReads();
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Begin a packet read if one is pending and decrement the pending count
    //--------------------------------------------------------------------------------------------------------------------------------------
    void tryBeginPendingPacketReads() noexcept {
        while ((!mbError) && (mNumPacketsPending > 0) && (numPacketsRequested() < BufferSize)) {
            mNumPacketsPending--;
            beginPacketRead();
        }
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Kicks off the read of one packet.
    // Assumes there is a free packet buffer slot for the packet to be read into and that no stream error has yet occurred.
    //--------------------------------------------------------------------------------------------------------------------------------------
    void beginPacketRead() noexcept {
        ASSERT(!mbError);
        ASSERT(numPacketsRequested() < BufferSize);

        try {
            // Default the receive time to mark the packet as not yet received
            int32_t dstBufIdx = mBufEndIdx;
            mReceiveTime[dstBufIdx] = {};

            // Kick off the async read
            asio::async_read(
                mTcpSocket,
                asio::buffer(&mBuffer[dstBufIdx], sizeof(PacketT)),
                [=](const asio::error_code error, const std::size_t bytesRead) noexcept {
                    if ((!error) && (bytesRead == sizeof(PacketT)) {
                        // Record the receive time for the packet to mark it as received and kick off any pending packet reads that we can now do.
                        // Only do this however if the stream has not since been flagged as having an error.
                        if (!mbError) {
                            mReceiveTime[dstBufIdx] = std::chrono::system_clock::now();
                            tryBeginPendingPacketReads();
                        }
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
    // Wait for a packet to become ready to read and return 'true' on success.
    // The given update/cancel callback can be invoked periodically to do logic and cancel the operation by returning 'false'.
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <class UpdateCancelCB>
    bool waitForPacketReady(const UpdateCancelCB& callback) noexcept {       
        // If there is a packet already ready then we are done
        if (hasPacketReady())
            return true;

        while (numPacketsRequested() > 0) {
            // Update networking and do platform updates periodically to keep the app responsive: this might cause the operation to complete.
            // Note that platform updates also include networking, but they are restricted to run only so often to keep CPU usage down.
            // Adding an explicit call here means we do updates on every single loop iteration for networking, while we are waiting.
            Network::doUpdates();
            Utils::doPlatformUpdates();

            // Did something go wrong?
            if (mbError)
                return false;
            
            // If there is a packet now ready then we are done
            if (hasPacketReady())
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

        return false;   // No more requested packets, wait fails...
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Handles a stream error.
    // Zeros all packets in the packet buffer and resets the buffer begin/end markers.
    //--------------------------------------------------------------------------------------------------------------------------------------
    void handleStreamError() noexcept {
        mbError = true;
        mBufBegIdx = 0;
        mBufEndIdx = 0;
        mNumPacketsPending = 0;

        for (int32_t i = 0; i < BufferSize; ++i) {
            mBuffer[i] = {};
            mReceiveTime[i] = {};
        }
    }

    SocketT&    mTcpSocket;                 // The socket to use: must be valid for the lifetime of this object
    int32_t     mBufBegIdx;                 // Which packet to read next from in the circular packet buffer
    int32_t     mBufEndIdx;                 // Which packet to write to next in the circular packet buffer
    PacketT     mBuffer[BufferSize];        // The packet buffer
    TimePointT  mReceiveTime[BufferSize];   // When each packet was received
    int32_t     mNumPacketsPending;         // The number of packets requested which aren't reading yet
    bool        mbError;                    // If true there was an error reading
};
