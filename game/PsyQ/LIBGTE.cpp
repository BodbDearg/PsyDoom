//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBGTE' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LIBGTE.h"

#include "LIBAPI.h"
#include "PsxVm/PsxVm.h"

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
    ctc2(makeU32(m.m[0][0], m.m[0][1]), 0);
    ctc2(makeU32(m.m[0][2], m.m[1][0]), 1);
    ctc2(makeU32(m.m[1][1], m.m[1][2]), 2);
    ctc2(makeU32(m.m[2][0], m.m[2][1]), 3);
    ctc2(m.m[2][2], 4);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the translation component of the given MATRIX to the GTE
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_SetTransMatrix(const MATRIX& m) noexcept {
    ctc2(m.t[0], 5);
    ctc2(m.t[1], 6);
    ctc2(m.t[2], 7);
}

void LIBGTE_SetGeomOffset() noexcept {
loc_800502EC:
    a0 <<= 16;
    a1 <<= 16;
    ctc2(a0, 24);
    ctc2(a1, 25);
    return;
}

void LIBGTE_SetGeomScreen() noexcept {
loc_80050304:
    ctc2(a0, 26);
    return;
}

void LIBGTE_InitGeom() noexcept {
loc_80050334:
    at = 0x80070000;                                    // Result = 80070000
    sw(ra, at + 0x6868);                                // Store to: gLIBGTE_InitGeom_miniStack[0] (80076868)
    LIBGTE__patch_gte();
    ra = 0x80070000;                                    // Result = 80070000
    ra = lw(ra + 0x6868);                               // Load from: gLIBGTE_InitGeom_miniStack[0] (80076868)
    v0 = mfc0(12);
    v1 = 0x40000000;                                    // Result = 40000000
    v0 |= v1;
    mtc0(v0, 12);
    t0 = 0x155;                                         // Result = 00000155
    ctc2(t0, 29);
    t0 = 0x100;                                         // Result = 00000100
    ctc2(t0, 30);
    t0 = 0x3E8;                                         // Result = 000003E8
    ctc2(t0, 26);
    t0 = -0x1062;                                       // Result = FFFFEF9E
    ctc2(t0, 27);
    t0 = 0x1400000;                                     // Result = 01400000
    ctc2(t0, 28);
    ctc2(zero, 24);
    ctc2(zero, 25);
    return;
}

void LIBGTE__patch_gte() noexcept {
loc_800503B4:
    at = 0x80070000;                                    // Result = 80070000
    sw(ra, at + 0x7E48);                                // Store to: gLIBGTE__patch_gte_miniStack[0] (80077E48)
    LIBAPI_EnterCriticalSection();
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x56;                                          // Result = 00000056
    ptr_call(t2);
    k0 = 0x80050000;                                    // Result = 80050000
    k1 = 0x80050000;                                    // Result = 80050000
    v0 = lw(v0 + 0x18);
    k0 += 0x41C;                                        // Result = LIBGTE__patch_gte + 68 (8005041C) (8005041C)
    k1 += 0x454;                                        // Result = LIBAPI_FlushCache (80050454)
loc_800503E4:
    v1 = lw(k0);
    k0 += 4;
    v0 += 4;
    sw(v1, v0 - 0x4);
    if (k0 != k1) goto loc_800503E4;
    LIBAPI_FlushCache();
    LIBAPI_ExitCriticalSection();
    ra = 0x80070000;                                    // Result = 80070000
    ra = lw(ra + 0x7E48);                               // Load from: gLIBGTE__patch_gte_miniStack[0] (80077E48)
    return;
    k0 = 0x100;                                         // Result = 00000100
    k0 = lw(k0 + 0x8);                                  // Load from: 00000108
    k0 = lw(k0);
    k0 = addi(k0, 0x8);
    sw(at, k0 + 0x4);
    sw(v0, k0 + 0x8);
    sw(v1, k0 + 0xC);
    sw(ra, k0 + 0x7C);
    v0 = mfc0(13);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Transform the given vector by the current GTE rotation matrix.
// GTE status flags are saved to the given output pointer.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGTE_RotTrans(const SVECTOR& vecIn, VECTOR& vecOut, int32_t& flagsOut) noexcept {
    mtc2(makeU32(vecIn.vx, vecIn.vy), 0);
    mtc2(vecIn.vz, 1);
    cop2(0x480012);
    vecOut.vx = mfc2(25);
    vecOut.vy = mfc2(26);
    vecOut.vz = mfc2(27);
    flagsOut = cfc2(31);
}
