#include "AudioCompressor.h"

#include <algorithm>
#include <cmath>

BEGIN_NAMESPACE(AudioCompressor)

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the audio compressor state with the specified settings.
//
// Setting meanings:
//  thresholdDB         Controls the signal level (in decibels) at which compression kicks in.
//  kneeWidthDB         How many decibels above the threshold the compression effect is faded in over. Softens compression ramp-up.
//  compressionRatio    How much the signal strength is divided when compressed. If '4' for example the signal strength is divided by '4'.
//  postGainDB          Gain (in decibels) to apply to the signal after compression.
//  lpfResponseTime     The 'time' in seconds it takes for the low pass filter to respond to changes in the waveform.
//                      Controls how much smoothing is applied to the waveform, or how low the filter's cut-off frequency is.
//                      Not an exact time, just a rough figure used to guide setting the parameter.
//  attackTime          An non-exact guide time (in seconds) for how long it takes the compressor to kick into full strength.
//  releaseTime         An non-exact guide time (in seconds) for how long it takes the compressor to relax back to fully off.
//------------------------------------------------------------------------------------------------------------------------------------------
void init(
    State& state,
    const float thresholdDB,
    const float kneeWidthDB,
    const float compressionRatio,
    const float postGainDB,
    const float lpfResponseTime,
    const float attackTime,
    const float releaseTime
) noexcept {
    // Compute the amount to lerp per sample for a 'target' lerp time specified in seconds
    const auto computeLerpFactor = [](const float lerpTimeInSeconds) noexcept -> float  {
        if (lerpTimeInSeconds > 0.0f) {
            const float lerpTimeInSamples = lerpTimeInSeconds * 44100.0f;
            return 1.0f - std::expf(-1.0f / lerpTimeInSamples);             // Note: exp(-x) = 1.0 / exp(x)
        } else {
            return 1.0f;
        }
    };

    state.thresholdDB = thresholdDB;
    state.kneeWidthDB = kneeWidthDB;
    state.compressionRatio = 1.0f / std::max(compressionRatio, 1.0f);
    state.postGainDB = postGainDB;
    state.lpfLerpFactor = computeLerpFactor(lpfResponseTime);
    state.attackLerpFactor = computeLerpFactor(attackTime);
    state.releaseLerpFactor = computeLerpFactor(releaseTime);
    state.lpfPrevSignalPower = {};
    state.prevSampleGainDB = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform dynamic range compression on a single audio sample.
// 
// References for this implementation:
//      https://openaudio.blogspot.com/2017/01/basic-dynamic-range-compressor.html
//      https://github.com/chipaudette/OpenAudio_ArduinoLibrary/blob/master/AudioEffectCompressor_F32.h
//      https://github.com/chipaudette/OpenAudio_ArduinoLibrary/blob/81492cc5aca290d95cd6b681729302148cb7e109/AudioEffectCompressor_F32.h
//------------------------------------------------------------------------------------------------------------------------------------------
void compress(State& state, float& sampleL, float& sampleR) noexcept {
    // Compute the instantaneous signal power of the current sample by squaring the signal (Ohm's law).
    // Take the max power of both channels:
    const float signalPower = std::max(sampleL * sampleL, sampleR * sampleR);

    // Smooth this power with the previous sample and save as the new smoothed sample.
    // Note: prevent smoothed signal power from dropping below -100 dBFS to prevent negative infinity when calculating signal power.
    const float lpfLerp = state.lpfLerpFactor;
    const float smoothedSignalPower = std::max(signalPower * lpfLerp + state.lpfPrevSignalPower * (1.0f - lpfLerp), 1.0e-10f);
    state.lpfPrevSignalPower = smoothedSignalPower;

    // Compute the signal power in decibels.
    // Note: normally the formula to convert is 'dB = 20 * log10(amplitude)' but we've already squared the signal so instead the multiply is by '10' instead.
    const float signalPowerDB = std::clamp(10.0f * std::log10(smoothedSignalPower), -100.0f, +100.0f);

    // Compute the instantaneous/non-smoothed gain in decibels.
    // Don't allow a positive gain, only negative!
    const float aboveThresholdDB = signalPowerDB - state.thresholdDB;
    const float goalAboveThresholdDB = aboveThresholdDB * state.compressionRatio;
    const float kneeWidthDB = state.kneeWidthDB;
    const float compressionStrength = std::clamp((kneeWidthDB > 0) ? aboveThresholdDB / kneeWidthDB : 1.0f, 0.0f, 1.0f);
    const float smoothedGoalAboveThresholdDB = goalAboveThresholdDB * compressionStrength + (1.0f - compressionStrength) * aboveThresholdDB;
    const float instantGainDB = std::clamp(smoothedGoalAboveThresholdDB - aboveThresholdDB, -100.0f, 0.0f);

    // Compute the smoothed gain in decibels and save as the new smoothed sample
    const float prevGainDB = state.prevSampleGainDB;
    const float gainLerp = (instantGainDB < prevGainDB) ? state.attackLerpFactor : state.releaseLerpFactor;
    const float smoothedGainDB = instantGainDB * gainLerp + prevGainDB * (1.0f - gainLerp);
    state.prevSampleGainDB = smoothedGainDB;

    // Convert the gain to linear gain and apply it to the sample.
    // Also apply the post gain at this point...
    const float finalGainDB = smoothedGainDB + state.postGainDB;
    const float linearGain = std::pow(10.0f, finalGainDB * (1.0f / 20.0f));
    sampleL *= linearGain;
    sampleR *= linearGain;
}

END_NAMESPACE(AudioCompressor)
