#include "LIBETC.h"

#include "LIBAPI.h"
#include "LIBC2.h"
#include "PsxVm/PsxVm.h"

void LIBETC_ResetCallback() noexcept {
loc_8004A7AC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5B90);                               // Load from: gpLIBETC_INTR_interruptsListPtr (80075B90)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(v0 + 0xC);
    ptr_call(v0);
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_InterruptCallback() noexcept {
loc_8004A7DC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5B90);                               // Load from: gpLIBETC_INTR_interruptsListPtr (80075B90)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(v0 + 0x8);
    ptr_call(v0);
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_DMACallback() noexcept {
loc_8004A80C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5B90);                               // Load from: gpLIBETC_INTR_interruptsListPtr (80075B90)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(v0 + 0x4);
    ptr_call(v0);
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_VSyncCallbacks() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5B90);                               // Load from: gpLIBETC_INTR_interruptsListPtr (80075B90)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(v0 + 0x14);
    ptr_call(v0);
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_StopCallback() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5B90);                               // Load from: gpLIBETC_INTR_interruptsListPtr (80075B90)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(v0 + 0x10);
    ptr_call(v0);
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_CheckCallback() noexcept {
loc_8004A89C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x5B96;                                       // Result = 80075B96
    v0 = lhu(v0);                                       // Load from: 80075B96
    return;
}

void LIBETC_GetIntrMask() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5BFC);                               // Load from: 80075BFC
    v0 = lhu(v0);
    return;
}

void LIBETC_SetIntrMask() noexcept {
loc_8004A8C8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5BFC);                               // Load from: 80075BFC
    v0 = lhu(v1);
    sh(a0, v1);
    return;
}

void LIBETC_INTR_startIntr() noexcept {
loc_8004A8E4:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x5B94;                                       // Result = 80075B94
    sw(ra, sp + 0x14);
    v0 = lhu(s0);                                       // Load from: 80075B94
    if (v0 != 0) goto loc_8004A990;
    LIBAPI_EnterCriticalSection();
    a0 = s0;                                            // Result = 80075B94
    a1 = 0x19;                                          // Result = 00000019
    LIBETC_INTR_memclr();
    a0 = s0 + 0x34;                                     // Result = 80075BC8
    LIBC2_setjmp();
    if (v0 == 0) goto loc_8004A934;
    LIBETC_INTR_trapIntr();
loc_8004A934:
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x5B94;                                       // Result = 80075B94
    v0 = 1;                                             // Result = 00000001
    sh(v0, s0);                                         // Store to: 80075B94
    LIBETC_startIntrDMA();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5B90);                               // Load from: gpLIBETC_INTR_interruptsListPtr (80075B90)
    sw(v0, v1 + 0x14);
    LIBETC_startIntrVSync();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5B90);                               // Load from: gpLIBETC_INTR_interruptsListPtr (80075B90)
    sw(v0, a0 + 0x4);
    LIBAPI__96_remove();
    a0 = s0 + 0x34;                                     // Result = 80075BC8
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x2B8;                                        // Result = 800802B8
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5BCC);                                // Store to: 80075BCC
    LIBAPI_HookEntryInt();
    LIBAPI_ExitCriticalSection();
loc_8004A990:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_INTR_trapIntr() noexcept {
loc_8004A9A4:
    sp -= 0x28;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5B96;                                       // Result = 80075B96
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    sh(v0, a1);                                         // Store to: 80075B96
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5BC4);                               // Load from: 80075BC4
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5BF8);                               // Load from: 80075BF8
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5BFC);                               // Load from: 80075BFC
    v1 = lhu(v1);
    a0 = lhu(a0);
    v0 &= v1;
    s0 = a0 & v0;
    if (s0 == 0) goto loc_8004AAB4;
    s3 = 1;                                             // Result = 00000001
    s4 = a1 + 2;                                        // Result = 80075B98
