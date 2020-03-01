#include "LIBETC.h"

#include "LIBAPI.h"
#include "LIBC2.h"
#include "PsxVm/PsxVm.h"

void LIBETC_ResetCallback() noexcept {
// TODO: RUN NATIVELY
#if 1
    emu_call(0x8004A7AC);
#else
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
#endif
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
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5B90);                               // Load from: gpLIBETC_INTR_interruptsListPtr (80075B90)
    v0 = lw(v0 + 0x4);
    ptr_call(v0);
}

void LIBETC_SetIntrMask() noexcept {
loc_8004A8C8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5BFC);                               // Load from: 80075BFC
    v0 = lhu(v1);
    sh(a0, v1);
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for the next vblank or return the time elapsed in vblanks or hblanks.
//
//  Mode:
//      0   Block until a vsync happens. Return the total number of horizontal blanking units elapsed (16-bit wrapping).
//      1   Return the number of horizontal blanking units elapsed (16-bit wrapping).
//    > 1   Wait for 'mode - 1' vblanks since the function was last invoked in this mode.
//          Return the number of horizontal blanking units elapsed (16-bit wrapping).
//    < 0   Return the total number of vertical blank units elapsed since program start.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBETC_VSync(const int32_t mode) noexcept {
    v0 = lw(0x80075D04);            // Load from: GPU_REG_GP1 (80075D04)
    v1 = lw(0x80075D08);            // Load from: TIMER_REG_ROOT_CNT_1 (80075D08)    
    uint32_t x0 = lw(v0);
    v0 = lw(v1);

    v1 = lw(0x80075D0C);            // Load from: gLIBETC_Hcount (80075D0C)
    v0 -= v1;
    uint32_t x1 = v0 & 0xFFFF;

    if (mode >= 0) {
        v0 = x1;

        if (mode != 1) {
            v0 = lw(0x80075D10);        // Load from: gLIBETC_VSync_UNKNOWN_VAR_3 (80075D10)
            v0--;
            v0 += mode;
            a1 = 0;

            if (mode > 0) {
                a1 = mode - 1;
            } else {
                a0 = v0;
                _thunk_LIBETC_v_wait();
                v0 = lw(0x80075D04);        // Load from: GPU_REG_GP1 (80075D04)
                x0 = lw(v0);
                a0 = lw(0x80075CCC);        // Load from: gLIBETC_Vcount (80075CCC)
                a1 = 1;
                a0++;
                _thunk_LIBETC_v_wait();
                v0 = 0x80000;
                v0 &= x0;

                if (v0 != 0) {
                    v1 = lw(0x80075D04);    // Load from: GPU_REG_GP1 (80075D04)
                    v0 = lw(v1);
                    v0 ^= x0;

                    if (i32(v0) >= 0) {
                        a0 = 0x80000000;

                        do {
                            v0 = lw(v1);
                            v0 ^= x0;
                            v0 &= a0;
                        } while (v0 == 0);
                    }
                }

                v0 = lw(0x80075CCC);        // Load from: gLIBETC_Vcount (80075CCC)                
                sw(v0, 0x80075D10);         // Store to: gLIBETC_VSync_UNKNOWN_VAR_3 (80075D10)

                v1 = lw(0x80075D08);        // Load from: TIMER_REG_ROOT_CNT_1 (80075D08)
                v1 = lw(v1);
                sw(v1, 0x80075D0C);         // Store to: gLIBETC_Hcount (80075D0C)
            }
        }

        return x1;
    }
    else {
        // PC-PSX: If you are polling vsync do a little emulation to pass the time.
        // Also advance the GPU emulation by a lot.
        #if PC_PSX_DOOM_MODS
            emulate_gpu(4096);
            emulate_sound_if_required();
        #endif

        return lw(0x80075CCC);      // Load from: gLIBETC_Vcount (80075CCC)
    }
}

void _thunk_LIBETC_VSync() noexcept {
    v0 = LIBETC_VSync(a0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Waits for the specified target vblank amount with a specified timeout.
// This is an internal PSYQ function and not documented.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBETC_v_wait(const int32_t targetVCount, const uint16_t timeout) noexcept {
    uint32_t timeoutLeft = (uint32_t) timeout << 15;
    int32_t vcount = (int32_t) lw(0x80075CCC);          // Load from: gLIBETC_Vcount (80075CCC)  

    while (vcount < targetVCount) {
        --timeoutLeft;

        if (timeoutLeft == 0xFFFFFFFF) {
            std::puts("VSync: timeout\n");

            a0 = 0;
            LIBAPI_ChangeClearPAD();
            
            a0 = 3;
            a1 = 0;
            LIBAPI_ChangeClearRCnt();
            break;
        }

        #if PC_PSX_DOOM_MODS
            emulate_frame();
        #endif

        vcount = (int32_t) lw(0x80075CCC);      // Load from: gLIBETC_Vcount (80075CCC)
    }
}

void _thunk_LIBETC_v_wait() noexcept {
    LIBETC_v_wait((int32_t) a0, (uint16_t) a1);
}
