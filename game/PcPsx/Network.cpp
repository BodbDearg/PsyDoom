#include "Network.h"

// This prevents warnings in ASIO about the Windows SDK target version not being specified
#if _WIN32
    #include <sdkddkver.h>
#endif

#include <asio.hpp>
#include <memory>

static std::unique_ptr<asio::io_context>        gpIoContext;
static std::unique_ptr<asio::ip::tcp::socket>   gpSocket;

// FIXME: TEMP STUFF FOR TESTING
void Network::initForServer() noexcept {
    gpIoContext.reset(new asio::io_context());
    gpSocket.reset(new asio::ip::tcp::socket(*gpIoContext));
    asio::ip::tcp::acceptor acceptor(*gpIoContext, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 1234));
    acceptor.accept(*gpSocket);
}

// FIXME: TEMP STUFF FOR TESTING
void Network::initForClient() noexcept {
    gpIoContext.reset(new asio::io_context());
    gpSocket.reset(new asio::ip::tcp::socket(*gpIoContext));
    gpSocket->connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 1234));
}

// FIXME: TEMP STUFF FOR TESTING
bool Network::sendBytes(const void* const pBuffer, const int32_t numBytes) noexcept {
    if (gpSocket && gpSocket->is_open()) {
        const std::byte* pCurBufferBytes = (const std::byte*) pBuffer;
        size_t bytesLeft = numBytes;

        while (bytesLeft > 0) {
            const size_t bytesWritten = asio::write(*gpSocket, asio::buffer(pCurBufferBytes, (size_t) bytesLeft));

            if (bytesWritten == 0)
                return false;

            pCurBufferBytes += bytesWritten;
            bytesLeft -= bytesWritten;
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
            const size_t bytesRead = asio::read(*gpSocket, asio::buffer(pCurBufferBytes, (size_t) bytesLeft));

            if (bytesRead == 0)
                return false;

            pCurBufferBytes += bytesRead;
            bytesLeft -= bytesRead;
        }
    }

    return false;
}