loc_8004AA08:
    s2 = 0;                                             // Result = 00000000
    if (s0 == 0) goto loc_8004AA80;
    s1 = s4;                                            // Result = 80075B98
loc_8004AA14:
    v0 = (i32(s2) < 0xB);
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 1;
        if (bJump) goto loc_8004AA80;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = s3 << s2;
        if (bJump) goto loc_8004AA6C;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5BF8);                               // Load from: 80075BF8
    v0 = ~v0;
    sh(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5BF8);                               // Load from: 80075BF8
    v0 = lhu(v0);
    v0 = lw(s1);
    if (v0 == 0) goto loc_8004AA6C;
    v0 = lw(s1);
    ptr_call(v0);
loc_8004AA6C:
    s1 += 4;
    s0 >>= 1;
    v0 = s0 & 0xFFFF;
    s2++;
    if (v0 != 0) goto loc_8004AA14;
loc_8004AA80:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x5BC4;                                       // Result = 80075BC4
    v0 = lw(v0);                                        // Load from: 80075BC4
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5BF8);                               // Load from: 80075BF8
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5BFC);                               // Load from: 80075BFC
    v1 = lhu(v1);
    a0 = lhu(a0);
    v0 &= v1;
    s0 = a0 & v0;
    if (s0 != 0) goto loc_8004AA08;
loc_8004AAB4:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5BF8);                               // Load from: 80075BF8
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5BFC);                               // Load from: 80075BFC
    v0 = lhu(a1);
    v1 = lhu(a2);
    v0 &= v1;
    if (v0 == 0) goto loc_8004AB34;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5C00);                               // Load from: 80075C00
    v1 = v0 + 1;
    v0 = (i32(v0) < 0x801);
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5C00);                                // Store to: 80075C00
    if (v0 != 0) goto loc_8004AB3C;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x187C;                                       // Result = STR_intr_timeout_err[0] (8001187C)
    a1 = lhu(a1);
    a2 = lhu(a2);
    LIBC2_printf();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5BF8);                               // Load from: 80075BF8
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5C00);                                 // Store to: 80075C00
    sh(0, v0);
    goto loc_8004AB3C;
loc_8004AB34:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5C00);                                 // Store to: 80075C00
loc_8004AB3C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x5B96;                                       // Result = 80075B96
    sh(0, v0);                                          // Store to: 80075B96
    LIBAPI_ReturnFromException();
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void LIBETC_INTR_setIntr() noexcept {
loc_8004AB74:
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5B98;                                       // Result = 80075B98
    v0 = s0 << 2;
    a0 = v0 + a1;
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    s3 = lw(a0);
    v0 = s3;
    if (s2 == s3) goto loc_8004AC84;
    v0 = lhu(a1 - 0x4);                                 // Load from: 80075B94
    {
        const bool bJump = (v0 == 0);
        v0 = s3;
        if (bJump) goto loc_8004AC84;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5BFC);                               // Load from: 80075BFC
    v1 = lhu(v0);
    sh(0, v0);
    s1 = v1 & 0xFFFF;
    if (s2 == 0) goto loc_8004AC04;
    v0 = 1;                                             // Result = 00000001
    v0 = v0 << s0;
    sw(s2, a0);
    v1 = lw(a1 + 0x2C);                                 // Load from: 80075BC4
    s1 |= v0;
    v0 |= v1;
    sw(v0, a1 + 0x2C);                                  // Store to: 80075BC4
    v0 = 4;                                             // Result = 00000004
    goto loc_8004AC30;
loc_8004AC04:
    v0 = 1;                                             // Result = 00000001
    v0 = v0 << s0;
    v0 = ~v0;
    sw(0, a0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5BC4);                               // Load from: 80075BC4
    s1 &= v0;
    v0 &= v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5BC4);                                // Store to: 80075BC4
    v0 = 4;                                             // Result = 00000004
