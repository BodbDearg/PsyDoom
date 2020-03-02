#include "LIBAPI.h"

#include "PsxVm/PsxVm.h"

void LIBAPI_CloseEvent() noexcept {
loc_80049C1C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 9;                                             // Result = 00000009
    emu_call(t2);
}

void LIBAPI_EnterCriticalSection() noexcept {
// TODO: RUN NATIVELY
#if 1
    emu_call(0x80049C2C);
#else
    a0 = 1;
    syscall(0x0);
#endif
}

void LIBAPI_write() noexcept {
loc_80049C3C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x35;                                          // Result = 00000035
    emu_call(t2);
}

void LIBAPI_EnableEvent() noexcept {
loc_80049C4C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0xC;                                           // Result = 0000000C
    emu_call(t2);
}

void LIBAPI_InitPAD() noexcept {
loc_80049C5C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x12;                                          // Result = 00000012
    emu_call(t2);
}

void LIBAPI_SetRCnt() noexcept {
loc_80049C6C:
    t0 = a0 & 0xFFFF;
    v0 = (i32(t0) < 3);
    a3 = 0x48;                                          // Result = 00000048
    if (v0 != 0) goto loc_80049C84;
    v0 = 0;                                             // Result = 00000000
    goto loc_80049D00;
loc_80049C84:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5B4C);                               // Load from: 80075B4C
    v1 = t0 << 4;
    v1 += v0;
    sh(0, v1 + 0x4);
    sh(a1, v1 + 0x8);
    v0 = (t0 < 2);
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x10;
        if (bJump) goto loc_80049CC4;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 1;
        if (bJump) goto loc_80049CB4;
    }
    a3 = 0x49;                                          // Result = 00000049
loc_80049CB4:
    {
        const bool bJump = (v0 != 0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_80049CE0;
    }
    a3 |= 0x100;
    goto loc_80049CE0;
loc_80049CC4:
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (t0 != v0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_80049CE0;
    }
    v0 = a2 & 1;
    {
        const bool bJump = (v0 != 0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_80049CE0;
    }
    a3 = 0x248;                                         // Result = 00000248
loc_80049CE0:
    v1 = t0 << 4;
    if (v0 == 0) goto loc_80049CEC;
    a3 |= 0x10;
loc_80049CEC:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5B4C);                               // Load from: 80075B4C
    v0 = 1;                                             // Result = 00000001
    v1 += a0;
    sh(a3, v1 + 0x4);
loc_80049D00:
    return;
}

void LIBAPI_GetRCnt() noexcept {
    v1 = a0 & 0xFFFF;
    v0 = (i32(v1) < 3);
    v1 <<= 4;
    if (v0 == 0) goto loc_80049D34;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5B4C);                               // Load from: 80075B4C
    v1 += v0;
    v0 = lhu(v1);
    goto loc_80049D38;
loc_80049D34:
    v0 = 0;                                             // Result = 00000000
loc_80049D38:
    return;
}

void LIBAPI_StartRCnt() noexcept {
loc_80049D40:
    v0 = a0 & 0xFFFF;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5B48);                               // Load from: 80075B48
    a0 = v0 << 2;
    at = 0x80070000;                                    // Result = 80070000
    at += a0;
    a0 = lw(at + 0x5B50);
    v1 = lw(a1 + 0x4);
    v0 = (i32(v0) < 3);
    v1 |= a0;
    sw(v1, a1 + 0x4);
    return;
}

void LIBAPI_StopRCnt() noexcept {
    a0 &= 0xFFFF;
    a0 <<= 2;
    a1 = 0x80070000;                                    // Result = 80070000
    v0 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5B48);                               // Load from: 80075B48
    v0 += a0;
    v0 = lw(v0 + 0x5B50);
    v1 = lw(a1 + 0x4);
    v0 = ~v0;
    v0 &= v1;
    sw(v0, a1 + 0x4);
    v0 = 1;                                             // Result = 00000001
    return;
}

void LIBAPI_ResetRCnt() noexcept {
    v1 = a0 & 0xFFFF;
    v0 = (i32(v1) < 3);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80049DCC;
    }
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5B4C);                               // Load from: 80075B4C
    v1 <<= 4;
    v1 += a0;
    sh(0, v1);
    goto loc_80049DD0;
loc_80049DCC:
    v0 = 0;                                             // Result = 00000000
loc_80049DD0:
    return;
}

void LIBAPI_DisableEvent() noexcept {
loc_80049DDC:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0xD;                                           // Result = 0000000D
    emu_call(t2);
}

void LIBAPI_StartPAD() noexcept {
loc_80049DEC:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x13;                                          // Result = 00000013
    emu_call(t2);
}

void LIBAPI_ChangeClearPAD() noexcept {
loc_80049DFC:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x5B;                                          // Result = 0000005B
    emu_call(t2);
}

void LIBAPI_OpenEvent() noexcept {
loc_80049E0C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 8;                                             // Result = 00000008
    emu_call(t2);
}

void LIBAPI_read() noexcept {
loc_80049E1C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x34;                                          // Result = 00000034
    emu_call(t2);
}

void LIBAPI_TestEvent() noexcept {
loc_80049E2C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0xB;                                           // Result = 0000000B
    emu_call(t2);
}

void LIBAPI_ExitCriticalSection() noexcept {
// TODO: RUN NATIVELY
#if 1
    emu_call(0x80049E3C);
#else
    a0 = 2;
    syscall(0x0);
#endif
}

void LIBAPI_open() noexcept {
loc_80049E4C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x32;                                          // Result = 00000032
    emu_call(t2);
}

void LIBAPI_ChangeClearRCnt() noexcept {
loc_8004AD60:
    t2 = 0xC0;                                          // Result = 000000C0
    t1 = 0xA;                                           // Result = 0000000A
    emu_call(t2);
}

void LIBAPI_ReturnFromException() noexcept {
loc_8004AD80:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x17;                                          // Result = 00000017
    emu_call(t2);
}

void LIBAPI_FlushCache() noexcept {
loc_80050454:
    t2 = 0xA0;                                          // Result = 000000A0
    t1 = 0x44;                                          // Result = 00000044
    emu_call(t2);
}

void LIBAPI_InitHeap() noexcept {
loc_80050884:
    t2 = 0xA0;                                          // Result = 000000A0
    t1 = 0x39;                                          // Result = 00000039
    emu_call(t2);
}

void LIBAPI_DeliverEvent() noexcept {
loc_80053D48:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 7;                                             // Result = 00000007
    emu_call(t2);
}

void LIBAPI_SysEnqIntRP() noexcept {
loc_80058A18:
    t2 = 0xC0;                                          // Result = 000000C0
    t1 = 2;                                             // Result = 00000002
    emu_call(t2);
}

void LIBAPI_AddDrv() noexcept {
loc_80058A28:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x47;                                          // Result = 00000047
    emu_call(t2);
}

void LIBAPI_DelDrv() noexcept {
loc_80058A38:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x48;                                          // Result = 00000048
    emu_call(t2);
}
