#pragma once

#include "wessarc.h"

struct SavedVoiceList;

// PlayStation sound driver command functions
extern const WessDriverFunc gWess_drv_cmds[19];

void start_record_music_mute(SavedVoiceList* const pVoices) noexcept;
void end_record_music_mute() noexcept;

void add_music_mute_note(
    const int16_t seqIdx,
    const int16_t trackStatIdx,
    const uint8_t note,
    const uint8_t volume,
    const patch_voice& patchVoice,
    const patch_sample& patchSample
) noexcept;

void wess_set_mute_release(const uint32_t newReleaseTimeMs) noexcept;
void TriggerPSXVoice(const voice_status& voiceStat, const uint8_t voiceNote, const uint8_t voiceVol) noexcept;
void PSX_DriverInit(master_status_structure& mstat) noexcept;
void PSX_DriverExit(master_status_structure& mstat) noexcept;
void PSX_DriverEntry1() noexcept;
void PSX_DriverEntry2() noexcept;
void PSX_DriverEntry3() noexcept;
void PSX_TrkOff(track_status& trackStat) noexcept;
void PSX_TrkMute(track_status& trackStat) noexcept;
void PSX_PatchChg(track_status& trackStat) noexcept;
void PSX_PatchMod(track_status& trackStat) noexcept;
void PSX_PitchMod(track_status& trackStat) noexcept;
void PSX_ZeroMod(track_status& trackStat) noexcept;
void PSX_ModuMod(track_status& trackStat) noexcept;
void PSX_VolumeMod(track_status& trackStat) noexcept;
void PSX_PanMod(track_status& trackStat) noexcept;
void PSX_PedalMod(track_status& trackStat) noexcept;
void PSX_ReverbMod(track_status& trackStat) noexcept;
void PSX_ChorusMod(track_status& trackStat) noexcept;

void PSX_voiceon(
    voice_status& voiceStat,
    track_status& trackStat,
    const patch_voice& patchVoice,
    const patch_sample& patchSample,
    const uint8_t voiceNote,
    const uint8_t voiceVol
) noexcept;

void PSX_voiceparmoff(voice_status& voiceStat) noexcept;
void PSX_voicerelease(voice_status& voiceStat) noexcept;

void PSX_voicenote(
    track_status& trackStat,
    const patch_voice& patchVoice,
    const patch_sample& patchSample,
    const uint8_t voiceNote,
    const uint8_t voiceVol
) noexcept;

void PSX_NoteOn(track_status& trackStat) noexcept;
void PSX_NoteOff(track_status& trackStat) noexcept;

#if PSYDOOM_MODS
    void wess_init_channels_reverb_amt(const uint8_t allChannelsAmt) noexcept;
#endif
