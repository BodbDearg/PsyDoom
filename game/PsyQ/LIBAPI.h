#pragma once

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Some timing related cause descriptors for 'OpenEvent' and 'SetRCnt'.
// There are lots more event causes in the real PsyQ SDK, I'm just showing all the timing ones for some context.
// Note: I only support using 'RCntCNT2' in this partial reimplementation of LIBAPI - attempts to use other ones will fail.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint32_t RCntCNT0  = 0xF2000000;   // Counter on every pixel displayed
static constexpr uint32_t RCntCNT1  = 0xF2000001;   // Increment counter on every horizontal line (horizontal sync)
static constexpr uint32_t RCntCNT2  = 0xF2000002;   // Increment counter on every 8 CPU clocks
static constexpr uint32_t RCntCNT3  = 0xF2000003;   // Fire interrupt on every vertical sync

// Event types and modes for 'OpenEvent' (respectively).
// I'm just supporting 1 type used by DOOM: the 'interrupt' event type & mode.
static constexpr int32_t EvSpINT    = 0x0002;
static constexpr int32_t EvMdINTR   = 0x1000;

// Root counter modes for 'SetRCnt': just supporting the 'interrupt' mode
static constexpr int32_t RCntMdINTR = 0x1000;

bool LIBAPI_CloseEvent(const int32_t event) noexcept;
void LIBAPI_EnterCriticalSection() noexcept;
int32_t LIBAPI_write(const int32_t fileDesc, const void* const pBuffer, const int32_t numBytes) noexcept;
bool LIBAPI_EnableEvent(const int32_t event) noexcept;
void LIBAPI_InitPAD() noexcept;
bool LIBAPI_SetRCnt(const int32_t cntType, const uint16_t target, const int32_t mode) noexcept;
bool LIBAPI_StartRCnt(const int32_t cntType) noexcept;
bool LIBAPI_DisableEvent(const int32_t event) noexcept;
void LIBAPI_StartPAD() noexcept;
void LIBAPI_ChangeClearPAD() noexcept;
int32_t LIBAPI_OpenEvent(const int32_t cause, const int32_t type, const int32_t mode, int32_t (* const pHandler)()) noexcept;
int32_t LIBAPI_read(const int32_t fileDesc, void* const pBuffer, const int32_t numBytes) noexcept;
bool LIBAPI_TestEvent(const uint32_t eventDescriptor) noexcept;
void LIBAPI_ExitCriticalSection() noexcept;
void LIBAPI_open() noexcept;
void LIBAPI_FlushCache() noexcept;
void LIBAPI_InitHeap() noexcept;
