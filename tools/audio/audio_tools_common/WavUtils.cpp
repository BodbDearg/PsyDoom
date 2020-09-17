#include "WavUtils.h"

#include "ByteInputStream.h"
#include "ByteVecOutputStream.h"
#include "Endian.h"
#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "VagUtils.h"

#include <memory>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(WavUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Byte swapping various data structures and validating
//------------------------------------------------------------------------------------------------------------------------------------------
void WavRootChunkHdr::endianCorrect() noexcept {
    fileId = Endian::hostToLittle(fileId);
    riffTypeId = Endian::hostToLittle(riffTypeId);
    chunkSize = Endian::hostToLittle(chunkSize);
}

bool WavRootChunkHdr::validate() noexcept {
    return ((fileId == WavFileId::RIFF) && (riffTypeId == WavFileId::WAVE));
}

void WavChunkHdr::endianCorrect() noexcept {
    chunkId = Endian::hostToLittle(chunkId);
    chunkSize = Endian::hostToLittle(chunkSize);
}

void FmtChunkHdr_Uncompressed::endianCorrect() noexcept {
    format = Endian::hostToLittle(format);
    numChannels = Endian::hostToLittle(numChannels);
    sampleRate = Endian::hostToLittle(sampleRate);
    avgBytesPerSec = Endian::hostToLittle(avgBytesPerSec);
    blockAlign = Endian::hostToLittle(blockAlign);
    bitsPerSample = Endian::hostToLittle(bitsPerSample);
}

bool FmtChunkHdr_Uncompressed::validate() noexcept {
    // Only supporting this type of .wav file for the purposes of these tools
    return (
        (format == WavFormat::PCM) &&
        (numChannels >= 1) &&
        (sampleRate >= 1) &&
        (bitsPerSample == 16)
    );
}

void SamplerChunkHdr::endianCorrect() noexcept {
    manufacturer = Endian::hostToLittle(manufacturer);
    product = Endian::hostToLittle(product);
    samplePeriod = Endian::hostToLittle(samplePeriod);
    baseNote = Endian::hostToLittle(baseNote);
    baseNoteFrac = Endian::hostToLittle(baseNoteFrac);
    smpteFormat = Endian::hostToLittle(smpteFormat);
    smpteOffset = Endian::hostToLittle(smpteOffset);
    numSampleLoops = Endian::hostToLittle(numSampleLoops);
    samplerDataSize = Endian::hostToLittle(samplerDataSize);
}

