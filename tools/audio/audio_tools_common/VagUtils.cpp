#include "VagUtils.h"

#include "Asserts.h"
#include "Endian.h"
#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "FileUtils.h"

#include <algorithm>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(VagUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// PlayStation ADPCM compression linear predictor co-efficients, both positive and negative.
// For more details on this see: https://problemkaputt.de/psx-spx.htm#cdromxaaudioadpcmcompression
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int32_t ADPCM_PREDICT_COEF_POS[5] = { 0, 60, 115,  98, 122 };
static constexpr int32_t ADPCM_PREDICT_COEF_NEG[5] = { 0,  0, -52, -55, -60 };

//------------------------------------------------------------------------------------------------------------------------------------------
// How much each nibble + shift combination contributes to the overall sound.
// The 1st dimension is the shift and the 2nd dimension is the nibble value.
// Precomputed to speed up encoding.
//------------------------------------------------------------------------------------------------------------------------------------------
struct ShiftNibbleEncodingTable {
    int16_t values[13][16];
};

static constexpr ShiftNibbleEncodingTable buildShiftNibbleEncodingTable() noexcept {
    ShiftNibbleEncodingTable table = {};

    for (uint16_t shift = 0; shift <= 12; ++shift) {
        for (uint16_t nibble = 0; nibble <= 15; ++nibble) {
            // N.B: the arithmetic right shift here is important!
            table.values[shift][nibble] = ((int16_t)(nibble << 12u)) >> shift;
        }
    }

    return table;
}

constexpr ShiftNibbleEncodingTable SHIFT_NIBBLE_ENC_TABLE = buildShiftNibbleEncodingTable();

