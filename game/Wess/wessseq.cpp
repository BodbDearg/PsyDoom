//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): core sequencer driver commands and sequencer functions.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessseq.h"

#include "PsxVm/PsxVm.h"
#include "wessarc.h"

// TODO: REMOVE ALL OF THESE
void _thunk_Eng_DriverInit() noexcept { Eng_DriverInit(*vmAddrToPtr<master_status_structure>(a0)); }
void _thunk_Eng_DriverExit() noexcept { Eng_DriverExit(*vmAddrToPtr<master_status_structure>(a0)); }
void _thunk_Eng_TrkOff() noexcept { Eng_TrkOff(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_TrkMute() noexcept { Eng_TrkMute(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_PatchChg() noexcept { Eng_PatchChg(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_PatchMod() noexcept { Eng_PatchMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_PitchMod() noexcept { Eng_PitchMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_ZeroMod() noexcept { Eng_ZeroMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_ModuMod() noexcept { Eng_ModuMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_VolumeMod() noexcept { Eng_VolumeMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_PanMod() noexcept { Eng_PanMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_PedalMod() noexcept { Eng_PedalMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_ReverbMod() noexcept { Eng_ReverbMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_ChorusMod() noexcept { Eng_ChorusMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_NoteOn() noexcept { Eng_NoteOn(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_NoteOff() noexcept { Eng_NoteOff(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_WriteIterBox() noexcept { Eng_WriteIterBox(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_TrkJump() noexcept { Eng_TrkJump(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_TrkEnd() noexcept { Eng_TrkEnd(*vmAddrToPtr<track_status>(a0)); }
void _thunk_Eng_NullEvent() noexcept { Eng_NullEvent(*vmAddrToPtr<track_status>(a0)); }

void (* const gWess_DrvFunctions[36])() = {
    // Manually called commands
    _thunk_Eng_DriverInit,          // 00
    _thunk_Eng_DriverExit,          // 01
    Eng_DriverEntry1,               // 02
    Eng_DriverEntry2,               // 03
    Eng_DriverEntry3,               // 04
    _thunk_Eng_TrkOff,              // 05
    _thunk_Eng_TrkMute,             // 06
    // Hardware driver commands
    _thunk_Eng_PatchChg,            // 07
    _thunk_Eng_PatchMod,            // 08
    _thunk_Eng_PitchMod,            // 09
    _thunk_Eng_ZeroMod,             // 10
    _thunk_Eng_ModuMod,             // 11
    _thunk_Eng_VolumeMod,           // 12
    _thunk_Eng_PanMod,              // 13
    _thunk_Eng_PedalMod,            // 14
    _thunk_Eng_ReverbMod,           // 15
    _thunk_Eng_ChorusMod,           // 16
    _thunk_Eng_NoteOn,              // 17
    _thunk_Eng_NoteOff,             // 18
    // Sequencer commands
    Eng_StatusMark,                 // 19
    Eng_GateJump,                   // 20
    Eng_IterJump,                   // 21
    Eng_ResetGates,                 // 22
    Eng_ResetIters,                 // 23
    _thunk_Eng_WriteIterBox,        // 24
    Eng_SeqTempo,                   // 25
    Eng_SeqGosub,                   // 26
    Eng_SeqJump,                    // 27
    Eng_SeqRet,                     // 28
    Eng_SeqEnd,                     // 29
    Eng_TrkTempo,                   // 30
    Eng_TrkGosub,                   // 31
    _thunk_Eng_TrkJump,             // 32
    Eng_TrkRet,                     // 33
    _thunk_Eng_TrkEnd,              // 34
    _thunk_Eng_NullEvent            // 35
};

// The size in bytes of each sequencer command
static constexpr uint8_t gWess_seq_CmdLength[36] = {
    0,  // DriverInit       (This command should NEVER be in a sequence)
    0,  // DriverExit       (This command should NEVER be in a sequence)
    0,  // DriverEntry1     (This command should NEVER be in a sequence)
    0,  // DriverEntry2     (This command should NEVER be in a sequence)
    0,  // DriverEntry3     (This command should NEVER be in a sequence)
    0,  // TrkOff           (This command should NEVER be in a sequence)
    0,  // TrkMute          (This command should NEVER be in a sequence)
    3,  // PatchChg
    2,  // PatchMod
    3,  // PitchMod
    2,  // ZeroMod
    2,  // ModuMod
    2,  // VolumeMod
    2,  // PanMod
    2,  // PedalMod
    2,  // ReverbMod
    2,  // ChorusMod
    3,  // NoteOn
    2,  // NoteOff
    4,  // StatusMark
    5,  // GateJump
    5,  // IterJump
    2,  // ResetGates
    2,  // ResetIters
    3,  // WriteIterBox
    3,  // SeqTempo
    3,  // SeqGosub
    3,  // SeqJump
    1,  // SeqRet
    1,  // SeqEnd
    3,  // TrkTempo
    3,  // TrkGosub
    3,  // TrkJump
    1,  // TrkRet
    1,  // TrkEnd
    1   // NullEvent
};

static_assert(C_ARRAY_SIZE(gWess_seq_CmdLength) == C_ARRAY_SIZE(gWess_DrvFunctions));

const VmPtr<uint8_t>            gWess_master_sfx_volume(0x80075A04);    // TODO: COMMENT
const VmPtr<uint8_t>            gWess_master_mus_volume(0x80075A05);    // TODO: COMMENT
const VmPtr<PanMode>            gWess_pan_status(0x80075A06);           // Pan mode: '0' if disabled, '1' if enabled, '2' if enabled (reverse)
const VmPtr<VmPtr<NoteState>>   gpWess_notestate(0x80075A10);           // TODO: COMMENT

static const VmPtr<VmPtr<master_status_structure>>      gpWess_eng_mstat(0x80075AC0);           // TODO: COMMENT
static const VmPtr<VmPtr<sequence_status>>              gpWess_eng_seqStats(0x80075ABC);        // TODO: COMMENT
static const VmPtr<VmPtr<track_status>>                 gpWess_eng_trackStats(0x80075AB8);      // TODO: COMMENT
static const VmPtr<uint8_t>                             gWess_eng_maxTracks(0x80075AB4);        // TODO: COMMENT

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads from track data the delta time (relative to the previous sequencer command) for when the next sequencer command is to be executed.
// Returns the pointer after reading the time amount, which may be incremented an arbitrary amount.
//
// The delta time amount returned is in terms of quarter note 'parts'.
// How many parts/divisions per quarter note there are depends on the timing setup of track, but a value of '120' is typical.
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t* Read_Vlq(uint8_t* const pTrackBytes, uint32_t& deltaTimeOut) noexcept {
    uint8_t* pCurTrackByte = pTrackBytes;
    uint32_t decodedTimeVal = *pCurTrackByte;
    ++pCurTrackByte;

    // The top bit set on each byte means there is another byte to follow.
    // Each byte can therefore only encode 7 bits, so we need 5 of them to encode 32-bits:
    if (pTrackBytes[0] & 0x80) {
        decodedTimeVal &= 0x7F;   // Top bit is not data
        uint8_t curByte;

        do {
            curByte = *pCurTrackByte;
            decodedTimeVal = (decodedTimeVal << 7) + (curByte & 0x7F);  // Incorporate another 7 bits to the output time value
            ++pCurTrackByte;
        } while (curByte & 0x80);   // Is there another byte to follow?
    }

    deltaTimeOut = decodedTimeVal;
    return pCurTrackByte;
}

void Write_Vlq() noexcept {
    sp -= 0x10;
    v0 = a1 & 0x7F;
    sb(v0, sp);
    a1 >>= 7;
    v1 = sp + 1;
    if (a1 == 0) goto loc_80047708;
loc_800476F4:
    v0 = a1 | 0x80;
    sb(v0, v1);
    a1 >>= 7;
    v1++;
    if (a1 != 0) goto loc_800476F4;
loc_80047708:
    v1--;
    v0 = lbu(v1);
    sb(v0, a0);
    v0 = lbu(v1);
    v0 &= 0x80;
    a0++;
    if (v0 != 0) goto loc_80047708;
    v0 = a0;
    sp += 0x10;
    return;
}

void Len_Vlq() noexcept {
    sp -= 0x20;
    a1 = sp + 0x10;
    v0 = a0 & 0x7F;
    sb(v0, sp);
    a0 >>= 7;
    v1 = sp + 1;
    if (a0 == 0) goto loc_8004776C;
loc_80047758:
    v0 = a0 | 0x80;
    sb(v0, v1);
    a0 >>= 7;
    v1++;
    if (a0 != 0) goto loc_80047758;
loc_8004776C:
    v1--;
    v0 = lbu(v1);
    sb(v0, a1);
    v0 = lbu(v1);
    v0 &= 0x80;
    a1++;
    if (v0 != 0) goto loc_8004776C;
    v0 = sp + 0x10;
    v0 = a1 - v0;
    v0 &= 0xFF;
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do initialization for the sequencer engine
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_DriverInit(master_status_structure& mstat) noexcept {
    *gpWess_eng_mstat = &mstat;
    *gpWess_eng_seqStats = mstat.pseqstattbl;
    *gpWess_eng_trackStats = mstat.ptrkstattbl;
    *gWess_eng_maxTracks = mstat.pmod_info->mod_hdr.trk_work_areas;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer shutdown: does nothing
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_DriverExit([[maybe_unused]] master_status_structure& mstat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer tick/update function: does nothing.
// Sometimes called when there is nothing else playing.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_DriverEntry1() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer tick/update function: never called
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_DriverEntry2() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer tick/update function: never called
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_DriverEntry3() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Mark the given track as not playing and update the track and parent sequence accordingly
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_TrkOff(track_status& trackStat) noexcept {
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    sequence_status& seqStat = mstat.pseqstattbl[trackStat.seq_owner];

    // Mark the track as not playing anymore
    if (!trackStat.stopped) {
        trackStat.stopped = true;
        seqStat.tracks_playing--;

        if (seqStat.tracks_playing == 0) {
            seqStat.playmode = SEQ_STATE_STOPPED;
        }
    }

    // Mark the track as inactive
    // TODO: what is the 'handled' flag here doing?
    if (!trackStat.handled) {
        // Clear the parent sequence's index slot for the track being disabled.
        // This marks the track is no longer active in the parent sequence:
        {
            const uint32_t maxSeqTracks = mstat.max_trks_perseq;
            uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

            for (uint32_t i = 0; i < maxSeqTracks; ++i) {
                // Is this the track being turned off? If so then mark it as inactive and stop search:
                if (pSeqTrackIndexes[i] == trackStat.refindx) {
                    pSeqTrackIndexes[i] = 0xFF;
                    break;
                }
            }
        }

        // Mark the track as inactive and update all stat counts
        trackStat.active = false;
        mstat.trks_active--;
        seqStat.tracks_active--;
        
        // If the sequence has no more tracks active then it too is now inactive
        if (seqStat.tracks_active == 0) {
            seqStat.active = false;
            mstat.seqs_active--;
        }
    }
    
    // TODO: What is this flag for?
    trackStat.timed = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_TrkMute([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_PatchChg([[maybe_unused]] track_status& trackStat) noexcept {
    // Note: this code originally updated an unreferenced/unused global with the new patch/sound number.
    // I've omitted that here (since it's not needed) to reduce the number of globals.
    trackStat.patchnum = trackStat.ppos[1];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_PatchMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_PitchMod([[maybe_unused]] track_status& trackStat) noexcept {
    // Note: this code originally updated an unreferenced/unused global with the new pitch modifier.
    // I've omitted that here (since it's not needed) to reduce the number of globals.
    const int16_t pitchMod = ((int16_t) trackStat.ppos[1]) | ((int16_t) trackStat.ppos[2] << 8);
    trackStat.pitch_cntrl = pitchMod;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_ZeroMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_ModuMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_VolumeMod([[maybe_unused]] track_status& trackStat) noexcept {
    // Note: this code originally updated an unreferenced/unused global with the new volume amount.
    // I've omitted that here (since it's not needed) to reduce the number of globals.
    trackStat.volume_cntrl = trackStat.ppos[1];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_PanMod([[maybe_unused]] track_status& trackStat) noexcept {
    // Note: this code originally updated an unreferenced/unused global with the new pan amount.
    // I've omitted that here (since it's not needed) to reduce the number of globals.
    trackStat.pan_cntrl = trackStat.ppos[1];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_PedalMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_ReverbMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_ChorusMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_NoteOn([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should never be called - implemented by the hardware sound driver instead. See the equivalent function in 'psxcmd.cpp' for more details.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_NoteOff([[maybe_unused]] track_status& trackStat) noexcept {}

void Eng_StatusMark() noexcept {
loc_80047A88:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lbu(v1 + 0xA);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDD8);                                 // Store to: 8007F228
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xDD8);                               // Load from: 8007F228
    if (v0 == 0) goto loc_80047B94;
    v0 = lw(v1 + 0x10);
    v1 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDD4);                                 // Store to: 8007F22C
    v1 = lbu(v1 + 0xF);
    v0 = v1 + 0xFF;
    goto loc_80047B84;
loc_80047AD8:
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0xDD4);                                // Load from: 8007F22C
    v0 = lbu(a2);
    if (v0 == 0) goto loc_80047B64;
    a1 = lw(a0 + 0x34);
    v1 = lbu(a2 + 0x1);
    v0 = lbu(a1 + 0x1);
    if (v1 != v0) goto loc_80047B40;
    v0 = lbu(a1 + 0x3);
    a1 = lbu(a1 + 0x2);
    a0 = lbu(a2 + 0x1);
    v0 <<= 8;
    a1 |= v0;
    sh(a1, a2 + 0x2);
    a1 <<= 16;
    v0 = lw(a2 + 0x4);
    a1 = u32(i32(a1) >> 16);
    ptr_call(v0);
    goto loc_80047B94;
loc_80047B40:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xDD8);                               // Load from: 8007F228
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDD8);                                 // Store to: 8007F228
    v0 &= 0xFF;
    if (v0 == 0) goto loc_80047B94;
loc_80047B64:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xDD4);                                // Load from: 8007F22C
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xDDC);                               // Load from: 8007F224
    v0 += 8;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDD4);                                 // Store to: 8007F22C
    v0 = v1 + 0xFF;
loc_80047B84:
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDDC);                                 // Store to: 8007F224
    if (v1 != 0) goto loc_80047AD8;
loc_80047B94:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Eng_GateJump() noexcept {
loc_80047BA4:
    a1 = a0;
    v1 = lbu(a1 + 0x2);
    a0 = lw(a1 + 0x34);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lbu(a0 + 0x1);
    v0 = lw(v0 + 0x10);
    v1 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDC8);                                 // Store to: 8007F238
    a0 = lbu(v1);
    v0 = 0xFF;                                          // Result = 000000FF
    if (a0 == 0) goto loc_80047C7C;
    if (a0 != v0) goto loc_80047C0C;
    v0 = lw(a1 + 0x34);
    v0 = lbu(v0 + 0x2);
    sb(v0, v1);
loc_80047C0C:
    v0 = lw(a1 + 0x34);
    v1 = lbu(v0 + 0x4);
    v0 = lbu(v0 + 0x3);
    v1 <<= 8;
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDD0);                                 // Store to: 8007F230
    v0 <<= 16;
    a0 = u32(i32(v0) >> 16);
    if (i32(a0) < 0) goto loc_80047C7C;
    v0 = lh(a1 + 0x18);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = a0 << 2;
        if (bJump) goto loc_80047C7C;
    }
    v1 = lw(a1 + 0x38);
    v0 += v1;
    v0 = lw(v0);
    v1 = lw(a1 + 0x30);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDCC);                                 // Store to: 8007F234
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDCC);                                 // Store to: 8007F234
    sw(v0, a1 + 0x34);
loc_80047C7C:
    v0 = lw(a1);
    v0 |= 0x40;
    sw(v0, a1);
    return;
}

void Eng_IterJump() noexcept {
loc_80047C90:
    a1 = a0;
    v1 = lbu(a1 + 0x2);
    a0 = lw(a1 + 0x34);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lbu(a0 + 0x1);
    v0 = lw(v0 + 0x14);
    v1 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDBC);                                 // Store to: 8007F244
    a0 = lbu(v1);
    v0 = 0xFF;                                          // Result = 000000FF
    if (a0 == 0) goto loc_80047D78;
    if (a0 != v0) goto loc_80047CF8;
    v0 = lw(a1 + 0x34);
    v0 = lbu(v0 + 0x2);
    sb(v0, v1);
    goto loc_80047D08;
loc_80047CF8:
    v0 = lbu(v1);
    v0--;
    sb(v0, v1);
loc_80047D08:
    v0 = lw(a1 + 0x34);
    v1 = lbu(v0 + 0x4);
    v0 = lbu(v0 + 0x3);
    v1 <<= 8;
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDC4);                                 // Store to: 8007F23C
    v0 <<= 16;
    a0 = u32(i32(v0) >> 16);
    if (i32(a0) < 0) goto loc_80047D78;
    v0 = lh(a1 + 0x18);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = a0 << 2;
        if (bJump) goto loc_80047D78;
    }
    v1 = lw(a1 + 0x38);
    v0 += v1;
    v0 = lw(v0);
    v1 = lw(a1 + 0x30);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDC0);                                 // Store to: 8007F240
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDC0);                                 // Store to: 8007F240
    sw(v0, a1 + 0x34);
loc_80047D78:
    v0 = lw(a1);
    v0 |= 0x40;
    sw(v0, a1);
    return;
}

void Eng_ResetGates() noexcept {
loc_80047D8C:
    v1 = a0;
    v0 = lw(v1 + 0x34);
    a0 = lbu(v0 + 0x1);
    v0 = 0xFF;                                          // Result = 000000FF
    if (a0 != v0) goto loc_80047E50;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 = lw(v0 + 0xC);
    a0 = lbu(v0 + 0xD);
    at = 0x80080000;                                    // Result = 80080000
    sb(a0, at - 0xDB8);                                 // Store to: 8007F248
    v1 = lbu(v1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lw(v0 + 0x10);
    v0 = a0 + 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDB8);                                 // Store to: 8007F248
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDB4);                                 // Store to: 8007F24C
    {
        const bool bJump = (a0 == 0);
        a0 = 0xFF;                                      // Result = 000000FF
        if (bJump) goto loc_80047E88;
    }
loc_80047E0C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xDB4);                                // Load from: 8007F24C
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDB4);                                 // Store to: 8007F24C
    sb(a0, v1);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xDB8);                               // Load from: 8007F248
    v0 = v1 + 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDB8);                                 // Store to: 8007F248
    if (v1 == 0) goto loc_80047E88;
    goto loc_80047E0C;
loc_80047E50:
    v1 = lbu(v1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x10);
    v1 = 0xFF;                                          // Result = 000000FF
    v0 += a0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDB4);                                 // Store to: 8007F24C
    sb(v1, v0);
loc_80047E88:
    return;
}

void Eng_ResetIters() noexcept {
loc_80047E90:
    v1 = a0;
    v0 = lw(v1 + 0x34);
    a0 = lbu(v0 + 0x1);
    v0 = 0xFF;                                          // Result = 000000FF
    if (a0 != v0) goto loc_80047F54;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 = lw(v0 + 0xC);
    a0 = lbu(v0 + 0xE);
    at = 0x80080000;                                    // Result = 80080000
    sb(a0, at - 0xDB0);                                 // Store to: 8007F250
    v1 = lbu(v1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lw(v0 + 0x14);
    v0 = a0 + 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDB0);                                 // Store to: 8007F250
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDAC);                                 // Store to: 8007F254
    {
        const bool bJump = (a0 == 0);
        a0 = 0xFF;                                      // Result = 000000FF
        if (bJump) goto loc_80047F8C;
    }
loc_80047F10:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xDAC);                                // Load from: 8007F254
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDAC);                                 // Store to: 8007F254
    sb(a0, v1);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xDB0);                               // Load from: 8007F250
    v0 = v1 + 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDB0);                                 // Store to: 8007F250
    if (v1 == 0) goto loc_80047F8C;
    goto loc_80047F10;
loc_80047F54:
    v1 = lbu(v1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x14);
    v1 = 0xFF;                                          // Result = 000000FF
    v0 += a0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDAC);                                 // Store to: 8007F254
    sb(v1, v0);
loc_80047F8C:
    return;
}

// TODO: what is this doing?
void Eng_WriteIterBox(track_status& trackStat) noexcept {
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];
    const uint8_t iterIdx = trackStat.ppos[1];
    seqStat.piters[iterIdx] = trackStat.ppos[2];
}

void Eng_SeqTempo() noexcept {
loc_80047FD8:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lbu(s0 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    a0 = lw(a0 + 0xC);
    v1 += v0;
    a1 = lh(v1 + 0x2);
    a0 = lw(a0 + 0x10);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a0;
    v0 = lhu(v0);
    a0 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD94);                                 // Store to: 8007F26C
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDA4);                                 // Store to: 8007F25C
    v1 = lbu(v1 + 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDA4);                                 // Store to: 8007F25C
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD9C);                                 // Store to: 8007F264
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xDA0);                                 // Store to: 8007F260
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == v1) goto loc_80048144;
loc_80048074:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xD9C);                                // Load from: 8007F264
    v1 = lbu(a0);
    v0 = 0xFF;                                          // Result = 000000FF
    {
        const bool bJump = (v1 == v0);
        v0 = a0 + 1;
        if (bJump) goto loc_8004811C;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD9C);                                 // Store to: 8007F264
    v0 = lbu(a0);
    v1 = lw(s0 + 0x34);
    a0 = v0 << 2;
    a0 += v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AB8);                               // Load from: 80075AB8
    a0 <<= 4;
    a0 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD98);                                 // Store to: 8007F268
    v0 = lbu(v1 + 0x2);
    v1 = lbu(v1 + 0x1);
    v0 <<= 8;
    v1 |= v0;
    sh(v1, a0 + 0x16);
    v0 = GetIntsPerSec();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD98);                                // Load from: 8007F268
    v0 <<= 16;
    a1 = lh(v1 + 0x14);
    a2 = lh(v1 + 0x16);
    a0 = u32(i32(v0) >> 16);
    v0 = CalcPartsPerInt((int16_t) a0, (int16_t) a1, (int16_t) a2);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xDA0);                               // Load from: 8007F260
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xD98);                                // Load from: 8007F268
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xDA0);                                 // Store to: 8007F260
    v1 &= 0xFF;
    sw(v0, a0 + 0x1C);
    if (v1 == 0) goto loc_80048144;
