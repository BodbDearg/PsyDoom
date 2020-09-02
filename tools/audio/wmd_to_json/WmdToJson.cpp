//------------------------------------------------------------------------------------------------------------------------------------------
// WmdToJson:
//      A simple command line program that converts a Williams Module File (.WMD) into a human readable JSON file.
//      Useful for examining the contents of a .WMD file.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Module.h"

#include <cstdio>

using namespace AudioTools;

int main(int argc, const char* const argv[]) noexcept {
    // Need 3 arguments exactly
    if (argc != 3) {
        std::printf("Usage: %s <INPUT_WMD_FILE_PATH> <OUTPUT_JSON_FILE_PATH>\n", argv[0]);
        return 1;
    }

    // Open the WMD file and setup a lambda to read it
    FILE* pWmdFile = std::fopen(argv[1], "rb");

    if (!pWmdFile) {
        std::printf("Could not open the .WMD file '%s'! Is the path valid?\n", argv[1]);
        return 1;
    }

    const StreamReadFunc wmdReader = [=](void* const pDst, const size_t size) noexcept(false) {
        // Zero sized operations always succeed
        if (size == 0)
            return;

        // Are we reading or seeking?
        if (pDst) {
            if (std::fread(pDst, size, 1, pWmdFile) != 1)
                throw std::exception("Error reading from file!");
        } else {
            if (std::fseek(pWmdFile, (long) size, SEEK_CUR) != 0)
                throw std::exception("Error seeking within file!");
        }
    };

    // Attempt to read the .WMD file into a new 'Module' data structure
    Module module = {};
    bool bReadModuleOk = false;

    try {
        module.readFromWmd(wmdReader);
        bReadModuleOk = true;
    } catch (std::exception e) {
        std::printf("An error occurred while reading the .WMD file '%s'! It may be corrupt. Error reason: %s\n", argv[1], e.what());
    } catch (...) {
        std::printf("An error occurred while reading the .WMD file '%s'! It may be corrupt.\n", argv[1]);
    }

    // Close up the input .WMD file and finish if we did not read the module OK
    std::fclose(pWmdFile);
    pWmdFile = nullptr;

    if (!bReadModuleOk)
        return 1;

    // TODO...
    return 0;
}
