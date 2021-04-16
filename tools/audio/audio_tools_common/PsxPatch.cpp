#include "PsxPatch.h"

#include "InputStream.h"
#include "JsonUtils.h"
#include "OutputStream.h"
#include "WmdFileTypes.h"

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver patch from json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatch::readFromJson(const rapidjson::Value& jsonRoot) THROWS {
    firstVoiceIdx = JsonUtils::clampedGetOrDefault<uint16_t>(jsonRoot, "firstVoiceIdx", 0);
    numVoices = JsonUtils::clampedGetOrDefault<uint16_t>(jsonRoot, "numVoices", 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver patch to json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatch::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    jsonRoot.AddMember("firstVoiceIdx", firstVoiceIdx, jsonAlloc);
    jsonRoot.AddMember("numVoices", numVoices, jsonAlloc);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver patch from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatch::readFromWmdFile(InputStream& in) THROWS {
    WmdPsxPatch wmdPatch = {};
    in.read(wmdPatch);
    wmdPatch.endianCorrect();

    this->firstVoiceIdx = wmdPatch.firstVoiceIdx;
    this->numVoices = wmdPatch.numVoices;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver patch to a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatch::writeToWmdFile(OutputStream& out) const THROWS {
    WmdPsxPatch psxPatch = {};
    psxPatch.numVoices = numVoices;
    psxPatch.firstVoiceIdx = firstVoiceIdx;

    psxPatch.endianCorrect();
    out.write(psxPatch);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the indexes for all the patch voices used in the patch
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatch::getPatchVoicesUsed(std::set<uint16_t>& patchVoiceIndexes) const noexcept {
    for (uint16_t i = 0; i < numVoices; ++i) {
        patchVoiceIndexes.insert(firstVoiceIdx + i);
    }
}

END_NAMESPACE(AudioTools)