loc_8004811C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xDA4);                               // Load from: 8007F25C
    v1 = -1;                                            // Result = FFFFFFFF
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDA4);                                 // Store to: 8007F25C
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    if (v0 != v1) goto loc_80048074;
loc_80048144:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Eng_SeqGosub() noexcept {
loc_80048158:
    a2 = a0;
    v0 = lw(a2 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD90);                                 // Store to: 8007F270
    v0 <<= 16;
    v1 = u32(i32(v0) >> 16);
    if (i32(v1) < 0) goto loc_80048320;
    v0 = lh(a2 + 0x18);
    v0 = (i32(v1) < i32(v0));
    if (v0 == 0) goto loc_80048320;
    v0 = lbu(a2 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    a0 = lw(a0 + 0xC);
    v1 += v0;
    a1 = lh(v1 + 0x2);
    a0 = lw(a0 + 0x10);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a0;
    v0 = lhu(v0);
    a0 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD78);                                 // Store to: 8007F288
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD8C);                                 // Store to: 8007F274
    v1 = lbu(v1 + 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD8C);                                 // Store to: 8007F274
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD80);                                 // Store to: 8007F280
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xD84);                                 // Store to: 8007F27C
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == v1) goto loc_80048320;
    t0 = 0xFF;                                          // Result = 000000FF
    a3 = 0x80070000;                                    // Result = 80070000
    a3 += 0x5B1A;                                       // Result = gWess_CmdLength[1A] (80075B1A)
