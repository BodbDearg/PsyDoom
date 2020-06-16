#pragma once

#include <cstdint>

// TODO: remove these eventually
static constexpr uint32_t HeapStartAddr = 0x800A9EC4;       // Where the heap starts in the PlayStation's address space (unwrapped)
static constexpr uint32_t StackEndAddr  = 0x200000;         // Where the stack ends (at 2 MiB) in the PlayStation's RAM (wrapped)
static constexpr uint32_t StackSize     = 0x8000;           // Size of the stack for PSX DOOM (32 KiB)

int32_t psx_main(const int argc, const char** const argv) noexcept;
