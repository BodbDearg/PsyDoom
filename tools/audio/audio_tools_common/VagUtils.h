#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>
#include <vector>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(VagUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// VAG file constants
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint32_t VAG_FILE_ID       = 0x70474156;
static constexpr uint32_t VAG_FILE_VERSION  = 0x03;

//------------------------------------------------------------------------------------------------------------------------------------------
// PlayStation ADPCM format constants
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint32_t   ADPCM_BLOCK_SIZE        = 16;       // The size in bytes of a PSX format ADPCM block
static constexpr uint32_t   ADPCM_BLOCK_NUM_SAMPLES = 28;       // The number of samples in a PSX format ADPCM block

//------------------------------------------------------------------------------------------------------------------------------------------
// Flags read from the 2nd byte of a PSX ADPCM block.
//
// Meanings:
//  LOOP_END:       If set then goto the repeat address after we are finished with the current ADPCM block
//  REPEAT:         Only used if 'LOOP_END' is set, whether we are repeating normally or silencing the voice.
//                  If NOT set when reaching a sample end , then the volumne envelope is immediately silenced.
//  LOOP_START:     If set then save the current ADPCM address as the repeat address
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint8_t ADPCM_FLAG_LOOP_END    = 0x01;
static constexpr uint8_t ADPCM_FLAG_REPEAT      = 0x02;
static constexpr uint8_t ADPCM_FLAG_LOOP_START  = 0x04;

//------------------------------------------------------------------------------------------------------------------------------------------
// Header format for a PS1 .VAG.
// Note that this header is stored in BIG ENDIAN format in the file!
//------------------------------------------------------------------------------------------------------------------------------------------
struct VagFileHdr {
    uint32_t    fileId;             // Should say 'VAGp'
    uint32_t    version;            // Should be '0x03'
    uint32_t    _reserved1;         // Unused...
    uint32_t    size;               // Size of the header + sound data
    uint32_t    sampleRate;         // Sound data sample rate
    uint32_t    _reserved2[3];      // Unused...
    char        name[16];           // Name for the VAG
    uint32_t    _unknown[4];        // Purpose unknown

    void endianCorrect() noexcept;
    bool validate() noexcept;
};

static_assert(sizeof(VagFileHdr) == 64);

void decodePsxAdpcmSamples(
    const std::byte* const pData,
    const uint32_t dataSize,
    std::vector<int16_t>& samplesOut,
    uint32_t& loopStartSampleIdx,
    uint32_t& loopEndSampleIdx
) noexcept;

bool writePsxAdpcmSoundToVagFile(
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate,
    const char* const filePath
) noexcept;

END_NAMESPACE(VagUtils)
END_NAMESPACE(AudioTools)
