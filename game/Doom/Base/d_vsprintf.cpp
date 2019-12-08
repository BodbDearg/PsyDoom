#include "d_vsprintf.h"

#include "PsxVm/PsxVm.h"

void D_mystrlen() noexcept {
    v1 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_800310BC;
    v0 = lbu(a0);
    a0++;
    if (v0 == 0) goto loc_800310C0;
loc_800310A0:
    v1++;
    v0 = lbu(a0);
    a0++;
    if (v0 != 0) goto loc_800310A0;
    goto loc_800310C0;
loc_800310BC:
    v1 = -1;                                            // Result = FFFFFFFF
loc_800310C0:
    v0 = v1;
    return;
}

void D_vsprintf() noexcept {
loc_800310C8:
    v0 = lbu(a1);
    t6 = a0;
    if (v0 == 0) goto loc_80031384;
    t5 = 0x75;                                          // Result = 00000075
loc_800310DC:
    v1 = lbu(a1);
    v0 = 0x25;                                          // Result = 00000025
    {
        const bool bJump = (v1 == v0)
        v0 = 0x30;                                      // Result = 00000030
        if (bJump) goto loc_800310F8;
    }
    v0 = lbu(a1);
    a1++;
    goto loc_80031180;
loc_800310F8:
    a1++;
    v1 = lbu(a1);
    t4 = 0x20;                                          // Result = 00000020
    if (v1 != v0) goto loc_80031114;
    t4 = 0x30;                                          // Result = 00000030
    a1++;
loc_80031114:
    v0 = lbu(a1);
    t2 = 0;                                             // Result = 00000000
    goto loc_8003113C;
loc_80031120:
    v0 = lbu(a1);
    a1++;
    v1 += t2;
    v1 <<= 1;
    v1 += v0;
    v0 = lbu(a1);
    t2 = v1 - 0x30;
loc_8003113C:
    v0 -= 0x30;
    v0 = (v0 < 0xA);
    v1 = t2 << 2;
    if (v0 != 0) goto loc_80031120;
    v1 = lbu(a1);
    v0 = 0x6C;                                          // Result = 0000006C
    a3 = 0;                                             // Result = 00000000
    if (v1 != v0) goto loc_80031168;
    a3 = 1;                                             // Result = 00000001
    a1++;
    v1 = lbu(a1);
loc_80031168:
    v0 = 0x63;                                          // Result = 00000063
    {
        const bool bJump = (v1 != v0)
        v0 = 0x73;                                      // Result = 00000073
        if (bJump) goto loc_8003118C;
    }
    v0 = lbu(a2);
    a2 += 4;
    a1++;
loc_80031180:
    sb(v0, a0);
    a0++;
    goto loc_80031374;
loc_8003118C:
    if (v1 != v0) goto loc_80031228;
    t1 = lw(a2);
    a2 += 4;
    a3 = 0;                                             // Result = 00000000
    if (t1 == 0) goto loc_800311D0;
    v0 = lbu(t1);
    v1 = t1 + 1;
    if (v0 == 0) goto loc_800311D4;
loc_800311B4:
    a3++;
    v0 = lbu(v1);
    v1++;
    if (v0 != 0) goto loc_800311B4;
    t0 = a3;
    goto loc_800311E4;
loc_800311D0:
    a3 = -1;                                            // Result = FFFFFFFF
loc_800311D4:
    t0 = a3;
    goto loc_800311E4;
loc_800311DC:
    sb(t4, a0);
    a0++;
loc_800311E4:
    v0 = t2;
    v0 = (i32(t0) < i32(v0));
    t2--;
    if (v0 != 0) goto loc_800311DC;
    v0 = lbu(t1);
    if (v0 == 0) goto loc_80031370;
loc_80031204:
    v0 = lbu(t1);
    t1++;
    sb(v0, a0);
    v0 = lbu(t1);
    a0++;
    if (v0 != 0) goto loc_80031204;
    a1++;
    goto loc_80031374;
loc_80031228:
    v0 = 0x6F;                                          // Result = 0000006F
    {
        const bool bJump = (v1 != v0)
        v0 = 0x78;                                      // Result = 00000078
        if (bJump) goto loc_8003123C;
    }
    t3 = 8;                                             // Result = 00000008
    goto loc_80031250;
loc_8003123C:
    t3 = 0x10;                                          // Result = 00000010
    if (v1 == v0) goto loc_80031250;
    v0 = 0x58;                                          // Result = 00000058
    {
        const bool bJump = (v1 != v0)
        v0 = 0x69;                                      // Result = 00000069
        if (bJump) goto loc_8003125C;
    }
loc_80031250:
    a3 = lw(a2);
    a2 += 4;
    goto loc_800312B4;
loc_8003125C:
    t3 = 0xA;                                           // Result = 0000000A
    if (v1 == v0) goto loc_80031278;
    v0 = 0x64;                                          // Result = 00000064
    if (v1 == v0) goto loc_80031278;
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 != t5) goto loc_8003138C;
loc_80031278:
    v1 = lw(a2);
    a2 += 4;
    if (i32(v1) >= 0) goto loc_800312B0;
    v0 = lbu(a1);
    {
        const bool bJump = (v0 == t5)
        v0 = 0x2D;                                      // Result = 0000002D
        if (bJump) goto loc_800312B0;
    }
    sb(v0, a0);
    a0++;
    a3 = -v1;
    if (t2 == 0) goto loc_800312B4;
    t2--;
    goto loc_800312B4;
loc_800312B0:
    a3 = v1;
loc_800312B4:
    t0 = 0;                                             // Result = 00000000
loc_800312B8:
    v1 = t0 + a0;
    if (t0 == 0) goto loc_800312FC;
    t1 = a0;
loc_800312C4:
    v0 = lbu(v1 - 0x1);
    sb(v0, v1);
    v1--;
    if (v1 != t1) goto loc_800312C4;
    if (t0 == 0) goto loc_800312FC;
    if (t2 == 0) goto loc_800312FC;
    if (a3 != 0) goto loc_800312FC;
    sb(t4, a0);
    goto loc_80031348;
loc_800312FC:
    divu(a3, t3);
    if (t3 != 0) goto loc_8003130C;
    _break(0x1C00);
loc_8003130C:
    v0 = hi;
    sb(v0, a0);
    v1 = lbu(a0);
    v0 = (v1 < 0xA);
    {
        const bool bJump = (v0 != 0)
        v0 = v1 + 0x30;
        if (bJump) goto loc_80031330;
    }
    v0 = v1 + 0x37;
loc_80031330:
    sb(v0, a0);
    divu(a3, t3);
    if (t3 != 0) goto loc_80031344;
    _break(0x1C00);
loc_80031344:
    a3 = lo;
loc_80031348:
    t0++;
    if (t2 == 0) goto loc_80031354;
    t2--;
loc_80031354:
    if (a3 != 0) goto loc_800312B8;
    if (t2 != 0) goto loc_800312B8;
    if (t0 == 0) goto loc_800312B8;
    a0 += t0;
loc_80031370:
    a1++;
loc_80031374:
    v0 = lbu(a1);
    if (v0 != 0) goto loc_800310DC;
loc_80031384:
    sb(0, a0);
    v0 = t6 - a0;
loc_8003138C:
    return;
}
