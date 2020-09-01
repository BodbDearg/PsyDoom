#include "Module.h"

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Skip the data in a .WMD file for a patch group of an unknown type
//------------------------------------------------------------------------------------------------------------------------------------------
static void skipWmdFilePatchGroup(const StreamReadFunc& reader, const WmdPatchGroupHdr& patchGroupHdr) noexcept(false) {
    if (patchGroupHdr.loadFlags & LOAD_PATCHES) {
        reader(nullptr, (size_t) patchGroupHdr.numPatches * patchGroupHdr.patchSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_PATCH_VOICES) {
        reader(nullptr, (size_t) patchGroupHdr.numPatchVoices * patchGroupHdr.patchVoiceSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_PATCH_SAMPLES) {
        reader(nullptr, (size_t) patchGroupHdr.numPatchSamples * patchGroupHdr.patchSampleSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_DRUM_PATCHES) {
        reader(nullptr, (size_t) patchGroupHdr.numDrumPatches * patchGroupHdr.drumPatchSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_EXTRA_DATA) {
        reader(nullptr, patchGroupHdr.extraDataSize);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver format patch group from the .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
static void readPsxWmdFilePatchGroup(
    const StreamReadFunc& reader,
    const WmdPatchGroupHdr& patchGroupHdr,
    PsxPatchGroup& patchGroup
) noexcept(false) {
    // Save basic patch group properties
    patchGroup = {};
    patchGroup.hwVoiceLimit = patchGroupHdr.hwVoiceLimit;

    // Load all patches
    if (patchGroupHdr.loadFlags & LOAD_PATCHES) {
        for (uint32_t i = 0; i < patchGroupHdr.numPatches; ++i) {
            WmdPsxPatch wmdPatch = {};
            reader(&wmdPatch, sizeof(WmdPsxPatch));
            wmdPatch.endianCorrect();

            PsxPatch& patch = patchGroup.patches.emplace_back();
            patch.firstVoiceIdx = wmdPatch.firstVoiceIdx;
            patch.numVoices = wmdPatch.numVoices;
        }
    }

    // Load all voices
    if (patchGroupHdr.loadFlags & LOAD_PATCH_VOICES) {
        for (uint32_t i = 0; i < patchGroupHdr.numPatchVoices; ++i) {
            WmdPsxPatchVoice wmdVoice = {};
            reader(&wmdVoice, sizeof(WmdPsxPatchVoice));
            wmdVoice.endianCorrect();

            PsxPatchVoice& voice = patchGroup.patchVoices.emplace_back();
            voice.sampleIdx = wmdVoice.sampleIdx;
            voice.volume = wmdVoice.volume;
            voice.pan = wmdVoice.pan;
            voice.baseNote = wmdVoice.baseNote;
            voice.baseNoteFrac = wmdVoice.baseNoteFrac;
            voice.noteMin = wmdVoice.noteMin;
            voice.noteMax = wmdVoice.noteMax;
            voice.pitchstepDown = wmdVoice.pitchstepDown;
            voice.pitchstepUp = wmdVoice.pitchstepUp;
            voice.priority = wmdVoice.priority;
            voice.adsrBits = (uint32_t) wmdVoice.adsr1 | ((uint32_t) wmdVoice.adsr2 << 16);
        }
    }

    // Load all patch samples
    if (patchGroupHdr.loadFlags & LOAD_PATCH_SAMPLES) {
        for (uint32_t i = 0; i < patchGroupHdr.numPatchSamples; ++i) {
            WmdPsxPatchSample wmdSample = {};
            reader(&wmdSample, sizeof(WmdPsxPatchSample));
            wmdSample.endianCorrect();

            PsxPatchSample& sample = patchGroup.patchSamples.emplace_back();
            sample.size = wmdSample.size;
        }
    }

    // Not supporting 'Drum Patches' as they are not used by PlayStation Doom; just skip over these if they are found:
    if (patchGroupHdr.loadFlags & LOAD_DRUM_PATCHES) {
        if (patchGroupHdr.numDrumPatches > 0) {
            std::printf("Warning: skipping 'drum patches' for the PSX sound driver patch group! These are not supported.\n");
            reader(nullptr, (size_t) patchGroupHdr.numDrumPatches * patchGroupHdr.drumPatchSize);
        }
    }

    // Skip over any extra data: the PSX driver doesn't use this and it shouldn't be here
    if (patchGroupHdr.loadFlags & LOAD_EXTRA_DATA) {
        if (patchGroupHdr.extraDataSize > 0) {
            std::printf("Warning: skipping 'extra data' for the PSX sound driver patch group! This is not supported.\n");
            reader(nullptr, patchGroupHdr.extraDataSize);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the entire module from the specified .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::readFromWmd(const StreamReadFunc& reader) noexcept(false) {
    // Read the header for the module file, verify correct and save it's basic info
    WmdModuleHdr moduleHdr = {};
    reader(&moduleHdr, sizeof(WmdModuleHdr));
    moduleHdr.endianCorrect();

    if (moduleHdr.moduleId != WMD_MODULE_ID)
        throw std::exception("Bad .WMD file ID!");

    if (moduleHdr.moduleVersion != WMD_VERSION)
        throw std::exception("Bad .WMD file version!");

    this->maxActiveSequences = moduleHdr.maxActiveSequences;
    this->maxActiveTracks = moduleHdr.maxActiveTracks;
    this->maxGatesPerSeq = moduleHdr.maxGatesPerSeq;
    this->maxItersPerSeq = moduleHdr.maxItersPerSeq;
    this->maxCallbacks = moduleHdr.maxCallbacks;

    // Read all patch groups
    for (uint32_t patchGrpIdx = 0; patchGrpIdx < moduleHdr.numPatchGroups; ++patchGrpIdx) {
        // Read the header for the patch group
        WmdPatchGroupHdr patchGroupHdr = {};
        reader(&patchGroupHdr, sizeof(WmdPatchGroupHdr));
        patchGroupHdr.endianCorrect();

        // If it's a PlayStation format patch group read it, otherwise skip
        if (patchGroupHdr.driverId == WmdSoundDriverId::PSX) {
            readPsxWmdFilePatchGroup(reader, patchGroupHdr, psxPatchGroup);
        } else {
            std::printf("Warning: skipping unknown format patch group with driver id '%u'!\n", patchGroupHdr.driverId);
            skipWmdFilePatchGroup(reader, patchGroupHdr);
        }
    }
}
