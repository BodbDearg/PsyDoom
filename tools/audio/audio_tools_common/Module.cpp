#include "Module.h"

#include "Asserts.h"
#include "ByteVecOutputStream.h"
#include "InputStream.h"

#include <algorithm>
#include <cstdio>
#include <string>

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: get or default a json field's value
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static T jsonGetOrDefault(const rapidjson::Value& jsonObj, const char* const fieldName, const T& defaultVal) noexcept {
    if (!jsonObj.IsObject())
        return defaultVal;
        
    auto iter = jsonObj.FindMember(fieldName);
    
    if (iter == jsonObj.MemberEnd())
        return defaultVal;
    
    const rapidjson::Value& fieldVal = iter->value;
    return (fieldVal.Is<T>()) ? fieldVal.Get<T>() : defaultVal;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: get integers of various types from a json object and clamp if out of range
//------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t jsonGetOrDefaultClampedUint8(const rapidjson::Value& jsonObj, const char* const fieldName, const uint8_t defaultVal) noexcept {
    const int64_t val64 = jsonGetOrDefault<int64_t>(jsonObj, fieldName, defaultVal);
    return (uint8_t) std::clamp<int64_t>(val64, 0, UINT8_MAX);
}

static uint16_t jsonGetOrDefaultClampedUint16(const rapidjson::Value& jsonObj, const char* const fieldName, const uint16_t defaultVal) noexcept {
    const int64_t val64 = jsonGetOrDefault<int64_t>(jsonObj, fieldName, defaultVal);
    return (uint16_t) std::clamp<int64_t>(val64, 0, UINT16_MAX);
}

static uint32_t jsonGetOrDefaultClampedUint32(const rapidjson::Value& jsonObj, const char* const fieldName, const uint32_t defaultVal) noexcept {
    const int64_t val64 = jsonGetOrDefault<int64_t>(jsonObj, fieldName, defaultVal);
    return (uint32_t) std::clamp<int64_t>(val64, 0, UINT32_MAX);
}

