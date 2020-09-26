//------------------------------------------------------------------------------------------------------------------------------------------
// VagTool:
//      Convert from .wav to a PlayStation 1 .vag audio file and visa-versa.
//      Also allows basic inspection of a .vag file.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VagUtils.h"
#include "WavUtils.h"

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Help/usage printing
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* const HELP_STR =
R"(Usage: VagTool <COMMAND-SWITCH> [COMMAND ARGS]

Command switches and arguments:

    -info <INPUT VAG FILE PATH>
        List the basic information for a PlayStation 1 .VAG file.
        Example:
            VagTool -info PSX_SOUND.VAG

    -wav-to-vag <INPUT WAV FILE PATH> <OUTPUT VAG FILE PATH>
        Convert an input .WAV file into PlayStation 1 .VAG format, which is ADPCM encoded.
        Notes:
            (1) The input .WAV file must be 16-bit, uncompressed PCM and in mono format.
            (2) Loop point info from .WAV files is extracted from the 'smpl' chunk. Only the 1st loop point is used.
            (3) Sounds are padded to multiples of 28 samples. Ideally supply audio already in this format.
            (4) Loop points are rounded to the nearest 28 samples as required by the hardware. Ideally do this yourself.
        Example:
            VagTool -wav-to-vag SOME_SOUND.WAV PSX_SOUND.VAG

    -vag-to-wav <INPUT WAV FILE PATH> <OUTPUT VAG FILE PATH>
        Convert an input PlayStation 1 .VAG file into a standard .WAV file.
        Notes:
            (1) The .WAV file will be output with loop point info via the 'smpl' chunk.
        Example:
            VagTool -vag-to-wav PSX_SOUND.VAG SOME_SOUND.WAV
)";

static void printHelp() noexcept {
    std::printf("%s\n", HELP_STR);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print the info for a .vag file
//------------------------------------------------------------------------------------------------------------------------------------------
static bool printVagInfo(const char* const vagFilePath) noexcept {
    // Read the input VAG file
    std::vector<std::byte> adpcmData;
    uint32_t sampleRate = {};
    std::string readErrorMsg;

    if (!VagUtils::readVagFile(vagFilePath, adpcmData, sampleRate, readErrorMsg)) {
        std::printf("%s\n", readErrorMsg.c_str());
        return false;
    }

    // Get the loop points by decoding the file, slightly wasteful in theory but more than OK for a tool like this
    std::vector<int16_t> pcmSamples;
    uint32_t loopStartSample = {};
    uint32_t loopEndSample = {};
    VagUtils::decodePsxAdpcmSamples(adpcmData.data(), (uint32_t) adpcmData.size(), pcmSamples, loopStartSample, loopEndSample);
    const bool bIsLooped = (loopStartSample != loopEndSample);

    // Print its info
    std::printf("Sample rate:                   %u\n", sampleRate);
    std::printf("Total duration (seconds):      %f\n", (double) pcmSamples.size() / (double) sampleRate);
    std::printf("Total duration (samples):      %zu\n", (adpcmData.size() / VagUtils::ADPCM_BLOCK_SIZE) * VagUtils::ADPCM_BLOCK_NUM_SAMPLES);
    std::printf("Audio only size (bytes):       %zu\n", adpcmData.size());
    std::printf("Audio + header size (bytes):   %zu\n", adpcmData.size() + sizeof(VagUtils::VagFileHdr));
    std::printf("Looped:                        %s\n", (bIsLooped) ? "Yes" : "No");
    
    if (bIsLooped) {
        const int loopDurationSamples = (int) loopEndSample - (int) loopStartSample;

        std::printf("Loop start sample:             %u\n", loopStartSample);
        std::printf("Loop end sample (exclusive):   %u\n", loopEndSample);
        std::printf("Loop duration (seconds):       %f\n", (double) loopDurationSamples / (double) sampleRate);
        std::printf("Loop duration (samples):       %d\n", loopDurationSamples);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a .wav file to a .vag file
//------------------------------------------------------------------------------------------------------------------------------------------
static bool convertWavToVag(const char* const wavFilePath, const char* const vagFilePath) noexcept {
    // Read the input .wav file
    std::string errorMsg;

    std::vector<int16_t> wavSamples;
    uint32_t numWavChannels = {};
    uint32_t wavSampleRate = {};
    uint32_t wavLoopStartSample = {};
    uint32_t wavLoopEndSample = {};

    if (!WavUtils::readWavFile(wavFilePath, wavSamples, numWavChannels, wavSampleRate, wavLoopStartSample, wavLoopEndSample, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    // The input wav must be mono to be converted
    if (numWavChannels != 1) {
        std::printf("Error! Input wav file '%s' must be in MONO format to be converted rather than having '%u' channels!\n", wavFilePath, (unsigned) numWavChannels);
        return false;
    }

    // Write the .vag file
    if (!VagUtils::writePcmSoundToVagFile(vagFilePath, wavSamples.data(), (uint32_t) wavSamples.size(), wavSampleRate, wavLoopStartSample, wavLoopEndSample)) {
        std::printf("Error! Failed to write to the output .vag file '%s'! Is that file path writable or is the disk full?\n", vagFilePath);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a .vag file to a .wav file
//------------------------------------------------------------------------------------------------------------------------------------------
static bool convertVagToWav(const char* const vagFilePath, const char* const wavFilePath) noexcept {
    // Read the ADPCM data from the .vag file
    std::string errorMsg;

    std::vector<std::byte> adpcmData;
    uint32_t vagSampleRate = {};

    if (!VagUtils::readVagFile(vagFilePath, adpcmData, vagSampleRate, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    // Decode it to PCM
    std::vector<int16_t> pcmSamples;
    uint32_t loopStartSample = {};
    uint32_t loopEndSample = {};
    VagUtils::decodePsxAdpcmSamples(adpcmData.data(), (uint32_t) adpcmData.size(), pcmSamples, loopStartSample, loopEndSample);

    // Write the .wav file
    if (!WavUtils::writePcmSoundToWavFile(wavFilePath, pcmSamples.data(), (uint32_t) pcmSamples.size(), 1, vagSampleRate, loopStartSample, loopEndSample)) {
        std::printf("Error! Failed to write to the output .wav file '%s'! Is that file path writable or is the disk full?\n", wavFilePath);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Program entrypoint
//------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, const char* const argv[]) noexcept {
    // Not enough arguments?
    if (argc < 2) {
        printHelp();
        return 1;
    }

    // See what command is being executed
    const char* const cmdSwitch = argv[1];

    if (std::strcmp(cmdSwitch, "-info") == 0) {
        if (argc == 3) {
            const char* const vagFilePath = argv[2];
            return (printVagInfo(vagFilePath)) ? 0 : 1;
        }
    }
    else if (std::strcmp(cmdSwitch, "-wav-to-vag") == 0) {
        if (argc == 4) {
            const char* const wavFilePath = argv[2];
            const char* const vagFilePath = argv[3];
            return (convertWavToVag(wavFilePath, vagFilePath)) ? 0 : 1;
        }
    }
    else if (std::strcmp(cmdSwitch, "-vag-to-wav") == 0) {
        if (argc == 4) {
            const char* const vagFilePath = argv[2];
            const char* const wavFilePath = argv[3];
            return (convertVagToWav(vagFilePath, wavFilePath)) ? 0 : 1;
        }
    }

    printHelp();
    return 1;
}
