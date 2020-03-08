//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBSPU' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------

#include "LIBSPU.h"

#include "LIBAPI.h"
#include "LIBC2.h"
#include "LIBETC.h"
#include "PcPsx/Macros.h"

BEGIN_THIRD_PARTY_INCLUDES

#include <cstdint>
#include <device/spu/spu.h>
#include <system.h>

END_THIRD_PARTY_INCLUDES

// N.B: must be done LAST due to MIPS register macros
#include "PsxVm/PsxVm.h"

// How big the sound RAM is available to the SPU
static constexpr uint32_t SPU_RAM_SIZE = 512 * 1024;

// The current reverb mode in use
static SpuReverbMode gReverbMode = SPU_REV_MODE_OFF;

// The 'base' note for each voice: this is the musical note at which the sample rate is considered to be 44,100 Hz.
// The actual semitone is encoded in the top 8-bits, the 1/128 semitone fraction is encoded in the low 8-bits.
//
// LIBSPU keeps track of this because it needs this reference note when converting notes to pitch.
// See the implementation of 'LIBSPU__spu_note2pitch' for more details on that.
static uint16_t gVoiceBaseNotes[SPU_NUM_VOICES] = {};

// Internal LIBSPU function: convert a note to a pitch.
// See definition for details.
uint16_t LIBSPU__spu_note2pitch(
    const int32_t centerNote,
    const int32_t centerNoteFrac,
    const int32_t offsetNote,
    const int32_t offsetNoteFrac
) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Set one or more (or all) properties on a voice or voices using the information in the given struct
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuSetVoiceAttr(const SpuVoiceAttr& attribs) noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;

    // Figure out what attributes to set for the specified voices
    const uint32_t attribMask = attribs.attr_mask;
    
    const bool bSetAllAttribs   = (attribMask == 0);
    const bool bSetPitch        = (bSetAllAttribs || (attribMask & SPU_VOICE_PITCH));
    const bool bSetBaseNote     = (bSetAllAttribs || (attribMask & SPU_VOICE_SAMPLE_NOTE));
    const bool bSetNote         = (bSetAllAttribs || (attribMask & SPU_VOICE_NOTE));
    const bool bSetWaveAddr     = (bSetAllAttribs || (attribMask & SPU_VOICE_WDSA));
    const bool bSetAttackRate   = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_AR));
    const bool bSetAttackMode   = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_AMODE));
    const bool bSetDecayRate    = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_DR));
    const bool bSetSustainLevel = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_SL));
    const bool bSetSustainRate  = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_SR));
    const bool bSetSustainMode  = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_SMODE));
    const bool bSetReleaseRate  = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_RR));
    const bool bSetReleaseMode  = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_RMODE));
    const bool bSetAdsrPart1    = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_ADSR1));
    const bool bSetAdsrPart2    = (bSetAllAttribs || (attribMask & SPU_VOICE_ADSR_ADSR2));
    const bool bSetWaveLoopAddr = (bSetAllAttribs || (attribMask & SPU_VOICE_LSAX));
    const bool bSetVolL         = (bSetAllAttribs || (attribMask & SPU_VOICE_VOLL));
    const bool bSetVolModeL     = (bSetAllAttribs || (attribMask & SPU_VOICE_VOLMODEL));
    const bool bSetVolR         = (bSetAllAttribs || (attribMask & SPU_VOICE_VOLR));
    const bool bSetVolModeR     = (bSetAllAttribs || (attribMask & SPU_VOICE_VOLMODER));

    // Set the required attributes for all specified voices
    const uint32_t voiceBits = attribs.voice_bits;

    for (int32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
        // Skip this voice if we're not setting its attributes
        if ((voiceBits & (1 << voiceIdx)) == 0)
            continue;

        // Set: voice 'pitch' or sample rate. Note that '4,096' = '44,100 Hz'.
        spu::Voice& voice = spu.voices[voiceIdx];

        if (bSetPitch) {
            voice.sampleRate._reg = attribs.pitch;
        }

        // Set: voice 'base' note at which the sample rate is regarded to be '44,100 Hz'
        if (bSetBaseNote) {
            gVoiceBaseNotes[voiceIdx] = attribs.sample_note;
        }

        // Set: voice pitch or sample rate from a musical note
        if (bSetNote) {
            // Note: the high 8 bits of these 'note' fields contain the actual semitone, the low 8-bits are 1/128 semitone increments
            const uint16_t baseNote = gVoiceBaseNotes[voiceIdx];
            const uint16_t note = attribs.note;
            const uint16_t sampleRate = LIBSPU__spu_note2pitch(baseNote >> 8, baseNote & 0xFF, note >> 8, note & 0xFF);
            voice.sampleRate._reg = sampleRate;
        }

        // Set: the start address (64-bit word index) for the voice wave data.
        // If the given address is not 64-bit aligned then it is aligned up to the next 64-bit boundary.
        if (bSetWaveAddr) {
            const uint32_t addr = (attribs.addr + 7) / 8;
            voice.startAddress._reg = (uint16_t) addr;
        }

        // Set: attack rate
        if (bSetAttackRate) {
            const uint32_t attackRate = (attribs.ar < 0x7F) ? attribs.ar : 0x7F;
            voice.adsr.attackStep = (attackRate & 0b0000'0011);
            voice.adsr.releaseShift = (attackRate & 0b0111'1100) >> 2;

            // Set: attack rate mode (exponential or not).
            // If not specified then default to 'linear' increase mode.
            if (bSetAttackMode) {
                voice.adsr.attackMode = (attribs.a_mode == SPU_VOICE_EXPIncN) ? 1 : 0;
            } else {
                voice.adsr.attackMode = 0;
            }
        }

        // Set: decay rate
        if (bSetDecayRate) {
            voice.adsr.decayShift = (attribs.dr < 0xF) ? attribs.dr : 0xF;
        }

        // Set: sustain level
        if (bSetSustainLevel) {
            voice.adsr.sustainLevel = (attribs.sl < 0xF) ? attribs.sl : 0xF;
        }

        // Set: sustain rate
        if (bSetSustainRate) {
            const uint32_t sustainRate = (attribs.sr < 0x7F) ? attribs.sr : 0x7F;
            voice.adsr.sustainStep = (sustainRate & 0b0000'0011);
            voice.adsr.sustainShift = (sustainRate & 0b0111'1100) >> 2;

            // Set: sustain rate mode (increase and exponential or not).
            // If not specified then default to 'increase' and NOT 'exponential' mode.
            if (bSetSustainMode) {
                uint8_t dir     = 0;    // 0, 1: increase / decrease
                uint8_t mode    = 0;    // 0, 1: linear / exponential

                // Some of this doesn't make sense to me, but it's what I observed in the original machine code for this function...
                switch (attribs.s_mode) {
                    case SPU_VOICE_DIRECT:      dir = 1;    mode = 0; break;
                    case SPU_VOICE_LINEARIncN:  dir = 0;    mode = 0; break;
                    case SPU_VOICE_LINEARIncR:  dir = 1;    mode = 0; break;
                    case SPU_VOICE_LINEARDecN:  dir = 1;    mode = 0; break;
                    case SPU_VOICE_LINEARDecR:  dir = 1;    mode = 0; break;
                    case SPU_VOICE_EXPIncN:     dir = 0;    mode = 1; break;
                    case SPU_VOICE_EXPIncR:     dir = 1;    mode = 0; break;
                    case SPU_VOICE_EXPDec:      dir = 1;    mode = 1; break;

                    default: break;
                }

                voice.adsr.sustainDirection = dir;
                voice.adsr.sustainMode = mode;
            } else {
                voice.adsr.sustainDirection = 0;
                voice.adsr.sustainMode = 0;
            }
        }

        // Set: release rate
        if (bSetReleaseRate) {
            const uint32_t releaseRate = (attribs.rr < 0x1F) ? attribs.rr : 0x1F;
            voice.adsr.releaseShift = releaseRate;

            // Set: release rate mode (exponential or not).
            // If not specified then default to 'linear' mode.
            voice.adsr.releaseMode = 0;

            if (bSetReleaseMode) {
                if (attribs.r_mode == SPU_VOICE_EXPDec) {
                    voice.adsr.releaseMode = 1;
                }
            }
        }

        // Set: envelope ADSR directly (1st and 2nd 16-bits)
        if (bSetAdsrPart1) {
            voice.adsr._reg &= 0xFFFF0000;
            voice.adsr._reg |= (uint32_t) attribs.adsr1;
        }

        if (bSetAdsrPart2) {
            voice.adsr._reg &= 0x0000FFFF;
            voice.adsr._reg |= ((uint32_t) attribs.adsr2) << 16;
        }

        // Set: wave loop address (64-bit word index).
        // If the given address is not 64-bit aligned then it is aligned up to the next 64-bit boundary.
        if (bSetWaveLoopAddr) {
            const uint32_t addr = (attribs.loop_addr + 7) / 8;
            voice.repeatAddress._reg = (uint16_t) addr;
        }

        // Set: left volume and mode
        if (bSetVolL) {
            const uint16_t mode = (bSetVolModeL) ? attribs.volmode.left : 0;

            if (mode == 0) {
                voice.volume.left = attribs.volume.left & 0x7FFF;
            } else {
                const uint16_t volBits = (attribs.volume.left < 0x7F) ? attribs.volume.left : 0x7F;
                const uint16_t modeBits = 0x8000 | (mode << 12);
                voice.volume.left = modeBits | volBits;
            }
        }

        // Set: right volume and mode
        if (bSetVolR) {
            const uint16_t mode = (bSetVolModeR) ? attribs.volmode.right : 0;

            if (mode == 0) {
                voice.volume.right = attribs.volume.right & 0x7FFF;
            } else {
                const uint16_t volBits = (attribs.volume.right < 0x7F) ? attribs.volume.right : 0x7F;
                const uint16_t modeBits = 0x8000 | (mode << 12);
                voice.volume.right = modeBits | volBits;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal LIBSPU function which converts a musical note to a frequency that can be set on a voice.
// The returned integer frequency is such that 4,096 units = 44,100 Hz.
// 
// Params:
//  baseNote:       Note at which the frequency is considered 44,100 Hz.
//                  For example '60' would be A5 (12 semitones per octave, 1st note of 5th octave).
//  baseNoteFrac:   Fractional offset to 'baseNote' in 1/128 units.
//  note:           The note to get the frequency for.
//  noteFrac:       Fractional offset to 'note' in 1/128 units.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t LIBSPU__spu_note2pitch(
    const int32_t baseNote,
    const int32_t baseNoteFrac,
    const int32_t note,
    const int32_t noteFrac
) noexcept {
    // Note: this implementation is COMPLETELY different to the original LIBSPU version.
    // That version used lookup tables of some sort to figure out the calculation.
    //
    // This re-implementation produces almost identical results, but is much easier to comprehend.
    // The trade off is that it's a good bit slower due to use of 'pow' - but that's not really a concern on modern systems.
    // I could take the time to fully understand and reverse the original method, but it's probably not worth it..
    const float baseNoteF   = baseNote + (float) baseNoteFrac / 128.0f;
    const float noteF       = note + (float) noteFrac / 128.0f;
    const float noteOffsetF = noteF - baseNoteF;

    // For a good explantion of the conversion from note to frequency, see:
    //  https://www.translatorscafe.com/unit-converter/en-US/calculator/note-frequency/
    // Note that the '4096.0' here represents 44,100 Hz - the base note frequency.
    const float freq = 4096.0f * std::powf(2.0f, noteOffsetF / 12.0f);

    if (freq > (float) UINT16_MAX) {
        return UINT16_MAX;
    }

    return (uint16_t) freq;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update one or more reverb parameters, including reverb mode.
// Note that if the reverb mode is changed, then the reverb depth is cleared to '0'.
// Returns 'SPU_ERROR' on failure, otherwise 'SPU_SUCCESS'.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBSPU_SpuSetReverbModeParam(const SpuReverbAttr& reverbAttr) noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;

    // If no attributes are set in the mask then the behavior is that ALL attributes are being updated
    const uint32_t attribMask = reverbAttr.mask;
    const bool bSetAllAttribs = (attribMask == 0);

    // Figure out what we are changing
    const bool bSetReverbMode           = (bSetAllAttribs || (attribMask & SPU_REV_MODE));
    const bool bSetReverbLeftDepth      = (bSetAllAttribs || (attribMask & SPU_REV_DEPTHL));
    const bool bSetReverbRightDepth     = (bSetAllAttribs || (attribMask & SPU_REV_DEPTHR));
    const bool bSetReverbDelay          = (bSetAllAttribs || (attribMask & SPU_REV_DELAYTIME));
    const bool bSetReverbFeedback       = (bSetAllAttribs || (attribMask & SPU_REV_FEEDBACK));
    const bool bClearReverbWorkingArea  = (reverbAttr.mode & SPU_REV_MODE_CLEAR_WA);

    // Set the new reverb mode (if changing) and grab the default reverb settings for whatever mode is now current
    if (bSetReverbMode) {
        const SpuReverbMode reverbMode = (SpuReverbMode)(reverbAttr.mode & (~SPU_REV_MODE_CLEAR_WA));   // Must remove the 'CLEAR_WA' (clear working area flag)

        if (reverbMode < SPU_REV_MODE_MAX) {
            gReverbMode = reverbMode;
            spu.reverbBase._reg = gReverbWorkAreaBaseAddrs[gReverbMode];    // Update the reverb working area base address when changing mode
        } else {
            // Bad reverb mode - this causes the call to fail!
            return SPU_ERROR;
        }
    }

    SpuReverbDef reverbDef = gReverbDefs[gReverbMode];

    // Initially we send all the fields to the SPU
    reverbDef.fieldBits = 0;

    // Manually adjust reverb delay time if the reverb mode allows it
    if (bSetReverbDelay) {
        if ((gReverbMode == SPU_REV_MODE_ECHO) || (gReverbMode == SPU_REV_MODE_DELAY)) {
            // Be more selective about what changes we make to SPU registers if we can
            if (!bSetReverbMode) {
                reverbDef.fieldBits = (
                    SPU_SAME_SIDE_REFRACT_ADDR_1_LEFT |
                    SPU_SAME_SIDE_REFRACT_ADDR_1_RIGHT |
                    SPU_COMB_ADDR_1_LEFT |
                    SPU_SAME_SIDE_REFRACT_ADDR_2_LEFT |
                    SPU_APF_ADDR_1_LEFT |
                    SPU_APF_ADDR_1_RIGHT
                );
            }

            // These are the tweaks that the original PsyQ SDK used by DOOM did based on custom delay parameters:
            const int32_t delay = reverbAttr.delay;
            const int16_t addrOffset = (int16_t)((delay * 4096) / 127);

            reverbDef.sameSideRefractAddr1Left = (int16_t)((delay * 8192) / 127) - reverbDef.apfOffset1;
            reverbDef.sameSideRefractAddr1Right = addrOffset - reverbDef.apfOffset2;
            reverbDef.sameSideRefractAddr2Left = addrOffset + reverbDef.sameSideRefractAddr2Right;
            reverbDef.combAddr1Left = addrOffset + reverbDef.combAddr1Right;
            reverbDef.apfAddr1Left = addrOffset + reverbDef.apfAddr2Left;
            reverbDef.apfAddr1Right = addrOffset + reverbDef.apfAddr2Right;
        }
    }

    // Manually adjust reverb feedback if the reverb mode allows it
    if (bSetReverbFeedback) {
        if ((gReverbMode == SPU_REV_MODE_ECHO) || (gReverbMode == SPU_REV_MODE_DELAY)) {
            // Be more selective about what changes we make to SPU registers if we can
            if (!bSetReverbMode) {
                reverbDef.fieldBits |= SPU_REVF_REFLECTION_VOLUME_2;
            }

            reverbDef.reflectionVolume2 = (uint16_t)((reverbAttr.feedback * 33024) / 127);
        }
    }

    // If the reverb mode is being set then we must disable master temporarily
    const bool bPrevMasterReverbEnabled = spu.control.masterReverb;
    spu.control.masterReverb = false;

    // Update the reverb depth.
    // Note that if the reverb mode is being set then LIBSPU must also set reverb depth temporarily to '0', as per the docs.
    // It's up to callers in that situation to call 'LIBSPU_SpuSetReverbDepth' to set the depth again after that...
    if (bSetReverbMode) {
        spu.reverbVolume.left = 0;
        spu.reverbVolume.right = 0;
    } else {
        if (bSetReverbLeftDepth) {
            spu.reverbVolume.left = reverbAttr.depth.left;
        }

        if (bSetReverbRightDepth) {
            spu.reverbVolume.right = reverbAttr.depth.right;
        }
    }
    
    // Update the SPU reverb registers if setting reverb mode, delay time or feedback.
    // If we are just updating the left/right reverb depth however, then we can skip this.
    if (bSetReverbMode || bSetReverbDelay || bSetReverbFeedback) {        
        const uint32_t regBits = reverbDef.fieldBits;
        const bool bSetAllRegs = (regBits == 0);
        
        const auto updateReg = [&](const uint32_t idx, const uint16_t value) noexcept {
            if (bSetAllRegs || (regBits & (1 << idx))) {
                spu.reverbRegisters[idx]._reg = value;
            }
        };

        updateReg(0,  reverbDef.apfOffset1);
        updateReg(1,  reverbDef.apfOffset2);
        updateReg(2,  reverbDef.reflectionVolume1);
        updateReg(3,  reverbDef.combVolume1);
        updateReg(4,  reverbDef.combVolume2);
        updateReg(5,  reverbDef.combVolume3);
        updateReg(6,  reverbDef.combVolume4);
        updateReg(7,  reverbDef.reflectionVolume2);
        updateReg(8,  reverbDef.apfVolume1);
        updateReg(9,  reverbDef.apfVolume2);
        updateReg(10, reverbDef.sameSideRefractAddr1Left);
        updateReg(11, reverbDef.sameSideRefractAddr1Right);
        updateReg(12, reverbDef.combAddr1Left);
        updateReg(13, reverbDef.combAddr1Right);
        updateReg(14, reverbDef.combAddr2Left);
        updateReg(15, reverbDef.combAddr2Right);
        updateReg(16, reverbDef.sameSideRefractAddr2Left);
        updateReg(17, reverbDef.sameSideRefractAddr2Right);
        updateReg(18, reverbDef.diffSideReflectAddr1Left);
        updateReg(19, reverbDef.diffSideReflectAddr1Right);
        updateReg(20, reverbDef.combAddr3Left);
        updateReg(21, reverbDef.combAddr3Right);
        updateReg(22, reverbDef.combAddr4Left);
        updateReg(23, reverbDef.combAddr4Right);
        updateReg(24, reverbDef.diffSideReflectAddr2Left);
        updateReg(25, reverbDef.diffSideReflectAddr2Right);
        updateReg(26, reverbDef.apfAddr1Left);
        updateReg(27, reverbDef.apfAddr1Right);
        updateReg(28, reverbDef.apfAddr2Left);
        updateReg(29, reverbDef.apfAddr2Right);
        updateReg(30, reverbDef.inputVolLeft);
        updateReg(31, reverbDef.inputVolRight);
    }
    
    // Clear the reverb working area if that was specified
    if (bClearReverbWorkingArea) {
        LIBSPU_SpuClearReverbWorkArea();
    }

    // Restore master reverb if we disabled it and return success
    spu.control.masterReverb = bPrevMasterReverbEnabled;
    return SPU_SUCCESS;
}

void LIBSPU__spu_init() noexcept {
loc_80051894:
    sp -= 0x48;
    sw(s1, sp + 0x3C);
    s1 = a0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A18);                               // Load from: 80076A18
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x40);
    sw(s0, sp + 0x38);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60EC);                                // Store to: gLIBSPU__spu_transMode (800860EC)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60F0);                                 // Store to: gLIBSPU__spu_addrMode (800860F0)
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at + 0x60E8);                                 // Store to: gLIBSPU__spu_tsa[0] (800860E8)
    v0 = lw(a0);
    v1 = 0xB0000;                                       // Result = 000B0000
    v0 |= v1;
    sw(v0, a0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    sh(0, v0 + 0x1AA);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_8005194C;
loc_8005190C:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_8005190C;
loc_8005194C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60E4);                                 // Store to: 800860E4
    v0 &= 0x7FF;
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_800519D4;
    }
loc_80051970:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60E4);                               // Load from: 800860E4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60E4);                                // Store to: 800860E4
    v0 = (i32(v0) < 0x1389);
    if (v0 != 0) goto loc_800519B4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1EB8;                                       // Result = STR_Sys_SPU_TO_Msg[0] (80011EB8)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1EC8;                                       // Result = STR_Sys_wait_reset_Msg[0] (80011EC8)
    LIBC2_printf();
    v0 = 2;                                             // Result = 00000002
    goto loc_800519D4;
loc_800519B4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    v0 &= 0x7FF;
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80051970;
    }
