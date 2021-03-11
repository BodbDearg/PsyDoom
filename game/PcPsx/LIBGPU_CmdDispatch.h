#pragma once

#include "Macros.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Module that handles dispatching LIBGPU format drawing commands/primitives to the emulated PlayStation 1 GPU
//------------------------------------------------------------------------------------------------------------------------------------------

#include "PsyQ/LIBGPU.h"

BEGIN_NAMESPACE(LIBGPU_CmdDispatch)

// Helpers to set GPU state
void setGpuTexPageId(const uint16_t texPageId) noexcept;
void setGpuClutId(const uint16_t clutId) noexcept;
void setGpuTexWin(const uint32_t texWin) noexcept;

// Primitive submission
void submit(const DR_MODE& drawMode) noexcept;
void submit(const DR_TWIN& texWin) noexcept;
void submit(const SPRT& sprite) noexcept;
void submit(const SPRT_8& sprite8) noexcept;
void submit(const LINE_F2& line) noexcept;
void submit(const POLY_FT3& poly) noexcept;
void submit(const POLY_F4& poly) noexcept;
void submit(const POLY_FT4& poly) noexcept;
void submit(const FLOORROW_FT& row) noexcept;
void submit(const WALLCOL_FT& col) noexcept;

END_NAMESPACE(LIBGPU_CmdDispatch)
