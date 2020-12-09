#include "Macros.h"

#include <cstdint>
#include <chrono>

struct NetPacket_Tick;

BEGIN_NAMESPACE(Network)

extern bool gbWasInitAborted;

bool initForServer() noexcept;
bool initForClient() noexcept;
void shutdown() noexcept;
bool isConnected() noexcept;
void doUpdates() noexcept;
bool sendBytes(const void* const pBuffer, const int32_t numBytes) noexcept;
bool recvBytes(void* pBuffer, const int32_t numBytes) noexcept;
bool sendTickPacket(const NetPacket_Tick& packet) noexcept;
bool requestTickPackets() noexcept;
bool recvTickPacket(NetPacket_Tick& packet, std::chrono::system_clock::time_point& receiveTime) noexcept;

END_NAMESPACE(Network)
