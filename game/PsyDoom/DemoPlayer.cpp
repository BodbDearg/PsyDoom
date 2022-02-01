//------------------------------------------------------------------------------------------------------------------------------------------
// A module responsible for much of the logic relating to demo playback.
// This includes emulation of the original game's demo playback (classic demos) and also PsyDoom's new extended demo format.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "DemoPlayer.h"

#include "DemoCommon.h"
#include "Doom/Base/i_main.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/UI/errormenu_main.h"
#include "Game.h"
#include "MapHash.h"
#include "SaveDataTypes.h"

#include <cstring>

using namespace DemoCommon;

BEGIN_NAMESPACE(DemoPlayer)

static GameSettings     gPrevGameSettings;                          // The game settings used prior to demo playback (used to restore later)
static padbuttons_t     gPrevPsxCtrlBindings[NUM_BINDABLE_BTNS];    // The previous original PSX gamepad control bindings (used to restore later)
static int32_t          gPrevPsxMouseSensitivity;                   // The previous original PSX mouse sensitivity (used to restore later)
static bool             gbUsingNewDemoFormat;                       // If 'true' then the new PsyDoom demo format is being played
static DemoTickInputs   gPrevTickInputs[MAXPLAYERS];                // The previous inputs of each player: used to avoid encoding repeats