loc_80048238:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD80);                                // Load from: 8007F280
    v0 = lbu(v1);
    {
        const bool bJump = (v0 == t0);
        v0 = v1 + 1;
        if (bJump) goto loc_800482F8;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD80);                                 // Store to: 8007F280
    v1 = lbu(v1);
    v0 = v1 << 2;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AB8);                               // Load from: 80075AB8
    v0 <<= 4;
    v0 += v1;
    a1 = lw(v0 + 0x40);
    a0 = lw(v0 + 0x34);
    v1 = a1 + 4;
    sw(v1, v0 + 0x40);
    v1 = lbu(a3);                                       // Load from: gWess_CmdLength[1A] (80075B1A)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD7C);                                 // Store to: 8007F284
    v1 += a0;
    sw(v1, a1);
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0xD7C);                                // Load from: 8007F284
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lh(v0 - 0xD90);                                // Load from: 8007F270
    v1 = lw(a1 + 0x38);
    v0 <<= 2;
    v0 += v1;
    v1 = lw(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xD84);                               // Load from: 8007F27C
    a0 = lw(a1 + 0x30);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD88);                                 // Store to: 8007F278
    v1 += a0;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xD84);                                 // Store to: 8007F27C
    v0 &= 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD88);                                 // Store to: 8007F278
    sw(v1, a1 + 0x34);
    if (v0 == 0) goto loc_80048320;
