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

#include "PsyQ/LIBGPU.h"

inline void I_AddPrim(const DR_MODE& drawMode) noexcept {

}

inline void I_AddPrim(const DR_TWIN& texWin) noexcept {

}

inline void I_AddPrim(const SPRT& sprite) noexcept {

}

inline void I_AddPrim(const LINE_F2& line) noexcept {

}

inline void I_AddPrim(const POLY_FT3& poly) noexcept {

}

inline void I_AddPrim(const POLY_F4& poly) noexcept {

}

inline void I_AddPrim(const POLY_FT4& poly) noexcept {

}
