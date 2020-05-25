#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(Network)

// FIXME: TEMP STUFF FOR TESTING
void initForServer() noexcept;
void initForClient() noexcept;
bool sendBytes(const void* const pBuffer, const int32_t numBytes) noexcept;
bool recvBytes(void* pBuffer, const int32_t numBytes) noexcept;

END_NAMESPACE(Network)