loc_8004AC30:
    {
        const bool bJump = (s0 != v0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_8004AC48;
    }
    a0 = 0;                                             // Result = 00000000
    a1 = (s2 < 1);
    LIBAPI_ChangeClearRCnt();
    v0 = 5;                                             // Result = 00000005
loc_8004AC48:
    {
        const bool bJump = (s0 != v0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_8004AC60;
    }
    a0 = 1;                                             // Result = 00000001
    a1 = (s2 < 1);
    LIBAPI_ChangeClearRCnt();
    v0 = 6;                                             // Result = 00000006
loc_8004AC60:
    a0 = 2;                                             // Result = 00000002
    if (s0 != v0) goto loc_8004AC70;
    a1 = (s2 < 1);
    LIBAPI_ChangeClearRCnt();
loc_8004AC70:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5BFC);                               // Load from: 80075BFC
    sh(s1, v0);
    v0 = s3;
loc_8004AC84:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void LIBETC_INTR_stopIntr() noexcept {
loc_8004ACA4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBAPI_EnterCriticalSection();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x5B94;                                       // Result = 80075B94
    a1 = 0x19;                                          // Result = 00000019
    LIBETC_INTR_memclr();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5BF8);                               // Load from: 80075BF8
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5BFC);                               // Load from: 80075BFC
    sh(0, v0);
    v0 = lhu(v0);
    sh(v0, v1);
    LIBETC_INTR_stopIntr_UNKNOWN_Helper1();
    LIBETC_INTR_stopIntr_UNKNOWN_Helper2();
    LIBAPI_ResetEntryInt();
    LIBAPI_ExitCriticalSection();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_INTR_memclr() noexcept {
loc_8004AD14:
    sp -= 8;
    v0 = a1 - 1;
    if (a1 == 0) goto loc_8004AD34;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8004AD24:
    sw(0, a0);
    v0--;
    a0 += 4;
    if (v0 != v1) goto loc_8004AD24;
loc_8004AD34:
    sp += 8;
    return;
}

void LIBETC_startIntrVSync() noexcept {
loc_8004B5F0:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x5C94;                                       // Result = 80075C94
    a0 = s0;                                            // Result = 80075C94
    sw(ra, sp + 0x14);
    a1 = 9;                                             // Result = 00000009
    LIBETC_INTR_VB_memclr();
    a0 = 3;                                             // Result = 00000003
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5CB8);                               // Load from: 80075CB8
    a1 = 0x80050000;                                    // Result = 80050000
    a1 -= 0x4964;                                       // Result = LIBETC_INTR_VB_trapIntrVSync (8004B69C)
    sw(0, v0);
    LIBETC_InterruptCallback();
    v0 = 0x80050000;                                    // Result = 80050000
    v0 -= 0x4800;                                       // Result = LIBETC_INTR_VB_setIntrVSync (8004B800)
    v1 = 1;                                             // Result = 00000001
    sh(v1, s0);                                         // Store to: 80075C94
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_INTR_stopIntr_UNKNOWN_Helper2() noexcept {
loc_8004B654:
    sp -= 0x18;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x5C94;                                       // Result = 80075C94
    sw(ra, sp + 0x10);
    sh(0, a0);                                          // Store to: 80075C94
    a1 = 9;                                             // Result = 00000009
    LIBETC_INTR_VB_memclr();
    a0 = 3;                                             // Result = 00000003
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5CB8);                               // Load from: 80075CB8
    a1 = 0;                                             // Result = 00000000
    sw(0, v0);
    LIBETC_InterruptCallback();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_INTR_VB_trapIntrVSync() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5CB8);                               // Load from: 80075CB8
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lw(v0);
    v0 >>= 24;
    s0 = v0 & 0x7F;
    if (s0 == 0) goto loc_8004B790;
    s3 = 1;                                             // Result = 00000001
    s4 = 0x80070000;                                    // Result = 80070000
    s4 += 0x5C98;                                       // Result = 80075C98
