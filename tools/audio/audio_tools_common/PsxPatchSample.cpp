#include "PsxPatchSample.h"

#include "InputStream.h"
#include "JsonUtils.h"
#include "OutputStream.h"
#include "WmdFileTypes.h"

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver patch sample from json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchSample::readFromJson(const rapidjson::Value& jsonRoot) THROWS {
    size = JsonUtils::clampedGetOrDefault<uint32_t>(jsonRoot, "size", 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver patch sample to json
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchSample::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    jsonRoot.AddMember("size", size, jsonAlloc);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver patch sample from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchSample::readFromWmd(InputStream& in) THROWS {
    WmdPsxPatchSample wmdSample = {};
    in.read(wmdSample);
    wmdSample.endianCorrect();

    this->size = wmdSample.size;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver patch sample to a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchSample::writeToWmd(OutputStream& out, const uint32_t wmdOffsetField) const THROWS {
    WmdPsxPatchSample psxSample = {};
    psxSample.offset = wmdOffsetField;
    psxSample.size = size;
    psxSample.spuAddr = 0;

    psxSample.endianCorrect();
    out.write(psxSample);
}

END_NAMESPACE(AudioTools)
