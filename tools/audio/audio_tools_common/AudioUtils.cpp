#include "AudioUtils.h"

#include <cmath>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(AudioUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out the sample rate of a given note (specified in semitones) using a reference base note (in semitones) and the sample
// rate that the base note sounds at. This is similar to a utility implemented for PsyDoom in the LIBSPU module.
//
// For a good explantion of the conversion from note to frequency, see:
//  https://www.translatorscafe.com/unit-converter/en-US/calculator/note-frequency/
//------------------------------------------------------------------------------------------------------------------------------------------
float getNoteSampleRate(const float baseNote, const float baseNoteSampleRate, const float note) noexcept {
    const float noteOffset = note - baseNote;
    const float sampleRate = baseNoteSampleRate * std::powf(2.0f, noteOffset / 12.0f);
    return sampleRate;
}

double getNoteSampleRate(const double baseNote, const double baseNoteSampleRate, const double note) noexcept {
    const double noteOffset = note - baseNote;
    const double sampleRate = baseNoteSampleRate * std::pow(2.0, noteOffset / 12.0);
    return sampleRate;
}

END_NAMESPACE(AudioUtils)
END_NAMESPACE(AudioTools)