//------------------------------------------------------------------------------------------------------------------------------------------
// Do byte swapping for little endian host CPUs.
// The VAG header is stored in big endian format in the file.
//------------------------------------------------------------------------------------------------------------------------------------------
void VagFileHdr::endianCorrect() noexcept {
    if (Endian::isLittle()) {
        fileId = Endian::byteSwap(fileId);
        version = Endian::byteSwap(version);
        _reserved1 = Endian::byteSwap(_reserved1);
        adpcmDataSize = Endian::byteSwap(adpcmDataSize);
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
        (adpcmDataSize > 0) &&
        (sampleRate > 0)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the contents of a .VAG file
//------------------------------------------------------------------------------------------------------------------------------------------
bool readVagFile(
    InputStream& in,
    const size_t fileSize,
    std::vector<std::byte>& adpcmDataOut,
    uint32_t& sampleRate,
    std::string& errorMsgOut
) noexcept {
    sampleRate = {};
    adpcmDataOut.clear();

    bool bReadOk = false;

    try {
        // Read the header, validate and save the sample rate
        VagFileHdr hdr = {};
        in.read(hdr);
        hdr.endianCorrect();

        // These checks SHOULD be done, but some of the PlayStation SDK tools don't seem to populate these fields always correctly.
        // Therefore skip the file id and version checks for the sake of compatibility...
        #if false
            if (hdr.fileId != VAG_FILE_ID)
                throw "File is not a .vag file! Invalid file id!";

            if (hdr.version != VAG_FILE_VERSION)
                throw "The .vag file version is not recognized! The only supported version is '3'.";
        #endif

        // Verify the size in the header file: it must be greater than '0' and be block size aligned
        if (hdr.adpcmDataSize <= 0)
            throw "Invalid size specified in the .vag file header!";

        if (hdr.adpcmDataSize % ADPCM_BLOCK_SIZE != 0)
            throw "Invalid size specified in the .vag file header!";

        // Make sure a sample rate is specified
        if (hdr.sampleRate <= 0)
            throw "Invalid sample rate specified in the .vag file header!";

        sampleRate = hdr.sampleRate;

        // Read the adpcm data for the VAG file.
        //
        // Note: some of the ADPCM data might be implicit (all zeros) since the header can specify more data than what is in the file.
        // Handle this by zeroing the implicit ADPCM data which isn't actually present in the file.
        adpcmDataOut.resize(hdr.adpcmDataSize);
        const int64_t adpcmDataSizeInFile = std::min<int64_t>((int64_t) fileSize - sizeof(VagFileHdr), hdr.adpcmDataSize);

        if (adpcmDataSizeInFile > 0) {
            in.readBytes(adpcmDataOut.data(), (size_t) adpcmDataSizeInFile);
        }

        // All good if we get to here
        bReadOk = true;
    }
    catch (const char* const exceptionMsg) {
        errorMsgOut = "An error occurred while reading the .vag file! It may not be a valid .vag. Error message: ";
        errorMsgOut += exceptionMsg;
    }
    catch (...) {
        errorMsgOut = "An error occurred while reading the .vag file! It may not be a valid .vag.";
    }

    return bReadOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: read a .vag file from a file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool readVagFile(
    const char* const filePath,
    std::vector<std::byte>& adpcmDataOut,
    uint32_t& sampleRate,
    std::string& errorMsgOut
) noexcept {
    // Determine the file size firstly (needed to read the VAG)
    const int64_t fileSize = FileUtils::getFileSize(filePath);

    if (fileSize < 0) {
        errorMsgOut = "Failed to determine the size of VAG format file '";
        errorMsgOut += filePath;
        errorMsgOut += "'!";
        return false;
    }

    // Read the VAG file itself
    bool bReadFileOk = false;

    try {
        FileInputStream in(filePath);
        bReadFileOk = readVagFile(in, (size_t) fileSize, adpcmDataOut, sampleRate, errorMsgOut);

        // If reading failed add the file name as additional context
        if (!bReadFileOk) {
            std::string errorPrefix = "Failed to read VAG format file '";
            errorPrefix += filePath;
            errorPrefix += "'! ";
            errorMsgOut.insert(errorMsgOut.begin(), errorPrefix.begin(), errorPrefix.end());
        }
    } catch (...) {
        errorMsgOut = "Failed to open VAG format file '";
        errorMsgOut += filePath;
        errorMsgOut += "' for reading! Does the file path exist and is it accessible?";
    }

    return bReadFileOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decodes the specified PSX ADPCM data from a .VAG file (or raw ADPCM) to the given buffer.
// Also saves the loop start and end sample indexes if the sample is looped.
// If the sample is NOT looped then these will both be set to zero.
// This code is more or less copied from the Spu implementation that PsyDoom uses.
//------------------------------------------------------------------------------------------------------------------------------------------
void decodePsxAdpcmSamples(
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
        const int32_t filterCoefPos = ADPCM_PREDICT_COEF_POS[adpcmFilter];
        const int32_t filterCoefNeg = ADPCM_PREDICT_COEF_NEG[adpcmFilter];

        // Decode all of the samples in the block
        for (int32_t sampleIdx = 0; sampleIdx < ADPCM_BLOCK_NUM_SAMPLES; sampleIdx++) {
            // Read this samples 4-bit data
            const uint16_t nibble = (sampleIdx % 2 == 0) ?
                ((uint16_t) adpcmBlock[2 + sampleIdx / 2] & 0x0F) >> 0:
                ((uint16_t) adpcmBlock[2 + sampleIdx / 2] & 0xF0) >> 4;

            // The 4-bit sample gets extended to 16-bit by shifting and is sign extended to 32-bit.
            // After that we scale by the sample shift, arithmetically.
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
bool writePsxAdpcmSoundToVagFile(
    OutputStream& out,
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate
) noexcept {
    // The data size must be a multiple of 16 or the PSX ADPCM block size
    if ((adpcmDataSize % ADPCM_BLOCK_SIZE) != 0)
        return false;

    bool bWrittenOk = false;

    try {
        // Makeup the .VAG file header and write to the file
        VagFileHdr vagHdr = {};
        vagHdr.fileId = VagUtils::VAG_FILE_ID;
        vagHdr.version = VagUtils::VAG_FILE_VERSION;
        vagHdr.adpcmDataSize = adpcmDataSize;
        vagHdr.sampleRate = sampleRate;
        vagHdr.endianCorrect();

        out.write(vagHdr);

        // Write the ADPCM data and flush to finish up
        out.writeBytes(pAdpcmData, adpcmDataSize);
        out.flush();
        bWrittenOk = true;
    }
    catch (...) {
        // Ignore...
    }

    return bWrittenOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: write an ADPCM sound to a .vag file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePsxAdpcmSoundToVagFile(
    const char* const filePath,
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate
) noexcept {
    try {
        FileOutputStream out(filePath, false);
        return writePsxAdpcmSoundToVagFile(out, pAdpcmData, adpcmDataSize, sampleRate);
    } catch (...) {
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Encode a PCM sound to PlayStation ADPCM and save to a .vag file
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePcmSoundToVagFile(
    OutputStream& out,
    const int16_t* const pSamples,
    const uint32_t numSamples,
    const uint32_t sampleRate,
    const uint32_t loopStartSampleIdx,
    const uint32_t loopEndSampleIdx
) noexcept {
    std::vector<std::byte> adpcmData;
    encodePcmSoundToPsxAdpcm(pSamples, numSamples, loopStartSampleIdx, loopEndSampleIdx, adpcmData);
    return writePsxAdpcmSoundToVagFile(out, adpcmData.data(), (uint32_t) adpcmData.size(), sampleRate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: write an PCM sound to a .vag file on disk, converting to ADPCM beforehand
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePcmSoundToVagFile(
    const char* const filePath,
    const int16_t* const pSamples,
    const uint32_t numSamples,
    const uint32_t sampleRate,
    const uint32_t loopStartSampleIdx,
    const uint32_t loopEndSampleIdx
) noexcept {
    try {
        FileOutputStream out(filePath, false);
        return writePcmSoundToVagFile(out, pSamples, numSamples, sampleRate, loopStartSampleIdx, loopEndSampleIdx);
    } catch (...) {
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Encode the given sound to PSX adpcm
//------------------------------------------------------------------------------------------------------------------------------------------
void encodePcmSoundToPsxAdpcm(
    const int16_t* const pSamples,
    const uint32_t numSamples,
    const uint32_t loopStartSampleIdx,
    const uint32_t loopEndSampleIdx,
    std::vector<std::byte>& adpcmDataOut
) noexcept {
    // Figure out which blocks we apply these flags for
    uint32_t loopStartBlock = UINT32_MAX;
    uint32_t loopRepeatBlock = UINT32_MAX;
    const bool bIsSoundLooped = (loopStartSampleIdx != loopEndSampleIdx);

    if (bIsSoundLooped) {
        loopStartBlock = (std::min(loopStartSampleIdx, numSamples) + ADPCM_BLOCK_NUM_SAMPLES / 2) / ADPCM_BLOCK_NUM_SAMPLES;
        loopRepeatBlock = (std::min(loopEndSampleIdx, numSamples) + ADPCM_BLOCK_NUM_SAMPLES / 2) / ADPCM_BLOCK_NUM_SAMPLES;

        // Note: the flag means loop AFTER the end of this block, so we have to decrement by 1
        if (loopRepeatBlock > 0) {
            loopRepeatBlock--;
        }
    }

    // Store the previous two encoded samples here
    int16_t prevEncSamples[2] = {};

    // Encode all of the blocks of sound
    const uint32_t numAdpcmBlocks = (numSamples + ADPCM_BLOCK_NUM_SAMPLES - 1) / ADPCM_BLOCK_NUM_SAMPLES;
    adpcmDataOut.clear();
    adpcmDataOut.resize((size_t) numAdpcmBlocks * ADPCM_BLOCK_SIZE);

    for (uint32_t blockIdx = 0; blockIdx < numAdpcmBlocks; ++blockIdx) {
        // Grab all of the samples for this block, zero pad to 28 samples if required (if we are at the end of the sound)
        const bool bIsLastBlock = (blockIdx + 1 >= numAdpcmBlocks);

        int16_t blockSamples[ADPCM_BLOCK_NUM_SAMPLES] = {};

        const uint32_t startSampIdx = blockIdx * ADPCM_BLOCK_NUM_SAMPLES;
        const uint32_t endSampIdx = std::min(startSampIdx + ADPCM_BLOCK_NUM_SAMPLES, numSamples);
        const int32_t numSamplesToCopy = endSampIdx - startSampIdx;

        std::memcpy(blockSamples, pSamples + startSampIdx, numSamplesToCopy * sizeof(int16_t));

        // Encode the ADPCM block
        encodePcmToPsxAdpcmBlock(
            blockSamples,
            prevEncSamples[0],
            prevEncSamples[1],
            ((blockIdx == loopStartBlock) && bIsSoundLooped),
            ((blockIdx == loopRepeatBlock) || bIsLastBlock),
            // The old PlayStation VAG tools set the loop repeat flag for every single sample block except the first, if the sound was looped.
            // I'm replicating the same behavior here...
            ((blockIdx != 0) && bIsSoundLooped),
            adpcmDataOut.data() + (size_t) blockIdx * ADPCM_BLOCK_SIZE,
            prevEncSamples[0],
            prevEncSamples[1]
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Evaluates a particular ADPCM encoding using the specified sample filter and sample shift.
// Returns the encoded nibbles, previous 2 encoded samples and the error of this encoding.
//------------------------------------------------------------------------------------------------------------------------------------------
static void tryPsxAdpcmEncoding(
    const uint32_t sampleFilter,
    const int32_t sampleShift,
    const int16_t inSamples[ADPCM_BLOCK_NUM_SAMPLES],
    const int16_t inPrevSample1,
    const int16_t inPrevSample2,
    uint8_t outNibbles[ADPCM_BLOCK_NUM_SAMPLES],
    int16_t& outPrevSample1,
    int16_t& outPrevSample2,
    uint64_t& outError
) noexcept {
    // Get the prediction filter co-efficients and the correction step based on the sample shift
    const int32_t predictCoefPos = ADPCM_PREDICT_COEF_POS[sampleFilter];
    const int32_t predictCoefNeg = ADPCM_PREDICT_COEF_NEG[sampleFilter];
    const int32_t adjustStep = 1 << std::clamp(12 - sampleShift, 0, 12);

    // Encode the nibbles attempting to correct the error for each sample and compute the error of this encoding as we go
    outError = 0;
    int16_t prevSamples[2] = { inPrevSample1, inPrevSample2 };

    for (uint32_t sampleIdx = 0; sampleIdx < ADPCM_BLOCK_NUM_SAMPLES; ++sampleIdx) {
        // Get the current sample, the prediction according to the filter and the error from the prediction
        const int32_t realSample = inSamples[sampleIdx];
        const int32_t predictedSample = (prevSamples[0] * predictCoefPos + prevSamples[1] * predictCoefNeg + 32) / 64;
        const int32_t predictionError = realSample - predictedSample;

        // Compute how many steps to adjust by to try and fix.
        // Clamp to within range for a 4-bit signed integer: this is our sample nibble.
        const int32_t adjustSteps = std::clamp(predictionError / adjustStep, -8, 7);
        const uint8_t sampleNibble = ((uint8_t) adjustSteps) & 0x0Fu;
        outNibbles[sampleIdx] = sampleNibble;

        // Save the sample we just encoded and shuffle backwards the last previous sample
        const int32_t encodedSampleUnclamped = predictedSample + (int32_t) SHIFT_NIBBLE_ENC_TABLE.values[sampleShift][sampleNibble];
        const int16_t encodedSample = (int16_t) std::clamp<int32_t>(encodedSampleUnclamped, INT16_MIN, INT16_MAX);
        prevSamples[1] = prevSamples[0];
        prevSamples[0] = encodedSample;

        // Update the error of this encoding: penalize heavily overflow
        const uint32_t encodingError = (uint32_t) std::abs(encodedSample - realSample);
        const uint32_t overflowError = (uint32_t) std::abs(encodedSampleUnclamped - encodedSample) * 64;
        outError += (uint64_t) encodingError * encodingError;
        outError += (uint64_t) overflowError * overflowError;
    }

    // Save the output previous samples
    outPrevSample1 = prevSamples[0];
    outPrevSample2 = prevSamples[1];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Encode the given samples in the PlayStation's ADPCM format
//------------------------------------------------------------------------------------------------------------------------------------------
void encodePcmToPsxAdpcmBlock(
    const int16_t samples[ADPCM_BLOCK_NUM_SAMPLES],
    const int16_t prevSample1,
    const int16_t prevSample2,
    const bool bLoopStartFlag,
    const bool bLoopEndFlag,
    const bool bRepeatFlag,
    std::byte adpcmDataOut[ADPCM_BLOCK_SIZE],
    int16_t& prevEncSampleOut1,
    int16_t& prevEncSampleOut2
) noexcept {
    // Try various combinations of ADPCM encoding and sample shift adjust to find the best one
    uint64_t bestError = UINT64_MAX;
    uint32_t bestSampleFilter = 0;
    uint32_t bestSampleShift = 0;
    uint8_t bestSampleNibbles[ADPCM_BLOCK_NUM_SAMPLES] = {};

    for (uint32_t sampleFilter = 0; sampleFilter <= 4; ++sampleFilter) {
        for (int32_t sampleShift = 0; sampleShift <= 12; ++sampleShift) {
            // Evaluate this encoding
            uint8_t sampleNibbles[ADPCM_BLOCK_NUM_SAMPLES];
            int16_t lastEncSample1;
            int16_t lastEncSample2;
            uint64_t error;

            tryPsxAdpcmEncoding(
                sampleFilter,
                sampleShift,
                samples,
                prevSample1,
                prevSample2,
                sampleNibbles,
                lastEncSample1,
                lastEncSample2,
                error
            );

            // Is this a better one? If so then remember it...
            if (error < bestError) {
                bestError = error;
                bestSampleFilter = sampleFilter;
                bestSampleShift = sampleShift;
                std::memcpy(bestSampleNibbles, sampleNibbles, sizeof(sampleNibbles));

                // Save this for the caller, so it knows the last two encoded samples for the best encoding
                prevEncSampleOut1 = lastEncSample1;
                prevEncSampleOut2 = lastEncSample2;
            }
        }
    }

    // Save the sample shift and the prediction filter
    adpcmDataOut[0] = (std::byte)(bestSampleShift | (bestSampleFilter << 4));

    // Save the ADPCM flags and the sample nibbles themselves
    adpcmDataOut[1] = (std::byte)(
        ((bLoopStartFlag) ? ADPCM_FLAG_LOOP_START : 0u) |
        ((bLoopEndFlag) ? ADPCM_FLAG_LOOP_END : 0u) |
        ((bRepeatFlag) ? ADPCM_FLAG_REPEAT : 0u)
    );

    for (uint32_t byteIdx = 0; byteIdx < ADPCM_BLOCK_NUM_SAMPLES / 2; ++byteIdx) {
        adpcmDataOut[2 + byteIdx] = (std::byte)(bestSampleNibbles[byteIdx * 2] | (bestSampleNibbles[byteIdx * 2 + 1] << 4));
    }
}

END_NAMESPACE(VagUtils)
END_NAMESPACE(AudioTools)
