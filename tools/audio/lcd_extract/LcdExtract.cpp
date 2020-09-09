//------------------------------------------------------------------------------------------------------------------------------------------
// LcdExtract:
//      Extracts sounds from a PlayStation Doom .LCD file to a series of PlayStation format sound files in .VAG format.
//      Requires the Williams Module file (.WMD) in order to know the sizes of each sound sample.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "AudioUtils.h"
#include "Endian.h"
#include "FileUtils.h"
#include "Module.h"
#include "ModuleFileUtils.h"
#include "VagUtils.h"

#include <algorithm>
#include <cmath>
#include <string>

using namespace AudioTools;

// The header for the LCD file as 16-bit words.
// The first word is the number of samples in the .LCD file, the rest of the words are the patch sample index of a sound in the .LCD file.
static uint16_t gLcdHeader[1024];

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to determine the sample rate that a given patch sample is encoded in.
// There's no actual information for this in either the .LCD file or the .WMD file, however the .WMD does define a 'base note'.
//
// The 'base note' is the note at which the sample plays back at 44,100 Hz, and in the case of SFX I'm using the distance between that and
// the note that PSX Doom uses to play back all SFX at (A#3/Bb3, or note '58') in order to figure out the sample rate. For music it appears
// that the note '48.0' might be a close approximation for what a base note might be, so I'm using that in the music case.
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t getPatchSampleRate(const uint32_t patchSampleIdx, const PsxPatchGroup& patchGroup) noexcept {
    // Look for a voice using this sample to get the 'base note' info and hence determine the 
    for (const PsxPatchVoice& voice : patchGroup.patchVoices) {
        // Ignore if this voice doesn't use this sample
        if (voice.sampleIdx != patchSampleIdx)
            continue;

        // Is this note SFX or a music note?
        // In the PSX Doom .WMD all SFX have a priority of '100' whereas music instruments have a priority of '128'.
        const bool bIsSfxSample = (voice.priority < 128);

        // Use the distance to the game's base note to figure out the sample rate
        constexpr double PSX_DOOM_SFX_BASE_NOTE = 58.0;
        constexpr double PSX_DOOM_MUS_BASE_NOTE = 48.0;
        constexpr double PSX_MAX_SAMPLE_RATE = 176400.0;

        const double gameBaseNote = (bIsSfxSample) ? PSX_DOOM_SFX_BASE_NOTE : PSX_DOOM_MUS_BASE_NOTE;
        const double voiceBaseNote = (double) voice.baseNote + (double) voice.baseNoteFrac / 128.0;
        const double sampleRate = AudioUtils::getNoteSampleRate(voiceBaseNote, 44100.0, gameBaseNote);
        const double sampleRateRounded = std::clamp(std::round(sampleRate), 1.0,  PSX_MAX_SAMPLE_RATE);

        return (uint32_t) sampleRateRounded;
    }
    
    // If we don't find a patch voice using the sample, then fallback to assuming it's at 11,050 Hz.
    // This is the sample rate used by a lot of PSX Doom sounds:
    return 11050;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the given LCD ADPCM data to a .VAG file
//------------------------------------------------------------------------------------------------------------------------------------------
static void writeSoundToVag(
    const std::string& filePathNoExt,
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate
) noexcept {
    // Makeup the .VAG file header
    const uint32_t vagTotalSize = sizeof(VagUtils::VagFileHdr) + adpcmDataSize;

    VagUtils::VagFileHdr vagHdr = {};
    vagHdr.fileId = VagUtils::VAG_FILE_ID;
    vagHdr.version = VagUtils::VAG_FILE_VERSION;
    vagHdr.size = vagTotalSize;
    vagHdr.sampleRate = sampleRate;
    vagHdr.name[0] = 'S';
    vagHdr.name[1] = 'A';
    vagHdr.name[2] = 'M';
    vagHdr.name[3] = 'P';
    vagHdr.endianCorrect();

    // Makeup a buffer with all of the data to be written to the .VAG file
    std::unique_ptr<std::byte[]> vagData(new std::byte[vagTotalSize]);
    std::memcpy(vagData.get(), &vagHdr, sizeof(VagUtils::VagFileHdr));
    std::memcpy(vagData.get() + sizeof(VagUtils::VagFileHdr), pAdpcmData, adpcmDataSize);

    // Write the .VAG file
    const std::string vagFilePath = filePathNoExt + ".VAG";

    if (!FileUtils::writeDataToFile(vagFilePath.c_str(), vagData.get(), vagTotalSize, false)) {
        std::printf("Error writing to the .VAG file '%s'!\n", vagFilePath.c_str());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the given LCD ADPCM data to a standard .WAV file in uncompressed mode.
// For more information, see: https://sites.google.com/site/musicgapi/technical-documents/wav-file-format
//------------------------------------------------------------------------------------------------------------------------------------------
static void writeSoundToWav(
    const std::string& filePathNoExt,
    const std::byte* const pAdpcmData,
    const uint32_t adpcmDataSize,
    const uint32_t sampleRate
) noexcept {
    // This code needs to be fixed up for big endian mode...
    static_assert(Endian::isLittle(), "writeSoundToWav only works for little endian currently!");

    // First decode the audio to regular uncompressed 16-bit samples
    std::vector<int16_t> samples;
    uint32_t loopStartSampleIdx = {};
    uint32_t loopEndSampleIdx = {};
    VagUtils::decodeAdpcmSamples(pAdpcmData, adpcmDataSize, samples, loopStartSampleIdx, loopEndSampleIdx);

    // Format for a sampler loop
    struct SamplerLoop {
        uint32_t cuePointId;    // Unique id for this loop cue point, so things like labels can be associated with it
        uint32_t type;          // 0 = normal loop forward
        uint32_t startSamp;     // Start of loop (in samples)
        uint32_t endSamp;       // End of loop (in samples)
        uint32_t fraction;      // Fraction of a sample at which to loop - 0x80000000 = 0.5 of a sample
        uint32_t playCount;     // 0 = play forever
    };

    static_assert(sizeof(SamplerLoop) == 24);

    // Format for the sampler chunk header + 1 baked in loop
    struct SamplerChunk {
        uint32_t chunkId;           // Should be 'smpl' in ASCII
        uint32_t dataSize;          // 36 + (Num Sample Loops * 24) + Sampler Data
        uint32_t manufacturer;      // MIDI registered manufacturer for sampler
        uint32_t product;           // MIDI registered product for sampler
        uint32_t samplePeriod;      // Duration of time that passes during 1 sample in nanoseconds
        uint32_t baseNote;          // MIDI note at which the sample is played back at its original sample rate
        uint32_t baseNoteFrac;      // Fraction of a note for base note: 0x80000000 = 0.5 of a semitone
        uint32_t smpteFormat;       // Timing related stuff...
        uint32_t smpteOffset;       // Timing related stuff...
        uint32_t numSampleLoops;    // Number of sample loops
        uint32_t samplerDataSize;   // Extra sampler data

        // The baked in sampler loop
        SamplerLoop samplerLoops[1];
    };

    static_assert(sizeof(SamplerChunk) == 68);

    // Header for the format chunk in uncompressed mode
    struct FmtChunkHdr {
        uint32_t chunkId;           // Should be 'fmt ' in ASCII
        uint32_t dataSize;          // 16 + extra format bytes
        uint16_t compressionCode;   // 1 = Uncompressed PCM
        uint16_t numChannels;       // 2 = Stereo,  1 = Mono
        uint32_t sampleRate;        // In Hz
        uint32_t avgBytesPerSec;    // Estimated bytes per second for streaming
        uint16_t blockAlign;        // Bytes per sample slice: (bits per sample / 8) * numChannels
        uint16_t bitsPerSample;     // 16, 8 etc.
    };

    static_assert(sizeof(FmtChunkHdr) == 24);

    // Open up an output stream for the .WAV file
    const std::string wavFilePath = filePathNoExt + ".WAV";
    FILE* const pFile = std::fopen(wavFilePath.c_str(), "wb");

    if (!pFile) {
        std::printf("Error writing to the .WAV file '%s'!\n", wavFilePath.c_str());
        return;
    }
    
    // Figure out the sizes for everything
    constexpr uint32_t fmtChunkPayloadSize = sizeof(FmtChunkHdr) - 8;
    constexpr uint32_t samplerChunkPlayloadSize = sizeof(SamplerChunk) - 8;
    const uint32_t dataChunkPayloadSize = (uint32_t)(samples.size() * sizeof(int16_t));
    const uint32_t riffChunkPayloadSize = fmtChunkPayloadSize + samplerChunkPlayloadSize + dataChunkPayloadSize + (3 * 8) + 4;  // 3x8 byte chunk headers + 4 byte 'WAVE' id

    // Write the RIFF chunk header and the WAVE chunk header
    std::fwrite("RIFF", 4, 1, pFile);
    std::fwrite(&riffChunkPayloadSize, sizeof(uint32_t), 1, pFile);
    std::fwrite("WAVE", 4, 1, pFile);

    // Write the format chunk
    FmtChunkHdr fmtHdr = {};
    fmtHdr.chunkId = 0x20746D66;            // fmt 
    fmtHdr.dataSize = fmtChunkPayloadSize;
    fmtHdr.compressionCode = 1;             // Uncompressed PCM
    fmtHdr.numChannels = 1;
    fmtHdr.sampleRate = sampleRate;
    fmtHdr.avgBytesPerSec = sampleRate * sizeof(int16_t);
    fmtHdr.blockAlign = sizeof(int16_t);
    fmtHdr.bitsPerSample = 16;
    std::fwrite(&fmtHdr, sizeof(fmtHdr), 1, pFile);

    // Write the sampler chunk header
    SamplerChunk sampChunk = {};
    sampChunk.chunkId = 0x6C706D73u;        // smpl
    sampChunk.dataSize = samplerChunkPlayloadSize;
    sampChunk.samplePeriod = 1000000000 / sampleRate;
    sampChunk.baseNote = 60;                // Play back at current sample rate at C5

    if (loopStartSampleIdx != loopEndSampleIdx) {
        sampChunk.numSampleLoops = 1;
        sampChunk.samplerLoops[0].cuePointId = 100;
        sampChunk.samplerLoops[0].startSamp = loopStartSampleIdx;
        sampChunk.samplerLoops[0].endSamp = (loopEndSampleIdx - 1);     // According to what I've read, the last sample is played so we need to '-1' here
    }

    std::fwrite(&sampChunk, sizeof(sampChunk), 1, pFile);

    // Write the header for the data chunk and the sample data itself
    std::fwrite("data", 4, 1, pFile);
    std::fwrite(&dataChunkPayloadSize, sizeof(uint32_t), 1, pFile);
    std::fwrite(samples.data(), sizeof(int16_t) * samples.size(), 1, pFile);

    // Close up the file to finish up
    std::fclose(pFile);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Core LCD extraction logic
//------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, const char* const argv[]) noexcept {
    // Can accept 3 or 4 arguments
    if ((argc != 3) && (argc != 4)) {
        std::printf("Usage: %s <INPUT_LCD_FILE_PATH> <INPUT_WMD_FILE_PATH> [OUTPUT_DIR]\n", argv[0]);
        return 1;
    }
    
    const char* const lcdFileIn = argv[1];
    const char* const wmdFileIn = argv[2];
    const char* const outputDir = (argc >= 4) ? argv[3] : "";
    
    // Read the input .WMD file
    Module module = {};
    std::string errorMsg;

    if (!ModuleFileUtils::readWmdFile(wmdFileIn, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return 1;
    }

    // Read the entire .LCD file into RAM
    const FileData lcdFileData = FileUtils::getContentsOfFile(lcdFileIn);

    if (!lcdFileData.bytes) {
        std::printf("Failed to read the .LCD file '%s'! It may be corrupt.\n", lcdFileIn);
        return 1;
    }

    // Copy the .LCD header words
    if (lcdFileData.size < 2048) {
        std::printf("The .LCD file '%s' is too small in size and is corrupt!\n", lcdFileIn);
        return 1;
    }

    std::memcpy(gLcdHeader, lcdFileData.bytes.get(), sizeof(gLcdHeader));

    // Get how many sounds there are in the .LCD
    const uint16_t numLcdSounds = gLcdHeader[0];

    if (numLcdSounds > 1023) {
        std::printf("The .LCD file '%s' is corrupt! It appears to contain too many samples ('%u').\n", lcdFileIn, (unsigned) numLcdSounds);
        return 1;
    }

    // Extract each of the sounds
    const PsxPatchGroup& patchGroup = module.psxPatchGroup;

    uint32_t lcdBytesLeft = (uint32_t) lcdFileData.size - (uint32_t) sizeof(gLcdHeader);
    std::byte* pCurLcdData = lcdFileData.bytes.get() + sizeof(gLcdHeader);

    for (uint16_t soundIdx = 0; (soundIdx < numLcdSounds) && (lcdBytesLeft > 0); ++soundIdx) {
        // Get what patch sample this is and validate it is in range
        const uint16_t patchSampleIdx = gLcdHeader[1 + soundIdx];

        if (patchSampleIdx >= patchGroup.patchSamples.size()) {
            std::printf("The .LCD file '%s' is corrupt! It references a patch sample (index %u) that does not exist in the .WMD file.\n", lcdFileIn, (unsigned) patchSampleIdx);
            return 1;
        }

        // Try to guess what sample rate it is at
        const uint32_t sampleRate = getPatchSampleRate(patchSampleIdx, patchGroup);

        // Get how many sound bytes to read
        const uint32_t patchSampleSize = patchGroup.patchSamples[patchSampleIdx].size;
        const uint32_t soundBytesToRead = std::min(patchSampleSize, lcdBytesLeft);

        if (soundBytesToRead < patchSampleSize) {
            std::printf(
                "WARNING: not enough bytes left in the file for patch sample with index '%u'! This sample will be truncated."
                "Needed '%u' bytes for the sample but the .LCD file only has '%u' bytes left!",
                (unsigned) patchSampleIdx,
                (unsigned) patchSampleSize,
                (unsigned) lcdBytesLeft
            );
        }

        // Makeup the output file path without an extension
        std::string outFilePathNoExt = outputDir;

        if ((!outFilePathNoExt.empty()) && (outFilePathNoExt.back() != '/') && (outFilePathNoExt.back() != '\\')) {
            outFilePathNoExt += '/';
        }

        outFilePathNoExt += "SAMP";

        {
            char sampleIdxZeroPadded[32];
            std::snprintf(sampleIdxZeroPadded, sizeof(sampleIdxZeroPadded), "%04u", (unsigned) patchSampleIdx);
            outFilePathNoExt += sampleIdxZeroPadded;
        }

        // Output to a .VAG file and a .WAV file
        writeSoundToVag(outFilePathNoExt, pCurLcdData, soundBytesToRead, sampleRate);
        writeSoundToWav(outFilePathNoExt, pCurLcdData, soundBytesToRead, sampleRate);

        // Move onto the next sound
        lcdBytesLeft -= soundBytesToRead;
        pCurLcdData += soundBytesToRead;
    }

    return 0;
}
