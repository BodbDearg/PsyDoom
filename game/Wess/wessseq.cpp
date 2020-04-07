//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): core sequencer driver commands and sequencer functions.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessseq.h"

#include "wessarc.h"

#define PSX_VM_NO_REGISTER_MACROS 1
#include "PsxVm/PsxVm.h"

const WessDriverFunc gWess_DrvFunctions[36] = {
    // Manually called commands
    Eng_DriverInit,                 // 00
    Eng_DriverExit,                 // 01
    Eng_DriverEntry1,               // 02
    Eng_DriverEntry2,               // 03
    Eng_DriverEntry3,               // 04
    Eng_TrkOff,                     // 05
    Eng_TrkMute,                    // 06
    // Hardware driver commands
    Eng_PatchChg,                   // 07
    Eng_PatchMod,                   // 08
    Eng_PitchMod,                   // 09
    Eng_ZeroMod,                    // 10
    Eng_ModuMod,                    // 11
    Eng_VolumeMod,                  // 12
    Eng_PanMod,                     // 13
    Eng_PedalMod,                   // 14
    Eng_ReverbMod,                  // 15
    Eng_ChorusMod,                  // 16
    Eng_NoteOn,                     // 17
    Eng_NoteOff,                    // 18
    // Sequencer commands
    Eng_StatusMark,                 // 19
    Eng_GateJump,                   // 20
    Eng_IterJump,                   // 21
    Eng_ResetGates,                 // 22
    Eng_ResetIters,                 // 23
    Eng_WriteIterBox,               // 24
    Eng_SeqTempo,                   // 25
    Eng_SeqGosub,                   // 26
    Eng_SeqJump,                    // 27
    Eng_SeqRet,                     // 28
    Eng_SeqEnd,                     // 29
    Eng_TrkTempo,                   // 30
    Eng_TrkGosub,                   // 31
    Eng_TrkJump,                    // 32
    Eng_TrkRet,                     // 33
    Eng_TrkEnd,                     // 34
    Eng_NullEvent                   // 35
};

// The size in bytes of each sequencer command
static constexpr uint8_t gWess_seq_CmdLength[36] = {
    0,      // DriverInit       (This command should NEVER be in a sequence)
    0,      // DriverExit       (This command should NEVER be in a sequence)
    0,      // DriverEntry1     (This command should NEVER be in a sequence)
    0,      // DriverEntry2     (This command should NEVER be in a sequence)
    0,      // DriverEntry3     (This command should NEVER be in a sequence)
    0,      // TrkOff           (This command should NEVER be in a sequence)
    0,      // TrkMute          (This command should NEVER be in a sequence)
    3,      // PatchChg
    2,      // PatchMod
    3,      // PitchMod
    2,      // ZeroMod
    2,      // ModuMod
    2,      // VolumeMod
    2,      // PanMod
    2,      // PedalMod
    2,      // ReverbMod
    2,      // ChorusMod
    3,      // NoteOn
    2,      // NoteOff
    4,      // StatusMark
    5,      // GateJump
    5,      // IterJump
    2,      // ResetGates
    2,      // ResetIters
    3,      // WriteIterBox
    3,      // SeqTempo
    3,      // SeqGosub
    3,      // SeqJump
    1,      // SeqRet
    1,      // SeqEnd
    3,      // TrkTempo
    3,      // TrkGosub
    3,      // TrkJump
    1,      // TrkRet
    1,      // TrkEnd
    1       // NullEvent
};

static_assert(C_ARRAY_SIZE(gWess_seq_CmdLength) == C_ARRAY_SIZE(gWess_DrvFunctions));

const VmPtr<uint8_t>            gWess_master_sfx_volume(0x80075A04);    // TODO: COMMENT
const VmPtr<uint8_t>            gWess_master_mus_volume(0x80075A05);    // TODO: COMMENT
const VmPtr<PanMode>            gWess_pan_status(0x80075A06);           // Current pan mode
const VmPtr<VmPtr<NoteState>>   gpWess_notestate(0x80075A10);           // TODO: COMMENT

