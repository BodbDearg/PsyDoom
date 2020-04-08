#include "s_sound.h"

#include "Doom/cdmaptbl.h"
#include "Doom/Renderer/r_main.h"
#include "i_main.h"
#include "PsxVm/PsxVm.h"
#include "sounds.h"
#include "Wess/lcdload.h"
#include "Wess/psxcd.h"
#include "Wess/psxspu.h"
#include "Wess/seqload.h"
#include "Wess/seqload_r.h"
#include "Wess/wessapi.h"
#include "Wess/wessapi_m.h"
#include "Wess/wessapi_p.h"
#include "Wess/wessapi_t.h"
#include "Wess/wessarc.h"

// Sound settings for the WESS PSX sound driver
static const int32_t gSound_PSXSettings[SNDHW_TAG_MAX * 2] = {
    SNDHW_TAG_DRIVER_ID,        PSX_ID,
    SNDHW_TAG_SOUND_EFFECTS,    1,
    SNDHW_TAG_MUSIC,            1,
    SNDHW_TAG_DRUMS,            1,
    SNDHW_TAG_END,              0
};

// What sound settings to use to when loading the .WMD file
static const int32_t* gSound_SettingsLists[2] = {
    gSound_PSXSettings,
    nullptr
};

// What track each piece of music uses
const uint32_t gCDTrackNum[NUM_CD_MUSIC_TRACKS] = {
    2,      // cdmusic_title_screen
    3,      // cdmusic_main_menu
    4,      // cdmusic_credits_demo
    5,      // cdmusic_intermission
    6,      // cdmusic_club_doom
    7,      // cdmusic_finale_doom1
    8       // cdmusic_finale_doom2
};

// Current volume cd music is played back at
const VmPtr<int32_t> gCdMusicVol(0x800775F8);

// The size of the WMD buffer
static constexpr uint32_t WMD_MEM_SIZE = 26000;

// The buffer used to hold the master status structure created by loading the .WMD file.
// Also holds sequence data for current level music.
static const VmPtr<uint8_t[WMD_MEM_SIZE]> gSound_WmdMem(0x80078588);

// The start pointer within 'gSound_WmdMem' where map music sequences can be loaded to.
// Anything at this address or after in the buffer can be used for that purpose.
static const VmPtr<VmPtr<uint8_t>> gpSound_MusicSeqData(0x80077E1C);

// The next address in SPU RAM to upload sounds to
static const VmPtr<uint32_t>    gSound_CurSpuAddr(0x80077E18);

// Sample blocks for general DOOM sounds and map specific music and sfx sounds
static const VmPtr<SampleBlock>     gDoomSndBlock(0x8007EC9C);
static const VmPtr<SampleBlock>     gMapSndBlock(0x8007EE30);

// Is the DOOMSFX.LCD file loaded? (main sound effect samples)
static const VmPtr<bool32_t>    gbDidLoadDoomSfxLcd(0x80077E20);

// Used to save the state of voices when pausing
static const VmPtr<NoteState>   gPausedMusVoiceState(0x8007EB18);

