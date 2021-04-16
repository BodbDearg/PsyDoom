//------------------------------------------------------------------------------------------------------------------------------------------
// WmdTool:
//      Utilities for converting .WMD files (Williams Module files) to JSON and visa versa.
//      Also utilities for importing and exporting sequences from and to MIDI.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MidiConvert.h"
#include "MidiTypes.h"
#include "MidiUtils.h"
#include "Module.h"
#include "ModuleFileUtils.h"

#include <algorithm>
#include <filesystem>

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Help/usage printing
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* const HELP_STR =
R"(Usage: WmdTool <COMMAND-SWITCH> [COMMAND ARGS]

Command switches and arguments:

    -wmd-to-json <INPUT WMD FILE PATH> <OUTPUT JSON FILE PATH>
        Convert an input module file in binary .WMD format to JSON text format.
        In this output format the data of the module file is human readable and can be edited by standard JSON editing tools.
        You should use this conversion prior to editing settings for a module and then do the opposite conversion to save those edits.
        Example:
            WmdTool -wmd-to-json DOOMSND.WMD DOOMSND.json

    -json-to-wmd <INPUT JSON FILE PATH> <OUTPUT WMD FILE PATH>
        Convert a module file in text JSON format to the binary .WMD format used by PlayStation Doom.
        Example:
            WmdTool -json-to-wmd DOOMSND.json DOOMSND.WMD

    -sequence-to-midi <INPUT JSON OR WMD FILE PATH> <SEQUENCE INDEX> <OUTPUT MIDI FILE PATH>
        Convert a sequence in the given module to MIDI format.
        Notes:
            (1) The sequence is specified by its index in the array of sequences in the module.
            (2) The input module file can be in JSON or binary .WMD format.
            (3) The tempo for the sequence is assumed to be constant and the same for all tracks.
                This will be the case for all tracks in the original PSX Doom.
        Example:
            WmdTool -sequence-to-midi DOOMSND.WMD 90 MAP01_MUSIC.MID
            WmdTool -sequence-to-midi DOOMSND.json 91 MAP02_MUSIC.MID

    -midi-to-sequence <INPUT MIDI FILE PATH> <INPUT/OUTPUT JSON OR WMD FILE PATH> <SEQUENCE INDEX> [-is-sfx]
        Import a sequence in MIDI format to the module.
        Notes:
            (1) The sequence is specified by its index in the array of sequences in the module.
            (2) The input module file can be in JSON or binary .WMD format and is saved to the same output format.
            (3) The sequence specified must already exist in the module. If you need to create a new empty sequence
                for a new song or sound, export the module file to JSON and add it using an appropriate JSON editor.
            (4) The track list for the sequence is completely replaced, however certain attributes for already existing
                tracks will be preserved to in order to support sequence updates. The preserved track attributes include:
                    - Start patch
                    - Start pitch bend
                    - Start volume
                    - Start pan
                    - Max voices
            (5) This tool DOES NOT assign patches to individual tracks, you must do this manually yourself.
                Again, export the module to JSON and edit in that format to setup the patches used by each track.
            (6) If the '-is-sfx' flag is specified then the sequence is treated as a sequence for a sound effect.
                This affects voice priority, sound class and whether the sequence is looped (music is automatically looped).
        Example:
            WmdTool -midi-to-sequence MY_MIDI.MID DOOMSND.WMD 90
            WmdTool -midi-to-sequence MY_MIDI.MID DOOMSND.JSON 1 -is-sfx

    -copy-sequences <INPUT JSON OR WMD FILE PATH> <OUTPUT JSON OR WMD FILE PATH> [SEQUENCE INDEX...]
        Copy the specified sequences from one module file to another.
        All referenced patches, patch voices and patch samples are also copied into the destination module with remapped indexes.
        Notes:
            (1) Sequences are specified by their indexes in the array of sequences in the module.
            (2) The input and output module files can be in either JSON or binary .WMD format.
            (3) If the output file does not exist then it will be created.
            (4) Each sequence can only be copied once, requests to copy the same sequence multiple times will be ignored.
            (5) Sequences are copied in ASCENDING order of sequence index to the destination module, regardless of command order.
            (6) The remapped sequence, patch, patch voice and patch sample numbers will be printed on successful output.
        Example:
            WmdTool -copy-sequences DOOMSND.WMD DOOMSND.json 90 91 92
            WmdTool -copy-sequences DOOMSND.json DOOMSND.WMD 92

    -copy-patches <INPUT JSON OR WMD FILE PATH> <OUTPUT JSON OR WMD FILE PATH> [PATCH INDEX...]
        Copy the specified patches from one module file to another.
        All referenced patch voices and patch samples are also copied into the destination module with remapped indexes.
        Notes:
            (1) Patches are specified by their indexes in the array of patches in the module.
            (2) The input and output module files can be in either JSON or binary .WMD format.
            (3) If the output file does not exist then it will be created.
            (4) Each patch can only be copied once, requests to copy the same patch multiple times will be ignored.
            (5) Patches are copied in ASCENDING order of patch index to the destination module, regardless of command order.
            (6) The remapped patch, patch voice and patch sample numbers will be printed on successful output.
        Example:
            WmdTool -copy-patches DOOMSND.WMD DOOMSND.json 0 1 2
            WmdTool -copy-patches DOOMSND.json DOOMSND.WMD 5
)";

