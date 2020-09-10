#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>
#include <vector>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(WavUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Various 32-bit ids used .wav files encoded as 32-bit values, with the first character being the lowest byte
//------------------------------------------------------------------------------------------------------------------------------------------
enum class WavFileId : uint32_t {
    RIFF    = 0x46464952,
    WAVE    = 0x45564157,
    fmt     = 0x20746D66,
    smpl    = 0x6C706D73,
    data    = 0x61746164,
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Wave file compression formats, not are listed here - just the most common ones...
//------------------------------------------------------------------------------------------------------------------------------------------
enum class WavFormat : uint16_t {
    Unknown         = 0x0000,
    PCM             = 0x0001,
    MicrosoftAdpcm  = 0x0002,
    MPEG            = 0x0050,
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Looping mode for a sampler loop
//------------------------------------------------------------------------------------------------------------------------------------------
enum class SmplLoopMode : uint32_t {
    LoopForward     = 0,    // Play forward from the loop start point
    PingPong        = 1,    // Play forward and then backward from the loop end point
    LoopBackward    = 2,    // Play backward to the loop start point
};

//------------------------------------------------------------------------------------------------------------------------------------------
// The header for .wav files root chunk
//------------------------------------------------------------------------------------------------------------------------------------------
struct WavRootChunkHdr {
    WavFileId   fileId;         // Should be 'RIFF'
    uint32_t    chunkSize;      // Size of the chunk's data; if an odd number then the chunks actual physical size is padded by 1 extra byte
    WavFileId   riffTypeId;     // Should be 'WAVE'

    void endianCorrect() noexcept;
    bool validate() noexcept;
};

static_assert(sizeof(WavRootChunkHdr) == 12);

//------------------------------------------------------------------------------------------------------------------------------------------
// Header for a child chunk (fmt, smpl, data etc.) in a .wav file
//------------------------------------------------------------------------------------------------------------------------------------------
struct WavChunkHdr {
    WavFileId   chunkId;        // What type of chunk this is
    uint32_t    chunkSize;      // Size of the chunk's data; if an odd number then the chunks actual physical size is padded by 1 extra byte

    void endianCorrect() noexcept;
};

static_assert(sizeof(WavChunkHdr) == 8);

//------------------------------------------------------------------------------------------------------------------------------------------
// Header for the wave file format description chunk - for uncompressed wave files only.
// An additional 2-byte field describing the number of extra format bytes is omitted here as that is only present for compressed .wav files.
//------------------------------------------------------------------------------------------------------------------------------------------
struct FmtChunkHdr_Uncompressed {
    WavFormat   format;             // Whether the file is compressed or not: only uncompressed is supported!
    uint16_t    numChannels;        // 2 = Stereo,  1 = Mono
    uint32_t    sampleRate;         // In Hz, e.g: 44100
    uint32_t    avgBytesPerSec;     // Estimated bytes per second for streaming: sampleRate * blockAlign
    uint16_t    blockAlign;         // Bytes per sample slice: (bits per sample / 8) * numChannels
    uint16_t    bitsPerSample;      // 16 or 8 etc: only '16' is currently supported

    void endianCorrect() noexcept;
    bool validate() noexcept;
};

static_assert(sizeof(FmtChunkHdr_Uncompressed) == 16);

//------------------------------------------------------------------------------------------------------------------------------------------
// Header for the sampler device description chunk.
// This is where we can save loopn points.
//------------------------------------------------------------------------------------------------------------------------------------------
struct SamplerChunkHdr {
    uint32_t manufacturer;      // Not using: MIDI registered manufacturer for sampler
    uint32_t product;           // Not using: MIDI registered product for sampler
    uint32_t samplePeriod;      // Duration of time that passes during 1 sample in nanoseconds
    uint32_t baseNote;          // MIDI note at which the sample is played back at its original sample rate
    uint32_t baseNoteFrac;      // Fraction of a note for base note: 0x80000000 = 0.5 of a semitone
    uint32_t smpteFormat;       // Not using: timing related stuff...
    uint32_t smpteOffset;       // Not using: timing related stuff...
    uint32_t numSampleLoops;    // Number of sample loops
    uint32_t samplerDataSize;   // Extra sampler data size (after sampler loops)

    void endianCorrect() noexcept;
};

static_assert(sizeof(SamplerChunkHdr) == 36);

//------------------------------------------------------------------------------------------------------------------------------------------
// Format for a sampler loop
//------------------------------------------------------------------------------------------------------------------------------------------
struct SamplerLoop {
    uint32_t        cuePointId;     // Unique id for this loop cue point, so things like labels can be associated with it
    SmplLoopMode    loopMode;       // 0 = Normal loop forward
    uint32_t        startSamp;      // Start of loop (in samples)
    uint32_t        endSamp;        // End of loop (in samples, note: this sample is PLAYED also!)
    uint32_t        fraction;       // Fraction of a sample at which to loop - 0x80000000 = 0.5 of a sample
    uint32_t        playCount;      // 0 = play forever

    void endianCorrect() noexcept;
};

static_assert(sizeof(SamplerLoop) == 24);

bool writePcmSoundToWavFile(
    const int16_t* const pSamples,
    const uint32_t numSamples,
    const uint16_t numChannels,
    const uint32_t sampleRate,
    const uint32_t loopStartSample,
    const uint32_t loopEndSample,
    const char* const filePath
) noexcept;

bool writePsxAdpcmSoundToWavFile(
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate,
    const char* const filePath
) noexcept;

END_NAMESPACE(WavUtils)
END_NAMESPACE(AudioTools)
