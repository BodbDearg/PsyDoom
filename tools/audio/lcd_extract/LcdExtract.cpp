//------------------------------------------------------------------------------------------------------------------------------------------
// LcdExtract:
//      Extracts sounds from a PlayStation Doom .LCD file to a series of PlayStation format sound files in .VAG format.
//      Requires the Williams Module file (.WMD) in order to know the sizes of each sound sample.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "FileUtils.h"
#include "Module.h"
#include "ModuleFileUtils.h"
#include "VagUtils.h"

#include <string>

using namespace AudioTools;

// The header for the LCD file as 16-bit words.
// The first word is the number of samples in the .LCD file, the rest of the words are the patch sample index of a sound in the .LCD file.
static uint16_t gLcdHeader[1024];

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
    uint32_t lcdBytesLeft = (uint32_t) lcdFileData.size - (uint32_t) sizeof(gLcdHeader);
    std::byte* pCurLcdData = lcdFileData.bytes.get() + sizeof(gLcdHeader);

    for (uint16_t soundIdx = 0; (soundIdx < numLcdSounds) && (lcdBytesLeft > 0); ++soundIdx) {
        // Get what patch sample this is and validate it is in range
        const uint16_t patchSampleIndex = gLcdHeader[1 + soundIdx];

        if (patchSampleIndex >= module.psxPatchGroup.patchSamples.size()) {
            std::printf("The .LCD file '%s' is corrupt! It references a patch sample (index %u) that does not exist in the .WMD file.\n", lcdFileIn, (unsigned) patchSampleIndex);
            return 1;
        }

        // Get how many sound bytes to read
        const uint32_t patchSampleSize = module.psxPatchGroup.patchSamples[patchSampleIndex].size;
        const uint32_t soundBytesToRead = std::min(patchSampleSize, lcdBytesLeft);

        if (soundBytesToRead < patchSampleSize) {
            std::printf(
                "WARNING: not enough bytes left in the file for patch sample with index '%u'! This sample will be truncated."
                "Needed '%u' bytes for the sample but the .LCD file only has '%u' bytes left!",
                (unsigned) patchSampleIndex,
                (unsigned) patchSampleSize,
                (unsigned) lcdBytesLeft
            );
        }

        // Makeup the output file path
        std::string vagFilePath = outputDir;

        if ((!vagFilePath.empty()) && (vagFilePath.back() != '/') && (vagFilePath.back() != '\\')) {
            vagFilePath += '/';
        }

        vagFilePath += "SAMP";

        {
            char sampleIdxZeroPadded[32];
            std::snprintf(sampleIdxZeroPadded, sizeof(sampleIdxZeroPadded), "%04u", (unsigned) patchSampleIndex);
            vagFilePath += sampleIdxZeroPadded;
        }

        vagFilePath += ".VAG";

        // Makeup the .VAG file header.
        // There's no way of telling what the sample rate, so guess it is 11,025 Hz...
        const uint32_t vagTotalSize = sizeof(VagUtils::VagFileHdr) + soundBytesToRead;

        VagUtils::VagFileHdr vagHdr = {};
        vagHdr.fileId = VagUtils::VAG_FILE_ID;
        vagHdr.version = VagUtils::VAG_FILE_VERSION;
        vagHdr.size = vagTotalSize;
        vagHdr.sampleRate = 11025;
        vagHdr.name[0] = 'S';
        vagHdr.name[1] = 'A';
        vagHdr.name[2] = 'M';
        vagHdr.name[3] = 'P';
        vagHdr.endianCorrect();

        // Makeup a buffer with all of the data to be written to the .VAG file
        std::unique_ptr<std::byte[]> vagData(new std::byte[vagTotalSize]);
        std::memcpy(vagData.get(), &vagHdr, sizeof(VagUtils::VagFileHdr));
        std::memcpy(vagData.get() + sizeof(VagUtils::VagFileHdr), pCurLcdData, soundBytesToRead);

        if (!FileUtils::writeDataToFile(vagFilePath.c_str(), vagData.get(), vagTotalSize, false)) {
            std::printf("Error writing to the .VAG file '%s'!\n", vagFilePath.c_str());
        }

        // Move onto the next sound
        lcdBytesLeft -= soundBytesToRead;
        pCurLcdData += soundBytesToRead;
    }

    return 0;
}
