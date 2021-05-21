#if !PSYDOOM_MODS

// Defines the settings for one music sequencer track
struct musicseq_t {
    CdFileId        lcdFile;            // LCD file containing the samples used by the music track
    int32_t         seqIdx;             // What sequence to trigger for the track (from the module file)
    SpuReverbMode   reverbMode;         // What type of reverb to use
    int16_t         reverbDepthL;       // Reverb depth: left
    int16_t         reverbDepthR;       // Reverb depth: right
};

// Definitions for all of map music in the game: Doom and Final Doom
static const musicseq_t gMusicSeqDefs_Doom[31] = {
    { CdFileId{},               0,      SPU_REV_MODE_OFF,       0x0000, 0x0000 },
    { CdFileId::MUSLEV1_LCD,    90,     SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdFileId::MUSLEV2_LCD,    91,     SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdFileId::MUSLEV3_LCD,    92,     SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF },
    { CdFileId::MUSLEV4_LCD,    93,     SPU_REV_MODE_HALL,      0x17FF, 0x17FF },
    { CdFileId::MUSLEV5_LCD,    94,     SPU_REV_MODE_STUDIO_A,  0x23FF, 0x23FF },
    { CdFileId::MUSLEV6_LCD,    95,     SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV7_LCD,    96,     SPU_REV_MODE_STUDIO_C,  0x26FF, 0x26FF },
    { CdFileId::MUSLEV8_LCD,    97,     SPU_REV_MODE_STUDIO_B,  0x2DFF, 0x2DFF },
    { CdFileId::MUSLEV9_LCD,    98,     SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdFileId::MUSLEV10_LCD,   99,     SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdFileId::MUSLEV11_LCD,   100,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV12_LCD,   101,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV13_LCD,   102,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdFileId::MUSLEV14_LCD,   103,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV15_LCD,   104,    SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF },
    { CdFileId::MUSLEV16_LCD,   105,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdFileId::MUSLEV17_LCD,   106,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV18_LCD,   107,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF },
    { CdFileId::MUSLEV19_LCD,   108,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV20_LCD,   109,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdFileId::MUSLEV19_LCD,   108,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdFileId::MUSLEV2_LCD,    91,     SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV1_LCD,    90,     SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV13_LCD,   102,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV7_LCD,    96,     SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF },
    { CdFileId::MUSLEV16_LCD,   105,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF },
    { CdFileId::MUSLEV5_LCD,    94,     SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdFileId::MUSLEV1_LCD,    90,     SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdFileId::MUSLEV17_LCD,   106,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF },
    { CdFileId{},               0,      SPU_REV_MODE_OFF,       0x0000, 0x0000 }
};

