#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Doom/psx_main.h"
#include "PcPsx/ModMgr.h"
#include "PsxVm/PsxVm.h"

int main(const int argc, const char** const argv) {
    // Initialize the PSX VM using the NTSC-U BIOS, NTSC-U PSXDOOM.EXE and the CDROM .cue file.
    // Also initialize the modding manager:
    if (!PsxVm::init("SCPH1001.BIN", "PSXDOOM.EXE", "Doom.cue")) {
        return 1;
    }

    ModMgr::init(argc, argv);

    // Run the game! This is the actual PSXDOOM.EXE entrypoint...
    psx_main();

    // We're done: shutdown
    PsxVm::shutdown();
    ModMgr::shutdown();
    return 0;
}