loc_800482F8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xD8C);                               // Load from: 8007F274
    v1 = -1;                                            // Result = FFFFFFFF
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD8C);                                 // Store to: 8007F274
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    if (v0 != v1) goto loc_80048238;
loc_80048320:
    v0 = lw(a2);
    v0 |= 0x40;
    sw(v0, a2);
    return;
}

void Eng_SeqJump() noexcept {
loc_80048334:
    a3 = a0;
    v0 = lw(a3 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD74);                                 // Store to: 8007F28C
    v0 <<= 16;
    a2 = u32(i32(v0) >> 16);
    if (i32(a2) < 0) goto loc_800484C8;
    v0 = lh(a3 + 0x18);
    v0 = (i32(a2) < i32(v0));
    if (v0 == 0) goto loc_800484C8;
    v0 = lbu(a3 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    a0 = lw(a0 + 0xC);
    v1 += v0;
    a1 = lh(v1 + 0x2);
    a0 = lw(a0 + 0x10);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a0;
    v0 = lhu(v0);
    a0 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD5C);                                 // Store to: 8007F2A4
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD70);                                 // Store to: 8007F290
    v1 = lbu(v1 + 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD70);                                 // Store to: 8007F290
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD64);                                 // Store to: 8007F29C
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xD68);                                 // Store to: 8007F298
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == v1) goto loc_800484C8;
    t1 = 0xFF;                                          // Result = 000000FF
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x5AB8);                               // Load from: 80075AB8
    a2 <<= 2;
