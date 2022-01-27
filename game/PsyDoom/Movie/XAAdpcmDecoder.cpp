#include "XAAdpcmDecoder.h"

#include "Asserts.h"
#include "CDXAFileStreamer.h"

#include <algorithm>
#include <cstring>

BEGIN_NAMESPACE(movie)
BEGIN_NAMESPACE(XAAdpcmDecoder)

//------------------------------------------------------------------------------------------------------------------------------------------
// Filter coefficients for XA-ADPCM, positive and negative.
// For more info see: https://problemkaputt.de/psx-spx.htm#cdromxaaudioadpcmcompression
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int16_t XA_ADPCM_POS_FILTER_TBL[5] = { 0, +60, +115, +98, +122 };
static constexpr int16_t XA_ADPCM_NEG_FILTER_TBL[5] = { 0,   0,  -52, -55, -60  };

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: returns the number of samples that will be decoded per ADPCM chunk for the specified format
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t getSamplesPerAdpcmChunk(const bool bStereo, const bool b8Bit) noexcept {
    if (bStereo) {
        return (b8Bit) ? ADPCM_CHUNK_MAX_SAMPLES / 4 : ADPCM_CHUNK_MAX_SAMPLES / 2;
    } else {
        // For mono we double up the decoded audio across 2 channels
        return (b8Bit) ? ADPCM_CHUNK_MAX_SAMPLES / 2 : ADPCM_CHUNK_MAX_SAMPLES;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: converts an unsigned 4-bit sample to a signed 16-bit quantity
//------------------------------------------------------------------------------------------------------------------------------------------
static int16_t sampleNibbleToSigned16Bit(const uint16_t nibble) noexcept {
    if (nibble & 0x8) {
        return (int16_t)((int32_t)(nibble << 12u) - 65536);
    } else {
        return (int16_t)(nibble << 12u);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: converts an unsigned 8-bit sample to a signed 16-bit quantity
//------------------------------------------------------------------------------------------------------------------------------------------
static int16_t sampleByteToSigned16Bit(const uint16_t byte) noexcept {
    if (byte & 0x80) {
        return (int16_t)((int32_t)(byte << 8u) - 65536);
    } else {
        return (int16_t)(byte << 8u);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: decodes a single ADPCM sample.
// Takes as input the decoding context, the unfiltered 16-bit sample, the filter coefficients, the shift amount and the previous samples.
//------------------------------------------------------------------------------------------------------------------------------------------
static int16_t decodeAdpcmSample(
    const int16_t unfilteredSample,
    const int16_t prevSample1,          // Old sample
    const int16_t prevSample2,          // Oldest sample
    const uint8_t sampleShift,
    const int16_t posFilter,
    const int16_t negFilter
) noexcept {
    const int32_t filteredSample = (unfilteredSample >> sampleShift) + (prevSample1 * posFilter + prevSample2 * negFilter + 32) / 64;
    return (int16_t) std::clamp(
        filteredSample,
        (int32_t) std::numeric_limits<int16_t>::min(),
        (int32_t) std::numeric_limits<int16_t>::max()
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decodes a chunk of ADPCM samples in 4-bit or 8-bit mono or stereo formats.
// For more on this, see: https://problemkaputt.de/psx-spx.htm#cdromxaaudioadpcmcompression
//------------------------------------------------------------------------------------------------------------------------------------------
template <bool FOUR_BIT, bool STEREO>
static void decodeAdpcmChunk(Context& ctx, const uint8_t* const pDataIn, int16_t* const pSamplesOut) noexcept {
    // Decode the 4/8 blocks/groups of samples; each block has 28 individual samples spread across the 28 32-bit data words:
    ASSERT_LOG((uintptr_t) pDataIn % sizeof(uint32_t) == 0, "Input pointer not 32-bit aligned, unaligned access!");
    const uint32_t* const pDataWords = (uint32_t*)(pDataIn + 16);
    int16_t* pOutSample = pSamplesOut;

    // How many blocks of samples are there?
    // This depends on whether the sample format is 4 or 8 bit:
    constexpr uint32_t NUM_BLOCKS = (FOUR_BIT) ? 8 : 4;

    for (uint32_t blockIdx = 0; blockIdx < NUM_BLOCKS; ++blockIdx) {
        // Note: the XA-ADPCM headers for each block are found in the first 16-bytes.
        // The first and last 4 bytes are copies of the ones in the middle, starting at byte 4.
        const uint8_t xaAdpcmHdr = pDataIn[4 + blockIdx];

        // Get the sample shift and filter coefficients via the XA-ADPCM header
        const uint8_t xaHdrShiftBits = xaAdpcmHdr & 0xFu;
        const uint8_t xaHdrFilterBits = (xaAdpcmHdr >> 4) & 0x3u;
        const uint8_t sampleShift = (xaHdrShiftBits < 13) ? xaHdrShiftBits : 9;   // Per NO$PSX: shift >= 13 acts the same as '9'
        const int16_t posFilter = XA_ADPCM_POS_FILTER_TBL[xaHdrFilterBits];
        const int16_t negFilter = XA_ADPCM_NEG_FILTER_TBL[xaHdrFilterBits];

        // Decode each sample in the block (28 total)
        for (uint32_t sampleIdx = 0; sampleIdx < ADPCM_DATA_WORDS_PER_CHUNK; ++sampleIdx) {
            // Get the unfiltered 4 or 8 bit sample and convert to a signed 16-bit sample:
            int16_t unfilteredSample;

            if constexpr (FOUR_BIT) {
                const uint8_t sampleNibble = (uint8_t)((pDataWords[sampleIdx] >> (blockIdx * 4)) & 0xFu);
                unfilteredSample = sampleNibbleToSigned16Bit(sampleNibble);
            } else {
                const uint8_t sampleByte = (uint8_t)(pDataWords[sampleIdx] >> (blockIdx * 8));
                unfilteredSample = sampleByteToSigned16Bit(sampleByte);
            }

            // Are we dealing with a mono or stereo data stream?
            // If it's mono then we double up the sample across both channels:
            if constexpr (STEREO) {
                // Stereo: is this the left or right channel?
                // The blocks for each channel are interleaved, with left being the first:
                const bool bIsRightChannel = (blockIdx & 1);

                if (bIsRightChannel) {
                    const int16_t prevSample1 = ctx.prevSamplesR[0];
                    const int16_t prevSample2 = ctx.prevSamplesR[1];
                    const int16_t sample = decodeAdpcmSample(unfilteredSample, prevSample1, prevSample2, sampleShift, posFilter, negFilter);
                    pOutSample[0] = sample;
                    ctx.addPrevRSample(sample);
                } else {
                    const int16_t prevSample1 = ctx.prevSamplesL[0];
                    const int16_t prevSample2 = ctx.prevSamplesL[1];
                    const int16_t sample = decodeAdpcmSample(unfilteredSample, prevSample1, prevSample2, sampleShift, posFilter, negFilter);
                    pOutSample[0] = sample;
                    ctx.addPrevLSample(sample);
                }

                pOutSample += 2;    // Note: the other channel will be handled by a separate block
            } else {
                // Mono: double up the sample for the left and right channels
                const int16_t prevSample1 = ctx.prevSamplesL[0];
                const int16_t prevSample2 = ctx.prevSamplesL[1];
                const int16_t sample = decodeAdpcmSample(unfilteredSample, prevSample1, prevSample2, sampleShift, posFilter, negFilter);
                pOutSample[0] = sample;
                pOutSample[1] = sample;
                ctx.addPrevLSample(sample);
                ctx.addPrevRSample(sample);
                pOutSample += 2;
            }
        }

        // Handle the end of the block for stereo mode, need to do rewinding due to the stride between channel samples
        if constexpr (STEREO) {
            const bool bIsRightChannel = (blockIdx & 1);

            if (bIsRightChannel) {
                // Put us onto the left channel for the next block
                pOutSample -= 1;
            } else {
                // Put us onto the right channel for the current block
                pOutSample -= ADPCM_DATA_WORDS_PER_CHUNK * 2;
                pOutSample += 1;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the ADPCM decoding context.
// Sets the values for the previous samples to zero.
//------------------------------------------------------------------------------------------------------------------------------------------
void Context::init() noexcept {
    std::memset(this, 0, sizeof(*this));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a new sample (left or right) to the list of previous samples for the context
//------------------------------------------------------------------------------------------------------------------------------------------
void Context::addPrevLSample(const int16_t sampleL) noexcept {
    prevSamplesL[1] = prevSamplesL[0];
    prevSamplesL[0] = sampleL;
}

void Context::addPrevRSample(const int16_t sampleR) noexcept {
    prevSamplesR[1] = prevSamplesR[0];
    prevSamplesR[0] = sampleR;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decodes one sector's worth of CDXA ADCPM audio
//------------------------------------------------------------------------------------------------------------------------------------------
bool decode(CDXAFileStreamer& fileIn, Context& ctx, SectorAudio& out) noexcept {
    // Firstly grab an audio sector from the stream.
    // If that fails then just provide an empty audio sector as output.
    out.numSamples = 0;

    const CDXASector* const pSector = fileIn.peekOrReadSectorType([](const CDXASector& sector) noexcept {
        return sector.header.isAudioSector();
    });

    if (!pSector) {
        return false;
    }

    // Find out whether the format is mono or stereo, 4-bits per sample or 8-bit:
    const bool bStereo = pSector->header.isStereo();
    const bool b8Bit = (pSector->header.getBitsPerSample() >= 8);
    const uint32_t samplesPerChunk = getSamplesPerAdpcmChunk(bStereo, b8Bit);

    // Decode each chunk of ADPCM data in the sector; each chunk is 128 bytes in size
    constexpr uint32_t ADPCM_CHUNK_SIZE = 128;
    static_assert(ADPCM_CHUNK_SIZE * ADPCM_CHUNKS_PER_SECTOR <= sizeof(CDXASector::data));

    {
        const uint8_t* pDataIn = pSector->data;
        int16_t* pSamplesOut = out.samples;

        // Each of these loops is specialized to the particular format present in the sector
        if (bStereo) {
            // Stereo
            if (b8Bit) {
                // 8-bit Stereo
                for (uint32_t chunkIdx = 0; chunkIdx < ADPCM_CHUNKS_PER_SECTOR; ++chunkIdx) {
                    decodeAdpcmChunk<false, true>(ctx, pDataIn, pSamplesOut);
                    pDataIn += ADPCM_CHUNK_SIZE;
                    pSamplesOut += samplesPerChunk;
                }
            } else {
                // 4-bit Stereo
                for (uint32_t chunkIdx = 0; chunkIdx < ADPCM_CHUNKS_PER_SECTOR; ++chunkIdx) {
                    decodeAdpcmChunk<true, true>(ctx, pDataIn, pSamplesOut);
                    pDataIn += ADPCM_CHUNK_SIZE;
                    pSamplesOut += samplesPerChunk;
                }
            }
        } else {
            // Mono
            if (b8Bit) {
                // 8-bit Mono
                for (uint32_t chunkIdx = 0; chunkIdx < ADPCM_CHUNKS_PER_SECTOR; ++chunkIdx) {
                    decodeAdpcmChunk<false, false>(ctx, pDataIn, pSamplesOut);
                    pDataIn += ADPCM_CHUNK_SIZE;
                    pSamplesOut += samplesPerChunk;
                }
            } else {
                // 4-bit Mono
                for (uint32_t chunkIdx = 0; chunkIdx < ADPCM_CHUNKS_PER_SECTOR; ++chunkIdx) {
                    decodeAdpcmChunk<true, false>(ctx, pDataIn, pSamplesOut);
                    pDataIn += ADPCM_CHUNK_SIZE;
                    pSamplesOut += samplesPerChunk;
                }
            }
        }
    }

    // Decoding succeeded, record the number of output samples and the sample rate
    out.numSamples = samplesPerChunk * ADPCM_CHUNKS_PER_SECTOR;
    out.sampleRate = pSector->header.getSampleRate();

    // Free up the cd sector since we are now done with it and return 'true' for success
    fileIn.freeSector(*pSector);
    return true;
}

END_NAMESPACE(XAAdpcmDecoder)
END_NAMESPACE(movie)