static const musicseq_t gMusicSeqDefs_FinalDoom[32] = {
    { CdFileId{},               0,      SPU_REV_MODE_OFF,       0x0000, 0x0000  },
    { CdFileId::MUSLEV23_LCD,   112,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV29_LCD,   118,    SPU_REV_MODE_STUDIO_C,  0x26FF, 0x26FF  },
    { CdFileId::MUSLEV24_LCD,   113,    SPU_REV_MODE_SPACE,     0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV30_LCD,   119,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF  },
    { CdFileId::MUSLEV21_LCD,   110,    SPU_REV_MODE_SPACE,     0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV27_LCD,   116,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV25_LCD,   114,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF  },
    { CdFileId::MUSLEV28_LCD,   117,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV22_LCD,   111,    SPU_REV_MODE_SPACE,     0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV26_LCD,   115,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF  },
    { CdFileId::MUSLEV1_LCD,    90,     SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF  },
    { CdFileId::MUSLEV2_LCD,    91,     SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF  },
    { CdFileId::MUSLEV3_LCD,    92,     SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF  },
    { CdFileId::MUSLEV4_LCD,    93,     SPU_REV_MODE_HALL,      0x17FF, 0x17FF  },
    { CdFileId::MUSLEV5_LCD,    94,     SPU_REV_MODE_STUDIO_A,  0x23FF, 0x23FF  },
    { CdFileId::MUSLEV6_LCD,    95,     SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV7_LCD,    96,     SPU_REV_MODE_STUDIO_C,  0x26FF, 0x26FF  },
    { CdFileId::MUSLEV8_LCD,    97,     SPU_REV_MODE_STUDIO_B,  0x2DFF, 0x2DFF  },
    { CdFileId::MUSLEV9_LCD,    98,     SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF  },
    { CdFileId::MUSLEV10_LCD,   99,     SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF  },
    { CdFileId::MUSLEV11_LCD,   100,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV12_LCD,   101,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF  },
    { CdFileId::MUSLEV13_LCD,   102,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF  },
    { CdFileId::MUSLEV14_LCD,   103,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV15_LCD,   104,    SPU_REV_MODE_STUDIO_B,  0x27FF, 0x27FF  },
    { CdFileId::MUSLEV16_LCD,   105,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF  },
    { CdFileId::MUSLEV17_LCD,   106,    SPU_REV_MODE_HALL,      0x1FFF, 0x1FFF  },
    { CdFileId::MUSLEV18_LCD,   107,    SPU_REV_MODE_SPACE,     0x0FFF, 0x0FFF  },
    { CdFileId::MUSLEV22_LCD,   111,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF  },
    { CdFileId::MUSLEV26_LCD,   115,    SPU_REV_MODE_STUDIO_C,  0x2FFF, 0x2FFF  },
    { CdFileId{},               0,      SPU_REV_MODE_OFF,       0x0000, 0x0000  },
};

static constexpr uint32_t NUM_MUSIC_SEQS_DOOM       = 20;   // How many music sequence tracks there are in the game: Doom
static constexpr uint32_t NUM_MUSIC_SEQS_FINAL_DOOM = 30;   // How many music sequence tracks there are in the game: Final Doom

// Defines what LCD file to load for a map and what music sequence definition to use
struct mapaudiodef_t {
    CdFileId    sfxLcdFile;     // LCD file SFX for monsters on the map
    uint32_t    musicSeqIdx;    // Index of the music sequence to use
};

// What sound LCD file and music track to use for all maps in the game: Doom
static const mapaudiodef_t gMapAudioDefs_Doom[62] = {
    { CdFileId{},           0   },
    { CdFileId::MAP01_LCD,  1   },
    { CdFileId::MAP02_LCD,  2   },
    { CdFileId::MAP03_LCD,  3   },
    { CdFileId::MAP04_LCD,  4   },
    { CdFileId::MAP05_LCD,  5   },
    { CdFileId::MAP06_LCD,  6   },
    { CdFileId::MAP07_LCD,  7   },
    { CdFileId::MAP08_LCD,  8   },
    { CdFileId::MAP09_LCD,  11  },
    { CdFileId::MAP10_LCD,  9   },
    { CdFileId::MAP11_LCD,  15  },
    { CdFileId::MAP12_LCD,  10  },
    { CdFileId::MAP13_LCD,  21  },
    { CdFileId::MAP14_LCD,  22  },
    { CdFileId::MAP15_LCD,  23  },
    { CdFileId::MAP16_LCD,  12  },
    { CdFileId::MAP17_LCD,  16  },
    { CdFileId::MAP18_LCD,  17  },
    { CdFileId::MAP19_LCD,  6   },
    { CdFileId::MAP20_LCD,  18  },
    { CdFileId::MAP21_LCD,  24  },
    { CdFileId::MAP22_LCD,  14  },
    { CdFileId::MAP23_LCD,  3   },
    { CdFileId::MAP24_LCD,  20  },
    { CdFileId::MAP25_LCD,  11  },
    { CdFileId::MAP26_LCD,  25  },
    { CdFileId::MAP27_LCD,  4   },
    { CdFileId::MAP28_LCD,  5   },
    { CdFileId::MAP29_LCD,  10  },
    { CdFileId::MAP30_LCD,  19  },
    { CdFileId::MAP31_LCD,  1   },
    { CdFileId::MAP32_LCD,  9   },
    { CdFileId::MAP33_LCD,  14  },
    { CdFileId::MAP34_LCD,  12  },
    { CdFileId::MAP35_LCD,  8   },
    { CdFileId::MAP36_LCD,  13  },
    { CdFileId::MAP37_LCD,  18  },
    { CdFileId::MAP38_LCD,  20  },
    { CdFileId::MAP39_LCD,  15  },
    { CdFileId::MAP40_LCD,  19  },
    { CdFileId::MAP41_LCD,  11  },
    { CdFileId::MAP42_LCD,  26  },
    { CdFileId::MAP43_LCD,  12  },
    { CdFileId::MAP44_LCD,  29  },
    { CdFileId::MAP45_LCD,  6   },
    { CdFileId::MAP46_LCD,  27  },
    { CdFileId::MAP47_LCD,  9   },
    { CdFileId::MAP48_LCD,  22  },
    { CdFileId::MAP49_LCD,  3   },
    { CdFileId::MAP50_LCD,  28  },
    { CdFileId::MAP51_LCD,  7   },
    { CdFileId::MAP52_LCD,  8   },
    { CdFileId::MAP53_LCD,  15  },
    { CdFileId::MAP54_LCD,  4   },
    { CdFileId::MAP55_LCD,  17  },
    { CdFileId::MAP56_LCD,  18  },
    { CdFileId::MAP57_LCD,  10  },
    { CdFileId::MAP58_LCD,  16  },
    { CdFileId::MAP59_LCD,  13  },
    { CdFileId::MAP60_LCD,  0   },
    { CdFileId{},           0   }
};

