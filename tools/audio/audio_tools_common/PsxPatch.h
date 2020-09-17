#pragma once

#include "Macros.h"

#include <cstdint>
#include <rapidjson/document.h>

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a patch/instrument: this is a collection of voices triggered in unison
//------------------------------------------------------------------------------------------------------------------------------------------
struct PsxPatch {
    uint16_t    firstVoiceIdx;      // Index of the first patch voice for the patch. Other patch voices follow contiguously in the voices list.
    uint16_t    numVoices;          // How many voices to use for the patch

    void readFromJson(const rapidjson::Value& jsonRoot) THROWS;
    void writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept;
    void readFromWmdFile(InputStream& in) THROWS;
    void writeToWmdFile(OutputStream& out) const THROWS;
};

END_NAMESPACE(AudioTools)
