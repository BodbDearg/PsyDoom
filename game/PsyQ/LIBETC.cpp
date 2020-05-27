#include "LIBETC.h"

#include "PsxVm/PsxVm.h"
#include <ctime>

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize system callbacks and interrupt handlers: doesn't need to do anything for PsyDoom!
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBETC_ResetCallback() noexcept {}

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
    // Wait for VBLANK not supported in PsyDoom! Just ignore the call.
    if (mode == 0)
        return 0;

    // PC-PSX: if this is being polled to pass time, ensure we are updating sound.
    // FIXME: update everything else also, window etc.
    #if PC_PSX_DOOM_MODS
        emulate_sound_if_required();
    #endif

    if (mode < 0) {
        // For the VBLANK count emulation use the time since the program started to get the count.
        const clock_t now = clock();
        double vblanks = ((double) now * 60.0) / (double) CLOCKS_PER_SEC;
        return (int32_t) vblanks;
    }
    else {
        // Horizontal blanking units not supported in PsyDoom, always return '0'.
        // Doom doesn't require this mode anyway, so it's OK:
        return 0;
    }
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
