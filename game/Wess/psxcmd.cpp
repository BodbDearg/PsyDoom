#include "psxcmd.h"

#include "psxspu.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBSPU.h"
#include "wessapi.h"
#include "wessarc.h"
#include "wessseq.h"

// TODO: REMOVE ALL OF THESE
void _thunk_PSX_DriverInit() noexcept { PSX_DriverInit(*vmAddrToPtr<master_status_structure>(a0)); }
void _thunk_PSX_DriverExit() noexcept { PSX_DriverExit(*vmAddrToPtr<master_status_structure>(a0)); }
void _thunk_PSX_DriverEntry1() noexcept { PSX_DriverEntry1(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_DriverEntry2() noexcept { PSX_DriverEntry2(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_DriverEntry3() noexcept { PSX_DriverEntry3(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_TrkOff() noexcept { PSX_TrkOff(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_TrkMute() noexcept { PSX_TrkMute(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_PatchMod() noexcept { PSX_PatchMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_ZeroMod() noexcept { PSX_ZeroMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_ModuMod() noexcept { PSX_ModuMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_VolumeMod() noexcept { PSX_VolumeMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_PanMod() noexcept { PSX_PanMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_PedalMod() noexcept { PSX_PedalMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_ReverbMod() noexcept { PSX_ReverbMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_ChorusMod() noexcept { PSX_ChorusMod(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_NoteOn() noexcept { PSX_NoteOn(*vmAddrToPtr<track_status>(a0)); }
void _thunk_PSX_NoteOff() noexcept { PSX_NoteOff(*vmAddrToPtr<track_status>(a0)); }

void (* const gWess_drv_cmds[19])() = {
    _thunk_PSX_DriverInit,      // 00
    _thunk_PSX_DriverExit,      // 01
    _thunk_PSX_DriverEntry1,    // 02
    _thunk_PSX_DriverEntry2,    // 03
    _thunk_PSX_DriverEntry3,    // 04
    _thunk_PSX_TrkOff,          // 05
    _thunk_PSX_TrkMute,         // 06
    PSX_PatchChg,               // 07
    _thunk_PSX_PatchMod,        // 08
    PSX_PitchMod,               // 09
    _thunk_PSX_ZeroMod,         // 10
    _thunk_PSX_ModuMod,         // 11
    _thunk_PSX_VolumeMod,       // 12
    _thunk_PSX_PanMod,          // 13
    _thunk_PSX_PedalMod,        // 14
    _thunk_PSX_ReverbMod,       // 15
    _thunk_PSX_ChorusMod,       // 16
    _thunk_PSX_NoteOn,          // 17
    _thunk_PSX_NoteOff          // 18
};

// TODO: COMMENT
static constexpr uint8_t VOICE_RELEASE_RATE = 13;

static const VmPtr<VmPtr<master_status_structure>>      gpWess_drv_mstat(0x8007F164);               // Pointer to the master status structure being used by the sequencer
static const VmPtr<VmPtr<sequence_status>>              gpWess_drv_seqStats(0x8007F168);            // TODO: COMMENT
static const VmPtr<VmPtr<track_status>>                 gpWess_drv_trackStats(0x8007F16C);          // Pointer to the array of track statuses for all tracks
static const VmPtr<VmPtr<voice_status>>                 gpWess_drv_voiceStats(0x8007F0B8);          // TODO: COMMENT
static const VmPtr<VmPtr<voice_status>>                 gpWess_drv_psxVoiceStats(0x8007F170);       // TODO: COMMENT
static const VmPtr<VmPtr<patch_group_data>>             gpWess_drv_patchGrpInfo(0x8007F178);        // TODO: COMMENT
static const VmPtr<VmPtr<patches_header>>               gpWess_drv_patchHeaders(0x8007F180);        // TODO: COMMENT
static const VmPtr<VmPtr<patchmaps_header>>             gpWess_drv_patchmaps(0x8007F184);           // TODO: COMMENT
static const VmPtr<VmPtr<patchinfo_header>>             gpWess_drv_patchInfos(0x8007F188);          // TODO: COMMENT
static const VmPtr<VmPtr<drumpmaps_header>>             gpWess_drv_drummaps(0x8007F18C);            // TODO: COMMENT
static const VmPtr<uint8_t>                             gWess_drv_numPatchTypes(0x8007F0B0);        // TODO: COMMENT
static const VmPtr<uint8_t>                             gWess_drv_totalVoices(0x8007F0AC);          // TODO: COMMENT
static const VmPtr<uint32_t>                            gWess_drv_hwVoiceLimit(0x8007F174);         // TODO: COMMENT
static const VmPtr<VmPtr<uint32_t>>                     gpWess_drv_curabstime(0x8007F17C);          // Pointer to the current absolute time for the sequencer system: TODO: COMMENT on what the time value is
static const VmPtr<uint8_t[SPU_NUM_VOICES]>             gWess_drv_chanReverbAmt(0x8007F1E8);        // Current reverb levels for each channel/voice
static const VmPtr<uint32_t>                            gWess_drv_releasedVoices(0x80075A08);       // TODO: COMMENT
static const VmPtr<uint32_t>                            gWess_drv_mutedVoices(0x80075A0C);          // TODO: COMMENT
static const VmPtr<SpuVoiceAttr>                        gWess_spuVoiceAttr(0x8007F190);             // Temporary used for setting voice parameters with LIBSPU
static const VmPtr<uint8_t[SPU_NUM_VOICES]>             gWess_spuKeyStatuses(0x8007F1D0);           // TODO: COMMENT

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the location where we will start recording note/voice details before temporarily muting
//------------------------------------------------------------------------------------------------------------------------------------------
void start_record_music_mute(NoteState* const pNoteState) noexcept {
    *gpWess_notestate = pNoteState;

    if (pNoteState) {
        pNoteState->numnotes = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears the location where we will record note/voice details while temporarily muting
//------------------------------------------------------------------------------------------------------------------------------------------
void end_record_music_mute() noexcept {
    *gpWess_notestate = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Records details about a playing note for later un-pausing
//------------------------------------------------------------------------------------------------------------------------------------------
void add_music_mute_note(
    const int16_t seqNum,
    const int16_t track,
    const uint8_t note,
    const uint8_t noteVol,
    const patchmaps_header& patchmap,
    const patchinfo_header& patchInfo
) noexcept {
    NoteState* const pNoteState = gpWess_notestate->get();

    if (pNoteState) {
        // PC-PSX: sanity check we haven't exceeded the bounds of the saved notes array.
        // This shouldn't be called more times than there are hardware voices available!
        ASSERT(pNoteState->numnotes < SPU_NUM_VOICES);

        NoteData& noteData = pNoteState->nd[pNoteState->numnotes];
        noteData.seq_num = seqNum;
        noteData.track = track;
        noteData.keynum = note;
        noteData.velnum = noteVol;
        noteData.patchmap = &patchmap;
        noteData.patchinfo = &patchInfo;

        pNoteState->numnotes++;
    }
}

void PSX_UNKNOWN_DrvFunc() noexcept {
    v1 = 0x10000000;                                    // Result = 10000000
    v0 = 0x1F;                                          // Result = 0000001F
    goto loc_80045AF0;
loc_80045AD8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A07);                              // Load from: gWess_UNKNOWN_status_byte (80075A07)
    {
        const bool bJump = (v0 == 0);
        v0 += 0xFF;
        if (bJump) goto loc_80045B04;
    }
    v1 = u32(i32(v1) >> 1);
loc_80045AF0:
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x5A07);                                // Store to: gWess_UNKNOWN_status_byte (80075A07)
    v0 = (i32(a0) < i32(v1));
    if (v0 != 0) goto loc_80045AD8;
loc_80045B04:
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger a sound to play with the specified volume and using the information in the voice status struct.
// Note: the note setting was apparently ignored by this function? Instead it is sourced from the voice status.
//------------------------------------------------------------------------------------------------------------------------------------------
void TriggerPSXVoice(const voice_status& voiceStat, [[maybe_unused]] const uint8_t voiceNote, const uint8_t voiceVol) noexcept {
    // Get the track status and LIBSPU struct we use to setup the voice
    SpuVoiceAttr& spuVoiceAttr = *gWess_spuVoiceAttr;
    track_status& trackStat = gpWess_drv_trackStats->get()[voiceStat.track];

    // These are the attributes we will set
    spuVoiceAttr.attr_mask = (
        SPU_VOICE_VOLL | SPU_VOICE_VOLR |
        SPU_VOICE_NOTE | SPU_VOICE_SAMPLE_NOTE |
        SPU_VOICE_WDSA |
        SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2
    );

    // This is the voice we are manipulating
    spuVoiceAttr.voice_bits = 1 << (voiceStat.refindx % 32);

    // Setup reverb
    if (trackStat.reverb == 0) {
        if (gWess_drv_chanReverbAmt[voiceStat.refindx] != 0) {
            LIBSPU_SpuSetReverbVoice(0, spuVoiceAttr.voice_bits);
            gWess_drv_chanReverbAmt[voiceStat.refindx] = 0;
        }
    } else {
        if (gWess_drv_chanReverbAmt[voiceStat.refindx] == 0) {
            LIBSPU_SpuSetReverbVoice(1, spuVoiceAttr.voice_bits);
            gWess_drv_chanReverbAmt[voiceStat.refindx] = 127;
        }
    }

    // Figure out pan amount
    int16_t triggerPan;

    if (*gWess_pan_status == PAN_OFF) {
        triggerPan = 64;
    } else {
        triggerPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.patchmaps->pan - 64;
        
        if (triggerPan > 127) {
            triggerPan = 127;
        }

        if (triggerPan < 0) {
            triggerPan = 0;
        }
    }

    // Figure out the trigger volume for the note (0-2047)
    uint32_t triggerVol = voiceVol;
    triggerVol *= voiceStat.patchmaps->volume;
    triggerVol *= trackStat.volume_cntrl;

    if (trackStat.sndclass == SNDFX_CLASS) {
        triggerVol *= (*gWess_master_sfx_volume);
    } else {
        triggerVol *= (*gWess_master_mus_volume);
    }

    triggerVol >>= 21;
    
    // Set volume using trigger volume and pan
    if (*gWess_pan_status == PAN_OFF) {
        const uint16_t spuVol = (int16_t) triggerVol * 64;
        spuVoiceAttr.volume.left = spuVol;
        spuVoiceAttr.volume.right = spuVol;
    } else {
        const int16_t spuVolL = (int16_t)(((int32_t) triggerVol * 128 * (128 - triggerPan)) / 128);
        const int16_t spuVolR = (int16_t)(((int32_t) triggerVol * 128 * (triggerPan + 1  )) / 128);

        if (*gWess_pan_status == PAN_ON) {
            spuVoiceAttr.volume.left = spuVolL;
            spuVoiceAttr.volume.right = spuVolR;
        } else {
            // PAN_ON_REV: reverse the channels when panning
            spuVoiceAttr.volume.left = spuVolR;
            spuVoiceAttr.volume.right = spuVolL;
        }
    }
    
    // Set the note to play and the base note
    if (trackStat.pitch_cntrl == 0) {
        // Not doing any pitch shifting
        spuVoiceAttr.note = (uint16_t) voiceStat.keynum << 8;
    } else {
        if (trackStat.pitch_cntrl >= 1) {
            // Pitch shifting: up
            const uint32_t pitchShiftFrac = trackStat.pitch_cntrl * voiceStat.patchmaps->pitchstep_max;
            const uint16_t pitchShiftNote = (uint16_t)((32 + pitchShiftFrac) >> 13);
            const uint16_t pitchShiftFine = (pitchShiftFrac & 0x1FFF) >> 6;

            // Possible bug? Should the fine mask here be '0xFF' instead?
            // This code is never triggered from what I can see, so maybe it does not matter... 
            spuVoiceAttr.note = (voiceStat.keynum + pitchShiftNote) << 8;
            spuVoiceAttr.note |= pitchShiftFine & 0x7F;
        }
        else {
            // Pitch shifting: down
            const uint32_t pitchShiftFrac = trackStat.pitch_cntrl * voiceStat.patchmaps->pitchstep_min;
            const uint16_t pitchShiftNote = (uint16_t)(((32 - pitchShiftFrac) >> 13) + 1);
            const uint16_t pitchShiftFine = 128 - ((pitchShiftFrac & 0x1FFF) >> 6);

            // Possible bug? Should the fine mask here be '0xFF' instead?
            // This code is never triggered from what I can see, so maybe it does not matter...
            spuVoiceAttr.note = (voiceStat.keynum - pitchShiftNote) << 8;
            spuVoiceAttr.note |= pitchShiftFine & 0x7F;
        }
    }

    spuVoiceAttr.sample_note = ((uint16_t) voiceStat.patchmaps->root_key << 8) | voiceStat.patchmaps->fine_adj;

    // Set the SPU address of the sound data
    spuVoiceAttr.addr = voiceStat.patchinfo->sample_pos;

    // Setup volume envelope
    spuVoiceAttr.adsr1 = voiceStat.patchmaps->adsr1;
    spuVoiceAttr.adsr2 = voiceStat.patchmaps->adsr2;

    // Trigger the note!
    LIBSPU_SpuSetKeyOnWithAttr(spuVoiceAttr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does initialization for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_DriverInit(master_status_structure& mstat) noexcept {
    // Save pointers to various master status fields and stats
    const uint8_t numPatchTypes = mstat.pmod_info->mod_hdr.patch_types_infile;

    *gpWess_drv_mstat = &mstat;
    *gpWess_drv_seqStats = mstat.pseqstattbl;
    *gpWess_drv_trackStats = mstat.ptrkstattbl;
    *gpWess_drv_voiceStats = mstat.pvoicestattbl;
    *gWess_drv_numPatchTypes = numPatchTypes;
    *gWess_drv_totalVoices = mstat.voices_total;
    *gpWess_drv_curabstime = mstat.pabstime;
    
    // Determine the start of the PSX hardware voices in the voices list
    *gpWess_drv_psxVoiceStats = gpWess_drv_voiceStats->get();

    for (uint8_t voiceIdx = 0; voiceIdx < mstat.voices_total; ++voiceIdx) {
        voice_status& voiceStat = gpWess_drv_voiceStats->get()[voiceIdx];

        if (voiceStat.patchtype == PSX_ID) {
            *gpWess_drv_psxVoiceStats = &voiceStat;
            break;
        }
    }

    // Determine the patch group info for the PSX hardware driver
    *gpWess_drv_patchGrpInfo = nullptr;

    for (uint8_t patchGrpIdx = 0; patchGrpIdx < numPatchTypes; ++patchGrpIdx) {
        patch_group_data& patchGrpInfo = mstat.ppat_info[patchGrpIdx];

        if (patchGrpInfo.pat_grp_hdr.patch_id == PSX_ID) {
            *gpWess_drv_patchGrpInfo = &patchGrpInfo;
            break;
        }
    }

    // Save various bits of patch group information and pointers for the PSX patch group
    patch_group_data& patchGrp = *gpWess_drv_patchGrpInfo->get();
    *gWess_drv_hwVoiceLimit = patchGrp.pat_grp_hdr.hw_voice_limit;

    {
        uint8_t* pPatchData = patchGrp.ppat_data.get();

        *gpWess_drv_patchHeaders = (patches_header*) pPatchData;
        pPatchData += sizeof(patches_header) * patchGrp.pat_grp_hdr.patches;

        *gpWess_drv_patchmaps = (patchmaps_header*) pPatchData;
        pPatchData += sizeof(patchmaps_header) * patchGrp.pat_grp_hdr.patchmaps;

        *gpWess_drv_patchInfos = (patchinfo_header*) pPatchData;
        pPatchData += sizeof(patchinfo_header) * patchGrp.pat_grp_hdr.patchinfo;

        *gpWess_drv_drummaps = (drumpmaps_header*) pPatchData;
    }
    
    // Do low level SPU initialization
    psxspu_init();

    // Init reverb levels for all channels
    for (int32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
        gWess_drv_chanReverbAmt[voiceIdx] = 127;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shut down the PlayStation sound driver.
// I don't think there's any way for this to be called in the retail game...
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_DriverExit([[maybe_unused]] master_status_structure& mstat) noexcept {
    LIBSPU_SpuQuit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update function for the PSX sound driver: invoked for each hardware timer interrupt updating the sequencer system.
// Releases or mutes voices that were requested to be released/muted and frees up voices that are no longer in use by the SPU.
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_DriverEntry1([[maybe_unused]] track_status& trackStat) noexcept {
    // Grab the list of available hardware voice statuses and how many voices are available
    const uint32_t maxHwVoices = *gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats->get();

    // Stop all currently active voices that are releasing and past their end time for release
    {
        // Grab some stuff needed in the loop
        master_status_structure& mstat = *gpWess_drv_mstat->get();

        uint8_t numActiveVoicesToVisit = mstat.voices_active;
        const uint32_t curAbsTime = *gpWess_drv_curabstime->get();

        // Turn off all applicable voices
        if (numActiveVoicesToVisit > 0) {
            for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < maxHwVoices; ++hwVoiceIdx) {
                voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];
                
                // Only stop the voice if active, releasing and if the time is past when it should be released
                if (voiceStat.active && voiceStat.release && (curAbsTime > voiceStat.pabstime)) {
                    PSX_voiceparmoff(voiceStat);

                    // If there are no active more voices left active then we are done
                    numActiveVoicesToVisit--;

                    if (numActiveVoicesToVisit == 0)
                        break;
                }
            }
        }
    }
    
    // Key off any voices that were requested to be released or muted
    {
        // Figure out what voices to key off (turn off)
        uint32_t voicesToTurnOff = 0;
    
        if (*gWess_drv_releasedVoices != 0) {
            // Some voices are requested to be released (key off) - incorporate those and clear the command
            voicesToTurnOff = *gWess_drv_releasedVoices;
            *gWess_drv_releasedVoices = 0;
        }

        if (*gWess_drv_mutedVoices != 0) {
            // Some voices are requested to be muted, quickly ramp those down exponetially.
            // This code here ensures that they fade out rapidly.
            SpuVoiceAttr& spuVoiceAttr = *gWess_spuVoiceAttr;

            spuVoiceAttr.voice_bits = *gWess_drv_mutedVoices;
            spuVoiceAttr.attr_mask = SPU_VOICE_ADSR_RMODE | SPU_VOICE_ADSR_RR;
            spuVoiceAttr.r_mode = SPU_VOICE_EXPDec;
            spuVoiceAttr.rr = VOICE_RELEASE_RATE;
            LIBSPU_SpuSetVoiceAttr(spuVoiceAttr);

            // Include these voices in the voices that will be keyed off and clear the command
            voicesToTurnOff |= *gWess_drv_mutedVoices;
            *gWess_drv_mutedVoices = 0;
        }
    
        // Key off the voices requested to be keyed off
        if (voicesToTurnOff != 0) {
            LIBSPU_SpuSetKey(0, voicesToTurnOff);
        }
    }
    
    // Release any voices that the SPU says are now off
    LIBSPU_SpuGetAllKeysStatus(gWess_spuKeyStatuses.get());

    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < maxHwVoices; ++hwVoiceIdx) {
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        if (voiceStat.active && (gWess_spuKeyStatuses[hwVoiceIdx] == SPU_OFF)) {
            PSX_voiceparmoff(voiceStat);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unused/implemented driver update function.
// Doesn't seem to be called for the PSX sound driver.
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_DriverEntry2([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unused/implemented driver update function.
// Doesn't seem to be called for the PSX sound driver.
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_DriverEntry3([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops the given track
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_TrkOff(track_status& trackStat) noexcept {
    // Mute the track
    PSX_TrkMute(trackStat);

    if (trackStat.voices_active == 0) {
        // If there are no voices left active in the track then stop in the sequencer
        Eng_TrkOff(trackStat);
    } else {
        // Otherwise decrement the number of sequence tracks playing and update state
        trackStat.off = true;
        trackStat.stopped = true;

        sequence_status& seqStat = gpWess_drv_seqStats->get()[trackStat.seq_owner];
        seqStat.tracks_playing--;

        if (seqStat.tracks_playing == 0) {
            seqStat.playmode = SEQ_STATE_STOPPED;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Command that mutes the given track
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_TrkMute(track_status& trackStat) noexcept {    
    // If there are no voices active in the track then there is nothing to do
    uint32_t numActiveTrackVoices = trackStat.voices_active;

    if (numActiveTrackVoices == 0)
        return;

    // Run through all PSX hardware voices and mute the ones belonging to this track
    uint32_t numHwVoices = *gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoices = gpWess_drv_psxVoiceStats->get();

    for (uint32_t voiceIdx = 0; voiceIdx < numHwVoices; ++voiceIdx) {
        voice_status& voiceStat = pHwVoices[voiceIdx];

        // Only mute this voice if active and belonging to this track, ignore otherwise
        if ((!voiceStat.active) || (voiceStat.track != trackStat.refindx))
            continue;
        
        // If the voice is not being killed, is a music voice and we are recording note state then save for later un-pause
        if (gpWess_notestate->get() && (!voiceStat.release) && (trackStat.sndclass == MUSIC_CLASS)) {
            sequence_status& seqStat = gpWess_drv_seqStats->get()[trackStat.seq_owner];

            add_music_mute_note(
                seqStat.seq_num,
                voiceStat.track,
                voiceStat.keynum,
                voiceStat.velnum,
                *voiceStat.patchmaps,
                *voiceStat.patchinfo
            );
        }

        // Begin releasing the voice
        voiceStat.adsr2 = 0x10000000 >> (31 - (VOICE_RELEASE_RATE % 32));   // TODO: what is this doing here?
        PSX_voicerelease(voiceStat);
        *gWess_drv_mutedVoices |= 1 << (voiceStat.refindx % 32);

        // If there are no more active voices in the track then we are done
        numActiveTrackVoices--;

        if (numActiveTrackVoices == 0)
            return;
    }
}

void PSX_PatchChg() noexcept {
loc_800466FC:
    v1 = lw(a0 + 0x34);
    v0 = lbu(v1 + 0x2);
    v1 = lbu(v1 + 0x1);
    v0 <<= 8;
    v1 |= v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0xF18);                                 // Store to: gWess_Dvr_thepatch (8007F0E8)
    sh(v1, a0 + 0xA);
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unimplemented mod/effect for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_PatchMod([[maybe_unused]] track_status& trackStat) noexcept {}

void PSX_PitchMod() noexcept {
loc_8004672C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    a0 = v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEFC);                                 // Store to: 8007F104
    v0 <<= 16;
    v1 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    if (v1 == v0) goto loc_80046960;
    v0 = lbu(s0 + 0x10);
    sh(a0, s0 + 0xE);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF14);                                 // Store to: 8007F0EC
    if (v0 == 0) goto loc_80046960;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF00);                                 // Store to: 8007F100
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF10);                                 // Store to: 8007F0F0
    s2 = 0x20;                                          // Result = 00000020
    if (v1 == v0) goto loc_80046960;
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0xE70;                                        // Result = 8007F190
loc_800467CC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF00);                                // Load from: 8007F100
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_8004692C;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    {
        const bool bJump = (v1 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8004692C;
    }
    v1 = lbu(a0 + 0x2);
    sw(s2, s1 + 0x4);                                   // Store to: 8007F194
    v0 = v0 << v1;
    sw(v0, s1);                                         // Store to: 8007F190
    v1 = lh(s0 + 0xE);
    if (v1 != 0) goto loc_8004682C;
    v0 = lbu(a0 + 0x5);
    v0 <<= 8;
    goto loc_800468FC;
loc_8004682C:
    if (i32(v1) <= 0) goto loc_80046888;
    v0 = lw(a0 + 0x8);
    v0 = lb(v0 + 0x9);
    mult(v1, v0);
    v0 = lo;
    v0 += 0x20;
    v1 = u32(i32(v0) >> 13);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF0C);                                 // Store to: 8007F0F4
    v0 &= 0x1FFF;
    v0 = u32(i32(v0) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF08);                                 // Store to: 8007F0F8
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF04);                                 // Store to: 8007F0FC
    v0 = lbu(a0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF08);                               // Load from: 8007F0F8
    v0 += v1;
    goto loc_800468E8;
loc_80046888:
    v0 = lw(a0 + 0x8);
    v0 = lb(v0 + 0x8);
    mult(v1, v0);
    v1 = lo;
    v1 = s2 - v1;
    v0 = u32(i32(v1) >> 13);
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF0C);                                 // Store to: 8007F0F4
    v1 &= 0x1FFF;
    v1 = u32(i32(v1) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF08);                                 // Store to: 8007F0F8
    v0 = 0x80;                                          // Result = 00000080
    v0 -= v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF04);                                 // Store to: 8007F0FC
    v0 = lbu(a0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF08);                               // Load from: 8007F0F8
    v0 -= v1;
loc_800468E8:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF04);                               // Load from: 8007F0FC
    v0 <<= 8;
    v1 &= 0x7F;
    v0 |= v1;
loc_800468FC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    sh(v0, s1 + 0x16);                                  // Store to: 8007F1A6
    LIBSPU_SpuSetVoiceAttr(*vmAddrToPtr<SpuVoiceAttr>(a0));
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF14);                                // Load from: 8007F0EC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF14);                                 // Store to: 8007F0EC
    if (v0 == 0) goto loc_80046960;
loc_8004692C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF00);                                // Load from: 8007F100
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF10);                                // Load from: 8007F0F0
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF00);                                 // Store to: 8007F100
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF10);                                 // Store to: 8007F0F0
    if (v1 != v0) goto loc_800467CC;
loc_80046960:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unimplemented mod/effect for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_ZeroMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unimplemented mod/effect for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_ModuMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer command that updates the volume level for a track and all of it's active voices
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_VolumeMod(track_status& trackStat) noexcept {
    // Cache these for the loop
    SpuVoiceAttr& spuVoiceAttr = *gWess_spuVoiceAttr;
    const uint32_t numHwVoices = *gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats->get();

    // Update the track volume with the amount in the sequencer command
    trackStat.volume_cntrl = trackStat.ppos[1];

    // Update the volume for all active voices used by the track. If there are none then we can just stop here:
    uint8_t numActiveVoicesToVisit = trackStat.voices_active;

    if (numActiveVoicesToVisit == 0)
        return;
    
    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < numHwVoices; ++hwVoiceIdx) {
        // Only update this voice if it's for this track and active, otherwise ignore
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        if ((!voiceStat.active) || (voiceStat.track != trackStat.refindx))
            continue;

        // Figure out the current pan amount (0-127)
        int16_t currentPan;

        if (*gWess_pan_status == PAN_OFF) {
            currentPan = 64;
        } else {
            currentPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.patchmaps->pan - 64;
                
            if (currentPan > 127) {
                currentPan = 127;
            }

            if (currentPan < 0) {
                currentPan = 0;
            }
        }

        // Figure out the updated volume level (0-2047)
        uint32_t updatedVol = (uint32_t) voiceStat.velnum;
        updatedVol *= voiceStat.patchmaps->volume;
        updatedVol *= trackStat.volume_cntrl;
            
        if (trackStat.sndclass == SNDFX_CLASS) {
            updatedVol *= (*gWess_master_sfx_volume);
        } else {
            updatedVol *= (*gWess_master_mus_volume);
        }

        updatedVol >>= 21;

        // Figure out the left/right volume using the computed volume and pan
        if (*gWess_pan_status == PAN_OFF) {
            const uint16_t spuVol = (int16_t) updatedVol * 64;
            spuVoiceAttr.volume.left = spuVol;
            spuVoiceAttr.volume.right = spuVol;
        } else {
            const int16_t spuVolL = (int16_t)(((int32_t) updatedVol * 128 * (128 - currentPan)) / 128);
            const int16_t spuVolR = (int16_t)(((int32_t) updatedVol * 128 * (currentPan + 1  )) / 128);

            if (*gWess_pan_status == PAN_ON) {
                spuVoiceAttr.volume.left = spuVolL;
                spuVoiceAttr.volume.right = spuVolR;
            } else {
                // PAN_ON_REV: reverse the channels when panning
                spuVoiceAttr.volume.left = spuVolR;
                spuVoiceAttr.volume.right = spuVolL;
            }
        }

        // These are the attributes and voices to update on the SPU
        spuVoiceAttr.attr_mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
        spuVoiceAttr.voice_bits = 1 << (voiceStat.refindx % 32);

        // Update the voice attributes on the SPU
        LIBSPU_SpuSetVoiceAttr(spuVoiceAttr);

        // If there are no more active voices to visit then we are done
        numActiveVoicesToVisit--;

        if (numActiveVoicesToVisit == 0)
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer command that updates the pan setting for a track and all of it's active voices
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_PanMod(track_status& trackStat) noexcept {
    // Cache these for the loop
    SpuVoiceAttr& spuVoiceAttr = *gWess_spuVoiceAttr;
    const uint32_t numHwVoices = *gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats->get();

    // Update the track pan setting with the amount in the sequencer command
    trackStat.pan_cntrl = trackStat.ppos[1];

    // If panning is disabled then there is nothing further to do
    if (*gWess_pan_status == PAN_OFF)
        return;

    // Update the pan for all active voices used by the track. If there are no active voices then we can stop here:
    uint8_t numActiveVoicesToVisit = trackStat.voices_active;

    if (numActiveVoicesToVisit == 0)
        return;

    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < numHwVoices; ++hwVoiceIdx) {
        // Only update this voice if it's for this track and active, otherwise ignore
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        if ((!voiceStat.active) || (voiceStat.track != trackStat.refindx))
            continue;

        // Figure out the updated pan amount (0-127)
        int16_t updatedPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.patchmaps->pan - 64;
        
        if (updatedPan > 127) {
            updatedPan = 127;
        }

        if (updatedPan < 0) {
            updatedPan = 0;
        }

        // Figure out the current volume level (0-2047)
        uint32_t currentVol = (uint32_t) voiceStat.velnum;
        currentVol *= voiceStat.patchmaps->volume;
        currentVol *= trackStat.volume_cntrl;

        if (trackStat.sndclass == SNDFX_CLASS) {
            currentVol *= (*gWess_master_sfx_volume);
        } else {
            currentVol *= (*gWess_master_mus_volume);
        }

        currentVol >>= 21;

        // Figure out the left/right volume using the computed volume and pan
        const int16_t spuVolL = (int16_t)(((int32_t) currentVol * 128 * (128 - updatedPan)) / 128);
        const int16_t spuVolR = (int16_t)(((int32_t) currentVol * 128 * (updatedPan + 1  )) / 128);

        if (*gWess_pan_status == PAN_ON) {
            spuVoiceAttr.volume.left = spuVolL;
            spuVoiceAttr.volume.right = spuVolR;
        } else {
            // PAN_ON_REV: reverse the channels when panning
            spuVoiceAttr.volume.left = spuVolR;
            spuVoiceAttr.volume.right = spuVolL;
        }

        // These are the attributes and voices to update on the SPU
        spuVoiceAttr.attr_mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
        spuVoiceAttr.voice_bits = 1 << (voiceStat.refindx % 32);

        // Update the voice attributes on the SPU
        LIBSPU_SpuSetVoiceAttr(spuVoiceAttr);
                
        // If there are no more active voices to visit then we are done
        numActiveVoicesToVisit--;

        if (numActiveVoicesToVisit == 0)
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unimplemented mod/effect for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_PedalMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unimplemented mod/effect for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_ReverbMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unimplemented mod/effect for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_ChorusMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Triggers the given voice using the given params and fills in part of it's allocated status structure
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_voiceon(
    voice_status& voiceStat,
    track_status& trackStat,
    const patchmaps_header& patchmap,
    const patchinfo_header& patchInfo,
    const uint8_t voiceNote,
    const uint8_t voiceVol
) noexcept {
    master_status_structure& mstat = *gpWess_drv_mstat->get();

    // Fill in basic fields on the voice status
    voiceStat.active = true;
    voiceStat.release = false;
    voiceStat.track = trackStat.refindx;
    voiceStat.keynum = voiceNote;
    voiceStat.velnum = voiceVol;

    #if PC_PSX_DOOM_MODS
        // PC-PSX: this field was always being set to SFX, it should be using the track sound class
        voiceStat.sndtype = trackStat.sndclass;
    #else
        voiceStat.sndtype = SNDFX_CLASS;
    #endif

    voiceStat.priority = trackStat.priority;
    voiceStat.patchmaps = &patchmap;
    voiceStat.patchinfo = &patchInfo;
    voiceStat.pabstime = *gpWess_drv_curabstime->get();
    
    // Setting ADSR parameters on the voice.
    //
    // TODO: what exactly does this mean here? Find out more.
    // TODO: is ADSR2 actually time?
    const int32_t adsr = (patchmap.adsr2 & 0x20) ? 0x10000000 : 0x05DC0000;
    const uint32_t adsrShift = (0x1F - (patchmap.adsr2 & 0x1F)) & 0x1F;
    voiceStat.adsr2 = adsr >> adsrShift;

    // Inc voice count stats
    trackStat.voices_active++;
    mstat.voices_active++;

    // Actually trigger the voice with the hardware
    TriggerPSXVoice(voiceStat, voiceNote, voiceVol);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free up the given voice, and potentially the parent track if it has no more voices active
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_voiceparmoff(voice_status& voiceStat) noexcept {
    master_status_structure& mstat = *gpWess_drv_mstat->get();
    track_status& trackStat = gpWess_drv_trackStats->get()[voiceStat.track];

    // Update track and global voice stats
    mstat.voices_active--;
    trackStat.voices_active--;

    // Turn off the track if there's no more voices active and it's allowed
    if ((trackStat.voices_active == 0) && trackStat.off) {
        Eng_TrkOff(trackStat);
    }

    // Voice is not active and not in the release phase
    voiceStat.active = false;
    voiceStat.release = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move the given voice to the 'release' state so it can begin to fade out
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_voicerelease(voice_status& voiceStat) noexcept {
    const uint32_t curAbsTime = *gpWess_drv_curabstime->get();
    *gWess_drv_releasedVoices |= 1 << (voiceStat.refindx % 32);

    voiceStat.release = true;
    voiceStat.pabstime = curAbsTime + voiceStat.adsr2;      // TODO: is ADSR2 actually time?
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocate a voice for a note to play and play it back it if possible
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_voicenote(
    track_status& trackStat,
    const patchmaps_header& patchmap,
    const patchinfo_header& patchInfo,
    const uint8_t voiceNote,
    const uint8_t voiceVol
) noexcept {
    // PC-PSX: these were previously globals in the original code, but only used in this function.
    // I've converted them to first to statics so they don't pollute global scope and can be seen only where they are used.
    //
    // Next I decided to make them locals, since the previous leftover values might cause a voice not to be triggered in very rare cases.
    // I would rather have a sound play and steal another voice than not if we are at the hardware voice limit (which is exceedingly rare).
    #if PC_PSX_DOOM_MODS
        voice_status* pStolenVoiceStat = nullptr;
        uint32_t stolenVoicePriority = 256;                 // N.B: max priority is 255 (UINT8_MAX), so this guarantees we steal the first active voice we run into
        uint32_t stolenVoiceAbsTime = 0xFFFFFFFF;
    #else
        static voice_status* pStolenVoiceStat = nullptr;
        static uint32_t stolenVoicePriority = 256;          // N.B: max priority is 255 (UINT8_MAX), so this guarantees we steal the first active voice we run into
        static uint32_t stolenVoiceAbsTime = 0xFFFFFFFF;
    #endif

    // Run through all the hardware voices and try to find one that is free to play the sound.
    // If none are available, try to steal a currently active voice and prioritize which one we steal according to certain rules.
    const uint32_t numHwVoices = *gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoices = gpWess_drv_psxVoiceStats->get();

    bool bStealVoice = false;

    for (uint32_t voiceIdx = 0; voiceIdx < numHwVoices; ++voiceIdx) {
        voice_status& voiceStat = pHwVoices[voiceIdx];

        // If this voice is not in use then we can just use it
        if (!voiceStat.active) {
            // Use the voice now and finish up
            PSX_voiceon(voiceStat, trackStat, patchmap, patchInfo, voiceNote, voiceVol);
            bStealVoice = false;
            break;
        }

        // Only consider stealing the voice if the track is higher priority than the voice
        if (trackStat.priority < voiceStat.priority)
            continue;

        // If this voice is lower priority than the previously stolen voice then just steal it.
        //
        // PC-PSX: also added a null check here for the previously stolen voice just to be safe.
        // The initial assignment of 'stolenVoicePriority' should mean that we always have a valid voice
        // status for the stolen voice, but this makes that a little more obvious at least.
        #if PC_PSX_DOOM_MODS
            bStealVoice = ((!pStolenVoiceStat) || (voiceStat.priority < stolenVoicePriority));
        #else
            bStealVoice = (voiceStat.priority < stolenVoicePriority);
        #endif

        // If this voice is higher (or equal) priority to the one we stole previously then only steal under certain circumstances
        if (!bStealVoice) {            
            if (voiceStat.release) {
                // TODO: what is this prioritization trying to achieve?
                if ((stolenVoiceAbsTime > voiceStat.pabstime) || pStolenVoiceStat->release) {
                    bStealVoice = true;
                }
            } else {
                // TODO: what is this prioritization trying to achieve?
                if ((stolenVoiceAbsTime > voiceStat.pabstime) && (!pStolenVoiceStat->release)) {
                    bStealVoice = true;
                }
            }
        }

        // Use this voice for playback, unless we find a better option
        if (bStealVoice) {
            stolenVoicePriority = voiceStat.priority;
            stolenVoiceAbsTime = voiceStat.pabstime;
            pStolenVoiceStat = &voiceStat;
        }
    }

    // Steal a currently active voice and begin playback if we decided to do that
    if (bStealVoice) {
        PSX_voiceparmoff(*pStolenVoiceStat);
        PSX_voiceon(*pStolenVoiceStat, trackStat, patchmap, patchInfo, voiceNote, voiceVol);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays a note for the audio sequencer.
// Reads the patch, musical note and volume to play at from the sequence data.
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_NoteOn(track_status& trackStat) noexcept {
    // Figure out the patch, musical note (i.e pitch) and volume to use for this note being played
    uint16_t patchNum;
    uint8_t voiceNote;
    
    if (trackStat.sndclass == DRUMS_CLASS) {
        // For drums the pitch and patch is determined by the drum type
        const uint8_t drumTypeIdx = trackStat.ppos[1];
        const drumpmaps_header& drumType = gpWess_drv_drummaps->get()[drumTypeIdx];

        patchNum = drumType.patchnum;
        voiceNote = (uint8_t) drumType.note;
    } else {
        patchNum = trackStat.patchnum;
        voiceNote = trackStat.ppos[1];
    }

    const uint8_t voiceVol = trackStat.ppos[2];
    
    // Play all of the sounds for the patch
    const patches_header& patch = gpWess_drv_patchHeaders->get()[patchNum];
    const uint16_t patchSndCount = patch.patchmap_cnt;

    for (uint16_t patchSndIdx = 0; patchSndIdx < patchSndCount; ++patchSndIdx) {
        // Grab the details for this particular patch sound
        const patchmaps_header& patchmap = gpWess_drv_patchmaps->get()[patch.patchmap_idx + patchSndIdx];
        const patchinfo_header& patchInfo = gpWess_drv_patchInfos->get()[patchmap.sample_id];

        // Is this sound actually loaded? Do not play anything if it's not in SPU RAM:
        if (patchInfo.sample_pos == 0)
            continue;

        // Only play the sound if the musical note is within the allowed range for the sound
        if ((voiceNote >= patchmap.note_min) && (voiceNote <= patchmap.note_max)) {
            PSX_voicenote(trackStat, patchmap, patchInfo, voiceNote, voiceVol);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer command that turns off a specified note for the specified track
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_NoteOff(track_status& trackStat) noexcept {
    // Grab some stuff we'll need in the loop
    const uint32_t numHwVoices = *gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats->get();

    const uint8_t cmdNote = trackStat.ppos[1];
    const uint8_t cmdTrackIdx = trackStat.refindx;
    
    // Run through all the hardware voices and disable the requested note for this track
    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < numHwVoices; ++hwVoiceIdx) {
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        // Only disable if the voice is playing and not already releasing
        if (voiceStat.active && (!voiceStat.release)) {
            if ((voiceStat.keynum == cmdNote) && (voiceStat.track == cmdTrackIdx)) {
                PSX_voicerelease(voiceStat);
            }
        }
    }
}
