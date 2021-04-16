#include "Sequence.h"

#include "InputStream.h"
#include "JsonUtils.h"
#include "OutputStream.h"
#include "Track.h"
#include "WmdFileTypes.h"

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read an entire sequence from json
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::readFromJson(const rapidjson::Value& jsonRoot) noexcept {
    // Sequence properties
    unknownWmdField = JsonUtils::clampedGetOrDefault<uint16_t>(jsonRoot, "unknownWmdField", 0);

    // Read all tracks
    tracks.clear();

    if (const rapidjson::Value* const pTracksArray = JsonUtils::tryGetArray(jsonRoot, "tracks")) {
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
void Sequence::readFromWmdFile(InputStream& in) THROWS {
    // Read the sequence header which tells us how many tracks there are
    WmdSequenceHdr seqHdr = {};
    in.read(seqHdr);
    seqHdr.endianCorrect();

    // Preserve this field for diff purposes against original .WMD files
    unknownWmdField = seqHdr.unknownField;

    // Read all the tracks in the sequence
    tracks.clear();

    for (uint32_t trackIdx = 0; trackIdx < seqHdr.numTracks; ++trackIdx) {
        tracks.emplace_back().readFromWmdFile(in);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write an entire sequence from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::writeToWmdFile(OutputStream& out) const THROWS {
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
        track.writeToWmdFile(out);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the indexes for all the patches used in the sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::getPatchesUsed(std::set<uint16_t>& patchIndexes) const noexcept {
    for (const Track& track : tracks) {
        track.getPatchesUsed(patchIndexes);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remap the patch indexes used in this sequence; the key/value pairs given specify the old and new patch indexes
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::remapPatches(const std::map<uint16_t, uint16_t>& oldAndNewPatchIndexes) noexcept {
    for (Track& track : tracks) {
        track.remapPatches(oldAndNewPatchIndexes);
    }
}

END_NAMESPACE(AudioTools)
