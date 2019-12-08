#include "p_firesky.h"

#include "PsxVm/PsxVm.h"

void P_UpdateFireSky() noexcept {
loc_80027CB0:
    t3 = a0;
    v0 = lh(t3 + 0x10);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7DC4);                               // Load from: gpLumpCache (8007823C)
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0);
    t1 = 0;                                             // Result = 00000000
    a1 = v0 + 0x48;
loc_80027CD4:
    t0 = 1;                                             // Result = 00000001
    t2 = t1 + 1;
    a2 = a1 + t1;
loc_80027CE0:
    a3 = lbu(a2);
    if (a3 != 0) goto loc_80027CF8;
    sb(0, a2 - 0x40);
    goto loc_80027D54;
loc_80027CF8:
    v0 = lbu(gp + 0x558);                               // Load from: gFireSkyRndIndex (80077B38)
    a0 = v0 + 1;
    v1 = v0 & 0xFF;
    v0 += 2;
    sb(a0, gp + 0x558);                                 // Store to: gFireSkyRndIndex (80077B38)
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x75A8;                                       // Result = RndTable[0] (80058A58)
    at += v1;
    v1 = lbu(at);
    a0 &= 0xFF;
    sb(v0, gp + 0x558);                                 // Store to: gFireSkyRndIndex (80077B38)
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x75A8;                                       // Result = RndTable[0] (80058A58)
    at += a0;
    v0 = lbu(at);
    v1 &= 3;
    v1 = t2 - v1;
    v1 &= 0x3F;
    v1 += a1;
    v0 &= 1;
    v0 = a3 - v0;
    sb(v0, v1 - 0x40);
loc_80027D54:
    a2 += 0x40;
    t0++;
    v0 = (i32(t0) < 0x80);
    a1 += 0x40;
    if (v0 != 0) goto loc_80027CE0;
    t1++;                                               // Result = 00000001
    v0 = (i32(t1) < 0x40);                              // Result = 00000001
    a1 -= 0x1FC0;
    if (v0 != 0) goto loc_80027CD4;
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, t3 + 0x1C);
    return;
}
