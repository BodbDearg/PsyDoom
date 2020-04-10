//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): PSX sequencer commands & sound driver.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "psxcmd.h"

#include "psxspu.h"
#include "PsyQ/LIBSPU.h"
#include "wessapi.h"
#include "wessarc.h"
#include "wessseq.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <algorithm>
END_THIRD_PARTY_INCLUDES

const WessDriverFunc gWess_drv_cmds[19] = {
    PSX_DriverInit,         // 00
    PSX_DriverExit,         // 01
    PSX_DriverEntry1,       // 02
    PSX_DriverEntry2,       // 03
    PSX_DriverEntry3,       // 04
    PSX_TrkOff,             // 05
    PSX_TrkMute,            // 06
    PSX_PatchChg,           // 07
    PSX_PatchMod,           // 08
    PSX_PitchMod,           // 09
    PSX_ZeroMod,            // 10
    PSX_ModuMod,            // 11
    PSX_VolumeMod,          // 12
    PSX_PanMod,             // 13
    PSX_PedalMod,           // 14
    PSX_ReverbMod,          // 15
    PSX_ChorusMod,          // 16
    PSX_NoteOn,             // 17
    PSX_NoteOff             // 18
};

static const VmPtr<VmPtr<master_status_structure>>      gpWess_drv_mstat(0x8007F164);               // Pointer to the master status structure being used by the sequencer
static const VmPtr<VmPtr<sequence_status>>              gpWess_drv_seqStats(0x8007F168);            // Saved reference to the list of sequence statuses (in the master status struct)
static const VmPtr<VmPtr<track_status>>                 gpWess_drv_trackStats(0x8007F16C);          // Saved reference to the list of track statuses (in the master status struct)
static const VmPtr<VmPtr<voice_status>>                 gpWess_drv_voiceStats(0x8007F0B8);          // Saved reference to the list of voice statuses (in the master status struct)
static const VmPtr<VmPtr<voice_status>>                 gpWess_drv_psxVoiceStats(0x8007F170);       // Saved reference to the list of PSX sound driver owned voice statuses (in the master status struct)
static const VmPtr<VmPtr<patch_group_data>>             gpWess_drv_patchGrpInfo(0x8007F178);        // Saved reference to the patch group info for the PSX sound driver
static const VmPtr<VmPtr<patches_header>>               gpWess_drv_patchHeaders(0x8007F180);        // Saved reference to the list of patches (in the master status struct)
static const VmPtr<VmPtr<patchmaps_header>>             gpWess_drv_patchmaps(0x8007F184);           // Saved reference to the list of patchmaps (in the master status struct)
static const VmPtr<VmPtr<patchinfo_header>>             gpWess_drv_patchInfos(0x8007F188);          // Saved reference to the list of patchinfos (in the master status struct)
static const VmPtr<VmPtr<drumpmaps_header>>             gpWess_drv_drummaps(0x8007F18C);            // Saved reference to the list of drummaps (in the master status struct)
static const VmPtr<uint8_t>                             gWess_drv_numPatchTypes(0x8007F0B0);        // How many patch groups there are in the currently loaded module
static const VmPtr<uint8_t>                             gWess_drv_totalVoices(0x8007F0AC);          // Maximum number of voices available to the sequencer system
static const VmPtr<uint32_t>                            gWess_drv_hwVoiceLimit(0x8007F174);         // Maximum number of voices available to the PSX sound driver
static const VmPtr<VmPtr<uint32_t>>                     gpWess_drv_curabstime(0x8007F17C);          // Pointer to the current absolute time (milliseconds since launch) for the sequencer system
static const VmPtr<uint8_t[SPU_NUM_VOICES]>             gWess_drv_chanReverbAmt(0x8007F1E8);        // Current reverb levels for each channel/voice
static const VmPtr<uint32_t>                            gWess_drv_voicesToRelease(0x80075A08);      // Bit mask for which voices are requested to be released (key off)
static const VmPtr<uint32_t>                            gWess_drv_voicesToMute(0x80075A0C);         // Bit mask for which voices are requested to be muted
static const VmPtr<uint8_t>                             gWess_drv_muteReleaseRate(0x80075A07);      // Controls how fast muted voices are faded out; higher values means a faster fade.
static const VmPtr<SpuVoiceAttr>                        gWess_spuVoiceAttr(0x8007F190);             // Temporary used for setting voice parameters with LIBSPU
static const VmPtr<uint8_t[SPU_NUM_VOICES]>             gWess_spuKeyStatuses(0x8007F1D0);           // Current voice statuses (4 possible states) returned by 'LIBSPU_SpuGetAllKeysStatus'.

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the release rate or how fast fade out occurs when muting voices
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_set_mute_release(const int32_t newReleaseRate) noexcept {
    int32_t maxRate = 0x10000000;
    *gWess_drv_muteReleaseRate = 31;

    // Note that the max release rate is '31' and minimum is '2' with this scheme
    while ((maxRate > newReleaseRate) && (*gWess_drv_muteReleaseRate != 0)) {
        maxRate >>= 1;
        *gWess_drv_muteReleaseRate -= 1;
    }
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
        triggerPan = WESS_PAN_CENTER;
    } else {
        triggerPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.patchmaps->pan - WESS_PAN_CENTER;
        triggerPan = std::min(triggerPan, (int16_t) WESS_PAN_RIGHT);
        triggerPan = std::max(triggerPan, (int16_t) WESS_PAN_LEFT);
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
        gWess_drv_chanReverbAmt[voiceIdx] = WESS_MAX_REVERB_DEPTH;
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
void PSX_DriverEntry1() noexcept {
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
        // Figure out what voices to key off (start releasing or turn off)
        uint32_t voicesToTurnOff = 0;
    
        if (*gWess_drv_voicesToRelease != 0) {
            // Some voices are requested to be released (key off) - incorporate those and clear the command
            voicesToTurnOff = *gWess_drv_voicesToRelease;
            *gWess_drv_voicesToRelease = 0;
        }

        if (*gWess_drv_voicesToMute != 0) {
            // Some voices are requested to be muted, quickly ramp those down exponetially.
            // This code here ensures that they fade out rapidly.
            SpuVoiceAttr& spuVoiceAttr = *gWess_spuVoiceAttr;

            spuVoiceAttr.voice_bits = *gWess_drv_voicesToMute;
            spuVoiceAttr.attr_mask = SPU_VOICE_ADSR_RMODE | SPU_VOICE_ADSR_RR;
            spuVoiceAttr.r_mode = SPU_VOICE_EXPDec;
            spuVoiceAttr.rr = *gWess_drv_muteReleaseRate;
            LIBSPU_SpuSetVoiceAttr(spuVoiceAttr);

            // Include these voices in the voices that will be keyed off and clear the command
            voicesToTurnOff |= *gWess_drv_voicesToMute;
            *gWess_drv_voicesToMute = 0;
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
void PSX_DriverEntry2() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unused/implemented driver update function.
// Doesn't seem to be called for the PSX sound driver.
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_DriverEntry3() noexcept {}

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
        voiceStat.adsr2 = 0x10000000 >> (31 - (*gWess_drv_muteReleaseRate % 32));   // TODO: what is this doing here?
        PSX_voicerelease(voiceStat);
        *gWess_drv_voicesToMute |= 1 << (voiceStat.refindx % 32);

        // If there are no more active voices in the track then we are done
        numActiveTrackVoices--;

        if (numActiveTrackVoices == 0)
            return;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Command that changes the patch for a track
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_PatchChg(track_status& trackStat) noexcept {
    // Read the new patch number from the sequencer command and save on the track
    trackStat.patchnum = trackStat.ppos[1];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unimplemented mod/effect for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_PatchMod([[maybe_unused]] track_status& trackStat) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer command that updates the pitch modification for a track and all of it's active voices
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_PitchMod(track_status& trackStat) noexcept {
    // Cache these for the loop
    SpuVoiceAttr& spuVoiceAttr = *gWess_spuVoiceAttr;
    const uint32_t numHwVoices = *gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats->get();

    // Read the pitch modification amount from the command: don't do anything if it's unchanged
    const int16_t pitchMod = ((int16_t) trackStat.ppos[1]) | ((int16_t) trackStat.ppos[2] << 8);

    if (trackStat.pitch_cntrl == pitchMod)
        return;

    trackStat.pitch_cntrl = pitchMod;

    // Update the pan for all active voices used by the track. If there are no active voices then we can stop here:
    uint8_t numActiveVoicesToVisit = trackStat.voices_active;

    if (numActiveVoicesToVisit == 0)
        return;
    
    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < numHwVoices; ++hwVoiceIdx) {
        // Only update this voice if it's for this track and active, otherwise ignore
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        if ((!voiceStat.active) || (voiceStat.track != trackStat.refindx))
            continue;
        
        // Set the note to play
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

        // These are the attributes and voices to update on the SPU
        spuVoiceAttr.attr_mask = SPU_VOICE_NOTE;
        spuVoiceAttr.voice_bits = 1 << (voiceStat.refindx % 32);
        
        // Update the voice attributes on the SPU
        LIBSPU_SpuSetVoiceAttr(spuVoiceAttr);

        // If there are no more active voices to visit then we are done
        numActiveVoicesToVisit--;

        if (numActiveVoicesToVisit == 0)
            return;
    }
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
            currentPan = WESS_PAN_CENTER;
        } else {
            currentPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.patchmaps->pan - WESS_PAN_CENTER;
            currentPan = std::min(currentPan, (int16_t) WESS_PAN_RIGHT);
            currentPan = std::max(currentPan, (int16_t) WESS_PAN_LEFT);
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
        int16_t updatedPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.patchmaps->pan - WESS_PAN_CENTER;
        updatedPan = std::min(updatedPan, (int16_t) WESS_PAN_RIGHT);
        updatedPan = std::max(updatedPan, (int16_t) WESS_PAN_LEFT);

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
    *gWess_drv_voicesToRelease |= 1 << (voiceStat.refindx % 32);

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
