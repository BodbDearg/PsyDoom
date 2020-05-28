#include "LIBCOMB.h"

#include "PcPsx/Network.h"
#include "PcPsx/ProgArgs.h"
#include "PsxVm/PsxVm.h"

void LIBCOMB_AddCOMB() noexcept {
    // FIXME: IMPLEMENT ME FULLY
    if (ProgArgs::gbIsNetServer) {
        Network::initForServer();
    }
    else if (ProgArgs::gbIsNetClient) {
        Network::initForClient();
    }
}

void LIBCOMB_DelCOMB() noexcept {
    // FIXME: IMPLEMENT ME FULLY
}

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

    // Cancel the current read (if any)
    if ((cmd == 2) && (subcmd == 3))
        // FIXME: need to implement this if a network operation is happening
        return 0;

    // Set RTS (Ready To Send) flag to either '1' or '0' using the param, and return CTS (Clear to Send) flag as '1' or '0'.
    // For PsyDoom we don't use any of these flags, so no logic here.
    if ((cmd == 3) && (subcmd == 0))
        // FIXME: change answer depending on network status (for now saying we are always clear to send)
        return 1;

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
