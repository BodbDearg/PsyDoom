#include "WmdFileTypes.h"

#include "Endian.h"

#include <map>
#include <string>

using namespace AudioTools;

// Note: must be uppercase for case insensitive comparison purposes
static std::map<std::string, WmdSoundDriverId> gStrToSoundDriverId = {
    { "NOSOUND",    WmdSoundDriverId::NoSound },
    { "PSX",        WmdSoundDriverId::PSX },
    { "GENERIC",    WmdSoundDriverId::GENERIC },
};

static std::map<std::string, WmdSoundClass> gStrToSoundClass = {
    { "SNDFX",      WmdSoundClass::SNDFX },
    { "MUSIC",      WmdSoundClass::MUSIC },
    { "DRUMS",      WmdSoundClass::DRUMS },
    { "SFXDRUMS",   WmdSoundClass::SFXDRUMS },
};

static std::map<std::string, WmdTrackCmdType> gStrToTrackCmdType = {
    { "DRIVERINIT",     WmdTrackCmdType::DriverInit },
    { "DRIVEREXIT",     WmdTrackCmdType::DriverExit },
    { "DRIVERENTRY1",   WmdTrackCmdType::DriverEntry1 },
    { "DRIVERENTRY2",   WmdTrackCmdType::DriverEntry2 },
    { "DRIVERENTRY3",   WmdTrackCmdType::DriverEntry3 },
    { "TRKOFF",         WmdTrackCmdType::TrkOff },
    { "TRKMUTE",        WmdTrackCmdType::TrkMute },
    { "PATCHCHG",       WmdTrackCmdType::PatchChg },
    { "PATCHMOD",       WmdTrackCmdType::PatchMod },
    { "PITCHMOD",       WmdTrackCmdType::PitchMod },
    { "ZEROMOD",        WmdTrackCmdType::ZeroMod },
    { "MODUMOD",        WmdTrackCmdType::ModuMod },
    { "VOLUMEMOD",      WmdTrackCmdType::VolumeMod },
    { "PANMOD",         WmdTrackCmdType::PanMod },
    { "PEDALMOD",       WmdTrackCmdType::PedalMod },
    { "REVERBMOD",      WmdTrackCmdType::ReverbMod },
    { "CHORUSMOD",      WmdTrackCmdType::ChorusMod },
    { "NOTEON",         WmdTrackCmdType::NoteOn },
    { "NOTEOFF",        WmdTrackCmdType::NoteOff },
    { "STATUSMARK",     WmdTrackCmdType::StatusMark },
    { "GATEJUMP",       WmdTrackCmdType::GateJump },
    { "ITERJUMP",       WmdTrackCmdType::IterJump },
    { "RESETGATES",     WmdTrackCmdType::ResetGates },
    { "RESETITERS",     WmdTrackCmdType::ResetIters },
    { "WRITEITERBOX",   WmdTrackCmdType::WriteIterBox },
    { "SEQTEMPO",       WmdTrackCmdType::SeqTempo },
    { "SEQGOSUB",       WmdTrackCmdType::SeqGosub },
    { "SEQJUMP",        WmdTrackCmdType::SeqJump },
    { "SEQRET",         WmdTrackCmdType::SeqRet },
    { "SEQEND",         WmdTrackCmdType::SeqEnd },
    { "TRKTEMPO",       WmdTrackCmdType::TrkTempo },
    { "TRKGOSUB",       WmdTrackCmdType::TrkGosub },
    { "TRKJUMP",        WmdTrackCmdType::TrkJump },
    { "TRKRET",         WmdTrackCmdType::TrkRet },
    { "TRKEND",         WmdTrackCmdType::TrkEnd },
    { "NULLEVENT",      WmdTrackCmdType::NullEvent },
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Endian correction stubs: swap bytes if in big endian mode.
// This way we can go from little to big, and big to little endian.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void endianCorrect([[maybe_unused]] T& value) noexcept {
    if constexpr (!Endian::isLittle()) {
        Endian::byteSwap(value);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Endian correction for WMD file data structures if the host CPU is big endian
//------------------------------------------------------------------------------------------------------------------------------------------
void WmdModuleHdr::endianCorrect() noexcept {
    ::endianCorrect(moduleId);
    ::endianCorrect(moduleVersion);
    ::endianCorrect(numSequences);
}

void WmdPatchGroupHdr::endianCorrect() noexcept {
    ::endianCorrect(loadFlags);
    ::endianCorrect(numPatches);
    ::endianCorrect(patchSize);
    ::endianCorrect(numPatchVoices);
    ::endianCorrect(patchVoiceSize);
    ::endianCorrect(numPatchSamples);
    ::endianCorrect(patchSampleSize);
    ::endianCorrect(numDrumPatches);
    ::endianCorrect(drumPatchSize);
    ::endianCorrect(extraDataSize);
}

void WmdSequenceHdr::endianCorrect() noexcept {
    ::endianCorrect(numTracks);
    ::endianCorrect(unknownField);
}

void WmdTrackHdr::endianCorrect() noexcept {
    ::endianCorrect(initPatchIdx);
    ::endianCorrect(initPitchCntrl);
    ::endianCorrect(initPpq);
    ::endianCorrect(initQpm);
    ::endianCorrect(numLabels);
    ::endianCorrect(cmdStreamSize);
}

void WmdPsxPatch::endianCorrect() noexcept {
    ::endianCorrect(numVoices);
    ::endianCorrect(firstVoiceIdx);
}

void WmdPsxPatchVoice::endianCorrect() noexcept {
    ::endianCorrect(sampleIdx);
    ::endianCorrect(adsr1);
    ::endianCorrect(adsr2);
}

void WmdPsxPatchSample::endianCorrect() noexcept {
    ::endianCorrect(offset);
    ::endianCorrect(size);
    ::endianCorrect(spuAddr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the size in bytes of the specified sequencer command.
// If '0' is returned then it is either an invalid command or not expected to be present in a track.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t AudioTools::getWmdTrackCmdSize(const WmdTrackCmdType type) noexcept {
    switch (type) {
        // Manually called commands (should never be in a track's command set)
        case WmdTrackCmdType::DriverInit:
        case WmdTrackCmdType::DriverExit:
        case WmdTrackCmdType::DriverEntry1:
        case WmdTrackCmdType::DriverEntry2:
        case WmdTrackCmdType::DriverEntry3:
        case WmdTrackCmdType::TrkOff:
        case WmdTrackCmdType::TrkMute:
            return 0;

        // Sound driver specific commands
        case WmdTrackCmdType::PatchChg:     return 3;
        case WmdTrackCmdType::PatchMod:     return 2;
        case WmdTrackCmdType::PitchMod:     return 3;
        case WmdTrackCmdType::ZeroMod:      return 2;
        case WmdTrackCmdType::ModuMod:      return 2;
        case WmdTrackCmdType::VolumeMod:    return 2;
        case WmdTrackCmdType::PanMod:       return 2;
        case WmdTrackCmdType::PedalMod:     return 2;
        case WmdTrackCmdType::ReverbMod:    return 2;
        case WmdTrackCmdType::ChorusMod:    return 2;
        case WmdTrackCmdType::NoteOn:       return 3;
        case WmdTrackCmdType::NoteOff:      return 2;

        // Sequencer generic commands
        case WmdTrackCmdType::StatusMark:   return 4;
        case WmdTrackCmdType::GateJump:     return 5;
        case WmdTrackCmdType::IterJump:     return 5;
        case WmdTrackCmdType::ResetGates:   return 2;
        case WmdTrackCmdType::ResetIters:   return 2;
        case WmdTrackCmdType::WriteIterBox: return 3;
        case WmdTrackCmdType::SeqTempo:     return 3;
        case WmdTrackCmdType::SeqGosub:     return 3;
        case WmdTrackCmdType::SeqJump:      return 3;
        case WmdTrackCmdType::SeqRet:       return 1;
        case WmdTrackCmdType::SeqEnd:       return 1;
        case WmdTrackCmdType::TrkTempo:     return 3;
        case WmdTrackCmdType::TrkGosub:     return 3;
        case WmdTrackCmdType::TrkJump:      return 3;
        case WmdTrackCmdType::TrkRet:       return 1;
        case WmdTrackCmdType::TrkEnd:       return 1;
        case WmdTrackCmdType::NullEvent:    return 1;

        default:
            break;
    }

    return 0;   // Unknown command type!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell how many arguments the given track command has
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t AudioTools::getNumWmdTrackCmdArgs(const WmdTrackCmdType type) noexcept {
    switch (type) {
        case WmdTrackCmdType::DriverInit:
        case WmdTrackCmdType::DriverExit:
        case WmdTrackCmdType::DriverEntry1:
        case WmdTrackCmdType::DriverEntry2:
        case WmdTrackCmdType::DriverEntry3:
        case WmdTrackCmdType::TrkOff:
        case WmdTrackCmdType::TrkMute:
        case WmdTrackCmdType::SeqRet:
        case WmdTrackCmdType::SeqEnd:
        case WmdTrackCmdType::TrkRet:
        case WmdTrackCmdType::TrkEnd:
        case WmdTrackCmdType::NullEvent:
            return 0;
            
        case WmdTrackCmdType::PatchChg:
        case WmdTrackCmdType::PatchMod:
        case WmdTrackCmdType::PitchMod:
        case WmdTrackCmdType::ZeroMod:
        case WmdTrackCmdType::ModuMod:
        case WmdTrackCmdType::VolumeMod:
        case WmdTrackCmdType::PanMod:
        case WmdTrackCmdType::PedalMod:
        case WmdTrackCmdType::ReverbMod:
        case WmdTrackCmdType::ChorusMod:
        case WmdTrackCmdType::NoteOff:
        case WmdTrackCmdType::ResetGates:
        case WmdTrackCmdType::ResetIters:
        case WmdTrackCmdType::SeqTempo:
        case WmdTrackCmdType::SeqGosub:
        case WmdTrackCmdType::SeqJump:
        case WmdTrackCmdType::TrkTempo:
        case WmdTrackCmdType::TrkGosub:
        case WmdTrackCmdType::TrkJump:
            return 1;
            
        case WmdTrackCmdType::NoteOn:
        case WmdTrackCmdType::StatusMark:        
        case WmdTrackCmdType::WriteIterBox:
            return 2;

        case WmdTrackCmdType::GateJump:
        case WmdTrackCmdType::IterJump:
            return 3;
        
        default: break;
    }
    
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Conversion of an enum value to a string
//------------------------------------------------------------------------------------------------------------------------------------------
const char* AudioTools::toString(const WmdSoundDriverId value) noexcept {
    switch (value) {
        case WmdSoundDriverId::NoSound: return "NoSound";
        case WmdSoundDriverId::PSX:     return "PSX";
        case WmdSoundDriverId::GENERIC: return "GENERIC";

        default:
            break;
    }
    
    return "";
}

const char* AudioTools::toString(const WmdSoundClass value) noexcept {
    switch (value) {
        case WmdSoundClass::SNDFX:      return "SNDFX";
        case WmdSoundClass::MUSIC:      return "MUSIC";
        case WmdSoundClass::DRUMS:      return "DRUMS";
        case WmdSoundClass::SFXDRUMS:   return "SFXDRUMS";
    }
    
    return "";
}

const char* AudioTools::toString(const WmdTrackCmdType value) noexcept {
    switch (value) {
        case WmdTrackCmdType::DriverInit:       return "DriverInit";
        case WmdTrackCmdType::DriverExit:       return "DriverExit";
        case WmdTrackCmdType::DriverEntry1:     return "DriverEntry1";
        case WmdTrackCmdType::DriverEntry2:     return "DriverEntry2";
        case WmdTrackCmdType::DriverEntry3:     return "DriverEntry3";
        case WmdTrackCmdType::TrkOff:           return "TrkOff";
        case WmdTrackCmdType::TrkMute:          return "TrkMute";
        case WmdTrackCmdType::PatchChg:         return "PatchChg";
        case WmdTrackCmdType::PatchMod:         return "PatchMod";
        case WmdTrackCmdType::PitchMod:         return "PitchMod";
        case WmdTrackCmdType::ZeroMod:          return "ZeroMod";
        case WmdTrackCmdType::ModuMod:          return "ModuMod";
        case WmdTrackCmdType::VolumeMod:        return "VolumeMod";
        case WmdTrackCmdType::PanMod:           return "PanMod";
        case WmdTrackCmdType::PedalMod:         return "PedalMod";
        case WmdTrackCmdType::ReverbMod:        return "ReverbMod";
        case WmdTrackCmdType::ChorusMod:        return "ChorusMod";
        case WmdTrackCmdType::NoteOn:           return "NoteOn";
        case WmdTrackCmdType::NoteOff:          return "NoteOff";
        case WmdTrackCmdType::StatusMark:       return "StatusMark";
        case WmdTrackCmdType::GateJump:         return "GateJump";
        case WmdTrackCmdType::IterJump:         return "IterJump";
        case WmdTrackCmdType::ResetGates:       return "ResetGates";
        case WmdTrackCmdType::ResetIters:       return "ResetIters";
        case WmdTrackCmdType::WriteIterBox:     return "WriteIterBox";
        case WmdTrackCmdType::SeqTempo:         return "SeqTempo";
        case WmdTrackCmdType::SeqGosub:         return "SeqGosub";
        case WmdTrackCmdType::SeqJump:          return "SeqJump";
        case WmdTrackCmdType::SeqRet:           return "SeqRet";
        case WmdTrackCmdType::SeqEnd:           return "SeqEnd";
        case WmdTrackCmdType::TrkTempo:         return "TrkTempo";
        case WmdTrackCmdType::TrkGosub:         return "TrkGosub";
        case WmdTrackCmdType::TrkJump:          return "TrkJump";
        case WmdTrackCmdType::TrkRet:           return "TrkRet";
        case WmdTrackCmdType::TrkEnd:           return "TrkEnd";
        case WmdTrackCmdType::NullEvent:        return "NullEvent";
    }
    
    return "";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper for string to enum conversion: returns the enum value '-1' if not found
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static T stringToEnum(const char* const str, const std::map<std::string, T>& nameToEnumMap) noexcept {
    // Get an uppercase version of the string for case insensitive comparison
    const size_t strLength = std::strlen(str);
    char strUpper[33];
    std::strncpy(strUpper, str, 32);
    strUpper[32] = 0;
    std::transform(strUpper, strUpper + strLength, strUpper, ::toupper);
    
    // Find in the name map and return the enum
    const auto iter = nameToEnumMap.find(strUpper);
    return (iter != nameToEnumMap.end()) ? iter->second : (T) -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Conversion of a string to an enum value
//------------------------------------------------------------------------------------------------------------------------------------------
WmdSoundDriverId AudioTools::stringToSoundDriverId(const char* const value) noexcept {
    return stringToEnum(value, gStrToSoundDriverId);
}

WmdSoundClass AudioTools::stringToSoundClass(const char* const value) noexcept {
    return stringToEnum(value, gStrToSoundClass);
}

WmdTrackCmdType AudioTools::stringToTrackCmdType(const char* const value) noexcept {
    return stringToEnum(value, gStrToTrackCmdType);
}
