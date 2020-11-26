#include "Module.h"

#include "Asserts.h"
#include "InputStream.h"
#include "JsonUtils.h"
#include "OutputStream.h"
#include "Sequence.h"
#include "WmdFileTypes.h"

#include <cstdio>

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the module from a json document
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::readFromJson(const rapidjson::Document& doc) THROWS {
    // Read basic module properties
    if (!doc.IsObject())
        throw "Module root must be a json object!";
    
    maxActiveSequences = JsonUtils::clampedGetOrDefault<uint8_t>(doc, "maxActiveSequences", 1);
    maxActiveTracks = JsonUtils::clampedGetOrDefault<uint8_t>(doc, "maxActiveTracks", 1);
    maxGatesPerSeq = JsonUtils::clampedGetOrDefault<uint8_t>(doc, "maxGatesPerSeq", 1);
    maxItersPerSeq = JsonUtils::clampedGetOrDefault<uint8_t>(doc, "maxItersPerSeq", 1);
    maxCallbacks = JsonUtils::clampedGetOrDefault<uint8_t>(doc, "maxCallbacks", 1);
    
    // Read the PSX sound driver patch group
    if (const auto patchGroupIter = doc.FindMember("psxPatchGroup"); patchGroupIter != doc.MemberEnd()) {
        const rapidjson::Value& patchGroupObj = patchGroupIter->value;
        psxPatchGroup.readFromJson(patchGroupObj);
    }
    
    // Read all sequences
    sequences.clear();

    if (const rapidjson::Value* const pSequencesArray = JsonUtils::tryGetArray(doc, "sequences")) {
        for (rapidjson::SizeType i = 0; i < pSequencesArray->Size(); ++i) {
            const rapidjson::Value& sequenceObj = (*pSequencesArray)[i];
            Sequence& sequence = sequences.emplace_back();
            sequence.readFromJson(sequenceObj);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the entire module to a json document
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::writeToJson(rapidjson::Document& doc) const noexcept {
    // Add basic module properties
    ASSERT(doc.IsObject());
    rapidjson::Document::AllocatorType& jsonAlloc = doc.GetAllocator();

    doc.AddMember("maxActiveSequences", maxActiveSequences, jsonAlloc);
    doc.AddMember("maxActiveTracks", maxActiveTracks, jsonAlloc);
    doc.AddMember("maxGatesPerSeq", maxGatesPerSeq, jsonAlloc);
    doc.AddMember("maxItersPerSeq", maxItersPerSeq, jsonAlloc);
    doc.AddMember("maxCallbacks", maxCallbacks, jsonAlloc);

    // Write the PSX driver patch group
    {
        rapidjson::Value patchGroupObj(rapidjson::kObjectType);
        psxPatchGroup.writeToJson(patchGroupObj, jsonAlloc);
        doc.AddMember("psxPatchGroup", patchGroupObj, jsonAlloc);
    }
    
    // Write all the sequences
    {
        rapidjson::Value sequencesArray(rapidjson::kArrayType);
        
        for (const Sequence& sequence : sequences) {
            rapidjson::Value sequenceObj(rapidjson::kObjectType);
            sequence.writeToJson(sequenceObj, jsonAlloc);
            sequencesArray.PushBack(sequenceObj, jsonAlloc);
        }
        
        doc.AddMember("sequences", sequencesArray, jsonAlloc);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the entire module from the specified .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::readFromWmdFile(InputStream& in) THROWS {
    // Read the header for the module file, verify correct and save it's basic info
    WmdModuleHdr moduleHdr = {};
    in.read(moduleHdr);
    moduleHdr.endianCorrect();

    if (moduleHdr.moduleId != WMD_MODULE_ID)
        throw "Bad .WMD file ID!";

    if (moduleHdr.moduleVersion != WMD_VERSION)
        throw "Bad .WMD file version!";
    
    this->maxActiveSequences = moduleHdr.maxActiveSequences;
    this->maxActiveTracks = moduleHdr.maxActiveTracks;
    this->maxGatesPerSeq = moduleHdr.maxGatesPerSeq;
    this->maxItersPerSeq = moduleHdr.maxItersPerSeq;
    this->maxCallbacks = moduleHdr.maxCallbacks;

    // Read all patch groups
    for (uint32_t patchGrpIdx = 0; patchGrpIdx < moduleHdr.numPatchGroups; ++patchGrpIdx) {
        // Read the header for the patch group
        WmdPatchGroupHdr patchGroupHdr = {};
        in.read(patchGroupHdr);
        patchGroupHdr.endianCorrect();

        // If it's a PlayStation format patch group read it, otherwise skip
        if (patchGroupHdr.driverId == WmdSoundDriverId::PSX) {
            psxPatchGroup = {};
            psxPatchGroup.readFromWmdFile(in, patchGroupHdr);
        } else {
            std::printf("Warning: skipping unsupported format patch group with driver id '%u'!\n", (unsigned) patchGroupHdr.driverId);
            skipReadingWmdPatchGroup(in, patchGroupHdr);
        }
    }

    // Read all sequences
    sequences.clear();
    
    for (uint32_t seqIdx = 0; seqIdx < moduleHdr.numSequences; ++seqIdx) {
        sequences.emplace_back().readFromWmdFile(in);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the entire module to the specified .WMD file.
//
//  Notes:
//      (1) Just writes the actual .WMD data, does not pad out to 2,048 byte multiples like the .WMD files on the PSX Doom disc.
//      (2) If some parts of the module are invalidly configured, writing may fail.
//          The output from this process should always be a valid .WMD file.
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::writeToWmdFile(OutputStream& out) const THROWS {
    // Make up the module header, endian correct and serialize it
    {
        WmdModuleHdr hdr = {};
        hdr.moduleId = WMD_MODULE_ID;
        hdr.moduleVersion = WMD_VERSION;

        if (sequences.size() > UINT16_MAX)
            throw "Too many sequences for a .WMD file!";

        hdr.numSequences = (uint16_t) sequences.size();
        hdr.numPatchGroups = 1; // Just the PSX patch group to be written
        hdr.maxActiveSequences = maxActiveSequences;
        hdr.maxActiveTracks = maxActiveTracks;
        hdr.maxGatesPerSeq = maxGatesPerSeq;
        hdr.maxItersPerSeq = maxItersPerSeq;
        hdr.maxCallbacks = maxCallbacks;

        hdr.endianCorrect();
        out.write(hdr);
    }

    // Write the PSX driver patch group
    psxPatchGroup.writeToWmdFile(out);

    // Write all the sequences
    for (const Sequence& sequence : sequences) {
        sequence.writeToWmdFile(out);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears all data in the module to the default values
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::resetToDefault() noexcept {
    // These are the settings in DOOMSND.WMD used by PlayStation Doom and Final Doom
    maxActiveSequences = 26;
    maxActiveTracks = 25;
    maxGatesPerSeq = 2;
    maxItersPerSeq = 2;
    maxCallbacks = 2;

    // Default the patch group and clear all sequences
    psxPatchGroup.resetToDefault();
    sequences.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ccopy patch samples (specified by index) from this module into the specified module.
// The indexes of the copied data structures in the given destination module are returned.
// Invalid patch sample indexes are ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::copyPatchSamples(
    const std::set<uint16_t>& srcPatchSampleIndexes,
    Module& intoModule,
    std::map<uint16_t, uint16_t>& oldAndNewSampleIndexes
) const noexcept {
    const std::vector<PsxPatchSample>& srcPatchSamples = psxPatchGroup.patchSamples;
    std::vector<PsxPatchSample>& dstPatchSamples = intoModule.psxPatchGroup.patchSamples;

    for (const uint16_t srcSampleIdx : srcPatchSampleIndexes) {
        if (srcSampleIdx < srcPatchSamples.size()) {
            const PsxPatchSample& srcPatchSample = srcPatchSamples[srcSampleIdx];
            const uint16_t dstPatchSampleIdx = (uint16_t) dstPatchSamples.size();
            dstPatchSamples.push_back(srcPatchSample);
            oldAndNewSampleIndexes[srcSampleIdx] = dstPatchSampleIdx;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copy patch voices (specified by index) from this module into the specified module.
// The indexes of the copied data structures in the given destination module are returned.
// Invalid patch voice indexes are ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::copyPatchVoices(
    const std::set<uint16_t>& srcPatchVoiceIndexes,
    Module& intoModule,
    std::map<uint16_t, uint16_t>& oldAndNewPatchVoiceIndexes,
    std::map<uint16_t, uint16_t>& oldAndNewPatchSampleIndexes
) const noexcept {
    // First copy the raw patch voices into the destination module and make a note of the referenced samples we'll need to copy
    const std::vector<PsxPatchVoice>& srcPatchVoices = psxPatchGroup.patchVoices;
    std::vector<PsxPatchVoice>& dstPatchVoices = intoModule.psxPatchGroup.patchVoices;

    const size_t dstVoicesStartIdx = dstPatchVoices.size();
    std::set<uint16_t> srcPatchSampleIndexes;

    for (const uint16_t srcVoiceIdx : srcPatchVoiceIndexes) {
        if (srcVoiceIdx < srcPatchVoices.size()) {
            const PsxPatchVoice& srcPatchVoice = srcPatchVoices[srcVoiceIdx];
            const uint16_t dstPatchVoiceIdx = (uint16_t) dstPatchVoices.size();
            dstPatchVoices.push_back(srcPatchVoice);
            oldAndNewPatchVoiceIndexes[srcVoiceIdx] = dstPatchVoiceIdx;
            srcPatchSampleIndexes.insert(srcPatchVoice.sampleIdx);
        }
    }

    // Next copy the samples to the destination module
    copyPatchSamples(srcPatchSampleIndexes, intoModule, oldAndNewPatchSampleIndexes);

    // Fixup the patch sample indexes in the destination module
    for (size_t dstVoiceIdx = dstVoicesStartIdx; dstVoiceIdx < dstPatchVoices.size(); ++dstVoiceIdx) {
        PsxPatchVoice& dstPatchVoice = dstPatchVoices[dstVoiceIdx];
        auto sampleRemapIter = oldAndNewPatchSampleIndexes.find(dstPatchVoice.sampleIdx);

        if (sampleRemapIter != oldAndNewPatchSampleIndexes.end()) {
            dstPatchVoice.sampleIdx = sampleRemapIter->second;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copy patches (specified by index) from this module into the specified module.
// The indexes of the copied data structures in the given destination module are returned.
// Invalid patch indexes are ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::copyPatches(
    const std::set<uint16_t>& srcPatchIndexes,
    Module& intoModule,
    std::map<uint16_t, uint16_t>& oldAndNewPatchIndexes,
    std::map<uint16_t, uint16_t>& oldAndNewPatchVoiceIndexes,
    std::map<uint16_t, uint16_t>& oldAndNewPatchSampleIndexes
) const noexcept {
    // First copy the raw patches into the destination module and make a note of the referenced patch voices we'll need to copy
    const std::vector<PsxPatch>& srcPatches = psxPatchGroup.patches;
    std::vector<PsxPatch>& dstPatches = intoModule.psxPatchGroup.patches;

    const size_t dstPatchesStartIdx = dstPatches.size();
    std::set<uint16_t> srcPatchVoiceIndexes;

    for (const uint16_t srcPatchIdx : srcPatchIndexes) {
        if (srcPatchIdx < srcPatches.size()) {
            const PsxPatch& srcPatch = srcPatches[srcPatchIdx];
            const uint16_t dstPatchIdx = (uint16_t) dstPatches.size();
            dstPatches.push_back(srcPatch);
            oldAndNewPatchIndexes[srcPatchIdx] = dstPatchIdx;

            for (uint16_t patchVoiceIdx = 0; patchVoiceIdx < srcPatch.numVoices; ++patchVoiceIdx) {
                srcPatchVoiceIndexes.insert(srcPatch.firstVoiceIdx + patchVoiceIdx);
            }
        }
    }

    // Next copy the patch voices to the destination module
    copyPatchVoices(
        srcPatchVoiceIndexes,
        intoModule,
        oldAndNewPatchVoiceIndexes,
        oldAndNewPatchSampleIndexes
    );

    // Fixup the patch voice indexes in the destination module
    for (size_t dstPatchIdx = dstPatchesStartIdx; dstPatchIdx < dstPatches.size(); ++dstPatchIdx) {
        PsxPatch& dstPatch = dstPatches[dstPatchIdx];
        auto voiceRemapIter = oldAndNewPatchVoiceIndexes.find(dstPatch.firstVoiceIdx);

        if (voiceRemapIter != oldAndNewPatchVoiceIndexes.end()) {
            dstPatch.firstVoiceIdx = voiceRemapIter->second;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copy sequences (specified by index) from this module into the specified module.
// The indexes of the copied data structures in the given destination module are returned.
// Invalid sequence indexes are ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::copySequences(
    const std::set<uint16_t>& srcSequenceIndexes,
    Module& intoModule,
    std::map<uint16_t, uint16_t>& oldAndNewSequenceIndexes,
    std::map<uint16_t, uint16_t>& oldAndNewPatchIndexes,
    std::map<uint16_t, uint16_t>& oldAndNewPatchVoiceIndexes,
    std::map<uint16_t, uint16_t>& oldAndNewPatchSampleIndexes
) const noexcept {
    // First copy the raw sequences into the destination module and make a note of the referenced patches we'll need to copy
    const std::vector<Sequence>& srcSequences = sequences;
    std::vector<Sequence>& dstSequences = intoModule.sequences;

    const size_t dstSequencesStartIdx = dstSequences.size();
    std::set<uint16_t> srcPatchIndexes;

    for (const uint16_t srcSequenceIdx : srcSequenceIndexes) {
        if (srcSequenceIdx < srcSequences.size()) {
            const Sequence& srcSequence = srcSequences[srcSequenceIdx];
            const uint16_t dstSequenceIdx = (uint16_t) dstSequences.size();
            dstSequences.push_back(srcSequence);
            oldAndNewSequenceIndexes[srcSequenceIdx] = dstSequenceIdx;
            srcSequence.getPatchesUsed(srcPatchIndexes);
        }
    }

    // Next copy the patches to the destination module
    copyPatches(
        srcPatchIndexes,
        intoModule,
        oldAndNewPatchIndexes,
        oldAndNewPatchVoiceIndexes,
        oldAndNewPatchSampleIndexes
    );

    // Remap all patches in the destination sequences so that they use the patches we copied into the destination module
    for (size_t dstSequenceIdx = dstSequencesStartIdx; dstSequenceIdx < dstSequences.size(); ++dstSequenceIdx) {
        Sequence& dstSequence = dstSequences[dstSequenceIdx];
        dstSequence.remapPatches(oldAndNewPatchIndexes);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skip the data in a .WMD file for a patch group of an unknown type
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::skipReadingWmdPatchGroup(InputStream& in, const WmdPatchGroupHdr& patchGroupHdr) THROWS {
    if (patchGroupHdr.loadFlags & LOAD_PATCHES) {
        in.skipBytes((size_t) patchGroupHdr.numPatches * patchGroupHdr.patchSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_PATCH_VOICES) {
        in.skipBytes((size_t) patchGroupHdr.numPatchVoices * patchGroupHdr.patchVoiceSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_PATCH_SAMPLES) {
        in.skipBytes((size_t) patchGroupHdr.numPatchSamples * patchGroupHdr.patchSampleSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_DRUM_PATCHES) {
        in.skipBytes((size_t) patchGroupHdr.numDrumPatches * patchGroupHdr.drumPatchSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_EXTRA_DATA) {
        in.skipBytes(patchGroupHdr.extraDataSize);
    }
}

END_NAMESPACE(AudioTools)