//------------------------------------------------------------------------------------------------------------------------------------------
// Save game settings modified by demo playback (for later restoration)
//------------------------------------------------------------------------------------------------------------------------------------------
static void saveModifiedGameSettings() noexcept {
    gPrevGameSettings = Game::gSettings;
    std::memcpy(gPrevPsxCtrlBindings, gCtrlBindings, sizeof(gCtrlBindings));
    gPrevPsxMouseSensitivity = gPsxMouseSensitivity;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Restore game settings previously modified by demo playback
//------------------------------------------------------------------------------------------------------------------------------------------
static void restoreModifiedGameSettings() noexcept {
    // Restore all the stuff we remembered
    Game::gSettings = gPrevGameSettings;
    std::memcpy(gCtrlBindings, gPrevPsxCtrlBindings, sizeof(gCtrlBindings));
    gPsxMouseSensitivity = gPrevPsxMouseSensitivity;

    // This should always be '0' upon first starting demo playback so we don't need to remember it specifically
    gCurPlayerIndex = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verification of various demo properties
//------------------------------------------------------------------------------------------------------------------------------------------
static bool verifyDemoFileVersion(const uint32_t version) noexcept {
    if (version == DEMO_FILE_VERSION) {
        return true;
    } else {
        RunDemoErrorMenu_InvalidDemoVersion();
        return false;
    }
}

static bool verifyDemoSkill(const skill_t skill) noexcept {
    const int32_t iSkill = (int32_t) skill;

    if ((iSkill >= 0) && (iSkill < NUMSKILLS)) {
        return true;
    } else {
        RunDemoErrorMenu_InvalidSkill();
        return false;
    }
}

static bool verifyDemoMapNum(const int32_t mapNum) noexcept {
    if ((mapNum >= 1) && (mapNum <= Game::getNumMaps())) {
        return true;
    } else {
        RunDemoErrorMenu_InvalidMapNumber();
        return false;
    }
}

static bool verifyDemoGameType(const gametype_t gameType) noexcept {
    const int32_t iGameType = (int32_t) gameType;

    if ((iGameType >= 0) && (iGameType < NUMGAMETYPES)) {
        return true;
    } else {
        RunDemoErrorMenu_InvalidGameType();
        return false;
    }
}

static bool verifyDemoPlayerIndex(const gametype_t gameType, const int32_t playerIdx) noexcept {
    const bool bValidPlayerIdx = (
        (playerIdx == 0) ||
        ((gameType != gt_single) && (playerIdx == 1))
    );

    if (bValidPlayerIdx) {
        return true;
    } else {
        RunDemoErrorMenu_InvalidPlayerNum();
        return false;
    }
}

static bool verifyMapHash(const uint64_t hashWord1, const uint64_t hashWord2) noexcept {
    const bool bValidHash = ((hashWord1 == MapHash::gWord1) && (hashWord2 == MapHash::gWord2));

    if (bValidHash) {
        return true;
    } else {
        RunDemoErrorMenu_BadMapHash();
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function to check if the specified number of values can be read from the demo buffer
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static bool demo_canRead(const uint32_t numElems = 1) noexcept {
    ASSERT(gpDemo_p);
    const size_t numBytes = sizeof(T) * numElems;
    return (gpDemo_p + numBytes <= gpDemoBufferEnd);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Peeks the next value from the demo buffer without consuming it.
// Useful for looking ahead in the demo buffer speculatively.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static T demo_peek() THROWS {
    ASSERT(gpDemo_p);
    constexpr size_t NUM_BYTES = sizeof(T);

    if (gpDemo_p + NUM_BYTES > gpDemoBufferEnd)
        throw "Unexpected EOF!";

    T value;
    std::memcpy(&value, gpDemo_p, NUM_BYTES);
    return value;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function for reading a single value from the demo buffer
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static T demo_read() THROWS {
    ASSERT(gpDemo_p);
    constexpr size_t NUM_BYTES = sizeof(T);

    if (gpDemo_p + NUM_BYTES > gpDemoBufferEnd)
        throw "Unexpected EOF!";

    T value;
    std::memcpy(&value, gpDemo_p, NUM_BYTES);
    gpDemo_p += NUM_BYTES;
    return value;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function for reading multiple values from the demo buffer
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void demo_read(T* const pElems, const uint32_t numElems) THROWS {
    ASSERT(gpDemo_p);
    const size_t numBytes = sizeof(T) * numElems;

    if (gpDemo_p + numBytes > gpDemoBufferEnd)
        throw "Unexpected EOF!";

    std::memcpy(pElems, gpDemo_p, numBytes);
    gpDemo_p += numBytes;
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
// Does the logic for before map load for the new demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool onBeforeMapLoad_newDemoFormat() noexcept {
    try {
        // Consume the '-1' signature (32-bit integer) for the new demo format
        demo_read<int32_t>();

        // Read and verify the basic demo properties
        const uint32_t demoFileVersion = Endian::littleToHost(demo_read<uint32_t>());
        const skill_t skill = Endian::littleToHost(demo_read<skill_t>());
        const int32_t mapNum = Endian::littleToHost(demo_read<int32_t>());
        const gametype_t gameType = Endian::littleToHost(demo_read<gametype_t>());
        const int32_t playerIdx = Endian::littleToHost(demo_read<int32_t>());

        const bool bValidDemoProperties = (
            verifyDemoFileVersion(demoFileVersion) &&
            verifyDemoSkill(skill) &&
            verifyDemoMapNum(mapNum) &&
            verifyDemoGameType(gameType) &&
            verifyDemoPlayerIndex(gameType, playerIdx)
        );

        if (!bValidDemoProperties)
            return false;

        // Read the game settings
        GameSettings settings = demo_read<GameSettings>();
        settings.endianCorrect();
        Game::gSettings = settings;

        // Set the current player index (important for multiplayer demos)
        gCurPlayerIndex = playerIdx;

        // Basic game initialization prior to map loading
        G_InitNew(skill, mapNum, gameType);
    }
    catch (...) {
        // Reached an unexpected end to the demo file!
        RunDemoErrorMenu_UnexpectedEOF();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the logic for before map load for the old demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool onBeforeMapLoad_oldDemoFormat() noexcept {
    try {
        // Read and verify the demo skill and map number
        const skill_t skill = Endian::littleToHost(demo_read<skill_t>());
        const int32_t mapNum = Endian::littleToHost(demo_read<int32_t>());
        const bool bValidDemoProperties = (verifyDemoSkill(skill) && verifyDemoMapNum(mapNum));

        if (!bValidDemoProperties)
            return false;

        // Read the control bindings for the demo: for original PSX Doom there are 8 bindings, for Final Doom there are 10.
        // Need to adjust demo reading accordingly depending on which game version we are dealing with.
        if (Game::isFinalDoom()) {
            demo_read(gCtrlBindings, NUM_BINDABLE_BTNS);
        } else {
            // Note: original Doom did not have the move forward/backward bindings (due to no mouse support) - hence they are zeroed here:
            demo_read(gCtrlBindings, 8);
            gCtrlBindings[8] = 0;
            gCtrlBindings[9] = 0;
        }

        // Endian correct the controls read from the demo
        if constexpr (!Endian::isLittle()) {
            for (uint32_t i = 0; i < NUM_BINDABLE_BTNS; ++i) {
                gCtrlBindings[i] = Endian::littleToHost(gCtrlBindings[i]);
            }
        }

        // For Final Doom read the mouse sensitivity
        if (Game::isFinalDoom()) {
            gPsxMouseSensitivity = Endian::littleToHost(demo_read<int32_t>());
        }

        // Determine the game settings to play back this classic demo correctly, depending on what game is being used.
        // N.B: this *MUST* be done before loading the level, as some of the settings (e.g nomonsters) affect level loading.
        Game::getClassicDemoGameSettings(Game::gSettings);

        // Basic game initialization prior to map loading
        G_InitNew(skill, mapNum, gt_single);
    }
    catch (...) {
        // Reached an unexpected end to the demo file!
        RunDemoErrorMenu_UnexpectedEOF();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the tick inputs for the new demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readTickInputs_newDemoFormat() noexcept {
    try {
        // Firstly read the status byte.
        // This will contain the following info for the following bits:
        // 
        //  7       Whether to use the previous tick inputs for player 1
        //  6       Whether to use the previous tick inputs for player 2
        //  3..5    Elapsed vblanks for player 1 (0-7)
        //  0..2    Elapsed vblanks for player 2 (0-7)
        //
        const uint8_t statusByte = demo_read<uint8_t>();

        // Set the elapsed vblank counts firstly
        gPlayersElapsedVBlanks[0] = (statusByte >> 3) & 0x7;
        gPlayersElapsedVBlanks[1] = (statusByte >> 0) & 0x7;

        // Read the tick inputs for each player or re-use the previous ones
        const auto readOrReuseTickInputs = [](const bool bReadNewInputs, const uint32_t playerIdx) {
            if (bReadNewInputs) {
                DemoTickInputs tickInputs = demo_read<DemoTickInputs>();
                DemoCommon::endianCorrect(tickInputs);
                tickInputs.deserializeTo(gTickInputs[playerIdx]);
                gPrevTickInputs[playerIdx] = tickInputs;
            } else {
                gPrevTickInputs[playerIdx].deserializeTo(gTickInputs[playerIdx]);
            }
        };

        readOrReuseTickInputs(statusByte & 0x80, 0);
        readOrReuseTickInputs(statusByte & 0x40, 1);
    }
    catch (...) {
        // Reached an unexpected end to the demo file!
        RunDemoErrorMenu_UnexpectedEOF();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the tick inputs for the old demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readTickInputs_oldDemoFormat() noexcept {
    try {
        const padbuttons_t padBtns = Endian::littleToHost(demo_read<uint32_t>());
        gTicButtons = padBtns;
        P_PsxButtonsToTickInputs(padBtns, gCtrlBindings, gTickInputs[gCurPlayerIndex]);
        return true;
    }
    catch (...) {
        RunDemoErrorMenu_UnexpectedEOF();
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// This should be called prior to loading the map.
// Reads some demo header info and checks that its contents are valid.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
bool onBeforeMapLoad() noexcept {
    // Remember modified settings for later restoration and setup the current demo buffer pointer
    saveModifiedGameSettings();
    gpDemo_p = gpDemoBuffer;

    // Which demo format are we dealing with?
    // The first 32-bit integer in the stream tells us this:
    try {
        gbUsingNewDemoFormat = (demo_peek<int32_t>() == -1);

        if (gbUsingNewDemoFormat) {
            return onBeforeMapLoad_newDemoFormat();
        } else {
            return onBeforeMapLoad_oldDemoFormat();
        }
    }
    catch (...) {
        // Reached an unexpected end to the demo file!
        RunDemoErrorMenu_UnexpectedEOF();
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// This should be called after loading the demo's map and before starting playback.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
bool onAfterMapLoad() noexcept {
    // This only does stuff for the new demo format!
    if (!gbUsingNewDemoFormat)
        return true;

    try {
        // Verify the map hash matches what we expect
        const uint64_t mapHashWord1 = Endian::hostToLittle(demo_read<uint64_t>());
        const uint64_t mapHashWord2 = Endian::hostToLittle(demo_read<uint64_t>());

        if (!verifyMapHash(mapHashWord1, mapHashWord2))
            return false;

        // Read the details for all the players starting the game, including health and ammo etc.
        const int32_t numPlayers = (gNetGame != gt_single) ? 2 : 1;

        for (int32_t playerIdx = 0; playerIdx < numPlayers; ++playerIdx) {
            // Deserialize the player struct firstly
            SavedPlayerT srcPlayer = demo_read<SavedPlayerT>();
            DemoCommon::endianCorrect(srcPlayer);
            player_t& dstPlayer = gPlayers[playerIdx];
            srcPlayer.deserializeTo(dstPlayer, true);

            // Synchronize the health of the player map object with the player
            dstPlayer.mo->health = dstPlayer.health;
        }

        // Init the previous tick inputs for all players to predefined/known starting values
        initPrevTickInputs();
    }
    catch (...) {
        // Reached an unexpected end to the demo file!
        RunDemoErrorMenu_UnexpectedEOF();
        return false;
    }

    // Success if we get to here!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the end of the demo has been reached
//------------------------------------------------------------------------------------------------------------------------------------------
bool hasReachedDemoEnd() noexcept {
    if (!gpDemo_p)
        return true;

    if (gbUsingNewDemoFormat) {
        return (!demo_canRead<DemoTickInputs>());
    } else {
        return (!demo_canRead<padbuttons_t>());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the current demo being played is in the new demo format
//------------------------------------------------------------------------------------------------------------------------------------------
bool isUsingNewDemoFormat() noexcept {
    return gbUsingNewDemoFormat;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if playback of a classic format demo is currently underway
//------------------------------------------------------------------------------------------------------------------------------------------
bool isPlayingAClassicDemo() noexcept {
    return (gbDemoPlayback && (!gbUsingNewDemoFormat));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the tick inputs for this tick.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
bool readTickInputs() noexcept {
    if (gbUsingNewDemoFormat) {
        return readTickInputs_newDemoFormat();
    } else {
        return readTickInputs_oldDemoFormat();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should be called when demo playback is done, or when it has been aborted due to an error
//------------------------------------------------------------------------------------------------------------------------------------------
void onPlaybackDone() noexcept {
    // Restore any changed game settings and reset the demo pointer
    restoreModifiedGameSettings();
    gpDemo_p = nullptr;

    // Cleanup all globals and reset them to a default state
    std::memset(gPrevTickInputs, 0, sizeof(gPrevTickInputs));
    gbUsingNewDemoFormat = false;
    gPrevPsxMouseSensitivity = {};
    std::memset(gPrevPsxCtrlBindings, 0, sizeof(gPrevPsxCtrlBindings));
    gPrevGameSettings = {};
}

END_NAMESPACE(DemoPlayer)
