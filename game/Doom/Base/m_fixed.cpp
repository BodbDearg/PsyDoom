#include "m_fixed.h"

#include "PsxVm/PsxVm.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Multiply two numbers in 16.16 fixed point format
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t FixedMul(const fixed_t a, const fixed_t b) noexcept {
    // Note: the real version of this relied on combining the seperate 32-bit 'hi' and 'lo' portions of the MIPS 'mulu' instruction result.
    // This version shortcuts that a bit and just uses 64-bit types provided by modern C++ rather than going through emulator instructions
    // to get a split hi/lo result. It's the exact same thing basically but this should hopefully be a little faster...
    const bool bNegativeResult = ((a ^ b) < 0);
    const uint64_t a_u64 = (a < 0) ? -a : a;
    const uint64_t b_u64 = (b < 0) ? -b : b;

    const uint64_t result_u64 = (a_u64 * b_u64) >> FRACBITS;
    const fixed_t result_abs = (fixed_t) result_u64;

    return (bNegativeResult) ? -result_abs : result_abs;
}

void _thunk_FixedMul() noexcept {
    v0 = FixedMul(a0, a1);
}

void FixedDiv() noexcept {
loc_8003F180:
    t0 = a0 ^ a1;
    v0 = add(zero, a0);
    if (i32(a0) > 0) goto loc_8003F190;
    v0 = sub(zero, a0);
loc_8003F190:
    v1 = add(zero, a1);
    if (i32(a1) > 0) goto loc_8003F19C;
    v1 = sub(zero, a1);
loc_8003F19C:
    a2 = 0x10000;                                       // Result = 00010000
    at = (v1 < v0);
    t2 = 0;                                             // Result = 00000000
    if (at == 0) goto loc_8003F1C0;
loc_8003F1AC:
    v1 <<= 1;
    a2 <<= 1;
    at = (v1 < v0);
    t2 = 0;
    if (at != 0) goto loc_8003F1AC;
loc_8003F1C0:
    at = (i32(v0) < i32(v1));
    if (at != 0) goto loc_8003F1D4;
    v0 = sub(v0, v1);
    t2 |= a2;
loc_8003F1D4:
    v0 <<= 1;
    a2 >>= 1;
    if (a2 == 0) goto loc_8003F1EC;
    if (v0 != 0) goto loc_8003F1C0;
loc_8003F1EC:
    v0 = add(zero, t2);                                 // Result = 00000000
    if (i32(t0) >= 0) goto loc_8003F1F8;
    v0 = sub(zero, t2);                                 // Result = 00000000
loc_8003F1F8:
    return;
}
