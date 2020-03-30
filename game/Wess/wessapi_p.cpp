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
                        a0 = ptrToVmAddr(&trackStat);
                        gWess_CmdFuncArr[trackStat.patchtype][TrkMute]();       // FIXME: convert to native function call
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
                    a0 = ptrToVmAddr(&trackStat);
                    gWess_CmdFuncArr[trackStat.patchtype][TrkMute]();   // FIXME: convert to native function call
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

void queue_wess_seq_restartall() noexcept {
loc_80041C88:
    sp -= 0x58;
    sw(s5, sp + 0x44);
    s5 = a0;
    sw(ra, sp + 0x54);
    sw(fp, sp + 0x50);
    sw(s7, sp + 0x4C);
    sw(s6, sp + 0x48);
    sw(s4, sp + 0x40);
    sw(s3, sp + 0x3C);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    v0 = Is_Module_Loaded();
    if (v0 == 0) goto loc_80041E44;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    t1 = lbu(v0 + 0x4);
    sw(t1, sp + 0x18);
    v1 = lw(v0 + 0xC);
    fp = lw(v0 + 0x20);
    t0 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (t1 == 0) goto loc_80041E30;
    t0--;
    if (t0 == v0) goto loc_80041E30;
    s7 = fp + 2;
loc_80041D04:
    v0 = lw(fp);
    v0 &= 1;
    t1 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_80041E1C;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s6 = lbu(s7 + 0x2);
    s2 = lbu(v0 + 0x1C);
    s4 = lw(s7 + 0xA);
    s2--;
    if (s2 == t1) goto loc_80041E08;
loc_80041D38:
    a0 = lbu(s4);
    v0 = 0xFF;                                          // Result = 000000FF
    a1 = fp;
    if (a0 == v0) goto loc_80041DF8;
    v0 = a0 << 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += a0;
    v1 = lw(v1 + 0x28);
    v0 <<= 4;
    sw(t0, sp + 0x28);
    s3 = v0 + v1;
    a0 = s3;
    trackstart(*vmAddrToPtr<track_status>(a0), *vmAddrToPtr<sequence_status>(a1));
    t0 = lw(sp + 0x28);
    if (s5 == 0) goto loc_80041DEC;
    v0 = lw(s5);
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80041DEC;
    s0 = s5;
loc_80041D90:
    v1 = lbu(s4);
    v0 = lh(s0 + 0x6);
    s1++;
    if (v1 != v0) goto loc_80041DD8;
    v1 = lh(s7);
    v0 = lh(s0 + 0x4);
    a0 = s3;
    if (v1 != v0) goto loc_80041DD8;
    v0 = lbu(s0 + 0x9);
    a3 = lbu(s0 + 0x8);
    sw(v0, sp + 0x10);
    a1 = lw(s0 + 0xC);
    a2 = lw(s0 + 0x10);
    sw(t0, sp + 0x28);

    PSX_voicenote(
        *vmAddrToPtr<track_status>(a0),
        *vmAddrToPtr<patchmaps_header>(a1),
        *vmAddrToPtr<patchinfo_header>(a2),
        (uint8_t) a3,
        (uint8_t) lw(sp + 0x10)
    );

    t0 = lw(sp + 0x28);
loc_80041DD8:
    v0 = lw(s5);
    v0 = (i32(s1) < i32(v0));
    s0 += 0x10;
    if (v0 != 0) goto loc_80041D90;
loc_80041DEC:
    s6--;
    if (s6 == 0) goto loc_80041E08;
loc_80041DF8:
    s2--;
    t1 = -1;                                            // Result = FFFFFFFF
    s4++;
    if (s2 != t1) goto loc_80041D38;
loc_80041E08:
    t1 = lw(sp + 0x18);
    t1--;
    sw(t1, sp + 0x18);
    if (t1 == 0) goto loc_80041E30;
loc_80041E1C:
    s7 += 0x18;
    t0--;
    t1 = -1;                                            // Result = FFFFFFFF
    fp += 0x18;
    if (t0 != t1) goto loc_80041D04;
loc_80041E30:
    v0 = 1;                                             // Result = 00000001
    if (s5 == 0) goto loc_80041E3C;
    sw(0, s5);
loc_80041E3C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80041E44:
    ra = lw(sp + 0x54);
    fp = lw(sp + 0x50);
    s7 = lw(sp + 0x4C);
    s6 = lw(sp + 0x48);
    s5 = lw(sp + 0x44);
    s4 = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x58;
    return;
}
