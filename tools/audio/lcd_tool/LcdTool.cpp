//------------------------------------------------------------------------------------------------------------------------------------------
// LcdTool:
//      Dump, list, create or append to a .LCD samples file
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Lcd.h"
#include "Module.h"
#include "ModuleFileUtils.h"
#include "VagUtils.h"
#include "WavUtils.h"

#include <algorithm>

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// The module and lcd file we are working with
//------------------------------------------------------------------------------------------------------------------------------------------
static Module   gModule;
static Lcd      gLcd;

//------------------------------------------------------------------------------------------------------------------------------------------
// An input patch sample index and associated sound file name
//------------------------------------------------------------------------------------------------------------------------------------------
struct PatchSampleFile {
    uint16_t        patchSampleIdx;
    const char*     filePath;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Help/usage printing
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* const HELP_STR = 
R"(Usage: LcdTool <LCD FILE PATH> <BINARY|JSON WMD FILE PATH> <COMMAND-SWITCH> [COMMAND ARGS]

Command switches and arguments:

    -list
        List the current contents of the .LCD file in the order that it is stored.
        Example:
        LcdTool BLAH.LCD DOOMSND.WMD -list

    -dump [OUTPUT DIR PATH]
        Dump all the sounds in the .LCD file to both .WAV and PlayStation .VAG format.
        Notes:
            (1) .WAV files will be output with loop point info via the 'smpl' chunk.
            (2) The output directory is optional: the current working directory will be used if not specified.
        Example:
            LcdTool BLAH.LCD DOOMSND.WMD -dump MyDestDir

    -create [<PATCH SAMPLE INDEX><SOUND FILE PATH>]...
        Create a completely new .LCD file with the given list of patch sample indexes and corresponding sound files.
        Notes:
            (1) The sound files can be in .WAV or .VAG format. Any .WAV files must be 16-bit, uncompressed PCM and mono.
            (2) Loop point info from .WAV files is extracted from the 'smpl' chunk. Only the 1st loop point is used.
            (3) Sounds are padded to multiples of 28 samples. Ideally supply audio already in this format.
            (4) Loop points are rounded to the nearest 28 samples as required by the hardware. Ideally do this yourself.
        Example:
            LcdTool BLAH.LCD DOOMSND.WMD -create 1 SOUND1.WAV 2 SOUND2.VAG

    -append [<PATCH SAMPLE INDEX><SOUND FILE PATH>]...
        Same as the '-create' command except an existing LCD file is appended to.
        Notes:
            (1) If a specified patch sample already exists in the .LCD file, then it will be replaced.
            (2) The sound files can be in .WAV or .VAG format. Any .WAV files must be 16-bit, uncompressed PCM and mono.
            (3) Loop point info from .WAV files is extracted from the 'smpl' chunk. Only the 1st loop point is used.
            (4) Sounds are padded to multiples of 28 samples. Ideally supply audio already in this format.
            (5) Loop points are rounded to the nearest 28 samples as required by the hardware. Ideally do this yourself.
        Example:
            LcdTool BLAH.LCD DOOMSND.WMD -append 101 SOUND101.WAV 102 SOUND102.VAG
)";

