#include "LIBETC.h"

#include "LIBAPI.h"
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
void LIBETC_v_wait(const int32_t targetVCount, [[maybe_unused]] const uint16_t timeout) noexcept {
    int32_t vcount = (int32_t) lw(0x80075CCC);          // Load from: gLIBETC_Vcount (80075CCC)

    while (vcount < targetVCount) {
        #if PC_PSX_DOOM_MODS
            emulate_frame();
        #endif

        vcount = (int32_t) lw(0x80075CCC);      // Load from: gLIBETC_Vcount (80075CCC)
    }
}

void _thunk_LIBETC_v_wait() noexcept {
    LIBETC_v_wait((int32_t) a0, (uint16_t) a1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the pressed buttons for pad 1 and pad 2 (digital controllers).
// The buttons for pad 1 are returned in the lower 16-bits, the buttons for pad 2 are returned in the high 16-bits.
// The input parameter is not used, and should be set to '0'.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBETC_PadRead([[maybe_unused]] const uint32_t unusedControllerId) noexcept {
    PsxVm::updateInput();
    return PsxVm::getControllerButtonBits();
}
