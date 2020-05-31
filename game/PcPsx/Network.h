#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(Network)

bool initForServer() noexcept;
bool initForClient() noexcept;
void shutdown() noexcept;
bool isConnected() noexcept;
void doUpdates() noexcept;
bool sendBytes(const void* const pBuffer, const int32_t numBytes) noexcept;
bool recvBytes(void* pBuffer, const int32_t numBytes) noexcept;

END_NAMESPACE(Network)
