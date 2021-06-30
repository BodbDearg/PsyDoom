#pragma once

#include "Macros.h"

#include <cstdint>

// Stat display modes
enum class StatDisplayMode : int32_t {
    None = 0,
    Kills = 1,
    KillsAndSecrets = 2,
    KillsSecretsAndItems = 3
};

BEGIN_NAMESPACE(PlayerPrefs)

// Length of PSX Doom passwords
constexpr int32_t PASSWORD_LEN = 10;

// Minimum and maximum values for sound and music volume
constexpr int32_t VOLUME_MIN = 0;
constexpr int32_t VOLUME_MAX = 100;

// Minimum and maximum values for the turn speed multiplier (expressed in whole percentages, 0-500%)
constexpr int32_t TURN_SPEED_MULT_MIN = 1;
constexpr int32_t TURN_SPEED_MULT_MAX = 500;

extern int32_t          gTurnSpeedMult100;
extern bool             gbAlwaysRun;
extern StatDisplayMode  gStatDisplayMode;

void setToDefaults() noexcept;
void load() noexcept;
void save() noexcept;
float getTurnSpeedMultiplier() noexcept;
void pushSoundAndMusicPrefs() noexcept;
void pullSoundAndMusicPrefs() noexcept;
void pushLastPassword() noexcept;
void pullLastPassword() noexcept;
bool shouldStartupWithVulkanRenderer() noexcept;

END_NAMESPACE(PlayerPrefs)
