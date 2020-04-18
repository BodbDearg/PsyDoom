//------------------------------------------------------------------------------------------------------------------------------------------
// Program args: parsing and storing command line arguments
//------------------------------------------------------------------------------------------------------------------------------------------
#include "ProgArgs.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <cstdio>
    #include <cstring>
END_THIRD_PARTY_INCLUDES

BEGIN_NAMESPACE(ProgArgs)

// Enable a temp hack to allow the engine to run beyond the original 30 FPS.
// TODO: Eventually support any framerate and interpolate.
bool gbUseHighFpsHack = false;

// The data directory to pull file overrides for the file modding mechanism, empty string when there is none.
// Any files placed in this directory matching original game file names will override the original game files.
// Note: stored as a C-String since it's gotten directly from program arguments, which will always be valid for the lifetime of the program.
const char* gDataDirPath = "";

// Format for a function that parses an argument.
// Takes in the current arguments list pointer and the number of arguments left, which is always expected to be at least '1'.
// Returns the number of arguments consumed.
typedef int (*ArgParser)(const int argc, const char** const argv);

static int parseArg_highFpsHack([[maybe_unused]] const int argc, const char** const argv) {
    if (std::strcmp(argv[0], "-highfps") == 0) {
        gbUseHighFpsHack = true;
        return 1;
    } 

    return 0;
}

static int parseArg_dataDir(const int argc, const char** const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-datadir") == 0)) {
        gDataDirPath = argv[1];
        return 2;
    } 

    return 0;
}

// A list of all the argument parsing functions
static constexpr ArgParser ARG_PARSERS[] = {
    parseArg_highFpsHack,
    parseArg_dataDir
};

void init(const int argc, const char** const argv) noexcept {
    // Consume all the arguments using parsers until there are none.
    // Note: first arg is the program name and that is always skipped.
    int argsLeft = argc - 1;
    const char** pCurArgv = argv + 1;

    while (argsLeft > 0) {
        // Run through all the parsers and handle the argument
        bool bHandledArg = false;

        for (ArgParser argParser : ARG_PARSERS) {
            const int argsConsumed = argParser(argsLeft, pCurArgv);

            if (argsConsumed > 0) {
                argsLeft -= argsConsumed;
                pCurArgv += argsConsumed;
                bHandledArg = true;
                break;
            }
        }

        // Unrecognized argument? If so warn about it and skip:
        if (!bHandledArg) {
            std::printf("Unrecognized command line argument '%s'! Arg will be ignored...", pCurArgv[0]);
            argsLeft--;
            pCurArgv++;
        }
    }
}

void shutdown() noexcept {
    // Reset everything back to its initial state and free any memory allocated (to help leak detection)
    gbUseHighFpsHack = false;
    gDataDirPath = "";
}

END_NAMESPACE(ProgArgs)
