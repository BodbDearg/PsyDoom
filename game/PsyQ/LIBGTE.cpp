//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBGTE' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LIBGTE.h"

static int16_t gGteRotMatrix[3][3];     // Emulated Geometry Transform Engine (GTE): current rotation matrix
static int32_t gGteTransVec[3];         // Emulated Geometry Transform Engine (GTE): current translation vector

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the rotation component of the given MATRIX to the GTE
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetRotMatrix(const MATRIX& m) noexcept {
    gGteRotMatrix[0][0] = m.m[0][0];
    gGteRotMatrix[0][1] = m.m[0][1];
    gGteRotMatrix[0][2] = m.m[0][2];
    gGteRotMatrix[1][0] = m.m[1][0];
    gGteRotMatrix[1][1] = m.m[1][1];
    gGteRotMatrix[1][2] = m.m[1][2];
    gGteRotMatrix[2][0] = m.m[2][0];
    gGteRotMatrix[2][1] = m.m[2][1];
    gGteRotMatrix[2][2] = m.m[2][2];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the translation component of the given MATRIX to the GTE
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetTransMatrix(const MATRIX& m) noexcept {
    gGteTransVec[0] = m.t[0];
    gGteTransVec[1] = m.t[1];
    gGteTransVec[2] = m.t[2];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the 'offset' values applied to geometry for the GTE for perspective projections.
// This call doesn't affect DOOM in any way, so I'm not properly implementing...
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetGeomOffset([[maybe_unused]] const int32_t x, [[maybe_unused]] const int32_t y) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the perspective projection distance 'h' from the eye to the screen.
// This call doesn't affect DOOM in any way, so I'm not properly implementing...
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetGeomScreen([[maybe_unused]] const int32_t h) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the geometry transformation engine; must be called before using GTE functionality.
// This function doesn't need to do anything for PSX DOOM - hence unimplemented here...
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_InitGeom() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Transform the given vector by the current GTE rotation matrix and translation vector.
// GTE status flags are saved to the given output flags field.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_RotTrans(const SVECTOR& vecIn, VECTOR& vecOut, int32_t& flagsOut) noexcept {
    // Do the matrix multiplication: this results in a 32.12 format number.
    // Technically it won't be that here on PC since we're using 64-bit numbers but it all gets truncated at the end anyway, so no difference.
    int64_t result[3];
    result[0] = (int64_t) gGteRotMatrix[0][0] * vecIn.vx + (int64_t) gGteRotMatrix[0][1] * vecIn.vy + (int64_t) gGteRotMatrix[0][2] * vecIn.vz;
    result[1] = (int64_t) gGteRotMatrix[1][0] * vecIn.vx + (int64_t) gGteRotMatrix[1][1] * vecIn.vy + (int64_t) gGteRotMatrix[1][2] * vecIn.vz;
    result[2] = (int64_t) gGteRotMatrix[2][0] * vecIn.vx + (int64_t) gGteRotMatrix[2][1] * vecIn.vy + (int64_t) gGteRotMatrix[2][2] * vecIn.vz;
    result[0] += (int64_t) gGteTransVec[0] << 12;
    result[1] += (int64_t) gGteTransVec[1] << 12;
    result[2] += (int64_t) gGteTransVec[2] << 12;

    // Save the result output and convert from 32.12 format back to a simple 32-bit integer
    vecOut.vx = (int32_t)(result[0] >> 12);
    vecOut.vy = (int32_t)(result[1] >> 12);
    vecOut.vz = (int32_t)(result[2] >> 12);

    // Don't care about the value of this - it's never used anywhere...
    flagsOut = 0;
}
