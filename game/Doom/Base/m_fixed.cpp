#include "m_fixed.h"

#include "PsxVm/PsxVm.h"

void FixedMul() noexcept {
loc_8003F134:
    t0 = a0 ^ a1;
    v0 = add(zero, a0);
    if (i32(a0) > 0) goto loc_8003F144;
    v0 = sub(zero, a0);
loc_8003F144:
    v1 = add(zero, a1);
    if (i32(a1) > 0) goto loc_8003F150;
    v1 = sub(zero, a1);
loc_8003F150:
    multu(v0, v1);
    a0 = hi;
    a1 = lo;
    v0 = a0 << 16;
    v1 = a1 >> 16;
    a0 = v0 + v1;
    v0 = add(zero, a0);
    if (i32(t0) >= 0) goto loc_8003F178;
    v0 = sub(zero, a0);
loc_8003F178:
    return;
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