void SamplerLoop::endianCorrect() noexcept {
    cuePointId = Endian::hostToLittle(cuePointId);
    loopMode = Endian::hostToLittle(loopMode);
    startSamp = Endian::hostToLittle(startSamp);
    endSamp = Endian::hostToLittle(endSamp);
    fraction = Endian::hostToLittle(fraction);
    playCount = Endian::hostToLittle(playCount);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a .wav file from the specified stream.
// Only 16-bit uncompressed PCM wave files can be read.
//------------------------------------------------------------------------------------------------------------------------------------------
bool readWavFile(
    InputStream& in,
    std::vector<int16_t>& samplesOut,
    uint32_t& numChannelsOut,
    uint32_t& sampleRateOut,
    uint32_t& loopStartSample,
    uint32_t& loopEndSample,
    std::string& errorMsgOut
) noexcept {
    // Clear output first
    samplesOut.clear();
    sampleRateOut = 0;
    numChannelsOut = 0;
    loopStartSample = 0;
    loopEndSample = 0;

    bool bReadOk = false;

    try {
        // Read the header for .wav root chunk
        WavRootChunkHdr rootHdr = {};
        in.read(rootHdr);
        rootHdr.endianCorrect();

        // Validate the header
        if (!rootHdr.validate())
            throw "File is not a valid WAV file!";

        // Read the entire root chunk
        const uint32_t rootChunkSize = rootHdr.chunkSize - sizeof(uint32_t);    // Note: the 4 bytes for 'riffTypeId' are included in the count, and we already read those... 
        std::unique_ptr<std::byte[]> rootData(new std::byte[rootChunkSize]);
        in.readBytes(rootData.get(), rootChunkSize);

        // Run through these bytes and try to find the 'fmt' chunk, 'smplr' chunk (if there) and the 'data' chunk.
        // Skip over any unknown chunks:
        uint32_t fmtChunkOffset = 0;
        uint32_t fmtChunkSize = 0;

        uint32_t smplChunkOffset = 0;
        uint32_t smplChunkSize = 0;

        uint32_t dataChunkOffset = 0;
        uint32_t dataChunkSize = 0;

        {
            ByteInputStream rootDataIn(rootData.get(), rootChunkSize);

            while (rootDataIn.tell() < rootChunkSize) {
                // Get the header for this chunk
                WavChunkHdr chunkHdr = {};
                rootDataIn.read(chunkHdr);
                chunkHdr.endianCorrect();

                // What chunk is this?
                if (chunkHdr.chunkId == WavFileId::fmt) {
                    fmtChunkOffset = (uint32_t) rootDataIn.tell();
                    fmtChunkSize = chunkHdr.chunkSize;
                } else if (chunkHdr.chunkId == WavFileId::smpl) {
                    smplChunkOffset = (uint32_t) rootDataIn.tell();
                    smplChunkSize = chunkHdr.chunkSize;
                } else if (chunkHdr.chunkId == WavFileId::data) {
                    dataChunkOffset = (uint32_t) rootDataIn.tell();
                    dataChunkSize = chunkHdr.chunkSize;
                }

                // Skip over the payload for the chunk
                rootDataIn.skipBytes(chunkHdr.chunkSize);
            }
        }

        // Okay read the 'fmt' chunk to figure out what format the wave file is in.
        // Once we have this, validate that the format is supported:
        FmtChunkHdr_Uncompressed fmtHdr = {};

        {
            ByteInputStream fmtChunkIn(rootData.get() + fmtChunkOffset, fmtChunkSize);
            fmtChunkIn.read(fmtHdr);
            fmtHdr.endianCorrect();
        }

        if (!fmtHdr.validate())
            throw "WAV file is not in a supporte format! Only 16-bit uncompressed PCM wave files can be read.";

        sampleRateOut = fmtHdr.sampleRate;
        numChannelsOut = fmtHdr.numChannels;

        // If there is a 'smpl' chunk then read that to try and find loop points
        if (smplChunkSize > 0) {
            // Read the header for the chunk
            SamplerChunkHdr smplHdr = {};
            ByteInputStream smplChunkIn(rootData.get() + smplChunkOffset, smplChunkSize);
            smplChunkIn.read(smplHdr);
            smplHdr.endianCorrect();

            // Are there any loop points defined?
            if (smplHdr.numSampleLoops > 0) {
                // Just read the first one
                SamplerLoop loopPt = {};
                smplChunkIn.read(loopPt);
                loopPt.endianCorrect();

                loopStartSample = loopPt.startSamp;
                loopEndSample = loopPt.endSamp + 1;     // Last sample in the 'SamplerLoop' is PLAYED. Our 'end' is an exclusive end rather than an inclusive one.
            }
        }

        // Finally read the sample data for the sound
        if (dataChunkSize > 0) {
            ByteInputStream dataChunkIn(rootData.get() + dataChunkOffset, dataChunkSize);
            samplesOut.resize(dataChunkSize / sizeof(int16_t));
            dataChunkIn.readArray(samplesOut.data(), samplesOut.size());
        }

        // All was good if we got to here
        bReadOk = true;
    }
    catch (const char* const exceptionMsg) {
        errorMsgOut = "An error occurred while reading the .wav file! It may not be a valid or in a supported format. Error message: ";
        errorMsgOut += exceptionMsg;
    }
    catch (...) {
        errorMsgOut = "An error occurred while reading the .wav file! It may not be a valid or in a supported format.";
    }

    return bReadOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: read a .wav file from a file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool readWavFile(
    const char* const filePath,
    std::vector<int16_t>& samplesOut,
    uint32_t& numChannelsOut,
    uint32_t& sampleRateOut,
    uint32_t& loopStartSample,
    uint32_t& loopEndSample,
    std::string& errorMsgOut
) noexcept {
    bool bReadFileOk = false;

    try {
        FileInputStream in(filePath);
        bReadFileOk = readWavFile(in, samplesOut, numChannelsOut, sampleRateOut, loopStartSample, loopEndSample, errorMsgOut);

        // If reading failed add the file name as additional context
        if (!bReadFileOk) {
            std::string errorPrefix = "Failed to read .wav file '";
            errorPrefix += filePath;
            errorPrefix += "'! ";
            errorMsgOut.insert(errorMsgOut.begin(), errorPrefix.begin(), errorPrefix.end());
        }
    } catch (...) {
        errorMsgOut = "Failed to open .wav file '";
        errorMsgOut += filePath;
        errorMsgOut += "' for reading! Does the file path exist and is it accessible?";
    }

    return bReadFileOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a sound in 16-bit uncompressed PCM format to the specified wave file.
//
// Notes:
//  (1) The sample count specified is the number of individual samples among all channels.
//      It must be a multiple of the number of channels.
//  (2) To not make a looping wave file, set the loop start and end sample to be equal to each other.
//  (3) The sound is always written in little endian format.
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePcmSoundToWavFile(
    OutputStream& out,
    const int16_t* const pSamples,
    const uint32_t numSamples,
    const uint16_t numChannels,
    const uint32_t sampleRate,
    const uint32_t loopStartSample,
    const uint32_t loopEndSample
) noexcept {
    // Validate input
    const bool bInvalidRequest = (
        (pSamples == nullptr) ||
        (numChannels == 0) ||
        (numSamples == 0) ||
        ((numSamples % numChannels) != 0) ||
        (sampleRate == 0)
    );

    if (bInvalidRequest)
        return false;

    // Do endian correction of the sample data if needed, always write as little endian
    std::vector<int16_t> byteSwappedSamples;
    
    if constexpr (Endian::isBig()) {
        byteSwappedSamples.reserve(numSamples);

        for (uint32_t i = 0; i < numSamples; ++i) {
            byteSwappedSamples.push_back(Endian::hostToLittle(pSamples[i]));
        }
    }

    const int16_t* const pEndianCorrectSamples = (Endian::isLittle()) ? pSamples : byteSwappedSamples.data();

    // Makeup a buffer that we will write all of the child chunks to
    ByteVecOutputStream rootChunkData;
    rootChunkData.getBytes().reserve(numSamples * sizeof(int16_t) + 1024 * 64);

    // Write the 'fmt' chunk first
    {
        WavChunkHdr chunkHdr = {};
        chunkHdr.chunkId = WavFileId::fmt;
        chunkHdr.chunkSize = sizeof(FmtChunkHdr_Uncompressed);
        chunkHdr.endianCorrect();

        FmtChunkHdr_Uncompressed fmtHdr = {};
        fmtHdr.format = WavFormat::PCM;
        fmtHdr.numChannels = numChannels;
        fmtHdr.sampleRate = sampleRate;
        fmtHdr.avgBytesPerSec = sampleRate * numChannels * (uint32_t) sizeof(int16_t);
        fmtHdr.blockAlign = sizeof(int16_t) * numChannels;
        fmtHdr.bitsPerSample = 16;
        fmtHdr.endianCorrect();

        rootChunkData.write(chunkHdr);
        rootChunkData.write(fmtHdr);
        rootChunkData.padAlign(2);
    }

    // Only write the sampler chunk if the file is looped
    if (loopStartSample != loopEndSample) {
        WavChunkHdr chunkHdr = {};
        chunkHdr.chunkId = WavFileId::smpl;
        chunkHdr.chunkSize = sizeof(SamplerChunkHdr) + sizeof(SamplerLoop);
        chunkHdr.endianCorrect();

        SamplerChunkHdr smpHdr = {};
        smpHdr.samplePeriod = 1000000000 / sampleRate;
        smpHdr.baseNote = 60;   // Play back at current sample rate at note 'C4' or 'Middle C'
        smpHdr.numSampleLoops = 1;
        smpHdr.endianCorrect();

        SamplerLoop smpLoop = {};
        smpLoop.cuePointId = 101;
        smpLoop.startSamp = loopStartSample;
        smpLoop.endSamp = loopEndSample - 1;    // According to what I've read, the last sample is played so we need to '-1' here
        smpLoop.endianCorrect();

        rootChunkData.write(chunkHdr);
        rootChunkData.write(smpHdr);
        rootChunkData.write(smpLoop);
        rootChunkData.padAlign(2);
    }

    // Write the wave data itself
    {
        WavChunkHdr chunkHdr = {};
        chunkHdr.chunkId = WavFileId::data;
        chunkHdr.chunkSize = numSamples * (uint32_t) sizeof(int16_t);
        chunkHdr.endianCorrect();

        rootChunkData.write(chunkHdr);
        rootChunkData.writeBytes(pEndianCorrectSamples, chunkHdr.chunkSize);
        rootChunkData.padAlign(2);
    }

    // Actually creating the .wav file
    bool bWroteWavOk = false;

    try {
        // Make up the root wave file header and write
        WavRootChunkHdr hdr = {};
        hdr.fileId = WavFileId::RIFF;
        hdr.riffTypeId = WavFileId::WAVE;
        hdr.chunkSize = (uint32_t) rootChunkData.tell() + sizeof(uint32_t);     // The 'riffTypeId' field must be included in the count also
        hdr.endianCorrect();

        out.write(hdr);

        // Write all of the data for the root chunk and flush to finish up
        out.writeBytes(rootChunkData.getBytes().data(), rootChunkData.tell());
        out.flush();
        bWroteWavOk = true;
    } catch (...) {
        // Ignore...
    }

    return bWroteWavOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: write to a .wav file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePcmSoundToWavFile(
    const char* const filePath,
    const int16_t* const pSamples,
    const uint32_t numSamples,
    const uint16_t numChannels,
    const uint32_t sampleRate,
    const uint32_t loopStartSample,
    const uint32_t loopEndSample
) noexcept {
    try {
        FileOutputStream out(filePath, false);
        return writePcmSoundToWavFile(out, pSamples, numSamples, numChannels, sampleRate, loopStartSample, loopEndSample);
    } catch (...) {
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a sound in the PlayStation's ADPCM format to a .wav file along with any loop points
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePsxAdpcmSoundToWavFile(
    OutputStream& out,
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate
) noexcept {
    // First decode the audio to regular uncompressed 16-bit samples
    std::vector<int16_t> samples;
    uint32_t loopStartSample = {};
    uint32_t loopEndSample = {};
    VagUtils::decodePsxAdpcmSamples(pAdpcmData, adpcmDataSize, samples, loopStartSample, loopEndSample);

    // Write to the specified output file
    return writePcmSoundToWavFile(out, samples.data(), (uint32_t) samples.size(), 1, sampleRate, loopStartSample, loopEndSample);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: write a PSX format ADPCM sound to a .wav file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePsxAdpcmSoundToWavFile(
    const char* const filePath,
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate
) noexcept {
    try {
        FileOutputStream out(filePath, false);
        return writePsxAdpcmSoundToWavFile(out, pAdpcmData, adpcmDataSize, sampleRate);
    } catch (...) {
        return false;
    }
}

END_NAMESPACE(WavUtils)
END_NAMESPACE(AudioTools)
