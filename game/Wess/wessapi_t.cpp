//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): sequence triggering and stopping.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessapi_t.h"

#include "wessapi.h"
#include "wessarc.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Update one or more playback attributes for the given sequencer track
//------------------------------------------------------------------------------------------------------------------------------------------
void updatetrackstat(track_status& trackStat, const TriggerPlayAttr* const pPlayAttribs) noexcept {
    // If no attributes were given or none are being set then just exit now - nothing to do
    if (!pPlayAttribs)
        return;

    const uint32_t attribsMask = pPlayAttribs->attribs_mask;

    if (attribsMask == 0)
        return;

    // Set all of the specified attributes, issuing sequencer commands where required to perform the change
    if (attribsMask & TRIGGER_VOLUME) {
        trackStat.volume_cntrl = pPlayAttribs->volume_cntrl;
    }

    if (attribsMask & TRIGGER_PAN) {
        trackStat.pan_cntrl = pPlayAttribs->pan_cntrl;
    }

    if (attribsMask & (TRIGGER_VOLUME | TRIGGER_PAN)) {
        // Set the volume and pan on the track
        trackStat.volume_cntrl = pPlayAttribs->volume_cntrl;
        trackStat.pan_cntrl = pPlayAttribs->pan_cntrl;

        // Issue a sequencer command to to update the volume levels: change the track command stream temporarily also to do this
        uint8_t* const pPrevCmd = trackStat.pcur_cmd;

        uint8_t cmdBytes[8];
        cmdBytes[0] = VolumeMod;
        cmdBytes[1] = trackStat.volume_cntrl;

        trackStat.pcur_cmd = cmdBytes;
        gWess_CmdFuncArr[trackStat.driver_id][VolumeMod](trackStat);
        trackStat.pcur_cmd = pPrevCmd;
    }
    else {
        if (attribsMask & TRIGGER_VOLUME) {
            // Set the volume on the track
            trackStat.volume_cntrl = pPlayAttribs->volume_cntrl;

            // Issue a sequencer command to to update the volume: change the track command stream temporarily also to do this
            uint8_t* const pPrevCmd = trackStat.pcur_cmd;

            uint8_t cmdBytes[8];
            cmdBytes[0] = VolumeMod;
            cmdBytes[1] = trackStat.volume_cntrl;

            trackStat.pcur_cmd = cmdBytes;
            gWess_CmdFuncArr[trackStat.driver_id][VolumeMod](trackStat);
            trackStat.pcur_cmd = pPrevCmd;
        }
        else if (attribsMask & TRIGGER_PAN) {
            // Set the pan on the track
            trackStat.pan_cntrl = pPlayAttribs->pan_cntrl;

            // Issue a sequencer command to to update the pan: change the track command stream temporarily also to do this
            uint8_t* const pPrevCmd = trackStat.pcur_cmd;

            uint8_t cmdBytes[8];
            cmdBytes[0] = PanMod;
            cmdBytes[1] = trackStat.pan_cntrl;

            trackStat.pcur_cmd = cmdBytes;
            gWess_CmdFuncArr[trackStat.driver_id][PanMod](trackStat);
            trackStat.pcur_cmd = pPrevCmd;
        }
    }

    if (attribsMask & TRIGGER_PATCH) {
        trackStat.patch_idx = pPlayAttribs->patch_idx;
    }

    if (attribsMask & TRIGGER_PITCH) {
        // Set the pitch on the track
        trackStat.pitch_cntrl = pPlayAttribs->pitch_cntrl;

        // Issue a sequencer command to update the patch: change the track command stream temporarily also to do this
        uint8_t* const pPrevCmd = trackStat.pcur_cmd;

        uint8_t cmdBytes[8];
        cmdBytes[0] = PitchMod;
        cmdBytes[1] = (uint8_t)((uint16_t) trackStat.pitch_cntrl >> 0);
        cmdBytes[2] = (uint8_t)((uint16_t) trackStat.pitch_cntrl >> 8);

        trackStat.pcur_cmd = cmdBytes;
        gWess_CmdFuncArr[trackStat.driver_id][PitchMod](trackStat);
        trackStat.pcur_cmd = pPrevCmd;
    }

    if (attribsMask & TRIGGER_MUTEMODE) {
        if (trackStat.mutegroups_mask & (1 << pPlayAttribs->mutegroup)) {
            trackStat.mute = true;
        } else {
            trackStat.mute = false;
        }
    }

    if (attribsMask & TRIGGER_TEMPO) {
        trackStat.tempo_qpm = pPlayAttribs->tempo_qpm;
        trackStat.tempo_ppi_frac = CalcPartsPerInt(GetIntsPerSec(), trackStat.tempo_ppq, trackStat.tempo_qpm);
    }

    if (attribsMask & TRIGGER_TIMED) {
        trackStat.end_abstime_qnp = trackStat.abstime_qnp + pPlayAttribs->playtime_qnp;
        trackStat.timed = true;
    }

    if (attribsMask & TRIGGER_LOOPED) {
        trackStat.looped = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger the specified sequence number and assign it the given type number
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_trigger_type(const int32_t seqIdx, const uintptr_t seqType) noexcept {
    master_status_structure& mstat = *gpWess_pm_stat;
    wess_seq_structrig(mstat.pmodule->psequences[seqIdx], seqIdx, seqType, false, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger the specified sequence number with custom play attributes and assign it the given type number
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_trigger_type_special(const int32_t seqIdx, const uintptr_t seqType, const TriggerPlayAttr* const pPlayAttribs) noexcept {
    master_status_structure& mstat = *gpWess_pm_stat;
    wess_seq_structrig(mstat.pmodule->psequences[seqIdx], seqIdx, seqType, false, pPlayAttribs);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update sequences of the specified type with the given play attributes
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_update_type_special(const uintptr_t seqType, const TriggerPlayAttr* const pPlayAttribs) noexcept {
    // Don't bother if there is no module loaded
    if (!Is_Module_Loaded())
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    gbWess_SeqOn = false;

    // Grab some basic info from the master status
    master_status_structure& mstat = *gpWess_pm_stat;

    const uint8_t maxSeqs = mstat.pmodule->hdr.max_active_sequences;
    const uint32_t maxTracksPerSeq = mstat.max_tracks_per_seq;
    uint8_t numActiveSeqsToVisit = mstat.num_active_seqs;

    if (numActiveSeqsToVisit > 0) {
        // Run through all of the active sequences of the specified type and update them all
        for (uint8_t seqStatIdx = 0; seqStatIdx < maxSeqs; ++seqStatIdx) {
            sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

            if (!seqStat.active)
                continue;

            if (seqStat.type == seqType) {
                // This is the sequence type we want: run through all of the active tracks in the sequence and update them all
                uint32_t numActiveTracksToVisit = seqStat.num_tracks_active;
                uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices;

                for (uint32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                    // Is this sequence track slot actually in use? Skip if not:
                    const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                    if (trackStatIdx == 0xFF)
                        continue;

                    // Only bother with this call if there are actually attributes to set.
                    // This check should have probably been done on function entry? This is somewhat wasteful doing it here:
                    if (pPlayAttribs) {
                        updatetrackstat(mstat.ptrack_stats[trackStatIdx], pPlayAttribs);
                    }

                    // If there are no more active tracks left to visit in the sequence then we are done
                    --numActiveTracksToVisit;

                    if (numActiveTracksToVisit == 0)
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
    gbWess_SeqOn = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop music sequences with the specified type number
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_stoptype(const uintptr_t seqType) noexcept {
    // If the module is not loaded then there is nothing to do
    if (!Is_Module_Loaded())
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    gbWess_SeqOn = false;

    // Grab some basic info from the master status
    master_status_structure& mstat = *gpWess_pm_stat;

    const uint8_t maxSeqs = mstat.pmodule->hdr.max_active_sequences;
    const uint32_t maxTracksPerSeq = mstat.max_tracks_per_seq;
    uint8_t numActiveSeqsToVisit = mstat.num_active_seqs;

    if (numActiveSeqsToVisit > 0) {
        // Run through all the sequences that are active looking for a sequence of a given type
        for (uint32_t seqStatIdx = 0; seqStatIdx < maxSeqs; ++seqStatIdx) {
            sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

            if (!seqStat.active)
                continue;

            if (seqStat.type == seqType) {
                // This is the sequence type we want to stop: run through all its active tracks and stop them all
                uint8_t numActiveTracksToVisit = seqStat.num_tracks_active;
                const uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices;

                for (uint8_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                    // Is this sequence track slot actually in use? Skip if not:
                    const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                    if (trackStatIdx == 0xFF)
                        continue;

                    // Call the driver function to turn off the track
                    track_status& trackStat = mstat.ptrack_stats[trackStatIdx];
                    gWess_CmdFuncArr[trackStat.driver_id][TrkOff](trackStat);

                    // If there are no more active tracks left to visit in the sequence then we are done
                    numActiveTracksToVisit--;

                    if (numActiveTracksToVisit == 0)
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
    gbWess_SeqOn = true;
}
