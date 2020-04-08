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

// What CD audio track each piece of CD music uses
const uint32_t gCDTrackNum[NUM_CD_MUSIC_TRACKS] = {
    2,      // cdmusic_title_screen
    3,      // cdmusic_main_menu
    4,      // cdmusic_credits_demo
    5,      // cdmusic_intermission
    6,      // cdmusic_club_doom
    7,      // cdmusic_finale_doom1
    8       // cdmusic_finale_doom2
};

// Defines the settings for one music sequencer track
struct musicseq_t {
    CdMapTbl_File   lcdFile;            // LCD file containing the samples used by the music track
    int32_t         seqIdx;             // What sequence to trigger for the track (from the module file)
    SpuReverbMode   reverbMode;         // What type of reverb to use
    int16_t         reverbDepthL;       // Reverb depth: left
    int16_t         reverbDepthR;       // Reverb depth: right
};

// Definitions for all of the available music sequences in the game.
// Note that not all of these are unique songs, some of the tracks are simply with different reverb settings.
static const musicseq_t gMusicSeqDefs[31] = {
    { CdMapTbl_File{},          0,      SPU_REV_MODE_OFF,       0x0000, 0x0000 },
    { CdMapTbl_File::MUSLEV1,   90,     SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdMapTbl_File::MUSLEV2,   91,     SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdMapTbl_File::MUSLEV3,   92,     SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF },
    { CdMapTbl_File::MUSLEV4,   93,     SPU_REV_MODE_HALL,      0x17FF, 0x17FF },
    { CdMapTbl_File::MUSLEV5,   94,     SPU_REV_MODE_STUDIO_A,  0x23FF, 0x23FF },
    { CdMapTbl_File::MUSLEV6,   95,     SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV7,   96,     SPU_REV_MODE_STUDIO_C,  0x26FF, 0x26FF },
    { CdMapTbl_File::MUSLEV8,   97,     SPU_REV_MODE_STUDIO_B,  0x2DFF, 0x2DFF },
    { CdMapTbl_File::MUSLEV9,   98,     SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdMapTbl_File::MUSLEV10,  99,     SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdMapTbl_File::MUSLEV11,  100,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV12,  101,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV13,  102,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdMapTbl_File::MUSLEV14,  103,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV15,  104,    SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF },
    { CdMapTbl_File::MUSLEV16,  105,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdMapTbl_File::MUSLEV17,  106,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV18,  107,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdMapTbl_File::MUSLEV19,  108,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV20,  109,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdMapTbl_File::MUSLEV19,  108,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdMapTbl_File::MUSLEV2,   91,     SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV1,   90,     SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV13,  102,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV7,   96,     SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF },
    { CdMapTbl_File::MUSLEV16,  105,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdMapTbl_File::MUSLEV5,   94,     SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdMapTbl_File::MUSLEV1,   90,     SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdMapTbl_File::MUSLEV17,  106,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdMapTbl_File{},          0,      SPU_REV_MODE_OFF,       0x0000, 0x0000 }
};

// How many music sequence tracks there are in the game
static constexpr uint32_t NUM_MUSIC_SEQS = 20;

// Defines what LCD file to load for a map and what music sequence definition to use
struct mapaudiodef_t {
    CdMapTbl_File   sfxLcdFile;     // LCD file SFX for monsters on the map
    uint32_t        musicSeqIdx;    // Index of the music sequence to use
};

