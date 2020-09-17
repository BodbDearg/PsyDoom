#pragma once

#include "Macros.h"

#include <cstdint>
#include <rapidjson/document.h>

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds details for a sound sample used by a patch voice; just holds the size in bytes of the sound sample
//------------------------------------------------------------------------------------------------------------------------------------------
struct PsxPatchSample {
    uint32_t    size;

    void readFromJson(const rapidjson::Value& jsonRoot) THROWS;
    void writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept;
    void readFromWmdFile(InputStream& in) THROWS;
    void writeToWmdFile(OutputStream& out, const uint32_t wmdOffsetField = 0) const THROWS;
};

END_NAMESPACE(AudioTools)
