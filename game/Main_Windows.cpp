#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Doom/psx_main.h"
#include "PcPsx/ModMgr.h"
#include "PcPsx/ProgArgs.h"
#include "PsxVm/PsxVm.h"

int main(const int argc, const char** const argv) {
    // Parse command line arguments
    ProgArgs::init(argc, argv);

    // Initialize the PSX VM using NTSC-U PSXDOOM.EXE and the CDROM .cue file.
    if (!PsxVm::init("PSXDOOM.EXE", "Doom.cue")) {
        return 1;
    }

    // Initialize the modding manager
    ModMgr::init();

    // Run the game! This is the actual PSXDOOM.EXE entrypoint...
    psx_main();

    // We're done: shutdown
    PsxVm::shutdown();
    ModMgr::shutdown();
    return 0;
}