loc_800519D4:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x70DC);                                // Store to: gLIBSPU__spu_mem_mode (800A8F24)
    v0 = 3;                                             // Result = 00000003
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6E60);                                // Store to: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v0 = 4;                                             // Result = 00000004
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    s0 = 0xFFFF;                                        // Result = 0000FFFF
    sh(v0, v1 + 0x1AC);
    sh(0, v1 + 0x180);
    sh(0, v1 + 0x182);
    sh(0, v1 + 0x184);
    sh(0, v1 + 0x186);
    sh(s0, v1 + 0x18C);
    sh(s0, v1 + 0x18E);
    sh(0, v1 + 0x198);
    sh(0, v1 + 0x19A);
    v0 = 0;                                             // Result = 00000000
    if (s1 != 0) goto loc_80051DF4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x6A24;                                       // Result = 80076A24
    a1 = 0x10;                                          // Result = 00000010
    v0 = 0x200;                                         // Result = 00000200
    sh(0, v1 + 0x190);
    sh(0, v1 + 0x192);
    sh(0, v1 + 0x194);
    sh(0, v1 + 0x196);
    sh(0, v1 + 0x1B0);
    sh(0, v1 + 0x1B2);
    sh(0, v1 + 0x1B4);
    sh(0, v1 + 0x1B6);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x60E8);                                // Store to: gLIBSPU__spu_tsa[0] (800860E8)
    LIBSPU__spu_writeByIO();
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, sp + 0x10);
    v0 = 0x1F;                                          // Result = 0000001F
    sw(v0, sp + 0x14);
    v0 = 0x3FFF;                                        // Result = 00003FFF
    sh(v0, sp + 0x20);
    v0 = 0x200;                                         // Result = 00000200
    sh(0, sp + 0x18);
    sh(0, sp + 0x1A);
    sw(v0, sp + 0x24);
    sw(0, sp + 0x2C);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60F0);                                 // Store to: gLIBSPU__spu_addrMode (800860F0)
    a0 = sp + 0x10;
    LIBSPU__spu_setVoiceAttr();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x188);
    sh(s0, v1 + 0x188);
    v0 = lhu(v1 + 0x18A);
    v0 |= 0xFF;
    sh(v0, v1 + 0x18A);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051B24;
