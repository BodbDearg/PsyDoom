#pragma once

#include "PsxVm/VmPtr.h"

enum SpuReverbMode : uint32_t;

// Maximum volume levels
static constexpr int16_t MAX_MASTER_VOL = 0x3FFF;
static constexpr int16_t MAX_CD_VOL     = 0x3CFF;

// This is the first usable address in SPU RAM for the application.
// Memory before this is reserved by the PlayStation for things like mixing CD Audio, audio input buffers and so on.
static constexpr uint32_t SPU_RAM_APP_BASE = 0x1010;

extern const VmPtr<uint32_t> gPsxSpu_sram_end;

void psxspu_init_reverb(
    const SpuReverbMode reverbMode,
    const int16_t depthLeft,
    const int16_t depthRight,
    const int32_t delay,
    const int32_t feedback
) noexcept;

void psxspu_set_reverb_depth(const int16_t depthLeft, const int16_t depthRight) noexcept;
void psxspu_init() noexcept;

void psxspu_setcdmixon() noexcept;
void psxspu_setcdmixoff() noexcept;

void psxspu_fadeengine() noexcept;

void psxspu_set_cd_vol(const int32_t vol) noexcept;
int32_t psxspu_get_cd_vol() noexcept;
void psxspu_start_cd_fade(const int32_t fadeTimeMs, const int32_t destVol) noexcept;
void psxspu_stop_cd_fade() noexcept;
bool psxspu_get_cd_fade_status() noexcept;

void psxspu_set_master_vol(const int32_t vol) noexcept;
int32_t psxspu_get_master_vol() noexcept;
void psxspu_start_master_fade() noexcept;
void psxspu_stop_master_fade() noexcept;
bool psxspu_get_master_fade_status() noexcept;
