#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Doom/psx_main.h"
#include "PcPsx/Config.h"
#include "PcPsx/ModMgr.h"
#include "PcPsx/ProgArgs.h"
#include "PsxVm/PsxVm.h"

int main(const int argc, const char** const argv) {
    // Parse command line arguments and configuration
    ProgArgs::init(argc, argv);
    Config::init();

    // Initialize the PSX VM using the NTSC-U PSX Doom disc (supplied as a .cue file).
    // TODO: make this path configurable.
    if (!PsxVm::init("Doom.cue"))
        return 1;
    
    // Initialize the modding manager
    ModMgr::init();

    // Run the game! This is the actual PSXDOOM.EXE entrypoint...
    psx_main();

    // We're done: shutdown
    PsxVm::shutdown();
    ModMgr::shutdown();
    Config::shutdown();
    ProgArgs::shutdown();
    return 0;
}
