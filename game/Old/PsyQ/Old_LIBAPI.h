#pragma once

#include <cstdint>

#if !PC_PSX_DOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Flags for 'LIBAPI_open' to specify how the stream behaves
//------------------------------------------------------------------------------------------------------------------------------------------

static constexpr uint32_t O_RDONLY  = 0x0001;                   // Open file in read only mode
static constexpr uint32_t O_WRONLY  = 0x0002;                   // Open file in write only mode
static constexpr uint32_t O_RDWR    = O_RDONLY | O_WRONLY;      // Open file in read/write mode
static constexpr uint32_t O_CREAT   = 0x0200;                   // Create the file, if it doesn't exist
static constexpr uint32_t O_NOWAIT  = 0x8000;                   // Do asynchronous (non blocking) I/O for the file

int32_t LIBAPI_open(const char* const pPath, const uint32_t flags) noexcept;
int32_t LIBAPI_read(const int32_t fileDesc, void* const pBuffer, const int32_t numBytes) noexcept;
int32_t LIBAPI_write(const int32_t fileDesc, const void* const pBuffer, const int32_t numBytes) noexcept;
bool LIBAPI_TestEvent(const uint32_t eventDescriptor) noexcept;

#endif  // #if !PC_PSX_DOOM_MODS
