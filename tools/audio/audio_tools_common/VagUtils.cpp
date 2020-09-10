#include "VagUtils.h"

#include "Asserts.h"
#include "Endian.h"
#include "Finally.h"

#include <algorithm>
#include <cstdio>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(VagUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Do byte swapping for little endian host CPUs.
// The VAG header is stored in big endian format in the file.
//------------------------------------------------------------------------------------------------------------------------------------------
void VagFileHdr::endianCorrect() noexcept {
    if (Endian::isLittle()) {
        fileId = Endian::byteSwap(fileId);
        version = Endian::byteSwap(version);
        _reserved1 = Endian::byteSwap(_reserved1);
        size = Endian::byteSwap(size);
        sampleRate = Endian::byteSwap(sampleRate);

        for (uint32_t& value : _reserved2) {
            value = Endian::byteSwap(value);
        }

        for (uint32_t& value : _unknown) {
            value = Endian::byteSwap(value);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verify the VAG header is valid
//------------------------------------------------------------------------------------------------------------------------------------------
bool VagFileHdr::validate() noexcept {
    return (
        (fileId == VAG_FILE_ID) &&
        (version == VAG_FILE_VERSION) &&
        (size > sizeof(VagFileHdr)) &&
        (sampleRate > 0)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decodes the specified PSX ADPCM data from a .VAG file (or raw ADPCM) to the given buffer.
// Also saves the loop start and end sample indexes if the sample is looped.
// If the sample is NOT looped then these will both be set to zero.
// This code is more or less copied from the Spu implementation that PsyDoom uses.
//------------------------------------------------------------------------------------------------------------------------------------------
void decodeAdpcmSamples(
    const std::byte* const pData,
    const uint32_t dataSize,
    std::vector<int16_t>& samplesOut,
    uint32_t& loopStartSampleIdx,
    uint32_t& loopEndSampleIdx
) noexcept {
    ASSERT(pData || (dataSize == 0));

    // How many sample blocks are there in the data?
    const uint32_t numSampleBlocks = dataSize / ADPCM_BLOCK_SIZE;

    // Setup the output buffer and set there to be no loop points initially
    samplesOut.clear();
    samplesOut.reserve((size_t) numSampleBlocks * ADPCM_BLOCK_NUM_SAMPLES);

    loopStartSampleIdx = 0;
    loopEndSampleIdx = 0;

    // Hold the last 2 ADPCM samples we decoded here with the newest first.
    // They are required for the adaptive decoding throughout and carry across ADPCM blocks.
    int16_t prevSamples[2] = { 0, 0 };

    // Continue decoding ADPCM blocks until there is no more
    bool bFoundLoopEnd = false;

    for (uint32_t sampleBlockIdx = 0; sampleBlockIdx < numSampleBlocks; ++sampleBlockIdx) {
        // Grab the data for this sample block
        uint8_t adpcmBlock[ADPCM_BLOCK_SIZE];
        std::memcpy(adpcmBlock, pData + sampleBlockIdx * ADPCM_BLOCK_SIZE, ADPCM_BLOCK_SIZE);

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

        // Check for looping flags in the second ADPCM header byte
        if (adpcmBlock[1] & ADPCM_FLAG_LOOP_START) {
            // Only use loop start if we haven't encountered a loop end yet.
            // Otherwise it will never be reached, unless the host software redirects the flow...
            if (!bFoundLoopEnd) {
                loopStartSampleIdx = sampleBlockIdx * ADPCM_BLOCK_NUM_SAMPLES;
            }
        }

        if ((adpcmBlock[1] & ADPCM_FLAG_LOOP_END) && (adpcmBlock[1] & ADPCM_FLAG_REPEAT)) {
            // Found the end of a sound that will loop.
            // Note that the loop end happens AFTER the end of the current block.
            bFoundLoopEnd = true;
            loopEndSampleIdx = (sampleBlockIdx + 1) * ADPCM_BLOCK_NUM_SAMPLES;
        }

        // Get the ADPCM filter co-efficients, both positive and negative.
        // For more details on this see: https://problemkaputt.de/psx-spx.htm#cdromxaaudioadpcmcompression
        constexpr int32_t FILTER_COEF_POS[5] = { 0, 60, 115,  98, 122 };
        constexpr int32_t FILTER_COEF_NEG[5] = { 0,  0, -52, -55, -60 };
        const int32_t filterCoefPos = FILTER_COEF_POS[adpcmFilter];
        const int32_t filterCoefNeg = FILTER_COEF_NEG[adpcmFilter];

        // Decode all of the samples in the block
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
            samplesOut.push_back((int16_t) sample);

            // Move previous samples forward
            prevSamples[1] = prevSamples[0];
            prevSamples[0] = (int16_t) sample;
        }
    }

    // If we didn't find a loop end then ignore any loop starts encountered
    if (!bFoundLoopEnd) {
        loopStartSampleIdx = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a sound encoded in the PlayStation's ADPCM format to the given VAG file on disk.
// Returns 'true' if the write was successful.
//------------------------------------------------------------------------------------------------------------------------------------------
bool writeAdpcmSoundToVagFile(
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate,
    const char* const filePath
) noexcept {
    // The data size must be a multiple of 16 or the PSX ADPCM block size
    if ((adpcmDataSize % ADPCM_BLOCK_SIZE) != 0)
        return false;

    // Open the specified file for writing
    FILE* const pFile = std::fopen(filePath, "wb");

    if (!pFile)
        return false;

    Finally closeFileOnExit = finally([=]() noexcept {
        std::fclose(pFile);
    });

    // Makeup the .VAG file header and write to the file
    const uint32_t vagTotalSize = sizeof(VagUtils::VagFileHdr) + adpcmDataSize;

    VagFileHdr vagHdr = {};
    vagHdr.fileId = VagUtils::VAG_FILE_ID;
    vagHdr.version = VagUtils::VAG_FILE_VERSION;
    vagHdr.size = vagTotalSize;
    vagHdr.sampleRate = sampleRate;
    vagHdr.endianCorrect();

    if (std::fwrite(&vagHdr, sizeof(VagFileHdr), 1, pFile) != 1)
        return false;

    // Write the ADPCM and flush to finish up
    if (std::fwrite(pAdpcmData, adpcmDataSize, 1, pFile) != 1)
        return false;

    return (std::fflush(pFile) == 0);
}

END_NAMESPACE(VagUtils)
END_NAMESPACE(AudioTools)
