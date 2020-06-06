#pragma once

#include <cstddef>
#include <cstdint>

// 64 KiB of GPU primitives can be stored in the GPU command buffer at once
static constexpr uint32_t GPU_CMD_BUFFER_SIZE = 64 * 1024;

extern std::byte            gGpuCmdsBuffer[GPU_CMD_BUFFER_SIZE];
extern std::byte*           gpGpuPrimsBeg;
extern std::byte*           gpGpuPrimsEnd;
extern const std::byte*     gGpuCmdsBufferEnd;

void I_AddPrim(const void* const pPrim) noexcept;