loc_8004B6E4:
    s2 = 0;                                             // Result = 00000000
    if (s0 == 0) goto loc_8004B76C;
    s1 = s4;                                            // Result = 80075C98
loc_8004B6F0:
    v0 = (i32(s2) < 7);
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 1;
        if (bJump) goto loc_8004B76C;
    }
    a1 = 0xFF0000;                                      // Result = 00FF0000
    if (v0 == 0) goto loc_8004B75C;
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    v0 = s2 + 0x18;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5CB8);                               // Load from: 80075CB8
    v0 = s3 << v0;
    v1 = lw(a0);
    v0 |= a1;
    v0 &= v1;
    sw(v0, a0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5CB8);                               // Load from: 80075CB8
    v0 = lw(v0);
    v0 = lw(s1);
    if (v0 == 0) goto loc_8004B75C;
    v0 = lw(s1);
    ptr_call(v0);
loc_8004B75C:
    s1 += 4;
    s0 >>= 1;
    s2++;
    if (s0 != 0) goto loc_8004B6F0;
loc_8004B76C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5CB8);                               // Load from: 80075CB8
    v0 = lw(v0);
    v0 >>= 24;
    s0 = v0 & 0x7F;
    if (s0 != 0) goto loc_8004B6E4;
loc_8004B790:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5CB8);                               // Load from: 80075CB8
    v0 = lw(a1);
    v1 = 0xFF000000;                                    // Result = FF000000
    v0 &= v1;
    v1 = 0x80000000;                                    // Result = 80000000
    if (v0 == v1) goto loc_8004B7C8;
    v0 = lw(a1);
    v0 &= 0x8000;
    if (v0 == 0) goto loc_8004B7DC;
loc_8004B7C8:
    a1 = lw(a1);
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1AB8;                                       // Result = STR_Sys_DMA_Bus_Err[0] (80011AB8)
    LIBC2_printf();
loc_8004B7DC:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void LIBETC_INTR_VB_setIntrVSync() noexcept {
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = a0;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x5C98;                                       // Result = 80075C98
    v0 = s2 << 2;
    sw(s1, sp + 0x14);
    s1 = v0 + v1;
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s0, sp + 0x10);
    s3 = lw(s1);
    s0 = a1;
    v0 = s3;
    if (s0 == s3) goto loc_8004B8E8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lhu(v0 + 0x5C94);                              // Load from: 80075C94
    {
        const bool bJump = (v0 == 0);
        v0 = s3;
        if (bJump) goto loc_8004B8E8;
    }
    a0 = 0;                                             // Result = 00000000
    LIBETC_SetIntrMask();
    a2 = v0;
    if (s0 == 0) goto loc_8004B8A0;
    v0 = 0xFF0000;                                      // Result = 00FF0000
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5CB8);                               // Load from: 80075CB8
    v0 |= 0xFFFF;                                       // Result = 00FFFFFF
    sw(s0, s1);
    a0 = lw(a1);
    v1 = s2 + 0x10;
    a0 &= v0;
    v0 = 1;                                             // Result = 00000001
    v0 = v0 << v1;
    v1 = 0x800000;                                      // Result = 00800000
    v0 |= v1;
    a0 |= v0;
    sw(a0, a1);
    goto loc_8004B8DC;
loc_8004B8A0:
    v0 = 0xFF0000;                                      // Result = 00FF0000
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5CB8);                               // Load from: 80075CB8
    v0 |= 0xFFFF;                                       // Result = 00FFFFFF
    sw(0, s1);
    v1 = lw(a1);
    a0 = s2 + 0x10;
    v1 &= v0;
    v0 = 0x800000;                                      // Result = 00800000
    v1 |= v0;
    v0 = 1;                                             // Result = 00000001
    v0 = v0 << a0;
    v0 = ~v0;
    v1 &= v0;
    sw(v1, a1);
