#pragma once

#include "PsxVm/VmPtr.h"

extern const VmPtr<VmPtr<std::byte>>        gpGpuPrimsBeg;
extern const VmPtr<VmPtr<std::byte>>        gpGpuPrimsEnd;
extern const VmPtr<std::byte[0x10000]>      gGpuCmdsBuffer;
extern const VmPtr<std::byte>               gGpuCmdsBufferEnd;

void I_AddPrim(const void* const pPrim) noexcept;

// C++ overload for convenience!
template <class T>
inline void I_AddPrim(const T& prim) noexcept {
    I_AddPrim(&prim);
}
