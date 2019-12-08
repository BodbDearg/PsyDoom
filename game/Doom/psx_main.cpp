#include "psx_main.h"

#include "PsxVm/PsxVm.h"

void psx_main() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x7E30;                                       // Result = gPSXCD_cbsyncsave (80077E30)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x613C;                                       // Result = 800A9EC4
loc_80050724:
    sw(0, v0);
    v0 += 4;
    at = (v0 < v1);
    if (at != 0) goto loc_80050724;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BF8);                               // Load from: StackEndAddr (80077BF8)
    v0 = addi(v0, -0x8);
    t0 = 0x80000000;                                    // Result = 80000000
    sp = v0 | t0;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x613C;                                       // Result = 800A9EC4
    a0 <<= 3;                                           // Result = 0054F620
    a0 >>= 3;                                           // Result = 000A9EC4
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BFC);                               // Load from: StackSize (80077BFC)
    a1 = v0 - v1;
    a1 -= a0;
    a0 |= t0;                                           // Result = 800A9EC4
    at = 0x80070000;                                    // Result = 80070000
    sw(ra, at + 0x7E58);                                // Store to: gProgramReturnAddr (80077E58)
    gp = 0x80070000;                                    // Result = 80070000
    gp += 0x75E0;                                       // Result = GPU_REG_GP0 (800775E0)
    fp = sp;
    a0 = addi(a0, 0x4);                                 // Result = 800A9EC8
    LIBAPI_InitHeap();
    ra = 0x80070000;                                    // Result = 80070000
    ra = lw(ra + 0x7E58);                               // Load from: gProgramReturnAddr (80077E58)
    I_Main();
    _break(0x1);
}
