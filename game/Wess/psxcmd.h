#pragma once

#include <cstdint>

struct master_status_structure;
struct NoteState;
struct track_status;
struct voice_status;

// PlayStation sound driver command functions.
//
// FIXME: Change to:
//  extern void (* const gWess_drv_cmds[19])(track_status&)
//
extern void (* const gWess_drv_cmds[19])();

void start_record_music_mute(NoteState* const pNoteState) noexcept;
void end_record_music_mute() noexcept;
void add_music_mute_note() noexcept;
void PSX_UNKNOWN_DrvFunc() noexcept;
void TriggerPSXVoice(const voice_status& voiceStat, const uint8_t voiceNote, const uint8_t voiceVol) noexcept;
void PSX_DriverInit() noexcept;
void PSX_DriverExit(master_status_structure& mstat) noexcept;
void PSX_DriverEntry1() noexcept;
void PSX_DriverEntry2(master_status_structure& mstat) noexcept;
void PSX_DriverEntry3(master_status_structure& mstat) noexcept;
void PSX_TrkOff() noexcept;
void PSX_TrkMute() noexcept;
void PSX_PatchChg() noexcept;
void PSX_PatchMod(track_status& trackStat) noexcept;
void PSX_PitchMod() noexcept;
void PSX_ZeroMod(track_status& trackStat) noexcept;
void PSX_ModuMod(track_status& trackStat) noexcept;
void PSX_VolumeMod() noexcept;
void PSX_PanMod() noexcept;
void PSX_PedalMod(track_status& trackStat) noexcept;
void PSX_ReverbMod(track_status& trackStat) noexcept;
void PSX_ChorusMod(track_status& trackStat) noexcept;
void PSX_voiceon() noexcept;
void PSX_voiceparmoff() noexcept;
void PSX_voicerelease() noexcept;
void PSX_voicenote() noexcept;
void PSX_NoteOn() noexcept;
void PSX_NoteOff() noexcept;
