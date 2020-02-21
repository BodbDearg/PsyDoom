//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBGTE' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LIBGTE.h"
#include "LIBAPI.h"

#define PSX_VM_NO_REGISTER_MACROS 1
#include "PsxVm/PsxVm.h"

#include <cpu/cpu.h>
#include <cpu/gte/gte.h>

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: makes a 32-bit unsigned integer from the 2 given 16-bit unsigned integers.
// The 2nd unsigned integer is the most significant.
//------------------------------------------------------------------------------------------------------------------------------------------
static inline uint32_t makeU32(const uint16_t u16a, const uint16_t u16b) noexcept {
    return (uint32_t) u16a | ((uint32_t) u16b << 16);
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the rotation component of the given MATRIX to the GTE
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetRotMatrix(const MATRIX& m) noexcept {
    GTE& gte = PsxVm::gpCpu->gte;

    gte.rotation[0][0] = m.m[0][0];
    gte.rotation[0][1] = m.m[0][1];
    gte.rotation[0][2] = m.m[0][2];
    gte.rotation[1][0] = m.m[1][0];
    gte.rotation[1][1] = m.m[1][1];
    gte.rotation[1][2] = m.m[1][2];
    gte.rotation[2][0] = m.m[2][0];
    gte.rotation[2][1] = m.m[2][1];
    gte.rotation[2][2] = m.m[2][2];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the translation component of the given MATRIX to the GTE
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetTransMatrix(const MATRIX& m) noexcept {
    GTE& gte = PsxVm::gpCpu->gte;

    gte.translation.x = m.t[0];
    gte.translation.y = m.t[1];
    gte.translation.z = m.t[2];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the 'offset' values applied to geometry for the GTE for perspective projections.
// This call doesn't really matter for DOOM.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetGeomOffset(const int32_t x, const int32_t y) noexcept {
    GTE& gte = PsxVm::gpCpu->gte;

    gte.of[0] = x << 16;    // The original code did these shifts, not sure why...
    gte.of[1] = y << 16;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the perspective projection distance 'h' from the eye to the screen.
// This call doesn't really matter for DOOM.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetGeomScreen(const int32_t h) noexcept {
    GTE& gte = PsxVm::gpCpu->gte;
    gte.h = (uint16_t) h;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the geometry transformation engine.
// Must be called before using GTE functionality.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_InitGeom() noexcept {
    // This function doesn't need to do anything for PSX DOOM.
    // All DOOM uses the GTE for is rotations, and we set all the state we need for that with 'LIBGTE_SetRotMatrix' and 'LIBGTE_SetTransMatrix'.
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Transform the given vector by the current GTE rotation matrix and translation vector.
// GTE status flags are saved to the given output pointer.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_RotTrans(const SVECTOR& vecIn, VECTOR& vecOut, int32_t& flagsOut) noexcept {
    GTE& gte = PsxVm::gpCpu->gte;

    // Set GTE input vector
    gte.v[0].x = vecIn.vx;
    gte.v[0].y = vecIn.vy;
    gte.v[0].z = vecIn.vz;

    // This setup is required to store the result correctly
    gte.flag.reg = 0;
    gte.sf = true;
    gte.lm = false;

    // Do the rotation and save the result
    gte.multiplyMatrixByVector(gte.rotation, gte.v[0], gte.translation);

    vecOut.vx = gte.ir[1];
    vecOut.vy = gte.ir[2];
    vecOut.vz = gte.ir[3];
    flagsOut = 0;
}
