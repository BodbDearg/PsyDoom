#include "Network.h"

#include "Doom/Base/i_main.h"
#include "ProgArgs.h"
#include "PsxPadButtons.h"
#include "PsyQ/LIBETC.h"
#include "Utils.h"
#include "Video.h"

// This prevents warnings in ASIO about the Windows SDK target version not being specified
#if _WIN32
    #include <sdkddkver.h>
#endif

#include <asio.hpp>

BEGIN_NAMESPACE(Network)

static bool                                     gbIsSendingBytes;
static std::unique_ptr<asio::io_context>        gpIoContext;
static std::unique_ptr<asio::ip::tcp::socket>   gpSocket;

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks for user input to cancel an abortable network operation like establishing a connection
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isCancelNetworkConnectionRequested() noexcept {
    // This is the original behavior of PSX Doom: the 'select' button was used to cancel.
    // TODO: replace with control binding.
    if (LIBETC_PadRead(0) & PAD_SELECT) {
        return true;
    }

    // TODO: support some sort of generic menu 'back' button binding here, other than 'select'.
    // This should be bound to keys like 'ESC'.
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Blocks until an asynchronous network operation completes, or is aborted; returns 'true' if the operation completed fully (not aborted).
// Queries the given flag (passed by reference) to tell if the operation is completed.
// While the operation is being waited on, platform updates are performed. The operation can also be marked non abortable too.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool waitForAsyncNetworkOp(bool& bFinishedFlag, const bool bIsAbortable) noexcept {
    while (true) {
        // Update the platform, and handle network related events: this might cause the operation to complete
        Utils::doPlatformUpdates();

        if (bFinishedFlag)
            break;

        // Not yet complete: check for abortion requests from the user - if that is allowed.
        // If we find none then yield some CPU since we will be waiting a while longer.
        if (bIsAbortable && isCancelNetworkConnectionRequested())
            return false;
        
        Utils::threadYield();
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Setup options and preferences for the given socket
//------------------------------------------------------------------------------------------------------------------------------------------
static void setSocketOptions(asio::ip::tcp::socket& socket) noexcept {
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
            setSocketOptions(*gpSocket);
        }
    }
    catch (...) {
        // Connection attempt failed for some reason...
    }

    // If the connection attempt failed then cleanup before exiting
    if (!bWasSuccessful) {
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
                    setSocketOptions(*gpSocket);
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
    gbIsSendingBytes = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if there is a network connection established
//------------------------------------------------------------------------------------------------------------------------------------------
bool isConnected() noexcept {
    return (gpSocket && gpSocket->is_open());
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
// Send the specified number of bytes along the network connection but do not wait for completion, for performance reasons.
// Returns 'true' if an attempt (only) was made to send the bytes - no guarantee they will all arrive OK.
//
// Notes:
//  (1) This call does NOT block and wait until all the bytes are confirmed sent OK, however only one send is allowed to proceed at a time.
//      If there was a previous send active, then this call WILL wait until that finishes.
//      This allows the sender to keep re-using the same packet buffer over and over.
//  (2) The buffer MUST remain valid for the duration of the network call
//  (3) If the send attempt fails immediately then the connection is killed immediately
//------------------------------------------------------------------------------------------------------------------------------------------
bool sendBytes(const void* const pBuffer, const int32_t numBytes) noexcept {
    // If there is no valid socket or the byte count is bad then this fails immediately
    if (!isConnected())
        return false;

    if (numBytes < 0) {
        shutdown();
        return false;
    }

    // If there was a previous write then wait for that finish up, only one is allowed at a time!
    if (gbIsSendingBytes) {
        Utils::doPlatformUpdates();

        while (gbIsSendingBytes) {
            Utils::threadYield();
            Utils::doPlatformUpdates();
        }
    }

    // If we killed the connection in a lambda then stop now
    if (!isConnected())
        return false;
    
    // Send the bytes but don't wait for the operation to finish
    try {
        asio::async_write(
            *gpSocket,
            asio::buffer(pBuffer, (size_t) numBytes),
            [&]([[maybe_unused]] const asio::error_code& error, [[maybe_unused]] const std::size_t bytesWritten) noexcept {
                gbIsSendingBytes = false;
            }
        );

        gbIsSendingBytes = true;
    }
    catch (...) {
        // Send attempt failed for some reason - kill the connection...
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Receive the specified number of bytes from the network connection and wait for completion.
// Returns 'true' on success completion, 'false' in all other error cases.
//
// Notes:
//  (1) If the receive attempt fails then the connection is killed immediately
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

END_NAMESPACE(Network)
