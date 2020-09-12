#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// PlayStation format ADSR envelope applied by the SPU to playing sounds.
// This is taken more or less verbatim from the 'SimpleSpu' implementation used by PsyDoom.
// This is the same bit layout (32-bits total) used by the PSX SPU itself.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PsxAdsrEnvelope {
    // 0-15: At what envelope level (inclusive) do we go from the decay phase into the sustain phase.
    // The actual envelope level is computed as follows: max((sustainLevel + 1) << 11, 0x7FFF)
    uint32_t sustainLevel : 4;

    // 0-15: Affects how long the decay portion of the envelope lasts.
    // Lower values mean a faster decay.
    uint32_t decayShift : 4;

    // 0-3: Affects how long the attack portion of the envelope lasts; higher values mean a faster attack.
    // The actual step is computed as follows: 7 - step.
    uint32_t attackStep : 2;

    // 0-31: Affects how long the attack portion of the envelope lasts.
    // Lower values mean a faster attack.
    uint32_t attackShift : 5;

    // If set then attack mode is exponential rather than linear
    uint32_t bAttackExp : 1;

    // 0-31: Affects how long the release portion of the envelope lasts.
    // Lower values mean a faster release.
    uint32_t releaseShift : 5;

    // If set then release mode is exponential rather than linear
    uint32_t bReleaseExp : 1;

    // How much to step the envelope in sustain mode.
    // The meaning of this depends on whether the sustain direction is 'increase' or 'decrease'.
    //  Increase: step =  7 - sustainStep
    //  Decrease: step = -8 + sustainStep
    uint32_t sustainStep : 2;

    // 0-31: Affects how long the sustain portion of the envelope lasts.
    // Lower values mean a faster sustain.
    uint32_t sustainShift : 5;

    // An unused bit of the envelope
    uint32_t _unused: 1;

    // If set then the sustain envelope is decreased over time.
    // If NOT set then it increases.
    uint32_t bSustainDec : 1;

    // Whether the sustain portion of the envelope increases or decreases exponentially.
    // If set then the change is exponential.
    uint32_t bSustainExp : 1;
};

END_NAMESPACE(AudioTools)
