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

void queue_wess_seq_update_type_special() noexcept {
    sp -= 0x40;
    sw(fp, sp + 0x38);
    fp = a1;
    sw(ra, sp + 0x3C);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    sw(a0, sp + 0x10);
    v0 = Is_Module_Loaded();
    if (v0 == 0) goto loc_80044578;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_8004456C;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_8004456C;
    s3 = s5 + 0xC;
loc_800444A8:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80044554;
    v0 = lw(s3 - 0x4);
    a2 = lw(sp + 0x10);
    if (v0 != a2) goto loc_80044544;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s1 = lw(s3);
    s0 = lbu(v0 + 0x1C);
    v0 = -1;                                            // Result = FFFFFFFF
    s0--;
    if (s0 == v0) goto loc_80044544;
    s7 = -1;                                            // Result = FFFFFFFF
loc_800444F8:
    a0 = lbu(s1);
    a2 = 0xFF;                                          // Result = 000000FF
    v0 = a0 << 2;
    if (a0 == a2) goto loc_80044538;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += a0;
    v1 = lw(v1 + 0x28);
    v0 <<= 4;
    a0 = v0 + v1;
    if (fp == 0) goto loc_8004452C;
    a1 = fp;
    updatetrackstat(*vmAddrToPtr<track_status>(a0), vmAddrToPtr<TriggerPlayAttr>(a1));
loc_8004452C:
    s2--;
    if (s2 == 0) goto loc_80044544;
loc_80044538:
    s0--;
    s1++;
    if (s0 != s7) goto loc_800444F8;
loc_80044544:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80044570;
    }
loc_80044554:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_800444A8;
loc_8004456C:
    v0 = 1;                                             // Result = 00000001
loc_80044570:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80044578:
    ra = lw(sp + 0x3C);
    fp = lw(sp + 0x38);
    s7 = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x40;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop music sequences with the specified type number
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_stoptype(const uint32_t seqType) noexcept {
    // If the module is not loaded then there is nothing to do
    if (!Is_Module_Loaded())
        return;

    // Disable sequencer timer interrupts temporarily while we stop tracks
    *gbWess_SeqOn = false;

    // Grab some basic stats from the master status
    master_status_structure& mstat = *gpWess_pm_stat->get();

    const uint8_t maxSeqs = mstat.pmod_info->mod_hdr.seq_work_areas;
    const uint32_t maxTracks = mstat.max_trks_perseq;

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

                for (uint8_t i = 0; i < maxTracks; ++i) {
                    // Is this sequence track not active?
                    const uint8_t trackIdx = pSeqTrackIndexes[i];

                    if (trackIdx == 0xFF)
                        continue;

                    // Stop the track
                    track_status& trackStat = mstat.ptrkstattbl[trackIdx];
                    a0 = ptrToVmAddr(&trackStat);
                    gWess_CmdFuncArr[trackStat.patchtype][TrkOff]();    // FIXME: convert to native function call

                    // If there are no more active tracks left then we are done
                    numActiveTracks--;

                    if (numActiveTracks == 0)
                        break;
                }
            }
            
            // If there are no more active sequences left to visit then we can bail early
            numActiveSeqsToVisit--;

            if (numActiveSeqsToVisit == 0)
                break;
        }
    }

    // Re-enable the sequencer timer
    *gbWess_SeqOn = true;
}
