//------------------------------------------------------------------------------------------------------------------------------------------
// A module responsible for most of the logic relating to demo recording.
// The demos are recorded in a new format specific to PsyDoom that has greater capabilities than the original format.
// Improvements include greater timing resolution (30Hz vs 15Hz ticks), analog movement and multiplayer support.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "DemoRecorder.h"

#include "DemoCommon.h"
#include "Doom/Base/i_main.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Endian.h"
#include "FileOutputStream.h"
#include "Game.h"
#include "MapHash.h"
#include "SaveDataTypes.h"
#include "Utils.h"

#include <algorithm>

using namespace DemoCommon;

BEGIN_NAMESPACE(DemoRecorder)

typedef std::unique_ptr<FileOutputStream> DemoFilePtr;

static std::string      gDemoFilePath;                  // Path of the demo file being recorded to
static DemoFilePtr      gpDemoFile;                     // The demo file currently being recorded to
static DemoTickInputs   gPrevTickInputs[MAXPLAYERS];    // The previous inputs of each player: used to avoid encoding repeats

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the path of the demo file that will be recorded for the current map
//------------------------------------------------------------------------------------------------------------------------------------------
static std::string getDemoFilePath() noexcept {
    const std::string userDataFolder = Utils::getOrCreateUserDataFolder();
    char demoFileName[64];
    std::snprintf(demoFileName, C_ARRAY_SIZE(demoFileName), "DEMO_MAP%02d.LMP", gGameMap);
    return userDataFolder + demoFileName;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Opens the demo file for recording (may fail)
//------------------------------------------------------------------------------------------------------------------------------------------
static void openDemoFile() THROWS {
    gDemoFilePath = getDemoFilePath();
    gpDemoFile = std::make_unique<FileOutputStream>(gDemoFilePath.c_str(), false);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes up the current demo file and therefore ends recording
//------------------------------------------------------------------------------------------------------------------------------------------
static void closeDemoFile() noexcept {
    gDemoFilePath.clear();
    gpDemoFile.reset();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Writes the demo file header.
// This includes all the information about the game and the starting state for all players.
// Note: expects the demo file to be open for writing!
//------------------------------------------------------------------------------------------------------------------------------------------
static void writeDemoHeader() THROWS {
    ASSERT(gpDemoFile);

    // Begin the demo with a 32-bit integer set to '-1'.
    // These is a signature to distinguish the new demo lump format from the old one.
    // In the old demo format this integer was the 'skill' field.
    gpDemoFile->write<int32_t>(Endian::hostToLittle(-1));

    // Record the current demo file version
    gpDemoFile->write<uint32_t>(Endian::hostToLittle(DEMO_FILE_VERSION));

    // Record the skill, map number, whether this is multiplayer and which player the demo is being played for
    gpDemoFile->write<int32_t>(Endian::hostToLittle(gGameSkill));
    gpDemoFile->write<int32_t>(Endian::hostToLittle(gGameMap));
    gpDemoFile->write<int32_t>(Endian::hostToLittle(gNetGame));
    gpDemoFile->write<int32_t>(Endian::hostToLittle(gCurPlayerIndex));

    // Record the game settings
    {
        GameSettings settings = Game::gSettings;
        settings.endianCorrect();
        gpDemoFile->write(settings);
    }

    // Record the hash of the map being played so we can verify the same map is being played for the demo
    gpDemoFile->write<uint64_t>(Endian::hostToLittle(MapHash::gWord1));
    gpDemoFile->write<uint64_t>(Endian::hostToLittle(MapHash::gWord2));

    // Record details for all the players starting the game, including health and ammo etc.
    const int32_t numPlayers = (gNetGame != gt_single) ? 2 : 1;

    for (int32_t playerIdx = 0; playerIdx < numPlayers; ++playerIdx) {
        // Convert the player into the save file format, endian correct and write it
        SavedPlayerT player = {};
        player.serializeFrom(gPlayers[playerIdx]);
        DemoCommon::endianCorrect(player);
        gpDemoFile->write(player);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Writes the specified 'DemoTickInputs' structure to the demo file
//------------------------------------------------------------------------------------------------------------------------------------------
static void writeTickInputs(const DemoTickInputs& tickInputs) noexcept {
    ASSERT(isRecording());

    if constexpr (Endian::isLittle()) {
        // Little endian CPU: just write the inputs as-is
        gpDemoFile->write(tickInputs);
    }
    else {
        // Big endian CPU: have to convert the tick inputs to little endian before writing
        DemoTickInputs tickInputsLE = tickInputs;
        tickInputsLE.byteSwap();
        gpDemoFile->write(tickInputsLE);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: initializes the previous tick inputs set at the start of demo recording
//------------------------------------------------------------------------------------------------------------------------------------------
static void initPrevTickInputs() noexcept {
    std::memset(gPrevTickInputs, 0, sizeof(gPrevTickInputs));

    for (DemoTickInputs& inputs : gPrevTickInputs) {
        inputs.directSwitchToWeapon = wp_nochange;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handles an error writing to the demo file
//------------------------------------------------------------------------------------------------------------------------------------------
static void handleDemoWriteError() noexcept {
    // Close up the demo file to flush any writes that we can
    const std::string demoPath = gDemoFilePath;
    closeDemoFile();

    // Issue a fatal error to let the user know about the problem
    I_Error("Error writing to demo file '%s'!", gDemoFilePath.c_str());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begins the process of recording a demo.
// If recording fails then a fatal error is issued.
//------------------------------------------------------------------------------------------------------------------------------------------
void begin() noexcept {
    try {
        openDemoFile();
        writeDemoHeader();
        initPrevTickInputs();
    } catch (...) {
        handleDemoWriteError();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ends recording of the current demo.
// Must only be called when the demo is recording!
//------------------------------------------------------------------------------------------------------------------------------------------
void end() noexcept {
    ASSERT(isRecording());

    try {
        gpDemoFile->flush();
        closeDemoFile();
    } catch (...) {
        handleDemoWriteError();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the demo is currently recording
//------------------------------------------------------------------------------------------------------------------------------------------
bool isRecording() noexcept {
    return (gpDemoFile != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Records the inputs and player timings for 1 tick of the game.
// Must only be called when the demo is recording!
//------------------------------------------------------------------------------------------------------------------------------------------
void recordTick() noexcept {
    ASSERT(isRecording());

    // Get the inputs for player 1 and 2
    DemoTickInputs p1Inputs = {};
    DemoTickInputs p2Inputs = {};
    p1Inputs.serializeFrom(gTickInputs[0]);
    p2Inputs.serializeFrom(gTickInputs[1]);

    // Firstly make up the status byte.
    // This will contain the following info for the following bits:
    // 
    //  7       Whether to use the previous tick inputs for player 1
    //  6       Whether to use the previous tick inputs for player 2
    //  3..5    Elapsed vblanks for player 1 (0-7)
    //  0..2    Elapsed vblanks for player 2 (0-7)
    //
    uint8_t statusByte = 0;

    if (!p1Inputs.equals(gPrevTickInputs[0])) {
        statusByte |= 0x80;
    }

    if (!p2Inputs.equals(gPrevTickInputs[1])) {
        statusByte |= 0x40;
    }

    statusByte |= ((uint8_t) std::clamp(gPlayersElapsedVBlanks[0], 0, 7)) << 3;
    statusByte |= ((uint8_t) std::clamp(gPlayersElapsedVBlanks[1], 0, 7));

    // Write the status byte followed by the inputs if they have changed
    try {
        gpDemoFile->write(statusByte);

        if (statusByte & 0x80) {
            writeTickInputs(p1Inputs);
        }

        if (statusByte & 0x40) {
            writeTickInputs(p2Inputs);
        }
    }
    catch (...) {
        handleDemoWriteError();
    }

    // Remember the current inputs as the previous ones
    gPrevTickInputs[0] = p1Inputs;
    gPrevTickInputs[1] = p2Inputs;
}

END_NAMESPACE(DemoRecorder)
