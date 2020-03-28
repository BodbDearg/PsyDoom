//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): master sequencer parameters manipulation.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessapi_m.h"

#include "PsxVm/PsxVm.h"
#include "PsxVm/VmSVal.h"
#include "wessapi.h"
#include "wessarc.h"
#include "wessseq.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the master volume for sound effects
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t wess_master_sfx_volume_get() noexcept {   
    return (Is_Module_Loaded()) ? *gWess_master_sfx_volume : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the master volume for music
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t wess_master_mus_volume_get() noexcept {
    return (Is_Module_Loaded()) ? *gWess_master_mus_volume : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the master volume for sound effects
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_master_sfx_vol_set(const uint8_t vol) noexcept {
    if (Is_Module_Loaded()) {
        *gWess_master_sfx_volume = vol;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the master music volume
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_master_mus_vol_set(const uint8_t musicVol) noexcept {
    // Don't bother if there is no module loaded
    if (!Is_Module_Loaded())
        return;

    // TODO: TEMP MEASURE FOR NOW - REMOVE EVENTUALLY: need a stack buffer in VM space for now
    struct Cmd {
        uint8_t bytes[8];
    };

    VmSVal<Cmd> cmd;
    uint8_t* const cmdBytes = cmd->bytes;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    *gbWess_SeqOn = false;

    // Update the master volume global
    *gWess_master_mus_volume = musicVol;

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

            // Run through all of the active tracks in the sequence and stop them all
            uint32_t numActiveSeqTracksLeft = seqStat.tracks_active;
            uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

            for (uint32_t i = 0; i < maxTracksPerSeq; ++i) {
                // Is this sequence track slot actually in use? Skip if not:
                const uint8_t trackIdx = pSeqTrackIndexes[i];

                if (trackIdx == 0xFF)
                    continue;

                // See if this track is music, if so then set its volume
                track_status& trackStat = mstat.ptrkstattbl[trackIdx];
                
                if (trackStat.sndclass == MUSIC_CLASS) {
                    // Issue a sequencer command to to update the volume levels: change the track command stream temporarily also to do this
                    uint8_t* const pPrevCmdBytes = trackStat.ppos.get();
                    trackStat.ppos = cmdBytes;

                    cmdBytes[0] = VolumeMod;
                    cmdBytes[1] = trackStat.volume_cntrl;
                    a0 = ptrToVmAddr(&trackStat);
                    gWess_CmdFuncArr[trackStat.patchtype][VolumeMod]();     // FIXME: convert to native function call

                    trackStat.ppos = pPrevCmdBytes;
                }

                // If there are no more tracks left active then we are done
                numActiveSeqTracksLeft--;

                if (numActiveSeqTracksLeft == 0)
                    break;
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Return whether panning is enabled
//------------------------------------------------------------------------------------------------------------------------------------------
PanMode wess_pan_mode_get() noexcept {
    return *gWess_pan_status;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set whether panning is enabled
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_pan_mode_set(const PanMode mode) noexcept {
    *gWess_pan_status = mode;
}