loc_80051AE4:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051AE4;
loc_80051B24:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051B84;
loc_80051B44:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051B44;
loc_80051B84:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051BE4;
loc_80051BA4:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051BA4;
loc_80051BE4:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051C44;
loc_80051C04:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051C04;
loc_80051C44:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x18C);
    v0 = 0xFFFF;                                        // Result = 0000FFFF
    sh(v0, v1 + 0x18C);
    v0 = lhu(v1 + 0x18E);
    v0 |= 0xFF;
    sh(v0, v1 + 0x18E);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051CD0;
loc_80051C90:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051C90;
loc_80051CD0:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051D30;
loc_80051CF0:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051CF0;
loc_80051D30:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051D90;
loc_80051D50:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051D50;
loc_80051D90:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051DF0;
loc_80051DB0:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051DB0;
loc_80051DF0:
    v0 = 0;                                             // Result = 00000000
loc_80051DF4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = 1;                                             // Result = 00000001
    at = 0x80090000;                                    // Result = 80090000
    sw(v1, at + 0x655C);                                // Store to: gLIBSPU__spu_inTransfer (8009655C)
    v1 = 0xC000;                                        // Result = 0000C000
    sh(v1, a0 + 0x1AA);
    at = 0x80090000;                                    // Result = 80090000
    sw(0, at + 0x7C20);                                 // Store to: gLIBSPU__spu_transferCallback (80097C20)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x652C);                                 // Store to: 8008652C
    ra = lw(sp + 0x40);
    s1 = lw(sp + 0x3C);
    s0 = lw(sp + 0x38);
    sp += 0x48;
    return;
}

