#include "PsxPatchGroup.h"

#include "AudioUtils.h"
#include "InputStream.h"
#include "JsonUtils.h"
#include "OutputStream.h"
#include "WmdFileTypes.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver patch group from json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchGroup::readFromJson(const rapidjson::Value& jsonRoot) THROWS {
    if (!jsonRoot.IsObject())
        throw "PSX Patch Group root must be a json object!";

    // Save basic patch group properties
    hwVoiceLimit = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "hwVoiceLimit", 1);

    // Load all patches, patch voices, and patch samples
    patches.clear();
    patchVoices.clear();
    patchSamples.clear();

    if (const rapidjson::Value* const pPatchesArray = JsonUtils::tryGetArray(jsonRoot, "patches")) {
        for (rapidjson::SizeType i = 0; i < pPatchesArray->Size(); ++i) {
            const rapidjson::Value& patchObj = (*pPatchesArray)[i];
            PsxPatch& patch = patches.emplace_back();
            patch.readFromJson(patchObj);
        }
    }

    if (const rapidjson::Value* const pPatchVoicesArray = JsonUtils::tryGetArray(jsonRoot, "patchVoices")) {
        for (rapidjson::SizeType i = 0; i < pPatchVoicesArray->Size(); ++i) {
            const rapidjson::Value& patchVoiceObj = (*pPatchVoicesArray)[i];
            PsxPatchVoice& patchVoice = patchVoices.emplace_back();
            patchVoice.readFromJson(patchVoiceObj);
        }
    }

    if (const rapidjson::Value* const pPatchSamplesArray = JsonUtils::tryGetArray(jsonRoot, "patchSamples")) {
        for (rapidjson::SizeType i = 0; i < pPatchSamplesArray->Size(); ++i) {
            const rapidjson::Value& patchSampleObj = (*pPatchSamplesArray)[i];
            PsxPatchSample& patchSample = patchSamples.emplace_back();
            patchSample.readFromJson(patchSampleObj);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver patch group to json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchGroup::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    // Group global properties
    jsonRoot.AddMember("hwVoiceLimit", hwVoiceLimit, jsonAlloc);

    // Save patch samples
    {
        rapidjson::Value patchSamplesArray(rapidjson::kArrayType);

        for (const PsxPatchSample& patchSample : patchSamples) {
            rapidjson::Value patchSampleObj(rapidjson::kObjectType);
            patchSample.writeToJson(patchSampleObj, jsonAlloc);
            patchSamplesArray.PushBack(patchSampleObj, jsonAlloc);
        }

        jsonRoot.AddMember("patchSamples", patchSamplesArray, jsonAlloc);
    }

    // Save patch voices
    {
        rapidjson::Value patchVoicesArray(rapidjson::kArrayType);

        for (const PsxPatchVoice& patchVoice : patchVoices) {
            rapidjson::Value patchVoiceObj(rapidjson::kObjectType);
            patchVoice.writeToJson(patchVoiceObj, jsonAlloc);
            patchVoicesArray.PushBack(patchVoiceObj, jsonAlloc);
        }

        jsonRoot.AddMember("patchVoices", patchVoicesArray, jsonAlloc);
    }

    // Save patches
    {
        rapidjson::Value patchesArray(rapidjson::kArrayType);

        for (const PsxPatch& patch : patches) {
            rapidjson::Value patchObj(rapidjson::kObjectType);
            patch.writeToJson(patchObj, jsonAlloc);
            patchesArray.PushBack(patchObj, jsonAlloc);
        }

        jsonRoot.AddMember("patches", patchesArray, jsonAlloc);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver patch group from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchGroup::readFromWmdFile(InputStream& in, const WmdPatchGroupHdr& hdr) THROWS {
    // Save basic patch group properties
    hwVoiceLimit = hdr.hwVoiceLimit;

    // Load all patches, patch voices, and patch samples
    patches.clear();
    patchVoices.clear();
    patchSamples.clear();

    if (hdr.loadFlags & LOAD_PATCHES) {
        for (uint32_t i = 0; i < hdr.numPatches; ++i) {
            patches.emplace_back().readFromWmdFile(in);
        }
    }

    if (hdr.loadFlags & LOAD_PATCH_VOICES) {
        for (uint32_t i = 0; i < hdr.numPatchVoices; ++i) {
            patchVoices.emplace_back().readFromWmdFile(in);
        }
    }

    if (hdr.loadFlags & LOAD_PATCH_SAMPLES) {
        for (uint32_t i = 0; i < hdr.numPatchSamples; ++i) {
            patchSamples.emplace_back().readFromWmdFile(in);
        }
    }

    // Not supporting 'Drum Patches' as they are not used by PlayStation Doom. Just skip over these if they are found:
    if (hdr.loadFlags & LOAD_DRUM_PATCHES) {
        if (hdr.numDrumPatches > 0) {
            std::printf("Warning: skipping 'drum patches' for the PSX sound driver patch group! These are not supported.\n");
            in.skipBytes((size_t) hdr.numDrumPatches * hdr.drumPatchSize);
        }
    }

    // Skip over any extra data; the PSX driver doesn't use this and it shouldn't be there:
    if (hdr.loadFlags & LOAD_EXTRA_DATA) {
        if (hdr.extraDataSize > 0) {
            std::printf("Warning: skipping 'extra data' for the PSX sound driver patch group! This is not supported.\n");
            in.skipBytes(hdr.extraDataSize);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver patch group to a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchGroup::writeToWmdFile(OutputStream& out) const THROWS {
    // Firstly makeup the header, endian correct and write
    {
        WmdPatchGroupHdr groupHdr = {};
        groupHdr.loadFlags = (WmdPatchGroupLoadFlags)(LOAD_PATCHES | LOAD_PATCH_VOICES | LOAD_PATCH_SAMPLES | LOAD_DRUM_PATCHES);   // Note: add the drum flag so our diff with original game .WMDs is the same
        groupHdr.driverId = WmdSoundDriverId::PSX;
        groupHdr.hwVoiceLimit = hwVoiceLimit;

        if (patches.size() > UINT16_MAX)
            throw "Too many patches in the PSX patch group for a .WMD file!";

        groupHdr.numPatches = (uint16_t) patches.size();
        groupHdr.patchSize = sizeof(WmdPsxPatch);

        if (patchVoices.size() > UINT16_MAX)
            throw "Too many patche voices in the PSX patch group for a .WMD file!";

        groupHdr.numPatchVoices = (uint16_t) patchVoices.size();
        groupHdr.patchVoiceSize = sizeof(WmdPsxPatchVoice);

        if (patchSamples.size() > UINT16_MAX)
            throw "Too many patch samples in the PSX patch group for a .WMD file!";

        groupHdr.numPatchSamples = (uint16_t) patchSamples.size();
        groupHdr.patchSampleSize = sizeof(WmdPsxPatchSample);

        // Note: drum patches are not supported but in the original .WMD file the size was set to '512'?
        groupHdr.numDrumPatches = 0;
        groupHdr.drumPatchSize = 512;
        groupHdr.extraDataSize = 0;

        groupHdr.endianCorrect();
        out.write(groupHdr);
    }

    // Write all patches, patch voices and patch samples.
    // Note that for patch samples I'm writing the value of the unused 'offset' field so that it is populated the same as original .WMD files.
    // This offset appears to be increased by the size of each sample in the module, and 2048 byte aligned.
    for (const PsxPatch& patch : patches) {
        patch.writeToWmdFile(out);
    }

    for (const PsxPatchVoice& patchVoice : patchVoices) {
        patchVoice.writeToWmdFile(out);
    }

    uint32_t sampleWmdOffsetField = 0;

    for (const PsxPatchSample& patchSample : patchSamples) {
        patchSample.writeToWmdFile(out, sampleWmdOffsetField);

        // Incrementing and 2048 byte aligning the value of this unused .WMD file field.
        // I only care about it because I want to generate .WMD files that match the originals exactly.
        // I suspect this may have been an offset into some sort of master .LCD or samples file used during development...
        sampleWmdOffsetField += patchSample.size;
        sampleWmdOffsetField += 2047;
        sampleWmdOffsetField /= 2048;
        sampleWmdOffsetField *= 2048;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears all data in the PlayStation patch group to the default values
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchGroup::resetToDefault() noexcept {
    hwVoiceLimit = 24;
    patchSamples.clear();
    patchVoices.clear();
    patches.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to determine the sample rate that a specified patch sample index is encoded in.
// There's no actual information for this in either the .LCD file or the .WMD file, however the .WMD does define a 'base note'.
//
// The 'base note' is the note at which the sample plays back at 44,100 Hz, and in the case of SFX I'm using the distance between that and
// the note that PSX Doom uses to play back all SFX at (A#3/Bb3, or note '58') in order to figure out the sample rate. For music it appears
// that the note '48.0' might be a close approximation for what a base note might be, so I'm using that in the music case.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t PsxPatchGroup::guessSampleRateForPatchSample(const uint32_t patchSampleIdx) const noexcept {
    // Look for a voice using this sample to get the 'base note' info and hence determine the 
    for (const PsxPatchVoice& voice : patchVoices) {
        // Ignore if this voice doesn't use this sample
        if (voice.sampleIdx != patchSampleIdx)
            continue;

        // Is this note SFX or a music note? In the PSX Doom .WMD all SFX voices have a priority of '100' whereas music voices have a priority of '128'.
        // This is a slightly brittle test but it does the job and saves us trawling through all the tracks to find SFX vs instrument voice usage...
        const bool bIsSfxSample = (voice.priority < 128);

        // Use the distance to the game's base note to figure out the sample rate
        constexpr double PSX_DOOM_SFX_BASE_NOTE = 58.0;
        constexpr double PSX_DOOM_MUS_BASE_NOTE = 48.0;
        constexpr double PSX_MAX_SAMPLE_RATE = 176400.0;

        const double gameBaseNote = (bIsSfxSample) ? PSX_DOOM_SFX_BASE_NOTE : PSX_DOOM_MUS_BASE_NOTE;
        const double voiceBaseNote = (double) voice.baseNote + (double) voice.baseNoteFrac / 256.0;
        const double sampleRate = AudioUtils::getNoteSampleRate(voiceBaseNote, 44100.0, gameBaseNote);
        const double sampleRateRounded = std::clamp(std::round(sampleRate), 1.0,  PSX_MAX_SAMPLE_RATE);

        return (uint32_t) sampleRateRounded;
    }

    // If we don't find a patch voice using the sample, then fallback to assuming it's at 11,050 Hz.
    // This is the sample rate used by a lot of PSX Doom sounds:
    return 11050;
}

END_NAMESPACE(AudioTools)
