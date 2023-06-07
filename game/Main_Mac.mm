#include "Doom/psx_main.h"
#include "PsyDoom/Launcher/Launcher.h"

#include <clocale>

int main(const int argc, const char* const* const argv) noexcept {
    // Interpret 'char*' strings as UTF-8 for 'fopen' etc.
    setlocale(LC_ALL, "en_US.UTF-8");

    // Run the game!
    @autoreleasepool {
        #if PSYDOOM_LAUNCHER
            return Launcher::launcherMain(argc, argv);
        #else
            return psx_main(argc, argv);
        #endif
    }
}
