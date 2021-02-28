#include "LIBETC.h"

#include "PcPsx/Controls.h"

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
//
// PsyDoom: this call is now just a stub kept for historical reference.
// The function doesn't actually do anything anymore, and never returns a valid vertical/horizontal blank count.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBETC_VSync([[maybe_unused]] const int32_t mode) noexcept { return 0; }

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the pressed buttons for pad 1 and pad 2 (digital controllers).
// The buttons for pad 1 are returned in the lower 16-bits, the buttons for pad 2 are returned in the high 16-bits.
// The input parameter is not used, and should be set to '0'.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBETC_PadRead([[maybe_unused]] const uint32_t unusedControllerId) noexcept {
    // PsyDoom: original PSX button bitmasks are now just used for entering original cheat code sequences.
    // The controls module will handle mapping whatever inputs are bound to represent original PSX buttons for this purpose.
    return Controls::getPSXCheatButtonBits();
}
