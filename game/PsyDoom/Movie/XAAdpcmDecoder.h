#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(movie)

class CDXAFileStreamer;

BEGIN_NAMESPACE(XAAdpcmDecoder)

//------------------------------------------------------------------------------------------------------------------------------------------
// Decoding context: keeps track of previously decoded samples.
// Required for decoding ADPCM.
//------------------------------------------------------------------------------------------------------------------------------------------
struct Context {
    int16_t     prevSamplesL[2];    // Previously decoded samples (left channel, most recent first)
    int16_t     prevSamplesR[2];    // Previously decoded samples (right channel, most recent first)

    void init() noexcept;
    void addPrevLSample(const int16_t sampleL) noexcept;
    void addPrevRSample(const int16_t sampleR) noexcept;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Decoder constants
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint32_t ADPCM_CHUNKS_PER_SECTOR       = 18;                   // The number of chunks of ADPCM audio per CD sector
static constexpr uint32_t ADPCM_DATA_WORD_SIZE          = sizeof(uint32_t);     // The size of ADPCM data words (32-bit)
static constexpr uint32_t ADPCM_DATA_WORDS_PER_CHUNK    = 28;                   // How many data words there are in an ADPCM chunk

// The maximum number of samples that can be encoded in one ADPCM chunk.
// There are '28' 4-byte words of sample data in each chunk. In 4-bit encoding mode, those yield 2x4x28 (224) samples.
// Also, if we are decoding mono audio then those samples are doubled up in the buffer, yielding decoded x2 samples.
static constexpr uint32_t ADPCM_CHUNK_MAX_SAMPLES = (2 * ADPCM_DATA_WORD_SIZE * ADPCM_DATA_WORDS_PER_CHUNK) * 2;

// Max samples encoded per sector
static constexpr uint32_t SECTOR_MAX_ADPCM_SAMPLES = ADPCM_CHUNKS_PER_SECTOR * ADPCM_CHUNK_MAX_SAMPLES;

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds a single sector's worth of decoded ADPCM audio, always in stereo format.
// Depending on whether the ADPCM is 4-bit or 8-bit encoded not all of the samples might be used.
// The samples which are not decoded will have an undefined value.
//------------------------------------------------------------------------------------------------------------------------------------------
struct SectorAudio {
    // The number of samples decoded into the 'samples' array.
    // This is a combined count for the left and right channels, and is always a multiple of '2'.
    uint32_t numSamples;

    // The decoded samples for the sector, in interleaved stereo format
    int16_t samples[SECTOR_MAX_ADPCM_SAMPLES];
};

bool decode(CDXAFileStreamer& fileIn, Context& ctx, SectorAudio& out) noexcept;

END_NAMESPACE(XAAdpcmDecoder)
END_NAMESPACE(movie)