// What sound LCD file and music track to use for all maps in the game: Final Doom
static mapaudiodef_t gMapAudioDefs_FinalDoom[33] = {
    { CdFileId{},           0   },
    { CdFileId::MAP01_LCD,  1   },
    { CdFileId::MAP02_LCD,  2   },
    { CdFileId::MAP03_LCD,  3   },
    { CdFileId::MAP04_LCD,  4   },
    { CdFileId::MAP05_LCD,  5   },
    { CdFileId::MAP06_LCD,  6   },
    { CdFileId::MAP07_LCD,  7   },
    { CdFileId::MAP08_LCD,  8   },
    { CdFileId::MAP09_LCD,  9   },
    { CdFileId::MAP10_LCD,  10  },
    { CdFileId::MAP11_LCD,  11  },
    { CdFileId::MAP12_LCD,  12  },
    { CdFileId::MAP13_LCD,  13  },
    { CdFileId::MAP14_LCD,  14  },
    { CdFileId::MAP15_LCD,  15  },
    { CdFileId::MAP16_LCD,  16  },
    { CdFileId::MAP17_LCD,  17  },
    { CdFileId::MAP18_LCD,  18  },
    { CdFileId::MAP19_LCD,  19  },
    { CdFileId::MAP20_LCD,  20  },
    { CdFileId::MAP21_LCD,  21  },
    { CdFileId::MAP22_LCD,  22  },
    { CdFileId::MAP23_LCD,  23  },
    { CdFileId::MAP24_LCD,  24  },
    { CdFileId::MAP25_LCD,  25  },
    { CdFileId::MAP26_LCD,  26  },
    { CdFileId::MAP27_LCD,  27  },
    { CdFileId::MAP28_LCD,  28  },
    { CdFileId::MAP29_LCD,  29  },
    { CdFileId::MAP30_LCD,  30  },
    { CdFileId::MAP60_LCD,  0   },
    { CdFileId{},           0   }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Platform implementation for starting a sound.
//
// Note: this function originally took 3 parameters, but since the 3rd param was completely unused we cannnot infer what it was.
// I've just removed this unknown 3rd param here for this reimplementation, since it serves no purpose.
//------------------------------------------------------------------------------------------------------------------------------------------
static void I_StartSound(mobj_t* const pOrigin, const sfxenum_t soundId) noexcept {
    // Ignore the request if the sound sequence number is invalid
    if (soundId >= NUMSFX)
        return;

    // Grab the listener (player) and default the pan/volume for now
    mobj_t* pListener = gPlayers[gCurPlayerIndex].mo;
    int32_t vol = WESS_MAX_MASTER_VOL;
    int32_t pan = WESS_PAN_CENTER;
    const bool bAttenuateSound = (pOrigin && (pOrigin != pListener));

    if (bAttenuateSound) {
        // Figure out the approximate distance to the sound source and don't play if too far away
        const fixed_t dx = std::abs(pListener->x - pOrigin->x);
        const fixed_t dy = std::abs(pListener->y - pOrigin->y);
        const fixed_t approxDist = dx + dy - d_rshift<1>(std::min(dx, dy));

        if (approxDist > S_CLIPPING_DIST)
            return;

        // Figure out the relative angle to the player.
        // Not sure what the addition of UINT32_MAX is about, was in Linux Doom also but not commented.
        angle_t angle = R_PointToAngle2(pListener->x, pListener->y, pOrigin->x, pOrigin->y);

        if (angle <= pListener->angle) {
            angle += UINT32_MAX;
        }

        angle -= pListener->angle;

        // Figure out pan amount based on the angle
        {
            const fixed_t sina = gFineSine[angle >> ANGLETOFINESHIFT];
            pan = WESS_PAN_CENTER - d_rshift<1>(d_fixed_to_int(FixedMul(sina, S_STEREO_SWING)));
        }

        // Figure out volume level
        if (approxDist < S_CLOSE_DIST) {
            vol = WESS_MAX_MASTER_VOL;
        } else {
            vol = (d_fixed_to_int(S_CLIPPING_DIST - approxDist) * WESS_MAX_MASTER_VOL) / S_ATTENUATOR;
        }

        // If the origin is exactly where the player is then do no pan
        if ((pOrigin->x == pListener->x) && (pOrigin->y == pListener->y)) {
            pan = WESS_PAN_CENTER;
        }

        // If volume is zero then don't play
        if (vol <= 0)
            return;
    }

    // Disable reverb if the origin is in a sector that forbids it
    TriggerPlayAttr sndAttribs;

    if (pOrigin && (pOrigin->subsector->sector->flags & SF_NO_REVERB)) {
        sndAttribs.reverb = 0;
    } else {
        sndAttribs.reverb = WESS_MAX_REVERB_DEPTH;
    }

    // Set the other sound attributes and play the sound.
    // Note that the original code also wrote volume and pan to unused globals here but I've omitted that code since it is useless:
    sndAttribs.attribs_mask = TRIGGER_VOLUME | TRIGGER_PAN | TRIGGER_REVERB;
    sndAttribs.volume_cntrl = (uint8_t) vol;
    sndAttribs.pan_cntrl = (uint8_t) pan;

    wess_seq_trigger_type_special(soundId, (uintptr_t) pOrigin, &sndAttribs);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop the currently playing music track
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StopMusic() noexcept {
    if (gCurMusicSeqIdx != 0) {
        const musicseq_t& musicSeqDef = (Game::isFinalDoom()) ? gMusicSeqDefs_FinalDoom[gCurMusicSeqIdx] : gMusicSeqDefs_Doom[gCurMusicSeqIdx];
        wess_seq_stop(musicSeqDef.seqIdx);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Start playing the selected music track (stops if currently playing)
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StartMusic() noexcept {
    S_StopMusic();

    if (gCurMusicSeqIdx != 0) {
        const musicseq_t& musicSeqDef = (Game::isFinalDoom()) ? gMusicSeqDefs_FinalDoom[gCurMusicSeqIdx] : gMusicSeqDefs_Doom[gCurMusicSeqIdx];
        wess_seq_trigger(musicSeqDef.seqIdx);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads all sound and music for the given map number (Note: '0' if menu, '60' if finale).
// PsyDoom: this function has been rewritten. For the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void S_LoadMapSoundAndMusic(const int32_t mapIdx) noexcept {
    // Final Doom has different music and map sounds
    const musicseq_t* const pMusicSeqDefs = (Game::isFinalDoom()) ? gMusicSeqDefs_FinalDoom : gMusicSeqDefs_Doom;

    // If sound and music is already loaded then bail out
    if (gLoadedSoundAndMusMapNum == mapIdx)
        return;

    // Stop current map music, free all music sequences and unload map SFX
    if (gLoadedSoundAndMusMapNum != 0) {
        if (gCurMusicSeqIdx != 0) {
            S_StopMusic();
            Utils::waitUntilSeqEnteredStatus(pMusicSeqDefs[gCurMusicSeqIdx].seqIdx, SequenceStatus::SEQUENCE_INACTIVE);
            wess_seq_range_free(0 + NUMSFX, (Game::isFinalDoom()) ? NUM_MUSIC_SEQS_FINAL_DOOM : NUM_MUSIC_SEQS_DOOM);
        }

        S_UnloadSampleBlock(gMapSndBlock);
    }

    // Either load or unload the main Doom SFX LCD
    const int32_t finaleMapNum = Game::getNumMaps() + 1;

    if (mapIdx == finaleMapNum) {
        // For the finale unload this LCD to free up SPU RAM, since a lot will be needed to hold all monster sounds
        S_UnloadSampleBlock(gDoomSndBlock);
        gbDidLoadDoomSfxLcd = false;
        gSound_MapLcdSpuStartAddr = SPU_RAM_APP_BASE;
    } else {
        // In all other cases ensure it is loaded
        if (!gbDidLoadDoomSfxLcd) {
            gSound_MapLcdSpuStartAddr = SPU_RAM_APP_BASE + wess_dig_lcd_load(CdFile::DOOMSFX_LCD, SPU_RAM_APP_BASE, &gDoomSndBlock, false);
            gbDidLoadDoomSfxLcd = true;
        }
    }

    // Load the music sequence and lcd file for the map music.
    // Also initialize the reverb mode depending on the music.
    const mapaudiodef_t& mapAudioDef = (Game::isFinalDoom()) ? gMapAudioDefs_FinalDoom[mapIdx] : gMapAudioDefs_Doom[mapIdx];
    gCurMusicSeqIdx = mapAudioDef.musicSeqIdx;
    uint32_t destSpuAddr = gSound_MapLcdSpuStartAddr;

    if (gCurMusicSeqIdx == 0) {
        // No music sequences for this map - turn off reverb
        psxspu_init_reverb(SPU_REV_MODE_OFF, 0, 0, 0, 0);
    } else {
        // Normal case: playing a map music sequence and initializing reverb.
        // Note: incorporating a change Final Doom made for all versions here, mute all audio first to prevent artifacts when initializing reverb:
        const int32_t masterVol = psxspu_get_master_vol();
        psxspu_set_master_vol(0);

        const musicseq_t& musicseq = pMusicSeqDefs[gCurMusicSeqIdx];
        psxspu_init_reverb(musicseq.reverbMode, musicseq.reverbDepthL, musicseq.reverbDepthR, 0, 0);
        wess_seq_load(musicseq.seqIdx, gpSound_MusicSeqData);
        destSpuAddr += wess_dig_lcd_load(musicseq.lcdFile, destSpuAddr, &gMapSndBlock, false);

        // Restore the master volume to what it was.
        // Again, this mute/restore was only added in Final Doom, but I'm going to do it for all Doom versions:
        psxspu_set_master_vol(masterVol);
    }

    // Remember what map we have loaded sound and music for
    gLoadedSoundAndMusMapNum = mapIdx;

    // Load the sound LCD file for the map
    if (mapAudioDef.sfxLcdFile != CdFileId{}) {
        wess_dig_lcd_load(mapAudioDef.sfxLcdFile, destSpuAddr, &gMapSndBlock, false);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the sound effect with the given id.
// The origin parameter is optional and helps determine panning/attenuation.
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StartSound(mobj_t* const pOrigin, const sfxenum_t soundId) noexcept {
    I_StartSound(pOrigin, soundId);
}

#endif  // #if !PSYDOOM_MODS