void LIBSPU__spu_writeByIO() noexcept {
loc_80051E98:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 + 0x60E8);                              // Load from: gLIBSPU__spu_tsa[0] (800860E8)
    sp -= 0x38;
    sw(s1, sp + 0x24);
    s1 = a1;
    sw(ra, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s0, sp + 0x20);
    a1 = lhu(v1 + 0x1AE);
    s2 = a0;
    sh(v0, v1 + 0x1A6);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x14);
    sw(0, sp + 0x10);
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    s4 = a1 & 0x7FF;
    if (v0 == 0) goto loc_80051F34;
loc_80051EF4:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051EF4;
loc_80051F34:
    v0 = (s1 < 0x41);
    if (s1 == 0) goto loc_8005211C;
    s3 = 0xD;                                           // Result = 0000000D
loc_80051F40:
    s0 = s1;
    if (v0 != 0) goto loc_80051F4C;
    s0 = 0x40;                                          // Result = 00000040
loc_80051F4C:
    v1 = 0;                                             // Result = 00000000
    if (i32(s0) <= 0) goto loc_80051F78;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
loc_80051F5C:
    v0 = lhu(s2);
    s2 += 2;
    v1 += 2;
    sh(v0, a0 + 0x1A8);
    v0 = (i32(v1) < i32(s0));
    if (v0 != 0) goto loc_80051F5C;
