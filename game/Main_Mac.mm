#include "Doom/psx_main.h"
#include "Doom/psx_main.h"
#include "PcPsx/ModMgr.h"

#define PSX_VM_NO_REGISTER_MACROS 1
#include "PsxVm/PsxVm.h"

#include <Foundation/Foundation.h>
#include <cstdio>

int main(const int argc, const char** const argv) noexcept {
    @autoreleasepool {
        // Set the working directory to the .app folder if we can't find Doom.cue:
        NSFileManager* fileMgr = [NSFileManager defaultManager];
        NSString* workingDir = [fileMgr currentDirectoryPath];
        std::printf("PsyDoom current working directory: %s\n", workingDir.UTF8String);
        
        if (![fileMgr fileExistsAtPath: @"Doom.cue"]) {
            NSURL* bundleURL = [[NSBundle mainBundle] bundleURL];
            NSURL* appFolder;
            
            if ([bundleURL.lastPathComponent.pathExtension isEqualToString: @"app"]) {
                appFolder = [[[NSBundle mainBundle] bundleURL] URLByDeletingLastPathComponent];
            } else {
                appFolder = bundleURL;
            }
            
            std::printf("'Doom.cue' not found at current PsyDoom working directory! Changing PsyDoom working dir to: %s\n", appFolder.path.UTF8String);
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
