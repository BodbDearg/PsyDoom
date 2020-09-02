#include "WmdFileTypes.h"

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Endian correction stubs: swap bytes if in big endian mode. This way we can go from little to big, and big to little endian.
// Right now I'm not bothering to implement but if we want to swap bytes for big endian mode, just write the code here.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void endianCorrect([[maybe_unused]] T& value) noexcept {
    // Don't bother implementing this for now...
    // Once C++ 20 is widely available this function could just use the endian stuff available as part of the standard.
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