loc_80048418:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD64);                                // Load from: 8007F29C
    v0 = lbu(v1);
    {
        const bool bJump = (v0 == t1);
        v0 = v1 + 1;
        if (bJump) goto loc_800484A0;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD64);                                 // Store to: 8007F29C
    v0 = lbu(v1);
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 4;
    v1 += t0;
    v0 = lw(v1 + 0x38);
    a1 = lw(v1 + 0x30);
    v0 += a2;
    a0 = lw(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xD68);                               // Load from: 8007F298
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD60);                                 // Store to: 8007F2A0
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD6C);                                 // Store to: 8007F294
    a0 += a1;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xD68);                                 // Store to: 8007F298
    v0 &= 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD6C);                                 // Store to: 8007F294
    sw(a0, v1 + 0x34);
    if (v0 == 0) goto loc_800484C8;
loc_800484A0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xD70);                               // Load from: 8007F290
    v1 = -1;                                            // Result = FFFFFFFF
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD70);                                 // Store to: 8007F290
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    if (v0 != v1) goto loc_80048418;
loc_800484C8:
    v0 = lw(a3);
    v0 |= 0x40;
    sw(v0, a3);
    return;
}

void Eng_SeqRet() noexcept {
loc_800484DC:
    a2 = a0;
    v0 = lbu(a2 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    a0 = lw(a0 + 0xC);
    v1 += v0;
    a1 = lh(v1 + 0x2);
    a0 = lw(a0 + 0x10);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a0;
    v0 = lhu(v0);
    a0 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD48);                                 // Store to: 8007F2B8
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD58);                                 // Store to: 8007F2A8
    v1 = lbu(v1 + 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD58);                                 // Store to: 8007F2A8
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD50);                                 // Store to: 8007F2B0
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xD54);                                 // Store to: 8007F2AC
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == v1) goto loc_80048618;
    t0 = 0xFF;                                          // Result = 000000FF
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5AB8);                               // Load from: 80075AB8
    a3 = -1;                                            // Result = FFFFFFFF
