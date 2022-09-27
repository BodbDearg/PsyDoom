//------------------------------------------------------------------------------------------------------------------------------------------
// Program args: parsing and storing command line arguments
//------------------------------------------------------------------------------------------------------------------------------------------
#include "ProgArgs.h"

#include "Doom/doomdef.h"
#include "WadList.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

BEGIN_NAMESPACE(ProgArgs)

// The default client and server port.
// Note: on Unix type environments (except macOS) we typically can't bind ports 0..1024 as a non-root user so change the default there.
#if defined(WIN32) || defined(__APPLE__)
    static constexpr uint16_t DEFAULT_NET_PORT = 666;
#else
    static constexpr uint16_t DEFAULT_NET_PORT = 1666;
#endif

// Override path for the game's .cue file; this takes precedence over the setting in the game's config .ini files
const char* gCueFileOverride;

// If true then run the game without sound or graphics.
// Can only be used for single demo playback, the main game won't run in this mode;
bool gbHeadlessMode = false;

// The data directory to pull file overrides for the file modding mechanism, empty string when there is none.
// Any files placed in this directory matching original game file names will override the original game files.
const char* gDataDirPath = "";

const char* gPlayDemoFilePath = "";             // The demo file to play and exit
const char* gSaveDemoResultFilePath = "";       // Path to a json file to save the demo result to
const char* gCheckDemoResultFilePath = "";      // Path to a json file to read the demo result from and verify a match with
bool        gbRecordDemos;                      // True if the game should record demos for every map played

bool        gbIsNetServer   = false;                // True if this peer is a server in a networked game (player 1, waits for client connection)
bool        gbIsNetClient   = false;                // True if this peer is a client in a networked game (player 2, connects to waiting server)
uint16_t    gServerPort     = DEFAULT_NET_PORT;     // Port that the server listens on or that the client connects to

// Cheat: if true then do not spawn any monsters
bool gbNoMonsters = false;

// If true then start each level with just a pistol and 50 rounds, and full health.
// Basically the exact same player state as if the level had been restarted.
bool gbPistolStart = false;

// Cheat intended for speed running: player moves and fires 2x faster.
// Doors and platforms also move 2x faster.
bool gbTurboMode = false;

// The map number and skill to use if warping on startup straight to a map.
int32_t gWarpMap = 0;
skill_t gWarpSkill = sk_hard;

// Host that the client connects to: private so we don't expose std::string everywhere
static std::string gServerHost;

// A list of main IWAD files added by the user, in left to right order.
// These can be used to modify/override lumps in the existing PSXDOOM.WAD without entirely replacing it.
// Note that the '-datadir' option must be used to actually play new map files; map data in any main IWAD files added will be ignored.
static std::vector<std::string> gUserWadFiles;

// Format for a function that parses an argument.
// Takes in the current arguments list pointer and the number of arguments left, which is always expected to be at least '1'.
// Returns the number of arguments consumed.
typedef int (*ArgParser)(const int argc, const char* const* const argv);

