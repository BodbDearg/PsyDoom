//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): manipulating tracks with a given integer 'type'.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessapi_t.h"

#include "PsxVm/PsxVm.h"
#include "PsxVm/VmSVal.h"
#include "wessapi.h"
#include "wessarc.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Update one or more playback attributes for the given sequencer track
//------------------------------------------------------------------------------------------------------------------------------------------
void updatetrackstat(track_status& trackStat, const TriggerPlayAttr* const pPlayAttribs) noexcept {
    // If no attributes were given or none are being set then just exit now - nothing to do
    if (!pPlayAttribs)
        return;
    
    const uint32_t attribMask = pPlayAttribs->mask;

    if (attribMask == 0)
        return;

    // TODO: TEMP MEASURE FOR NOW - REMOVE EVENTUALLY: need a stack buffer in VM space for now
    struct Cmd {
        uint8_t bytes[8];
    };

    VmSVal<Cmd> cmd;
    uint8_t* const cmdBytes = cmd->bytes;
    
    // Set all of the specified attributes, issuing sequencer commands where required to perform the change
    if (attribMask & TRIGGER_VOLUME) {
        trackStat.volume_cntrl = pPlayAttribs->volume;
    }

    if (attribMask & TRIGGER_PAN) {
        trackStat.pan_cntrl = pPlayAttribs->pan;
    }

    if (attribMask & (TRIGGER_VOLUME | TRIGGER_PAN)) {
        // Set the volume and pan on the track
        trackStat.volume_cntrl = pPlayAttribs->volume;
        trackStat.pan_cntrl = pPlayAttribs->pan;

        // Issue a sequencer command to to update the volume levels: change the track command stream temporarily also to do this
        uint8_t* const pOldCmdBytes = trackStat.ppos.get();        
        trackStat.ppos = cmdBytes;

        cmdBytes[0] = VolumeMod;
        cmdBytes[1] = trackStat.volume_cntrl;        
        a0 = ptrToVmAddr(&trackStat);
        gWess_CmdFuncArr[trackStat.patchtype][VolumeMod]();     // FIXME: convert to native function call
        
        trackStat.ppos = pOldCmdBytes;
    }
    else {
        if (attribMask & TRIGGER_VOLUME) {
            // Set the volume on the track
            trackStat.volume_cntrl = pPlayAttribs->volume;

            // Issue a sequencer command to to update the volume: change the track command stream temporarily also to do this
            uint8_t* const pOldCmdBytes = trackStat.ppos.get();
            trackStat.ppos = cmdBytes;

            cmdBytes[0] = VolumeMod;
            cmdBytes[1] = trackStat.volume_cntrl;
            a0 = ptrToVmAddr(&trackStat);
            gWess_CmdFuncArr[trackStat.patchtype][VolumeMod]();     // FIXME: convert to native function call

            trackStat.ppos = pOldCmdBytes;
        }
        else if (attribMask & TRIGGER_PAN) {
            // Set the pan on the track
            trackStat.pan_cntrl = pPlayAttribs->pan;

            // Issue a sequencer command to to update the pan: change the track command stream temporarily also to do this
            uint8_t* const pOldCmdBytes = trackStat.ppos.get();
            trackStat.ppos = cmdBytes;

            cmdBytes[0] = PanMod;
            cmdBytes[1] = trackStat.pan_cntrl;
            a0 = ptrToVmAddr(&trackStat);
            gWess_CmdFuncArr[trackStat.patchtype][PanMod]();        // FIXME: convert to native function call
            
            trackStat.ppos = pOldCmdBytes;
        }
    }

    if (attribMask & TRIGGER_PATCH) {
        trackStat.patchnum = pPlayAttribs->patch;
    }

    if (attribMask & TRIGGER_PITCH) {
        // Set the pitch on the track
        trackStat.pitch_cntrl = pPlayAttribs->pitch;

        // Issue a sequencer command to update the patch: change the track command stream temporarily also to do this
        uint8_t* const pOldCmdBytes = trackStat.ppos.get();
        trackStat.ppos = cmd->bytes;
        
        cmdBytes[0] = PitchMod;
        cmdBytes[1] = (uint8_t)(trackStat.pitch_cntrl >> 0);
        cmdBytes[2] = (uint8_t)(trackStat.pitch_cntrl >> 8);
        a0 = ptrToVmAddr(&trackStat);
        gWess_CmdFuncArr[trackStat.patchtype][PitchMod]();          // FIXME: convert to native function call

        trackStat.ppos = pOldCmdBytes;
    }

    if (attribMask & TRIGGER_MUTEMODE) {
        if (trackStat.mutemask & (1 << pPlayAttribs->mutemode)) {
            trackStat.mute = true;
        } else {
            trackStat.mute = false;
        }
    }

    if (attribMask & TRIGGER_TEMPO) {
        trackStat.qpm = pPlayAttribs->tempo;
        trackStat.ppi = CalcPartsPerInt(GetIntsPerSec(), trackStat.ppq, trackStat.qpm);
    }

    if (attribMask & TRIGGER_TIMED) {
        trackStat.endppi = trackStat.totppi + pPlayAttribs->timeppq;
        trackStat.timed = true;
    }

    if (attribMask & TRIGGER_LOOPED) {
        trackStat.looped = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger the specified sequence number and assign it the given type number
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_trigger_type(const int32_t seqNum, const uint32_t seqType) noexcept {
    master_status_structure& mstat = *gpWess_pm_stat->get();
    wess_seq_structrig(mstat.pmod_info->pseq_info[seqNum], seqNum, seqType, false, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger the specified sequence number with custom play attributes and assign it the given type number
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_trigger_type_special(const int32_t seqNum, const uint32_t seqType, const TriggerPlayAttr* const pPlayAttribs) noexcept {
    master_status_structure& mstat = *gpWess_pm_stat->get();
    wess_seq_structrig(mstat.pmod_info->pseq_info[seqNum], seqNum, seqType, false, pPlayAttribs);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update sequences of the specified type with the given play attributes
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_update_type_special(const uint32_t seqType, const TriggerPlayAttr* const pPlayAttribs) noexcept {
    // Don't bother if there is no module loaded
    if (!Is_Module_Loaded())
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    *gbWess_SeqOn = false;

    // Grab some basic info from the master status
    master_status_structure& mstat = *gpWess_pm_stat->get();

    const uint8_t maxSeqs = mstat.pmod_info->mod_hdr.seq_work_areas;
    const uint32_t maxTracksPerSeq = mstat.max_trks_perseq;
    uint8_t numActiveSeqsToVisit = mstat.seqs_active;

    if (numActiveSeqsToVisit > 0) {
        // Run through all of the active sequences of the specified type and update them all
        for (uint8_t seqIdx = 0; seqIdx < maxSeqs; ++seqIdx) {
            sequence_status& seqStat = mstat.pseqstattbl[seqIdx];

            if (!seqStat.active)
                continue;

            if (seqStat.seq_type == seqType) {
                // This is the sequence type we want: tun through all of the active tracks in the sequence and update them all
                uint32_t numActiveSeqTracksToVisit = seqStat.tracks_active;
                uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

                for (uint32_t i = 0; i < maxTracksPerSeq; ++i) {
                    // Is this sequence track slot actually in use? Skip if not:
                    const uint8_t trackIdx = pSeqTrackIndexes[i];

                    if (trackIdx == 0xFF)
                        continue;
                    
                    // Only bother with this call if there are actually attributes to set.
                    // This check should have probably been done on function entry? This is somewhat wasteful doing it here:
                    if (pPlayAttribs) {
                        updatetrackstat(mstat.ptrkstattbl[trackIdx], pPlayAttribs);
                    }

                    // If there are no more active tracks left to visit then we are done
                    --numActiveSeqTracksToVisit;

                    if (numActiveSeqTracksToVisit == 0)
                        break;
                }
            }

            // If there are no more active sequences to visit then we are done
            --numActiveSeqsToVisit;

            if (numActiveSeqsToVisit == 0)
                break;
        }
    }

    // Re-enable the sequencer
    *gbWess_SeqOn = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop music sequences with the specified type number
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_stoptype(const uint32_t seqType) noexcept {
    // If the module is not loaded then there is nothing to do
    if (!Is_Module_Loaded())
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    *gbWess_SeqOn = false;

    // Grab some basic info from the master status
    master_status_structure& mstat = *gpWess_pm_stat->get();

    const uint8_t maxSeqs = mstat.pmod_info->mod_hdr.seq_work_areas;
    const uint32_t maxTracksPerSeq = mstat.max_trks_perseq;
    uint8_t numActiveSeqsToVisit = mstat.seqs_active;

    if (numActiveSeqsToVisit > 0) {
        // Run through all the sequences that are active looking for a sequence of a given type
        for (uint32_t seqIdx = 0; seqIdx < maxSeqs; ++seqIdx) {
            sequence_status& seqStat = mstat.pseqstattbl[seqIdx];

            if (!seqStat.active)
                continue;
            
            if (seqStat.seq_type == seqType) {
                // This is the sequence type we want to stop: run through all its active tracks and stop them all
                uint8_t numActiveTracks = seqStat.tracks_active;
                const uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

                for (uint8_t i = 0; i < maxTracksPerSeq; ++i) {
                    // Is this sequence track slot actually in use? Skip if not:
                    const uint8_t trackIdx = pSeqTrackIndexes[i];

                    if (trackIdx == 0xFF)
                        continue;

                    // Call the driver function to turn off the track
                    track_status& trackStat = mstat.ptrkstattbl[trackIdx];
                    a0 = ptrToVmAddr(&trackStat);
                    gWess_CmdFuncArr[trackStat.patchtype][TrkOff]();    // FIXME: convert to native function call

                    // If there are no more tracks left active then we are done
                    numActiveTracks--;

                    if (numActiveTracks == 0)
                        break;
                }
            }
            
            // If there are no more active sequences to visit then we are done
            numActiveSeqsToVisit--;

            if (numActiveSeqsToVisit == 0)
                break;
        }
    }

    // Re-enable the sequencer
    *gbWess_SeqOn = true;
}