loc_8004857C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD50);                                // Load from: 8007F2B0
    v0 = lbu(v1);
    {
        const bool bJump = (v0 == t0);
        v0 = v1 + 1;
        if (bJump) goto loc_800485F0;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD50);                                 // Store to: 8007F2B0
    v0 = lbu(v1);
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 4;
    v1 += a1;
    a0 = lw(v1 + 0x40);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD4C);                                 // Store to: 8007F2B4
    v0 = a0 - 4;
    sw(v0, v1 + 0x40);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xD54);                               // Load from: 8007F2AC
    a0 = lw(a0 - 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xD54);                                 // Store to: 8007F2AC
    v0 &= 0xFF;
    sw(a0, v1 + 0x34);
    if (v0 == 0) goto loc_80048618;
loc_800485F0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xD58);                               // Load from: 8007F2A8
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD58);                                 // Store to: 8007F2A8
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    if (v0 != a3) goto loc_8004857C;
loc_80048618:
    v0 = lw(a2);
    v0 |= 0x40;
    sw(v0, a2);
    return;
}

void Eng_SeqEnd() noexcept {
loc_8004862C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0);
    v0 &= 4;
    if (v0 != 0) goto loc_80048784;
    v1 = lbu(s0 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 = v1 << 1;
    v0 += v1;
    v1 = lw(a0 + 0x20);
    v0 <<= 3;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD44);                                 // Store to: 8007F2BC
    v1 = lbu(v0 + 0x4);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD38);                                 // Store to: 8007F2C8
    v1 = lw(v0 + 0xC);
    v0 = lbu(a0 + 0x1C);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD34);                                 // Store to: 8007F2CC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD3C);                                 // Store to: 8007F2C4
    v1 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD34);                                 // Store to: 8007F2CC
    if (v0 == v1) goto loc_800488BC;
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
loc_800486C8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD3C);                                // Load from: 8007F2C4
    v1 = lbu(v0);
    v0 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == v0) goto loc_80048748;
    a0 += v1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AB8);                               // Load from: 80075AB8
    a0 <<= 4;
    a0 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD40);                                 // Store to: 8007F2C0
    v0 = lbu(a0 + 0x3);
    v0 <<= 2;
    v0 += s0;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    ptr_call(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD38);                                // Load from: 8007F2C8
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD38);                                 // Store to: 8007F2C8
    if (v0 == 0) goto loc_800488BC;
