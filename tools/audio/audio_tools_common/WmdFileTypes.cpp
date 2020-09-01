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
