#include "Network.h"

#include "ProgArgs.h"

// This prevents warnings in ASIO about the Windows SDK target version not being specified
#if _WIN32
    #include <sdkddkver.h>
#endif

#include <asio.hpp>

static std::unique_ptr<asio::io_context>        gpIoContext;
static std::unique_ptr<asio::ip::tcp::socket>   gpSocket;

// FIXME: TEMP STUFF FOR TESTING
void Network::initForServer() noexcept {
    try {
        // Create the io context and socket
        gpIoContext.reset(new asio::io_context());
        gpSocket.reset(new asio::ip::tcp::socket(*gpIoContext));

        // Wait for a connection and accept
        asio::ip::tcp::acceptor acceptor(*gpIoContext, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), ProgArgs::gServerPort));
        acceptor.accept(*gpSocket);
    }
    catch (...) {
        std::printf("ERROR: Network::initForServer: error listening for an incoming connection!");
        gpSocket.reset();
        gpIoContext.reset();
    }
}

// FIXME: TEMP STUFF FOR TESTING
void Network::initForClient() noexcept {
    try {
        // Create the io context and socket
        gpIoContext.reset(new asio::io_context());
        gpSocket.reset(new asio::ip::tcp::socket(*gpIoContext));

        // Resolve the host addresss
        asio::ip::tcp::resolver tcpResolver(*gpIoContext);
        asio::ip::tcp::resolver::query tcpResolverQuery(ProgArgs::getServerHost(), std::to_string(ProgArgs::gServerPort));
        asio::ip::tcp::resolver::iterator tcpResolveIter = tcpResolver.resolve(tcpResolverQuery);
        
        // Connect to the resolved address
        while (tcpResolveIter != asio::ip::tcp::resolver::iterator{}) {
            gpSocket->connect(*tcpResolveIter);
            break;
        }
    }
    catch (...) {
        std::printf("ERROR: Network::initForClient: error connecting to the server!");
        gpSocket.reset();
        gpIoContext.reset();
    }
}

// FIXME: TEMP STUFF FOR TESTING
bool Network::sendBytes(const void* const pBuffer, const int32_t numBytes) noexcept {
    if (gpSocket && gpSocket->is_open()) {
        const std::byte* pCurBufferBytes = (const std::byte*) pBuffer;
        size_t bytesLeft = numBytes;

        while (bytesLeft > 0) {
            try {
                const size_t bytesWritten = asio::write(*gpSocket, asio::buffer(pCurBufferBytes, (size_t) bytesLeft));

                if (bytesWritten == 0)
                    return false;

                pCurBufferBytes += bytesWritten;
                bytesLeft -= bytesWritten;
            }
            catch (...) {
                std::printf("ERROR: Network::sendBytes: error sending %d bytes!", numBytes);
                return false;
            }
        }
    }

    return false;
}

// FIXME: TEMP STUFF FOR TESTING
bool Network::recvBytes(void* pBuffer, const int32_t numBytes) noexcept {
    if (gpSocket && gpSocket->is_open()) {
        std::byte* pCurBufferBytes = (std::byte*) pBuffer;
        size_t bytesLeft = numBytes;

        while (bytesLeft > 0) {
            try {
                const size_t bytesRead = asio::read(*gpSocket, asio::buffer(pCurBufferBytes, (size_t) bytesLeft));

                if (bytesRead == 0)
                    return false;

                pCurBufferBytes += bytesRead;
                bytesLeft -= bytesRead;
            }
            catch (...) {
                std::printf("ERROR: Network::recvBytes: error receiving %d bytes!", numBytes);
                return false;
            }
        }
    }

    return false;
}