loc_80048748:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD3C);                                // Load from: 8007F2C4
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD34);                                // Load from: 8007F2CC
    v0++;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD3C);                                 // Store to: 8007F2C4
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD34);                                 // Store to: 8007F2CC
    if (v1 == v0) goto loc_800488BC;
    goto loc_800486C8;
loc_80048784:
    v1 = lbu(s0 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 = v1 << 1;
    v0 += v1;
    v1 = lw(a0 + 0x20);
    v0 <<= 3;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD44);                                 // Store to: 8007F2BC
    v1 = lbu(v0 + 0x4);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD38);                                 // Store to: 8007F2C8
    v1 = lw(v0 + 0xC);
    v0 = lbu(a0 + 0x1C);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD34);                                 // Store to: 8007F2CC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD3C);                                 // Store to: 8007F2C4
    v1 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD34);                                 // Store to: 8007F2CC
    if (v0 == v1) goto loc_800488AC;
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
loc_800487F8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD3C);                                // Load from: 8007F2C4
    v1 = lbu(v0);
    v0 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == v0) goto loc_80048878;
    a0 += v1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AB8);                               // Load from: 80075AB8
    a0 <<= 4;
    a0 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD40);                                 // Store to: 8007F2C0
    v0 = lbu(a0 + 0x3);
    v0 <<= 2;
    v0 += s1;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    ptr_call(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD38);                                // Load from: 8007F2C8
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD38);                                 // Store to: 8007F2C8
    if (v0 == 0) goto loc_800488AC;
loc_80048878:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD3C);                                // Load from: 8007F2C4
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD34);                                // Load from: 8007F2CC
    v0++;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD3C);                                 // Store to: 8007F2C4
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD34);                                 // Store to: 8007F2CC
    if (v1 != v0) goto loc_800487F8;
loc_800488AC:
    v0 = lw(s0);
    v0 |= 0x40;
    sw(v0, s0);
