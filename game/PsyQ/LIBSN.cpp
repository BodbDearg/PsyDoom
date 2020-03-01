#include "LIBSN.h"

#include "PsxVm/PsxVm.h"

void LIBSN__main() noexcept {
loc_800507AC:
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x6878);                               // Load from: gbLIBSN__main_called (80076878)
    sp -= 0x10;
    sw(s0, sp + 0x4);
    sw(s1, sp + 0x8);
    sw(ra, sp + 0xC);
    {
        const bool bJump = (t0 != 0);
        t0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80050804;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(t0, at + 0x6878);                                // Store to: gbLIBSN__main_called (80076878)
    s0 = 0x80010000;                                    // Result = 80010000
    s1 = 0;                                             // Result = 00000000
    if (s1 == 0) goto loc_80050804;
loc_800507EC:
    t0 = lw(s0);
    s0 += 4;
    s1--;
    ptr_call(t0);
    if (s1 != 0) goto loc_800507EC;
loc_80050804:
    ra = lw(sp + 0xC);
    s1 = lw(sp + 0x8);
    s0 = lw(sp + 0x4);
    sp += 0x10;
    return;
}