static int32_t jsonGetOrDefaultClampedInt32(const rapidjson::Value& jsonObj, const char* const fieldName, const uint32_t defaultVal) noexcept {
    const int64_t val64 = jsonGetOrDefault<int64_t>(jsonObj, fieldName, defaultVal);
    return (int32_t) std::clamp<int64_t>(val64, INT32_MIN, INT32_MAX);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: get a child json array by name (readonly) or null
//------------------------------------------------------------------------------------------------------------------------------------------
static const rapidjson::Value* jsonGetArrayOrNull(const rapidjson::Value& jsonObj, const char* const fieldName) noexcept {
    if (jsonObj.IsObject()) {
        if (auto iter = jsonObj.FindMember(fieldName); iter != jsonObj.MemberEnd()) {
            const rapidjson::Value& fieldVal = iter->value;
            return (fieldVal.IsArray()) ? &fieldVal : nullptr;
        }
    }
    
    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver format patch group (and child data structures) from json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::readFromJson(const rapidjson::Value& jsonRoot) THROWS {
    sampleIdx = jsonGetOrDefaultClampedUint16(jsonRoot, "sampleIdx", 0);
    volume = jsonGetOrDefaultClampedUint8(jsonRoot, "volume", 0);
    pan = jsonGetOrDefaultClampedUint8(jsonRoot, "pan", 0);
    reverb = jsonGetOrDefaultClampedUint8(jsonRoot, "reverb", 0);
    baseNote = jsonGetOrDefaultClampedUint8(jsonRoot, "baseNote", 0);
    baseNoteFrac = jsonGetOrDefaultClampedUint8(jsonRoot, "baseNoteFrac", 0);
    noteMin = jsonGetOrDefaultClampedUint8(jsonRoot, "noteMin", 0);
    noteMax = jsonGetOrDefaultClampedUint8(jsonRoot, "noteMax", 0);
    pitchstepDown = jsonGetOrDefaultClampedUint8(jsonRoot, "pitchstepDown", 0);
    pitchstepUp = jsonGetOrDefaultClampedUint8(jsonRoot, "pitchstepUp", 0);
    priority = jsonGetOrDefaultClampedUint8(jsonRoot, "priority", 0);
    adsr.sustainLevel = (uint32_t) std::clamp<int64_t>(jsonGetOrDefault<int64_t>(jsonRoot, "adsr_sustainLevel", 0), 0, 15);
    adsr.decayShift = (uint32_t) std::clamp<int64_t>(jsonGetOrDefault<int64_t>(jsonRoot, "adsr_decayShift", 0), 0, 15);
    adsr.attackStep = (uint32_t) std::clamp<int64_t>(jsonGetOrDefault<int64_t>(jsonRoot, "adsr_attackStep", 0), 0, 3);
    adsr.attackShift = (uint32_t) std::clamp<int64_t>(jsonGetOrDefault<int64_t>(jsonRoot, "adsr_attackShift", 0), 0, 31);
    adsr.bAttackExp = jsonGetOrDefault<bool>(jsonRoot, "adsr_attackExponential", false);
    adsr.releaseShift = (uint32_t) std::clamp<int64_t>(jsonGetOrDefault<int64_t>(jsonRoot, "adsr_releaseShift", 0), 0, 31);
    adsr.bReleaseExp = jsonGetOrDefault<bool>(jsonRoot, "adsr_releaseExponential", false);
    adsr.sustainStep = (uint32_t) std::clamp<int64_t>(jsonGetOrDefault<int64_t>(jsonRoot, "adsr_sustainStep", 0), 0, 3);
    adsr.sustainShift = (uint32_t) std::clamp<int64_t>(jsonGetOrDefault<int64_t>(jsonRoot, "adsr_sustainShift", 0), 0, 31);
    adsr.bSustainDec = jsonGetOrDefault<bool>(jsonRoot, "adsr_sustainDecrease", false);
    adsr.bSustainExp = jsonGetOrDefault<bool>(jsonRoot, "adsr_sustainExponential", false);
}

void PsxPatchSample::readFromJson(const rapidjson::Value& jsonRoot) THROWS {
    size = jsonGetOrDefaultClampedUint32(jsonRoot, "size", 0);
}

void PsxPatch::readFromJson(const rapidjson::Value& jsonRoot) THROWS {
    firstVoiceIdx = jsonGetOrDefaultClampedUint16(jsonRoot, "firstVoiceIdx", 0);
    numVoices = jsonGetOrDefaultClampedUint16(jsonRoot, "numVoices", 0);
}

void PsxPatchGroup::readFromJson(const rapidjson::Value& jsonRoot) THROWS {
    if (!jsonRoot.IsObject())
        throw "PSX Patch Group root must be a json object!";

    // Save basic patch group properties
    hwVoiceLimit = jsonGetOrDefaultClampedUint8(jsonRoot, "hwVoiceLimit", 1);
    
    // Load all patches, patch voices, and patch samples
    patches.clear();
    patchVoices.clear();
    patchSamples.clear();
    
    if (const rapidjson::Value* const pPatchesArray = jsonGetArrayOrNull(jsonRoot, "patches")) {
        for (rapidjson::SizeType i = 0; i < pPatchesArray->Size(); ++i) {
            const rapidjson::Value& patchObj = (*pPatchesArray)[i];
            PsxPatch& patch = patches.emplace_back();
            patch.readFromJson(patchObj);
        }
    }
    
    if (const rapidjson::Value* const pPatchVoicesArray = jsonGetArrayOrNull(jsonRoot, "patchVoices")) {
        for (rapidjson::SizeType i = 0; i < pPatchVoicesArray->Size(); ++i) {
            const rapidjson::Value& patchVoiceObj = (*pPatchVoicesArray)[i];
            PsxPatchVoice& patchVoice = patchVoices.emplace_back();
            patchVoice.readFromJson(patchVoiceObj);
        }
    }
    
    if (const rapidjson::Value* const pPatchSamplesArray = jsonGetArrayOrNull(jsonRoot, "patchSamples")) {
        for (rapidjson::SizeType i = 0; i < pPatchSamplesArray->Size(); ++i) {
            const rapidjson::Value& patchSampleObj = (*pPatchSamplesArray)[i];
            PsxPatchSample& patchSample = patchSamples.emplace_back();
            patchSample.readFromJson(patchSampleObj);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver format patch group (and child data structures) to json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    jsonRoot.AddMember("sampleIdx", sampleIdx, jsonAlloc);
    jsonRoot.AddMember("volume", volume, jsonAlloc);
    jsonRoot.AddMember("pan", pan, jsonAlloc);
    jsonRoot.AddMember("reverb", reverb, jsonAlloc);
    jsonRoot.AddMember("baseNote", baseNote, jsonAlloc);
    jsonRoot.AddMember("baseNoteFrac", baseNoteFrac, jsonAlloc);
    jsonRoot.AddMember("noteMin", noteMin, jsonAlloc);
    jsonRoot.AddMember("noteMax", noteMax, jsonAlloc);
    jsonRoot.AddMember("pitchstepDown", pitchstepDown, jsonAlloc);
    jsonRoot.AddMember("pitchstepUp", pitchstepUp, jsonAlloc);
    jsonRoot.AddMember("priority", priority, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainLevel", adsr.sustainLevel, jsonAlloc);
    jsonRoot.AddMember("adsr_decayShift", adsr.decayShift, jsonAlloc);
    jsonRoot.AddMember("adsr_attackStep", adsr.attackStep, jsonAlloc);
    jsonRoot.AddMember("adsr_attackShift", adsr.attackShift, jsonAlloc);
    jsonRoot.AddMember("adsr_attackExponential", (bool) adsr.bAttackExp, jsonAlloc);
    jsonRoot.AddMember("adsr_releaseShift", adsr.releaseShift, jsonAlloc);
    jsonRoot.AddMember("adsr_releaseExponential", (bool) adsr.bReleaseExp, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainStep", adsr.sustainStep, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainShift", adsr.sustainShift, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainDecrease", (bool) adsr.bSustainDec, jsonAlloc);
    jsonRoot.AddMember("adsr_sustainExponential", (bool) adsr.bSustainExp, jsonAlloc);
}

void PsxPatchSample::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    jsonRoot.AddMember("size", size, jsonAlloc);
}

void PsxPatch::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    jsonRoot.AddMember("firstVoiceIdx", firstVoiceIdx, jsonAlloc);
    jsonRoot.AddMember("numVoices", numVoices, jsonAlloc);
}

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
// Read a PlayStation sound driver format patch group (and child data structures) from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::readFromWmd(InputStream& in) THROWS {
    WmdPsxPatchVoice wmdVoice = {};
    in.read(wmdVoice);
    wmdVoice.endianCorrect();

    this->sampleIdx = wmdVoice.sampleIdx;
    this->volume = wmdVoice.volume;
    this->pan = wmdVoice.pan;
    this->reverb = wmdVoice.reverb;
    this->baseNote = wmdVoice.baseNote;
    this->baseNoteFrac = wmdVoice.baseNoteFrac;
    this->noteMin = wmdVoice.noteMin;
    this->noteMax = wmdVoice.noteMax;
    this->pitchstepDown = wmdVoice.pitchstepDown;
    this->pitchstepUp = wmdVoice.pitchstepUp;
    this->priority = wmdVoice.priority;
    this->adsrBits = (uint32_t) wmdVoice.adsr1 | ((uint32_t) wmdVoice.adsr2 << 16);
}

void PsxPatchSample::readFromWmd(InputStream& in) THROWS {
    WmdPsxPatchSample wmdSample = {};
    in.read(wmdSample);
    wmdSample.endianCorrect();

    this->size = wmdSample.size;
}

void PsxPatch::readFromWmd(InputStream& in) THROWS {
    WmdPsxPatch wmdPatch = {};
    in.read(wmdPatch);
    wmdPatch.endianCorrect();

    this->firstVoiceIdx = wmdPatch.firstVoiceIdx;
    this->numVoices = wmdPatch.numVoices;
}

void PsxPatchGroup::readFromWmd(InputStream& in, const WmdPatchGroupHdr& hdr) THROWS {
    // Save basic patch group properties
    hwVoiceLimit = hdr.hwVoiceLimit;

    // Load all patches, patch voices, and patch samples
    patches.clear();
    patchVoices.clear();
    patchSamples.clear();
    
    if (hdr.loadFlags & LOAD_PATCHES) {
        for (uint32_t i = 0; i < hdr.numPatches; ++i) {
            patches.emplace_back().readFromWmd(in);
        }
    }

    if (hdr.loadFlags & LOAD_PATCH_VOICES) {
        for (uint32_t i = 0; i < hdr.numPatchVoices; ++i) {
            patchVoices.emplace_back().readFromWmd(in);
        }
    }

    if (hdr.loadFlags & LOAD_PATCH_SAMPLES) {
        for (uint32_t i = 0; i < hdr.numPatchSamples; ++i) {
            patchSamples.emplace_back().readFromWmd(in);
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
// Write a PlayStation sound driver format patch group (and child data structures) to a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::writeToWmd(OutputStream& out) const THROWS {
    WmdPsxPatchVoice psxVoice = {};
    psxVoice.priority = priority;
    psxVoice.reverb = reverb;
    psxVoice.volume = volume;
    psxVoice.pan = pan;
    psxVoice.baseNote = baseNote;
    psxVoice.baseNoteFrac = baseNoteFrac;
    psxVoice.noteMin = noteMin;
    psxVoice.noteMax = noteMax;
    psxVoice.pitchstepDown = pitchstepDown;
    psxVoice.pitchstepUp = pitchstepUp;
    psxVoice.sampleIdx = sampleIdx;
    psxVoice.adsr1 = (uint16_t)(adsrBits);
    psxVoice.adsr2 = (uint16_t)(adsrBits >> 16);

    psxVoice.endianCorrect();
    out.write(psxVoice);
}

void PsxPatchSample::writeToWmd(OutputStream& out, const uint32_t wmdOffsetField) const THROWS {
    WmdPsxPatchSample psxSample = {};
    psxSample.offset = wmdOffsetField;
    psxSample.size = size;
    psxSample.spuAddr = 0;

    psxSample.endianCorrect();
    out.write(psxSample);
}

void PsxPatch::writeToWmd(OutputStream& out) const THROWS {
    WmdPsxPatch psxPatch = {};
    psxPatch.numVoices = numVoices;
    psxPatch.firstVoiceIdx = firstVoiceIdx;
    
    psxPatch.endianCorrect();
    out.write(psxPatch);
}

void PsxPatchGroup::writeToWmd(OutputStream& out) const THROWS {
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
        patch.writeToWmd(out);
    }

    for (const PsxPatchVoice& patchVoice : patchVoices) {
        patchVoice.writeToWmd(out);
    }

    uint32_t sampleWmdOffsetField = 0;

    for (const PsxPatchSample& patchSample : patchSamples) {
        patchSample.writeToWmd(out, sampleWmdOffsetField);

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
// Read a single track command from json
//------------------------------------------------------------------------------------------------------------------------------------------
void TrackCmd::readFromJson(const rapidjson::Value& jsonRoot) noexcept {
    type = stringToTrackCmdType(jsonGetOrDefault<const char*>(jsonRoot, "type", ""));
    delayQnp = jsonGetOrDefaultClampedUint32(jsonRoot, "delayQnp", 0);
    arg1 = jsonGetOrDefaultClampedInt32(jsonRoot, "arg1", 0);
    arg2 = jsonGetOrDefaultClampedInt32(jsonRoot, "arg2", 0);
    arg3 = jsonGetOrDefaultClampedInt32(jsonRoot, "arg3", 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a single track command to json
//------------------------------------------------------------------------------------------------------------------------------------------
void TrackCmd::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    jsonRoot.AddMember("type", rapidjson::StringRef(toString(type)), jsonAlloc);
    jsonRoot.AddMember("delayQnp", delayQnp, jsonAlloc);
    
    const int32_t numCmdArgs = getNumWmdTrackCmdArgs(type);
    
    if (numCmdArgs >= 1) {
        jsonRoot.AddMember("arg1", arg1, jsonAlloc);
    }
    
    if (numCmdArgs >= 2) {
        jsonRoot.AddMember("arg2", arg2, jsonAlloc);
    }
    
    if (numCmdArgs >= 3) {
        jsonRoot.AddMember("arg3", arg3, jsonAlloc);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a single track command from a .WMD file and return how many bytes were read
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t TrackCmd::readFromWmd(InputStream& in) THROWS {
    // First read the delay (in quarter note parts) until the command
    uint32_t numBytesRead = Module::readVarLenQuant(in, delayQnp);

    // Next grab the command id
    in.read(type);
    numBytesRead += sizeof(WmdTrackCmdType);

    // Read the data for the command into this stack buffer (which is more than big enough)
    std::byte cmdData[8] = {};
    const uint32_t cmdSize = getWmdTrackCmdSize(type);

    if (cmdSize <= 0)
        throw "Unexpected command in the track's command stream!";

    const uint32_t cmdDataSize = cmdSize - 1;

    if (cmdDataSize > 8)
        throw "Bad track command payload size!";

    in.readBytes(cmdData, cmdDataSize);
    numBytesRead += cmdDataSize;

    // Handle parsing that data out into arguments, default them all for now
    switch (type) {
        case WmdTrackCmdType::SeqRet:
        case WmdTrackCmdType::SeqEnd:
        case WmdTrackCmdType::TrkRet:
        case WmdTrackCmdType::TrkEnd:
        case WmdTrackCmdType::NullEvent:
            break;

        case WmdTrackCmdType::PatchMod:
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
            arg1 = (uint8_t) cmdData[0];
            break;

        case WmdTrackCmdType::PatchChg:
        case WmdTrackCmdType::SeqTempo:
        case WmdTrackCmdType::TrkTempo:
            arg1 = ((uint16_t) cmdData[0]) | ((uint16_t) cmdData[1] << 8);
            break;

        case WmdTrackCmdType::PitchMod:
        case WmdTrackCmdType::SeqGosub:
        case WmdTrackCmdType::TrkGosub:
        case WmdTrackCmdType::SeqJump:
        case WmdTrackCmdType::TrkJump:
            arg1 = (int16_t)((uint16_t) cmdData[0] | ((uint16_t) cmdData[1] << 8));
            break;

        case WmdTrackCmdType::NoteOn:
        case WmdTrackCmdType::WriteIterBox:
            arg1 = (uint8_t) cmdData[0];
            arg2 = (uint8_t) cmdData[1];
            break;

        case WmdTrackCmdType::StatusMark:
            arg1 = (uint8_t) cmdData[0];
            arg2 = (int16_t)((uint16_t) cmdData[1] | ((uint16_t) cmdData[2] << 8));
            break;

        case WmdTrackCmdType::GateJump:
        case WmdTrackCmdType::IterJump:
            arg1 = (uint8_t) cmdData[0];
            arg2 = (uint8_t) cmdData[1];
            arg3 = (int16_t)((uint16_t) cmdData[2] | ((uint16_t) cmdData[3] << 8));
            break;

        // These commands (or unknown commands) should never be in the command stream!
        case WmdTrackCmdType::DriverInit:
        case WmdTrackCmdType::DriverExit:
        case WmdTrackCmdType::DriverEntry1:
        case WmdTrackCmdType::DriverEntry2:
        case WmdTrackCmdType::DriverEntry3:
        case WmdTrackCmdType::TrkOff:
        case WmdTrackCmdType::TrkMute:
        default:
            throw "Unexpected command type in the track's command stream!";
    }

    return numBytesRead;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a single track command to a .WMD file and return how many bytes were written
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t TrackCmd::writeToWmd(OutputStream& out) const THROWS {
    // First write the delay (in quarter note parts) until the command
    uint32_t numBytesWritten = Module::writeVarLenQuant(out, delayQnp);

    // Next Write the command id
    out.write(type);
    numBytesWritten += sizeof(WmdTrackCmdType);

    // Put the data for the command into this stack buffer, which is more than big enough.
    // First verify however that it is a command that we can write to the .WMD:
    std::byte cmdData[8] = {};
    const uint32_t cmdSize = getWmdTrackCmdSize(type);

    if (cmdSize <= 0)
        throw "Unexpected command in the track's command stream which cannot be serialized to a .WMD file!";

    const uint32_t cmdDataSize = cmdSize - 1;

    if (cmdDataSize > 8)
        throw "Bad track command payload size!";

    // Convert the command data to bytes
    switch (type) {
        case WmdTrackCmdType::SeqRet:
        case WmdTrackCmdType::SeqEnd:
        case WmdTrackCmdType::TrkRet:
        case WmdTrackCmdType::TrkEnd:
        case WmdTrackCmdType::NullEvent:
            break;

        case WmdTrackCmdType::PatchMod:
        case WmdTrackCmdType::ZeroMod:
        case WmdTrackCmdType::ModuMod:
        case WmdTrackCmdType::VolumeMod:
        case WmdTrackCmdType::PanMod:
        case WmdTrackCmdType::PedalMod:
        case WmdTrackCmdType::ReverbMod:
        case WmdTrackCmdType::ChorusMod:
        case WmdTrackCmdType::NoteOff:
        case WmdTrackCmdType::ResetGates:
        case WmdTrackCmdType::ResetIters: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte) arg1;
        }   break;

        case WmdTrackCmdType::PatchChg:
        case WmdTrackCmdType::SeqTempo:
        case WmdTrackCmdType::TrkTempo: {
            if ((arg1 < 0) || (arg1 > UINT16_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte)(arg1);
            cmdData[1] = (std::byte)(arg1 >> 8);
        }   break;

        case WmdTrackCmdType::PitchMod:
        case WmdTrackCmdType::SeqGosub:
        case WmdTrackCmdType::TrkGosub:
        case WmdTrackCmdType::SeqJump:
        case WmdTrackCmdType::TrkJump: {
            if ((arg1 < INT16_MIN) || (arg1 > INT16_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte)((uint32_t) arg1);
            cmdData[1] = (std::byte)((uint32_t) arg1 >> 8);
        }   break;

        case WmdTrackCmdType::NoteOn:
        case WmdTrackCmdType::WriteIterBox: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            if ((arg2 < 0) || (arg2 > UINT8_MAX))
                throw "Track cmd has arg2 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte) arg1;
            cmdData[1] = (std::byte) arg2;
        }   break;

        case WmdTrackCmdType::StatusMark: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            if ((arg2 < INT16_MIN) || (arg2 > INT16_MAX))
                throw "Track cmd has arg2 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte)(arg1);
            cmdData[1] = (std::byte)((uint32_t) arg2);
            cmdData[2] = (std::byte)((uint32_t) arg2 >> 8);
        }   break;

        case WmdTrackCmdType::GateJump:
        case WmdTrackCmdType::IterJump: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            if ((arg2 < 0) || (arg2 > UINT8_MAX))
                throw "Track cmd has arg2 which is out of range for a .WMD file!";

            if ((arg3 < INT16_MIN) || (arg3 > INT16_MAX))
                throw "Track cmd has arg3 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte)(arg1);
            cmdData[1] = (std::byte)(arg2);
            cmdData[2] = (std::byte)((uint32_t) arg3);
            cmdData[3] = (std::byte)((uint32_t) arg3 >> 8);
        }   break;

        // These commands (or unknown commands) should never be in the command stream!
        case WmdTrackCmdType::DriverInit:
        case WmdTrackCmdType::DriverExit:
        case WmdTrackCmdType::DriverEntry1:
        case WmdTrackCmdType::DriverEntry2:
        case WmdTrackCmdType::DriverEntry3:
        case WmdTrackCmdType::TrkOff:
        case WmdTrackCmdType::TrkMute:
        default:
            throw "Unexpected command in the track's command stream which cannot be serialized to a .WMD file!";
    }

    // Serialize the bytes for the command data
    out.writeBytes(cmdData, cmdDataSize);
    numBytesWritten += cmdDataSize;
    return numBytesWritten;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read an entire track from json
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::readFromJson(const rapidjson::Value& jsonRoot) noexcept {
    // Read track global properties
    driverId = stringToSoundDriverId(jsonGetOrDefault<const char*>(jsonRoot, "driverId", ""));
    soundClass = stringToSoundClass(jsonGetOrDefault<const char*>(jsonRoot, "soundClass", ""));
    initPpq = jsonGetOrDefaultClampedUint16(jsonRoot, "initPpq", 120);
    initQpm = jsonGetOrDefaultClampedUint16(jsonRoot, "initQpm", 120);
    initPatchIdx = jsonGetOrDefaultClampedUint16(jsonRoot, "initPatchIdx", 0);
    initPitchCntrl = jsonGetOrDefaultClampedUint16(jsonRoot, "initPitchCntrl", 0);
    initVolumeCntrl = jsonGetOrDefaultClampedUint8(jsonRoot, "initVolumeCntrl", 0x7Fu);
    initPanCntrl = jsonGetOrDefaultClampedUint8(jsonRoot, "initPanCntrl", 0x40u);
    initReverb = jsonGetOrDefaultClampedUint8(jsonRoot, "initReverb", 0);
    initMutegroupsMask = jsonGetOrDefaultClampedUint8(jsonRoot, "initMutegroupsMask", 0);
    maxVoices = jsonGetOrDefaultClampedUint8(jsonRoot, "maxVoices", 1);
    locStackSize = jsonGetOrDefaultClampedUint8(jsonRoot, "locStackSize", 1);
    priority = jsonGetOrDefaultClampedUint8(jsonRoot, "priority", 0);

    // Read track labels
    labels.clear();

    if (const rapidjson::Value* const pLabelsArray = jsonGetArrayOrNull(jsonRoot, "labels")) {
        for (rapidjson::SizeType i = 0; i < pLabelsArray->Size(); ++i) {
            const rapidjson::Value& labelVal = (*pLabelsArray)[i];
            
            if (labelVal.IsNumber()) {
                const uint32_t labelCmdIdx = (uint32_t) std::clamp<double>(labelVal.Get<double>(), 0, UINT32_MAX);
                labels.push_back(labelCmdIdx);
            }
        }
    }

    // Read track commands
    cmds.clear();

    if (const rapidjson::Value* const pCmdsArray = jsonGetArrayOrNull(jsonRoot, "cmds")) {
        for (rapidjson::SizeType i = 0; i < pCmdsArray->Size(); ++i) {
            const rapidjson::Value& trackCmdObj = (*pCmdsArray)[i];
            TrackCmd& trackCmd = cmds.emplace_back();
            trackCmd.readFromJson(trackCmdObj);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write an entire track to json
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    // Write track global properties
    jsonRoot.AddMember("driverId", rapidjson::StringRef(toString(driverId)), jsonAlloc);
    jsonRoot.AddMember("soundClass", rapidjson::StringRef(toString(soundClass)), jsonAlloc);
    jsonRoot.AddMember("initPpq", initPpq, jsonAlloc);
    jsonRoot.AddMember("initQpm", initQpm, jsonAlloc);
    jsonRoot.AddMember("initPatchIdx", initPatchIdx, jsonAlloc);
    jsonRoot.AddMember("initPitchCntrl", initPitchCntrl, jsonAlloc);
    jsonRoot.AddMember("initVolumeCntrl", initVolumeCntrl, jsonAlloc);
    jsonRoot.AddMember("initPanCntrl", initPanCntrl, jsonAlloc);
    jsonRoot.AddMember("initReverb", initReverb, jsonAlloc);
    jsonRoot.AddMember("initMutegroupsMask", initMutegroupsMask, jsonAlloc);
    jsonRoot.AddMember("maxVoices", maxVoices, jsonAlloc);
    jsonRoot.AddMember("locStackSize", locStackSize, jsonAlloc);
    jsonRoot.AddMember("priority", priority, jsonAlloc);
    
    // Write track labels
    {
        rapidjson::Value labelsArray(rapidjson::kArrayType);
        
        for (uint32_t cmdIdx : labels) {
            labelsArray.PushBack(cmdIdx, jsonAlloc);
        }
        
        jsonRoot.AddMember("labels", labelsArray, jsonAlloc);
    }
    
    // Write track commands
    {
        rapidjson::Value cmdsArray(rapidjson::kArrayType);
        
        for (const TrackCmd& cmd : cmds) {
            rapidjson::Value cmdObj(rapidjson::kObjectType);
            cmd.writeToJson(cmdObj, jsonAlloc);
            cmdsArray.PushBack(cmdObj, jsonAlloc);
        }
        
        jsonRoot.AddMember("cmds", cmdsArray, jsonAlloc);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read an entire track from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::readFromWmd(InputStream& in) THROWS {
    // First get the header and save basic track properties
    WmdTrackHdr trackHdr = {};
    in.read(trackHdr);
    trackHdr.endianCorrect();

    this->driverId = trackHdr.driverId;
    this->soundClass = trackHdr.soundClass;
    this->initPpq = trackHdr.initPpq;
    this->initQpm = trackHdr.initQpm;
    this->initPatchIdx = trackHdr.initPatchIdx;
    this->initPitchCntrl = trackHdr.initPitchCntrl;
    this->initVolumeCntrl = trackHdr.initVolumeCntrl;
    this->initPanCntrl = trackHdr.initPanCntrl;
    this->initReverb = trackHdr.initReverb;
    this->initMutegroupsMask = trackHdr.initMutegroupsMask;
    this->maxVoices = trackHdr.maxVoices;
    this->locStackSize = trackHdr.locStackSize;
    this->priority = trackHdr.priority;

    // Read raw track labels: in the .WMD these will be in terms of raw byte offsets.
    // We will convert these to command indexes below so the command stream can be edited easier.
    std::unique_ptr<uint32_t[]> labelByteOffsets(new uint32_t[trackHdr.numLabels]);
    in.readArray(labelByteOffsets.get(), trackHdr.numLabels);

    // Store the byte offsets of each command in this list as we read them
    std::vector<uint32_t> cmdByteOffsets;
    cmdByteOffsets.reserve(trackHdr.cmdStreamSize);

    // Read each track command
    cmds.clear();
    cmds.reserve(trackHdr.cmdStreamSize);
    
    uint32_t curCmdByteOffset = 0;

    while (curCmdByteOffset < trackHdr.cmdStreamSize) {
        TrackCmd& cmd = cmds.emplace_back();
        cmdByteOffsets.push_back(curCmdByteOffset + Module::getVarLenQuantLen(cmd.delayQnp));   // Note: labels skip the command delay time bytes
        curCmdByteOffset += cmd.readFromWmd(in);

        // Are we past the end of the stream, if so that is an error and indicates a corrupted sequence:
        if (curCmdByteOffset > trackHdr.cmdStreamSize)
            throw "Unexpected end of track command stream! Track data may be corrupt!";
    }

    // Populate the labels list with the index of the command to jump to.
    // Need to convert from byte offsets to command indexes:
    labels.clear();
    labels.reserve(trackHdr.numLabels);

    for (uint32_t i = 0; i < trackHdr.numLabels; ++i) {
        const uint32_t labelByteOffset = labelByteOffsets[i];
        const auto cmdIter = std::lower_bound(cmdByteOffsets.begin(), cmdByteOffsets.end(), labelByteOffset);

        if (cmdIter != cmdByteOffsets.end()) {
            const uint32_t labelCmdIdx = (uint32_t)(cmdIter - cmdByteOffsets.begin());
            labels.push_back(labelCmdIdx);
        } else {
            throw "Invalid byte offset for track label! Track data may be corrupt!";
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write an entire track to a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::writeToWmd(OutputStream& out) const THROWS {
    // Makeup a buffer and byte stream to write all the track data to
    ByteVecOutputStream trackData;
    trackData.getBytes().reserve(cmds.size() * 8);

    // Write all of the track commands to the stream and save the byte offset of each command
    std::vector<uint32_t> cmdOffsets;
    cmdOffsets.reserve(cmds.size());

    for (const TrackCmd& cmd : cmds) {
        cmdOffsets.push_back((uint32_t) trackData.tell() + Module::getVarLenQuantLen(cmd.delayQnp));   // Note: labels skip the command delay time bytes
        cmd.writeToWmd(trackData);
    }

    // Makeup the header for the track, endian correct and write to the file
    {
        WmdTrackHdr hdr = {};
        hdr.driverId = driverId;
        hdr.maxVoices = maxVoices;
        hdr.priority = priority;
        hdr.lockChannel = 0;
        hdr.soundClass = soundClass;
        hdr.initReverb = initReverb;
        hdr.initPatchIdx = initPatchIdx;
        hdr.initPitchCntrl = initPitchCntrl;
        hdr.initVolumeCntrl = initVolumeCntrl;
        hdr.initPanCntrl = initPanCntrl;
        hdr.locStackSize = locStackSize;
        hdr.initMutegroupsMask = initMutegroupsMask;
        hdr.initPpq = initPpq;
        hdr.initQpm = initQpm;
        
        if (labels.size() > UINT16_MAX)
            throw "Too many labels in a track for a .WMD file!";

        hdr.numLabels = (uint16_t) labels.size();
        hdr.cmdStreamSize = (uint32_t) trackData.tell();

        hdr.endianCorrect();
        out.write(hdr);
    }

    // Write the track labels to the file
    for (uint32_t cmdIdx : labels) {
        // Make sure the label is in range
        if (cmdIdx >= cmds.size())
            throw "Bad track label which cannot be serialized! References a track command that does not exist (out of range).";

        // Get the byte offset of the command and write
        const uint32_t cmdByteOffset = cmdOffsets[cmdIdx];
        out.write(cmdByteOffset);
    }

    // Write the track data to the file
    out.writeBytes(trackData.getBytes().data(), trackData.tell());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read an entire sequence from json
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::readFromJson(const rapidjson::Value& jsonRoot) noexcept {
    // Sequence properties
    unknownWmdField = jsonGetOrDefaultClampedUint16(jsonRoot, "unknownWmdField", 0);

    // Read all tracks
    tracks.clear();

    if (const rapidjson::Value* const pTracksArray = jsonGetArrayOrNull(jsonRoot, "tracks")) {
        for (rapidjson::SizeType i = 0; i < pTracksArray->Size(); ++i) {
            const rapidjson::Value& trackObj = (*pTracksArray)[i];
            Track& track = tracks.emplace_back();
            track.readFromJson(trackObj);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write an entire sequence to json
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    // Sequence properties
    jsonRoot.AddMember("unknownWmdField", unknownWmdField, jsonAlloc);
    
    // Write all tracks
    {
        rapidjson::Value tracksArray(rapidjson::kArrayType);
        
        for (const Track& track : tracks) {
            rapidjson::Value trackObj(rapidjson::kObjectType);
            track.writeToJson(trackObj, jsonAlloc);
            tracksArray.PushBack(trackObj, jsonAlloc);
        }
        
        jsonRoot.AddMember("tracks", tracksArray, jsonAlloc);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read an entire sequence from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::readFromWmd(InputStream& in) THROWS {
    // Read the sequence header which tells us how many tracks there are
    WmdSequenceHdr seqHdr = {};
    in.read(seqHdr);
    seqHdr.endianCorrect();

    // Preserve this field for diff purposes against original .WMD files
    unknownWmdField = seqHdr.unknownField;

    // Read all the tracks in the sequence
    tracks.clear();
    
    for (uint32_t trackIdx = 0; trackIdx < seqHdr.numTracks; ++trackIdx) {
        tracks.emplace_back().readFromWmd(in);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write an entire sequence from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::writeToWmd(OutputStream& out) const THROWS {
    // Write the header first
    {
        WmdSequenceHdr hdr = {};

        if (tracks.size() > UINT16_MAX)
            throw "Too many tracks in a sequence for a .WMD file!";

        hdr.numTracks = (uint16_t) tracks.size();
        hdr.unknownField = unknownWmdField;

        hdr.endianCorrect();
        out.write(hdr);
    }

    // Then write all the tracks
    for (const Track& track : tracks) {
        track.writeToWmd(out);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the module from a json document
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::readFromJson(const rapidjson::Document& doc) THROWS {
    // Read basic module properties
    if (!doc.IsObject())
        throw "Module root must be a json object!";
    
    maxActiveSequences = jsonGetOrDefaultClampedUint8(doc, "maxActiveSequences", 1);
    maxActiveTracks = jsonGetOrDefaultClampedUint8(doc, "maxActiveTracks", 1);
    maxGatesPerSeq = jsonGetOrDefaultClampedUint8(doc, "maxGatesPerSeq", 1);
    maxItersPerSeq = jsonGetOrDefaultClampedUint8(doc, "maxItersPerSeq", 1);
    maxCallbacks = jsonGetOrDefaultClampedUint8(doc, "maxCallbacks", 1);
    
    // Read the PSX sound driver patch group
    if (const auto patchGroupIter = doc.FindMember("psxPatchGroup"); patchGroupIter != doc.MemberEnd()) {
        const rapidjson::Value& patchGroupObj = patchGroupIter->value;
        psxPatchGroup.readFromJson(patchGroupObj);
    }
    
    // Read all sequences
    sequences.clear();

    if (const rapidjson::Value* const pSequencesArray = jsonGetArrayOrNull(doc, "sequences")) {
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
void Module::readFromWmd(InputStream& in) THROWS {
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
            psxPatchGroup.readFromWmd(in, patchGroupHdr);
        } else {
            std::printf("Warning: skipping unsupported format patch group with driver id '%u'!\n", (unsigned) patchGroupHdr.driverId);
            skipReadingWmdPatchGroup(in, patchGroupHdr);
        }
    }

    // Read all sequences
    sequences.clear();
    
    for (uint32_t seqIdx = 0; seqIdx < moduleHdr.numSequences; ++seqIdx) {
        sequences.emplace_back().readFromWmd(in);
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
void Module::writeToWmd(OutputStream& out) const THROWS {
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
    psxPatchGroup.writeToWmd(out);

    // Write all the sequences
    for (const Sequence& sequence : sequences) {
        sequence.writeToWmd(out);
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a variable length quantity from track data, similar to how certain data is encoded in the MIDI standard.
// Returns number of bytes read and the output value, which may be up to 5 bytes.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t Module::readVarLenQuant(InputStream& in, uint32_t& valueOut) THROWS {
    // Grab the first byte in the quantity
    uint8_t curByte = in.read<uint8_t>();
    uint32_t numBytesRead = 1;

    // The top bit set on each byte means there is another byte to follow.
    // Each byte can therefore only encode 7 bits, so we need 5 of them to encode 32-bits:
    uint32_t decodedVal = curByte & 0x7Fu;

    while (curByte & 0x80) {
        // Make room for more data
        decodedVal <<= 7;

        // Read the next byte and incorporate into the value
        curByte = in.read<uint8_t>();
        numBytesRead++;
        decodedVal |= (uint32_t) curByte & 0x7Fu;

        // Sanity check, there should only be at most 5 bytes!
        if (numBytesRead > 5)
            throw "Read VLQ: too many bytes! Quantity encoding is not valid!";
    }

    valueOut = decodedVal;
    return numBytesRead;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a variable length quantity to track data, similar to how certain data is encoded in the MIDI standard.
// Returns number of bytes written, which may be up to 5 bytes.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t Module::writeVarLenQuant(OutputStream& out, const uint32_t valueIn) THROWS {
    // Encode the given value into a stack buffer, writing a '0x80' bit flag whenever more bytes follow
    uint8_t encodedBytes[8];
    uint8_t* pEncodedByte = encodedBytes;

    {
        *pEncodedByte = (uint8_t)(valueIn & 0x7F);
        pEncodedByte++;
        uint32_t bitsLeftToEncode = valueIn >> 7;

        while (bitsLeftToEncode != 0) {
            *pEncodedByte = (uint8_t)(bitsLeftToEncode | 0x80);
            bitsLeftToEncode >>= 7;
            pEncodedByte++;
        }
    }
    
    // Write the encoded value to the given output stream.
    // Note that the ordering here gets reversed, so it winds up being read in the correct order.
    uint32_t numBytesWritten = 0;

    do {
        pEncodedByte--;
        out.write(*pEncodedByte);
        numBytesWritten++;
    } while (*pEncodedByte & 0x80);

    return numBytesWritten;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells how many bytes would be used to encode a 32-bit value using variable length (MIDI style) encoding.
// The returned answer will be between 1-5 bytes.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t Module::getVarLenQuantLen(const uint32_t valueIn) noexcept {
    uint32_t numEncodedBytes = 1;
    uint32_t bitsLeftToEncode = valueIn >> 7;

    while (bitsLeftToEncode != 0) {
        numEncodedBytes++;
        bitsLeftToEncode >>= 7;
    }

    return numEncodedBytes;
}
