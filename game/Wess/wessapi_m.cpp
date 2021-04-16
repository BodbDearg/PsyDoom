//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): master sequencer parameters manipulation.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessapi_m.h"

#include "wessapi.h"
#include "wessseq.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the master volume for sound effects
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t wess_master_sfx_volume_get() noexcept {
    return (Is_Module_Loaded()) ? gWess_master_sfx_volume : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the master volume for music
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t wess_master_mus_volume_get() noexcept {
    return (Is_Module_Loaded()) ? gWess_master_mus_volume : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the master volume for sound effects
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_master_sfx_vol_set(const uint8_t vol) noexcept {
    if (Is_Module_Loaded()) {
        gWess_master_sfx_volume = vol;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the master music volume
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_master_mus_vol_set(const uint8_t musicVol) noexcept {
    // Don't bother if there is no module loaded
    if (!Is_Module_Loaded())
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    gbWess_SeqOn = false;

    // Update the master volume global
    gWess_master_mus_volume = musicVol;

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

            // Run through all of the active tracks in the sequence and stop them all
            uint32_t numActiveTracksToVisit = seqStat.num_tracks_active;
            uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices;

            for (uint32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                // Is this sequence track slot actually in use? Skip if not:
                const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                if (trackStatIdx == 0xFF)
                    continue;

                // See if this track is music, if so then set its volume
                track_status& trackStat = mstat.ptrack_stats[trackStatIdx];

                if (trackStat.sound_class == MUSIC_CLASS) {
                    // Issue a sequencer command to to update the volume levels: change the track command stream temporarily also to do this
                    uint8_t* const pPrevCmdBytes = trackStat.pcur_cmd;

                    uint8_t cmdBytes[8];
                    cmdBytes[0] = VolumeMod;
                    cmdBytes[1] = trackStat.volume_cntrl;

                    trackStat.pcur_cmd = cmdBytes;
                    gWess_CmdFuncArr[trackStat.driver_id][VolumeMod](trackStat);
                    trackStat.pcur_cmd = pPrevCmdBytes;
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

    // Re-enable the sequencer
    gbWess_SeqOn = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return whether panning is enabled
//------------------------------------------------------------------------------------------------------------------------------------------
PanMode wess_pan_mode_get() noexcept {
    return gWess_pan_status;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set whether panning is enabled
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_pan_mode_set(const PanMode mode) noexcept {
    gWess_pan_status = mode;
}
