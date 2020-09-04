//------------------------------------------------------------------------------------------------------------------------------------------
// WmdToJson:
//      A simple command line program that converts a Williams Module File (.WMD) into a human readable JSON file.
//      Useful for examining the contents of a .WMD file.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Module.h"
#include "ModuleFileUtils.h"

using namespace AudioTools;

int main(int argc, const char* const argv[]) noexcept {
    // Need 3 arguments exactly
    if (argc != 3) {
        std::printf("Usage: %s <INPUT_WMD_FILE_PATH> <OUTPUT_JSON_FILE_PATH>\n", argv[0]);
        return 1;
    }

    // Read the input .WMD
    const char* const wmdFileIn = argv[1];
    const char* const jsonFileOut = argv[2];

    Module module = {};
    std::string errorMsg;

    if (!ModuleFileUtils::readWmdFile(wmdFileIn, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return 1;
    }

    if (!ModuleFileUtils::writeJsonFile(jsonFileOut, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return 1;
    }

    return 0;
}
