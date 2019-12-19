#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "PsxVm/PsxVm.h"
#include "Doom/psx_main.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    // Initialize the PSX VM using the NTSC-U BIOS, NTSC-U PSXDOOM.EXE and the CDROM .cue file
    if (!PsxVm::init("SCPH1001.BIN", "PSXDOOM.EXE", "Doom.cue")) {
        return 1;
    }

    // Run the game!
    // This is the actual PSXDOOM.EXE entrypoint.
    psx_main();

    // We're done: shutdown
    PsxVm::shutdown();
    return 0;
}
