//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities for reading various versions of 'GameSettings' and migrating them to newer versions
//------------------------------------------------------------------------------------------------------------------------------------------
#include "GameSettings.h"

#include "Asserts.h"
#include "Endian.h"
#include "Game.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <type_traits>

//------------------------------------------------------------------------------------------------------------------------------------------
// Game version:            1.1.0+
// Demo file version:       14
// Net protocol version:    31
//------------------------------------------------------------------------------------------------------------------------------------------
typedef GameSettings GameSettingsV2;

//------------------------------------------------------------------------------------------------------------------------------------------
// Game version:            1.0.x
// Demo file version:       11
// Net protocol version:    28
//------------------------------------------------------------------------------------------------------------------------------------------
struct GameSettingsV1 {
    static constexpr uint32_t VERSION = 1;

    uint8_t     bUsePalTimings;
    uint8_t     bUseDemoTimings;
    uint8_t     bFixKillCount;
    uint8_t     bFixLineActivation;
    uint8_t     bUseExtendedPlayerShootRange;
    uint8_t     bFixMultiLineSpecialCrossing;
    uint8_t     bUsePlayerRocketBlastFix;
    uint8_t     bUseSuperShotgunDelayTweak;
    uint8_t     bUseMoveInputLatencyTweak;
    uint8_t     bUseItemPickupFix;
    uint8_t     bUseFinalDoomPlayerMovement;
    uint8_t     bAllowMovementCancellation;
    uint8_t     bAllowTurningCancellation;
    uint8_t     bFixViewBobStrength;
    uint8_t     bFixGravityStrength;
    uint8_t     bNoMonsters;
    uint8_t     bPistolStart;
    uint8_t     bTurboMode;
    uint8_t     bUseLostSoulSpawnFix;
    uint8_t     bUseLineOfSightOverflowFix;
    uint8_t     bRemoveMaxCrossLinesLimit;
    uint8_t     bFixOutdoorBulletPuffs;
    uint8_t     bFixBlockingGibsBug;
    uint8_t     bFixSoundPropagation;
    uint8_t     bFixSpriteVerticalWarp;
    uint8_t     bAllowMultiMapPickup;
    uint8_t     bEnableMapPatches_GamePlay;
    int32_t     lostSoulSpawnLimit;
    int32_t     viewBobbingStrengthFixed;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Byte swaps all fields in the specified 'GameSettings' structure
//------------------------------------------------------------------------------------------------------------------------------------------
template <class GameSettingsT>
static void byteSwapGameSettings(GameSettingsT& settings) noexcept {
    Endian::byteSwapInPlace(settings.bUsePalTimings);
    Endian::byteSwapInPlace(settings.bUseDemoTimings);
    Endian::byteSwapInPlace(settings.bFixKillCount);
    Endian::byteSwapInPlace(settings.bFixLineActivation);
    Endian::byteSwapInPlace(settings.bUseExtendedPlayerShootRange);
    Endian::byteSwapInPlace(settings.bFixMultiLineSpecialCrossing);
    Endian::byteSwapInPlace(settings.bUsePlayerRocketBlastFix);
    Endian::byteSwapInPlace(settings.bUseSuperShotgunDelayTweak);
    Endian::byteSwapInPlace(settings.bUseMoveInputLatencyTweak);
    Endian::byteSwapInPlace(settings.bUseItemPickupFix);
    Endian::byteSwapInPlace(settings.bUseFinalDoomPlayerMovement);
    Endian::byteSwapInPlace(settings.bAllowMovementCancellation);
    Endian::byteSwapInPlace(settings.bAllowTurningCancellation);
    Endian::byteSwapInPlace(settings.bFixViewBobStrength);
    Endian::byteSwapInPlace(settings.bFixGravityStrength);
    Endian::byteSwapInPlace(settings.bNoMonsters);
    Endian::byteSwapInPlace(settings.bPistolStart);
    Endian::byteSwapInPlace(settings.bTurboMode);
    Endian::byteSwapInPlace(settings.bUseLostSoulSpawnFix);
    Endian::byteSwapInPlace(settings.bUseLineOfSightOverflowFix);
    Endian::byteSwapInPlace(settings.bRemoveMaxCrossLinesLimit);
    Endian::byteSwapInPlace(settings.bFixOutdoorBulletPuffs);
    Endian::byteSwapInPlace(settings.bFixBlockingGibsBug);
    Endian::byteSwapInPlace(settings.bFixSoundPropagation);
    Endian::byteSwapInPlace(settings.bFixSpriteVerticalWarp);
    Endian::byteSwapInPlace(settings.bAllowMultiMapPickup);
    Endian::byteSwapInPlace(settings.bEnableMapPatches_GamePlay);
    Endian::byteSwapInPlace(settings.lostSoulSpawnLimit);
    Endian::byteSwapInPlace(settings.viewBobbingStrengthFixed);

    if constexpr (GameSettingsT::VERSION >= 2) {
        Endian::byteSwapInPlace(settings.bNoMonstersBossFixup);
        Endian::byteSwapInPlace(settings.bCoopNoFriendlyFire);
        Endian::byteSwapInPlace(settings.bCoopForceSpawnDeathmatchThings);
        Endian::byteSwapInPlace(settings.bDmExitDisabled);
        Endian::byteSwapInPlace(settings.bCoopPreserveKeys);
        Endian::byteSwapInPlace(settings.bDmActivateBossSpecialSectors);
        Endian::byteSwapInPlace(settings.dmFragLimit);
        Endian::byteSwapInPlace(settings.coopPreserveAmmoFactor);
        Endian::byteSwapInPlace(settings.bSinglePlayerForceSpawnDmThings);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Migrates an old (or current) version of 'GameSettings' to the current version of 'GameSettings'
//------------------------------------------------------------------------------------------------------------------------------------------
template <class OldGameSettingsT>
static void migrateGameSettings(OldGameSettingsT& oldSettings, GameSettings& newSettings) noexcept {
    // Start by initializing the output settings struct to emulate the original PSX Doom behavior.
    // If there is a field missing from 'oldSettings' the value it will get in 'newSettings' will emulate the original PSX Doom behavior.
    Game::getClassicDemoGameSettings(newSettings);

    // Do the assignments
    #define COPY_GAME_SETTINGS_FIELD(FIELD_NAME)\
        newSettings.FIELD_NAME = oldSettings.FIELD_NAME;

    COPY_GAME_SETTINGS_FIELD(bUsePalTimings);
    COPY_GAME_SETTINGS_FIELD(bUseDemoTimings);
    COPY_GAME_SETTINGS_FIELD(bFixKillCount);
    COPY_GAME_SETTINGS_FIELD(bFixLineActivation);
    COPY_GAME_SETTINGS_FIELD(bUseExtendedPlayerShootRange);
    COPY_GAME_SETTINGS_FIELD(bFixMultiLineSpecialCrossing);
    COPY_GAME_SETTINGS_FIELD(bUsePlayerRocketBlastFix);
    COPY_GAME_SETTINGS_FIELD(bUseSuperShotgunDelayTweak);
    COPY_GAME_SETTINGS_FIELD(bUseMoveInputLatencyTweak);
    COPY_GAME_SETTINGS_FIELD(bUseItemPickupFix);
    COPY_GAME_SETTINGS_FIELD(bUseFinalDoomPlayerMovement);
    COPY_GAME_SETTINGS_FIELD(bAllowMovementCancellation);
    COPY_GAME_SETTINGS_FIELD(bAllowTurningCancellation);
    COPY_GAME_SETTINGS_FIELD(bFixViewBobStrength);
    COPY_GAME_SETTINGS_FIELD(bFixGravityStrength);
    COPY_GAME_SETTINGS_FIELD(bNoMonsters);
    COPY_GAME_SETTINGS_FIELD(bPistolStart);
    COPY_GAME_SETTINGS_FIELD(bTurboMode);
    COPY_GAME_SETTINGS_FIELD(bUseLostSoulSpawnFix);
    COPY_GAME_SETTINGS_FIELD(bUseLineOfSightOverflowFix);
    COPY_GAME_SETTINGS_FIELD(bRemoveMaxCrossLinesLimit);
    COPY_GAME_SETTINGS_FIELD(bFixOutdoorBulletPuffs);
    COPY_GAME_SETTINGS_FIELD(bFixBlockingGibsBug);
    COPY_GAME_SETTINGS_FIELD(bFixSoundPropagation);
    COPY_GAME_SETTINGS_FIELD(bFixSpriteVerticalWarp);
    COPY_GAME_SETTINGS_FIELD(bAllowMultiMapPickup);
    COPY_GAME_SETTINGS_FIELD(bEnableMapPatches_GamePlay);
    COPY_GAME_SETTINGS_FIELD(lostSoulSpawnLimit);
    COPY_GAME_SETTINGS_FIELD(viewBobbingStrengthFixed);

    if constexpr (OldGameSettingsT::VERSION >= 2) {
        COPY_GAME_SETTINGS_FIELD(bNoMonstersBossFixup);
        COPY_GAME_SETTINGS_FIELD(bCoopNoFriendlyFire);
        COPY_GAME_SETTINGS_FIELD(bCoopForceSpawnDeathmatchThings);
        COPY_GAME_SETTINGS_FIELD(bDmExitDisabled);
        COPY_GAME_SETTINGS_FIELD(bCoopPreserveKeys);
        COPY_GAME_SETTINGS_FIELD(bDmActivateBossSpecialSectors);
        COPY_GAME_SETTINGS_FIELD(dmFragLimit);
        COPY_GAME_SETTINGS_FIELD(coopPreserveAmmoFactor);
        COPY_GAME_SETTINGS_FIELD(bSinglePlayerForceSpawnDmThings);
    }

    #undef COPY_GAME_SETTINGS_FIELD
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bytes swaps the 'GameSettings' struct from little to big endian or big endian to little if the host architecture is big-endian
//------------------------------------------------------------------------------------------------------------------------------------------
template <class GameSettingsT>
static void endianCorrectGameSettings(GameSettingsT& settings) noexcept {
    if constexpr (Endian::isBig()) {
        byteSwapGameSettings(settings);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a 'GameSettings' struct of the specified type from the given buffer.
// After reading, the 'GameSettings' struct is endian corrected and migrated to the current version.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ReadGameSettingsT>
static void readAndMigrateGameSettingsImpl(const void* const pSrcBuffer, GameSettings& dstSettings) noexcept {
    if constexpr (std::is_same_v<ReadGameSettingsT, GameSettings>) {
        // Easy case: the 'GameSettings' struct version we are reading is the same as the current version
        std::memcpy(&dstSettings, pSrcBuffer, sizeof(GameSettings));
        endianCorrectGameSettings(dstSettings);
    }
    else {
        // Harder case: read the old 'GameSettings' version, endian correct and then migrate it into the new 'GameSettings' version
        ReadGameSettingsT oldSettings;
        std::memcpy(&oldSettings, pSrcBuffer, sizeof(ReadGameSettingsT));
        endianCorrectGameSettings(oldSettings);
        migrateGameSettings(oldSettings, dstSettings);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Externally accessible helper functions for the current 'GameSettings'
//------------------------------------------------------------------------------------------------------------------------------------------
void GameSettings::byteSwap() noexcept {
    byteSwapGameSettings(*this);
}

void GameSettings::endianCorrect() noexcept {
    endianCorrectGameSettings(*this);
}

BEGIN_NAMESPACE(GameSettingUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns which 'GameSettings' struct version can be used for the specified demo file version.
// Returns '-1' if there is no suitable mapping.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getGameSettingsVersionForDemoFileVersion(const int32_t demoFileVersion) noexcept {
    switch (demoFileVersion) {
        case 11:    return 1;
        case 14:    return 2;
    }

    ASSERT_FAIL_F("No 'GameSettings' mapping for demo file version '%d'! Support might need to be added here...", demoFileVersion);
    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the size in bytes of the specified version of the 'GameSettings' struct.
// Returns '-1' if the version is unrecognized.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getGameSettingsSize(const int32_t gameSettingsVersion) noexcept {
    switch (gameSettingsVersion) {
        case 1:     return sizeof(GameSettingsV1);
        case 2:     return sizeof(GameSettingsV2);
    }

    ASSERT_FAIL_F("Invalid 'GameSettings' version '%d'!", gameSettingsVersion);
    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to read a 'GameSettings' struct of the specified version from the given buffer.
// After reading, the 'GameSettings' struct is endian corrected and migrated to the current version.
// 
// Notes:
//  (1) The buffer is assumed to be big enough for the game settings version.
//  (1) Returns 'false' on failure, 'true' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
bool readAndMigrateGameSettings(const int32_t gameSettingsVersion, const void* const pSrcBuffer, GameSettings& dstSettings) noexcept {
    switch (gameSettingsVersion) {
        case 1:     readAndMigrateGameSettingsImpl<GameSettingsV1>(pSrcBuffer, dstSettings);    break;
        case 2:     readAndMigrateGameSettingsImpl<GameSettingsV2>(pSrcBuffer, dstSettings);    break;

        default:
            ASSERT_FAIL_F("Invalid 'GameSettings' version '%d'!", gameSettingsVersion);
            return false;
    }

    // If we get to here then the operation was a success
    return true;
}

END_NAMESPACE(GameSettingUtils)
