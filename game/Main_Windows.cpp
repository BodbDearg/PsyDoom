#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "PsxVm/PsxVm.h"
#include "Doom/psx_main.h"

int main(int argc, char** argv) {
    // Initialize the PSX VM stuff and then launch the app!
    if (!PsxVm::init("SCPH1001.BIN", "PSXDOOM.EXE", "Doom.cue")) {
        return 1;
    }

    psx_main();
    PsxVm::shutdown();
    return 0;
}
