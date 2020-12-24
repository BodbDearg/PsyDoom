#pragma once

#if !PSYDOOM_MODS

#include <cstdint>

void LIBGPU_DrawOTag(const void* const pPrimList, std::byte* const pGpuCmdBuffer) noexcept;

#endif