// What sound LCD file and music track to use for all maps in the game
static const mapaudiodef_t gMapAudioDefs[62] = {
    { CdMapTbl_File{},              0   },
    { CdMapTbl_File::MAP01_LCD,     1   },
    { CdMapTbl_File::MAP02_LCD,     2   },
    { CdMapTbl_File::MAP03_LCD,     3   },
    { CdMapTbl_File::MAP04_LCD,     4   },
    { CdMapTbl_File::MAP05_LCD,     5   },
    { CdMapTbl_File::MAP06_LCD,     6   },
    { CdMapTbl_File::MAP07_LCD,     7   },
    { CdMapTbl_File::MAP08_LCD,     8   },
    { CdMapTbl_File::MAP09_LCD,     11  },
    { CdMapTbl_File::MAP10_LCD,     9   },
    { CdMapTbl_File::MAP11_LCD,     15  },
    { CdMapTbl_File::MAP12_LCD,     10  },
    { CdMapTbl_File::MAP13_LCD,     21  },
    { CdMapTbl_File::MAP14_LCD,     22  },
    { CdMapTbl_File::MAP15_LCD,     23  },
    { CdMapTbl_File::MAP16_LCD,     12  },
    { CdMapTbl_File::MAP17_LCD,     16  },
    { CdMapTbl_File::MAP18_LCD,     17  },
    { CdMapTbl_File::MAP19_LCD,     6   },
    { CdMapTbl_File::MAP20_LCD,     18  },
    { CdMapTbl_File::MAP21_LCD,     24  },
    { CdMapTbl_File::MAP22_LCD,     14  },
    { CdMapTbl_File::MAP23_LCD,     3   },
    { CdMapTbl_File::MAP24_LCD,     20  },
    { CdMapTbl_File::MAP25_LCD,     11  },
    { CdMapTbl_File::MAP26_LCD,     25  },
    { CdMapTbl_File::MAP27_LCD,     4   },
    { CdMapTbl_File::MAP28_LCD,     5   },
    { CdMapTbl_File::MAP29_LCD,     10  },
    { CdMapTbl_File::MAP30_LCD,     19  },
    { CdMapTbl_File::MAP31_LCD,     1   },
    { CdMapTbl_File::MAP32_LCD,     9   },
    { CdMapTbl_File::MAP33_LCD,     14  },
    { CdMapTbl_File::MAP34_LCD,     12  },
    { CdMapTbl_File::MAP35_LCD,     8   },
    { CdMapTbl_File::MAP36_LCD,     13  },
    { CdMapTbl_File::MAP37_LCD,     18  },
    { CdMapTbl_File::MAP38_LCD,     20  },
    { CdMapTbl_File::MAP39_LCD,     15  },
    { CdMapTbl_File::MAP40_LCD,     19  },
    { CdMapTbl_File::MAP41_LCD,     11  },
    { CdMapTbl_File::MAP42_LCD,     26  },
    { CdMapTbl_File::MAP43_LCD,     12  },
    { CdMapTbl_File::MAP44_LCD,     29  },
    { CdMapTbl_File::MAP45_LCD,     6   },
    { CdMapTbl_File::MAP46_LCD,     27  },
    { CdMapTbl_File::MAP47_LCD,     9   },
    { CdMapTbl_File::MAP48_LCD,     22  },
    { CdMapTbl_File::MAP49_LCD,     3   },
    { CdMapTbl_File::MAP50_LCD,     28  },
    { CdMapTbl_File::MAP51_LCD,     7   },
    { CdMapTbl_File::MAP52_LCD,     8   },
    { CdMapTbl_File::MAP53_LCD,     15  },
    { CdMapTbl_File::MAP54_LCD,     4   },
    { CdMapTbl_File::MAP55_LCD,     17  },
    { CdMapTbl_File::MAP56_LCD,     18  },
    { CdMapTbl_File::MAP57_LCD,     10  },
    { CdMapTbl_File::MAP58_LCD,     16  },
    { CdMapTbl_File::MAP59_LCD,     13  },
    { CdMapTbl_File::MAP60_LCD,     0   },
    { CdMapTbl_File{},              0   }
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

// Which of the music sequence definitions is currently playing
static const VmPtr<uint32_t> gCurMusicSeqIdx(0x80077E14);

// What map we have sound and music currently loaded for
static const VmPtr<uint32_t> gLoadedSoundAndMusMapNum(0x80077E10);

// The next address in SPU RAM to upload sounds to.
// TODO: rename - this is really where to upload map stuff to.
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop the currently playing music track
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StopMusic() noexcept {
    if (*gCurMusicSeqIdx != 0) {
        wess_seq_stop(gMusicSeqDefs[*gCurMusicSeqIdx].seqIdx);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Start playing the selected music track (stops if currently playing)
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StartMusic() noexcept {
    S_StopMusic();

    if (*gCurMusicSeqIdx != 0) {
        wess_seq_trigger(gMusicSeqDefs[*gCurMusicSeqIdx].seqIdx);
    }
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads all sound and music for the given map number (Note: '0' if menu, '60' if finale)
//------------------------------------------------------------------------------------------------------------------------------------------
void S_LoadMapSoundAndMusic(const int32_t mapIdx) noexcept {
    // If sound and music is already loaded then bail out
    if (*gLoadedSoundAndMusMapNum == mapIdx)
        return;

    // Stop current map music, free all music sequences and unload map SFX
    if (*gLoadedSoundAndMusMapNum != 0) {
        if (*gCurMusicSeqIdx != 0) {
            S_StopMusic();

            while (wess_seq_status(gMusicSeqDefs[*gCurMusicSeqIdx].seqIdx) != SequenceStatus::SEQUENCE_INACTIVE) {
                // TODO: PC-PSX - update the window etc. here?
            }

            wess_seq_range_free(0 + NUMSFX, NUM_MUSIC_SEQS);
        }

        S_UnloadSampleBlock(*gMapSndBlock);
    }

    // Either load or unload the main Doom SFX LCD
    if (mapIdx == 60) {
        // For the finale unload the LCD to free up RAM
        S_UnloadSampleBlock(*gDoomSndBlock);
        *gbDidLoadDoomSfxLcd = false;
        *gSound_CurSpuAddr = SPU_RAM_APP_BASE;
    } else {
        // In all other cases ensure it is loaded
        if (!*gbDidLoadDoomSfxLcd) {
            *gSound_CurSpuAddr = SPU_RAM_APP_BASE + wess_dig_lcd_load(CdMapTbl_File::DOOMSFX_LCD, SPU_RAM_APP_BASE, gDoomSndBlock.get(), false);
            *gbDidLoadDoomSfxLcd = true;
        }
    }

    // Load the music sequence and lcd file for the map music.
    // Also initialize the reverb mode depending on the music.
    *gCurMusicSeqIdx = gMapAudioDefs[mapIdx].musicSeqIdx;
    uint32_t destSpuAddr = *gSound_CurSpuAddr;

    if (*gCurMusicSeqIdx == 0) {
        // No music sequences for this map - turn off reverb
        psxspu_init_reverb(SPU_REV_MODE_OFF, 0, 0, 0, 0);
    } else {
        // Normal case: playing a map music sequence
        const musicseq_t& musicseq = gMusicSeqDefs[*gCurMusicSeqIdx];
        psxspu_init_reverb(musicseq.reverbMode, musicseq.reverbDepthL, musicseq.reverbDepthR, 0, 0);
        wess_seq_load(musicseq.seqIdx, gpSound_MusicSeqData->get());
        destSpuAddr += wess_dig_lcd_load(musicseq.lcdFile, destSpuAddr, gMapSndBlock.get(), false);
    }

    // Remember what map we have loaded sound and music for
    *gLoadedSoundAndMusMapNum = mapIdx;

    // Load the sound LCD file for the map
    if (gMapAudioDefs[mapIdx].sfxLcdFile != CdMapTbl_File{}) {
        wess_dig_lcd_load(gMapAudioDefs[mapIdx].sfxLcdFile, destSpuAddr, gMapSndBlock.get(), false);
    }
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
