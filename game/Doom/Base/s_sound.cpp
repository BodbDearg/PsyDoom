#include "s_sound.h"

#include "PsxVm/PsxVm.h"

void S_SetSfxVolume() noexcept {
loc_80040FAC:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 &= 0xFF;
    wess_master_sfx_vol_set();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void S_SetMusicVolume() noexcept {
loc_80040FCC:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a0 = s0 & 0xFF;
    wess_master_mus_vol_set();
    a0 = s0 << 7;
    v0 = (i32(a0) < 0x3D00);
    if (v0 != 0) goto loc_80040FF8;
    a0 = 0x3CFF;                                        // Result = 00003CFF
loc_80040FF8:
    psxspu_set_cd_vol();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void S_StopMusicSequence() noexcept {
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
    wess_seq_stop();
loc_80041040:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void S_StartMusicSequence() noexcept {
loc_80041050:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    S_StopMusicSequence();
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
    wess_seq_trigger();
loc_80041088:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void ZeroHalfWord() noexcept {
loc_80041098:
    sh(0, a0);
    return;
}

void S_UnloadSamples() noexcept {
loc_800410A0:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lhu(s0);
    s1 = 0xFFFF;                                        // Result = 0000FFFF
    if (v0 == 0) goto loc_80041100;
loc_800410C4:
    v0 = lhu(s0);
    v0 += s1;
    sh(v0, s0);
    v0 = lhu(s0);
    v0 <<= 1;
    v0 += s0;
    a0 = lhu(v0 + 0x2);
    a1 = 0;                                             // Result = 00000000
    wess_dig_set_sample_position();
    v0 = lhu(s0);
    if (v0 != 0) goto loc_800410C4;
loc_80041100:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
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
    S_StopMusicSequence();
loc_80041154:
    v0 = lw(gp + 0x834);                                // Load from: gCurMapMusicNum (80077E14)
    v0 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54E0;                                       // Result = MapMusicDefs[1] (800754E0)
    at += v0;
    a0 = lw(at);
    wess_seq_status();
    a0 = 0x5A;                                          // Result = 0000005A
    if (v0 != s0) goto loc_80041154;
    a1 = 0x14;                                          // Result = 00000014
    wess_seq_range_free();
loc_80041188:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x11D0;                                       // Result = gMapMusSfxLoadedSamples[0] (8007EE30)
    S_UnloadSamples();
    v0 = 0x3C;                                          // Result = 0000003C
loc_8004119C:
    if (s1 != v0) goto loc_800411C8;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x1364;                                       // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    S_UnloadSamples();
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
    wess_dig_lcd_load();
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
    psxspu_init_reverb();
    v0 = lw(gp + 0x834);                                // Load from: gCurMapMusicNum (80077E14)
    a1 = lw(gp + 0x83C);                                // Load from: gpMusSequencesEnd (80077E1C)
    v0 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x54E0;                                       // Result = MapMusicDefs[1] (800754E0)
    at += v0;
    a0 = lw(at);
    wess_seq_load();
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
    wess_dig_lcd_load();
    s0 += v0;
    goto loc_800412D0;
loc_800412B8:
    sw(0, sp + 0x10);
    a0 = 0;                                             // Result = 00000000
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    psxspu_init_reverb();
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
    wess_dig_lcd_load();
loc_80041300:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void S_Pause() noexcept {
loc_80041318:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = 0x80080000;                                    // Result = 80080000
    a1 -= 0x14E8;                                       // Result = gSavedMusVoiceState[0] (8007EB18)
    a0 = 1;                                             // Result = 00000001
    queue_wess_seq_pauseall();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void S_Resume() noexcept {
loc_80041340:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x14E8;                                       // Result = gSavedMusVoiceState[0] (8007EB18)
    queue_wess_seq_restartall();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void S_StopSound() noexcept {
loc_80041368:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    wess_seq_stoptype();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void S_Clear() noexcept {
loc_80041388:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    wess_seq_stopall();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
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
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
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
    R_PointToAngle2();
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
    wess_seq_trigger_type_special();
loc_80041594:
    ra = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x40;
    return;
}

void S_StartSound() noexcept {
loc_800415B4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a2 = 0;                                             // Result = 00000000
    I_StartSound();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void S_UpdateSounds() noexcept {
loc_800415D4:
    v0 = lw(gp + 0x84C);                                // Load from: gNumSoundTics (80077E2C)
    v0++;
    sw(v0, gp + 0x84C);                                 // Store to: gNumSoundTics (80077E2C)
    return;
}
