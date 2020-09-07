//------------------------------------------------------------------------------------------------------------------------------------------
// JsonToWmd:
//      A simple command line program that converts a .json file to a binary Williams Module File (.WMD).
//      Useful for generating a new .WMD after editing it's data via .json.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Module.h"
#include "ModuleFileUtils.h"

using namespace AudioTools;

int main(int argc, const char* const argv[]) noexcept {
    // Need 3 arguments exactly
    if (argc != 3) {
        std::printf("Usage: %s <INPUT_JSON_FILE_PATH> <OUTPUT_WMD_FILE_PATH>\n", argv[0]);
        return 1;
    }
    
    const char* const jsonFileIn = argv[1];
    const char* const wmdFileOut = argv[2];
    
    // Read the input .json
    Module module = {};
    std::string errorMsg;

    if (!ModuleFileUtils::readJsonFile(jsonFileIn, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return 1;
    }

    // Write the output .WMD
    if (!ModuleFileUtils::writeWmdFile(wmdFileOut, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return 1;
    }

    return 0;
}
