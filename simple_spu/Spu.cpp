#include "Spu.h"

#include "Asserts.h"

using namespace Spu;

// A series of co-efficients used by the SPU's gaussian sample interpolation.
// For more details on this see: https://problemkaputt.de/psx-spx.htm#cdromxaaudioadpcmcompression
static constexpr int32_t INTERP_GAUSS_TABLE[512] = {
    -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0003, 0x0003,
    0x0003, 0x0004, 0x0004, 0x0005, 0x0005, 0x0006, 0x0007, 0x0007, 0x0008, 0x0009, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
    0x000F, 0x0010, 0x0011, 0x0012, 0x0013, 0x0015, 0x0016, 0x0018, 0x0019, 0x001B, 0x001C, 0x001E, 0x0020, 0x0021, 0x0023, 0x0025,
    0x0027, 0x0029, 0x002C, 0x002E, 0x0030, 0x0033, 0x0035, 0x0038, 0x003A, 0x003D, 0x0040, 0x0043, 0x0046, 0x0049, 0x004D, 0x0050,
    0x0054, 0x0057, 0x005B, 0x005F, 0x0063, 0x0067, 0x006B, 0x006F, 0x0074, 0x0078, 0x007D, 0x0082, 0x0087, 0x008C, 0x0091, 0x0096,
    0x009C, 0x00A1, 0x00A7, 0x00AD, 0x00B3, 0x00BA, 0x00C0, 0x00C7, 0x00CD, 0x00D4, 0x00DB, 0x00E3, 0x00EA, 0x00F2, 0x00FA, 0x0101,
    0x010A, 0x0112, 0x011B, 0x0123, 0x012C, 0x0135, 0x013F, 0x0148, 0x0152, 0x015C, 0x0166, 0x0171, 0x017B, 0x0186, 0x0191, 0x019C,
    0x01A8, 0x01B4, 0x01C0, 0x01CC, 0x01D9, 0x01E5, 0x01F2, 0x0200, 0x020D, 0x021B, 0x0229, 0x0237, 0x0246, 0x0255, 0x0264, 0x0273,
    0x0283, 0x0293, 0x02A3, 0x02B4, 0x02C4, 0x02D6, 0x02E7, 0x02F9, 0x030B, 0x031D, 0x0330, 0x0343, 0x0356, 0x036A, 0x037E, 0x0392,
    0x03A7, 0x03BC, 0x03D1, 0x03E7, 0x03FC, 0x0413, 0x042A, 0x0441, 0x0458, 0x0470, 0x0488, 0x04A0, 0x04B9, 0x04D2, 0x04EC, 0x0506,
    0x0520, 0x053B, 0x0556, 0x0572, 0x058E, 0x05AA, 0x05C7, 0x05E4, 0x0601, 0x061F, 0x063E, 0x065C, 0x067C, 0x069B, 0x06BB, 0x06DC,
    0x06FD, 0x071E, 0x0740, 0x0762, 0x0784, 0x07A7, 0x07CB, 0x07EF, 0x0813, 0x0838, 0x085D, 0x0883, 0x08A9, 0x08D0, 0x08F7, 0x091E,
    0x0946, 0x096F, 0x0998, 0x09C1, 0x09EB, 0x0A16, 0x0A40, 0x0A6C, 0x0A98, 0x0AC4, 0x0AF1, 0x0B1E, 0x0B4C, 0x0B7A, 0x0BA9, 0x0BD8,
    0x0C07, 0x0C38, 0x0C68, 0x0C99, 0x0CCB, 0x0CFD, 0x0D30, 0x0D63, 0x0D97, 0x0DCB, 0x0E00, 0x0E35, 0x0E6B, 0x0EA1, 0x0ED7, 0x0F0F,
    0x0F46, 0x0F7F, 0x0FB7, 0x0FF1, 0x102A, 0x1065, 0x109F, 0x10DB, 0x1116, 0x1153, 0x118F, 0x11CD, 0x120B, 0x1249, 0x1288, 0x12C7,
    0x1307, 0x1347, 0x1388, 0x13C9, 0x140B, 0x144D, 0x1490, 0x14D4, 0x1517, 0x155C, 0x15A0, 0x15E6, 0x162C, 0x1672, 0x16B9, 0x1700,
    0x1747, 0x1790, 0x17D8, 0x1821, 0x186B, 0x18B5, 0x1900, 0x194B, 0x1996, 0x19E2, 0x1A2E, 0x1A7B, 0x1AC8, 0x1B16, 0x1B64, 0x1BB3,
    0x1C02, 0x1C51, 0x1CA1, 0x1CF1, 0x1D42, 0x1D93, 0x1DE5, 0x1E37, 0x1E89, 0x1EDC, 0x1F2F, 0x1F82, 0x1FD6, 0x202A, 0x207F, 0x20D4,
    0x2129, 0x217F, 0x21D5, 0x222C, 0x2282, 0x22DA, 0x2331, 0x2389, 0x23E1, 0x2439, 0x2492, 0x24EB, 0x2545, 0x259E, 0x25F8, 0x2653,
    0x26AD, 0x2708, 0x2763, 0x27BE, 0x281A, 0x2876, 0x28D2, 0x292E, 0x298B, 0x29E7, 0x2A44, 0x2AA1, 0x2AFF, 0x2B5C, 0x2BBA, 0x2C18,
    0x2C76, 0x2CD4, 0x2D33, 0x2D91, 0x2DF0, 0x2E4F, 0x2EAE, 0x2F0D, 0x2F6C, 0x2FCC, 0x302B, 0x308B, 0x30EA, 0x314A, 0x31AA, 0x3209,
    0x3269, 0x32C9, 0x3329, 0x3389, 0x33E9, 0x3449, 0x34A9, 0x3509, 0x3569, 0x35C9, 0x3629, 0x3689, 0x36E8, 0x3748, 0x37A8, 0x3807,
    0x3867, 0x38C6, 0x3926, 0x3985, 0x39E4, 0x3A43, 0x3AA2, 0x3B00, 0x3B5F, 0x3BBD, 0x3C1B, 0x3C79, 0x3CD7, 0x3D35, 0x3D92, 0x3DEF,
    0x3E4C, 0x3EA9, 0x3F05, 0x3F62, 0x3FBD, 0x4019, 0x4074, 0x40D0, 0x412A, 0x4185, 0x41DF, 0x4239, 0x4292, 0x42EB, 0x4344, 0x439C,
    0x43F4, 0x444C, 0x44A3, 0x44FA, 0x4550, 0x45A6, 0x45FC, 0x4651, 0x46A6, 0x46FA, 0x474E, 0x47A1, 0x47F4, 0x4846, 0x4898, 0x48E9,
    0x493A, 0x498A, 0x49D9, 0x4A29, 0x4A77, 0x4AC5, 0x4B13, 0x4B5F, 0x4BAC, 0x4BF7, 0x4C42, 0x4C8D, 0x4CD7, 0x4D20, 0x4D68, 0x4DB0,
    0x4DF7, 0x4E3E, 0x4E84, 0x4EC9, 0x4F0E, 0x4F52, 0x4F95, 0x4FD7, 0x5019, 0x505A, 0x509A, 0x50DA, 0x5118, 0x5156, 0x5194, 0x51D0,
    0x520C, 0x5247, 0x5281, 0x52BA, 0x52F3, 0x532A, 0x5361, 0x5397, 0x53CC, 0x5401, 0x5434, 0x5467, 0x5499, 0x54CA, 0x54FA, 0x5529,
    0x5558, 0x5585, 0x55B2, 0x55DE, 0x5609, 0x5632, 0x565B, 0x5684, 0x56AB, 0x56D1, 0x56F6, 0x571B, 0x573E, 0x5761, 0x5782, 0x57A3,
    0x57C3, 0x57E2, 0x57FF, 0x581C, 0x5838, 0x5853, 0x586D, 0x5886, 0x589E, 0x58B5, 0x58CB, 0x58E0, 0x58F4, 0x5907, 0x5919, 0x592A,
    0x593A, 0x5949, 0x5958, 0x5965, 0x5971, 0x597C, 0x5986, 0x598F, 0x5997, 0x599E, 0x59A4, 0x59A9, 0x59AD, 0x59B0, 0x59B2, 0x59B3,
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
    ASSERT(pDst);
    const uint32_t endOffset = offset + numBytes;
    const uint32_t bytesToZero = (endOffset > ramSize) ? endOffset - ramSize : 0;
    const uint32_t bytesToRead = numBytes - bytesToZero;
    std::memset(pDst + bytesToRead, 0, bytesToZero);
    std::memcpy(pDst, pRam + offset, bytesToRead);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decode an ADPCM block for the given voice
//------------------------------------------------------------------------------------------------------------------------------------------
static void decodeAdpcmBlock(Voice& voice, std::byte adpcmBlock[ADPCM_BLOCK_SIZE]) noexcept {
    // Hold the last 2 ADPCM samples we decoded here with the newest first.
    // They are required for the adaptive decoding throughout and carry across ADPCM blocks.
    int16_t prevSamples[2] = {
        voice.samples[Voice::SAMPLE_BUFFER_SIZE - 1],
        voice.samples[Voice::SAMPLE_BUFFER_SIZE - 2],
    };

    // Save the last 3 samples of the previous ADPCM block in the part of the samples buffer reserved for that.
    // We'll need them later for interpolation.
    static_assert(Voice::NUM_PREV_SAMPLES == 3);
    voice.samples[0] = voice.samples[Voice::SAMPLE_BUFFER_SIZE - 3];
    voice.samples[1] = voice.samples[Voice::SAMPLE_BUFFER_SIZE - 2];
    voice.samples[2] = voice.samples[Voice::SAMPLE_BUFFER_SIZE - 1];

    // Get the shift and filter to use from the first ADPCM header byte.
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

        // The 4-bit sample gets extended to 16-bit by shifting and is sign extended to 32-bit.
        // After that we scale by the sample shift.
        int32_t sample = (int32_t)(int16_t)(nibble << 12);
        sample >>= sampleShift;

        // Mix in previous samples using the filter coefficients chosen and scale the result; also clamp to a 16-bit range
        sample += (prevSamples[0] * filterCoefPos + prevSamples[1] * filterCoefNeg + 32) / 64;
        sample = std::clamp<int32_t>(sample, INT16_MIN, INT16_MAX);
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
    // Envelope level shouldn't be negative, but just in case...
    const int32_t absEnvLevel = std::abs((int32_t) envLevel);
    
    // Gather basic info for the envelope phase
    int32_t     targetLevel;
    int32_t     stepUnscaled;
    uint32_t    stepScale;
    bool        bExponential;

    switch (phase) {
        // Attack phase: ramps up to the maximum volume always
        case EnvPhase::Attack: {
            targetLevel = MAX_ENV_LEVEL;
            stepScale = env.attackShift;
            stepUnscaled = 7 - (int32_t) env.attackStep;
            bExponential = env.bAttackExp;
        }   break;

        // Decay phase: ramps down to the sustain level and always exponentially
        case EnvPhase::Decay: {
            targetLevel = std::min<int32_t>(((int32_t) env.sustainLevel + 1) * 2048, MAX_ENV_LEVEL);
            stepScale = env.decayShift;
            stepUnscaled = -8;
            bExponential = true;
        }   break;

        // Sustain phase: has no target level (-1, lasts forever) and can ramp up or down
        case EnvPhase::Sustain: {
            targetLevel = -1;
            stepScale = env.sustainShift;
            stepUnscaled = (env.bSustainDec) ? (int32_t) env.sustainStep - 8 : 7 - (int32_t) env.sustainStep;
            bExponential = env.bSustainExp;
        }   break;

        // Release, off or unknown phase: fade out to zero
        case EnvPhase::Release:
        case EnvPhase::Off:
        default: {
            targetLevel = MIN_ENV_LEVEL;
            stepScale = env.releaseShift;
            stepUnscaled = -8;
            bExponential = env.bReleaseExp;
        }   break;
    }

    // Scale the step accordingly and decide how many cycles an envelope step takes
    const int32_t stepScaleMultiplier = 1 << std::max<int32_t>(0, 11 - stepScale);

    EnvPhaseParams params;
    params.targetLevel = targetLevel;
    params.stepCycles = 1 << std::max<int32_t>(0, stepScale - 11);
    params.step = stepUnscaled * stepScaleMultiplier;
    
    if (bExponential) {
        // Adjustments based on the current envelope level when the envelope mode is 'exponential'.
        // Slower fade-outs as the envelope level decreases & 4x slower fade-ins when envelope level surpasses '0x6000'.
        if ((stepUnscaled > 0) && (absEnvLevel > 0x6000)) {
            params.stepCycles *= 4;
        }
        else if (stepUnscaled < 0) {
            params.step = (int32_t)(((int64_t) params.step * absEnvLevel) >> 15);
        }
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
    int32_t newEnvLevel = std::clamp<int32_t>(voice.envLevel + envParams.step, MIN_ENV_LEVEL, MAX_ENV_LEVEL);

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
    } else {
        voice.envWaitCycles = envParams.stepCycles;
    }
    
    voice.envLevel = (int16_t) newEnvLevel;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get a requested sample from the voice's sample buffer.
// Returns a zeroed sample if the sample buffer has not been filled or if the index is out of range.
//------------------------------------------------------------------------------------------------------------------------------------------
static Sample getVoiceSample(const Voice& voice, const uint32_t index) noexcept {
    if (voice.bSamplesLoaded) {
        if (index < Voice::SAMPLE_BUFFER_SIZE) {
            return voice.samples[index];
        }
    }

    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current (interpolated) sample for the given voice
//------------------------------------------------------------------------------------------------------------------------------------------
static Sample getInterpolatedVoiceSample(const Voice& voice) noexcept {
    // What sample and interpolation index should we use?
    const int32_t curSampleIdx  = (int32_t) voice.adpcmBlockPos.fields.sampleIdx;
    const int32_t gaussTableIdx = (int32_t)(uint8_t) voice.adpcmBlockPos.fields.gaussIdx;

    // Get the most recent sample and previous 3 samples
    const int32_t samp1 = getVoiceSample(voice, Voice::NUM_PREV_SAMPLES + curSampleIdx - 3);
    const int32_t samp2 = getVoiceSample(voice, Voice::NUM_PREV_SAMPLES + curSampleIdx - 2);
    const int32_t samp3 = getVoiceSample(voice, Voice::NUM_PREV_SAMPLES + curSampleIdx - 1);
    const int32_t samp4 = getVoiceSample(voice, Voice::NUM_PREV_SAMPLES + curSampleIdx    );

    // Sanity check...
    static_assert(-1 >> 1 == -1, "Right shift on signed types must be an arithmetic shift!");

    // Note: MSVC: for some reason bad code is sometimes generated in release builds for the gauss factors - prevent this by use of 'volatile'.
    // I'm not sure why this is occuring, it doesn't seem like there is any UB here...
    const int32_t gaussIdx1 = (255 - gaussTableIdx) & 0x1FF;
    const int32_t gaussIdx2 = (511 - gaussTableIdx) & 0x1FF;
    const int32_t gaussIdx3 = (256 + gaussTableIdx) & 0x1FF;
    const int32_t gaussIdx4 = (      gaussTableIdx) & 0x1FF;
    const volatile int32_t gaussFactor1 = INTERP_GAUSS_TABLE[gaussIdx1];
    const volatile int32_t gaussFactor2 = INTERP_GAUSS_TABLE[gaussIdx2];
    const volatile int32_t gaussFactor3 = INTERP_GAUSS_TABLE[gaussIdx3];
    const volatile int32_t gaussFactor4 = INTERP_GAUSS_TABLE[gaussIdx4];

    // According to No$PSX it shouldn't be possible for this table to cause an overflow past 16-bits.
    // Hence I'm not bothering to clamp here...
    const int32_t sampMix1 = (gaussFactor1 * samp1) >> 15;
    const int32_t sampMix2 = (gaussFactor2 * samp2) >> 15;
    const int32_t sampMix3 = (gaussFactor3 * samp3) >> 15;
    const int32_t sampMix4 = (gaussFactor4 * samp4) >> 15;

    return (int16_t)(sampMix1 + sampMix2 + sampMix3 + sampMix4);
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
        voice.bSamplesLoaded = true;
        bHandleAdpcmFlags = true;
    }

    // Process the ADSR envelope for the voice
    stepVoiceEnvelope(voice);

    // Get the interpolated sample for the voice, attenuate by the volume envelope and voice volume, and add to the output.
    // Only bother doing this however if the voice is actually turned on.
    if (!voice.bDisabled) {
        const Sample rawSample = getInterpolatedVoiceSample(voice);
        const Sample sampleEnvScaled = rawSample * voice.envLevel;
        const int16_t realVoiceVolL = (int16_t) std::clamp((int32_t) voice.volume.left * 2, INT16_MIN, +INT16_MAX);     // N.B: voice volume was divided by 2
        const int16_t realVoiceVolR = (int16_t) std::clamp((int32_t) voice.volume.right * 2, INT16_MIN, +INT16_MAX);

        const StereoSample sampleVolScaled = {
            sampleEnvScaled * realVoiceVolL,
            sampleEnvScaled * realVoiceVolR
        };
        
        output += sampleVolScaled;

        // Only include in the output to reverberate if reverb is enabled for the voice
        if (voice.bDoReverb) {
            outputToReverb += sampleVolScaled;
        }
    }

    // Advance the position of the voice within the current sample block.
    // Note that the original PSX SPU wouldn't allow frequencies of more than 176,400 Hz (0x4000), hence we clamp the frequency here.
    // Certain pieces of music in Doom need this clamping to be done in order to sound correct.
    voice.adpcmBlockPos.counter += std::min<uint16_t>(voice.sampleRate, MAX_SAMPLE_RATE);

    // Is it time to read another ADPCM block because we have consumed the current one?
    if (voice.adpcmBlockPos.fields.sampleIdx >= ADPCM_BLOCK_NUM_SAMPLES) {
        voice.adpcmBlockPos.fields.sampleIdx -= ADPCM_BLOCK_NUM_SAMPLES;
        voice.adpcmCurAddr8 += ADPCM_BLOCK_SIZE / 8;
        voice.bSamplesLoaded = false;

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
// Process/update all voices and get 1 sample of output from them
//------------------------------------------------------------------------------------------------------------------------------------------
static void stepVoices(
    Voice* const pVoices,
    const int32_t numVoices,
    const std::byte* pRam,
    const uint32_t ramSize,
    StereoSample& output,
    StereoSample& outputToReverb
) noexcept {
    ASSERT(pVoices || (numVoices == 0));

    for (int32_t voiceIdx = 0; voiceIdx < numVoices; ++voiceIdx) {
        stepVoice(pVoices[voiceIdx], pRam, ramSize, output, outputToReverb);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Mixes sound from an external input; does nothing if there is no current external input
//------------------------------------------------------------------------------------------------------------------------------------------
static void mixExternalInput(
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
static void doReverb(
    std::byte* pRam,
    const uint32_t ramSize,
    const uint32_t reverbBaseAddr8,
    uint32_t& reverbCurAddr,
    const Volume reverbVol,
    const bool bReverbWriteEnable,
    const ReverbRegs& reverbRegs,
    const StereoSample reverbInput,
    StereoSample& reverbOutput
) noexcept {
    // Helper: wrap an address to be within the reverb work area and guarantee that 16-bits can be read safely.
    // If there is no reverb work area (which should never be the case) then the address '0' is returned.
    const uint32_t reverbBaseAddr = reverbBaseAddr8 * 8;
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
        const uint32_t addr = wrapRevAddr16(reverbCurAddr + addrRelative);
        const uint16_t data = (uint16_t) pRam[addr] | ((uint16_t) pRam[addr + 1] << 8);
        return (int16_t) data;
    };

    const auto revW = [=](uint32_t addrRelative, const int16_t sample) noexcept {
        if (bReverbWriteEnable) {
            const uint16_t data = (uint16_t) sample;
            const uint32_t addr = wrapRevAddr16(reverbCurAddr + addrRelative);
            pRam[addr] = (std::byte) data;
            pRam[addr + 1] = (std::byte)(data >> 8);
        }
    };

    // This is based almost exactly on: https://problemkaputt.de/psx-spx.htm#spureverbformula.
    // First grab all of the reverb inputs we will be dealing with and the real relative reverb addresses (need to x8 them).
    const uint32_t addrLSame1   = (uint32_t) reverbRegs.addrLSame1 * 8;
    const uint32_t addrLSame2   = (uint32_t) reverbRegs.addrLSame2 * 8;
    const uint32_t addrRSame1   = (uint32_t) reverbRegs.addrRSame1 * 8;
    const uint32_t addrRSame2   = (uint32_t) reverbRegs.addrRSame2 * 8;
    const uint32_t addrLDiff1   = (uint32_t) reverbRegs.addrLDiff1 * 8;
    const uint32_t addrLDiff2   = (uint32_t) reverbRegs.addrLDiff2 * 8;
    const uint32_t addrRDiff1   = (uint32_t) reverbRegs.addrRDiff1 * 8;
    const uint32_t addrRDiff2   = (uint32_t) reverbRegs.addrRDiff2 * 8;
    const uint32_t addrLComb1   = (uint32_t) reverbRegs.addrLComb1 * 8;
    const uint32_t addrLComb2   = (uint32_t) reverbRegs.addrLComb2 * 8;
    const uint32_t addrLComb3   = (uint32_t) reverbRegs.addrLComb3 * 8;
    const uint32_t addrLComb4   = (uint32_t) reverbRegs.addrLComb4 * 8;
    const uint32_t addrRComb1   = (uint32_t) reverbRegs.addrRComb1 * 8;
    const uint32_t addrRComb2   = (uint32_t) reverbRegs.addrRComb2 * 8;
    const uint32_t addrRComb3   = (uint32_t) reverbRegs.addrRComb3 * 8;
    const uint32_t addrRComb4   = (uint32_t) reverbRegs.addrRComb4 * 8;
    const uint32_t addrLAPF1    = (uint32_t) reverbRegs.addrLAPF1 * 8;
    const uint32_t addrLAPF2    = (uint32_t) reverbRegs.addrLAPF2 * 8;
    const uint32_t addrRAPF1    = (uint32_t) reverbRegs.addrRAPF1 * 8;
    const uint32_t addrRAPF2    = (uint32_t) reverbRegs.addrRAPF2 * 8;
    const uint32_t dispAPF1     = (uint32_t) reverbRegs.dispAPF1 * 8;
    const uint32_t dispAPF2     = (uint32_t) reverbRegs.dispAPF2 * 8;

    const int16_t volWall   = reverbRegs.volWall;
    const int16_t volIIR    = reverbRegs.volIIR;
    const int16_t volComb1  = reverbRegs.volComb1;
    const int16_t volComb2  = reverbRegs.volComb2;
    const int16_t volComb3  = reverbRegs.volComb3;
    const int16_t volComb4  = reverbRegs.volComb4;
    const int16_t volAPF1   = reverbRegs.volAPF1;
    const int16_t volAPF2   = reverbRegs.volAPF2;

    // Scale the sample which is being fed into the reverb:
    const Sample inputL = reverbInput.left * reverbRegs.volLIn;
    const Sample inputR = reverbInput.right * reverbRegs.volRIn;

    // Same side reflection (left-to-left and right-to-right)
    {
        const Sample l1 = revR(addrLSame2);
        const Sample r1 = revR(addrRSame2);
        const Sample l2 = revR(addrLSame1 - 2);
        const Sample r2 = revR(addrRSame1 - 2);

        revW(addrLSame1, (inputL + l1 * volWall - l2) * volIIR + l2);   // Left to left
        revW(addrRSame1, (inputR + r1 * volWall - r2) * volIIR + r2);   // Right to right
    }

    // Different side reflection (left-to-right and right-to-left)
    {
        const Sample l1 = revR(addrLDiff2);
        const Sample r1 = revR(addrRDiff2);
        const Sample l2 = revR(addrLDiff1 - 2);
        const Sample r2 = revR(addrRDiff1 - 2);

        revW(addrLDiff1, (inputL + r1 * volWall - l2) * volIIR + l2);   // Right to left
        revW(addrRDiff1, (inputR + l1 * volWall - r2) * volIIR + r2);   // Left to right
    }

    // Early echo (comb filter, with input from buffer)
    Sample outL;
    Sample outR;

    outL = (
        (Sample) revR(addrLComb1) * volComb1 +
        (Sample) revR(addrLComb2) * volComb2 +
        (Sample) revR(addrLComb3) * volComb3 +
        (Sample) revR(addrLComb4) * volComb4
    );

    outR = (
        (Sample) revR(addrRComb1) * volComb1 +
        (Sample) revR(addrRComb2) * volComb2 +
        (Sample) revR(addrRComb3) * volComb3 +
        (Sample) revR(addrRComb4) * volComb4
    );

    // Late reverb APF1 (all pass filter 1, with input from COMB)
    outL = outL - (Sample) revR(addrLAPF1 - dispAPF1) * volAPF1;
    revW(addrLAPF1, outL);
    outL = outL * volAPF1 + revR(addrLAPF1 - dispAPF1);

    outR = outR - (Sample) revR(addrRAPF1 - dispAPF1) * volAPF1;
    revW(addrRAPF1, outR);
    outR = outR * volAPF1 + revR(addrRAPF1 - dispAPF1);

    // Late reverb APF2 (all pass filter 2, with input from APF1)
    outL = outL - (Sample) revR(addrLAPF2 - dispAPF2) * volAPF2;
    revW(addrLAPF2, outL);
    outL = outL * volAPF2 + revR(addrLAPF2 - dispAPF2);

    outR = outR - (Sample) revR(addrRAPF2 - dispAPF2) * volAPF2;
    revW(addrRAPF2, outR);
    outR = outR * volAPF2 + revR(addrRAPF2 - dispAPF2);

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
void doMasterMix(
    const StereoSample dryOutput,
    const StereoSample reverbOutput,
    const Volume masterVol,
    StereoSample& output
) noexcept {
    // Note: master volume is expected to be +/- 0x3FFF.
    // Need to clamp if exceeding this and also scale by 2.
    const StereoSample wetOutput = dryOutput + reverbOutput;
    const Volume scaledMasterVol = {
        (int16_t)(std::clamp(masterVol.left, MIN_MASTER_VOLUME, MAX_MASTER_VOLUME) * 2),
        (int16_t)(std::clamp(masterVol.right, MIN_MASTER_VOLUME, MAX_MASTER_VOLUME) * 2),
    };

    output = wetOutput * scaledMasterVol;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Core initialization and teardown
//------------------------------------------------------------------------------------------------------------------------------------------
void Spu::initCore(Core& core, const uint32_t ramSize, const uint32_t voiceCount) noexcept {
    // Zero init everything by default
    core = {};

    // Zero voices is a valid use-case, if for example you wanted to use this as a PS1 reverb DSP
    if (voiceCount > 0) {
        core.pVoices = new Voice[voiceCount];
        core.numVoices = voiceCount;
        
        for (uint32_t i = 0; i < voiceCount; ++i) {
            core.pVoices[i] = {};
        }
    }

    // Note: pad RAM size to the nearest 16-bytes to ensure the 8-byte addressing mode of the SPU always works.
    // Some SPU RAM must always be provided also, in order for the SPU to be used.
    ASSERT(ramSize > 0);
    const uint32_t roundedRamSize = ((ramSize + 15) / 16) * 16;

    core.pRam = new std::byte[roundedRamSize];
    core.ramSize = roundedRamSize;
    std::memset(core.pRam, 0, roundedRamSize);
}

void Spu::initPS1Core(Core& core) noexcept {
    Spu::initCore(core, 512 * 1024, 24);
}

void Spu::initPS2Core(Core& core) noexcept {
    Spu::initCore(core, 2048 * 1024, 48);
}

void Spu::destroyCore(Core& core) noexcept {
    delete[] core.pVoices;
    delete[] core.pRam;
    core = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Start playing the given voice
//------------------------------------------------------------------------------------------------------------------------------------------
StereoSample Spu::stepCore(Core& core) noexcept {
    // Process all voices firstly and silence the output if we are not unmuted
    StereoSample output = {};
    StereoSample outputToReverb = {};
    stepVoices(core.pVoices, core.numVoices, core.pRam, core.ramSize, output, outputToReverb);

    if (!core.bUnmute) {
        output = {};
        outputToReverb = {};
    }

    // Mix any external input
    if (core.bExtEnabled) {
        mixExternalInput(
            core.pExtInputCallback,
            core.pExtInputUserData,
            core.extInputVol,
            core.bExtReverbEnable,
            output,
            outputToReverb
        );
    }

    // Do reverb every 2 cycles: PSX reverb operates at 22,050 Hz and the SPU operates at 44,100 Hz
    if ((core.cycleCount & 1) == 0) {
        doReverb(
            core.pRam,
            core.ramSize,
            core.reverbBaseAddr8,
            core.reverbCurAddr,
            core.reverbVol,
            core.bReverbWriteEnable,
            core.reverbRegs,
            outputToReverb,
            core.processedReverb
        );
    }

    // Do the final mixing and finish up
    doMasterMix(output, core.processedReverb, core.masterVol, output);
    core.cycleCount++;
    return output;
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

    // Zero the 3 previous samples used for interpolation and previous 2 samples used for ADPCM decoding
    static_assert(Voice::NUM_PREV_SAMPLES == 3);
    voice.samples[0] = 0;
    voice.samples[1] = 0;
    voice.samples[2] = 0;
    voice.samples[Voice::SAMPLE_BUFFER_SIZE - 2] = 0;
    voice.samples[Voice::SAMPLE_BUFFER_SIZE - 1] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Puts the given voice into release mode
//------------------------------------------------------------------------------------------------------------------------------------------
void Spu::keyOff(Voice& voice) noexcept {
    voice.envPhase = EnvPhase::Release;
    voice.envWaitCycles = 0;
}
