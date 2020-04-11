#pragma once

#include "wessarc.h"

struct master_status_structure;
struct SavedVoiceList;
struct track_status;

// Pan mode
enum PanMode : uint8_t {
    PAN_OFF,        // No pan
    PAN_ON,         // Pan is on
    PAN_ON_REV      // Pan is on (reverse channels)
};

extern const VmPtr<uint8_t>                 gWess_master_sfx_volume;
extern const VmPtr<uint8_t>                 gWess_master_mus_volume;
extern const VmPtr<PanMode>                 gWess_pan_status;
extern const VmPtr<VmPtr<SavedVoiceList>>   gpWess_savedVoices;

// Sequencer command functions
extern const WessDriverFunc gWess_DrvFunctions[36];

uint8_t* Read_Vlq(uint8_t* const pTrackBytes, uint32_t& deltaTimeOut) noexcept;
uint8_t* Write_Vlq(uint8_t* const pTrackBytes, const uint32_t deltaTime) noexcept;
int32_t Len_Vlq(const uint32_t deltaTime) noexcept;
void Eng_DriverInit(master_status_structure& mstat) noexcept;
void Eng_DriverExit(master_status_structure& mstat) noexcept;
void Eng_DriverEntry1() noexcept;
void Eng_DriverEntry2() noexcept;
void Eng_DriverEntry3() noexcept;
void Eng_TrkOff(track_status& trackStat) noexcept;
void Eng_TrkMute(track_status& trackStat) noexcept;
void Eng_PatchChg(track_status& trackStat) noexcept;
void Eng_PatchMod(track_status& trackStat) noexcept;
void Eng_PitchMod(track_status& trackStat) noexcept;
void Eng_ZeroMod(track_status& trackStat) noexcept;
void Eng_ModuMod(track_status& trackStat) noexcept;
void Eng_VolumeMod(track_status& trackStat) noexcept;
void Eng_PanMod(track_status& trackStat) noexcept;
void Eng_PedalMod(track_status& trackStat) noexcept;
void Eng_ReverbMod(track_status& trackStat) noexcept;
void Eng_ChorusMod(track_status& trackStat) noexcept;
void Eng_NoteOn(track_status& trackStat) noexcept;
void Eng_NoteOff(track_status& trackStat) noexcept;
void Eng_StatusMark(track_status& trackStat) noexcept;
void Eng_GateJump(track_status& trackStat) noexcept;
void Eng_IterJump(track_status& trackStat) noexcept;
void Eng_ResetGates(track_status& trackStat) noexcept;
void Eng_ResetIters(track_status& trackStat) noexcept;
void Eng_WriteIterBox(track_status& trackStat) noexcept;
void Eng_SeqTempo(track_status& trackStat) noexcept;
void Eng_SeqGosub(track_status& trackStat) noexcept;
void Eng_SeqJump(track_status& trackStat) noexcept;
void Eng_SeqRet(track_status& trackStat) noexcept;
void Eng_SeqEnd(track_status& trackStat) noexcept;
void Eng_TrkTempo(track_status& trackStat) noexcept;
void Eng_TrkGosub(track_status& trackStat) noexcept;
void Eng_TrkJump(track_status& trackStat) noexcept;
void Eng_TrkRet(track_status& trackStat) noexcept;
void Eng_TrkEnd(track_status& trackStat) noexcept;
void Eng_NullEvent(track_status& trackStat) noexcept;
void SeqEngine() noexcept;
