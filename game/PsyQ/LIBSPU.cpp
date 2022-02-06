//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBSPU' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LIBSPU.h"

#include "Asserts.h"
#include "PsyDoom/PsxVm.h"
#include "Spu.h"

#include <cmath>
#include <cstring>

// This table defines the sample rates for an entire octave of notes (12 semitones) in 1/16 semitone steps.
// The first note in the octave plays at 44,100 Hz (0x1000) and the last note (the start of the next octave) at 88,200 Hz (0x2000).
// The sample rates are in the scale/format used by the PlayStation SPU.
// This table is used by 'LIBSPU__spu_note2pitch' to figure out the sample rate for a note to be played.
// The sample rates are scaled appropriately depending on the octave of the note to play.
static const uint16_t OCTAVE_SAMPLE_RATES[] = {
    0x1000, 0x100E, 0x101D, 0x102C, 0x103B, 0x104A, 0x1059, 0x1068,     // C
    0x1078, 0x1087, 0x1096, 0x10A5, 0x10B5, 0x10C4, 0x10D4, 0x10E3,
    0x10F3, 0x1103, 0x1113, 0x1122, 0x1132, 0x1142, 0x1152, 0x1162,     // C#
    0x1172, 0x1182, 0x1193, 0x11A3, 0x11B3, 0x11C4, 0x11D4, 0x11E5,
    0x11F5, 0x1206, 0x1216, 0x1227, 0x1238, 0x1249, 0x125A, 0x126B,     // D
    0x127C, 0x128D, 0x129E, 0x12AF, 0x12C1, 0x12D2, 0x12E3, 0x12F5,
    0x1306, 0x1318, 0x132A, 0x133C, 0x134D, 0x135F, 0x1371, 0x1383,     // D#
    0x1395, 0x13A7, 0x13BA, 0x13CC, 0x13DE, 0x13F1, 0x1403, 0x1416,
    0x1428, 0x143B, 0x144E, 0x1460, 0x1473, 0x1486, 0x1499, 0x14AC,     // E
    0x14BF, 0x14D3, 0x14E6, 0x14F9, 0x150D, 0x1520, 0x1534, 0x1547,
    0x155B, 0x156F, 0x1583, 0x1597, 0x15AB, 0x15BF, 0x15D3, 0x15E7,     // F
    0x15FB, 0x1610, 0x1624, 0x1638, 0x164D, 0x1662, 0x1676, 0x168B,
    0x16A0, 0x16B5, 0x16CA, 0x16DF, 0x16F4, 0x170A, 0x171F, 0x1734,     // F#
    0x174A, 0x175F, 0x1775, 0x178B, 0x17A1, 0x17B6, 0x17CC, 0x17E2,
    0x17F9, 0x180F, 0x1825, 0x183B, 0x1852, 0x1868, 0x187F, 0x1896,     // G
    0x18AC, 0x18C3, 0x18DA, 0x18F1, 0x1908, 0x191F, 0x1937, 0x194E,
    0x1965, 0x197D, 0x1995, 0x19AC, 0x19C4, 0x19DC, 0x19F4, 0x1A0C,     // G#
    0x1A24, 0x1A3C, 0x1A55, 0x1A6D, 0x1A85, 0x1A9E, 0x1AB7, 0x1ACF,
    0x1AE8, 0x1B01, 0x1B1A, 0x1B33, 0x1B4C, 0x1B66, 0x1B7F, 0x1B98,     // A
    0x1BB2, 0x1BCC, 0x1BE5, 0x1BFF, 0x1C19, 0x1C33, 0x1C4D, 0x1C67,
    0x1C82, 0x1C9C, 0x1CB7, 0x1CD1, 0x1CEC, 0x1D07, 0x1D22, 0x1D3D,     // A#
    0x1D58, 0x1D73, 0x1D8E, 0x1DA9, 0x1DC5, 0x1DE0, 0x1DFC, 0x1E18,
    0x1E34, 0x1E50, 0x1E6C, 0x1E88, 0x1EA4, 0x1EC1, 0x1EDD, 0x1EFA,     // B
    0x1F16, 0x1F33, 0x1F50, 0x1F6D, 0x1F8A, 0x1FA7, 0x1FC5, 0x1FE2,
    0x2000,                                                             // C, Next octave
};

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
    const uint16_t centerNoteFrac,
    const int32_t offsetNote,
    const uint16_t offsetNoteFrac
) noexcept;

