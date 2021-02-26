//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): PSX sequencer commands & sound driver.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "psxcmd.h"

#include "Macros.h"
#include "psxspu.h"
#include "wessapi.h"
#include "wessseq.h"

#include <algorithm>

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

static constexpr uint32_t MAX_RELEASE_TIME_MS       = 0x10000000;   // Maximum time something can release for (milliseconds)
static constexpr uint32_t MAX_FAST_RELEASE_TIME_MS  = 0x05DC0000;   // A scaled version of the maximum release time that faster, used by some voices.

static master_status_structure*     gpWess_drv_mstat;               // Pointer to the master status structure being used by the sequencer
static sequence_status*             gpWess_drv_sequenceStats;       // Saved reference to the list of sequence statuses (in the master status struct)
static track_status*                gpWess_drv_trackStats;          // Saved reference to the list of track statuses (in the master status struct)
static voice_status*                gpWess_drv_voiceStats;          // Saved reference to the list of voice statuses (in the master status struct)
static voice_status*                gpWess_drv_psxVoiceStats;       // Saved reference to the list of PSX sound driver owned voice statuses (in the master status struct)
static patch_group_data*            gpWess_drv_patchGroup;          // Saved reference to the patch group info for the PSX sound driver
static patch*                       gpWess_drv_patches;             // Saved reference to the list of patches (in the master status struct)
static patch_voice*                 gpWess_drv_patchVoices;         // Saved reference to the list of patch voices (in the master status struct)
static patch_sample*                gpWess_drv_patchSamples;        // Saved reference to the list of patch samples (in the master status struct)
static drum_patch*                  gpWess_drv_drumPatches;         // Saved reference to the list of drum patches (in the master status struct)

