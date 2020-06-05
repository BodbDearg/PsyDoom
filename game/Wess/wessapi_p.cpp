//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): pausing and un-pausing sequences.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessapi_p.h"

#include "psxcmd.h"
#include "wessapi.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpause the specified track in the given sequence
//------------------------------------------------------------------------------------------------------------------------------------------
static void trackstart(track_status& trackStat, sequence_status& seqStat) noexcept {
    if (trackStat.stopped) {
        trackStat.stopped = false;
        seqStat.num_tracks_playing += 1;

        if (seqStat.num_tracks_playing > 0) {
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
        seqStat.num_tracks_playing -= 1;

        if (seqStat.num_tracks_playing == 0) {
            seqStat.playmode = SEQ_STATE_STOPPED;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pause the specified sequence and optionally mute it immediately
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_pause(const int32_t seqIdx, const bool bMute) noexcept {
    // Don't bother if the sequence number is not valid
    if (!Is_Seq_Num_Valid(seqIdx))
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
        // Run through all of the active sequences with the given number and stop them all
        for (uint8_t seqStatIdx = 0; seqStatIdx < maxSeqs; ++seqStatIdx) {
            sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

            if (!seqStat.active)
                continue;

            // If this is the sequence number we are interested in then run through all of it's tracks and stop each of them
            if (seqStat.seq_idx == seqIdx) {
                uint32_t numActiveTracksToVisit = seqStat.num_tracks_active;
                uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices.get();

                for (uint32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                    // Is this sequence track slot actually in use? Skip if not:
                    const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                    if (trackStatIdx == 0xFF)
                        continue;

                    // Pause the track
                    track_status& trackStat = mstat.ptrack_stats[trackStatIdx];
                    trackstop(trackStat, seqStat);
                    
                    // If muting then call the driver function to mute the track
                    if (bMute) {
                        gWess_CmdFuncArr[trackStat.driver_id][TrkMute](trackStat);
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
// Unpause the specified sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_restart(const int32_t seqIdx) noexcept {
    // Don't bother if the sequence number is not valid
    if (!Is_Seq_Num_Valid(seqIdx))
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
        // Run through all of the active sequences and stop them all
        for (uint8_t seqStatIdx = 0; seqStatIdx < maxSeqs; ++seqStatIdx) {
            sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

            if (!seqStat.active)
                continue;
            
            // If this is the sequence number we are interested in then run through all of it's tracks and pause each of them
            if (seqStat.seq_idx == seqIdx) {
                uint32_t numActiveTracksToVisit = seqStat.num_tracks_active;
                uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices.get();

                for (uint32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                    // Is this sequence track slot actually in use? Skip if not:
                    const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                    if (trackStatIdx == 0xFF)
                        continue;

                    // Unpause the track
                    trackstart(mstat.ptrack_stats[trackStatIdx], seqStat);

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
// Pause all currently playing sequences.
// If muting completely, the current state for all voices can optionally be recorded to the given state struct for later restoration.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_pauseall(const bool bMute, SavedVoiceList* const pSavedVoices) noexcept {
    // Don't bother if there is no module loaded
    if (!Is_Module_Loaded())
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    gbWess_SeqOn = false;
    
    // If muting temporarily, then save the state of all voices to the given state struct (if given).
    // This allows the voices to be restored to what they were previously.
    if (bMute) {
        start_record_music_mute(pSavedVoices);
    }

    // Grab some basic info from the master status
    master_status_structure& mstat = *gpWess_pm_stat;

    const uint8_t maxSeqs = mstat.pmodule->hdr.max_active_sequences;
    const uint32_t maxTracksPerSeq = mstat.max_tracks_per_seq;
    uint8_t numActiveSeqsToVisit = mstat.num_active_seqs;

    if (numActiveSeqsToVisit > 0) {
        // Run through all of the active sequences and pause them all
        for (uint8_t seqStatIdx = 0; seqStatIdx < maxSeqs; ++seqStatIdx) {
            sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

            if (!seqStat.active)
                continue;

            // Run through all of the active tracks in the sequence and pause them all
            uint32_t numActiveTracksToVisit = seqStat.num_tracks_active;
            uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices.get();

            for (uint32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                // Is this sequence track slot actually in use? Skip if not:
                const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                if (trackStatIdx == 0xFF)
                    continue;

                // Issue a driver command to mute the track (if required) and then pause the track
                track_status& trackStat = mstat.ptrack_stats[trackStatIdx];
                
                if (bMute == 1) {
                    gWess_CmdFuncArr[trackStat.driver_id][TrkMute](trackStat);
                }

                trackstop(trackStat, seqStat);

                // If there are no more active tracks left to visit in the sequence then we are done
                numActiveTracksToVisit--;

                if (numActiveTracksToVisit == 0)
                    break;
            }
            
            // If there are no more active sequences to visit then we are done
            numActiveSeqsToVisit--;

            if (numActiveSeqsToVisit == 0)
                break;
        }
    }

    // Not recording any more voice details while pausing
    if (bMute) {
        end_record_music_mute();
    }

    // Re-enable the sequencer
    gbWess_SeqOn = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Restart all previously paused sequences.
// Individual voices can also be restored to their previous states using the saved voice state struct.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_restartall(SavedVoiceList* const pSavedVoices) noexcept {
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
        for (uint8_t seqStatIdx = 0; seqStatIdx < maxSeqs; ++seqStatIdx) {
            sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

            if (!seqStat.active)
                continue;

            // Run through all of the active tracks in the sequence and restart them all
            uint32_t numActiveTracksToVisit = seqStat.num_tracks_active;
            uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices.get();

            for (uint32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                // Is this sequence track slot actually in use? Skip if not:
                const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                if (trackStatIdx == 0xFF)
                    continue;

                // Unpause this track
                track_status& trackStat = mstat.ptrack_stats[trackStatIdx];
                trackstart(trackStat, seqStat);

                // If voices were previously saved for restoring then replay them now
                if (pSavedVoices) {
                    for (int32_t noteIdx = 0; noteIdx < pSavedVoices->size; ++noteIdx) {
                        SavedVoice& voice = pSavedVoices->voices[noteIdx];

                        // Only restart the voice if it is for this track and sequence
                        if ((voice.trackstat_idx == trackStatIdx) && (voice.seq_idx == seqStat.seq_idx)) {
                            PSX_voicenote(trackStat, *voice.ppatch_voice, *voice.ppatch_sample, voice.note, voice.volume);
                        }
                    }
                }

                // If there are no more active tracks left to visit in the sequence then we are done
                numActiveTracksToVisit--;

                if (numActiveTracksToVisit == 0)
                    break;
            }

            // If there are no more active sequences to visit then we are done
            numActiveSeqsToVisit--;

            if (numActiveSeqsToVisit == 0)
                break;
        }
    }
    
    // Clear storage in the list of saved voices for re-use again
    if (pSavedVoices) {
        pSavedVoices->size = 0;
    }

    // Re-enable the sequencer
    gbWess_SeqOn = true;
}
