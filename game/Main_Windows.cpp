#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "PsxVm/PsxVm.h"
#include "Doom/psx_main.h"

int WINAPI wWinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPWSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow
) {
    // Initialize the PSX VM stuff and then launch the app!
    if (!PsxVm::init("SCPH1001.BIN", "PSXDOOM.EXE", "Doom.cue")) {
        return 1;
    }

    psx_main();
    PsxVm::shutdown();
    return 0;
}
