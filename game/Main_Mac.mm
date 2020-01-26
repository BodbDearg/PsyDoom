#include "Doom/psx_main.h"
#include "Doom/psx_main.h"
#include "PcPsx/ModMgr.h"

#define PSX_VM_NO_REGISTER_MACROS 1
#include "PsxVm/PsxVm.h"

#include <Foundation/Foundation.h>

int main(const int argc, const char** const argv) noexcept {
    @autoreleasepool {
        // Set the working directory to the .app folder if not specified
        NSFileManager* fileMgr = [NSFileManager defaultManager];
        NSString* workingDir = [fileMgr currentDirectoryPath];
        
        if ([workingDir isEqualToString: @"/"]) {
            NSURL* appFolder = [[[NSBundle mainBundle] bundleURL] URLByDeletingLastPathComponent];
            [fileMgr changeCurrentDirectoryPath: appFolder.path];
        }
    
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
    }

    return 0;
}
