#pragma once

#include "PsxAdsrEnvelope.h"

#include <rapidjson/document.h>

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Settings for one individual voice/sound in a patch/instrument
//------------------------------------------------------------------------------------------------------------------------------------------
struct PsxPatchVoice {
    uint16_t    sampleIdx;          // The index of the patch sample to use for this voice
    uint8_t     volume;             // Volume to play the voice at
    uint8_t     pan;                // Voice pan 64 to 191 - 128 is the voice center. Values outside this range are clamped by the PSX driver.
    uint8_t     reverb;             // How much reverb to apply to the voice. Note: the PSX sound driver ignores this.
    uint8_t     baseNote;           // The 'base' note/semitone at which the sound sample is regarded to play back at 44,100 Hz (used to compute sample frequency for triggered notes)
    uint8_t     baseNoteFrac;       // The .8 fractional part of the base note
    uint8_t     noteMin;            // Minimum semitone at which the voice can be played - does not sound below this
    uint8_t     noteMax;            // Maximum semitone at which the voice can be played - does not sound above this
    uint8_t     pitchstepDown;      // How big each unit of pitch shift is when pitch shifting down (in 1/8192 units)
    uint8_t     pitchstepUp;        // How big each unit of pitch shift is when pitch shifting up (in 1/8192 units)
    uint8_t     priority;           // Voice priority: used to determine what to kill when we're out of voices

    // ADSR envelope for the voice
    union {
        PsxAdsrEnvelope     adsr;
        uint32_t            adsrBits;
    };
    
    void readFromJson(const rapidjson::Value& jsonRoot) THROWS;
    void writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept;
    void readFromWmd(InputStream& in) THROWS;
    void writeToWmd(OutputStream& out) const THROWS;
};

END_NAMESPACE(AudioTools)
