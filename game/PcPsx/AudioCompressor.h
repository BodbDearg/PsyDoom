#include "Macros.h"

BEGIN_NAMESPACE(AudioCompressor)

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds settings and state for the audio compressor
//------------------------------------------------------------------------------------------------------------------------------------------
struct State {
    float thresholdDB;          // The compression threshold in dB (decibels); audio above this level is compressed
    float kneeWidthDB;          // How many decibels above the threshold the compression effect is faded in over. Softens compression ramp-up.
    float compressionRatio;     // The compression ratio multiplier
    float postGainDB;           // Post gain to apply to the signal after compression (in dB)
    float lpfLerpFactor;        // Specifies how much of the new sample we use when applying the low pass filter
    float attackLerpFactor;     // Specifies how much of the new gain value to use when the compressor is in the attack envelope phase
    float releaseLerpFactor;    // Specifies how much of the new gain value to use when the compressor is in the release envelope phase
    float lpfPrevSignalPower;   // Previous sample signal power computed by the low pass filter
    float prevSampleGainDB;     // Previous sample gain in dB (decibels) - used for attack and release envelope smoothing
};

void init(
    State& state,
    const float thresholdDB,
    const float kneeWidthDB,
    const float compressionRatio,
    const float postGainDB,
    const float lpfResponseTime,
    const float attackTime,
    const float releaseTime
) noexcept;

void compress(State& state, float& sampleL, float& sampleR) noexcept;

END_NAMESPACE(AudioCompressor)
