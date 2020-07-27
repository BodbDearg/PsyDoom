//------------------------------------------------------------------------------------------------------------------------------------------
// LIBAPI stuff which is no longer used.
// Note: these are NOT the original functions, merely stubs and function signatures to remind of these previously used functions.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Old_LIBAPI.h"

#if !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Open a file/stream
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBAPI_open([[maybe_unused]] const char* const pPath, [[maybe_unused]] const uint32_t flags) noexcept {
    ASSERT_FAIL("PsyDoom: needs to be re-implemented!");
    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a given number of bytes to the specified stream/file
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBAPI_write([[maybe_unused]] const int32_t fileDesc, const void* const pBuffer, const int32_t numBytes) noexcept {
    ASSERT_FAIL("PsyDoom: needs to be re-implemented!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a given number of bytes from the specified stream/file
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBAPI_read([[maybe_unused]] const int32_t fileDesc, void* const pBuffer, const int32_t numBytes) noexcept {
    ASSERT_FAIL("PsyDoom: needs to be re-implemented!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check to see if a given event has fired or is in the 'set' state
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_TestEvent([[maybe_unused]] const uint32_t eventDescriptor) noexcept {
    ASSERT_FAIL("PsyDoom: needs to be re-implemented!");
    return true;
}

#endif  // #if !PSYDOOM_MODS
