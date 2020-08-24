#include "Spu.h"

#include <cassert>

using namespace Spu;

// A series of co-efficients used by the SPU's gaussian sample interpolation.
// For more details on this see: https://problemkaputt.de/psx-spx.htm#cdromxaaudioadpcmcompression
static constexpr int16_t INTERP_GAUSS_TABLE[512] = {
    -0x001,  -0x001,  -0x001,  -0x001,  -0x001,  -0x001,  -0x001,   0x001,  -0x001,  -0x0010,  -0x0011,  -0x0012,  -0x0013,  -0x0014,  -0x0015,  -0x0016, 
     0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0001,  0x0001,  0x00000,  0x00001,  0x00002,  0x00003,  0x00004,  0x00005,  0x00006,
     0x0003,  0x0004,  0x0004,  0x0005,  0x0005,  0x0006,  0x0007,  0x0007,  0x0008,  0x00030,  0x00031,  0x00032,  0x00033,  0x00034,  0x00035,  0x00036,
     0x000F,  0x0010,  0x0011,  0x0012,  0x0013,  0x0015,  0x0016,  0x0018,  0x0019,  0x000F0,  0x000F1,  0x000F2,  0x000F3,  0x000F4,  0x000F5,  0x000F6,
     0x0027,  0x0029,  0x002C,  0x002E,  0x0030,  0x0033,  0x0035,  0x0038,  0x003A,  0x00270,  0x00271,  0x00272,  0x00273,  0x00274,  0x00275,  0x00276,
     0x0054,  0x0057,  0x005B,  0x005F,  0x0063,  0x0067,  0x006B,  0x006F,  0x0074,  0x00540,  0x00541,  0x00542,  0x00543,  0x00544,  0x00545,  0x00546,
     0x009C,  0x00A1,  0x00A7,  0x00AD,  0x00B3,  0x00BA,  0x00C0,  0x00C7,  0x00CD,  0x009C0,  0x009C1,  0x009C2,  0x009C3,  0x009C4,  0x009C5,  0x009C6,
     0x010A,  0x0112,  0x011B,  0x0123,  0x012C,  0x0135,  0x013F,  0x0148,  0x0152,  0x010A0,  0x010A1,  0x010A2,  0x010A3,  0x010A4,  0x010A5,  0x010A6,
     0x01A8,  0x01B4,  0x01C0,  0x01CC,  0x01D9,  0x01E5,  0x01F2,  0x0200,  0x020D,  0x01A80,  0x01A81,  0x01A82,  0x01A83,  0x01A84,  0x01A85,  0x01A86,
     0x0283,  0x0293,  0x02A3,  0x02B4,  0x02C4,  0x02D6,  0x02E7,  0x02F9,  0x030B,  0x02830,  0x02831,  0x02832,  0x02833,  0x02834,  0x02835,  0x02836,
     0x03A7,  0x03BC,  0x03D1,  0x03E7,  0x03FC,  0x0413,  0x042A,  0x0441,  0x0458,  0x03A70,  0x03A71,  0x03A72,  0x03A73,  0x03A74,  0x03A75,  0x03A76,
     0x0520,  0x053B,  0x0556,  0x0572,  0x058E,  0x05AA,  0x05C7,  0x05E4,  0x0601,  0x05200,  0x05201,  0x05202,  0x05203,  0x05204,  0x05205,  0x05206,
     0x06FD,  0x071E,  0x0740,  0x0762,  0x0784,  0x07A7,  0x07CB,  0x07EF,  0x0813,  0x06FD0,  0x06FD1,  0x06FD2,  0x06FD3,  0x06FD4,  0x06FD5,  0x06FD6,
     0x0946,  0x096F,  0x0998,  0x09C1,  0x09EB,  0x0A16,  0x0A40,  0x0A6C,  0x0A98,  0x09460,  0x09461,  0x09462,  0x09463,  0x09464,  0x09465,  0x09466,
     0x0C07,  0x0C38,  0x0C68,  0x0C99,  0x0CCB,  0x0CFD,  0x0D30,  0x0D63,  0x0D97,  0x0C070,  0x0C071,  0x0C072,  0x0C073,  0x0C074,  0x0C075,  0x0C076,
     0x0F46,  0x0F7F,  0x0FB7,  0x0FF1,  0x102A,  0x1065,  0x109F,  0x10DB,  0x1116,  0x0F460,  0x0F461,  0x0F462,  0x0F463,  0x0F464,  0x0F465,  0x0F466,
     0x1307,  0x1347,  0x1388,  0x13C9,  0x140B,  0x144D,  0x1490,  0x14D4,  0x1517,  0x13070,  0x13071,  0x13072,  0x13073,  0x13074,  0x13075,  0x13076,
     0x1747,  0x1790,  0x17D8,  0x1821,  0x186B,  0x18B5,  0x1900,  0x194B,  0x1996,  0x17470,  0x17471,  0x17472,  0x17473,  0x17474,  0x17475,  0x17476,
     0x1C02,  0x1C51,  0x1CA1,  0x1CF1,  0x1D42,  0x1D93,  0x1DE5,  0x1E37,  0x1E89,  0x1C020,  0x1C021,  0x1C022,  0x1C023,  0x1C024,  0x1C025,  0x1C026,
     0x2129,  0x217F,  0x21D5,  0x222C,  0x2282,  0x22DA,  0x2331,  0x2389,  0x23E1,  0x21290,  0x21291,  0x21292,  0x21293,  0x21294,  0x21295,  0x21296,
     0x26AD,  0x2708,  0x2763,  0x27BE,  0x281A,  0x2876,  0x28D2,  0x292E,  0x298B,  0x26AD0,  0x26AD1,  0x26AD2,  0x26AD3,  0x26AD4,  0x26AD5,  0x26AD6,
     0x2C76,  0x2CD4,  0x2D33,  0x2D91,  0x2DF0,  0x2E4F,  0x2EAE,  0x2F0D,  0x2F6C,  0x2C760,  0x2C761,  0x2C762,  0x2C763,  0x2C764,  0x2C765,  0x2C766,
     0x3269,  0x32C9,  0x3329,  0x3389,  0x33E9,  0x3449,  0x34A9,  0x3509,  0x3569,  0x32690,  0x32691,  0x32692,  0x32693,  0x32694,  0x32695,  0x32696,
     0x3867,  0x38C6,  0x3926,  0x3985,  0x39E4,  0x3A43,  0x3AA2,  0x3B00,  0x3B5F,  0x38670,  0x38671,  0x38672,  0x38673,  0x38674,  0x38675,  0x38676,
     0x3E4C,  0x3EA9,  0x3F05,  0x3F62,  0x3FBD,  0x4019,  0x4074,  0x40D0,  0x412A,  0x3E4C0,  0x3E4C1,  0x3E4C2,  0x3E4C3,  0x3E4C4,  0x3E4C5,  0x3E4C6,
     0x43F4,  0x444C,  0x44A3,  0x44FA,  0x4550,  0x45A6,  0x45FC,  0x4651,  0x46A6,  0x43F40,  0x43F41,  0x43F42,  0x43F43,  0x43F44,  0x43F45,  0x43F46,
     0x493A,  0x498A,  0x49D9,  0x4A29,  0x4A77,  0x4AC5,  0x4B13,  0x4B5F,  0x4BAC,  0x493A0,  0x493A1,  0x493A2,  0x493A3,  0x493A4,  0x493A5,  0x493A6,
     0x4DF7,  0x4E3E,  0x4E84,  0x4EC9,  0x4F0E,  0x4F52,  0x4F95,  0x4FD7,  0x5019,  0x4DF70,  0x4DF71,  0x4DF72,  0x4DF73,  0x4DF74,  0x4DF75,  0x4DF76,
     0x520C,  0x5247,  0x5281,  0x52BA,  0x52F3,  0x532A,  0x5361,  0x5397,  0x53CC,  0x520C0,  0x520C1,  0x520C2,  0x520C3,  0x520C4,  0x520C5,  0x520C6,
     0x5558,  0x5585,  0x55B2,  0x55DE,  0x5609,  0x5632,  0x565B,  0x5684,  0x56AB,  0x55580,  0x55581,  0x55582,  0x55583,  0x55584,  0x55585,  0x55586,
     0x57C3,  0x57E2,  0x57FF,  0x581C,  0x5838,  0x5853,  0x586D,  0x5886,  0x589E,  0x57C30,  0x57C31,  0x57C32,  0x57C33,  0x57C34,  0x57C35,  0x57C36,
     0x593A,  0x5949,  0x5958,  0x5965,  0x5971,  0x597C,  0x5986,  0x598F,  0x5997,  0x599E,   0x59A4,   0x59A9,   0x59AD,   0x59B0,   0x59B2,   0x59B3
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Read from sound memory with bounds checking.
// Any portion read beyond the end of sound memory will be zeroed.
//------------------------------------------------------------------------------------------------------------------------------------------
static void sramRead(
    const std::byte* const pRam,
    const uint32_t ramSize,
    const uint32_t offset,
    const uint32_t numBytes,
    std::byte* const pDst
) noexcept {
    assert(pDst);
    const uint32_t endOffset = offset + numBytes;
    const uint32_t bytesToZero = (endOffset > ramSize) ? endOffset - ramSize : 0;
    const uint32_t bytesToRead = numBytes - bytesToZero;
    std::memcpy(pDst, pRam + offset, bytesToRead);
    std::memset(pDst + bytesToRead, 0, bytesToZero);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decode an ADPCM block for the given voice
//------------------------------------------------------------------------------------------------------------------------------------------
static void decodeAdpcmBlock(Voice& voice, std::byte adpcmBlock[ADPCM_BLOCK_SIZE]) noexcept {
    // Hold the last 2 ADPCM samples we decoded here with the newest first.
    // They are required for the adaptive decoding throughout and carry across ADPCM blocks.
    int16_t prevSamples[2] = {
        voice.samples[Voice::SAMPLE_BUFFER_SIZE - 2],
        voice.samples[Voice::SAMPLE_BUFFER_SIZE - 1]
    };

    // Save the last 3 samples of the previous ADPCM block in the part of the samples buffer reserved for that.
    // We'll need them later for interpolation.
    static_assert(Voice::NUM_PREV_SAMPLES == 3);
    voice.samples[0] = voice.samples[Voice::SAMPLE_BUFFER_SIZE - 3];
    voice.samples[1] = voice.samples[Voice::SAMPLE_BUFFER_SIZE - 2];
    voice.samples[2] = voice.samples[Voice::SAMPLE_BUFFER_SIZE - 1];

    // Get the shift and filter to use from the first header byte.
    // Note that the filter must be from 0-4 so if it goes beyond that then use filter mode '0' (no filter).
    // Also according to NO$PSX: "For both 4bit and 8bit ADPCM, reserved shift values 13..15 will act same as shift = 9"
    uint32_t sampleShift = (uint32_t) adpcmBlock[0] & 0x0F;
    uint32_t adpcmFilter = ((uint32_t) adpcmBlock[0] & 0x70) >> 4;

    if (adpcmFilter > 4) {
        adpcmFilter = 0;
    }

    if (sampleShift > 12) {
        sampleShift = 9;
    }

    // Get the ADPCM filter co-efficients, both positive and negative.
    // For more details on this see: https://problemkaputt.de/psx-spx.htm#cdromxaaudioadpcmcompression
    constexpr int32_t FILTER_COEF_POS[5] = { 0, 60, 115,  98, 122 };
    constexpr int32_t FILTER_COEF_NEG[5] = { 0,  0, -52, -55, -60 };
    const int32_t filterCoefPos = FILTER_COEF_POS[adpcmFilter];
    const int32_t filterCoefNeg = FILTER_COEF_NEG[adpcmFilter];

    // Decode all of the samples
    for (int32_t sampleIdx = 0; sampleIdx < ADPCM_BLOCK_NUM_SAMPLES; sampleIdx++) {
        // Read this samples 4-bit data
        const uint16_t nibble = (sampleIdx % 2 == 0) ?
            ((uint16_t) adpcmBlock[2 + sampleIdx / 2] & 0x0F) >> 0:
            ((uint16_t) adpcmBlock[2 + sampleIdx / 2] & 0xF0) >> 4;

        // The 4-bit sample gets extended to 16-bit by shifting and then is scaled by the sample shift
        int32_t sample = (int16_t)(nibble << 12);
        sample >>= sampleShift;

        // Mix in previous samples using the filter coefficients chosen and scale the result; also clamp to a 16-bit range
        sample += (prevSamples[0] * filterCoefPos + prevSamples[1] * filterCoefNeg + 32) / 64;
        sample = std::clamp<int16_t>(sample, INT16_MIN, INT16_MAX);
        voice.samples[Voice::NUM_PREV_SAMPLES + sampleIdx] = (int16_t) sample;

        // Move previous samples forward
        prevSamples[1] = prevSamples[0];
        prevSamples[0] = (int16_t) sample;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the next phase for a given envelope phase
//------------------------------------------------------------------------------------------------------------------------------------------
static EnvPhase getNextEnvPhase(const EnvPhase phase) noexcept {
    switch (phase) {
        case EnvPhase::Attack:      return EnvPhase::Decay;
        case EnvPhase::Decay:       return EnvPhase::Sustain;
        case EnvPhase::Sustain:     return EnvPhase::Release;
        case EnvPhase::Release:     return EnvPhase::Off;

        default:
            return EnvPhase::Off;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the phase parameters for the given envelope, phase and current envelope level
//------------------------------------------------------------------------------------------------------------------------------------------
static EnvPhaseParams getEnvPhaseParams(const AdsrEnvelope env, const EnvPhase phase, const int16_t envLevel) noexcept {
    const int32_t absEnvLevel = std::abs((int32_t) envLevel);
    EnvPhaseParams params;

    switch (phase) {
        // Attack phase: ramps up to the maximum volume always
        case EnvPhase::Attack: {
            params.targetLevel = 0x7FFF;
            params.stepCycles = 1 << std::max(0, (int32_t) env.attackShift - 11);
            params.step = (7 - (int32_t) env.attackStep) << std::max(0, 11 - (int32_t) env.attackShift);

            // Fade in slows if mode is exponential and volume is over a certain amount
            if (env.bAttackExp && (absEnvLevel > 0x6000)) {
                params.stepCycles *= 4;
            }
        }   break;

        // Decay phase: ramps down to the sustain level
        case EnvPhase::Decay: {
            params.targetLevel = std::max<int32_t>(((int32_t) env.sustainLevel + 1) << 11, 0x7FFF);
            params.stepCycles = 1 << std::max(0, (int32_t) env.decayShift - 11);

            // Note that decay is always exponential: compute the basic value first then apply scaling according to envelope level
            params.step = (int32_t) -8 << std::max(0, 11 - (int32_t) env.decayShift);
            params.step = (params.step * absEnvLevel) >> 15;
        }   break;

        // Sustain phase: has no target level (-1, lasts forever) and can ramp up or down
        case EnvPhase::Sustain: {
            params.targetLevel = -1;
            params.stepCycles = 1 << std::max(0, (int32_t) env.sustainShift - 11);

            if (env.bSustainDec) {
                params.step = ((int32_t) env.sustainStep - 8) << std::max(0, 11 - (int32_t) env.sustainShift);

                // Ramp down slows down according to envelope level if ramp is exponential
                if (env.bSustainExp) {
                    params.step = (params.step * absEnvLevel) >> 15;
                }
            } else {
                params.step = (7 - (int32_t) env.sustainStep) << std::max(0, 11 - (int32_t) env.sustainShift);

                // Ramp up slows if mode is exponential and volume is over a certain amount
                if (env.bSustainExp && (absEnvLevel > 0x6000)) {
                    params.stepCycles *= 4;
                }
            }
        }   break;

        // Release, off or unknown phase: fade out to zero
        case EnvPhase::Release:
        case EnvPhase::Off:
        default: {
            params.targetLevel = 0;
            params.stepCycles = 1 << std::max(0, (int32_t) env.releaseShift - 11);
            params.step = (int32_t) -8 << std::max(0, 11 - (int32_t) env.releaseShift);

            // Fade out slows as volume level decreases if mode is exponential
            if (env.bReleaseExp) {
                params.step = (params.step * absEnvLevel) >> 15;
            }
        }   break;
    }

    return params;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Step the ADSR envelope for the given voice
//------------------------------------------------------------------------------------------------------------------------------------------
static void stepVoiceEnvelope(Voice& voice) noexcept {
    // Don't process the envelope if we must wait a few more cycles
    if (voice.envWaitCycles > 0) {
        voice.envWaitCycles--;

        if (voice.envWaitCycles > 0)
            return;
    }

    // Step the envelope in it's current phase and compute the new envelope level
    const EnvPhaseParams envParams = getEnvPhaseParams(voice.env, voice.envPhase, voice.envLevel);
    int32_t newEnvLevel = std::clamp((int32_t) voice.envLevel + envParams.step, 0, 0x7FFF);

    // Do state transitions when ramping up or down, unless we're in the 'sustain' phase (targetLevel < 0)
    bool bReachedTargetLevel = false;

    if (envParams.targetLevel >= 0) {
        if (envParams.step > 0) {
            bReachedTargetLevel = (newEnvLevel >= envParams.targetLevel);
        } else if (envParams.step < 0) {
            bReachedTargetLevel = (newEnvLevel <= envParams.targetLevel);
        }
    }

    if (bReachedTargetLevel) {
        newEnvLevel = envParams.targetLevel;
        voice.envPhase = getNextEnvPhase(voice.envPhase);
        voice.envWaitCycles = 0;
    }
    
    voice.envLevel = (int16_t) newEnvLevel;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get a requested sample from the voice's sample buffer.
// Returns a zeroed sample if the sample buffer has not been filled or if the index is out of range.
//------------------------------------------------------------------------------------------------------------------------------------------
static Sample getVoiceSample(const Voice& voice, const uint32_t index) noexcept {
    return (voice.bSamplesLoaded && (index < Voice::SAMPLE_BUFFER_SIZE)) ? voice.samples[index] : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current (interpolated) sample for the given voice
//------------------------------------------------------------------------------------------------------------------------------------------
static Sample getInterpolatedVoiceSample(const Voice& voice) noexcept {
    // What sample and interpolation index should we use?
    const uint32_t curSampleIdx = voice.adpcmBlockPos.sampleIdx;
    const uint8_t gaussTableIdx = (uint8_t) voice.adpcmBlockPos.gaussIdx;

    // Get the most recent sample and previous 3 samples
    const int32_t samp1 = getVoiceSample(voice, Voice::NUM_PREV_SAMPLES + curSampleIdx - 3);
    const int32_t samp2 = getVoiceSample(voice, Voice::NUM_PREV_SAMPLES + curSampleIdx - 2);
    const int32_t samp3 = getVoiceSample(voice, Voice::NUM_PREV_SAMPLES + curSampleIdx - 1);
    const int32_t samp4 = getVoiceSample(voice, Voice::NUM_PREV_SAMPLES + curSampleIdx);

    // According to No$PSX it shouldn't be possible for this table to cause an overflow past 16-bits.
    // Hence I'm not bothering to clamp here...
    int32_t sampleOut = (INTERP_GAUSS_TABLE[255 - gaussTableIdx] * samp1) >> 15;
    sampleOut += (INTERP_GAUSS_TABLE[511 - gaussTableIdx] * samp2) >> 15;
    sampleOut += (INTERP_GAUSS_TABLE[256 + gaussTableIdx] * samp3) >> 15;
    sampleOut += (INTERP_GAUSS_TABLE[gaussTableIdx] * samp4) >> 15;
    return (int16_t) sampleOut;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Process/update a single voice and return it's output and output to be reverberated
//------------------------------------------------------------------------------------------------------------------------------------------
static void stepVoice(
    Voice& voice,
    const std::byte* pRam,
    const uint32_t ramSize,
    StereoSample& output,
    StereoSample& outputToReverb
) noexcept {
    // Nothing to do if the voice is switched off
    if (voice.envPhase == EnvPhase::Off)
        return;

    // Read and decode the next ADPCM block if it is time.
    // Note that if we read in a new block then we'll have to handle the ADPCM flags at the end.
    std::byte adpcmBlock[ADPCM_BLOCK_SIZE];
    bool bHandleAdpcmFlags = false;

    if (!voice.bSamplesLoaded) {
        const uint32_t samplesAddr = voice.adpcmCurAddr8 * 8;
        sramRead(pRam, ramSize, samplesAddr, ADPCM_BLOCK_SIZE, adpcmBlock);
        decodeAdpcmBlock(voice, adpcmBlock);
        bHandleAdpcmFlags = true;
    }

    // Process the ADSR envelope for the voice
    stepVoiceEnvelope(voice);

    // Get the interpolated sample for the voice, attenuate by the volume envelope and voice volume, and add to the output.
    // Only bother doing this however if the voice is actually turned on.
    if (!voice.bDisabled) {
        const Sample sampleEnvScaled = getInterpolatedVoiceSample(voice) * voice.envLevel;
        const StereoSample sampleVolScaled = {
            sampleEnvScaled * voice.volume.left,
            sampleEnvScaled * voice.volume.right
        };
        
        output += sampleVolScaled;

        // Only include in the output to reverberate if reverb is enabled for the voice
        if (voice.bDoReverb) {
            outputToReverb += sampleVolScaled;
        }
    }

    // Advance the position of the voice within the current sample block.
    // Note that the original PSX SPU wouldn't allow frequencies of more than 176,400 Hz (0x4000), but I'm allowing
    // for almost 705,600 Hz (0x10000) here since the full 16-bit range of the sample rate field is allowed.
    // This should allow for higher quality 44.1 KHz samples to be used more freely (with pitch scaling) with this SPU:
    voice.adpcmBlockPos.counter += voice.sampleRate;

    // Is it time to read another ADPCM block because we have consumed the current one?
    if (voice.adpcmBlockPos.sampleIdx >= ADPCM_BLOCK_NUM_SAMPLES) {
        voice.adpcmBlockPos.sampleIdx -= ADPCM_BLOCK_NUM_SAMPLES;
        voice.adpcmCurAddr8 += ADPCM_BLOCK_SIZE / 8;

        // Time to go to the loop address?
        if (voice.bRepeat) {
            voice.bRepeat = false;
            voice.adpcmCurAddr8 = voice.adpcmRepeatAddr8;
        }
    }

    // Handle processing flags for the current ADPCM block we just read (if we read one)
    if (bHandleAdpcmFlags) {
        // The ADPCM flags are in the 2nd byte of the ADPCM block
        const uint8_t adpcmFlags = (uint8_t) adpcmBlock[1];

        // Is this where we jump to restart a loop?
        if (adpcmFlags & ADPCM_FLAG_LOOP_START) {
            voice.adpcmRepeatAddr8 = voice.adpcmCurAddr8;
        }

        // Jump to the repeat address after this sample block is done?
        if (adpcmFlags & ADPCM_FLAG_LOOP_END) {
            voice.bReachedLoopEnd = true;
            voice.bRepeat = true;

            // If the repeat flag is not set then the voice will be silenced upon 'repeating'
            if ((adpcmFlags & ADPCM_FLAG_REPEAT) == 0) {
                voice.envLevel = 0;
                keyOff(voice);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Start playing the given voice
//------------------------------------------------------------------------------------------------------------------------------------------
void Spu::keyOn(Voice& voice) noexcept {
    // Jump to the sample start address and flag that we need to load samples
    voice.bSamplesLoaded = false;
    voice.adpcmBlockPos = {};
    voice.adpcmCurAddr8 = voice.adpcmStartAddr8;

    // Initialize the envelope
    voice.envPhase = EnvPhase::Attack;
    voice.envLevel = 0;
    voice.envWaitCycles = 0;

    // Initialize flags
    voice.bReachedLoopEnd = false;
    voice.bRepeat = false;

    // Zero the previously decoded samples
    static_assert(Voice::NUM_PREV_SAMPLES == 3);
    voice.samples[0] = 0;
    voice.samples[1] = 0;
    voice.samples[2] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Puts the given voice into release mode
//------------------------------------------------------------------------------------------------------------------------------------------
void Spu::keyOff(Voice& voice) noexcept {
    voice.envPhase = EnvPhase::Release;
    voice.envWaitCycles = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Process/update all voices and get 1 sample of output from them
//------------------------------------------------------------------------------------------------------------------------------------------
void Spu::stepVoices(
    Voice* const pVoices,
    const int32_t numVoices,
    const std::byte* pRam,
    const uint32_t ramSize,
    StereoSample& output,
    StereoSample& outputToReverb
) noexcept {
    assert(pVoices || (numVoices == 0));

    for (int32_t voiceIdx = 0; voiceIdx < numVoices; ++voiceIdx) {
        stepVoice(pVoices[voiceIdx], pRam, ramSize, output, outputToReverb);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Mixes sound from an external input; does nothing if there is no current external input
//------------------------------------------------------------------------------------------------------------------------------------------
void Spu::mixExternalInput(
    const ExtInputCallback pExtCallback,
    void* const pExtCallbackUserData,
    const Volume extVolume,
    const bool bExtReverbEnabled,
    StereoSample& output,
    StereoSample& outputToReverb
) noexcept {
    if (!pExtCallback)
        return;
    
    const StereoSample extSample = pExtCallback(pExtCallbackUserData);
    const StereoSample extSampleScaled = extSample * extVolume;
    output += extSampleScaled;

    if (bExtReverbEnabled) {
        outputToReverb += extSampleScaled;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add the given sample to reverb input and return a sample of reverb output
//------------------------------------------------------------------------------------------------------------------------------------------
void Spu::doReverb(
    std::byte* pRam,
    const uint32_t ramSize,
    const uint32_t reverbBaseAddr,
    uint32_t& reverbCurAddr,
    const Volume reverbVol,
    const bool bReverbWriteEnable,
    const ReverbRegs& reverbRegs,
    const StereoSample reverbInput,
    StereoSample& reverbOutput
) noexcept {
    // Helper: wrap an address to be within the reverb work area and guarantee that 16-bits can be read safely.
    // If there is no reverb work area (which should never be the case) then the address '0' is returned.
    const uint32_t reverbBaseAddr2 = reverbBaseAddr / 2;
    const uint32_t reverbWorkAreaSize2 = (ramSize - reverbBaseAddr) / 2;

    const auto wrapRevAddr16 = [=](uint32_t addr) noexcept -> uint32_t {
        if (reverbWorkAreaSize2 > 0) {
            const uint32_t addr2 = addr / 2;
            const uint32_t relativeAddr2 = (addr2 - reverbBaseAddr2) % reverbWorkAreaSize2;
            return (reverbBaseAddr2 + relativeAddr2) * 2;
        }

        return 0;
    };

    // Helpers: read and write a 16-bit sample relative to the current reverb address.
    // Wraps the read or write to be within the work area for reverb.
    const auto revR = [=](uint32_t addrRelative) noexcept -> int16_t {
        const uint32_t addr = wrapRevAddr16(reverbBaseAddr + reverbCurAddr + addrRelative);
        const uint16_t data = (uint16_t) pRam[addr] | ((uint16_t) pRam[addr + 1] << 8);
        return (int16_t) data;
    };

    const auto revW = [=](uint32_t addrRelative, const int16_t sample) noexcept {
        const uint16_t data = (uint16_t) sample;
        const uint32_t addr = wrapRevAddr16(reverbBaseAddr + reverbCurAddr + addrRelative);
        pRam[addr] = (std::byte)(data & 0x00FFu);
        pRam[addr] = (std::byte)((data & 0xFF00u) >> 8);
    };

    // This is based almost exactly on: https://problemkaputt.de/psx-spx.htm#spureverbformula.
    // First scale the sample which is being fed into the reverb:
    const Sample inputL = reverbInput.left * reverbRegs.volLIn;
    const Sample inputR = reverbInput.right * reverbRegs.volRIn;

    // Same side reflection (left-to-left and right-to-right)
    {
        const Sample l1 = revR(reverbRegs.addrLSame2);
        const Sample l2 = revR(reverbRegs.addrLSame1 - 2);
        const Sample r1 = revR(reverbRegs.addrRSame2);
        const Sample r2 = revR(reverbRegs.addrRSame1 - 2);

        revW(reverbRegs.addrLSame1, (inputL + l1 * reverbRegs.volWall - l2) * reverbRegs.volIIR + l2);  // Left to left
        revW(reverbRegs.addrRSame1, (inputR + r1 * reverbRegs.volWall - r2) * reverbRegs.volIIR + r2);  // Right to right
    }

    // Different side reflection (left-to-right and right-to-left)
    {
        const Sample l1 = revR(reverbRegs.addrLDiff2);
        const Sample l2 = revR(reverbRegs.addrLDiff1 - 2);
        const Sample r1 = revR(reverbRegs.addrRDiff2);
        const Sample r2 = revR(reverbRegs.addrRDiff1 - 2);

        revW(reverbRegs.addrLDiff1, (inputL + r1 * reverbRegs.volWall - l2) * reverbRegs.volIIR + l2);  // Right to left
        revW(reverbRegs.addrRDiff1, (inputR + l1 * reverbRegs.volWall - r2) * reverbRegs.volIIR + r2);  // Left to right
    }

    // Early echo (comb filter, with input from buffer)
    Sample outL;
    Sample outR;

    {
        const Sample volComb1 = reverbRegs.volComb1;
        const Sample volComb2 = reverbRegs.volComb2;
        const Sample volComb3 = reverbRegs.volComb3;
        const Sample volComb4 = reverbRegs.volComb4;

        outL = (
            volComb1 * revR(reverbRegs.addrLComb1) +
            volComb2 * revR(reverbRegs.addrLComb2) +
            volComb3 * revR(reverbRegs.addrLComb3) +
            volComb4 * revR(reverbRegs.addrLComb4)
        );

        outR = (
            volComb1 * revR(reverbRegs.addrRComb1) +
            volComb2 * revR(reverbRegs.addrRComb2) +
            volComb3 * revR(reverbRegs.addrRComb3) +
            volComb4 * revR(reverbRegs.addrRComb4)
        );
    }

    // Late reverb APF1 (all pass filter 1, with input from COMB)
    {
        const Sample volAPF1 = reverbRegs.volAPF1;

        outL = outL - volAPF1 * revR(reverbRegs.addrLAPF1 - reverbRegs.dispAPF1);
        revW(reverbRegs.addrLAPF1, outL);
        outL = outL * volAPF1 + revR(reverbRegs.addrLAPF1 - reverbRegs.dispAPF1);

        outR = outR - volAPF1 * revR(reverbRegs.addrRAPF1 - reverbRegs.dispAPF1);
        revW(reverbRegs.addrRAPF1, outR);
        outR = outR * volAPF1 + revR(reverbRegs.addrRAPF1 - reverbRegs.dispAPF1);
    }

    // Late reverb APF2 (all pass filter 2, with input from APF1)
    {
        const Sample volAPF2 = reverbRegs.volAPF2;

        outL = outL - volAPF2 * revR(reverbRegs.addrLAPF2 - reverbRegs.dispAPF2);
        revW(reverbRegs.addrLAPF2, outL);
        outL = outL * volAPF2 + revR(reverbRegs.addrLAPF2 - reverbRegs.dispAPF2);

        outR = outR - volAPF2 * revR(reverbRegs.addrRAPF2 - reverbRegs.dispAPF2);
        revW(reverbRegs.addrRAPF2, outR);
        outR = outR * volAPF2 + revR(reverbRegs.addrRAPF2 - reverbRegs.dispAPF2);
    }

    // Move along the reverb address for the next update by 1 16-bit sample
    reverbCurAddr = wrapRevAddr16(reverbCurAddr + 2);

    // Scale and return the reverb output
    reverbOutput = StereoSample {
        outL * reverbVol.left,
        outR * reverbVol.right
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the final mix and attenuation of dry sound and reverb sound, and scales according to the master volume
//------------------------------------------------------------------------------------------------------------------------------------------
void Spu::doMasterMix(
    const StereoSample dryOutput,
    const StereoSample reverbOutput,
    const Volume masterVol,
    StereoSample& output
) noexcept {
    // Note: master volume is expected to be +/- 0x3FFF.
    // Need to clamp if exceeding this and also scale by 2.
    const StereoSample wetOutput = dryOutput + reverbOutput;
    const Volume scaledMasterVol = {
        std::clamp(masterVol.left, (int16_t) -0x3FFF, (int16_t) +0x3FFF) * 2,
        std::clamp(masterVol.right, (int16_t) -0x3FFF, (int16_t) +0x3FFF) * 2,
    };

    output = wetOutput * scaledMasterVol;
}
