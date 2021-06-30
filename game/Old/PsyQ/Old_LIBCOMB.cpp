//------------------------------------------------------------------------------------------------------------------------------------------
// The entire LIBCOMB (Serial I/O) library is no longer needed anymore.
// Note: these are NOT the original functions, merely stubs and partial reimplementations that I previously used for a while.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Old_LIBCOMB.h"

#if !PSYDOOM_MODS

#include "PsyDoom/Network.h"
#include "PsyDoom/ProgArgs.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Install the serial link driver for the PlayStation: doesn't need to do anything in PsyDoom!
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBCOMB_AddCOMB() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Un-install the serial link driver for the PlayStation: doesn't need to do anything in PsyDoom!
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBCOMB_DelCOMB() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do various serial link commands and return the result.
// I've only implemented a small portion of them here for PsyDoom, just what is needed!
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t LIBCOMB__comb_control([[maybe_unused]] const int32_t cmd, [[maybe_unused]] const int32_t subcmd, [[maybe_unused]] const int32_t param) noexcept {
    // Set the communication bit rate: for PsyDoom we just ignore this
    if ((cmd == 1) && (subcmd == 3))
        return 0;

    // Clear serial connection error flags: for PsyDoom we don't need to do anything for this
    if ((cmd == 2) && (subcmd == 1))
        return 0;

    // Cancel the current read (if any) - in PsyDoom this will kill the whole connection if done
    if ((cmd == 2) && (subcmd == 3)) {
        Network::shutdown();
        return 0;
    }

    // Set RTS (Ready To Send) flag to either '1' or '0' using the param, and return CTS (Clear to Send) flag as '1' or '0'.
    // For PsyDoom we don't use any of these flags, so no logic here.
    if ((cmd == 3) && (subcmd == 0))
        return Network::isConnected() ? 1 : 0;

    // Unhandled command + subcommand combo: return '-1' to indicate an error
    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if this PlayStation is cleared/able to send data to the other PlayStation, returns 'true' if that is the case
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBCOMB_CombCTS() noexcept {
    return LIBCOMB__comb_control(3, 0, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reset the serial connection error flags and always return '0'
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBCOMB_CombResetError() noexcept {
    return LIBCOMB__comb_control(2, 1, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cancels the current read and always return '0'
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBCOMB_CombCancelRead() noexcept {
    return LIBCOMB__comb_control(2, 3, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the communication bit rate for the serial link cable and return '0'
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBCOMB_CombSetBPS(const int32_t commsBps) noexcept {
    return LIBCOMB__comb_control(1, 3, commsBps);
}

#endif