// Unused tick count for how many sound 'updates' ticks were done
static const VmPtr<uint32_t>    gNumSoundTics(0x80077E2C);

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the master sound effects volume
//------------------------------------------------------------------------------------------------------------------------------------------
void S_SetSfxVolume(int32_t sfxVol) noexcept {
    wess_master_sfx_vol_set((uint8_t) sfxVol);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the master music volume
//------------------------------------------------------------------------------------------------------------------------------------------
void S_SetMusicVolume(const int32_t musVol) noexcept {
    // Set the volume for both the sound sequencer and for cd audio
    wess_master_mus_vol_set((uint8_t) musVol);

    // TODO: where does the '0x3CFF' constant come from?
    int32_t cdVol = musVol * 128;

    if (cdVol > 0x3CFF) {
        cdVol = 0x3CFF;
    }

    psxspu_set_cd_vol(cdVol);
}

void S_StopMusic() noexcept {
loc_80041014:
    v0 = lw(gp + 0x834);                                // Load from: gCurMapMusicNum (80077E14)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_80041040;
    v0 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54E0;                                       // Result = MapMusicDefs[1] (800754E0)
    at += v0;
    a0 = lw(at);
    wess_seq_stop(a0);
loc_80041040:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void S_StartMusic() noexcept {
loc_80041050:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    S_StopMusic();
    v0 = lw(gp + 0x834);                                // Load from: gCurMapMusicNum (80077E14)
    {
        const bool bJump = (v0 == 0);
        v0 <<= 4;
        if (bJump) goto loc_80041088;
    }
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54E0;                                       // Result = MapMusicDefs[1] (800754E0)
    at += v0;
    a0 = lw(at);
    wess_seq_trigger(a0);
loc_80041088:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the given sample block
//------------------------------------------------------------------------------------------------------------------------------------------
void S_InitSampleBlock(SampleBlock& block) noexcept {
    block.numsamps = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unload all loaded samples in the given sample block
//------------------------------------------------------------------------------------------------------------------------------------------
void S_UnloadSampleBlock(SampleBlock& sampleBlock) noexcept {
    // Zero the patch address in SPU RAM for every single loaded sample:
    while (sampleBlock.numsamps > 0) {
        sampleBlock.numsamps--;
        const uint32_t patchIdx = sampleBlock.sampindx[sampleBlock.numsamps];
        wess_dig_set_sample_position(patchIdx, 0);
    }
}

void S_LoadSoundAndMusic() noexcept {
loc_80041118:
    v0 = lw(gp + 0x830);                                // Load from: gLoadedSoundsMapNum (80077E10)
    sp -= 0x28;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x20);
    sw(s0, sp + 0x18);
    if (v0 == s1) goto loc_80041300;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x3C;                                      // Result = 0000003C
        if (bJump) goto loc_8004119C;
    }
    v0 = lw(gp + 0x834);                                // Load from: gCurMapMusicNum (80077E14)
    if (v0 == 0) goto loc_80041188;
    s0 = 1;                                             // Result = 00000001
    S_StopMusic();
loc_80041154:
    v0 = lw(gp + 0x834);                                // Load from: gCurMapMusicNum (80077E14)
    v0 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54E0;                                       // Result = MapMusicDefs[1] (800754E0)
    at += v0;
    a0 = lw(at);
    v0 = wess_seq_status(a0);
    a0 = 0x5A;                                          // Result = 0000005A
    if (v0 != s0) goto loc_80041154;
    a1 = 0x14;                                          // Result = 00000014
    v0 = wess_seq_range_free(a0, a1);
loc_80041188:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x11D0;                                       // Result = gMapMusSfxLoadedSamples[0] (8007EE30)
    S_UnloadSampleBlock(*vmAddrToPtr<SampleBlock>(a0));
    v0 = 0x3C;                                          // Result = 0000003C
loc_8004119C:
    if (s1 != v0) goto loc_800411C8;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x1364;                                       // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    S_UnloadSampleBlock(*vmAddrToPtr<SampleBlock>(a0));
    v0 = 0x1010;                                        // Result = 00001010
    sw(0, gp + 0x840);                                  // Store to: gbDidLoadDoomSfxLcd (80077E20)
    sw(v0, gp + 0x838);                                 // Store to: gNextSoundUploadAddr (80077E18)
    v0 = s1 << 3;
    goto loc_80041204;
loc_800411C8:
    v0 = lw(gp + 0x840);                                // Load from: gbDidLoadDoomSfxLcd (80077E20)
    {
        const bool bJump = (v0 != 0);
        v0 = s1 << 3;
        if (bJump) goto loc_80041204;
    }
    a0 = 0xC8;                                          // Result = 000000C8
    a1 = 0x1010;                                        // Result = 00001010
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x1364;                                       // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    a3 = 0;                                             // Result = 00000000
    v0 = wess_dig_lcd_load((CdMapTbl_File) a0, a1, vmAddrToPtr<SampleBlock>(a2), a3);
    v0 += 0x1010;
    sw(v0, gp + 0x838);                                 // Store to: gNextSoundUploadAddr (80077E18)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x840);                                 // Store to: gbDidLoadDoomSfxLcd (80077E20)
    v0 = s1 << 3;
loc_80041204:
    at = 0x80070000;                                    // Result = 80070000
    at += 0x56D0;                                       // Result = gMapSndLcdFileAndMusNum[1] (800756D0)
    at += v0;
    v0 = lw(at);
    s0 = lw(gp + 0x838);                                // Load from: gNextSoundUploadAddr (80077E18)
    sw(v0, gp + 0x834);                                 // Store to: gCurMapMusicNum (80077E14)
    {
        const bool bJump = (v0 == 0);
        v0 <<= 4;
        if (bJump) goto loc_800412B8;
    }
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54E4;                                       // Result = MapMusicDefs[2] (800754E4)
    at += v0;
    a0 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54E8;                                       // Result = MapMusicDefs[3] (800754E8)
    at += v0;
    a1 = lh(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54EA;                                       // Result = MapMusicDefs[3] (800754EA)
    at += v0;
    a2 = lh(at);
    a3 = 0;                                             // Result = 00000000
    sw(0, sp + 0x10);
    psxspu_init_reverb((SpuReverbMode) a0, (int16_t) a1, (int16_t) a2, a3, lw(sp + 0x10));
    v0 = lw(gp + 0x834);                                // Load from: gCurMapMusicNum (80077E14)
    a1 = lw(gp + 0x83C);                                // Load from: gpMusSequencesEnd (80077E1C)
    v0 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54E0;                                       // Result = MapMusicDefs[1] (800754E0)
    at += v0;
    a0 = lw(at);
    v0 = wess_seq_load(a0, vmAddrToPtr<void>(a1));
    a1 = s0;
    v0 = lw(gp + 0x834);                                // Load from: gCurMapMusicNum (80077E14)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x11D0;                                       // Result = gMapMusSfxLoadedSamples[0] (8007EE30)
    v0 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54DC;                                       // Result = MapMusicDefs[0] (800754DC)
    at += v0;
    a0 = lw(at);
    a3 = 0;                                             // Result = 00000000
    v0 = wess_dig_lcd_load((CdMapTbl_File) a0, a1, vmAddrToPtr<SampleBlock>(a2), a3);
    s0 += v0;
    goto loc_800412D0;
loc_800412B8:
    sw(0, sp + 0x10);
    a0 = 0;                                             // Result = 00000000
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    psxspu_init_reverb((SpuReverbMode) a0, (int16_t) a1, (int16_t) a2, a3, lw(sp + 0x10));
loc_800412D0:
    v0 = s1 << 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x56CC;                                       // Result = gMapSndLcdFileAndMusNum[0] (800756CC)
    at += v0;
    a0 = lw(at);
    sw(s1, gp + 0x830);                                 // Store to: gLoadedSoundsMapNum (80077E10)
    a1 = s0;
    if (a0 == 0) goto loc_80041300;
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x11D0;                                       // Result = gMapMusSfxLoadedSamples[0] (8007EE30)
    a3 = 0;                                             // Result = 00000000
    v0 = wess_dig_lcd_load((CdMapTbl_File) a0, a1, vmAddrToPtr<SampleBlock>(a2), a3);
loc_80041300:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops all playing sounds and sequencer music: music state is also saved for later restoring
//------------------------------------------------------------------------------------------------------------------------------------------
void S_Pause() noexcept {
    wess_seq_pauseall(true, gPausedMusVoiceState.get());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Resume playing previously paused music
//------------------------------------------------------------------------------------------------------------------------------------------
void S_Resume() noexcept {
    wess_seq_restartall(gPausedMusVoiceState.get());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop playing the specified sound
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StopSound(const VmPtr<mobj_t> origin) noexcept {
    wess_seq_stoptype(origin.addr());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops all playing sounds and sequencer music
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StopAll() noexcept {
    wess_seq_stopall();
}

void I_StartSound() noexcept {
loc_800413A8:
    sp -= 0x40;
    sw(s3, sp + 0x34);
    s3 = a1;
    v0 = (i32(s3) < 0x5A);
    sw(ra, sp + 0x38);
    sw(s2, sp + 0x30);
    sw(s1, sp + 0x2C);
    sw(s0, sp + 0x28);
    if (v0 == 0) goto loc_80041594;
    v0 = *gCurPlayerIndex;
    s0 = a0;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    at += v0;
    s2 = lw(at);
    a3 = 0x7F;                                          // Result = 0000007F
    if (s0 == 0) goto loc_80041530;
    t0 = 0x40;                                          // Result = 00000040
    if (s0 == s2) goto loc_80041534;
    a0 = lw(s2);
    a2 = lw(s0);
    a1 = lw(s2 + 0x4);
    a3 = lw(s0 + 0x4);
    v0 = a0 - a2;
    if (i32(v0) >= 0) goto loc_8004142C;
    v0 = -v0;
loc_8004142C:
    t0 = v0;
    v0 = a1 - a3;
    v1 = v0;
    if (i32(v0) >= 0) goto loc_80041440;
    v1 = -v1;
loc_80041440:
    v0 = (i32(v1) < i32(t0));
    t1 = t0 + v1;
    if (v0 == 0) goto loc_80041450;
    t0 = v1;
loc_80041450:
    v0 = u32(i32(t0) >> 1);
    s1 = t1 - v0;
    v0 = 0x4640000;                                     // Result = 04640000
    v0 = (i32(v0) < i32(s1));
    if (v0 != 0) goto loc_80041594;
    _thunk_R_PointToAngle2();
    a0 = lw(s2 + 0x24);
    v1 = v0;
    v0 = (a0 < v1);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - 1;
        if (bJump) goto loc_8004148C;
    }
    v1 -= a0;
    goto loc_80041490;
loc_8004148C:
    v1 = v0 - a0;
loc_80041490:
    v1 >>= 19;
    a0 = 0x630000;                                      // Result = 00630000
    v0 = v1 << 2;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    v1 = lw(at);
    a0 |= 0xFFFF;                                       // Result = 0063FFFF
    a0 = (i32(a0) < i32(s1));
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 5;
    v0 = u32(i32(v0) >> 16);
    v1 = 0x80;                                          // Result = 00000080
    t0 = v1 - v0;
    t0 = u32(i32(t0) >> 1);
    if (a0 != 0) goto loc_800414DC;
    a3 = 0x7F;                                          // Result = 0000007F
    goto loc_800414F4;
loc_800414DC:
    v1 = 0x4640000;                                     // Result = 04640000
    v1 -= s1;
    v1 = u32(i32(v1) >> 16);
    v0 = v1 << 7;
    v0 -= v1;
    a3 = u32(i32(v0) >> 10);
loc_800414F4:
    v1 = lw(s0);
    v0 = lw(s2);
    if (v1 != v0) goto loc_80041520;
    v1 = lw(s0 + 0x4);
    v0 = lw(s2 + 0x4);
    if (v1 != v0) goto loc_80041520;
    t0 = 0x40;                                          // Result = 00000040
loc_80041520:
    if (i32(a3) <= 0) goto loc_80041594;
    goto loc_80041534;
loc_80041530:
    t0 = 0x40;                                          // Result = 00000040
loc_80041534:
    v0 = 0x7F;                                          // Result = 0000007F
    if (s0 == 0) goto loc_80041568;
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v0 = lw(v0 + 0x24);
    v0 &= 1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x7F;                                      // Result = 0000007F
        if (bJump) goto loc_80041568;
    }
    sb(0, sp + 0x1B);
    goto loc_8004156C;
loc_80041568:
    sb(v0, sp + 0x1B);
loc_8004156C:
    a0 = s3;
    a1 = s0;
    v0 = 0x103;                                         // Result = 00000103
    sw(v0, sp + 0x10);
    sb(a3, sp + 0x14);
    sw(a3, gp + 0x844);                                 // Store to: gLastSoundPan (80077E24)
    sb(t0, sp + 0x15);
    sw(t0, gp + 0x848);                                 // Store to: gLastSoundVol (80077E28)
    a2 = sp + 0x10;
    wess_seq_trigger_type_special(a0, a1, vmAddrToPtr<TriggerPlayAttr>(a2));
loc_80041594:
    ra = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x40;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the sound effect with the given id.
// The origin parameter is optional and helps determine panning/attenuation.
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StartSound(mobj_t* const pOrigin, const sfxenum_t soundId) noexcept {
    a0 = ptrToVmAddr(pOrigin);
    a1 = soundId;
    a2 = 0;
    I_StartSound();
}

void _thunk_S_StartSound() noexcept {    
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Placeholder where sound update logic per tick can be done
//------------------------------------------------------------------------------------------------------------------------------------------
void S_UpdateSounds() noexcept {
    // This didn't do anything for PSX DOOM other than incrementing this global sound tick counter.
    // Updates to the sequencer system were instead driven by hardware timer interrupts, firing approximately 120 times a second.
    *gNumSoundTics += 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the PlayStation sound system.
// The given temporary buffer is used to hold the .WMD file while it is being processed.
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxSoundInit(const int32_t sfxVol, const int32_t musVol, void* const pTmpWmdLoadBuffer) noexcept {    
    // Initialize the WESS API and low level CDROM utilities
    wess_init();
    psxcd_init();

    // Read the WMD file into the given buffer (assumes it is big enough)
    PsxCd_File* const pFile = psxcd_open(CdMapTbl_File::DOOMSFX_WMD);

    #if PC_PSX_DOOM_MODS
        if (!pFile) {
            FATAL_ERROR("Failed to open DOOMSFX.WMD!");
        }
    #endif

    psxcd_read(pTmpWmdLoadBuffer, pFile->file.size, *pFile);
    psxcd_close(*pFile);

    // Initialize the sample blocks used to keep track what sounds are uploaded to where in SPU RAM
    S_InitSampleBlock(*gDoomSndBlock);
    S_InitSampleBlock(*gMapSndBlock);

    // Load the main module (.WMD) file.
    // This initializes common stuff used for all music and sounds played in the game.
    wess_load_module(pTmpWmdLoadBuffer, gSound_WmdMem.get(), WMD_MEM_SIZE, gSound_SettingsLists);

    // Initialize the music sequence and LCD (samples file) loaders
    master_status_structure& mstat = *wess_get_master_status();
    wess_dig_lcd_loader_init(&mstat);
    wess_seq_loader_init(&mstat, CdMapTbl_File::DOOMSFX_WMD, true);

    // Load all of the very simple 'music sequences' for sound effects in the game.
    // These are kept loaded at all times since they are small.
    const int32_t sfxSequenceBytes = wess_seq_range_load(0, NUMSFX, wess_get_wmd_end());

    // We load map music sequences to the start of the remaining bytes in WMD memory buffer
    *gpSound_MusicSeqData = wess_get_wmd_end() + sfxSequenceBytes;

    // Init master volume levels
    S_SetSfxVolume(sfxVol);
    S_SetMusicVolume(musVol);

    // Load the main SFX LCD: this is required for the main menu.
    // Also set the next location to upload to SPU RAM at after the load finishes.
    *gbDidLoadDoomSfxLcd = false;
    *gSound_CurSpuAddr = SPU_RAM_APP_BASE + wess_dig_lcd_load(CdMapTbl_File::DOOMSFX_LCD, SPU_RAM_APP_BASE, gDoomSndBlock.get(), false);
    *gbDidLoadDoomSfxLcd = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown the PlayStation sound system
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxSoundExit() noexcept {
    // Did nothing - not required for a PS1 game...
    // TODO: PC-PSX should cleanup logic be considered here?
}