loc_80051F78:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v1 + 0x1AA);
    v0 = a0 & 0xFFCF;
    a0 = v0 | 0x10;
    sh(a0, v1 + 0x1AA);
    sw(s3, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_80051FD0;
loc_80051FA8:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_80051FD0:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051FA8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60E4);                                 // Store to: 800860E4
    v0 &= 0x400;
    if (v0 == 0) goto loc_80052070;
loc_8005200C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60E4);                               // Load from: 800860E4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60E4);                                // Store to: 800860E4
    v0 = (i32(v0) < 0x1389);
    if (v0 != 0) goto loc_80052050;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1EB8;                                       // Result = STR_Sys_SPU_TO_Msg[0] (80011EB8)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1ED8;                                       // Result = STR_Sys_wait_wrdy_H_Msg[0] (80011ED8)
    LIBC2_printf();
    goto loc_80052070;
loc_80052050:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    v0 &= 0x400;
    if (v0 != 0) goto loc_8005200C;
loc_80052070:
    sw(s3, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_800520A8;
loc_80052080:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_800520A8:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80052080;
    sw(s3, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_800520F8;
loc_800520D0:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_800520F8:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_800520D0;
    s1 -= s0;
    v0 = (s1 < 0x41);
    if (s1 != 0) goto loc_80051F40;
loc_8005211C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v0 + 0x1AA);
    a0 &= 0xFFCF;
    sh(a0, v0 + 0x1AA);
    v0 = lhu(v0 + 0x1AE);
    a1 = s4 & 0xFFFF;
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60E4);                                 // Store to: 800860E4
    v0 &= 0x7FF;
    if (v0 == a1) goto loc_800521B8;
loc_80052154:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60E4);                               // Load from: 800860E4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60E4);                                // Store to: 800860E4
    v0 = (i32(v0) < 0x1389);
    if (v0 != 0) goto loc_80052198;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1EB8;                                       // Result = STR_Sys_SPU_TO_Msg[0] (80011EB8)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1EEC;                                       // Result = STR_Sys_wait_dmaf_clear_Msg[0] (80011EEC)
    LIBC2_printf();
    goto loc_800521B8;
loc_80052198:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    v0 &= 0x7FF;
    if (v0 != a1) goto loc_80052154;
loc_800521B8:
    ra = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}

void LIBSPU__spu_setVoiceAttr() noexcept {
loc_800531EC:
    t2 = 0;                                             // Result = 00000000
    t1 = lw(a0 + 0x4);
    t3 = 0x800B0000;                                    // Result = 800B0000
    t3 = lw(t3 - 0x70DC);                               // Load from: gLIBSPU__spu_mem_mode (800A8F24)
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    t0 = (t1 < 1);
    t5 = t1 & 0x10;
    t4 = a3;
loc_80053210:
    v0 = 1;                                             // Result = 00000001
    v1 = lw(a0);
    v0 = v0 << t2;
    v0 &= v1;
    if (v0 == 0) goto loc_80053354;
    a2 = t2 << 3;
    if (t0 != 0) goto loc_8005323C;
    v0 = t1 & 1;
    if (v0 == 0) goto loc_80053248;
loc_8005323C:
    v0 = lhu(a0 + 0x8);
    sh(v0, t4);
loc_80053248:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_8005325C;
    v0 = t1 & 2;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 << 1;
        if (bJump) goto loc_80053268;
    }
loc_8005325C:
    v1 = lhu(a0 + 0xA);
    v0 += a3;
    sh(v1, v0 + 0x2);
loc_80053268:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_8005327C;
    v0 = t1 & 4;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 << 1;
        if (bJump) goto loc_80053288;
    }
loc_8005327C:
    v1 = lhu(a0 + 0x10);
    v0 += a3;
    sh(v1, v0 + 0x4);
loc_80053288:
    v0 = t1 & 8;
    if (t0 != 0) goto loc_80053298;
    if (v0 == 0) goto loc_800532FC;
loc_80053298:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60F0);                               // Load from: gLIBSPU__spu_addrMode (800860F0)
    {
        const bool bJump = (v0 != 0);
        v0 = a2 << 1;
        if (bJump) goto loc_800532B8;
    }
    v1 = lhu(a0 + 0x14);
    v0 += a3;
    goto loc_800532F8;
loc_800532B8:
    a1 = lw(a0 + 0x14);
    if (t3 == 0) goto loc_800532E8;
    divu(a1, t3);
    if (t3 != 0) goto loc_800532D4;
    _break(0x1C00);
loc_800532D4:
    v0 = hi;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 << 1;
        if (bJump) goto loc_800532E8;
    }
    a1 += t3;
loc_800532E8:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v0 += a3;
    v1 = a1 >> v1;
loc_800532F8:
    sh(v1, v0 + 0x6);
loc_800532FC:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_8005330C;
    if (t5 == 0) goto loc_80053318;
loc_8005330C:
    v1 = lhu(a0 + 0x1E);
    v0 += a3;
    sh(v1, v0 + 0x8);
loc_80053318:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_80053328;
    if (t5 == 0) goto loc_80053334;
loc_80053328:
    v1 = lhu(a0 + 0x1C);
    v0 += a3;
    sh(v1, v0 + 0xA);
