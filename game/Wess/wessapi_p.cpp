//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): pausing and un-pausing sequences.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessapi_p.h"

#include "psxcmd.h"
#include "PsxVm/PsxVm.h"
#include "wessapi.h"
#include "wessarc.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpause the specified track in the given sequence
//------------------------------------------------------------------------------------------------------------------------------------------
static void trackstart(track_status& trackStat, sequence_status& seqStat) noexcept {
    if (trackStat.stopped) {
        trackStat.stopped = false;
        seqStat.tracks_playing += 1;

        if (seqStat.tracks_playing > 0) {
            seqStat.playmode = SEQ_STATE_PLAYING;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pause the specified track in the given sequence
//------------------------------------------------------------------------------------------------------------------------------------------
static void trackstop(track_status& trackStat, sequence_status& seqStat) noexcept {
    if (!trackStat.stopped) {
        trackStat.stopped = true;
        seqStat.tracks_playing -= 1;

        if (seqStat.tracks_playing == 0) {
            seqStat.playmode = SEQ_STATE_STOPPED;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pause the specified sequence and optionally mute it immediately
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_pause(const int32_t seqNum, const bool bMute) noexcept {
    // Don't bother if the sequence number is not valid
    if (!Is_Seq_Num_Valid(seqNum))
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
        // Run through all of the active sequences with the given number and stop them all
        for (uint8_t seqIdx = 0; seqIdx < maxSeqs; ++seqIdx) {
            sequence_status& seqStat = mstat.pseqstattbl[seqIdx];

            if (!seqStat.active)
                continue;

            // If this is the sequence number we are interested in then run through all of it's tracks and stop each of them
            if (seqStat.seq_num == seqNum) {
                uint32_t numActiveSeqTracksToVisit = seqStat.tracks_active;
                uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

                for (uint32_t i = 0; i < maxTracksPerSeq; ++i) {
                    // Is this sequence track slot actually in use? Skip if not:
                    const uint8_t trackIdx = pSeqTrackIndexes[i];

                    if (trackIdx == 0xFF)
                        continue;

                    // Pause the track
                    track_status& trackStat = mstat.ptrkstattbl[trackIdx];
                    trackstop(trackStat, seqStat);
                    
                    // If muting then call the driver function to mute the track
                    if (bMute) {
                        gWess_CmdFuncArr[trackStat.patchtype][TrkMute](trackStat);
                    }

                    // If there are no more tracks left active to visit then we are done
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
// Unpause the specified sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_restart(const int32_t seqNum) noexcept {
    // Don't bother if the sequence number is not valid
    if (!Is_Seq_Num_Valid(seqNum))
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
        // Run through all of the active sequences and stop them all
        for (uint8_t seqIdx = 0; seqIdx < maxSeqs; ++seqIdx) {
            sequence_status& seqStat = mstat.pseqstattbl[seqIdx];

            if (!seqStat.active)
                continue;
            
            // If this is the sequence number we are interested in then run through all of it's tracks and pause each of them
            if (seqStat.seq_num == seqNum) {
                uint32_t numActiveSeqTracksToVisit = seqStat.tracks_active;
                uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

                for (uint32_t i = 0; i < maxTracksPerSeq; ++i) {
                    // Is this sequence track slot actually in use? Skip if not:
                    const uint8_t trackIdx = pSeqTrackIndexes[i];

                    if (trackIdx == 0xFF)
                        continue;

                    // Unpause the track
                    trackstart(mstat.ptrkstattbl[trackIdx], seqStat);

                    // If there are no more tracks left active to visit then we are done
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
// Pause all currently playing sequences.
// If muting completely, the current note state can optionally be recorded to the given state struct for later restoration.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_pauseall(const bool bMute, NoteState* const pNoteState) noexcept {
    // Don't bother if there is no module loaded
    if (!Is_Module_Loaded())
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    *gbWess_SeqOn = false;
    
    // If muting temporarily, then save the state of all notes to the given state struct (if given).
    // This allows the notes to be restored to what they were previously.
    if (bMute) {
        start_record_music_mute(pNoteState);
    }

    // Grab some basic info from the master status
    master_status_structure& mstat = *gpWess_pm_stat->get();

    const uint8_t maxSeqs = mstat.pmod_info->mod_hdr.seq_work_areas;
    const uint32_t maxTracksPerSeq = mstat.max_trks_perseq;
    uint8_t numActiveSeqs = mstat.seqs_active;

    if (numActiveSeqs > 0) {
        // Run through all of the active sequences and pause them all
        for (uint8_t seqIdx = 0; seqIdx < maxSeqs; ++seqIdx) {
            sequence_status& seqStat = mstat.pseqstattbl[seqIdx];

            if (!seqStat.active)
                continue;

            // Run through all of the active tracks in the sequence and pause them all
            uint32_t numActiveSeqTracks = seqStat.tracks_active;
            uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

            for (uint32_t i = 0; i < maxTracksPerSeq; ++i) {
                // Is this sequence track slot actually in use? Skip if not:
                const uint8_t trackIdx = pSeqTrackIndexes[i];

                if (trackIdx == 0xFF)
                    continue;

                // Issue a driver command to mute the track (if required) and then pause the track
                track_status& trackStat = mstat.ptrkstattbl[trackIdx];
                
                if (bMute == 1) {
                    gWess_CmdFuncArr[trackStat.patchtype][TrkMute](trackStat);
                }

                trackstop(trackStat, seqStat);

                // If there are no more tracks left active then we are done
                numActiveSeqTracks--;

                if (numActiveSeqTracks == 0)
                    break;
            }
            
            // If there are no more active sequences to visit then we are done
            numActiveSeqs--;

            if (numActiveSeqs == 0)
                break;
        }
    }

    // Not recording any more note details while pausing
    if (bMute) {
        end_record_music_mute();
    }

    // Re-enable the sequencer
    *gbWess_SeqOn = true;
}


//------------------------------------------------------------------------------------------------------------------------------------------
// Restart all previously paused sequences.
// Individual notes can also be restored to their previous states using the saved note state struct.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_restartall(NoteState* const pPrevNoteState) noexcept {
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
        for (uint8_t seqIdx = 0; seqIdx < maxSeqs; ++seqIdx) {
            sequence_status& seqStat = mstat.pseqstattbl[seqIdx];

            if (!seqStat.active)
                continue;

            // Run through all of the active tracks in the sequence and restart them all
            uint32_t numSeqTracksActive = seqStat.tracks_active;
            uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

            for (uint32_t i = 0; i < maxTracksPerSeq; ++i) {
                // Is this sequence track slot actually in use? Skip if not:
                const uint8_t trackIdx = pSeqTrackIndexes[i];

                if (trackIdx == 0xFF)
                    continue;

                // Unpause this track
                track_status& trackStat = mstat.ptrkstattbl[trackIdx];
                trackstart(trackStat, seqStat);

                // If notes were previously saved for restoring then replay them now
                if (pPrevNoteState) {
                    for (int32_t noteIdx = 0; noteIdx < pPrevNoteState->numnotes; ++noteIdx) {
                        NoteData& noteData = pPrevNoteState->nd[noteIdx];

                        // Only restart the note if it is for this track and sequence
                        if ((noteData.track == trackIdx) && (noteData.seq_num == seqStat.seq_num)) {
                            PSX_voicenote(trackStat, *noteData.patchmap, *noteData.patchinfo, noteData.keynum, noteData.velnum);
                        }
                    }
                }

                // If there are no more tracks left active then we are done
                numSeqTracksActive--;

                if (numSeqTracksActive == 0)
                    break;
            }

            // If there are no more active sequences to visit then we are done
            numActiveSeqsToVisit--;

            if (numActiveSeqsToVisit == 0)
                break;
        }
    }
    
    // Clear the storage in the note state struct for re-use again
    if (pPrevNoteState) {
        pPrevNoteState->numnotes = 0;
    }

    // Re-enable the sequencer
    *gbWess_SeqOn = true;
}
