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

int32_t LIBCOMB__comb_control([[maybe_unused]] const int32_t cmd, [[maybe_unused]] const int32_t subcmd, [[maybe_unused]] const int32_t param) noexcept {
    // FIXME: IMPLEMENT ME FULLY

    // Set RTS (Ready To Send) flag to either '1' or '0' using the param, and return CTS (Clear to Send) flag as '1' or '0'
    if ((cmd == 3) && (subcmd == 0)) {
        // Say we are always clear to send.
        // TODO: change answer depending on network status?
        return 1;
    }

    return 0;
}