loc_80053334:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_80053348;
    v0 = t1 & 0x100;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 << 1;
        if (bJump) goto loc_80053354;
    }
loc_80053348:
    v1 = lhu(a0 + 0x18);
    v0 += a3;
    sh(v1, v0 + 0xE);
loc_80053354:
    t2++;
    v0 = (i32(t2) < 0x18);
    t4 += 0x10;
    if (v0 != 0) goto loc_80053210;
    return;
}

void LIBSPU_SpuSetCommonAttr() noexcept {
loc_80053DA0:
    t0 = lw(a0);
    t1 = (t0 < 1);
    sp -= 0x10;
    if (t1 != 0) goto loc_80053DC8;
    v0 = t0 & 1;
    {
        const bool bJump = (v0 == 0);
        v0 = t0 & 4;
        if (bJump) goto loc_80053E7C;
    }
    if (v0 == 0) goto loc_80053E30;
loc_80053DC8:
    v1 = lh(a0 + 0x8);
    v0 = (v1 < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80053E30;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x1F14;                                       // Result = JumpTable_LIBSPU_SpuSetCommonAttr_1[0] (80011F14)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80053E30: goto loc_80053E30;
        case 0x80053DF8: goto loc_80053DF8;
        case 0x80053E00: goto loc_80053E00;
        case 0x80053E08: goto loc_80053E08;
        case 0x80053E10: goto loc_80053E10;
        case 0x80053E18: goto loc_80053E18;
        case 0x80053E20: goto loc_80053E20;
        case 0x80053E28: goto loc_80053E28;
        default: jump_table_err(); break;
    }
loc_80053DF8:
    a1 = 0x8000;                                        // Result = 00008000
    goto loc_80053E38;
loc_80053E00:
    a1 = 0x9000;                                        // Result = 00009000
    goto loc_80053E38;
loc_80053E08:
    a1 = 0xA000;                                        // Result = 0000A000
    goto loc_80053E38;
loc_80053E10:
    a1 = 0xB000;                                        // Result = 0000B000
    goto loc_80053E38;
loc_80053E18:
    a1 = 0xC000;                                        // Result = 0000C000
    goto loc_80053E38;
loc_80053E20:
    a1 = 0xD000;                                        // Result = 0000D000
    goto loc_80053E38;
loc_80053E28:
    a1 = 0xE000;                                        // Result = 0000E000
    goto loc_80053E38;
loc_80053E30:
    a3 = lhu(a0 + 0x4);
    a1 = 0;                                             // Result = 00000000
loc_80053E38:
    v0 = a3 & 0x7FFF;
    if (a1 == 0) goto loc_80053E6C;
    a2 = lh(a0 + 0x4);
    v0 = (i32(a2) < 0x80);
    v1 = a2;
    if (v0 != 0) goto loc_80053E5C;
    a3 = 0x7F;                                          // Result = 0000007F
    goto loc_80053E68;
loc_80053E5C:
    a3 = v1;
    if (i32(a2) >= 0) goto loc_80053E68;
    a3 = 0;                                             // Result = 00000000
loc_80053E68:
    v0 = a3 & 0x7FFF;
loc_80053E6C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 |= a1;
    sh(v0, v1 + 0x180);
loc_80053E7C:
    v0 = t0 & 2;
    if (t1 != 0) goto loc_80053E94;
    {
        const bool bJump = (v0 == 0);
        v0 = t0 & 8;
        if (bJump) goto loc_80053F48;
    }
    if (v0 == 0) goto loc_80053EFC;
loc_80053E94:
    v1 = lh(a0 + 0xA);
    v0 = (v1 < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80053EFC;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x1F34;                                       // Result = JumpTable_LIBSPU_SpuSetCommonAttr_2[0] (80011F34)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80053EFC: goto loc_80053EFC;
        case 0x80053EC4: goto loc_80053EC4;
        case 0x80053ECC: goto loc_80053ECC;
        case 0x80053ED4: goto loc_80053ED4;
        case 0x80053EDC: goto loc_80053EDC;
        case 0x80053EE4: goto loc_80053EE4;
        case 0x80053EEC: goto loc_80053EEC;
        case 0x80053EF4: goto loc_80053EF4;
        default: jump_table_err(); break;
    }
loc_80053EC4:
    a1 = 0x8000;                                        // Result = 00008000
    goto loc_80053F04;
loc_80053ECC:
    a1 = 0x9000;                                        // Result = 00009000
    goto loc_80053F04;
loc_80053ED4:
    a1 = 0xA000;                                        // Result = 0000A000
    goto loc_80053F04;
loc_80053EDC:
    a1 = 0xB000;                                        // Result = 0000B000
    goto loc_80053F04;
loc_80053EE4:
    a1 = 0xC000;                                        // Result = 0000C000
    goto loc_80053F04;
loc_80053EEC:
    a1 = 0xD000;                                        // Result = 0000D000
    goto loc_80053F04;
loc_80053EF4:
    a1 = 0xE000;                                        // Result = 0000E000
    goto loc_80053F04;
loc_80053EFC:
    t2 = lhu(a0 + 0x6);
    a1 = 0;                                             // Result = 00000000
loc_80053F04:
    v0 = t2 & 0x7FFF;
    if (a1 == 0) goto loc_80053F38;
    a2 = lh(a0 + 0x6);
    v0 = (i32(a2) < 0x80);
    v1 = a2;
    if (v0 != 0) goto loc_80053F28;
    t2 = 0x7F;                                          // Result = 0000007F
    goto loc_80053F34;
loc_80053F28:
    t2 = v1;
    if (i32(a2) >= 0) goto loc_80053F34;
    t2 = 0;                                             // Result = 00000000
loc_80053F34:
    v0 = t2 & 0x7FFF;
loc_80053F38:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 |= a1;
    sh(v0, v1 + 0x182);
loc_80053F48:
    v0 = t0 & 0x40;
    if (t1 != 0) goto loc_80053F58;
    if (v0 == 0) goto loc_80053F6C;
loc_80053F58:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 = lhu(a0 + 0x10);
    sh(v0, v1 + 0x1B0);
loc_80053F6C:
    v0 = t0 & 0x80;
    if (t1 != 0) goto loc_80053F7C;
    if (v0 == 0) goto loc_80053F90;
loc_80053F7C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 = lhu(a0 + 0x12);
    sh(v0, v1 + 0x1B2);
loc_80053F90:
    v0 = t0 & 0x100;
    if (t1 != 0) goto loc_80053FA0;
    if (v0 == 0) goto loc_80053FE4;
loc_80053FA0:
    v0 = lw(a0 + 0x14);
    if (v0 != 0) goto loc_80053FC8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFFB;
    goto loc_80053FE0;
loc_80053FC8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 |= 4;
loc_80053FE0:
    sh(v1, v0 + 0x1AA);
loc_80053FE4:
    v0 = t0 & 0x200;
    if (t1 != 0) goto loc_80053FF4;
    if (v0 == 0) goto loc_80054038;
loc_80053FF4:
    v0 = lw(a0 + 0x18);
    if (v0 != 0) goto loc_8005401C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFFE;
    goto loc_80054034;
loc_8005401C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 |= 1;
loc_80054034:
    sh(v1, v0 + 0x1AA);
loc_80054038:
    v0 = t0 & 0x400;
    if (t1 != 0) goto loc_80054048;
    if (v0 == 0) goto loc_8005405C;
loc_80054048:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 = lhu(a0 + 0x1C);
    sh(v0, v1 + 0x1B4);
loc_8005405C:
    v0 = t0 & 0x800;
    if (t1 != 0) goto loc_8005406C;
    if (v0 == 0) goto loc_80054080;
loc_8005406C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 = lhu(a0 + 0x1E);
    sh(v0, v1 + 0x1B6);
loc_80054080:
    v0 = t0 & 0x1000;
    if (t1 != 0) goto loc_80054090;
    if (v0 == 0) goto loc_800540D4;
loc_80054090:
    v0 = lw(a0 + 0x20);
    if (v0 != 0) goto loc_800540B8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFF7;
    goto loc_800540D0;
loc_800540B8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 |= 8;
loc_800540D0:
    sh(v1, v0 + 0x1AA);
loc_800540D4:
    v0 = t0 & 0x2000;
    if (t1 != 0) goto loc_800540E4;
    if (v0 == 0) goto loc_80054128;
loc_800540E4:
    v0 = lw(a0 + 0x24);
    if (v0 != 0) goto loc_8005410C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFFD;
    goto loc_80054124;
loc_8005410C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 |= 2;
loc_80054124:
    sh(v1, v0 + 0x1AA);
loc_80054128:
    sp += 0x10;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the address in SPU RAM where reverb effects are performed.
// Any bytes past this address are used for reverb.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBSPU_SpuGetReverbOffsetAddr() noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;
    return (uint32_t) spu.reverbBase._reg * 8;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Zero out the area used for reverb effects.
// Will return 'SPU_ERROR' if that area is currently in use, otherwise 'SPU_SUCCESS'.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBSPU_SpuClearReverbWorkArea() noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;

    // Can't clear the reverb area if reverb is active!
    // Also can't clear if no reverb address is set:
    const uint32_t reverbBaseAddr = (uint32_t) spu.reverbBase._reg * 8;

    if (spu.control.masterReverb || (reverbBaseAddr == 0)) {
        return SPU_ERROR;
    }

    // Zero the reverb area
    if (reverbBaseAddr < SPU_RAM_SIZE) {
        const uint32_t reverbAreaSize = SPU_RAM_SIZE - reverbBaseAddr;
        std::memset(spu.ram.data() + reverbBaseAddr, 0, reverbAreaSize);
    }

    return SPU_SUCCESS;
}