loc_8004B8DC:
    a0 = a2;
    LIBETC_SetIntrMask();
    v0 = s3;
loc_8004B8E8:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void LIBETC_INTR_VB_memclr() noexcept {
loc_8004B908:
    sp -= 8;
    v0 = a1 - 1;
    if (a1 == 0) goto loc_8004B928;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8004B918:
    sw(0, a0);
    v0--;
    a0 += 4;
    if (v0 != v1) goto loc_8004B918;
loc_8004B928:
    sp += 8;
    return;
}

void LIBETC_startIntrDMA() noexcept {
loc_8004B934:
    sp -= 0x18;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5CC0);                               // Load from: 80075CC0
    v0 = 0x107;                                         // Result = 00000107
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    sw(v0, v1);
    a0 = 0;                                             // Result = 00000000
    LIBAPI_ChangeClearPAD();
    a0 = 3;                                             // Result = 00000003
    a1 = 0;                                             // Result = 00000000
    LIBAPI_ChangeClearRCnt();
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x5CC4;                                       // Result = 80075CC4
    a0 = s0;                                            // Result = 80075CC4
    a1 = 2;                                             // Result = 00000002
    LIBETC_INTR_DMA_memclr();
    a1 = 0x80050000;                                    // Result = 80050000
    a1 -= 0x461C;                                       // Result = LIBETC_INTR_DMA_trapIntrDMA (8004B9E4)
    a0 = 0;                                             // Result = 00000000
    LIBETC_InterruptCallback();
    v0 = 0x80050000;                                    // Result = 80050000
    v0 -= 0x45D0;                                       // Result = LIBETC_INTR_DMA_setIntrDMA (8004BA30)
    v1 = 1;                                             // Result = 00000001
    sh(v1, s0);                                         // Store to: 80075CC4
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_INTR_stopIntr_UNKNOWN_Helper1() noexcept {
loc_8004B9AC:
    sp -= 0x18;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x5CC4;                                       // Result = 80075CC4
    a1 = 2;                                             // Result = 00000002
    sw(ra, sp + 0x10);
    sh(0, a0);                                          // Store to: 80075CC4
    LIBETC_INTR_DMA_memclr();
    a0 = 0;                                             // Result = 00000000
    a1 = 0;                                             // Result = 00000000
    LIBETC_InterruptCallback();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_INTR_DMA_trapIntrDMA() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5CCC);                               // Load from: gLIBETC_Vcount (80075CCC)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5CC8);                               // Load from: 80075CC8
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5CCC);                                // Store to: gLIBETC_Vcount (80075CCC)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5CCC);                               // Load from: gLIBETC_Vcount (80075CCC)
    if (v1 == 0) goto loc_8004BA20;
    ptr_call(v1);
loc_8004BA20:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBETC_INTR_DMA_setIntrDMA() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lhu(v0 + 0x5CC4);                              // Load from: 80075CC4
    if (v0 == 0) goto loc_8004BA60;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5CC8);                               // Load from: 80075CC8
    if (a0 == v1) goto loc_8004BA60;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5CC8);                                // Store to: 80075CC8
loc_8004BA60:
    v0 = v1;
    return;
}

