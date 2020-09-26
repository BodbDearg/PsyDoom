//------------------------------------------------------------------------------------------------------------------------------------------
// WmdTool:
//      Utilities for converting .WMD files (Williams Module files) to JSON and visa versa.
//      Also utilities for importing and exporting sequences from and to MIDI.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Module.h"
#include "ModuleFileUtils.h"

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Help/usage printing
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* const HELP_STR =
R"(Usage: WmdTool <COMMAND-SWITCH> [COMMAND ARGS]

Command switches and arguments:

    -wmd-to-json <INPUT WMD FILE PATH> <OUTPUT JSON FILE PATH>
        Convert an input module file in binary .WMD format to JSON text format.
        In this output format the data of the module file is human readable and can be edited by standard JSON editing tools.
        You should use this conversion prior to editing settings for a module and then do the opposite conversion to save those edits.
        Example:
            WmdTool -wmd-to-json DOOMSND.WMD DOOMSND.json

    -json-to-wmd <INPUT JSON FILE PATH> <OUTPUT WMD FILE PATH>
        Convert a module file in text JSON format to the binary .WMD format used by PlayStation Doom.
        Example:
            WmdTool -json-to-wmd DOOMSND.json DOOMSND.WMD
)";

static void printHelp() noexcept {
    std::printf("%s\n", HELP_STR);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a module file in .WMD format to JSON format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool convertWmdToJson(const char* const wmdFileIn, const char* const jsonFileOut) noexcept {
    // Read the input .WMD
    Module module = {};
    std::string errorMsg;

    if (!ModuleFileUtils::readWmdFile(wmdFileIn, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    // Write the output JSON file
    if (!ModuleFileUtils::writeJsonFile(jsonFileOut, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a module file in JSON format to .WMD format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool convertJsonToWmd(const char* const jsonFileIn, const char* const wmdFileOut) noexcept {
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Program entrypoint
//------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, const char* const argv[]) noexcept {
    // Not enough arguments?
    if (argc < 2) {
        printHelp();
        return 1;
    }

    // See what command is being executed
    const char* const cmdSwitch = argv[1];

    if (std::strcmp(cmdSwitch, "-wmd-to-json") == 0) {
        if (argc == 4) {
            const char* const wmdFilePath = argv[2];
            const char* const jsonFilePath = argv[3];
            return (convertWmdToJson(wmdFilePath, jsonFilePath)) ? 0 : 1;
        }
    }
    else if (std::strcmp(cmdSwitch, "-json-to-wmd") == 0) {
        if (argc == 4) {
            const char* const jsonFilePath = argv[2];
            const char* const wmdFilePath = argv[3];
            return (convertJsonToWmd(jsonFilePath, wmdFilePath)) ? 0 : 1;
        }
    }

    printHelp();
    return 1;
}