void LIBSPU__SpuInit() noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;

    LIBETC_ResetCallback();
    
    a0 = 0;
    LIBSPU__spu_init();
    
    LIBSPU_SpuStart();

    // Ensure the reverb work area address is correct
    if (gReverbMode < SPU_REV_MODE_MAX) {
        spu.reverbBase._reg = gReverbWorkAreaBaseAddrs[gReverbMode];
    } else {
        spu.reverbBase._reg = gReverbWorkAreaBaseAddrs[0];
    }
    
    sw(0, 0x80076A58);      // Store to: 80076A58
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin SPU processing. Note: this is called automatically by 'LIBSPU_SpuInit'.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuStart() noexcept {
    // Doesn't need to do anything in this LIBSPU reimplementation.
    // This previously setup DMA related events, but now since we have no more DMA there is nothing do be done...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the depth of reverb using the settings in the given structure.
// By default both left and right channels are set, but you can set independently using 'SPU_REV_DEPTHL' and 'SPU_REV_DEPTHR' mask flags.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuSetReverbDepth(const SpuReverbAttr& reverb) noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;

    if ((reverb.mask == 0) || (reverb.mask & SPU_REV_DEPTHL)) {
        spu.reverbVolume.left = reverb.depth.left;
    }

    if ((reverb.mask == 0) || (reverb.mask & SPU_REV_DEPTHR)) {
        spu.reverbVolume.right = reverb.depth.right;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable or disable reverb for specific voices, or set the reverb enabled/disabled status for ALL voices.
// Returns a bit mask indicating which voices have reverb enabled, upon return.
//
// The meaning of 'onOff' is as follows:
//  SPU_OFF : disable reverb for voices with set bits in 'voiceBits'
//  SPU_ON  : enable reverb for voices with set bits in 'voiceBits'
//  SPU_BIT : enable or disable reverb for ALL voices based on whether the corresponding voice bit is set in 'voiceBits'.
//            If the bit is set then reverb is enabled.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBSPU_SpuSetReverbVoice(const int32_t onOff, const int32_t voiceBits) noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;
    
    // Enabling/disabling reverb for every single voice with the bit mask?
    if (onOff == SPU_BIT) {
        for (int32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
            spu::Voice& voice = spu.voices[voiceIdx];
            voice.reverb = ((voiceBits & (1 << voiceIdx)) != 0);
        }

        return voiceBits;
    }
    
    // Enable or disable reverb for specific voices and return the reverb status of all voices after
    const bool bEnableReverb = (onOff != SPU_OFF) || true;
    int32_t enabledVoiceBits = 0;

    for (int32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
        spu::Voice& voice = spu.voices[voiceIdx];

        if (voiceBits & (1 << voiceIdx)) {
            voice.reverb = bEnableReverb;
        }

        enabledVoiceBits |= (voice.reverb) ? (1 << voiceIdx) : 0;
    }

    return enabledVoiceBits;
}

void LIBSPU_SpuInit() noexcept {
loc_80054580:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0;                                             // Result = 00000000
    LIBSPU__SpuInit();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Turn reverb on or off and return the new reverb on/off setting.
// In the real LIBSPU, this could fail due to 'SpuMalloc' occupying the work area required for reverb.
// Since DOOM does not use SpuMalloc, this can never fail.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBSPU_SpuSetReverb(const int32_t onOff) noexcept {
    const bool bEnable = (onOff != SPU_OFF);
    spu::SPU& spu = *PsxVm::gpSpu;
    spu.control.masterReverb = bEnable;
    return (bEnable) ? SPU_ON : SPU_OFF;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Terminate SPU processing and release any resources used
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuQuit() noexcept {
    // Doesn't need to do anything in this LIBSPU reimplementation.
    // This previously cleaned up DMA related events, but now since we have no more DMA there is nothing do be done...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Query if there is any ongoing transfers to SPU RAM
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBSPU_SpuIsTransferCompleted([[maybe_unused]] const SpuTransferQuery mode) noexcept {
    // The answer to this question is always 'yes' for this LIBSPU reimplementation.
    // All transfer operations are now completely synchronous.
    return true;
}

void _thunk_LIBSPU_SpuIsTransferCompleted() noexcept {
    v0 = LIBSPU_SpuIsTransferCompleted((SpuTransferQuery) a0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize SPU memory management.
// This call is now a NO-OP because DOOM does not use 'SpuMalloc' - hence no need to implement this.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuInitMalloc([[maybe_unused]] const int32_t maxAllocs, [[maybe_unused]] uint8_t* const pMemMangementTbl) noexcept {
    // Ignore because DOOM doesn't use 'SpuMalloc'...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the mode of transfer from main RAM to SPU RAM
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuSetTransferMode([[maybe_unused]] const SpuTransferMode mode) noexcept {
    // Don't need to do anything in the emulated environment - we always do a straight 'memcpy' to SPU RAM...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set where in SPU RAM the next transfer of data will be to.
// The given address must be in range and is rounded up to the next 8 byte boundary.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBSPU_SpuSetTransferStartAddr(const uint32_t addr) noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;

    // Per PsyQ docs the address given is rounded up to the next 8-byte boundary.
    // It also must be in range or the instruction is ignored and '0' returned.
    const uint32_t alignedAddr = (addr + 7) & (~7u);

    if (alignedAddr < SPU_RAM_SIZE) {
        spu.dataAddress._reg = (uint16_t)(alignedAddr / 8);
        spu.currentDataAddress = alignedAddr;
        return alignedAddr;
    } else {
        return 0;
    }
}

void _thunk_LIBSPU_SpuSetTransferStartAddr() noexcept {
    v0 = LIBSPU_SpuSetTransferStartAddr(a0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the specified number of bytes to SPU RAM at the previously set transfer address.
// Returns the number of bytes written, which may be less than the request if it is out of bounds.
//
// Note: unlike the original PsyQ SDK, the write here happens SYNCHRONOUSLY and immediately.
// Originally this operation would be done via DMA and would have have taken some time...
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBSPU_SpuWrite(const void* const pData, const uint32_t size) noexcept {
    // Figure out how much data we can copy to SPU RAM, do the write and then return what we did
    spu::SPU& spu = *PsxVm::gpSpu;
    const uint32_t maxWriteSize = (spu.currentDataAddress < SPU_RAM_SIZE) ? SPU_RAM_SIZE - spu.currentDataAddress : 0;
    const uint32_t thisWriteSize = (size <= maxWriteSize) ? size : maxWriteSize;

    std::memcpy(spu.ram.data() + spu.currentDataAddress, pData, thisWriteSize);
    return thisWriteSize;
}

void _thunk_LIBSPU_SpuWrite() noexcept {
    v0 = LIBSPU_SpuWrite(vmAddrToPtr<void>(a0), a1);
}

void LIBSPU_SpuSetKeyOnWithAttr() noexcept {
loc_800548F4:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    LIBSPU_SpuSetVoiceAttr(*vmAddrToPtr<SpuVoiceAttr>(a0));
    a1 = lw(s0);
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuSetKey(a0, a1);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin voices ramp up (attack phase or 'key on') or begin voice ramp down (release phase or 'key off').
// The voices affected are specified by the given voice bit mask.
// The on/off action to perform must be either 'SPU_OFF' or 'SPU_ON'
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuSetKey(const int32_t onOff, const uint32_t voiceBits) noexcept {
    static_assert(SPU_NUM_VOICES == spu::SPU::VOICE_COUNT);
    spu::SPU& spu = *PsxVm::gpSpu;
    
    if (onOff == SPU_OFF) {
        for (uint32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
            if (voiceBits & (1 << voiceIdx)) {
                spu::Voice& voice = spu.voices[voiceIdx];
                voice.keyOff();
            }
        }
    }
    else if (onOff == SPU_ON) {
        const uint64_t sysCycles = PsxVm::gpSystem->cycles;

        for (uint32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
            if (voiceBits & (1 << voiceIdx)) {
                spu::Voice& voice = spu.voices[voiceIdx];
                voice.keyOn(0);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the state of all SPU voices. The following return values mean the following:
//
//  SPU_OFF         : Key off status,   Envelope is '0'         (silent)
//  SPU_ON          : Key on status,    Envelope is != '0'      (attack/decay)
//  SPU_OFF_ENV_ON  : Key off status,   Envelope is != '0'      (release)
//  SPU_ON_ENV_OFF  : Key on status,    Envelope is '0'         (sustain)
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuGetAllKeysStatus(uint8_t statuses[SPU_NUM_VOICES]) noexcept {
    spu::SPU& spu = *PsxVm::gpSpu;

    for (uint32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
        const spu::Voice::State voiceState = spu.voices[voiceIdx].state;

        switch (voiceState) {
            case spu::Voice::State::Attack:
            case spu::Voice::State::Decay:
                statuses[voiceIdx] = SPU_ON;
                break;

            case spu::Voice::State::Sustain:
                statuses[voiceIdx] = SPU_ON_ENV_OFF;
                break;

            case spu::Voice::State::Release:
                statuses[voiceIdx] = SPU_OFF_ENV_ON;
                break;

            case spu::Voice::State::Off:
            default:
                statuses[voiceIdx] = SPU_OFF;
                break;
        }
    }
}
