#include "Lcd.h"

#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "PsxPatchGroup.h"

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads all the PlayStation ADPCM format sounds in a .LCD file.
// Requires the patch group from the game's module file in order to tell the size of each sample.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Lcd::readFromLcdFile(InputStream& in, const PsxPatchGroup& patchGroup, std::string& errorMsgOut) noexcept {
    try {
        // Read the header for the LCD file which is 2048 bytes exactly or 1024 16-bit words.
        // The first word is the number of samples in the .LCD file, the rest of the words are the patch sample index of a sound in the .LCD file.
        uint16_t lcdHeader[1024];
        in.readArray(lcdHeader, C_ARRAY_SIZE(lcdHeader));

        // Get how many sounds there are in the .LCD
        const uint16_t numLcdSounds = lcdHeader[0];

        if (numLcdSounds > MAX_LCD_FILE_SAMPLES)
            throw "Too many sounds in the .LCD file! File is corrupt!";

        // Extract each of the sounds in the .LCD
        for (uint16_t soundIdx = 0; soundIdx < numLcdSounds; ++soundIdx) {
            // Get what patch sample this is and validate it is in range
            const uint16_t patchSampleIdx = lcdHeader[1 + soundIdx];

            if (patchSampleIdx >= patchGroup.patchSamples.size())
                throw "LCD file references a patch sample index that doesn't exist in the WMD file! .LCD may be corrupt or out of date!";

            // Read the sample data for the sound and save
            const uint32_t sampleSize = patchGroup.patchSamples[patchSampleIdx].size;

            LcdSample& sample = samples.emplace_back();
            sample.patchSampleIdx = patchSampleIdx;
            sample.adpcmData.resize(sampleSize);
            in.readBytes(sample.adpcmData.data(), sampleSize);
        }

        // If we got to here then all is good
        return true;
    }
    catch (const char* const msgStr) {
        errorMsgOut = msgStr;
    }
    catch (...) {
        errorMsgOut = "An IO error occurred while reading the .LCD file! .LCD may be corrupt or out of date versus the WMD file!";
    }

    // Reading failed if we got to here
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: read a .LCD file from a file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool Lcd::readFromLcdFile(const char* const filePath, const PsxPatchGroup& patchGroup, std::string& errorMsgOut) noexcept {
    bool bReadFileOk = false;

    try {
        FileInputStream in(filePath);
        bReadFileOk = readFromLcdFile(in, patchGroup, errorMsgOut);

        // If reading failed add the file name as additional context
        if (!bReadFileOk) {
            std::string errorPrefix = "Failed to read the .LCD file  '";
            errorPrefix += filePath;
            errorPrefix += "'! ";
            errorMsgOut.insert(errorMsgOut.begin(), errorPrefix.begin(), errorPrefix.end());
        }
    } catch (...) {
        errorMsgOut = "Failed to open .LCD file '";
        errorMsgOut += filePath;
        errorMsgOut += "' for reading! Does the file path exist and is it accessible?";
    }

    return bReadFileOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Writes all of the PlayStation ADPCM format sounds to a .LCD file.
// Requires the patch group from the game's module file in order to tell the size of each sample.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Lcd::writeToLcdFile(OutputStream& out, const PsxPatchGroup& patchGroup, std::string& errorMsgOut) const noexcept {
    try {
        // Can't write the LCD file if there are too many sounds
        if (samples.size() > MAX_LCD_FILE_SAMPLES)
            throw "Too many samples to write a .LCD file!";

        // Write the number of samples in the .LCD file and the patch sample indices of each sample
        out.write<uint16_t>((uint16_t) samples.size());

        for (const LcdSample& sample : samples) {
            out.write<uint16_t>(sample.patchSampleIdx);
        }

        // Pad out the rest of 2048 byte header with zero bytes
        out.padAlign(2048, std::byte(0));

        // Write the ADPCM data for all the samples
        for (const LcdSample& sample : samples) {
            // Validate the sample is valid for that is in the module file first, before writing the data
            if (sample.patchSampleIdx >= patchGroup.patchSamples.size())
                throw "Failed to write the .LCD file because one or more of the patch sample indexes are out of range for the .WMD file!";

            if (sample.adpcmData.size() != patchGroup.patchSamples[sample.patchSampleIdx].size)
                throw "Failed to write the .LCD file because the ADPCM data size for one or more samples does not match the sample size specified by the .WMD file!";

            out.writeBytes(sample.adpcmData.data(), sample.adpcmData.size());
        }

        // The .LCD file gets padded out to the next 2,048 bytes on the PSX Doom disc
        out.padAlign(2048, std::byte(0));

        // Successful if we got to here!
        return true;
    }
    catch (const char* const msgStr) {
        errorMsgOut = msgStr;
    }
    catch (...) {
        errorMsgOut = "An IO error occurred while reading the .LCD file! .LCD may be corrupt or out of date versus the WMD file!";
    }

    // Writing failed if we reach here
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: write to a .LCD file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool Lcd::writeToLcdFile(const char* const filePath, const PsxPatchGroup& patchGroup, std::string& errorMsgOut) const noexcept {
    bool bWroteFileOk = false;

    try {
        FileOutputStream out(filePath, false);
        bWroteFileOk = writeToLcdFile(out, patchGroup, errorMsgOut);

        // If reading failed add the file name as additional context
        if (!bWroteFileOk) {
            std::string errorPrefix = "Failed to write to the .LCD file  '";
            errorPrefix += filePath;
            errorPrefix += "'! ";
            errorMsgOut.insert(errorMsgOut.begin(), errorPrefix.begin(), errorPrefix.end());
        }
    } catch (...) {
        errorMsgOut = "Failed to open .LCD file '";
        errorMsgOut += filePath;
        errorMsgOut += "' for writing! Does the file path exist and is it writable?";
    }

    return bWroteFileOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find the lcd sample for the specified patch sample index
//------------------------------------------------------------------------------------------------------------------------------------------
const LcdSample* Lcd::findPatchSample(const uint16_t patchSampleIdx) const noexcept {
    for (const LcdSample& sample : samples) {
        if (sample.patchSampleIdx == patchSampleIdx)
            return &sample;
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find the lcd sample for the specified patch sample index
//------------------------------------------------------------------------------------------------------------------------------------------
LcdSample* Lcd::findPatchSample(const uint16_t patchSampleIdx) noexcept {
    for (LcdSample& sample : samples) {
        if (sample.patchSampleIdx == patchSampleIdx)
            return &sample;
    }

    return nullptr;
}

END_NAMESPACE(AudioTools)
