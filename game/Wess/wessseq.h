#pragma once

#include "PsxVm/VmPtr.h"

struct master_status_structure;
struct NoteState;
struct track_status;

// Pan mode
enum PanMode : uint8_t {
    PAN_OFF,        // No pan
    PAN_ON,         // Pan is on
    PAN_ON_REV      // Pan is on (reverse channels)
};

extern const VmPtr<uint8_t>             gWess_master_sfx_volume;
extern const VmPtr<uint8_t>             gWess_master_mus_volume;
extern const VmPtr<PanMode>             gWess_pan_status;
extern const VmPtr<VmPtr<NoteState>>    gpWess_pnotestate;

// Sequencer command functions.
//
// FIXME: Change to:
//  extern void (* const gWess_DrvFunctions[36])(track_status&)
//
extern void (* const gWess_DrvFunctions[36])();

uint8_t* Read_Vlq(uint8_t* const pTrackBytes, uint32_t& deltaTimeOut) noexcept;
void Write_Vlq() noexcept;
void Len_Vlq() noexcept;
void Eng_DriverInit(master_status_structure& mstat) noexcept;
void Eng_DriverExit() noexcept;
void Eng_DriverEntry1() noexcept;
void Eng_DriverEntry2() noexcept;
void Eng_DriverEntry3() noexcept;
void Eng_TrkOff() noexcept;
void Eng_TrkMute() noexcept;
void Eng_PatchChg() noexcept;
void Eng_PatchMod() noexcept;
void Eng_PitchMod() noexcept;
void Eng_ZeroMod() noexcept;
void Eng_ModuMod() noexcept;
void Eng_VolumeMod() noexcept;
void Eng_PanMod() noexcept;
void Eng_PedalMod() noexcept;
void Eng_ReverbMod() noexcept;
void Eng_ChorusMod() noexcept;
void Eng_NoteOn() noexcept;
void Eng_NoteOff() noexcept;
void Eng_StatusMark() noexcept;
void Eng_GateJump() noexcept;
void Eng_IterJump() noexcept;
void Eng_ResetGates() noexcept;
void Eng_ResetIters() noexcept;
void Eng_WriteIterBox() noexcept;
void Eng_SeqTempo() noexcept;
void Eng_SeqGosub() noexcept;
void Eng_SeqJump() noexcept;
void Eng_SeqRet() noexcept;
void Eng_SeqEnd() noexcept;
void Eng_TrkTempo() noexcept;
void Eng_TrkGosub() noexcept;
void Eng_TrkJump() noexcept;
void Eng_TrkRet() noexcept;
void Eng_TrkEnd() noexcept;
void Eng_NullEvent() noexcept;
void SeqEngine() noexcept;
