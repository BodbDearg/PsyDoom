//------------------------------------------------------------------------------------------------------------------------------------------
// A module responsible for much of the logic relating to demo playback.
// This includes emulation of the original game's demo playback (classic demos) and also PsyDoom's new extended demo format.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "DemoPlayer.h"

#include "DemoCommon.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Game/p_user.h"
#include "Doom/Renderer/r_main.h"
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
static DemoFormat       gPlayingDemoFormat;                         // Which format of demo is currently being played
static DemoTickInputs   gPrevTickInputs[MAXPLAYERS];                // The previous inputs of each player: used to avoid encoding repeats

//------------------------------------------------------------------------------------------------------------------------------------------
// Tick inputs for the 'GEC Master Edition' demo format
//------------------------------------------------------------------------------------------------------------------------------------------
struct GecDemoTickInputs {
    uint32_t    buttons;
    uint8_t     analogTurn;
    uint8_t     analogTurnStickY;   // Unused currently: y-axis motion for the analog turn stick
    uint8_t     analogSideMove;
    uint8_t     analogForwardMove;
};

static_assert(sizeof(GecDemoTickInputs) == 8);

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
static bool verifyDemoFileVersion(const uint32_t demoFileVersion) noexcept {
    // For now this is all we need to determine if a demo file version is supported.
    // If there is a valid 'GameSettings' version mapping and size then we should be good to read the demo.
    const int32_t gameSettingsVersion = GameSettingUtils::getGameSettingsVersionForDemoFileVersion(demoFileVersion);

    if (gameSettingsVersion >= 1) {
        if (GameSettingUtils::getGameSettingsSize(gameSettingsVersion) >= 1)
            return true;
    }

    // Invalid demo version!
    RunDemoErrorMenu_InvalidDemoVersion();
    return false;
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
// Helper function for skipping past multiple values in the demo buffer
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void demo_skip(const uint32_t numElems) THROWS {
    ASSERT(gpDemo_p);
    const size_t numBytes = sizeof(T) * numElems;

    if (gpDemo_p + numBytes > gpDemoBufferEnd)
        throw "Unexpected EOF!";
    
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
// Does the logic for before map load for the original PSX Doom demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool onBeforeMapLoad_classicDemoFormat() noexcept {
    try {
        // Read and verify the demo skill and map number.
        // Note: the map number can be remapped depending on the current game constants.
        const skill_t skill = Endian::littleToHost(demo_read<skill_t>());
        const int32_t origMapNum = Endian::littleToHost(demo_read<int32_t>());
        const int32_t mapNum = (gCurBuiltInDemo.mapNumOverrider) ? gCurBuiltInDemo.mapNumOverrider(origMapNum) : origMapNum;
        const bool bValidDemoProperties = (verifyDemoSkill(skill) && verifyDemoMapNum(mapNum));

        if (!bValidDemoProperties)
            return false;

        // Read the control bindings for the demo: for original PSX Doom there are 8 bindings, for Final Doom there are 10.
        // Need to adjust demo reading accordingly depending on which game version we are dealing with.
        if (gCurBuiltInDemo.bFinalDoomDemo) {
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
        if (gCurBuiltInDemo.bFinalDoomDemo) {
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
// Does the logic for before map load for PsyDoom's new demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool onBeforeMapLoad_psydoomDemoFormat() noexcept {
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

        // Make sure there is enough data to read all the game settings.
        // Note that 'verifyDemoFileVersion()' should have validated both the game settings version and size already.
        const int32_t gameSettingsVersion = GameSettingUtils::getGameSettingsVersionForDemoFileVersion(demoFileVersion);
        const int32_t gameSettingsSize = GameSettingUtils::getGameSettingsSize(gameSettingsVersion);
        ASSERT(gameSettingsVersion >= 1);
        ASSERT(gameSettingsSize >= 1);

        if (!demo_canRead<std::byte>(gameSettingsSize)) {
            RunDemoErrorMenu_UnexpectedEOF();
            return false;
        }

        // Read (and migrate, if needed) the game settings and skip past those bytes.
        // Note that 'readAndMigrateGameSettings' is always expected to succeed here: if it doesn't then that is an internal logic error.
        GameSettings settings = {};

        if (!GameSettingUtils::readAndMigrateGameSettings(gameSettingsVersion, gpDemo_p, settings)) {
            RunDemoErrorMenu_UnexpectedError();
            return false;
        }

        demo_skip<std::byte>(gameSettingsSize);
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
// Does the logic for before map load for the 'GEC Master Edition' demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool onBeforeMapLoad_gecFormat() noexcept {
    try {
        // Consume the demo signature word
        demo_read<int32_t>();

        // Read the game flags word and apply the appropriate game settings based on this.
        {
            const uint32_t gameFlags = demo_read<uint32_t>();

            // Start by using the classic Doom settings
            GameSettings& settings = Game::gSettings;
            Game::getClassicDemoGameSettings(settings);

            // Apply the game flags to the game settings.
            // Note that the following flags are unused due to them being user preferences which don't affect gameplay:
            // 
            //  ShowFPS                 0x2
            //  UseLoadingBar           0x4
            //  ControllerVibration     0x8
            //
            const auto applyGameFlag = [&](uint8_t& bSettingsField, const uint32_t gameFlagMask) noexcept {
                bSettingsField = ((gameFlags & gameFlagMask) != 0);
            };

            applyGameFlag(settings.bUsePalTimings,                  0x1);
            applyGameFlag(settings.bUseFinalDoomPlayerMovement,     0x10);
            applyGameFlag(settings.bUsePlayerRocketBlastFix,        0x20);
            applyGameFlag(settings.bUseSuperShotgunDelayTweak,      0x40);
            applyGameFlag(settings.bUseMoveInputLatencyTweak,       0x80);
            applyGameFlag(settings.bUseItemPickupFix,               0x100);
            applyGameFlag(settings.bFixViewBobStrength,             0x200);
            applyGameFlag(settings.bFixGravityStrength,             0x400);
            applyGameFlag(settings.bUseLostSoulSpawnFix,            0x800);
            applyGameFlag(settings.bUseLineOfSightOverflowFix,      0x1000);
            applyGameFlag(settings.bFixOutdoorBulletPuffs,          0x2000);
            applyGameFlag(settings.bFixLineActivation,              0x4000);
            applyGameFlag(settings.bFixBlockingGibsBug,             0x8000);
            applyGameFlag(settings.bFixSoundPropagation,            0x10000);
            applyGameFlag(settings.bAllowMultiMapPickup,            0x20000);
            applyGameFlag(settings.bFixSpriteVerticalWarp,          0x40000);
            applyGameFlag(settings.bFixKillCount,                   0x80000);

            // Tweaks specifically to match the behavior of GEC ME
            settings.lostSoulSpawnLimit = 16;
            settings.bAllowMovementCancellation = settings.bUseFinalDoomPlayerMovement;
        }

        // Read and verify the demo skill and map number.
        // Note: the map number can be remapped depending on the current game constants.
        const skill_t skill = Endian::littleToHost(demo_read<skill_t>());
        const int32_t origMapNum = Endian::littleToHost(demo_read<int32_t>());
        const int32_t mapNum = (gCurBuiltInDemo.mapNumOverrider) ? gCurBuiltInDemo.mapNumOverrider(origMapNum) : origMapNum;
        const bool bValidDemoProperties = (verifyDemoSkill(skill) && verifyDemoMapNum(mapNum));

        if (!bValidDemoProperties)
            return false;

        // Read the control bindings for the demo.
        // Note: this is the 'Final Doom' format where there are 10 bindings.
        demo_read(gCtrlBindings, NUM_BINDABLE_BTNS);

        // Endian correct the controls read from the demo
        if constexpr (!Endian::isLittle()) {
            for (uint32_t i = 0; i < NUM_BINDABLE_BTNS; ++i) {
                gCtrlBindings[i] = Endian::littleToHost(gCtrlBindings[i]);
            }
        }

        // Read the mouse sensitivity
        gPsxMouseSensitivity = Endian::littleToHost(demo_read<int32_t>());

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
// Reads the tick inputs for the original PSX Doom demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readTickInputs_classicDemoFormat() noexcept {
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
// Reads the tick inputs for PsyDoom's new demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readTickInputs_psydoomDemoFormat() noexcept {
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
// Reads the tick inputs for the 'GEC Master Edition' demo format.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readTickInputs_gecDemoFormat() noexcept {
    try {
        // Read and endian correct the inputs
        GecDemoTickInputs gecInputs = demo_read<GecDemoTickInputs>();
        endianCorrect(gecInputs.buttons);
        endianCorrect(gecInputs.analogTurn);
        endianCorrect(gecInputs.analogTurnStickY);
        endianCorrect(gecInputs.analogSideMove);
        endianCorrect(gecInputs.analogForwardMove);

        // Convert digital and mouse inputs to PsyDoom's tick format
        TickInputs& tickInputs = gTickInputs[gCurPlayerIndex];
        P_PsxButtonsToTickInputs(gecInputs.buttons, gCtrlBindings, tickInputs);

        // Helper: converts a byte for an analog input to the range -127 to +127
        const auto analogByteTo127 = [](const uint8_t analogInputByte) noexcept -> int32_t {
            return (analogInputByte & 0x80) ? (int32_t) analogInputByte - 0x80 : (int32_t) analogInputByte - 0x7F;
        };

        // Helper: tells if the specified analog input (in the range -127 to +127) is outside of the deadzone
        const auto isAnalog127OutsideDeadzone = [](const int32_t analogInput127) noexcept -> bool {
            return (std::abs(analogInput127) >= 24);
        };

        // Helper: converts from an analog input in the range -127 to +127 to the range -1 to +1 in 16.16 format
        const auto normalizeAnalog127 = [](const int32_t analogInput127) noexcept -> fixed_t {
            return d_lshift<FRACBITS>(analogInput127) / 127;
        };

        // Convert analog turning to PsyDoom's format
        const int32_t analogTurning127 = analogByteTo127(gecInputs.analogTurn);

        if (isAnalog127OutsideDeadzone(analogTurning127)) {
            // Slow and fast turning speeds (when not running and running respectively)
            constexpr fixed_t TURN_SPEED[2]      = { 600, 1000 };
            constexpr fixed_t FAST_TURN_SPEED[2] = { 800, 1400 };

            // Compute the turning amount and save
            const fixed_t analogSensitivity = d_lshift<FRACBITS>(d_lshift<1>(gPsxMouseSensitivity)) / 100;
            const fixed_t slowTurnSpeed = TURN_SPEED[tickInputs.fRun() ? 1 : 0];
            const fixed_t fastTurnSpeed = FAST_TURN_SPEED[tickInputs.fRun() ? 1 : 0];

            const fixed_t analogTurnFrac = normalizeAnalog127(analogTurning127);
            const fixed_t turnSpeedMix = std::abs(analogTurnFrac);
            const fixed_t turnSpeedUnscaled = slowTurnSpeed * (FRACUNIT - turnSpeedMix) + fastTurnSpeed * turnSpeedMix;
            const fixed_t turnSpeed = d_rshift<FRACBITS>(FixedMul(turnSpeedUnscaled, analogSensitivity));
            const angle_t turnAmount = d_rshift<2>(FixedMul(turnSpeed, analogTurnFrac));

            tickInputs.setAnalogTurn(d_lshift<TURN_TO_ANGLE_SHIFT>(turnAmount));
        }
        else {
            tickInputs.setAnalogTurn(0);
        }

        // Convert analog movement for the tick to PsyDoom's format
        const int32_t analogSideMove127 = analogByteTo127(gecInputs.analogSideMove);
        const int32_t analogForwardMove127 = analogByteTo127(gecInputs.analogForwardMove);

        if (isAnalog127OutsideDeadzone(analogSideMove127) || isAnalog127OutsideDeadzone(analogForwardMove127)) {
            // Convert the analog input to PsyDoom's own single byte encoding
            tickInputs._analogSideMove = (uint8_t) std::abs(analogSideMove127);
            tickInputs._analogSideMove |= (analogSideMove127 < 0) ? 0x80 : 0x00;
            tickInputs._analogForwardMove = (uint8_t) std::abs(analogForwardMove127);
            tickInputs._analogForwardMove |= (analogForwardMove127 >= 0) ? 0x80 : 0x00;     // Note: invert forward movement by checking '>= 0' instead of '< 0'

            // In the GEC demo format any digital movement gets cancelled when making analog movements
            tickInputs.fStrafeLeft() = false;
            tickInputs.fStrafeRight() = false;
            tickInputs.fMoveForward() = false;
            tickInputs.fMoveBackward() = false;
            tickInputs.psxMouseDy = 0;

            if (tickInputs.fStrafe()) {
                // When strafing is enabled then cancel all digital and mouse turning too (since they become movements).
                // Note that analog turning is left alone however, since it's not affected by this modifier.
                tickInputs.fTurnLeft() = false;
                tickInputs.fTurnRight() = false;
                tickInputs.psxMouseDx = 0;
            }
        }
        else {
            tickInputs.setAnalogSideMove(0);
            tickInputs.setAnalogForwardMove(0);
        }

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
    // The first 32-bit integer in the stream tells us this, depending on whether it matches certain format signatures:
    try {
        constexpr int32_t PSYDOOM_DEMO_SIGNATURE = -1;
        constexpr int32_t GEC_ME_DEMO_SIGNATURE = 0x454D5644;   // 'DVME' in little endian

        const int32_t firstDemoWord = Endian::littleToHost(demo_peek<int32_t>());

        if (firstDemoWord == PSYDOOM_DEMO_SIGNATURE) {
            gPlayingDemoFormat = DemoFormat::PsyDoom;
            return onBeforeMapLoad_psydoomDemoFormat();
        }
        else if (firstDemoWord == GEC_ME_DEMO_SIGNATURE) {
            gPlayingDemoFormat = DemoFormat::GecMe;
            return onBeforeMapLoad_gecFormat();
        }
        else {
            // If we don't find a matching format signature then it must be a classic demo (first word is 'skill')
            gPlayingDemoFormat = DemoFormat::Classic;
            return onBeforeMapLoad_classicDemoFormat();
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
    // This only does stuff for PsyDoom's new demo format!
    if (gPlayingDemoFormat != DemoFormat::PsyDoom)
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

    switch (gPlayingDemoFormat) {
        case DemoFormat::None:      return true;
        case DemoFormat::Classic:   return (!demo_canRead<padbuttons_t>());
        case DemoFormat::PsyDoom:   return (!demo_canRead<DemoTickInputs>());
        case DemoFormat::GecMe:     return (!demo_canRead<GecDemoTickInputs>());
    }

    FatalErrors::raise("DemoPlayer::hasReachedDemoEnd: unhandled demo format!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if level in the demo being played was completed (intermission screen reached)
//------------------------------------------------------------------------------------------------------------------------------------------
bool wasLevelCompleted() noexcept {
    // The demo player should always exit before this flag has been set, so this case should never be true.
    // If we DO see the flag set for some reason however, then that's the best indication the level was completed.
    if (gbDidCompleteLevel)
        return true;

    // This is the normal situation with demos.
    // We have to go through all the thinkers and find a scheduled delayed 'exit' action.
    // That indicates level exit to the intermission screen is just about to take place.
    for (thinker_t* pThinker = gThinkerCap.next; pThinker != &gThinkerCap; pThinker = pThinker->next) {
        if ((void*) pThinker->function == (void*) &T_DelayedAction) {
            delayaction_t& delayedAction = reinterpret_cast<delayaction_t&>(*pThinker);

            if (delayedAction.actionfunc == G_CompleteLevel)
                return true;
        }
    }

    // Level wasn't finished!
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells which format of demo is currently being played ('None' if no demo is being played)
//------------------------------------------------------------------------------------------------------------------------------------------
DemoFormat getPlayingDemoFormat() noexcept {
    return gPlayingDemoFormat;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the music which normally plays for a map should be overriden due to a demo being played.
// If this is the case then the credits music should play instead.
//------------------------------------------------------------------------------------------------------------------------------------------
bool shouldOverrideMapMusicForDemo() noexcept {
    // Classic and GEC ME demos override map music
    return (
        (gPlayingDemoFormat == DemoFormat::Classic) ||
        (gPlayingDemoFormat == DemoFormat::GecMe)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if player turning actions should be capped to 30 Hz
//------------------------------------------------------------------------------------------------------------------------------------------
bool isPlayerTurning30HzCapped() noexcept {
    return (
        (gPlayingDemoFormat == DemoFormat::Classic) ||
        (gPlayingDemoFormat == DemoFormat::GecMe)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the tick inputs for this tick.
// Returns 'false' if the demo should not be played due to some kind of error.
//------------------------------------------------------------------------------------------------------------------------------------------
bool readTickInputs() noexcept {
    switch (gPlayingDemoFormat) {
        case DemoFormat::None:      return false;
        case DemoFormat::Classic:   return readTickInputs_classicDemoFormat();
        case DemoFormat::PsyDoom:   return readTickInputs_psydoomDemoFormat();
        case DemoFormat::GecMe:     return readTickInputs_gecDemoFormat();
    }

    FatalErrors::raise("DemoPlayer::readTickInputs: unhandled demo format!");
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
    gPlayingDemoFormat = DemoFormat::None;
    gPrevPsxMouseSensitivity = {};
    std::memset(gPrevPsxCtrlBindings, 0, sizeof(gPrevPsxCtrlBindings));
    gPrevGameSettings = {};
}

END_NAMESPACE(DemoPlayer)