static void printHelp() noexcept {
    std::printf("%s\n", HELP_STR);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a module file in .json or binary .wmd format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readModuleFile(const char* const filePath, Module& module, std::string& errorMsg) noexcept {
    // Uppercase the filename for case insensitivity
    std::string filePathUpper = filePath;
    std::transform(
        filePathUpper.begin(),
        filePathUpper.end(),
        filePathUpper.begin(),
        [](const char c) noexcept { return (char) ::toupper(c); }
    );

    // Is it probably a JSON file?
    const bool bGuessIsJsonFile = ((filePathUpper.length() >= 5) && (filePathUpper.rfind(".JSON") == filePathUpper.length() - 5));

    // Do the reading of the module file, starting with the guessed format
    const auto readAsWmd = [&]() noexcept {
        errorMsg.clear();
        return ModuleFileUtils::readWmdFile(filePath, module, errorMsg);
    };

    const auto readAsJson = [&]() noexcept {
        errorMsg.clear();
        return ModuleFileUtils::readJsonFile(filePath, module, errorMsg);
    };

    if (bGuessIsJsonFile) {
        if (!readAsJson()) {
            return readAsWmd();
        }
    } else {
        if (!readAsWmd()) {
            return readAsJson();
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the input module file in .json or .wmd (binary) format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readInputWmdFile(const char* wmdFilePath) noexcept {
    gModule = {};
    std::string errorMsg;

    if (!readModuleFile(wmdFilePath, gModule, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the input .lcd file: must be done AFTER reading the input .wmd file
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readInputLcdFile(const char* lcdFilePath) noexcept {
    gLcd = {};
    std::string errorMsg;

    if (!gLcd.readFromLcdFile(lcdFilePath, gModule.psxPatchGroup, errorMsg)) {
        std::printf("%s\n", lcdFilePath);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets ADPCM data from the specified file.
// The file can either be a .vag or .wav file; if the file is in WAV format then it is encoded into the PSX ADPCM format.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool getSoundAdpcmData(const char* const soundFilePath, std::vector<std::byte>& adpcmData) noexcept {
    // Uppercase the filename for case insensitivity
    std::string uppercaseName = soundFilePath;
    std::transform(
        uppercaseName.begin(),
        uppercaseName.end(),
        uppercaseName.begin(),
        [](const char c) noexcept { return (char) ::toupper(c); }
    );

    // Is it a wav file? If it isn't then assume it's a .vag:
    const bool bIsWavFile = ((uppercaseName.length() >= 4) && (uppercaseName.rfind(".WAV") == uppercaseName.length() - 4));

    // Read the .vag or .wav file
    std::string errorMsg;

    if (bIsWavFile) {
        // Read the samples for the .wav
        std::vector<int16_t> pcmSamples;
        uint32_t numChannels = {};
        uint32_t sampleRate = {};
        uint32_t loopStartSamp = {};
        uint32_t loopEndSamp = {};

        if (!WavUtils::readWavFile(soundFilePath, pcmSamples, numChannels, sampleRate, loopStartSamp, loopEndSamp, errorMsg)) {
            std::printf("%s\n", errorMsg.c_str());
            return false;
        }

        // The .wav file must be mono to be used
        if (numChannels != 1) {
            std::printf("Error! Input .wav file '%s' is stereo! Only mono .wav files must be supplied!\n", errorMsg.c_str());
            return false;
        }

        // Encode the .wav file samples to adpcm
        VagUtils::encodePcmSoundToPsxAdpcm(pcmSamples.data(), (uint32_t) pcmSamples.size(), loopStartSamp, loopEndSamp, adpcmData);
    }
    else {
        // Read the samples for the .vag file
        uint32_t sampleRate = {};

        if (!VagUtils::readVagFile(soundFilePath, adpcmData, sampleRate, errorMsg)) {
            std::printf("%s\n", errorMsg.c_str());
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Dump the contents of the LCD file in both .wav and .vag formats
//------------------------------------------------------------------------------------------------------------------------------------------
static bool dumpLcd(const char* const lcdFilePath, const char* const wmdFilePath, const char* const outputDir) noexcept {
    // Read the input files
    if ((!readInputWmdFile(wmdFilePath)) || (!readInputLcdFile(lcdFilePath)))
        return false;

    // Save each of the lcd file sounds
    const PsxPatchGroup& patchGroup = gModule.psxPatchGroup;

    for (const LcdSample& sample : gLcd.samples) {
        // Guess the sample rate of this sample
        const uint32_t sampleRate = patchGroup.guessSampleRateForPatchSample(sample.patchSampleIdx);

        // Makeup the output file path without an extension in the form 'SAMP####' where '####' is the patch sample index
        std::string outFilePathNoExt = outputDir;

        if ((!outFilePathNoExt.empty()) && (outFilePathNoExt.back() != '/') && (outFilePathNoExt.back() != '\\')) {
            outFilePathNoExt += '/';
        }

        outFilePathNoExt += "SAMP";

        {
            char sampleIdxZeroPadded[32];
            std::snprintf(sampleIdxZeroPadded, sizeof(sampleIdxZeroPadded), "%04u", (unsigned) sample.patchSampleIdx);
            outFilePathNoExt += sampleIdxZeroPadded;
        }

        // Output to a .vag file and .wav file
        std::string vagFilePath = outFilePathNoExt + ".vag";
        std::string wavFilePath = outFilePathNoExt + ".wav";

        if (!VagUtils::writePsxAdpcmSoundToVagFile(vagFilePath.c_str(), sample.adpcmData.data(), (uint32_t) sample.adpcmData.size(), sampleRate)) {
            std::printf("Failed to write to output .vag file '%s'! Is the path writable?\n", vagFilePath.c_str());
            return false;
        }

        if (!WavUtils::writePsxAdpcmSoundToWavFile(wavFilePath.c_str(), sample.adpcmData.data(), (uint32_t) sample.adpcmData.size(), sampleRate)) {
            std::printf("Failed to write output .wav file '%s'! Is the path writable?\n", wavFilePath.c_str());
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// List the contents of the LCD file
//------------------------------------------------------------------------------------------------------------------------------------------
static bool listLcd(const char* const lcdFilePath, const char* const wmdFilePath) noexcept {
    // Read the input files
    if ((!readInputWmdFile(wmdFilePath)) || (!readInputLcdFile(lcdFilePath)))
        return false;

    // List all contained samples
    std::printf("Patch Sample Index,    Size (In bytes),    Size (Samples),     Guessed Sample Rate,    Guessed Length (Seconds),\n");

    uint32_t totalSizeInSamples = 0;
    uint32_t totalSizeInBytes = 0;

    for (const LcdSample& sample : gLcd.samples) {
        const uint32_t numBlocks = (uint32_t) sample.adpcmData.size() / VagUtils::ADPCM_BLOCK_SIZE;
        const uint32_t numSamples = numBlocks * VagUtils::ADPCM_BLOCK_NUM_SAMPLES;
        const uint32_t sampleRate = gModule.psxPatchGroup.guessSampleRateForPatchSample(sample.patchSampleIdx);
        const float duration = (float) numSamples / (float) sampleRate;

        // Print the stats for this sample
        std::printf("%-18u,    ", (unsigned) sample.patchSampleIdx);
        std::printf("%-15u,    ", (unsigned) sample.adpcmData.size());
        std::printf("%-14u,     ", (unsigned) numSamples);
        std::printf("%-19u,    ", (unsigned) sampleRate);
        std::printf("%-24f,\n", duration);

        // Add to the totals
        totalSizeInSamples += numSamples;
        totalSizeInBytes += (uint32_t) sample.adpcmData.size();
    }

    // Print the totals for all samples
    std::printf("\n");
    std::printf("Total size (samples):  %u\n", (unsigned) totalSizeInSamples);
    std::printf("Total size (bytes):    %u\n", (unsigned) totalSizeInBytes);

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Build an LCD file
//------------------------------------------------------------------------------------------------------------------------------------------
static bool buildLcd(
    const char* const lcdFilePath,
    const char* const wmdFilePath,
    const std::vector<PatchSampleFile>& patchSampleFiles,
    const bool bAppend
) noexcept {
    // Firstly get the adpcm data for all of the input sounds specified
    std::vector<std::vector<std::byte>> patchSamplesAdpcmData;

    for (const PatchSampleFile& patchSampleFile : patchSampleFiles) {
        if (!getSoundAdpcmData(patchSampleFile.filePath, patchSamplesAdpcmData.emplace_back()))
            return false;
    }

    // Need to read the input module file for verification purposes
    if (!readInputWmdFile(wmdFilePath))
        return false;

    // Read an existing LCD file if appending
    gLcd = {};

    if (bAppend) {
        if (!readInputLcdFile(lcdFilePath))
            return false;
    }

    // Add in all of the samples to the LCD
    uint32_t inputSampleIdx = 0;

    for (const PatchSampleFile& patchSampleFile : patchSampleFiles) {
        // Can we replace the audio for an existing entry in the LCD file or will we make a new one?
        if (LcdSample* pSample = gLcd.findPatchSample(patchSampleFile.patchSampleIdx); pSample) {
            pSample->adpcmData = std::move(patchSamplesAdpcmData[inputSampleIdx]);
        } else {
            LcdSample& sample = gLcd.samples.emplace_back();
            sample.patchSampleIdx = patchSampleFile.patchSampleIdx;
            sample.adpcmData = std::move(patchSamplesAdpcmData[inputSampleIdx]);
        }

        ++inputSampleIdx;
    }

    // Write out the LCD file to disk to finish up
    std::string errorMsg;

    if (!gLcd.writeToLcdFile(lcdFilePath, gModule.psxPatchGroup, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Program entrypoint
//------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, const char* const argv[]) noexcept {
    // Not enough arguments?
    if (argc < 4) {
        printHelp();
        return 1;
    }

    // See what command is being executed
    const char* const lcdFilePath = argv[1];
    const char* const wmdFilePath = argv[2];
    const char* const cmdSwitch = argv[3];

    if (std::strcmp(cmdSwitch, "-list") == 0) {
        // No args for this one
        if (argc == 4) {
            return (listLcd(lcdFilePath, wmdFilePath)) ? 0 : 1;
        }
    }
    else if (std::strcmp(cmdSwitch, "-dump") == 0) {
        // Dump dir is optional
        if ((argc == 4) || (argc == 5)) {
            const char* const outputDir = (argc >= 5) ? argv[4] : "";
            return (dumpLcd(lcdFilePath, wmdFilePath, outputDir)) ? 0 : 1;
        }
    }
    else if ((std::strcmp(cmdSwitch, "-create") == 0) || (std::strcmp(cmdSwitch, "-append") == 0)) {
        // Arg list for this one must be in pairs of two
        if (argc % 2 != 0) {
            printHelp();
            return 1;
        }

        // Build the list of patch sample indexes and associated files to pack
        const bool bAppendMode = (std::strcmp(cmdSwitch, "-append") == 0);
        std::vector<PatchSampleFile> patchSampleFiles;

        try {
            for (int i = 4; i < argc; i += 2) {
                const int patchSampleIdx = std::stoi(argv[i]);

                if ((patchSampleIdx < 0) || (patchSampleIdx > UINT16_MAX))
                    throw;

                patchSampleFiles.emplace_back(PatchSampleFile{ (uint16_t) patchSampleIdx, argv[i + 1]});
            }
        }
        catch (...) {
            std::printf("One or more patch sample indexes are malformed! Patch sample indexes must be integers between 0-65535.\n\n");
            printHelp();
            return 1;
        }

        // Build the LCD file
        return (buildLcd(lcdFilePath, wmdFilePath, patchSampleFiles, bAppendMode)) ? 0 : 1;
    }

    printHelp();
    return 1;
}
