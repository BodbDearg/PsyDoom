#include "Doom/psx_main.h"

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

        return psx_main(argc, argv);
    }
}
