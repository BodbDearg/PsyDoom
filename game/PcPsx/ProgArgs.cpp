//------------------------------------------------------------------------------------------------------------------------------------------
// Program args: parsing and storing command line arguments
//------------------------------------------------------------------------------------------------------------------------------------------
#include "ProgArgs.h"

#include <cstring>
#include <string>

BEGIN_NAMESPACE(ProgArgs)

// The default client and server port
static constexpr uint16_t DEFAULT_NET_PORT = 666;

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

// Host that the client connects to: private so we don't expose std::string everywhere
static std::string gServerHost;

// Format for a function that parses an argument.
// Takes in the current arguments list pointer and the number of arguments left, which is always expected to be at least '1'.
// Returns the number of arguments consumed.
typedef int (*ArgParser)(const int argc, const char** const argv);

static int parseArg_cue([[maybe_unused]] const int argc, const char** const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-cue") == 0)) {
        gCueFileOverride = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_headless([[maybe_unused]] const int argc, const char** const argv) {
    if (std::strcmp(argv[0], "-headless") == 0) {
        gbHeadlessMode = true;
        return 1;
    }

    return 0;
}

static int parseArg_datadir(const int argc, const char** const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-datadir") == 0)) {
        gDataDirPath = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_playdemo(const int argc, const char** const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-playdemo") == 0)) {
        gPlayDemoFilePath = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_saveresult(const int argc, const char** const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-saveresult") == 0)) {
        gSaveDemoResultFilePath = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_checkresult(const int argc, const char** const argv) {
    if ((argc >= 2) && (std::strcmp(argv[0], "-checkresult") == 0)) {
        gCheckDemoResultFilePath = argv[1];
        return 2;
    }

    return 0;
}

static int parseArg_nomonsters(const int argc, const char** const argv) {
    if ((argc >= 1) && (std::strcmp(argv[0], "-nomonsters") == 0)) {
        gbNoMonsters = true;
        return 1;
    }

    return 0;
}

static int parseArg_pistolstart(const int argc, const char** const argv) {
    if ((argc >= 1) && (std::strcmp(argv[0], "-pistolstart") == 0)) {
        gbPistolStart = true;
        return 1;
    }

    return 0;
}

static int parseArg_turbo(const int argc, const char** const argv) {
    if ((argc >= 1) && (std::strcmp(argv[0], "-turbo") == 0)) {
        gbTurboMode = true;
        return 1;
    }

    return 0;
}

static int parseArg_server([[maybe_unused]] const int argc, const char** const argv) {
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

static int parseArg_client([[maybe_unused]] const int argc, const char** const argv) {
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

// A list of all the argument parsing functions
static constexpr ArgParser ARG_PARSERS[] = {
    parseArg_cue,
    parseArg_headless,
    parseArg_datadir,
    parseArg_playdemo,
    parseArg_saveresult,
    parseArg_checkresult,
    parseArg_nomonsters,
    parseArg_pistolstart,
    parseArg_turbo,
    parseArg_server,
    parseArg_client,
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the arguments parser and parses the given list of arguments
//------------------------------------------------------------------------------------------------------------------------------------------
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
            std::printf("Unrecognized command line argument '%s'! Arg will be ignored...\n", pCurArgv[0]);
            argsLeft--;
            pCurArgv++;
        }
    }
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
}

const char* getServerHost() noexcept {
    return gServerHost.c_str();
}

END_NAMESPACE(ProgArgs)