static int parseArg_cue([[maybe_unused]] const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-cue") == 0)) {
        gCueFileOverride = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_headless([[maybe_unused]] const int argc, const char* const* const argv) {
    if (std::strcmp(argv[0], "-headless") == 0) {
        gbHeadlessMode = true;
        return 1;
    }

    return 0;
}

static int parseArg_datadir(const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-datadir") == 0)) {
        gDataDirPath = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_playdemo(const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-playdemo") == 0)) {
        gPlayDemoFilePath = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_saveresult(const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-saveresult") == 0)) {
        gSaveDemoResultFilePath = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_checkresult(const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-checkresult") == 0)) {
        gCheckDemoResultFilePath = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_record([[maybe_unused]] const int argc, const char* const* const argv) {
    if (std::strcmp(argv[0], "-record") == 0) {
        gbRecordDemos = true;
        return 1;
    }

    return 0;
}

static int parseArg_nomonsters(const int argc, const char* const* const argv) {
    if ((argc >= 1) && (std::strcmp(argv[0], "-nomonsters") == 0)) {
        gbNoMonsters = true;
        return 1;
    }

    return 0;
}

static int parseArg_pistolstart(const int argc, const char* const* const argv) {
    if ((argc >= 1) && (std::strcmp(argv[0], "-pistolstart") == 0)) {
        gbPistolStart = true;
        return 1;
    }

    return 0;
}

static int parseArg_turbo(const int argc, const char* const* const argv) {
    if ((argc >= 1) && (std::strcmp(argv[0], "-turbo") == 0)) {
        gbTurboMode = true;
        return 1;
    }

    return 0;
}

static int parseArg_server([[maybe_unused]] const int argc, const char* const* const argv) {
    if (std::strcmp(argv[0], "-server") == 0) {
        gbIsNetServer = true;

        // Is there a port number following?
        if ((argc >= 2) && (argv[1][0] != '-')) {
            bool bValidPort = false;

            try {
                const int port = std::stoi(argv[1]);

                if ((port >= 0) && (port <= UINT16_MAX)) {
                    gServerPort = (uint16_t) port;
                    bValidPort = true;
                }
            } catch (...) {
                // Ignore..
            }

            if (!bValidPort) {
                std::printf("Bad server port number '%s'! Arg will be ignored...\n", argv[1]);
            }

            return 2;
        }

        return 1;
    }

    return 0;
}

static int parseArg_client([[maybe_unused]] const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-client") == 0)) {
        gbIsNetClient = true;

        // Parse the host and possibly port (separated by ':'):
        const char* const pFirstColon = std::strchr(argv[1], ':');

        if (!pFirstColon) {
            gServerHost = argv[1];
        } else {
            gServerHost = std::string(argv[1], pFirstColon - argv[1]);
            bool bValidPort = false;

            try {
                const int port = std::stoi(pFirstColon + 1);

                // Note: the '0' wildcard port is not valid for a client: only valid for servers
                if ((port >= 1) && (port <= UINT16_MAX)) {
                    gServerPort = (uint16_t) port;
                    bValidPort = true;
                }
            } catch (...) {
                // Ignore..
            }

            if (!bValidPort) {
                std::printf("Bad server port number '%s'! Arg will be ignored...\n", pFirstColon + 1);
            }
        }

        return 2;
    }

    return 0;
}

static int parseArg_file(const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-file") == 0)) {
        gUserWadFiles.push_back(argv[1]);
        return 2;
    }

    return 0;
}

static int parseArg_nolauncher(const int argc, const char* const* const argv) {
    // Note: '-nolauncher' is a dummy/null argument that doesn't actually do anything.
    // It causes the launcher not to show simply because a command line argument has been specified.
    // Any other command line argument can also have the same effect.
    if ((argc >= 1) && (std::strcmp(argv[0], "-nolauncher") == 0))
        return 1;

    return 0;
}

static int parseArg_warp(const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-warp") == 0)) {
        gWarpMap = std::atoi(argv[1]);
        return 2;
    }

    return 0;
}

static int parseArg_skill(const int argc, const char* const* const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-skill") == 0)) {
        gWarpSkill = (skill_t) std::clamp(std::atoi(argv[1]), 0, (int) NUMSKILLS - 1);
        return 2;
    }

    return 0;
}

// A list of all the argument parsing functions
static constexpr ArgParser ARG_PARSERS[] = {
    parseArg_cue,
    parseArg_headless,
    parseArg_datadir,
    parseArg_playdemo,
    parseArg_saveresult,
    parseArg_checkresult,
    parseArg_record,
    parseArg_nomonsters,
    parseArg_pistolstart,
    parseArg_turbo,
    parseArg_server,
    parseArg_client,
    parseArg_file,
    parseArg_nolauncher,
    parseArg_warp,
    parseArg_skill
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs additional validation and sanity checks for program arguments to fix some unsupported/invalid combos
//------------------------------------------------------------------------------------------------------------------------------------------
static void validateAndSanitizeArgs() noexcept {
    if (gbHeadlessMode && (!gPlayDemoFilePath[0])) {
        std::printf("The '-headless' switch can only be used in conjunction with '-playdemo'! Arg will be ignored...\n");
        gbHeadlessMode = false;
    }

    if (gbRecordDemos && gPlayDemoFilePath[0]) {
        std::printf("Can't use '-record' in conjunction with '-playdemo'! Arg will be ignored...\n");
        gbRecordDemos = false;
    }

    if (gbIsNetClient && gbIsNetServer) {
        std::printf("Can't use '-server' in conjunction with '-client'! Arg will be ignored...\n");
        gbIsNetServer = false;
    }

    if (gbRecordDemos && gSaveDemoResultFilePath[0]) {
        std::printf("Can't use '-saveresult' in conjunction with '-record'! Arg will be ignored...\n");
        gSaveDemoResultFilePath = "";
    }

    if (gbRecordDemos && gCheckDemoResultFilePath[0]) {
        std::printf("Can't use '-checkresult' in conjunction with '-record'! Arg will be ignored...\n");
        gCheckDemoResultFilePath = "";
    }

    if ((gWarpMap > 0) && gPlayDemoFilePath[0]) {
        std::printf("The '-warp' argument conflicts with '-playdemo'! Arg will be ignored...\n");
        gWarpMap = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the arguments parser and parses the given list of arguments
//------------------------------------------------------------------------------------------------------------------------------------------
void init(const int argc, const char* const* const argv) noexcept {
    // Consume all the arguments using parsers until there are none.
    // Note: first arg is the program name and that is always skipped.
    int argsLeft = argc - 1;
    const char* const* pCurArgv = argv + 1;

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
            std::printf("Unrecognized command line argument '%s'! Arg will be ignored...\n", pCurArgv[0]);
            argsLeft--;
            pCurArgv++;
        }
    }

    // Additional validation and sanitization
    validateAndSanitizeArgs();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup parsed argument stuff
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    // Reset everything back to its initial state and free any memory allocated (to help leak detection)
    gCueFileOverride = nullptr;
    gbHeadlessMode = false;
    gDataDirPath = "";
    gPlayDemoFilePath = "";
    gSaveDemoResultFilePath = "";
    gCheckDemoResultFilePath = "";
    gbIsNetServer = false;
    gbIsNetClient = false;
    gServerPort = DEFAULT_NET_PORT;
    gbNoMonsters = false;
    gbPistolStart = false;
    gbTurboMode = false;
    gUserWadFiles.clear();
}

const char* getServerHost() noexcept {
    return gServerHost.c_str();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds user WADs specified via the program argument list to the specified WAD list
//------------------------------------------------------------------------------------------------------------------------------------------
void addWadArgsToList(WadList& wadList) noexcept {
    // Add in backwards order so WADs added on the right side of the argument list have higher precedence
    for (int32_t i = (int32_t) gUserWadFiles.size() - 1; i >= 0; --i) {
        wadList.add(gUserWadFiles[i].c_str());
    }
}

END_NAMESPACE(ProgArgs)
