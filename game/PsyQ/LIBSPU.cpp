//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBSPU' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LIBSPU.h"

#include "PcPsx/PsxVm.h"
#include "Spu.h"

#include <cmath>

// The current reverb mode in use
static SpuReverbMode gReverbMode = SPU_REV_MODE_OFF;

// Where to write to next in SPU ram
static uint32_t gTransferStartAddr;

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
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    const uint32_t voiceBits = attribs.voice_bits;

    for (uint32_t voiceIdx = 0; voiceIdx < spu.numVoices; ++voiceIdx) {
        // Skip this voice if we're not setting its attributes
        if ((voiceBits & (1 << voiceIdx)) == 0)
            continue;

        // Set: voice 'pitch' or sample rate. Note that '4,096' = '44,100 Hz'.
        Spu::Voice& voice = spu.pVoices[voiceIdx];

        if (bSetPitch) {
            voice.sampleRate = attribs.pitch;
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
            voice.sampleRate = sampleRate;
        }

        // Set: the start address (64-bit word index) for the voice wave data.
        // If the given address is not 64-bit aligned then it is aligned up to the next 64-bit boundary.
        if (bSetWaveAddr) {
            const uint32_t addr = (attribs.addr + 7) / 8;
            voice.adpcmStartAddr8 = addr;
        }

        // Set: attack rate
        if (bSetAttackRate) {
            const uint32_t attackRate = (attribs.ar < 0x7F) ? attribs.ar : 0x7F;
            voice.env.attackStep = (attackRate & 0b0000'0011);
            voice.env.releaseShift = (attackRate & 0b0111'1100) >> 2;

            // Set: attack rate mode (exponential or not).
            // If not specified then default to 'linear' increase mode.
            if (bSetAttackMode) {
                voice.env.bAttackExp = (attribs.a_mode == SPU_VOICE_EXPIncN) ? 1 : 0;
            } else {
                voice.env.bAttackExp = 0;
            }
        }

        // Set: decay rate
        if (bSetDecayRate) {
            voice.env.decayShift = (attribs.dr < 0xF) ? attribs.dr : 0xF;
        }

        // Set: sustain level
        if (bSetSustainLevel) {
            voice.env.sustainLevel = (attribs.sl < 0xF) ? attribs.sl : 0xF;
        }

        // Set: sustain rate
        if (bSetSustainRate) {
            const uint32_t sustainRate = (attribs.sr < 0x7F) ? attribs.sr : 0x7F;
            voice.env.sustainStep = (sustainRate & 0b0000'0011);
            voice.env.sustainShift = (sustainRate & 0b0111'1100) >> 2;

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

                voice.env.bSustainDec = dir;
                voice.env.bSustainExp = mode;
            } else {
                voice.env.bSustainDec = 0;
                voice.env.bSustainExp = 0;
            }
        }

        // Set: release rate
        if (bSetReleaseRate) {
            const uint32_t releaseRate = (attribs.rr < 0x1F) ? attribs.rr : 0x1F;
            voice.env.releaseShift = releaseRate;

            // Set: release rate mode (exponential or not).
            // If not specified then default to 'linear' mode.
            voice.env.bReleaseExp = 0;

            if (bSetReleaseMode) {
                if (attribs.r_mode == SPU_VOICE_EXPDec) {
                    voice.env.bReleaseExp = 1;
                }
            }
        }

        // Set: envelope ADSR directly (1st and 2nd 16-bits)
        if (bSetAdsrPart1) {
            voice.envBits &= 0xFFFF0000;
            voice.envBits |= (uint32_t) attribs.adsr1;
        }

        if (bSetAdsrPart2) {
            voice.envBits &= 0x0000FFFF;
            voice.envBits |= ((uint32_t) attribs.adsr2) << 16;
        }

        // Set: wave loop address (64-bit word index).
        // If the given address is not 64-bit aligned then it is aligned up to the next 64-bit boundary.
        if (bSetWaveLoopAddr) {
            const uint32_t addr = (attribs.loop_addr + 7) / 8;
            voice.adpcmRepeatAddr8 = addr;
        }

        // Set: left volume and mode
        if (bSetVolL) {
            const uint16_t mode = (bSetVolModeL) ? attribs.volmode.left : 0;

            if (mode == 0) {
                voice.volume.left = attribs.volume.left & 0x7FFF;
            } else {
                const uint16_t volBits = (attribs.volume.left < 0x7F) ? attribs.volume.left : 0x7F;
                const uint16_t modeBits = 0x8000 | ((mode - 1) << 12);
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
                const uint16_t modeBits = 0x8000 | ((mode - 1) << 12);
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
//                  For example '60' would be C5 (12 semitones per octave, 1st note of 5th octave).
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
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    if (bSetReverbMode) {
        const SpuReverbMode reverbMode = (SpuReverbMode)(reverbAttr.mode & (~SPU_REV_MODE_CLEAR_WA));   // Must remove the 'CLEAR_WA' (clear working area flag)

        if (reverbMode < SPU_REV_MODE_MAX) {
            gReverbMode = reverbMode;
            spu.reverbBaseAddr8 = gReverbWorkAreaBaseAddrs[gReverbMode];    // Update the reverb working area base address when changing mode
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
                    SPU_REVF_SAME_SIDE_REFRACT_ADDR_1_LEFT |
                    SPU_REVF_SAME_SIDE_REFRACT_ADDR_1_RIGHT |
                    SPU_REVF_COMB_ADDR_1_LEFT |
                    SPU_REVF_SAME_SIDE_REFRACT_ADDR_2_LEFT |
                    SPU_REVF_APF_ADDR_1_LEFT |
                    SPU_REVF_APF_ADDR_1_RIGHT
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
    const bool bPrevReverbEnabled = spu.bReverbWriteEnable;
    spu.bReverbWriteEnable = false;

    // Update the reverb depth.
    // Note that if the reverb mode is being set then LIBSPU must also set reverb depth temporarily to '0', as per the docs.
    // It's up to callers in that situation to call 'LIBSPU_SpuSetReverbDepth' to set the depth again after that...
    if (bSetReverbMode) {
        spu.reverbVol.left = 0;
        spu.reverbVol.right = 0;
    } else {
        if (bSetReverbLeftDepth) {
            spu.reverbVol.left = reverbAttr.depth.left;
        }

        if (bSetReverbRightDepth) {
            spu.reverbVol.right = reverbAttr.depth.right;
        }
    }
    
    // Update the SPU reverb registers if setting reverb mode, delay time or feedback.
    // If we are just updating the left/right reverb depth however, then we can skip this.
    if (bSetReverbMode || bSetReverbDelay || bSetReverbFeedback) {
        const uint32_t regBits = reverbDef.fieldBits;
        const bool bSetAllRegs = (regBits == 0);
        
        const auto updateReg = [=](const uint32_t idx, auto& reg, const uint16_t value) noexcept {
            if (bSetAllRegs || (regBits & (1 << idx))) {
                reg = value;
            }
        };

        updateReg(0, spu.reverbRegs.dispAPF1, reverbDef.apfOffset1);
        updateReg(1, spu.reverbRegs.dispAPF2, reverbDef.apfOffset2);
        updateReg(2, spu.reverbRegs.volIIR, reverbDef.reflectionVolume1);
        updateReg(3, spu.reverbRegs.volComb1, reverbDef.combVolume1);
        updateReg(4, spu.reverbRegs.volComb2, reverbDef.combVolume2);
        updateReg(5, spu.reverbRegs.volComb3, reverbDef.combVolume3);
        updateReg(6, spu.reverbRegs.volComb4, reverbDef.combVolume4);
        updateReg(7, spu.reverbRegs.volWall, reverbDef.reflectionVolume2);
        updateReg(8, spu.reverbRegs.volAPF1, reverbDef.apfVolume1);
        updateReg(9, spu.reverbRegs.volAPF2, reverbDef.apfVolume2);
        updateReg(10, spu.reverbRegs.addrLSame1, reverbDef.sameSideRefractAddr1Left);
        updateReg(11, spu.reverbRegs.addrRSame1, reverbDef.sameSideRefractAddr1Right);
        updateReg(12, spu.reverbRegs.addrLComb1, reverbDef.combAddr1Left);
        updateReg(13, spu.reverbRegs.addrRComb1, reverbDef.combAddr1Right);
        updateReg(14, spu.reverbRegs.addrLComb2, reverbDef.combAddr2Left);
        updateReg(15, spu.reverbRegs.addrRComb2, reverbDef.combAddr2Right);
        updateReg(16, spu.reverbRegs.addrLSame2, reverbDef.sameSideRefractAddr2Left);
        updateReg(17, spu.reverbRegs.addrRSame2, reverbDef.sameSideRefractAddr2Right);
        updateReg(18, spu.reverbRegs.addrLDiff1, reverbDef.diffSideReflectAddr1Left);
        updateReg(19, spu.reverbRegs.addrRDiff1, reverbDef.diffSideReflectAddr1Right);
        updateReg(20, spu.reverbRegs.addrLComb3, reverbDef.combAddr3Left);
        updateReg(21, spu.reverbRegs.addrRComb3, reverbDef.combAddr3Right);
        updateReg(22, spu.reverbRegs.addrLComb4, reverbDef.combAddr4Left);
        updateReg(23, spu.reverbRegs.addrRComb4, reverbDef.combAddr4Right);
        updateReg(24, spu.reverbRegs.addrLDiff2, reverbDef.diffSideReflectAddr2Left);
        updateReg(25, spu.reverbRegs.addrRDiff2, reverbDef.diffSideReflectAddr2Right);
        updateReg(26, spu.reverbRegs.addrLAPF1, reverbDef.apfAddr1Left);
        updateReg(27, spu.reverbRegs.addrRAPF1, reverbDef.apfAddr1Right);
        updateReg(28, spu.reverbRegs.addrLAPF2, reverbDef.apfAddr2Left);
        updateReg(29, spu.reverbRegs.addrRAPF2, reverbDef.apfAddr2Right);
        updateReg(30, spu.reverbRegs.volLIn, reverbDef.inputVolLeft);
        updateReg(31, spu.reverbRegs.volRIn, reverbDef.inputVolRight);
    }
    
    // Clear the reverb working area if that was specified
    if (bClearReverbWorkingArea) {
        LIBSPU_SpuClearReverbWorkArea();
    }

    // Restore master reverb if we disabled it and return success
    spu.bReverbWriteEnable = bPrevReverbEnabled;
    return SPU_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the specified common/master sound settings using the given stuct
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuSetCommonAttr(const SpuCommonAttr& attribs) noexcept {
    // Figure out what attributes we are setting
    const uint32_t attribMask = attribs.mask;
    
    const bool bSetAllAttribs   = (attribMask == 0);
    const bool bSetMVolL        = (bSetAllAttribs || (attribMask & SPU_COMMON_MVOLL));
    const bool bSetMVolModeL    = (bSetAllAttribs || (attribMask & SPU_COMMON_MVOLMODEL));
    const bool bSetMVolR        = (bSetAllAttribs || (attribMask & SPU_COMMON_MVOLR));
    const bool bSetMVolModeR    = (bSetAllAttribs || (attribMask & SPU_COMMON_MVOLMODER));
    const bool bSetCdVolL       = (bSetAllAttribs || (attribMask & SPU_COMMON_CDVOLL));
    const bool bSetCdVolR       = (bSetAllAttribs || (attribMask & SPU_COMMON_CDVOLR));
    const bool bSetCdReverb     = (bSetAllAttribs || (attribMask & SPU_COMMON_CDREV));
    const bool bSetCdMix        = (bSetAllAttribs || (attribMask & SPU_COMMON_CDMIX));

    // Attributes relating to PlayStation external inputs are ignored for PsyDoom (see comments below)
    #if false
        const bool bSetExtVolL      = (bSetAllAttribs || (attribMask & SPU_COMMON_EXTVOLL));
        const bool bSetExtVolR      = (bSetAllAttribs || (attribMask & SPU_COMMON_EXTVOLR));
        const bool bSetExtReverb    = (bSetAllAttribs || (attribMask & SPU_COMMON_EXTREV));
        const bool bSetExtMix       = (bSetAllAttribs || (attribMask & SPU_COMMON_EXTMIX));
    #endif

    // Set: master volume and mode (left)
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    if (bSetMVolL) {
        const uint16_t mode = (bSetMVolModeL) ? attribs.mvolmode.left : 0;

        if (mode == 0) {
            spu.masterVol.left = attribs.mvol.left & 0x7FFF;
        } else {
            const uint16_t volBits = std::max(std::min(attribs.mvol.left, (int16_t) 0x7F), (int16_t) 0);
            const uint16_t modeBits = 0x8000 | ((mode - 1) << 12);
            spu.masterVol.left = modeBits | volBits;
        }
    }

    // Set: master volume and mode (right)
    if (bSetMVolR) {
        const uint16_t mode = (bSetMVolModeR) ? attribs.mvolmode.right : 0;

        if (mode == 0) {
            spu.masterVol.right = attribs.mvol.right & 0x7FFF;
        } else {
            const uint16_t volBits = std::max(std::min(attribs.mvol.right, (int16_t) 0x7F), (int16_t) 0);
            const uint16_t modeBits = 0x8000 | ((mode - 1) << 12);
            spu.masterVol.right = modeBits | volBits;
        }
    }

    // Note: PsyDoom's new SPU implemention only supports a single external input, which is used to supply CD audio.
    // Because of this, parameters relating to CD volume and reverb are set for the 'external input' on the SPU and not on dedicated
    // fields relating to CD audio specifically. Because there is only one external input also, I'm ignoring anything requested here
    // related to the PlayStation's original external input. Doom didn't use this input so that's okay to do...
    
    // Set: cd volume left and right
    if (bSetCdVolL) {
        spu.extInputVol.left = attribs.cd.volume.left;
    }

    if (bSetCdVolR) {
        spu.extInputVol.right = attribs.cd.volume.right;
    }

    // Set: cd reverb and mix enabled
    if (bSetCdReverb) {
        spu.bExtReverbEnable = (attribs.cd.reverb != 0);
    }

    if (bSetCdMix) {
        spu.bExtEnabled = (attribs.cd.mix != 0);
    }

    // Attributes relating to PlayStation external inputs are ignored for PsyDoom (see comments above)
    #if false
        if (bSetExtVolL) {
            spu.extInputVol.left = attribs.ext.volume.left;
        }

        if (bSetExtVolR) {
            spu.extInputVol.right = attribs.ext.volume.right;
        }
    
        if (bSetExtReverb) {
            spu.bExtReverbEnable = (attribs.ext.reverb != 0);
        }

        if (bSetExtMix) {
            spu.bExtEnabled = (attribs.ext.mix != 0);
        }
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the address in SPU RAM where reverb effects are performed.
// Any bytes past this address are used for reverb.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBSPU_SpuGetReverbOffsetAddr() noexcept {
    PsxVm::LockSpu spuLock;
    return PsxVm::gSpu.reverbBaseAddr8 * 8;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Zero out the area used for reverb effects.
// Will return 'SPU_ERROR' if that area is currently in use, otherwise 'SPU_SUCCESS'.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBSPU_SpuClearReverbWorkArea() noexcept {
    // Can't clear the reverb area if reverb is active!
    // Also can't clear if no reverb address is set:
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    const uint32_t reverbBaseAddr = (uint32_t) spu.reverbBaseAddr8 * 8;

    if (spu.bReverbWriteEnable || (reverbBaseAddr == 0)) {
        return SPU_ERROR;
    }

    // Zero the reverb area
    if (reverbBaseAddr < spu.ramSize) {
        const uint32_t reverbAreaSize = spu.ramSize - reverbBaseAddr;
        std::memset(spu.pRam + reverbBaseAddr, 0, reverbAreaSize);
    }

    return SPU_SUCCESS;
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
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    if ((reverb.mask == 0) || (reverb.mask & SPU_REV_DEPTHL)) {
        spu.reverbVol.left = reverb.depth.left;
    }

    if ((reverb.mask == 0) || (reverb.mask & SPU_REV_DEPTHR)) {
        spu.reverbVol.right = reverb.depth.right;
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
    // Enabling/disabling reverb for every single voice with the bit mask?
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    if (onOff == SPU_BIT) {
        for (int32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
            Spu::Voice& voice = spu.pVoices[voiceIdx];
            voice.bDoReverb = ((voiceBits & (1 << voiceIdx)) != 0);
        }

        return voiceBits;
    }
    
    // Enable or disable reverb for specific voices and return the reverb status of all voices after
    const bool bEnableReverb = (onOff != SPU_OFF);
    int32_t enabledVoiceBits = 0;

    for (int32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
        Spu::Voice& voice = spu.pVoices[voiceIdx];

        if (voiceBits & (1 << voiceIdx)) {
            voice.bDoReverb = bEnableReverb;
        }

        enabledVoiceBits |= (voice.bDoReverb) ? (1 << voiceIdx) : 0;
    }

    return enabledVoiceBits;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the SPU to a default state
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuInit() noexcept {
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    spu.bExtEnabled = false;
    spu.bExtReverbEnable = false;
    spu.bUnmute = false;
    spu.bReverbWriteEnable = false;

    spu.masterVol = {};
    spu.reverbVol = {};
    spu.extInputVol = {};

    for (uint32_t voiceIdx = 0; voiceIdx < spu.numVoices; ++voiceIdx) {
        Spu::Voice& voice = spu.pVoices[voiceIdx];

        Spu::keyOff(voice);
        voice.volume = {};
        voice.sampleRate = 0x00FF;
        voice.adpcmStartAddr8 = 0;
        voice.env = {};
    }

    spu.bUnmute = true;
    LIBSPU_SpuStart();

    // Ensure the reverb work area address is correct
    if (gReverbMode < SPU_REV_MODE_MAX) {
        spu.reverbBaseAddr8 = gReverbWorkAreaBaseAddrs[gReverbMode];
    } else {
        spu.reverbBaseAddr8 = gReverbWorkAreaBaseAddrs[0];
    }

    gTransferStartAddr = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Turn reverb on or off and return the new reverb on/off setting.
// In the real LIBSPU, this could fail due to 'SpuMalloc' occupying the work area required for reverb.
// Since DOOM does not use SpuMalloc, this can never fail.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBSPU_SpuSetReverb(const int32_t onOff) noexcept {
    const bool bEnable = (onOff != SPU_OFF);

    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;
    spu.bReverbWriteEnable = bEnable;
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
    // Per PsyQ docs the address given is rounded up to the next 8-byte boundary.
    // It also must be in range or the instruction is ignored and '0' returned.
    const uint32_t alignedAddr = (addr + 7) & (~7u);
    
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    if (alignedAddr < spu.ramSize) {
        gTransferStartAddr = alignedAddr;
        return alignedAddr;
    } else {
        return 0;
    }
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
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    const uint32_t maxWriteSize = (gTransferStartAddr < spu.ramSize) ? spu.ramSize - gTransferStartAddr : 0;
    const uint32_t thisWriteSize = (size <= maxWriteSize) ? size : maxWriteSize;

    std::memcpy(spu.pRam + gTransferStartAddr, pData, thisWriteSize);
    return thisWriteSize;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the specified attributes for the specified voices in the data structure.
// Also 'key on' (begin playing) the specified voices.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuSetKeyOnWithAttr(const SpuVoiceAttr& attribs) noexcept {
    LIBSPU_SpuSetVoiceAttr(attribs);
    LIBSPU_SpuSetKey(1, attribs.voice_bits);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin voices ramp up (attack phase or 'key on') or begin voice ramp down (release phase or 'key off').
// The voices affected are specified by the given voice bit mask.
// The on/off action to perform must be either 'SPU_OFF' or 'SPU_ON'
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBSPU_SpuSetKey(const int32_t onOff, const uint32_t voiceBits) noexcept {
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    const uint32_t numVoicesToSet = std::min(SPU_NUM_VOICES, spu.numVoices);
    
    if (onOff == SPU_OFF) {
        for (uint32_t voiceIdx = 0; voiceIdx < numVoicesToSet; ++voiceIdx) {
            if (voiceBits & (1 << voiceIdx)) {
                Spu::Voice& voice = spu.pVoices[voiceIdx];
                Spu::keyOff(voice);
            }
        }
    }
    else if (onOff == SPU_ON) {
        for (uint32_t voiceIdx = 0; voiceIdx < numVoicesToSet; ++voiceIdx) {
            if (voiceBits & (1 << voiceIdx)) {
                Spu::Voice& voice = spu.pVoices[voiceIdx];
                Spu::keyOn(voice);
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
    // Get the statuses
    Spu::Core& spu = PsxVm::gSpu;
    const uint32_t numVoicesToGet = std::min(SPU_NUM_VOICES, spu.numVoices);

    for (uint32_t voiceIdx = 0; voiceIdx < numVoicesToGet; ++voiceIdx) {
        const Spu::EnvPhase envPhase = spu.pVoices[voiceIdx].envPhase;

        switch (envPhase) {
            case Spu::EnvPhase::Attack:
            case Spu::EnvPhase::Decay:
                statuses[voiceIdx] = SPU_ON;
                break;

            case Spu::EnvPhase::Sustain:
                statuses[voiceIdx] = SPU_ON_ENV_OFF;
                break;

            case Spu::EnvPhase::Release:
                statuses[voiceIdx] = SPU_OFF_ENV_ON;
                break;

            case Spu::EnvPhase::Off:
            default:
                statuses[voiceIdx] = SPU_OFF;
                break;
        }
    }

    // Just in case the SPU has less voices than we expect
    for (uint32_t voiceIdx = numVoicesToGet; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
        statuses[voiceIdx] = SPU_OFF;
    }
}
