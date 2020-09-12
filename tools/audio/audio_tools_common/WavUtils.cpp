#include "WavUtils.h"

#include "ByteVecOutputStream.h"
#include "Endian.h"
#include "OutputStream.h"
#include "VagUtils.h"

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
// Write a sound in 16-bit uncompressed PCM format to the specified wave file.
//
// Notes:
//  (1) The sample count specified is the number of individual samples among all channels.
//      It must be a multiple of the number of channels.
//  (2) To not make a looping wave file, set the loop start and end sample to be equal to each other.
//  (3) The sound is always written in little endian format.
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePcmSoundToWavFile(
    const int16_t* const pSamples,
    const uint32_t numSamples,
    const uint16_t numChannels,
    const uint32_t sampleRate,
    const uint32_t loopStartSample,
    const uint32_t loopEndSample,
    OutputStream& out
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
        hdr.chunkSize = (uint32_t) rootChunkData.tell();
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
// Write a sound in the PlayStation's ADPCM format to a .wav file along with any loop points
//------------------------------------------------------------------------------------------------------------------------------------------
bool writePsxAdpcmSoundToWavFile(
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate,
    OutputStream& out
) noexcept {
    // First decode the audio to regular uncompressed 16-bit samples
    std::vector<int16_t> samples;
    uint32_t loopStartSample = {};
    uint32_t loopEndSample = {};
    VagUtils::decodePsxAdpcmSamples(pAdpcmData, adpcmDataSize, samples, loopStartSample, loopEndSample);

    // Write to the specified output file
    return writePcmSoundToWavFile(samples.data(), (uint32_t) samples.size(), 1, sampleRate, loopStartSample, loopEndSample, out);
}

END_NAMESPACE(WavUtils)
END_NAMESPACE(AudioTools)