static const VmPtr<VmPtr<master_status_structure>>      gpWess_eng_mstat(0x80075AC0);           // TODO: COMMENT
static const VmPtr<VmPtr<sequence_status>>              gpWess_eng_seqStats(0x80075ABC);        // TODO: COMMENT
static const VmPtr<VmPtr<track_status>>                 gpWess_eng_trackStats(0x80075AB8);      // TODO: COMMENT
static const VmPtr<uint8_t>                             gWess_eng_maxTracks(0x80075AB4);        // TODO: COMMENT

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads from track data the delta time (relative to the previous sequencer command) for when the next sequencer command is to be executed.
// Returns the pointer after reading the time amount, which may be incremented an variable/arbitrary amount.
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

//------------------------------------------------------------------------------------------------------------------------------------------
// The opposite function to 'Read_Vlq': not used by the game but probably used by music generation tools.
// Write a variable length delta time value for a track to the given byte stream, using up to 5 bytes to encode it.
// Returns the pointer after the written value.
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t* Write_Vlq(uint8_t* const pTrackBytes, const uint32_t deltaTime) noexcept {
    // Encode the given value into a stack buffer, writing a '0x80' bit flag whenever more bytes follow
    uint8_t encodedBytes[8];
    uint8_t* pEncodedByte = encodedBytes;

    {
        *pEncodedByte = (uint8_t)(deltaTime & 0x7F);
        pEncodedByte++;
        uint32_t bitsToEncode = deltaTime >> 7;

        while (bitsToEncode != 0) {
            *pEncodedByte = (uint8_t)(bitsToEncode | 0x80);
            bitsToEncode >>= 7;
            pEncodedByte++;
        }
    }
    
    // Copy the encoded value to the given output buffer.
    // Note that the ordering here gets reversed, so it winds up being read in the correct order.
    uint8_t* pOutputByte = pTrackBytes;

    do {
        pEncodedByte--;
        *pOutputByte = *pEncodedByte;
        pOutputByte++;
    } while (*pEncodedByte & 0x80);

    return pOutputByte;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out the size in bytes of the given track delta time value when encoded into a track
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t Len_Vlq(const uint32_t deltaTime) noexcept {
    // Do a dummy encoding to find out the length
    uint8_t encodedBytes[8];
    uint8_t* pEncodedBytesEnd = Write_Vlq(encodedBytes, deltaTime);
    return (int32_t)(pEncodedBytesEnd - encodedBytes);
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Invoke a single (only!) registered sequencer callback matching the type specified in the command, with the value specified in the command
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_StatusMark(track_status& trackStat) noexcept {
    // If there are no callbacks active then there is nothing to do
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    uint8_t activeCallbacksLeft = mstat.callbacks_active;

    if (activeCallbacksLeft <= 0)
        return;

    // Try to find a matching active callback type to invoke
    const uint8_t maxCallbacks = mstat.pmod_info->mod_hdr.callback_areas;
        
    for (uint8_t callbackIdx = 0; callbackIdx < maxCallbacks; ++callbackIdx) {
        callback_status& callbackStat = mstat.pcalltable[callbackIdx];

        // Ignore this callback if it's not active
        if (!callbackStat.active)
            continue;

        // Only invoke the callback if it matches the type in the command
        const uint8_t callbackType = trackStat.ppos[1];

        if (callbackStat.type == callbackType) {
            // FIXME: remove this once real function pointers are used
            typedef void (*CallbackFunc)(uint8_t callbackType, int16_t value);
            const CallbackFunc callback = (CallbackFunc) PsxVm::getVmFuncForAddr(callbackStat.callfunc.addr());

            // Invoke the callback with the value specified in the command
            const int16_t callbackVal = ((int16_t) trackStat.ppos[2]) | ((int16_t) trackStat.ppos[3] << 8);
            callbackStat.curval = callbackVal;
            callback(callbackType, callbackVal);
            break;
        }
            
        // If there are no more callbacks left to visit then we are done searching
        activeCallbacksLeft--;

        if (activeCallbacksLeft == 0)
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Jump to a label if a specified boolean gate is set (value > 0).
// If the gate is reset (value 0xFF) then it's value is also set to the value specified by the command.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_GateJump(track_status& trackStat) noexcept {
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];

    // Get the specified gate value
    const uint8_t gateIdx = trackStat.ppos[1];    
    const uint8_t gateVal = seqStat.pgates[gateIdx];

    if (gateVal != 0) {
        if (gateVal == 0xFF) {
            seqStat.pgates[gateIdx] = trackStat.ppos[2];
        }

        // Goto the track label specified in the command (if valid)
        const int32_t labelIdx = ((int16_t) trackStat.ppos[3]) | ((int16_t) trackStat.ppos[4] << 8);
        
        if ((labelIdx >= 0) && (labelIdx < (int32_t) trackStat.labellist_count)) {
            const uint32_t targetOffset = trackStat.plabellist[labelIdx];
            trackStat.ppos = trackStat.pstart.get() + targetOffset;
        }
    }

    trackStat.skip = true;  // Tell the sequencer to not automatically determine the next command
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Jump to a label while the specicied iteration count (iter) is > 0, up to a maximum number of times.
// The number of items is initialized on the first jump to the specified amount if the iteration count was previously reset (0xFF).
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_IterJump(track_status& trackStat) noexcept {
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];

    // Get the specified iteration count
    const uint8_t iterIdx = trackStat.ppos[1];    
    const uint8_t iterVal = seqStat.piters[iterIdx];

    // Only do the jump if it's not zero
    if (iterVal != 0) {
        // If the count has not been intialized yet then it's value is initialized from the command.
        // Otherwise we decrement the iteration count.
        if (iterVal == 0xFF) {
            seqStat.piters[iterIdx] = trackStat.ppos[2];
        } else {
            seqStat.piters[iterIdx] = iterVal - 1;
        }

        // Goto the track label specified in the command (if valid)
        const int32_t labelIdx = ((int16_t) trackStat.ppos[3]) | ((int16_t) trackStat.ppos[4] << 8);

        if ((labelIdx >= 0) && (labelIdx < (int32_t) trackStat.labellist_count)) {
            const uint32_t targetOffset = trackStat.plabellist[labelIdx];
            trackStat.ppos = trackStat.pstart.get() + targetOffset;
        }
    }

    trackStat.skip = true;  // Tell the sequencer to not automatically determine the next command
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clear the specified boolean gate for a track, or clear all gates for the track
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_ResetGates(track_status& trackStat) noexcept {
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];

    // Either reset all gates (gate index '0xFF') or reset a specific one
    const uint8_t gateIdx = trackStat.ppos[1];

    if (gateIdx == 0xFF) {
        const uint8_t gatesPerSeq = mstat.pmod_info->mod_hdr.gates_per_seq;

        for (uint8_t i = 0; i < gatesPerSeq; ++i) {
            seqStat.pgates[i] = 0xFF;
        }
    } else {
        seqStat.pgates[gateIdx] = 0xFF;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clear the specified iteration (iter) count for a track, or clear all iteration counts for the track
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_ResetIters(track_status& trackStat) noexcept {
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];

    // Either reset all iters (iter index '0xFF') or reset a specific one
    const uint8_t iterIdx = trackStat.ppos[1];

    if (iterIdx == 0xFF) {
        const uint8_t itersPerSeq = mstat.pmod_info->mod_hdr.iters_per_seq;

        for (uint8_t i = 0; i < itersPerSeq; ++i) {
            seqStat.piters[i] = 0xFF;
        }
    } else {
        seqStat.piters[iterIdx] = 0xFF;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the value for a specified iteration count in a track
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_WriteIterBox(track_status& trackStat) noexcept {
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];
    const uint8_t iterIdx = trackStat.ppos[1];
    seqStat.piters[iterIdx] = trackStat.ppos[2];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Command which updates the tempo for all tracks in the sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_SeqTempo(track_status& trackStat) noexcept {
    // Helper variables for the loop
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];
    sequence_data& seqInfo = mstat.pmod_info->pseq_info[seqStat.seq_num];

    // Read the new quarter notes per minute (BPM) amount from the command
    const uint16_t newQpm = ((uint16_t) trackStat.ppos[1]) | ((uint16_t) trackStat.ppos[2] << 8);
    
    // Set the tempo for all active tracks in this track's sequence
    uint8_t activeTracksLeft = seqStat.tracks_active;
    
    for (uint16_t i = 0; i < seqInfo.seq_hdr.tracks; ++i) {
        // See if this sequence track slot is in use, if not then ignore.
        //
        // BUG FIX: in the original code there was a bug here where it would not be able advance onto the next sequence track
        // slot once it encountered an unused one, thus some tracks in the sequence might not have had their tempo set as intended.
        // This bug is fixed however with this re-implementation, because of the way the loop is written.
        const uint8_t trackIdx = seqStat.ptrk_indxs[i];

        if (trackIdx == 0xFF)
            continue;

        // Update the quarter notes per minute and parts per interrupt (16.16) advancement for this track
        track_status& thisTrackStat = gpWess_eng_trackStats->get()[trackIdx];
        thisTrackStat.qpm = newQpm;
        thisTrackStat.ppi = CalcPartsPerInt(GetIntsPerSec(), thisTrackStat.ppq, thisTrackStat.qpm);

        // If there are no more active tracks left to update then we are done
        activeTracksLeft--;
        
        if (activeTracksLeft == 0) 
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Similar to 'Eng_TrkGosub' but for all tracks in a given track's sequence.
// Makes all the tracks jump to the specified label and remembers the return location (past the current command) in each track's stack.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_SeqGosub(track_status& trackStat) noexcept {
    // Some useful stuff for the loop below
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];
    const sequence_data& seqInfo = mstat.pmod_info->pseq_info[seqStat.seq_num];

    // Get the label to jump to from the command and do not do any jump if the label index is invalid
    const int32_t labelIdx = ((int16_t) trackStat.ppos[1]) | ((int16_t) trackStat.ppos[2] << 8);

    if ((labelIdx >= 0) && (labelIdx < (int32_t) trackStat.labellist_count)) {
        // Update the current position for all active tracks in the sequence
        uint8_t activeTracksLeft = seqStat.tracks_active;

        for (uint16_t i = 0; i < seqInfo.seq_hdr.tracks; ++i) {
            // See if this sequence track slot is in use, if not then ignore.
            //
            // BUG FIX: in the original code there was a bug here where it would not be able advance onto the next sequence track
            // slot once it encountered an unused one, thus some tracks in the sequence might not have had their position set as intended.
            // This bug is fixed however with this re-implementation, because of the way the loop is written.
            const uint8_t trackIdx = seqStat.ptrk_indxs[i];

            if (trackIdx == 0xFF)
                continue;

            // Save the current location in the track's location stack and use up a location stack slot
            track_status& thisTrackStat = gpWess_eng_trackStats->get()[trackIdx];
            *thisTrackStat.psp = thisTrackStat.ppos.get() + gWess_seq_CmdLength[SeqGosub];
            thisTrackStat.psp += 1;
            
            // Update the track to the new location
            thisTrackStat.ppos = thisTrackStat.pstart.get() + thisTrackStat.plabellist[labelIdx];

            // If there are no more active tracks then we are done
            activeTracksLeft--;

            if (activeTracksLeft == 0)
                break;
        }
    }

    trackStat.skip = true;      // Tell the sequencer not to determine the next sequencer command automatically
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Command which makes all tracks in the sequence jump to a specified label
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_SeqJump(track_status& trackStat) noexcept {    
    // Some useful stuff for the loop below
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];
    const sequence_data& seqInfo = mstat.pmod_info->pseq_info[seqStat.seq_num];
    
    // Get the label to jump to from the command and do not do any jump if the label index is invalid
    const int32_t labelIdx = ((int16_t) trackStat.ppos[1]) | ((int16_t) trackStat.ppos[2] << 8);

    if ((labelIdx >= 0) && (labelIdx < (int32_t) trackStat.labellist_count)) {
        // Update the current position for all active tracks in the sequence
        uint8_t activeTracksLeft = seqStat.tracks_active;

        for (uint16_t i = 0; i < seqInfo.seq_hdr.tracks; ++i) {
            // See if this sequence track slot is in use, if not then ignore.
            //
            // BUG FIX: in the original code there was a bug here where it would not be able advance onto the next sequence track
            // slot once it encountered an unused one, thus some tracks in the sequence might not have had their position set as intended.
            // This bug is fixed however with this re-implementation, because of the way the loop is written.
            const uint8_t trackIdx = seqStat.ptrk_indxs[i];

            if (trackIdx == 0xFF)
                continue;

            // Update the current position of the track
            track_status& thisTrackStat = gpWess_eng_trackStats->get()[trackIdx];
            const uint32_t targetOffset = thisTrackStat.plabellist[labelIdx];
            thisTrackStat.ppos = thisTrackStat.pstart.get() + targetOffset;

            // If there are no more active tracks then we are done
            activeTracksLeft--;

            if (activeTracksLeft == 0)
                break;
        }
    }

    trackStat.skip = true;      // Tell the sequencer not to determine the next sequencer command automatically
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns all tracks in a track's sequence to the location previously saved by a 'Eng_SeqGosub' command.
// Reads the location to return to from each track's location stack.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_SeqRet(track_status& trackStat) noexcept {
    // Some useful stuff for the loop below
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    sequence_status& seqStat = gpWess_eng_seqStats->get()[trackStat.seq_owner];
    const sequence_data& seqInfo = mstat.pmod_info->pseq_info[seqStat.seq_num];
    
    // Restore the previous location for all active tracks in the sequence
    uint8_t activeTracksLeft = seqStat.tracks_active;

    for (uint16_t i = 0; i < seqInfo.seq_hdr.tracks; ++i) {
        // See if this sequence track slot is in use, if not then ignore.
        //
        // BUG FIX: in the original code there was a bug here where it would not be able advance onto the next sequence track
        // slot once it encountered an unused one, thus some tracks in the sequence might not have had their position set as intended.
        // This bug is fixed however with this re-implementation, because of the way the loop is written.
        const uint8_t trackIdx = seqStat.ptrk_indxs[i];

        if (trackIdx == 0xFF)
            continue;

        // Restore the previously saved track location and free up the location stack slot
        track_status& thisTrackStat = gpWess_eng_trackStats->get()[trackIdx];
        thisTrackStat.psp -= 1;
        thisTrackStat.ppos = *thisTrackStat.psp;

        // If there are no more active tracks then we are done
        activeTracksLeft--;

        if (activeTracksLeft == 0)
            break;
    }

    trackStat.skip = true;      // Tell the sequencer not to determine the next sequencer command automatically
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ends/mutes all tracks in the given track's parent sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_SeqEnd(track_status& trackStat) noexcept {
    // Helper variables for the loop
    master_status_structure& mstat = *gpWess_eng_mstat->get();
    sequence_status& seqStat = mstat.pseqstattbl[trackStat.seq_owner];

    // Run through all of the active tracks in the sequence and mute them all
    uint8_t activeTracksLeft = seqStat.tracks_active;

    for (uint32_t i = 0; i < mstat.max_trks_perseq; ++i) {
        // Ignore this track if not active
        const uint8_t trackIdx = seqStat.ptrk_indxs[i];

        if (trackIdx == 0xFF)
            continue;

        // Mute the track
        track_status& thisTrackStat = gpWess_eng_trackStats->get()[trackIdx];
        gWess_CmdFuncArr[thisTrackStat.patchtype][TrkOff](thisTrackStat);

        // If there are no more tracks left then we are done
        activeTracksLeft--;

        if (activeTracksLeft == 0)
            break;
    }

    // TODO: what is this trying to do?
    if (trackStat.handled) {
        trackStat.skip = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the tempo or beats per minute for a track
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_TrkTempo(track_status& trackStat) noexcept {
    // Set the new quarter notes per second (BPM) amount
    const uint16_t qpm = ((uint16_t) trackStat.ppos[1]) | ((uint16_t) trackStat.ppos[2] << 8);
    trackStat.qpm = qpm;

    // Update how many quarter note parts to advance the track by, per hardware interrupt (16.16 format)
    trackStat.ppi = CalcPartsPerInt(GetIntsPerSec(), trackStat.ppq, trackStat.qpm);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Jump to a specified label and remember the current location (after this command) in the track's location stack.
// The remembered location can be returned to later.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_TrkGosub(track_status& trackStat) noexcept {
    // Ignore the command if the label is invalid
    const int32_t labelIdx = ((int16_t) trackStat.ppos[1]) | ((int16_t) trackStat.ppos[2] << 8);
  
    if ((labelIdx >= 0) && (labelIdx < (int32_t) trackStat.labellist_count)) {
        // Save the return location to after this command in the track's location stack.
        // PC-PSX: small correction here, though makes no difference - the command length taken should be 'TrkGosub' instead of 'SeqGosub'.
        #if PC_PSX_DOOM_MODS
            *trackStat.psp = trackStat.ppos.get() + gWess_seq_CmdLength[TrkGosub];
        #else
            *trackStat.psp = trackStat.ppos.get() + gWess_seq_CmdLength[SeqGosub];
        #endif

        // Used up one slot in the location stack
        trackStat.psp += 1;
        
        // Jump to the specified label
        const uint32_t targetOffset = trackStat.plabellist[labelIdx];
        trackStat.ppos = trackStat.pstart.get() + targetOffset;
        trackStat.skip = true;
    }
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the track to the location previously saved by a 'TrkGosub' command.
// Reads the location to return to from the track's location stack.
//------------------------------------------------------------------------------------------------------------------------------------------
void Eng_TrkRet(track_status& trackStat) noexcept {
    // Restore the previously saved track location and free up the location stack slot
    trackStat.psp -= 1;
    trackStat.ppos = *trackStat.psp;

    // Read the time until when the next command executes and instruct the sequencer not to automatically determine this
    trackStat.ppos = Read_Vlq(trackStat.ppos.get(), trackStat.deltatime);
    trackStat.skip = true;
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
        gWess_CmdFuncArr[trackStat.patchtype][TrkOff](trackStat);

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
                    gWess_CmdFuncArr[trackStat.patchtype][TrkOff](trackStat);
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
                            gWess_CmdFuncArr[trackStat.patchtype][seqCmd](trackStat);

                            // Skip past the command bytes and read the delta time until the next command
                            trackStat.ppos += gWess_seq_CmdLength[seqCmd];
                            trackStat.ppos = Read_Vlq(trackStat.ppos.get(), trackStat.deltatime);
                        } 
                        else if ((seqCmd >= StatusMark) && (seqCmd <= NullEvent)) {
                            // The sequencer executes this command: do it!
                            gWess_DrvFunctions[seqCmd](trackStat);

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
                            Eng_SeqEnd(trackStat);
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
