#include "Doom/psx_main.h"
#include "PsyDoom/Launcher/Launcher.h"

int main(const int argc, const char* const* const argv) noexcept {
    @autoreleasepool {
        #if PSYDOOM_LAUNCHER
            return Launcher::launcherMain(argc, argv);
        #else
            return psx_main(argc, argv);
        #endif
    }
}
