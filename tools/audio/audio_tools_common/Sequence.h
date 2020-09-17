#pragma once

#include "Track.h"

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

struct Track;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents an entire sequence (music or a sound) to be played by the sequencer
//------------------------------------------------------------------------------------------------------------------------------------------
struct Sequence {
    // An unknown field read from the Williams Module File (.WMD file).
    // Its purpose is unknown because it is never used and I can't determine any pattern to its value.
    // Preserving for only diff purposes against original .WMD files!
    uint16_t unknownWmdField;

    // All of the tracks in the sequence
    std::vector<Track> tracks;

    void readFromJson(const rapidjson::Value& jsonRoot) noexcept;
    void writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept;
    void readFromWmdFile(InputStream& in) THROWS;
    void writeToWmdFile(OutputStream& out) const THROWS;
};

END_NAMESPACE(AudioTools)