static void printHelp() noexcept {
    std::printf("%s\n", HELP_STR);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the specified file exist? Returns 'false' also if an error determining occurred
//------------------------------------------------------------------------------------------------------------------------------------------
static bool fileExists(const char* const filePath) noexcept {
    try {
        return std::filesystem::exists(filePath);
    } catch (...) {
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell if the given file path has the .WMD extension
//------------------------------------------------------------------------------------------------------------------------------------------
static bool filePathHasWmdExtension(const char* const filePath) noexcept {
    const size_t fpathLen = std::strlen(filePath);
    return (
        (fpathLen >= 4) &&
        (std::toupper(filePath[fpathLen - 4] == '.')) &&
        (std::toupper(filePath[fpathLen - 3] == 'W')) &&
        (std::toupper(filePath[fpathLen - 2] == 'M')) &&
        (std::toupper(filePath[fpathLen - 1] == 'D'))
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a module file in .json or binary .wmd format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readModuleFile(const char* const filePath, bool& bIsJsonModule, Module& module) noexcept {
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
    bIsJsonModule = bGuessIsJsonFile;

    // Do the reading of the module file, starting with the guessed format
    std::string readAsWmdErrorMsg;
    std::string readAsJsonErrorMsg;

    const auto readAsWmd = [&]() noexcept {
        if (ModuleFileUtils::readWmdFile(filePath, module, readAsWmdErrorMsg)) {
            bIsJsonModule = false;
            return true;
        }

        return false;
    };

    const auto readAsJson = [&]() noexcept {
        if (ModuleFileUtils::readJsonFile(filePath, module, readAsJsonErrorMsg)) {
            bIsJsonModule = true;
            return true;
        }

        return false;
    };

    if (bGuessIsJsonFile) {
        if (!readAsJson()) {
            if (!readAsWmd()) {
                std::printf("%s\n", readAsJsonErrorMsg.c_str());
                return false;
            }
        }
    } else {
        if (!readAsWmd()) {
            if (!readAsJson()) {
                std::printf("%s\n", readAsWmdErrorMsg.c_str());
                return false;
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a module file in .WMD format to JSON format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool convertWmdToJson(const char* const wmdFileIn, const char* const jsonFileOut) noexcept {
    // Read the input .WMD file
    Module module = {};
    std::string errorMsg;

    if (!ModuleFileUtils::readWmdFile(wmdFileIn, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    // Write the output JSON file
    if (!ModuleFileUtils::writeJsonFile(jsonFileOut, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a module file in JSON format to .WMD format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool convertJsonToWmd(const char* const jsonFileIn, const char* const wmdFileOut) noexcept {
    // Read the input JSON file
    Module module = {};
    std::string errorMsg;

    if (!ModuleFileUtils::readJsonFile(jsonFileIn, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    // Write the output .WMD file
    if (!ModuleFileUtils::writeWmdFile(wmdFileOut, module, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a sequence to MIDI format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool convertSequenceToMidi(const char* const wmdFileIn, int sequenceIdx, const char* const midiFileOut) noexcept {
    // Read the input module file firstly
    Module module = {};
    bool bIsJsonModule = {};

    if (!readModuleFile(wmdFileIn, bIsJsonModule, module))
        return false;

    // Is the sequence index in range?
    if ((sequenceIdx < 0) || (sequenceIdx >= (int) module.sequences.size())) {
        std::printf("Invalid sequence index within the module specified! Valid range is 0-%zu!\n", module.sequences.size());
        return false;
    }

    // Convert the sequence to a MIDI file
    MidiFile midiFile = {};
    MidiConvert::sequenceToMidi(module.sequences[sequenceIdx], midiFile);

    // Save the MIDI file
    if (!MidiUtils::writeMidiFile(midiFileOut, midiFile)) {
        std::printf("Error! Failed to write to the output .midi file '%s'! Is that file path writable or is the disk full?\n", midiFileOut);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Import a MIDI file to a sequence
//------------------------------------------------------------------------------------------------------------------------------------------
static bool convertMidiToSequence(const char* const midiFileIn, const char* const moduleFilePath, const int sequenceIdx, const bool bIsSfx) noexcept {
    // Read the input MIDI file firstly
    std::string errorMsg;
    MidiFile midiFile = {};

    if (!MidiUtils::readMidiFile(midiFileIn, midiFile, errorMsg)) {
        std::printf("%s\n", errorMsg.c_str());
        return false;
    }

    // Then read the module file
    Module module = {};
    bool bIsJsonModule = {};

    if (!readModuleFile(moduleFilePath, bIsJsonModule, module))
        return false;

    // Is the sequence index in range?
    if ((sequenceIdx < 0) || (sequenceIdx >= (int) module.sequences.size())) {
        std::printf("Invalid sequence index within the module specified! Valid range is 0-%zu!\n", module.sequences.size());
        return false;
    }

    // Convert the MIDI to a sequence
    MidiConvert::midiToSequence(midiFile, module.sequences[sequenceIdx], (!bIsSfx));

    // Save the module file
    if (bIsJsonModule) {
        if (!ModuleFileUtils::writeJsonFile(moduleFilePath, module, errorMsg)) {
            std::printf("%s\n", errorMsg.c_str());
            return false;
        }
    } else {
        if (!ModuleFileUtils::writeWmdFile(moduleFilePath, module, errorMsg)) {
            std::printf("%s\n", errorMsg.c_str());
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copy sequences or patches to a destination WMD file in binary or json format
//------------------------------------------------------------------------------------------------------------------------------------------
static bool copyPatchesOrSequences(
    const char* const srcModuleFilePath,
    const char* const dstModuleFilePath,
    const bool bCopySequences,
    const std::set<uint16_t>& elemIndexesToCopy
) noexcept {
    // First read the source module file
    Module srcModule = {};
    bool bIsSrcModuleJson = {};

    if (!readModuleFile(srcModuleFilePath, bIsSrcModuleJson, srcModule))
        return false;

    // Read the destination module if it exists, otherwise default initialize
    Module dstModule = {};
    bool bIsDstModuleJson = {};

    if (fileExists(dstModuleFilePath)) {
        if (!readModuleFile(dstModuleFilePath, bIsDstModuleJson, dstModule))
            return false;
    } else {
        dstModule.resetToDefault();
        bIsDstModuleJson = (!filePathHasWmdExtension(dstModuleFilePath));
    }

    // Copy the sequences or patches into the destination module (2 different modes)
    std::map<uint16_t, uint16_t> oldAndNewSequenceIndexes;
    std::map<uint16_t, uint16_t> oldAndNewPatchIndexes;
    std::map<uint16_t, uint16_t> oldAndNewPatchVoiceIndexes;
    std::map<uint16_t, uint16_t> oldAndNewPatchSampleIndexes;

    if (bCopySequences) {
        srcModule.copySequences(
            elemIndexesToCopy,
            dstModule,
            oldAndNewSequenceIndexes,
            oldAndNewPatchIndexes,
            oldAndNewPatchVoiceIndexes,
            oldAndNewPatchSampleIndexes
        );
    } else {
        srcModule.copyPatches(
            elemIndexesToCopy,
            dstModule,
            oldAndNewPatchIndexes,
            oldAndNewPatchVoiceIndexes,
            oldAndNewPatchSampleIndexes
        );
    }

    // Try to save the output module
    {
        std::string errorMsg;
        bool bWriteSuccesful = {};

        if (bIsDstModuleJson) {
            bWriteSuccesful = ModuleFileUtils::writeJsonFile(dstModuleFilePath, dstModule, errorMsg);
        } else {
            bWriteSuccesful = ModuleFileUtils::writeWmdFile(dstModuleFilePath, dstModule, errorMsg);
        }

        if (!bWriteSuccesful) {
            std::printf("%s\n", errorMsg.c_str());
            return false;
        }
    }

    // All good if we got to here, print the details for the stuff copied
    std::printf("Changes saved successfully to '%s'!\n", dstModuleFilePath);

    const auto printOldAndNewIndexes = [](const std::map<uint16_t, uint16_t>& oldAndNewIndexMap) noexcept {
        for (std::pair<uint16_t, uint16_t> oldAndNew : oldAndNewIndexMap) {
            std::printf("    %-4u -> %u\n", oldAndNew.first, oldAndNew.second);
        }
    };

    if (bCopySequences) {
        std::printf("\nCopied sequences:\n");
        printOldAndNewIndexes(oldAndNewSequenceIndexes);
    }

    std::printf("\nCopied patches:\n");
    printOldAndNewIndexes(oldAndNewPatchIndexes);

    std::printf("\nCopied patch voices:\n");
    printOldAndNewIndexes(oldAndNewPatchVoiceIndexes);

    std::printf("\nCopied patch samples:\n");
    printOldAndNewIndexes(oldAndNewPatchSampleIndexes);

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

    if (std::strcmp(cmdSwitch, "-wmd-to-json") == 0) {
        if (argc == 4) {
            const char* const wmdFilePath = argv[2];
            const char* const jsonFilePath = argv[3];
            return (convertWmdToJson(wmdFilePath, jsonFilePath)) ? 0 : 1;
        }
    }
    else if (std::strcmp(cmdSwitch, "-json-to-wmd") == 0) {
        if (argc == 4) {
            const char* const jsonFilePath = argv[2];
            const char* const wmdFilePath = argv[3];
            return (convertJsonToWmd(jsonFilePath, wmdFilePath)) ? 0 : 1;
        }
    }
    else if (std::strcmp(cmdSwitch, "-sequence-to-midi") == 0) {
        if (argc == 5) {
            const char* const moduleFilePath = argv[2];
            const char* const sequenceIdxStr = argv[3];
            const char* const midiFilePath = argv[4];
            int sequenceIdx = {};

            try {
                sequenceIdx = std::stoi(sequenceIdxStr);
            } catch (...) {
                printHelp();
                return 1;
            }

            return (convertSequenceToMidi(moduleFilePath, sequenceIdx, midiFilePath)) ? 0 : 1;
        }
    }
    else if (std::strcmp(cmdSwitch, "-midi-to-sequence") == 0) {
        if ((argc == 5) || (argc == 6)) {
            const char* const midiFilePath = argv[2];
            const char* const moduleFilePath = argv[3];
            const char* const sequenceIdxStr = argv[4];
            int sequenceIdx = {};

            try {
                sequenceIdx = std::stoi(sequenceIdxStr);
            } catch (...) {
                printHelp();
                return 1;
            }

            bool bIsSfx = false;

            if (argc == 6) {
                const char* const extraSwitch = argv[5];

                if (std::strcmp(extraSwitch, "-is-sfx") == 0) {
                    bIsSfx = true;
                } else {
                    printHelp();
                    return 1;
                }
            }

            return (convertMidiToSequence(midiFilePath, moduleFilePath, sequenceIdx, bIsSfx)) ? 0 : 1;
        }
    }
    else if ((std::strcmp(cmdSwitch, "-copy-sequences") == 0) || (std::strcmp(cmdSwitch, "-copy-patches") == 0)) {
        const bool bCopySequences = (std::strcmp(cmdSwitch, "-copy-sequences") == 0);

        if (argc >= 4) {
            // Get the indexes of the elements to copy
            std::set<uint16_t> elemIndexesToCopy;

            for (int i = 4; i < argc; ++i) {
                try {
                    const int elemIdx = std::stoi(argv[i]);

                    if ((elemIdx < 0) || (elemIdx > UINT16_MAX))
                        throw;

                    elemIndexesToCopy.insert((uint16_t) elemIdx);
                } catch (...) {
                    printHelp();
                    return 1;
                }
            }

            // Do the copy
            const char* const srcModuleFilePath = argv[2];
            const char* const dstModuleFilePath = argv[3];
            return (copyPatchesOrSequences(srcModuleFilePath, dstModuleFilePath, bCopySequences, elemIndexesToCopy)) ? 0 : 1;
        }
    }

    printHelp();
    return 1;
}