void LIBETC_INTR_DMA_memclr() noexcept {
loc_8004BA68:
    sp -= 8;
    v0 = a1 - 1;
    if (a1 == 0) goto loc_8004BA88;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8004BA78:
    sw(0, a0);
    v0--;
    a0 += 4;
    if (v0 != v1) goto loc_8004BA78;
loc_8004BA88:
    sp += 8;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for the next vblank or return the time elapsed in vblanks since program start or hblanks since last invocation.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBETC_VSync() noexcept {
// TODO: RUN NATIVELY
#if 1
    emu_call(0x8004BA94);
#else
    sp -= 0x20;
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);

    v0 = lw(0x80075D04);        // Load from: GPU_REG_GP1 (80075D04)
    v1 = lw(0x80075D08);        // Load from: TIMER_REG_ROOT_CNT_1 (80075D08)    
    s0 = lw(v0);
    v0 = lw(v1);

    v1 = lw(0x80075D0C);        // Load from: gLIBETC_Hcount (80075D0C)
    v0 -= v1;
    s1 = v0 & 0xFFFF;

    if (i32(a0) >= 0) {
        v0 = s1;

        if (a0 != 1) {
            v0 = lw(0x80075D10);        // Load from: gLIBETC_VSync_UNKNOWN_VAR_3 (80075D10)
            v0--;
            v0 += a0;
            a1 = 0;

            if (i32(a0) > 0) {
                a1 = a0 - 1;
            } else {
                a0 = v0;
                _thunk_LIBETC_v_wait();
                v0 = lw(0x80075D04);        // Load from: GPU_REG_GP1 (80075D04)
                s0 = lw(v0);
                a0 = lw(0x80075CCC);        // Load from: gLIBETC_Vcount (80075CCC)
                a1 = 1;
                a0++;
                _thunk_LIBETC_v_wait();
                v0 = 0x80000;
                v0 &= s0;

                if (v0 != 0) {
                    v1 = lw(0x80075D04);    // Load from: GPU_REG_GP1 (80075D04)
                    v0 = lw(v1);
                    v0 ^= s0;

                    if (i32(v0) >= 0) {
                        a0 = 0x80000000;

                        do {
                            v0 = lw(v1);
                            v0 ^= s0;
                            v0 &= a0;
                        } while (v0 == 0);
                    }
                }

                v0 = lw(0x80075CCC);        // Load from: gLIBETC_Vcount (80075CCC)
                v1 = lw(0x80075D08);        // Load from: TIMER_REG_ROOT_CNT_1 (80075D08)
                sw(v0, 0x80075D10);         // Store to: gLIBETC_VSync_UNKNOWN_VAR_3 (80075D10)
                v1 = lw(v1);
                v0 = s1;
                sw(v1, 0x80075D0C);         // Store to: gLIBETC_Hcount (80075D0C)
            }
        }
    } else {
        v0 = lw(0x80075CCC);        // Load from: gLIBETC_Vcount (80075CCC)
    }

    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
#endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Waits for the specified target vblank amount with a specified timeout.
// This is an internal PSYQ function and not documented.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBETC_v_wait(const int32_t targetVCount, const uint16_t timeout) noexcept {
    uint32_t timeoutLeft = (uint32_t) timeout << 15;
    int32_t vcount = (int32_t) lw(0x80075CCC);          // Load from: gLIBETC_Vcount (80075CCC)  

    #if PC_PSX_DOOM_MODS == 1
        emulate_frame();
    #endif

    while (vcount < targetVCount) {
        --timeoutLeft;

        if (timeoutLeft == 0xFFFFFFFF) {
            a0 = 0x80011AD4;                    // Result = STR_Sys_VSync_Timeout_Err[0] (80011AD4)
            LIBC2_puts();
            a0 = 0;
            LIBAPI_ChangeClearPAD();
            a0 = 3;
            a1 = 0;
            LIBAPI_ChangeClearRCnt();
            break;
        }

        #if PC_PSX_DOOM_MODS == 1
            emulate_frame();
        #endif

        vcount = (int32_t) lw(0x80075CCC);      // Load from: gLIBETC_Vcount (80075CCC)
    }
}

void _thunk_LIBETC_v_wait() noexcept {
    LIBETC_v_wait(a0, a1);
}

void LIBETC_SetVideoMode() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5DA4);                               // Load from: gLIBETC_videoMode (80075DA4)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5DA4);                                // Store to: gLIBETC_videoMode (80075DA4)
    return;
}

void LIBETC_GetVideoMode() noexcept {
loc_8004E918:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5DA4);                               // Load from: gLIBETC_videoMode (80075DA4)
    return;
}
