#include "Track.h"

#include "ByteVecOutputStream.h"
#include "InputStream.h"
#include "JsonUtils.h"
#include "MidiUtils.h"
#include "OutputStream.h"
#include "TrackCmd.h"
#include "WmdFileTypes.h"

#include <memory>

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read an entire track from json
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::readFromJson(const rapidjson::Value& jsonRoot) noexcept {
    // Read track global properties
    driverId = stringToSoundDriverId(JsonUtils::getOrDefault<const char*>(jsonRoot, "driverId", ""));
    soundClass = stringToSoundClass(JsonUtils::getOrDefault<const char*>(jsonRoot, "soundClass", ""));
    initPpq = JsonUtils::clampedGetOrDefault<uint16_t>(jsonRoot, "initPpq", 120);
    initQpm = JsonUtils::clampedGetOrDefault<uint16_t>(jsonRoot, "initQpm", 120);
    initPatchIdx = JsonUtils::clampedGetOrDefault<uint16_t>(jsonRoot, "initPatchIdx", 0);
    initPitchCntrl = JsonUtils::clampedGetOrDefault<int16_t>(jsonRoot, "initPitchCntrl", 0);
    initVolumeCntrl = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "initVolumeCntrl", 0x7Fu);
    initPanCntrl = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "initPanCntrl", 0x40u);
    initReverb = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "initReverb", 0);
    initMutegroupsMask = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "initMutegroupsMask", 0);
    maxVoices = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "maxVoices", 1);
    locStackSize = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "locStackSize", 1);
    priority = JsonUtils::clampedGetOrDefault<uint8_t>(jsonRoot, "priority", 0);

    // Read track labels
    labels.clear();

    if (const rapidjson::Value* const pLabelsArray = JsonUtils::tryGetArray(jsonRoot, "labels")) {
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

    if (const rapidjson::Value* const pCmdsArray = JsonUtils::tryGetArray(jsonRoot, "cmds")) {
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
void Track::readFromWmdFile(InputStream& in) THROWS {
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
        cmdByteOffsets.push_back(curCmdByteOffset + MidiUtils::getVarLenQuantLen(cmd.delayQnp));    // Note: labels skip the command delay time bytes
        curCmdByteOffset += cmd.readFromWmdFile(in);

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
void Track::writeToWmdFile(OutputStream& out) const THROWS {
    // Makeup a buffer and byte stream to write all the track data to
    ByteVecOutputStream trackData;
    trackData.getBytes().reserve(cmds.size() * 8);

    // Write all of the track commands to the stream and save the byte offset of each command
    std::vector<uint32_t> cmdOffsets;
    cmdOffsets.reserve(cmds.size());

    for (const TrackCmd& cmd : cmds) {
        cmdOffsets.push_back((uint32_t) trackData.tell() + MidiUtils::getVarLenQuantLen(cmd.delayQnp));     // Note: labels skip the command delay time bytes
        cmd.writeToWmdFile(trackData);
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
// Get the indexes for all the patches used in the track
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::getPatchesUsed(std::set<uint16_t>& patchIndexes) const noexcept {
    patchIndexes.insert(initPatchIdx);

    for (const TrackCmd& cmd : cmds) {
        // Note: 'PatchMod' is both unused and unimplemented by PSX Doom, hence not checking for those command types here.
        // We don't know what the arguments for that command actually mean since it is completely unused and unimplemented in Doom (cannot infer usage).
        if (cmd.type == WmdTrackCmdType::PatchChg) {
            patchIndexes.insert((uint16_t) cmd.arg1);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remap the patch indexes used in this track; the key/value pairs given specify the old and new patch indexes
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::remapPatches(const std::map<uint16_t, uint16_t>& oldAndNewPatchIndexes) noexcept {
    // Remap the init patch
    if (const auto remapIter = oldAndNewPatchIndexes.find(initPatchIdx); remapIter != oldAndNewPatchIndexes.end()) {
        initPatchIdx = remapIter->second;
    }

    // Remap patches referenced in track commands
    for (TrackCmd& cmd : cmds) {
        // Note: 'PatchMod' is both unused and unimplemented by PSX Doom, hence not checking for those command types here.
        // We don't know what the arguments for that command actually mean since it is completely unused and unimplemented in Doom (cannot infer usage).
        if (cmd.type == WmdTrackCmdType::PatchChg) {
            const uint16_t oldPatchIdx = (uint16_t) cmd.arg1;

            if (const auto remapIter = oldAndNewPatchIndexes.find(oldPatchIdx); remapIter != oldAndNewPatchIndexes.end()) {
                cmd.arg1 = (uint16_t) remapIter->second;
            }
        }
    }
}

END_NAMESPACE(AudioTools)
