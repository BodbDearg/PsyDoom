//------------------------------------------------------------------------------------------------------------------------------------------
// LcdExtract:
//      Extracts sounds from a PlayStation Doom .LCD file to a series of PlayStation format sound files in .VAG format.
//      Requires the Williams Module file (.WMD) in order to know the sizes of each sound sample.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "AudioUtils.h"
#include "Endian.h"
#include "FileOutputStream.h"
#include "FileUtils.h"
#include "Module.h"
#include "ModuleFileUtils.h"
#include "VagUtils.h"
#include "WavUtils.h"

#include <algorithm>
#include <cmath>
#include <string>

using namespace AudioTools;

// The header for the LCD file as 16-bit words.
// The first word is the number of samples in the .LCD file, the rest of the words are the patch sample index of a sound in the .LCD file.
static uint16_t gLcdHeader[1024];

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
        const uint32_t sampleRate = patchGroup.guessSampleRateForPatchSample(patchSampleIdx);

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

        // Output to a .VAG file
        {
            std::string vagFilePath = outFilePathNoExt + ".VAG";
            bool bWroteVagFileOk = false;
            
            try {
                FileOutputStream out(vagFilePath.c_str(), false);
                bWroteVagFileOk = VagUtils::writePsxAdpcmSoundToVagFile(pCurLcdData, soundBytesToRead, sampleRate, out);
            } catch (...) {
                // Ignore...
            }

            if (!bWroteVagFileOk) {
                std::printf("Failed to create output .VAG file '%s'!\n", vagFilePath.c_str());
            }
        }

        // Output to a .WAV file
        {
            std::string wavFilePath = outFilePathNoExt + ".wav";
            bool bWroteWavFileOk = false;

            try {
                FileOutputStream out(wavFilePath.c_str(), false);
                bWroteWavFileOk = WavUtils::writePsxAdpcmSoundToWavFile(pCurLcdData, soundBytesToRead, sampleRate, out);
            } catch (...) {
                // Ignore...
            }

            if (!bWroteWavFileOk) {
                std::printf("Failed to create output .wav file '%s'!\n", wavFilePath.c_str());
            }
        }

        // Move onto the next sound
        lcdBytesLeft -= soundBytesToRead;
        pCurLcdData += soundBytesToRead;
    }

    return 0;
}
