#pragma once

#include "PsxPatchSample.h"
#include "PsxPatchVoice.h"
#include "PsxPatch.h"

#include <vector>

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

struct PsxPatch;
struct PsxPatchSample;
struct PsxPatchVoice;
struct WmdPatchGroupHdr;

//------------------------------------------------------------------------------------------------------------------------------------------
// Root container for all the patches and associated data structures for the PlayStation sound driver
//------------------------------------------------------------------------------------------------------------------------------------------
struct PsxPatchGroup {
    uint8_t                         hwVoiceLimit;       // How many hardware voices there are
    std::vector<PsxPatchSample>     patchSamples;       // Samples used by patch voices
    std::vector<PsxPatchVoice>      patchVoices;        // Individual voices in a patch
    std::vector<PsxPatch>           patches;            // Patches/instruments

    void readFromJson(const rapidjson::Value& jsonRoot) THROWS;
    void writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept;
    void readFromWmdFile(InputStream& in, const WmdPatchGroupHdr& hdr) THROWS;
    void writeToWmdFile(OutputStream& out) const THROWS;
    void resetToDefault() noexcept;
    uint32_t guessSampleRateForPatchSample(const uint32_t patchSampleIdx) const noexcept;
};

END_NAMESPACE(AudioTools)
