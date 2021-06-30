#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Module dealing with low level drawing commands and submitting them to the GPU.
//
// In the original .EXE (and older verions of PsyDoom) this module used to hold drawing commands in a buffer and submitted these command
// streams to the GPU when required, as an array of 32-bit words.
//
// In the current version of PsyDoom however primitives are submitted directly to the GPU for performance and simplicity.
// Therefore there are no more command buffers and 'I_AddPrim' just acts as a simple wrapper rather than stuffing commands into a queue.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PsyDoom/LIBGPU_CmdDispatch.h"

inline void I_AddPrim(const DR_MODE& drawMode) noexcept {
    LIBGPU_CmdDispatch::submit(drawMode);
}

inline void I_AddPrim(const DR_TWIN& texWin) noexcept {
    LIBGPU_CmdDispatch::submit(texWin);
}

inline void I_AddPrim(const SPRT& sprite) noexcept {
    LIBGPU_CmdDispatch::submit(sprite);
}

inline void I_AddPrim(const LINE_F2& line) noexcept {
    LIBGPU_CmdDispatch::submit(line);
}

inline void I_AddPrim(const POLY_FT3& poly) noexcept {
    LIBGPU_CmdDispatch::submit(poly);
}

inline void I_AddPrim(const POLY_F4& poly) noexcept {
    LIBGPU_CmdDispatch::submit(poly);
}

inline void I_AddPrim(const POLY_FT4& poly) noexcept {
    LIBGPU_CmdDispatch::submit(poly);
}

inline void I_AddPrim(const FLOORROW_FT& row) noexcept {
    LIBGPU_CmdDispatch::submit(row);
}

inline void I_AddPrim(const WALLCOL_GT& col) noexcept {
    LIBGPU_CmdDispatch::submit(col);
}
