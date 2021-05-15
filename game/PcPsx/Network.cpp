#include "Network.h"

#include "Doom/Base/i_main.h"
#include "Doom/Game/p_tick.h"
#include "Doom/UI/m_main.h"
#include "Input.h"
#include "NetPacketReader.h"
#include "NetPacketWriter.h"
#include "ProgArgs.h"
#include "PsxPadButtons.h"
#include "PsyQ/LIBETC.h"
#include "Utils.h"
#include "Video.h"

// This prevents warnings in ASIO about the Windows SDK target version not being specified
#if _WIN32
    #include <sdkddkver.h>
#endif

BEGIN_DISABLE_HEADER_WARNINGS
    #include <asio.hpp>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Network)

// A flag set to true if network init was aborted by the user
bool gbWasInitAborted = false;

// Maximum number of input/output tick packets that can be buffered
static constexpr int32_t MAX_TICK_PKTS = 2;

static std::unique_ptr<asio::io_context>                                    gpIoContext;
static std::unique_ptr<asio::ip::tcp::socket>                               gpSocket;
static std::unique_ptr<NetPacketReader<NetPacket_Tick, MAX_TICK_PKTS>>      gTickPacketReader;
static std::unique_ptr<NetPacketWriter<NetPacket_Tick, MAX_TICK_PKTS>>      gTickPacketWriter;
static bool                                                                 gbWasWaitForAsyncNetOpAborted;

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks for user input to cancel an abortable network operation like establishing a connection
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isCancelNetworkConnectionRequested() noexcept {
    // Cancel the network operation if the menu 'back' button is pressed
    TickInputs inputs;
    P_GatherTickInputs(inputs);
    return inputs.bMenuBack;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Blocks until an asynchronous network operation completes, or is aborted; returns 'true' if the operation completed fully (not aborted).
// Queries the given flag (passed by reference) to tell if the operation is completed.
// While the operation is being waited on, platform updates are performed. The operation can also be marked non abortable too.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool waitForAsyncNetworkOp(bool& bFinishedFlag, const bool bIsAbortable) noexcept {
    gbWasWaitForAsyncNetOpAborted = false;

    if (bFinishedFlag)
        return true;

    while (true) {
        // If app quit is requested then abort now with failure
        if (Input::isQuitRequested())
            return false;

        // Update the platform, and handle network related events: this might cause the operation to complete.
        doUpdates();
        Utils::doPlatformUpdates();

        if (bFinishedFlag)
            break;

        // Not yet complete: check for abortion requests from the user - if that is allowed.
        if (bIsAbortable && isCancelNetworkConnectionRequested()) {
            gbWasWaitForAsyncNetOpAborted = true;
            return false;
        }

        // While we are waiting update the display to help prevent stutter after long pauses (if we are waiting a long time).
        // See the 'Utils.cpp' file for more comments on this issue; we don't do this for the Vulkan backend also.
        if (Video::gBackendType != Video::BackendType::Vulkan) {
            Video::displayFramebuffer();
        } else {
            // Quick hack for the Vulkan renderer, in case the user resizes the screen while we are connecting - redraw everything...
            M_DrawNetworkConnectDisplay();
        }

        Utils::threadYield();
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Setup options and preferences for the given socket among other stuff
//------------------------------------------------------------------------------------------------------------------------------------------
static void postConnectSetup(asio::ip::tcp::socket& socket) noexcept {
    // Set all the socket options
    auto trySetOption = [&](auto option) noexcept {
        try {
            socket.set_option(option);
        }
        catch (...) {
            // Ignore if not supported on this platform...
        }
    };

    trySetOption(asio::ip::tcp::no_delay(true));                        // Disable Nagles algorithm
    trySetOption(asio::socket_base::send_buffer_size(1024 * 16));       // 16 KiB
    trySetOption(asio::socket_base::receive_buffer_size(1024 * 16));    // 16 KiB
    trySetOption(asio::socket_base::keep_alive(false));                 // Shouldn't be neccessary when client + server are talking constantly
    trySetOption(asio::socket_base::send_low_watermark(0));             // Don't wait to batch up sends/receives (Doom only sends a small amount of data)
    trySetOption(asio::socket_base::receive_low_watermark(0));          // Don't wait to batch up sends/receives (Doom only sends a small amount of data)

    // Setup the tick packet reader/writers
    gTickPacketReader.reset(new NetPacketReader<NetPacket_Tick, MAX_TICK_PKTS>(*gpSocket));
    gTickPacketWriter.reset(new NetPacketWriter<NetPacket_Tick, MAX_TICK_PKTS>(*gpSocket));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a connection for the server of a game (player 1).
// Waits until an incomming connection is received from the client (player 2).
// Updates the window etc. while all this is happening and allows the connection attempt to be aborted.
//------------------------------------------------------------------------------------------------------------------------------------------
bool initForServer() noexcept {
    // Clean up any previous connection first
    shutdown();
    bool bWasSuccessful = false;

    try {
        // Create the io context and socket
        gpIoContext.reset(new asio::io_context());
        gpSocket.reset(new asio::ip::tcp::socket(*gpIoContext));

        // Start asynchronously waiting to accept a connection
        bool bDoneAsyncOp = false;
        asio::ip::tcp::acceptor acceptor(*gpIoContext, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), ProgArgs::gServerPort));

        acceptor.async_accept(
            *gpSocket,
            [&](const asio::error_code& error) noexcept {
                bDoneAsyncOp = true;
                bWasSuccessful = (!error);
            }
        );

        waitForAsyncNetworkOp(bDoneAsyncOp, true);

        if (gpSocket && bWasSuccessful) {
            postConnectSetup(*gpSocket);
        }
    }
    catch (...) {
        // Connection attempt failed for some reason...
    }

    // If the connection attempt failed then cleanup before exiting
    if (!bWasSuccessful) {
        gbWasInitAborted = gbWasWaitForAsyncNetOpAborted;
        shutdown();
    }

    return bWasSuccessful;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a connection for a client of a game (player 2).
// Waits until a connection is made to the server (player 1).
// Updates the window etc. while all this is happening and allows the connection attempt to be aborted.
//------------------------------------------------------------------------------------------------------------------------------------------
bool initForClient() noexcept {
    // Clean up any previous connection first
    shutdown();
    bool bWasSuccessful = false;

    try {
        // Create the io context and socket
        gpIoContext.reset(new asio::io_context());
        gpSocket.reset(new asio::ip::tcp::socket(*gpIoContext));

        // Start asynchronously resolving the host address
        asio::ip::tcp::resolver tcpResolver(*gpIoContext);
        asio::ip::tcp::resolver::query tcpResolverQuery(ProgArgs::getServerHost(), std::to_string(ProgArgs::gServerPort));

        asio::ip::tcp::resolver::iterator resolverIter;
        bool bDoneAsyncOp = false;

        tcpResolver.async_resolve(
            tcpResolverQuery,
            [&](const asio::error_code& error, asio::ip::tcp::resolver::iterator iter) noexcept {
                bDoneAsyncOp = true;
                resolverIter = iter;
                bWasSuccessful = (!error);
            }
        );

        // If we finished that and were successful then start trying to connect to the server
        if (waitForAsyncNetworkOp(bDoneAsyncOp, true) && bWasSuccessful) {
            // Doing a new operation: so reset done/success flags
            bDoneAsyncOp = false;
            bWasSuccessful = false;

            while (resolverIter != asio::ip::tcp::resolver::iterator{}) {
                gpSocket->async_connect(
                    *resolverIter,
                    [&](const asio::error_code& error) noexcept {
                        bDoneAsyncOp = true;
                        bWasSuccessful = (!error);
                    }
                );

                waitForAsyncNetworkOp(bDoneAsyncOp, true);

                if (gpSocket && bWasSuccessful) {
                    postConnectSetup(*gpSocket);
                }

                break;
            }
        }
    }
    catch (...) {
        // Connection attempt failed for some reason...
    }

    // If the connection attempt failed then cleanup before exiting
    if (!bWasSuccessful) {
        gbWasInitAborted = gbWasWaitForAsyncNetOpAborted;
        shutdown();
    }

    return bWasSuccessful;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes up the current network connection (if any)
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gpSocket.reset();
    gpIoContext.reset();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if there is a network connection established
//------------------------------------------------------------------------------------------------------------------------------------------
bool isConnected() noexcept {
    return (gpSocket && gpSocket->is_open());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the scoket for the current network connection
//------------------------------------------------------------------------------------------------------------------------------------------
asio::ip::tcp::socket* getSocket() noexcept {
    return gpSocket.get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Process any network related events that need to be handled
//------------------------------------------------------------------------------------------------------------------------------------------
void doUpdates() noexcept {
    if (gpIoContext) {
        gpIoContext->restart();
        gpIoContext->poll();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Send the specified number of bytes along the network connection and wait for completion.
// Returns 'true' on success completion, 'false' in all other error cases.
//
// Notes:
//  (1) If the write attempt fails then the connection is killed immediately
//------------------------------------------------------------------------------------------------------------------------------------------
bool sendBytes(const void* const pBuffer, const int32_t numBytes) noexcept {
    // If there is no valid socket or the byte count is bad then this fails immediately
    if (!isConnected())
        return false;

    if (numBytes < 0) {
        shutdown();
        return false;
    }

    // Begin writing the bytes and wait until that is done
    bool bWasSuccessful = false;
    bool bDoneAsyncOp = false;

    try {
        asio::async_write(
            *gpSocket,
            asio::buffer(pBuffer, (size_t) numBytes),
            [&](const asio::error_code& error, const std::size_t bytesWritten) noexcept {
                bDoneAsyncOp = true;
                bWasSuccessful = ((!error) && (bytesWritten == numBytes));
            }
        );

        waitForAsyncNetworkOp(bDoneAsyncOp, false);
    }
    catch (...) {
        // Send attempt failed for some reason - kill the connection...
        shutdown();
        return false;
    }

    // If the send attempt failed for some reason then kill the connection
    if (!bWasSuccessful) {
        shutdown();
    }

    return bWasSuccessful;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the specified number of bytes from the network connection and wait for completion.
// Returns 'true' on success completion, 'false' in all other error cases.
//
// Notes:
//  (1) If the read attempt fails then the connection is killed immediately
//------------------------------------------------------------------------------------------------------------------------------------------
bool recvBytes(void* pBuffer, const int32_t numBytes) noexcept {
    // If there is no valid socket or the byte count is bad then this fails immediately
    if (!isConnected())
        return false;

    if (numBytes < 0) {
        shutdown();
        return false;
    }

    // Begin receiving the bytes and wait until that is done
    bool bWasSuccessful = false;
    bool bDoneAsyncOp = false;

    try {
        asio::async_read(
            *gpSocket,
            asio::buffer(pBuffer, (size_t) numBytes),
            [&](const asio::error_code error, const std::size_t bytesRead) noexcept {
                bDoneAsyncOp = true;
                bWasSuccessful = ((!error) && (bytesRead == numBytes));
            }
        );

        waitForAsyncNetworkOp(bDoneAsyncOp, false);
    }
    catch (...) {
        // Receive attempt failed for some reason - kill the connection...
        shutdown();
        return false;
    }

    // If the receive attempt failed for some reason then kill the connection
    if (!bWasSuccessful) {
        shutdown();
    }

    return bWasSuccessful;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Send a tick update packet: this call may or may not block, depending on whether the outgoing packet queue is full or not.
//------------------------------------------------------------------------------------------------------------------------------------------
bool sendTickPacket(const NetPacket_Tick& packet) noexcept {
    if (!isConnected())
        return false;

    if (!gTickPacketWriter->writePacket(packet, nullptr)) {
        shutdown();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Request that the tick packet buffer be filled as much as possible and kick off as many packet reads as we can.
//------------------------------------------------------------------------------------------------------------------------------------------
bool requestTickPackets() noexcept {
    if (!isConnected())
        return false;

    if (!gTickPacketReader->asyncFillPacketBuffer()) {
        shutdown();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read one tick packet and block until it is available or an error occurs, in which case 'false' is returned.
// Returns the time that the packet was received at also.
//------------------------------------------------------------------------------------------------------------------------------------------
bool recvTickPacket(NetPacket_Tick& packet, std::chrono::system_clock::time_point& receiveTime) noexcept {
    if (!isConnected())
        return false;

    if (!gTickPacketReader->popRequestedPacket(packet, receiveTime, nullptr)) {
        shutdown();
        return false;
    }

    return true;
}

END_NAMESPACE(Network)