#if PSYDOOM_LIMIT_REMOVING
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: computes the reverb base address (divided by 8) for potentially extended PSX sound ram given a reverb base address in
// terms of the original 512 KB available (and divided by 8). The address returned will be at the end of the extended RAM area, if extended.
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t LIBSPU_GetExtReverbBaseAddr(const uint16_t origPsxReverbBaseAddr8) noexcept {
    // Note: shouldn't need to lock the SPU for this query since ram size is set on init
    Spu::Core& spu = PsxVm::gSpu;
    uint32_t extBaseAddr8 = spu.ramSize / 8;

    if (extBaseAddr8 > UINT16_MAX) {
        extBaseAddr8 -= UINT16_MAX;     // Reverb area is 512 KiB maximum, at the end of SRAM
    } else {
        extBaseAddr8 = 0;               // SPU ram is not extended, reverb area starts at the start of SRAM
    }

    extBaseAddr8 += origPsxReverbBaseAddr8;
    return extBaseAddr8;
}
#endif  // #if PSYDOOM_LIMIT_REMOVING

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

    const SpuVoiceMask voiceBits = attribs.voice_bits;

    for (uint32_t voiceIdx = 0; voiceIdx < spu.numVoices; ++voiceIdx) {
        // Skip this voice if we're not setting its attributes
        if ((voiceBits & (SpuVoiceMask(1) << voiceIdx)) == 0)
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
            // Note: the original PSX code set the low 16-bits of the ADSR envelope directly using 'attribs.adsr1'.
            // We can't rely on that method however because a particular bitfield order is not guaranteed in C++.
            // Instead decode all fields individually in a portable manner:
            voice.env.sustainLevel = attribs.adsr1 & 0xF;
            voice.env.decayShift = (attribs.adsr1 >> 4) & 0xF;
            voice.env.attackStep = (attribs.adsr1 >> 8) & 0x3;
            voice.env.attackShift = (attribs.adsr1 >> 10) & 0x1F;
            voice.env.bAttackExp = (attribs.adsr1 >> 15);
        }

        if (bSetAdsrPart2) {
            // Note: the original PSX code set the high 16-bits of the ADSR envelope directly using 'attribs.adsr2'.
            // We can't rely on that method however because a particular bitfield order is not guaranteed in C++.
            // Instead decode all fields individually in a portable manner:
            voice.env.releaseShift = attribs.adsr2 & 0x1F;
            voice.env.bReleaseExp = (attribs.adsr2 >> 5) & 0x1;
            voice.env.sustainStep = (attribs.adsr2 >> 6) & 0x3;
            voice.env.sustainShift = (attribs.adsr2 >> 8) & 0x1F;
            voice.env._unused = (attribs.adsr2 >> 13) & 0x1;
            voice.env.bSustainDec = (attribs.adsr2 >> 14) & 0x1;
            voice.env.bSustainExp = attribs.adsr2 >> 15;
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
    const int32_t centerNote,
    const uint16_t centerNoteFrac,
    const int32_t offsetNote,
    const uint16_t offsetNoteFrac
) noexcept {
    // Get the fractional component of the note (which may be a semitone or more).
    // Once we have that drop 3 bits of precision to convert from 1/128 to 1/16 semitone steps, and wrap to a fraction.
    const int32_t noteFracUnwrapped = offsetNoteFrac + centerNoteFrac;
    const int32_t noteFrac = (noteFracUnwrapped >> 3) & 0xF;

    // Compute the note to sound relative to the center/root note which plays at 44,100 Hz.
    // Note: also need to account for fractional note parts that are >= 1 semitone.
    const int32_t note = offsetNote - centerNote + noteFracUnwrapped / 128;

    // Compute what octave is being sounded, relative to the center/root note and the index of the note in that octave.
    const int32_t octave = (note < 0) ? (note - 11) / 12 : note / 12;
    const int32_t octaveStartNote = octave * 12;
    const int32_t noteInOctave = note - octaveStartNote;
    ASSERT((noteInOctave >= 0) && (noteInOctave <= 11));

    // Using the octave relative note, and the fractional note component (1/16 semitone steps) compute the sample rate table lookup index
    const uint32_t lutIndex = (noteInOctave << 4) | noteFrac;
    ASSERT((lutIndex >= 0) && (lutIndex < 12 * 16));
    const uint16_t baseSampleRate = OCTAVE_SAMPLE_RATES[lutIndex];

    // Scale the sample rate depending on how many octaves up or down we are from the one that starts at 44,100 Hz
    if (octave > 0) {
        return baseSampleRate << +octave;
    } else if (octave < 0) {
        return baseSampleRate >> -octave;
    } else {
        return baseSampleRate;
    }
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

            // Update the reverb working area base address when changing mode
            #if PSYDOOM_LIMIT_REMOVING
                spu.reverbBaseAddr8 = LIBSPU_GetExtReverbBaseAddr(gReverbWorkAreaBaseAddrs[reverbMode]);
            #else
                spu.reverbBaseAddr8 = gReverbWorkAreaBaseAddrs[gReverbMode];
            #endif
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

    const uint32_t reverbBaseAddr = spu.reverbBaseAddr8 * 8;

    if (spu.bReverbWriteEnable || (reverbBaseAddr == 0))
        return SPU_ERROR;

    // Zero the reverb area
    #if SIMPLE_SPU_FLOAT_SPU
        std::memset(spu.pReverbRam, 0, sizeof(float) * spu.numReverbRamSamples);
    #else
        if (reverbBaseAddr < spu.ramSize) {
            const uint32_t reverbAreaSize = spu.ramSize - reverbBaseAddr;
            std::memset(spu.pRam + reverbBaseAddr, 0, reverbAreaSize);
        }
    #endif

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
SpuVoiceMask LIBSPU_SpuSetReverbVoice(const int32_t onOff, const SpuVoiceMask voiceBits) noexcept {
    // Enabling/disabling reverb for every single voice with the bit mask?
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    if (onOff == SPU_BIT) {
        for (uint32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
            Spu::Voice& voice = spu.pVoices[voiceIdx];
            voice.bDoReverb = (voiceBits & (SpuVoiceMask(1) << voiceIdx));
        }

        return voiceBits;
    }

    // Enable or disable reverb for specific voices and return the reverb status of all voices after
    const bool bEnableReverb = (onOff != SPU_OFF);
    SpuVoiceMask enabledVoiceBits = 0;

    for (uint32_t voiceIdx = 0; voiceIdx < SPU_NUM_VOICES; ++voiceIdx) {
        Spu::Voice& voice = spu.pVoices[voiceIdx];

        if (voiceBits & (SpuVoiceMask(1) << voiceIdx)) {
            voice.bDoReverb = bEnableReverb;
        }

        enabledVoiceBits |= (voice.bDoReverb) ? (SpuVoiceMask(1) << voiceIdx) : 0;
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
    const uint16_t reverbBaseAddr8 = gReverbWorkAreaBaseAddrs[(gReverbMode < SPU_REV_MODE_MAX) ? gReverbMode : 0];

    #if PSYDOOM_LIMIT_REMOVING
        spu.reverbBaseAddr8 = LIBSPU_GetExtReverbBaseAddr(reverbBaseAddr8);
    #else
        spu.reverbBaseAddr8 = reverbBaseAddr8;
    #endif

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
void LIBSPU_SpuSetKey(const int32_t onOff, const SpuVoiceMask voiceBits) noexcept {
    Spu::Core& spu = PsxVm::gSpu;
    PsxVm::LockSpu spuLock;

    const uint32_t numVoicesToSet = std::min(SPU_NUM_VOICES, spu.numVoices);

    if (onOff == SPU_OFF) {
        for (uint32_t voiceIdx = 0; voiceIdx < numVoicesToSet; ++voiceIdx) {
            if (voiceBits & (SpuVoiceMask(1) << voiceIdx)) {
                Spu::Voice& voice = spu.pVoices[voiceIdx];
                Spu::keyOff(voice);
            }
        }
    }
    else if (onOff == SPU_ON) {
        for (uint32_t voiceIdx = 0; voiceIdx < numVoicesToSet; ++voiceIdx) {
            if (voiceBits & (SpuVoiceMask(1) << voiceIdx)) {
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