static uint8_t          gWess_drv_numPatchGroups;                   // How many patch groups there are in the currently loaded module
static uint8_t          gWess_drv_totalVoices;                      // Maximum number of voices available to the sequencer system
static uint32_t         gWess_drv_hwVoiceLimit;                     // Maximum number of voices available to the PSX sound driver
static uint32_t*        gpWess_drv_cur_abstime_ms;                  // Pointer to the current absolute time (milliseconds since launch) for the sequencer system
static uint8_t          gWess_drv_chanReverbAmt[SPU_NUM_VOICES];    // Current reverb levels for each channel/voice
static uint32_t         gWess_drv_voicesToRelease;                  // Bit mask for which voices are requested to be released (key off)
static uint32_t         gWess_drv_voicesToMute;                     // Bit mask for which voices are requested to be muted
static uint8_t          gWess_drv_muteReleaseRate;                  // Controls how fast muted voices are faded out; higher values means a faster fade.
static SpuVoiceAttr     gWess_spuVoiceAttr;                         // Temporary used for setting voice parameters with LIBSPU
static uint8_t          gWess_spuKeyStatuses[SPU_NUM_VOICES];       // Current voice statuses (4 possible states) returned by 'LIBSPU_SpuGetAllKeysStatus'.

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the location where we will start recording voice details before temporarily muting
//------------------------------------------------------------------------------------------------------------------------------------------
void start_record_music_mute(SavedVoiceList* const pVoices) noexcept {
    gpWess_savedVoices = pVoices;

    if (pVoices) {
        pVoices->size = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears the location where we will record voice details while temporarily muting
//------------------------------------------------------------------------------------------------------------------------------------------
void end_record_music_mute() noexcept {
    gpWess_savedVoices = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Records details about a playing voice for later restoring
//------------------------------------------------------------------------------------------------------------------------------------------
void add_music_mute_note(
    const int16_t seqIdx,
    const int16_t trackStatIdx,
    const uint8_t note,
    const uint8_t volume,
    const patch_voice& patchVoice,
    const patch_sample& patchSample
) noexcept {
    SavedVoiceList* const pSavedVoices = gpWess_savedVoices;

    if (pSavedVoices) {
        // PsyDoom: sanity check we haven't exceeded the bounds of the saved notes array.
        // This shouldn't be called more times than there are hardware voices available!
        WESS_ASSERT(pSavedVoices->size < SPU_NUM_VOICES);

        SavedVoice& voice = pSavedVoices->voices[pSavedVoices->size];
        voice.seq_idx = seqIdx;
        voice.trackstat_idx = trackStatIdx;
        voice.note = note;
        voice.volume = volume;
        voice.ppatch_voice = &patchVoice;
        voice.ppatch_sample = &patchSample;

        pSavedVoices->size++;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the release rate or how fast fade out occurs when muting voices
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_set_mute_release(const uint32_t newReleaseTimeMs) noexcept {
    // Figure out the SPU release rate value to use, it's a power of 2 shift/scale.
    // Note that the max release rate is '31' and minimum is '2' with this code.
    uint32_t approxReleaseTimeMs = MAX_RELEASE_TIME_MS;
    gWess_drv_muteReleaseRate = 31;

    while ((approxReleaseTimeMs > newReleaseTimeMs) && (gWess_drv_muteReleaseRate != 0)) {
        approxReleaseTimeMs >>= 1;
        gWess_drv_muteReleaseRate -= 1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger a sound to play with the specified volume and using the information in the voice status struct.
// Note: the note setting was apparently ignored by this function? Instead it is sourced from the voice status.
//------------------------------------------------------------------------------------------------------------------------------------------
void TriggerPSXVoice(const voice_status& voiceStat, [[maybe_unused]] const uint8_t voiceNote, const uint8_t voiceVol) noexcept {
    // Get the track status and LIBSPU struct we use to setup the voice
    SpuVoiceAttr& spuVoiceAttr = gWess_spuVoiceAttr;
    track_status& trackStat = gpWess_drv_trackStats[voiceStat.trackstat_idx];

    // These are the attributes we will set
    spuVoiceAttr.attr_mask = (
        SPU_VOICE_VOLL | SPU_VOICE_VOLR |
        SPU_VOICE_NOTE | SPU_VOICE_SAMPLE_NOTE |
        SPU_VOICE_WDSA |
        SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2
    );

    // This is the voice we are manipulating
    spuVoiceAttr.voice_bits = 1 << (voiceStat.ref_idx % 32);

    // Setup reverb
    if (trackStat.reverb == 0) {
        if (gWess_drv_chanReverbAmt[voiceStat.ref_idx] != 0) {
            LIBSPU_SpuSetReverbVoice(0, spuVoiceAttr.voice_bits);
            gWess_drv_chanReverbAmt[voiceStat.ref_idx] = 0;
        }
    } else {
        if (gWess_drv_chanReverbAmt[voiceStat.ref_idx] == 0) {
            LIBSPU_SpuSetReverbVoice(1, spuVoiceAttr.voice_bits);
            gWess_drv_chanReverbAmt[voiceStat.ref_idx] = 127;
        }
    }

    // Figure out pan amount
    int16_t triggerPan;

    if (gWess_pan_status == PAN_OFF) {
        triggerPan = WESS_PAN_CENTER;
    } else {
        // Note: deduct 'WESS_PAN_CENTER' since panning should be centered when both these settings are at the center
        triggerPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.ppatch_voice->pan - WESS_PAN_CENTER;
        triggerPan = std::min(triggerPan, (int16_t) WESS_PAN_RIGHT);
        triggerPan = std::max(triggerPan, (int16_t) WESS_PAN_LEFT);
    }

    // Figure out the trigger volume for the note (0-2047)
    uint32_t triggerVol = voiceVol;
    triggerVol *= voiceStat.ppatch_voice->volume;
    triggerVol *= trackStat.volume_cntrl;

    if (trackStat.sound_class == SNDFX_CLASS) {
        triggerVol *= gWess_master_sfx_volume;
    } else {
        triggerVol *= gWess_master_mus_volume;
    }

    triggerVol >>= 21;
    
    // Set volume using trigger volume and pan
    if (gWess_pan_status == PAN_OFF) {
        const uint16_t spuVol = (int16_t) triggerVol * 64;
        spuVoiceAttr.volume.left = spuVol;
        spuVoiceAttr.volume.right = spuVol;
    } else {
        const int16_t spuVolL = (int16_t)(((int32_t) triggerVol * 128 * (128 - triggerPan)) / 128);
        const int16_t spuVolR = (int16_t)(((int32_t) triggerVol * 128 * (triggerPan + 1  )) / 128);

        if (gWess_pan_status == PAN_ON) {
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
        spuVoiceAttr.note = (uint16_t) voiceStat.note << 8;
    } else {
        if (trackStat.pitch_cntrl >= 1) {
            // Pitch shifting: up
            const uint32_t pitchShiftFrac = trackStat.pitch_cntrl * voiceStat.ppatch_voice->pitchstep_up;
            const uint16_t pitchShiftNote = (uint16_t)((32 + pitchShiftFrac) >> 13);
            const uint16_t pitchShiftFine = (pitchShiftFrac & 0x1FFF) >> 6;

            spuVoiceAttr.note = (voiceStat.note + pitchShiftNote) << 8;
            spuVoiceAttr.note |= pitchShiftFine & 0x7F;
        }
        else {
            // Pitch shifting: down
            const uint32_t pitchShiftFrac = trackStat.pitch_cntrl * voiceStat.ppatch_voice->pitchstep_down;
            const uint16_t pitchShiftNote = (uint16_t)(((32 - pitchShiftFrac) >> 13) + 1);
            const uint16_t pitchShiftFine = 128 - ((pitchShiftFrac & 0x1FFF) >> 6);

            spuVoiceAttr.note = (voiceStat.note - pitchShiftNote) << 8;
            spuVoiceAttr.note |= pitchShiftFine & 0x7F;
        }
    }

    spuVoiceAttr.sample_note = ((uint16_t) voiceStat.ppatch_voice->base_note << 8) | voiceStat.ppatch_voice->base_note_frac;

    // Set the SPU address of the sound data
    spuVoiceAttr.addr = voiceStat.ppatch_sample->spu_addr;

    // Setup volume envelope
    spuVoiceAttr.adsr1 = voiceStat.ppatch_voice->adsr1;
    spuVoiceAttr.adsr2 = voiceStat.ppatch_voice->adsr2;

    // Trigger the note!
    LIBSPU_SpuSetKeyOnWithAttr(spuVoiceAttr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does initialization for the PSX sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_DriverInit(master_status_structure& mstat) noexcept {
    // Save pointers to various master status fields and stats
    const uint8_t numPatchGroups = mstat.pmodule->hdr.num_patch_groups;

    gpWess_drv_mstat = &mstat;
    gpWess_drv_sequenceStats = mstat.psequence_stats;
    gpWess_drv_trackStats = mstat.ptrack_stats;
    gpWess_drv_voiceStats = mstat.pvoice_stats;
    gWess_drv_numPatchGroups = numPatchGroups;
    gWess_drv_totalVoices = mstat.max_voices;
    gpWess_drv_cur_abstime_ms = mstat.pabstime_ms;
    
    // Determine the start of the PSX hardware voices in the voices list
    gpWess_drv_psxVoiceStats = gpWess_drv_voiceStats;

    for (uint8_t voiceIdx = 0; voiceIdx < mstat.max_voices; ++voiceIdx) {
        voice_status& voiceStat = gpWess_drv_voiceStats[voiceIdx];

        if (voiceStat.driver_id == PSX_ID) {
            gpWess_drv_psxVoiceStats = &voiceStat;
            break;
        }
    }

    // Determine the patch group info for the PSX hardware driver
    gpWess_drv_patchGroup = nullptr;

    for (uint8_t patchGroupIdx = 0; patchGroupIdx < numPatchGroups; ++patchGroupIdx) {
        patch_group_data& patchGroup = mstat.ppatch_groups[patchGroupIdx];

        if (patchGroup.hdr.driver_id == PSX_ID) {
            gpWess_drv_patchGroup = &patchGroup;
            break;
        }
    }

    // Save various bits of patch group information and pointers for the PSX patch group
    patch_group_data& patchGroup = *gpWess_drv_patchGroup;
    gWess_drv_hwVoiceLimit = patchGroup.hdr.hw_voice_limit;

    {
        uint8_t* pPatchData = patchGroup.pdata;
        gpWess_drv_patches = (patch*) pPatchData;
        pPatchData += sizeof(patch) * patchGroup.hdr.num_patches;

        wess_align_byte_ptr(pPatchData, alignof(patch_voice));
        gpWess_drv_patchVoices = (patch_voice*) pPatchData;
        pPatchData += sizeof(patch_voice) * patchGroup.hdr.num_patch_voices;

        wess_align_byte_ptr(pPatchData, alignof(patch_sample));
        gpWess_drv_patchSamples = (patch_sample*) pPatchData;
        pPatchData += sizeof(patch_sample) * patchGroup.hdr.num_patch_samples;

        wess_align_byte_ptr(pPatchData, alignof(drum_patch));
        gpWess_drv_drumPatches = (drum_patch*) pPatchData;
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
    const uint32_t maxHwVoices = gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats;

    // Stop all currently active voices that are releasing and past their end time for release
    {
        // Grab some stuff needed in the loop
        master_status_structure& mstat = *gpWess_drv_mstat;

        uint8_t numActiveVoicesLeftToVisit = mstat.num_active_voices;
        const uint32_t curAbsTime = *gpWess_drv_cur_abstime_ms;

        // Turn off all applicable voices
        if (numActiveVoicesLeftToVisit > 0) {
            for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < maxHwVoices; ++hwVoiceIdx) {
                voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];
                
                // Only stop the voice if active, releasing and if the time is past when it should be released
                if (voiceStat.active && voiceStat.release && (curAbsTime > voiceStat.onoff_abstime_ms)) {
                    PSX_voiceparmoff(voiceStat);

                    // If there are no active more voices left active then we are done
                    numActiveVoicesLeftToVisit--;

                    if (numActiveVoicesLeftToVisit == 0)
                        break;
                }
            }
        }
    }
    
    // Key off any voices that were requested to be released or muted
    {
        // Figure out what voices to key off (start releasing or turn off)
        uint32_t voicesToTurnOff = 0;
    
        if (gWess_drv_voicesToRelease != 0) {
            // Some voices are requested to be released (key off) - incorporate those and clear the command
            voicesToTurnOff = gWess_drv_voicesToRelease;
            gWess_drv_voicesToRelease = 0;
        }

        if (gWess_drv_voicesToMute != 0) {
            // Some voices are requested to be muted, quickly ramp those down exponetially.
            // This code here ensures that they fade out rapidly.
            SpuVoiceAttr& spuVoiceAttr = gWess_spuVoiceAttr;

            spuVoiceAttr.voice_bits = gWess_drv_voicesToMute;
            spuVoiceAttr.attr_mask = SPU_VOICE_ADSR_RMODE | SPU_VOICE_ADSR_RR;
            spuVoiceAttr.r_mode = SPU_VOICE_EXPDec;
            spuVoiceAttr.rr = gWess_drv_muteReleaseRate;
            LIBSPU_SpuSetVoiceAttr(spuVoiceAttr);

            // Include these voices in the voices that will be keyed off and clear the command
            voicesToTurnOff |= gWess_drv_voicesToMute;
            gWess_drv_voicesToMute = 0;
        }
    
        // Key off the voices requested to be keyed off
        if (voicesToTurnOff != 0) {
            LIBSPU_SpuSetKey(0, voicesToTurnOff);
        }
    }
    
    // Release any voices that the SPU says are now off
    LIBSPU_SpuGetAllKeysStatus(gWess_spuKeyStatuses);

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

    if (trackStat.num_active_voices == 0) {
        // If there are no voices left active in the track then stop in the sequencer
        Eng_TrkOff(trackStat);
    } else {
        // Otherwise decrement the number of sequence tracks playing and update state
        trackStat.off = true;

        // PsyDoom: bug-fix, don't double count the track being stopped if it's already stopped.
        // Otherwise we can potentially corrupt the track playing count and cause it to underflow etc.
        // This can potentially cause infinite freezes in places where we are waiting for sounds to stop, like loading transitions.
        #if PSYDOOM_MODS
            const bool bNeedToStopTrack = (!trackStat.stopped);
        #else
            const bool bNeedToStopTrack = true;
        #endif

        sequence_status& seqStat = gpWess_drv_sequenceStats[trackStat.seqstat_idx];

        if (bNeedToStopTrack) {
            trackStat.stopped = true;
            WESS_ASSERT(seqStat.num_tracks_playing > 0);
            seqStat.num_tracks_playing--;
        }

        if (seqStat.num_tracks_playing == 0) {
            seqStat.playmode = SEQ_STATE_STOPPED;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Command that mutes the given track
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_TrkMute(track_status& trackStat) noexcept {
    // If there are no voices active in the track then there is nothing to do
    uint32_t numActiveVoicesToVisit = trackStat.num_active_voices;

    if (numActiveVoicesToVisit == 0)
        return;

    // Run through all PSX hardware voices and mute the ones belonging to this track
    uint32_t numHwVoices = gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoices = gpWess_drv_psxVoiceStats;

    for (uint32_t voiceIdx = 0; voiceIdx < numHwVoices; ++voiceIdx) {
        voice_status& voiceStat = pHwVoices[voiceIdx];

        // Only mute this voice if active and belonging to this track, ignore otherwise
        if ((!voiceStat.active) || (voiceStat.trackstat_idx != trackStat.ref_idx))
            continue;
        
        // If the voice is not being killed, is a music voice and we are recording voice state then save for later un-pause
        if (gpWess_savedVoices && (!voiceStat.release) && (trackStat.sound_class == MUSIC_CLASS)) {
            sequence_status& seqStat = gpWess_drv_sequenceStats[trackStat.seqstat_idx];

            add_music_mute_note(
                seqStat.seq_idx,
                voiceStat.trackstat_idx,
                voiceStat.note,
                voiceStat.volume,
                *voiceStat.ppatch_voice,
                *voiceStat.ppatch_sample
            );
        }

        // Begin releasing the voice and figure out how long it will take to fade based on the release rate.
        // The release rate is exponential, hence the shifts here:
        voiceStat.release_time_ms = MAX_RELEASE_TIME_MS >> (31 - (gWess_drv_muteReleaseRate % 32));
        PSX_voicerelease(voiceStat);
        gWess_drv_voicesToMute |= 1 << (voiceStat.ref_idx % 32);

        // If there are no more active voices to visit then we are done
        numActiveVoicesToVisit--;

        if (numActiveVoicesToVisit == 0)
            return;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Command that changes the patch for a track
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_PatchChg(track_status& trackStat) noexcept {
    // Read the new patch number from the sequencer command and save on the track
    trackStat.patch_idx = ((uint16_t) trackStat.pcur_cmd[1]) | ((uint16_t) trackStat.pcur_cmd[2] << 8);
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
    SpuVoiceAttr& spuVoiceAttr =  gWess_spuVoiceAttr;
    const uint32_t numHwVoices = gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats;

    // Read the pitch modification amount from the command: don't do anything if it's unchanged
    const int16_t pitchMod = ((int16_t) trackStat.pcur_cmd[1]) | ((int16_t) trackStat.pcur_cmd[2] << 8);

    if (trackStat.pitch_cntrl == pitchMod)
        return;

    trackStat.pitch_cntrl = pitchMod;

    // Update the pan for all active voices used by the track. If there are no active voices then we can stop here:
    uint8_t numActiveVoicesToVisit = trackStat.num_active_voices;

    if (numActiveVoicesToVisit == 0)
        return;
    
    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < numHwVoices; ++hwVoiceIdx) {
        // Only update this voice if it's for this track and active, otherwise ignore
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        if ((!voiceStat.active) || (voiceStat.trackstat_idx != trackStat.ref_idx))
            continue;
        
        // Set the note to play
        if (trackStat.pitch_cntrl == 0) {
            // Not doing any pitch shifting
            spuVoiceAttr.note = (uint16_t) voiceStat.note << 8;
        } else {
            if (trackStat.pitch_cntrl >= 1) {
                // Pitch shifting: up
                const uint32_t pitchShiftFrac = trackStat.pitch_cntrl * voiceStat.ppatch_voice->pitchstep_up;
                const uint16_t pitchShiftNote = (uint16_t)((32 + pitchShiftFrac) >> 13);
                const uint16_t pitchShiftFine = (pitchShiftFrac & 0x1FFF) >> 6;

                spuVoiceAttr.note = (voiceStat.note + pitchShiftNote) << 8;
                spuVoiceAttr.note |= pitchShiftFine & 0x7F;
            }
            else {
                // Pitch shifting: down
                const uint32_t pitchShiftFrac = trackStat.pitch_cntrl * voiceStat.ppatch_voice->pitchstep_down;
                const uint16_t pitchShiftNote = (uint16_t)(((32 - pitchShiftFrac) >> 13) + 1);
                const uint16_t pitchShiftFine = 128 - ((pitchShiftFrac & 0x1FFF) >> 6);

                spuVoiceAttr.note = (voiceStat.note - pitchShiftNote) << 8;
                spuVoiceAttr.note |= pitchShiftFine & 0x7F;
            }
        }

        // These are the attributes and voices to update on the SPU
        spuVoiceAttr.attr_mask = SPU_VOICE_NOTE;
        spuVoiceAttr.voice_bits = 1 << (voiceStat.ref_idx % 32);
        
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
    SpuVoiceAttr& spuVoiceAttr = gWess_spuVoiceAttr;
    const uint32_t numHwVoices = gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats;

    // Update the track volume with the amount in the sequencer command
    trackStat.volume_cntrl = trackStat.pcur_cmd[1];

    // Update the volume for all active voices used by the track. If there are none then we can just stop here:
    uint8_t numActiveVoicesToVisit = trackStat.num_active_voices;

    if (numActiveVoicesToVisit == 0)
        return;
    
    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < numHwVoices; ++hwVoiceIdx) {
        // Only update this voice if it's for this track and active, otherwise ignore
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        if ((!voiceStat.active) || (voiceStat.trackstat_idx != trackStat.ref_idx))
            continue;

        // Figure out the current pan amount (0-127)
        int16_t currentPan;

        if (gWess_pan_status == PAN_OFF) {
            currentPan = WESS_PAN_CENTER;
        } else {
            // Note: deduct 'WESS_PAN_CENTER' since panning should be centered when both these settings are at the center
            currentPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.ppatch_voice->pan - WESS_PAN_CENTER;
            currentPan = std::min(currentPan, (int16_t) WESS_PAN_RIGHT);
            currentPan = std::max(currentPan, (int16_t) WESS_PAN_LEFT);
        }

        // Figure out the updated volume level (0-2047)
        uint32_t updatedVol = (uint32_t) voiceStat.volume;
        updatedVol *= voiceStat.ppatch_voice->volume;
        updatedVol *= trackStat.volume_cntrl;
        
        if (trackStat.sound_class == SNDFX_CLASS) {
            updatedVol *= gWess_master_sfx_volume;
        } else {
            updatedVol *= gWess_master_mus_volume;
        }

        updatedVol >>= 21;

        // Figure out the left/right volume using the computed volume and pan
        if (gWess_pan_status == PAN_OFF) {
            const uint16_t spuVol = (int16_t) updatedVol * 64;
            spuVoiceAttr.volume.left = spuVol;
            spuVoiceAttr.volume.right = spuVol;
        } else {
            const int16_t spuVolL = (int16_t)(((int32_t) updatedVol * 128 * (128 - currentPan)) / 128);
            const int16_t spuVolR = (int16_t)(((int32_t) updatedVol * 128 * (currentPan + 1  )) / 128);

            if (gWess_pan_status == PAN_ON) {
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
        spuVoiceAttr.voice_bits = 1 << (voiceStat.ref_idx % 32);

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
    SpuVoiceAttr& spuVoiceAttr = gWess_spuVoiceAttr;
    const uint32_t numHwVoices = gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats;

    // Update the track pan setting with the amount in the sequencer command
    trackStat.pan_cntrl = trackStat.pcur_cmd[1];

    // If panning is disabled then there is nothing further to do
    if (gWess_pan_status == PAN_OFF)
        return;

    // Update the pan for all active voices used by the track. If there are no active voices then we can stop here:
    uint8_t numActiveVoicesToVisit = trackStat.num_active_voices;

    if (numActiveVoicesToVisit == 0)
        return;

    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < numHwVoices; ++hwVoiceIdx) {
        // Only update this voice if it's for this track and active, otherwise ignore
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        if ((!voiceStat.active) || (voiceStat.trackstat_idx != trackStat.ref_idx))
            continue;

        // Figure out the updated pan amount (0-127)
        int16_t updatedPan = (int16_t) trackStat.pan_cntrl + (int16_t) voiceStat.ppatch_voice->pan - WESS_PAN_CENTER;
        updatedPan = std::min(updatedPan, (int16_t) WESS_PAN_RIGHT);
        updatedPan = std::max(updatedPan, (int16_t) WESS_PAN_LEFT);

        // Figure out the current volume level (0-2047)
        uint32_t currentVol = (uint32_t) voiceStat.volume;
        currentVol *= voiceStat.ppatch_voice->volume;
        currentVol *= trackStat.volume_cntrl;

        if (trackStat.sound_class == SNDFX_CLASS) {
            currentVol *= gWess_master_sfx_volume;
        } else {
            currentVol *= gWess_master_mus_volume;
        }

        currentVol >>= 21;

        // Figure out the left/right volume using the computed volume and pan
        const int16_t spuVolL = (int16_t)(((int32_t) currentVol * 128 * (128 - updatedPan)) / 128);
        const int16_t spuVolR = (int16_t)(((int32_t) currentVol * 128 * (updatedPan + 1  )) / 128);

        if (gWess_pan_status == PAN_ON) {
            spuVoiceAttr.volume.left = spuVolL;
            spuVoiceAttr.volume.right = spuVolR;
        } else {
            // PAN_ON_REV: reverse the channels when panning
            spuVoiceAttr.volume.left = spuVolR;
            spuVoiceAttr.volume.right = spuVolL;
        }

        // These are the attributes and voices to update on the SPU
        spuVoiceAttr.attr_mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
        spuVoiceAttr.voice_bits = 1 << (voiceStat.ref_idx % 32);

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
    const patch_voice& patchVoice,
    const patch_sample& patchSample,
    const uint8_t voiceNote,
    const uint8_t voiceVol
) noexcept {
    master_status_structure& mstat = *gpWess_drv_mstat;

    // Fill in basic fields on the voice status
    voiceStat.active = true;
    voiceStat.release = false;
    voiceStat.trackstat_idx = trackStat.ref_idx;
    voiceStat.note = voiceNote;
    voiceStat.volume = voiceVol;

    #if PSYDOOM_MODS
        // PsyDoom: this field was always being set to SFX, it should be using the track sound class
        voiceStat.sound_class = trackStat.sound_class;
    #else
        voiceStat.sound_class = SNDFX_CLASS;
    #endif

    voiceStat.priority = trackStat.priority;
    voiceStat.ppatch_voice = &patchVoice;
    voiceStat.ppatch_sample = &patchSample;
    voiceStat.onoff_abstime_ms = *gpWess_drv_cur_abstime_ms;
    
    // Figure out how long it takes for the voice to fade out.
    // The low 5 bits affect the exponential falloff, the 6th bit controls how that falloff is scaled.
    const uint32_t adsr = (patchVoice.adsr2 & 0x20) ? MAX_RELEASE_TIME_MS : MAX_FAST_RELEASE_TIME_MS;
    const uint32_t adsrShift = 31 - (patchVoice.adsr2 % 32);
    voiceStat.release_time_ms = adsr >> adsrShift;
    
    // Increment voice count stats
    WESS_ASSERT(trackStat.num_active_voices < UINT8_MAX);
    trackStat.num_active_voices++;

    WESS_ASSERT(mstat.num_active_voices < UINT8_MAX);
    mstat.num_active_voices++;

    // Actually trigger the voice with the hardware
    TriggerPSXVoice(voiceStat, voiceNote, voiceVol);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free up the given voice, and potentially the parent track if it has no more voices active
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_voiceparmoff(voice_status& voiceStat) noexcept {
    master_status_structure& mstat = *gpWess_drv_mstat;
    track_status& trackStat = gpWess_drv_trackStats[voiceStat.trackstat_idx];

    // Update track and global voice stats
    WESS_ASSERT(mstat.num_active_voices > 0);
    mstat.num_active_voices--;

    WESS_ASSERT(trackStat.num_active_voices > 0);
    trackStat.num_active_voices--;

    // Turn off the track if there's no more voices active and it's allowed
    if ((trackStat.num_active_voices == 0) && trackStat.off) {
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
    const uint32_t curAbsTime = *gpWess_drv_cur_abstime_ms;
    gWess_drv_voicesToRelease |= 1 << (voiceStat.ref_idx % 32);

    voiceStat.release = true;
    voiceStat.onoff_abstime_ms = curAbsTime + voiceStat.release_time_ms;    // Compute when the voice is to be turned off or deallocated
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocate an individual voice for a note being played and begin playing it if possible
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_voicenote(
    track_status& trackStat,
    const patch_voice& patchVoice,
    const patch_sample& patchSample,
    const uint8_t voiceNote,
    const uint8_t voiceVol
) noexcept {
    // PsyDoom: these were previously globals in the original code, but only used in this function.
    // I've converted them to first to statics so they don't pollute global scope and can be seen only where they are used.
    //
    // Next I decided to make them locals, since the previous leftover values might cause a voice not to be triggered in very rare cases.
    // I would rather have a sound play and steal another voice than not if we are at the hardware voice limit (which is a rare case).
    #if PSYDOOM_MODS
        voice_status* pStolenVoiceStat = nullptr;
        uint32_t stolenVoicePriority = 256;                 // N.B: max priority is 255 (UINT8_MAX), so this guarantees we steal the first active voice we run into
        uint32_t stolenVoiceOnOffAbsTime = 0xFFFFFFFF;
    #else
        static voice_status* pStolenVoiceStat = nullptr;
        static uint32_t stolenVoicePriority = 256;          // N.B: max priority is 255 (UINT8_MAX), so this guarantees we steal the first active voice we run into
        static uint32_t stolenVoiceOnOffAbsTime = 0xFFFFFFFF;
    #endif

    // Run through all the hardware voices and try to find one that is free to play the sound.
    // If none are available, try to steal a currently active voice and prioritize which one we steal according to certain rules.
    const uint32_t numHwVoices = gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoices = gpWess_drv_psxVoiceStats;

    bool bStealVoice = false;

    for (uint32_t voiceIdx = 0; voiceIdx < numHwVoices; ++voiceIdx) {
        voice_status& voiceStat = pHwVoices[voiceIdx];

        // If this voice is not in use then we can just use it
        if (!voiceStat.active) {
            // Use the voice now and finish up
            PSX_voiceon(voiceStat, trackStat, patchVoice, patchSample, voiceNote, voiceVol);
            bStealVoice = false;
            break;
        }

        // Only consider stealing the voice if the track is higher priority than the voice
        if (trackStat.priority < voiceStat.priority)
            continue;

        // If this voice is lower priority than the previously stolen voice then just steal it.
        //
        // PsyDoom: also added a null check here for the previously stolen voice just to be safe.
        // The initial assignment of 'stolenVoicePriority' should mean that we always have a valid voice
        // status for the stolen voice, but this makes that a little more obvious at least.
        #if PSYDOOM_MODS
            bStealVoice = ((!pStolenVoiceStat) || (voiceStat.priority < stolenVoicePriority));
        #else
            bStealVoice = (voiceStat.priority < stolenVoicePriority);
        #endif

        // If this voice is higher (or equal) priority to the one we stole previously then only steal under certain circumstances
        if (!bStealVoice) {
            if (voiceStat.release) {
                // Steal this voice instead if it ends sooner or if the currently chosen voice to steal is NOT being released
                if ((stolenVoiceOnOffAbsTime > voiceStat.onoff_abstime_ms) || (!pStolenVoiceStat->release)) {
                    bStealVoice = true;
                }
            } else {
                // Steal this voice instead if it ends sooner and the currently chosen voice to steal is also NOT being released
                if ((stolenVoiceOnOffAbsTime > voiceStat.onoff_abstime_ms) && (!pStolenVoiceStat->release)) {
                    bStealVoice = true;
                }
            }
        }

        // Use this voice for playback, unless we find a better option
        if (bStealVoice) {
            stolenVoicePriority = voiceStat.priority;
            stolenVoiceOnOffAbsTime = voiceStat.onoff_abstime_ms;
            pStolenVoiceStat = &voiceStat;
        }
    }

    // Steal a currently active voice and begin playback if we decided to do that
    if (bStealVoice) {
        PSX_voiceparmoff(*pStolenVoiceStat);
        PSX_voiceon(*pStolenVoiceStat, trackStat, patchVoice, patchSample, voiceNote, voiceVol);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays a note for the audio sequencer.
// Reads the patch, musical note and volume to play at from the sequence data.
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_NoteOn(track_status& trackStat) noexcept {
    // Figure out the patch, musical note (i.e pitch) and volume to use for this note being played
    uint16_t patchIdx;
    uint8_t voiceNote;
    
    if (trackStat.sound_class == DRUMS_CLASS) {
        // For drums the note to play back with is determined by the drum patch: drums always use the same note
        const uint8_t drumTypeIdx = trackStat.pcur_cmd[1];
        const drum_patch& drumPatch = gpWess_drv_drumPatches[drumTypeIdx];

        patchIdx = drumPatch.patch_idx;
        voiceNote = (uint8_t) drumPatch.note;
    } else {
        patchIdx = trackStat.patch_idx;
        voiceNote = trackStat.pcur_cmd[1];
    }

    const uint8_t voiceVol = trackStat.pcur_cmd[2];
    
    // Play all of the voices/sounds for the patch/instrument
    const patch& patch = gpWess_drv_patches[patchIdx];
    const uint16_t patchVoiceCount = patch.num_voices;

    for (uint16_t voiceIdx = 0; voiceIdx < patchVoiceCount; ++voiceIdx) {
        // Grab the details for this particular patch voice
        const patch_voice& patchVoice = gpWess_drv_patchVoices[patch.first_voice_idx + voiceIdx];
        const patch_sample& patchSample = gpWess_drv_patchSamples[patchVoice.sample_idx];

        // Is this sample actually loaded? Do not play anything if it's not in SPU RAM:
        if (patchSample.spu_addr == 0)
            continue;

        // Only play the sound if the musical note is within the allowed range for the sound
        if ((voiceNote >= patchVoice.note_min) && (voiceNote <= patchVoice.note_max)) {
            PSX_voicenote(trackStat, patchVoice, patchSample, voiceNote, voiceVol);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequencer command that turns off a specified note for the specified track
//------------------------------------------------------------------------------------------------------------------------------------------
void PSX_NoteOff(track_status& trackStat) noexcept {
    // Grab some stuff we'll need in the loop
    const uint32_t numHwVoices = gWess_drv_hwVoiceLimit;
    voice_status* const pHwVoiceStats = gpWess_drv_psxVoiceStats;

    const uint8_t cmdNote = trackStat.pcur_cmd[1];
    const uint8_t cmdTrackIdx = trackStat.ref_idx;
    
    // Run through all the hardware voices and disable the requested note for this track
    for (uint32_t hwVoiceIdx = 0; hwVoiceIdx < numHwVoices; ++hwVoiceIdx) {
        voice_status& voiceStat = pHwVoiceStats[hwVoiceIdx];

        // Only disable if the voice is playing and not already releasing
        if (voiceStat.active && (!voiceStat.release)) {
            if ((voiceStat.note == cmdNote) && (voiceStat.trackstat_idx == cmdTrackIdx)) {
                PSX_voicerelease(voiceStat);
            }
        }
    }
}