loc_800488BC:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void Eng_TrkTempo() noexcept {
loc_800488D4:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lw(s0 + 0x34);
    v0 = lbu(v1 + 0x2);
    v1 = lbu(v1 + 0x1);
    v0 <<= 8;
    v1 |= v0;
    sh(v1, s0 + 0x16);
    v0 = GetIntsPerSec();
    v0 <<= 16;
    a1 = lh(s0 + 0x14);
    a2 = lh(s0 + 0x16);
    a0 = u32(i32(v0) >> 16);
    v0 = CalcPartsPerInt((int16_t) a0, (int16_t) a1, (int16_t) a2);
    sw(v0, s0 + 0x1C);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Eng_TrkGosub() noexcept {
loc_80048930:
    a1 = a0;
    v0 = lw(a1 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    v0 <<= 16;
    a2 = u32(i32(v0) >> 16);
    if (i32(a2) < 0) goto loc_800489BC;
    v0 = lh(a1 + 0x18);
    v0 = (i32(a2) < i32(v0));
    if (v0 == 0) goto loc_800489BC;
    a0 = lw(a1 + 0x40);
    v1 = lw(a1 + 0x34);
    v0 = a0 + 4;
    sw(v0, a1 + 0x40);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5B1A);                              // Load from: gWess_CmdLength[1A] (80075B1A)
    v0 += v1;
    sw(v0, a0);
    v1 = lw(a1 + 0x38);
    v0 = a2 << 2;
    v0 += v1;
    v1 = lw(v0);
    v0 = lw(a1);
    a0 = lw(a1 + 0x30);
    v0 |= 0x40;
    v1 += a0;
    sw(v0, a1);
    sw(v1, a1 + 0x34);
loc_800489BC:
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer command that jumps to a specified label
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_TrkJump(track_status& trackStat) noexcept {
    const int32_t labelIdx = ((int16_t) trackStat.ppos[1]) | ((int16_t) trackStat.ppos[2] << 8);

    // Ignore the command if the label is invalid
    if ((labelIdx >= 0) && (labelIdx < (int32_t) trackStat.labellist_count)) {
        const uint32_t targetOffset = trackStat.plabellist[labelIdx];

        // Goto the destination offset and set the 'skip' flag so the sequencer doesn't try to load the next command automatically
        trackStat.ppos = trackStat.pstart.get() + targetOffset;
        trackStat.deltatime = 0;
        trackStat.skip = true;
    }
}

void Eng_TrkRet() noexcept {
loc_80048A34:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lw(s0 + 0x40);
    v0 = v1 - 4;
    sw(v0, s0 + 0x40);
    a0 = lw(v1 - 0x4);
    a1 = s0 + 4;
    sw(a0, s0 + 0x34);
    v0 = ptrToVmAddr(Read_Vlq(vmAddrToPtr<uint8_t>(a0), *vmAddrToPtr<uint32_t>(a1)));
    v1 = lw(s0);
    sw(v0, s0 + 0x34);
    v1 |= 0x40;
    sw(v1, s0);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer command for the end of a track: mute the track or repeat it (if looped)
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_TrkEnd(track_status& trackStat) noexcept {
    if (trackStat.looped && (trackStat.totppi >= 16)) {
        // Repeat the track when looped (and if a small amount of time has elapsed) and go back to the start.
        // Also set the 'skip' flag so the sequencer does not try to goto the next command automatically.
        trackStat.skip = true;
        trackStat.ppos = trackStat.pstart;
        trackStat.ppos = Read_Vlq(trackStat.pstart.get(), trackStat.deltatime);
    }
    else {
        // Not repeating: mute the track
        a0 = ptrToVmAddr(&trackStat);
        gWess_CmdFuncArr[trackStat.patchtype][TrkOff]();

        // TODO: what is this trying to do?
        if (trackStat.handled) {
            trackStat.skip = true;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer command that does nothing
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_NullEvent([[maybe_unused]] track_status& trackStat) noexcept {
    // This command does nothing...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// The main sequencer tick/update update function which is called approximately 120 times a second.
// This is what drives sequencer timing and executes sequencer commands.
// Originally this was driven via interrupts coming from the PlayStation's hardware timers.
//------------------------------------------------------------------------------------------------------------------------------------------
void SeqEngine() noexcept {
    // Some helper variables for the loop
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    track_status* const pTrackStats = gpWess_eng_trackStats->get();
    const uint8_t maxTracks = *gWess_eng_maxTracks;

    // Run through all of the active tracks and run sequencer commands for them
    uint8_t numActiveTracksToVisit = mstat.trks_active;

    if (numActiveTracksToVisit > 0) {
        for (uint8_t trackIdx = 0; trackIdx < maxTracks; ++trackIdx) {
            track_status& trackStat = pTrackStats[trackIdx];

            // Skip past tracks that are not playing
            if (!trackStat.active)
                continue;

            // Only run sequencer commands for the track if it isn't paused
            if (!trackStat.stopped) {
                // Advance the track's time markers
                trackStat.starppi += trackStat.ppi;                 // Advance elapsed fractional quarter note 'parts' (16.16 fixed point format)
                trackStat.totppi += trackStat.starppi >> 16;        // Advance track total time in whole quarter note parts
                trackStat.accppi += trackStat.starppi >> 16;        // Advance track delta time till the next command in whole quarter note parts
                trackStat.starppi &= 0xFFFF;                        // We've advanced time by the whole part of this number: discount that part

                // Is it time to turn off a timed track?
                if (trackStat.timed && (trackStat.totppi >= trackStat.endppi)) {
                    // Turn off the timed track
                    a0 = ptrToVmAddr(&trackStat);
                    gWess_CmdFuncArr[trackStat.patchtype][TrkOff]();    // FIXME: convert to native call
                }
                else {
                    // Not a timed track or not reached the end. Continue executing sequencer commands while the track's time marker is >=
                    // to when the next command happens and while the track remains active and not stopped:
                    while ((trackStat.accppi >= trackStat.deltatime) && trackStat.active && (!trackStat.stopped)) {                                                
                        // Time to execute a new sequencer command: read that command firstly
                        const uint8_t seqCmd = trackStat.ppos[0];

                        // We have passed the required amount of delay/time until this sequencer command executes.
                        // Do not count that elapsed amount towards the next command delay/delta-time:
                        trackStat.accppi -= trackStat.deltatime;

                        // Decide what executes this command, the sequencer engine or the hardware driver
                        if ((seqCmd >= PatchChg) && (seqCmd <= NoteOff)) {
                            // The hardware sound driver executes this command: do it!
                            a0 = ptrToVmAddr(&trackStat);
                            gWess_CmdFuncArr[trackStat.patchtype][seqCmd]();    // FIXME: convert to native call

                            // Skip past the command bytes and read the delta time until the next command
                            trackStat.ppos += gWess_seq_CmdLength[seqCmd];
                            trackStat.ppos = Read_Vlq(trackStat.ppos.get(), trackStat.deltatime);
                        } 
                        else if ((seqCmd >= StatusMark) && (seqCmd <= NullEvent)) {
                            // The sequencer executes this command: do it!
                            a0 = ptrToVmAddr(&trackStat);
                            gWess_DrvFunctions[seqCmd]();   // FIXME: convert to native call

                            // Automatically go onto the next sequencer command unless we are instructed to skip doing that.
                            // Some commands which change the control flow will set the 'skip' flag so that they may set where to go to next.
                            if (trackStat.active && (!trackStat.skip)) {
                                // Skip past the command bytes and read the delta time until the next command
                                trackStat.ppos += gWess_seq_CmdLength[seqCmd];
                                trackStat.ppos = Read_Vlq(trackStat.ppos.get(), trackStat.deltatime);
                            } else {
                                // Clear this instruction: 'skip' is just done once when requested
                                trackStat.skip = false;
                            }
                        } else {
                            // This is an unknown command or a command that should NOT be in a sequence.
                            // Since we don't know what to do, just stop the track.
                            a0 = ptrToVmAddr(&trackStat);
                            Eng_SeqEnd();   // FIXME: convert to native call
                        }
                    }
                }
            }

            // If there are no more tracks to process then stop now
            numActiveTracksToVisit--;

            if (numActiveTracksToVisit == 0)
                break;
        }
    }

    // Call the 'update' function for the sound driver: this does management, such as freeing up unused hardware voices
    track_status& firstTrack = pTrackStats[0];
    gWess_CmdFuncArr[firstTrack.patchtype][DriverEntry1]();
}
