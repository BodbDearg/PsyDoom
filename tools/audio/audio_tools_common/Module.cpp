#include "Module.h"

#include <algorithm>

using namespace AudioTools;

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver format patch group to a .json file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchGroup::writeToJson(rapidjson::Value& jsonRoot) const noexcept {
    // TODO...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a PlayStation sound driver format patch group (and child data structures) from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::readFromWmd(const StreamReadFunc& streamRead) noexcept(false) {
    WmdPsxPatchVoice wmdVoice = {};
    streamRead(&wmdVoice, sizeof(WmdPsxPatchVoice));
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

void PsxPatchSample::readFromWmd(const StreamReadFunc& streamRead) noexcept(false) {
    WmdPsxPatchSample wmdSample = {};
    streamRead(&wmdSample, sizeof(WmdPsxPatchSample));
    wmdSample.endianCorrect();

    this->size = wmdSample.size;
}

void PsxPatch::readFromWmd(const StreamReadFunc& streamRead) noexcept(false) {
    WmdPsxPatch wmdPatch = {};
    streamRead(&wmdPatch, sizeof(WmdPsxPatch));
    wmdPatch.endianCorrect();

    this->firstVoiceIdx = wmdPatch.firstVoiceIdx;
    this->numVoices = wmdPatch.numVoices;
}

void PsxPatchGroup::readFromWmd(const StreamReadFunc& streamRead, const WmdPatchGroupHdr& hdr) noexcept(false) {
    // Save basic patch group properties
    hwVoiceLimit = hdr.hwVoiceLimit;

    // Load all patches, patch voices, and patch samples
    if (hdr.loadFlags & LOAD_PATCHES) {
        for (uint32_t i = 0; i < hdr.numPatches; ++i) {
            patches.emplace_back().readFromWmd(streamRead);
        }
    }

    if (hdr.loadFlags & LOAD_PATCH_VOICES) {
        for (uint32_t i = 0; i < hdr.numPatchVoices; ++i) {
            patchVoices.emplace_back().readFromWmd(streamRead);
        }
    }

    if (hdr.loadFlags & LOAD_PATCH_SAMPLES) {
        for (uint32_t i = 0; i < hdr.numPatchSamples; ++i) {
            patchSamples.emplace_back().readFromWmd(streamRead);
        }
    }

    // Not supporting 'Drum Patches' as they are not used by PlayStation Doom. Just skip over these if they are found:
    if (hdr.loadFlags & LOAD_DRUM_PATCHES) {
        if (hdr.numDrumPatches > 0) {
            std::printf("Warning: skipping 'drum patches' for the PSX sound driver patch group! These are not supported.\n");
            streamRead(nullptr, (size_t) hdr.numDrumPatches * hdr.drumPatchSize);
        }
    }

    // Skip over any extra data; the PSX driver doesn't use this and it shouldn't be there:
    if (hdr.loadFlags & LOAD_EXTRA_DATA) {
        if (hdr.extraDataSize > 0) {
            std::printf("Warning: skipping 'extra data' for the PSX sound driver patch group! This is not supported.\n");
            streamRead(nullptr, hdr.extraDataSize);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a PlayStation sound driver format patch group (and child data structures) to a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxPatchVoice::writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false) {
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
    streamWrite(&psxVoice, sizeof(WmdPsxPatchVoice));
}

void PsxPatchSample::writeToWmd(const StreamWriteFunc& streamWrite, const uint32_t wmdOffsetField) const noexcept(false) {
    WmdPsxPatchSample psxSample = {};
    psxSample.offset = wmdOffsetField;
    psxSample.size = size;
    psxSample.spuAddr = 0;

    psxSample.endianCorrect();
    streamWrite(&psxSample, sizeof(WmdPsxPatchSample));
}

void PsxPatch::writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false) {
    WmdPsxPatch psxPatch = {};
    psxPatch.numVoices = numVoices;
    psxPatch.firstVoiceIdx = firstVoiceIdx;
    
    psxPatch.endianCorrect();
    streamWrite(&psxPatch, sizeof(WmdPsxPatch));
}

void PsxPatchGroup::writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false) {
    // Firstly makeup the header, endian correct and write
    {
        WmdPatchGroupHdr groupHdr = {};
        groupHdr.loadFlags = (WmdPatchGroupLoadFlags)(LOAD_PATCHES | LOAD_PATCH_VOICES | LOAD_PATCH_SAMPLES | LOAD_DRUM_PATCHES);   // Note: add the drum flag so our diff with original game .WMDs is the same
        groupHdr.driverId = WmdSoundDriverId::PSX;
        groupHdr.hwVoiceLimit = hwVoiceLimit;
        
        if (patches.size() > UINT16_MAX)
            throw std::exception("Too many patches in the PSX patch group for a .WMD file!");

        groupHdr.numPatches = (uint16_t) patches.size();
        groupHdr.patchSize = sizeof(WmdPsxPatch);

        if (patchVoices.size() > UINT16_MAX)
            throw std::exception("Too many patche voices in the PSX patch group for a .WMD file!");

        groupHdr.numPatchVoices = (uint16_t) patchVoices.size();
        groupHdr.patchVoiceSize = sizeof(WmdPsxPatchVoice);

        if (patchSamples.size() > UINT16_MAX)
            throw std::exception("Too many patch samples in the PSX patch group for a .WMD file!");

        groupHdr.numPatchSamples = (uint16_t) patchSamples.size();
        groupHdr.patchSampleSize = sizeof(WmdPsxPatchSample);

        // Note: drum patches are not supported but in the original .WMD file the size was set to '512'?
        groupHdr.numDrumPatches = 0;
        groupHdr.drumPatchSize = 512;
        groupHdr.extraDataSize = 0;

        groupHdr.endianCorrect();
        streamWrite(&groupHdr, sizeof(WmdPatchGroupHdr));
    }

    // Write all patches, patch voices and patch samples.
    // Note that for patch samples I'm writing the value of the unused 'offset' field so that it is populated the same as original .WMD files.
    // This offset appears to be increased by the size of each sample in the module, and 2048 byte aligned.
    for (const PsxPatch& patch : patches) {
        patch.writeToWmd(streamWrite);
    }

    for (const PsxPatchVoice& patchVoice : patchVoices) {
        patchVoice.writeToWmd(streamWrite);
    }

    uint32_t sampleWmdOffsetField = 0;

    for (const PsxPatchSample& patchSample : patchSamples) {
        patchSample.writeToWmd(streamWrite, sampleWmdOffsetField);

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
// Read a single track command from a .WMD file and return how many bytes were read
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t TrackCmd::readFromWmd(const StreamReadFunc& streamRead) noexcept(false) {
    // First read the delay (in quarter note parts) until the command
    uint32_t numBytesRead = Module::readVarLenQuant(streamRead, delayQnp);

    // Next grab the command id
    streamRead(&type, sizeof(WmdTrackCmdType));
    numBytesRead += sizeof(WmdTrackCmdType);

    // Read the data for the command into this stack buffer (which is more than big enough)
    std::byte cmdData[8] = {};
    const uint32_t cmdSize = getWmdTrackCmdSize(type);

    if (cmdSize <= 0)
        throw std::exception("Unexpected command in the track's command stream!");

    const uint32_t cmdDataSize = cmdSize - 1;

    if (cmdDataSize > 8)
        throw std::exception("Bad track command payload size!");

    streamRead(cmdData, cmdDataSize);
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
            throw std::exception("Unexpected command type in the track's command stream!");
    }

    return numBytesRead;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a single track command to a .WMD file and return how many bytes were written
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t TrackCmd::writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false) {
    // First write the delay (in quarter note parts) until the command
    uint32_t numBytesWritten = Module::writeVarLenQuant(streamWrite, delayQnp);

    // Next Write the command id
    streamWrite(&type, sizeof(WmdTrackCmdType));
    numBytesWritten += sizeof(WmdTrackCmdType);

    // Put the data for the command into this stack buffer, which is more than big enough.
    // First verify however that it is a command that we can write to the .WMD:
    std::byte cmdData[8] = {};
    const uint32_t cmdSize = getWmdTrackCmdSize(type);

    if (cmdSize <= 0)
        throw std::exception("Unexpected command in the track's command stream which cannot be serialized to a .WMD file!");

    const uint32_t cmdDataSize = cmdSize - 1;

    if (cmdDataSize > 8)
        throw std::exception("Bad track command payload size!");

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
                throw std::exception("Track cmd has arg1 which is out of range for a .WMD file!");

            cmdData[0] = (std::byte) arg1;
        }   break;

        case WmdTrackCmdType::PatchChg:
        case WmdTrackCmdType::SeqTempo:
        case WmdTrackCmdType::TrkTempo: {
            if ((arg1 < 0) || (arg1 > UINT16_MAX))
                throw std::exception("Track cmd has arg1 which is out of range for a .WMD file!");

            cmdData[0] = (std::byte)(arg1);
            cmdData[1] = (std::byte)(arg1 >> 8);
        }   break;

        case WmdTrackCmdType::PitchMod:
        case WmdTrackCmdType::SeqGosub:
        case WmdTrackCmdType::TrkGosub:
        case WmdTrackCmdType::SeqJump:
        case WmdTrackCmdType::TrkJump: {
            if ((arg1 < INT16_MIN) || (arg1 > INT16_MAX))
                throw std::exception("Track cmd has arg1 which is out of range for a .WMD file!");

            cmdData[0] = (std::byte)((uint32_t) arg1);
            cmdData[1] = (std::byte)((uint32_t) arg1 >> 8);
        }   break;

        case WmdTrackCmdType::NoteOn:
        case WmdTrackCmdType::WriteIterBox: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw std::exception("Track cmd has arg1 which is out of range for a .WMD file!");

            if ((arg2 < 0) || (arg2 > UINT8_MAX))
                throw std::exception("Track cmd has arg2 which is out of range for a .WMD file!");

            cmdData[0] = (std::byte) arg1;
            cmdData[1] = (std::byte) arg2;
        }   break;

        case WmdTrackCmdType::StatusMark: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw std::exception("Track cmd has arg1 which is out of range for a .WMD file!");

            if ((arg2 < INT16_MIN) || (arg2 > INT16_MAX))
                throw std::exception("Track cmd has arg2 which is out of range for a .WMD file!");

            cmdData[0] = (std::byte)(arg1);
            cmdData[1] = (std::byte)((uint32_t) arg2);
            cmdData[2] = (std::byte)((uint32_t) arg2 >> 8);
        }   break;

        case WmdTrackCmdType::GateJump:
        case WmdTrackCmdType::IterJump: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw std::exception("Track cmd has arg1 which is out of range for a .WMD file!");

            if ((arg2 < 0) || (arg2 > UINT8_MAX))
                throw std::exception("Track cmd has arg2 which is out of range for a .WMD file!");

            if ((arg3 < INT16_MIN) || (arg3 > INT16_MAX))
                throw std::exception("Track cmd has arg3 which is out of range for a .WMD file!");

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
            throw std::exception("Unexpected command in the track's command stream which cannot be serialized to a .WMD file!");
    }

    // Serialize the bytes for the command data
    streamWrite(cmdData, cmdDataSize);
    numBytesWritten += cmdDataSize;
    return numBytesWritten;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read an entire track from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::readFromWmd(const StreamReadFunc& streamRead) noexcept(false) {
    // First get the header and save basic track properties
    WmdTrackHdr trackHdr = {};
    streamRead(&trackHdr, sizeof(WmdTrackHdr));
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
    streamRead(labelByteOffsets.get(), sizeof(uint32_t) * trackHdr.numLabels);

    // Store the byte offsets of each command in this list as we read them
    std::vector<uint32_t> cmdByteOffsets;
    cmdByteOffsets.reserve(trackHdr.cmdStreamSize);

    // Read each track command
    uint32_t curCmdByteOffset = 0;

    while (curCmdByteOffset < trackHdr.cmdStreamSize) {
        TrackCmd& cmd = cmds.emplace_back();
        cmdByteOffsets.push_back(curCmdByteOffset + Module::getVarLenQuantLen(cmd.delayQnp));   // Note: labels skip the command delay time bytes
        curCmdByteOffset += cmd.readFromWmd(streamRead);

        // Are we past the end of the stream, if so that is an error and indicates a corrupted sequence:
        if (curCmdByteOffset > trackHdr.cmdStreamSize)
            throw std::exception("Unexpected end of track command stream! Track data may be corrupt!");
    }

    // Populate the labels list with the index of the command to jump to.
    // Need to convert from byte offsets to command indexes:
    labels.reserve(trackHdr.numLabels);

    for (uint32_t i = 0; i < trackHdr.numLabels; ++i) {
        const uint32_t labelByteOffset = labelByteOffsets[i];
        const auto cmdIter = std::lower_bound(cmdByteOffsets.begin(), cmdByteOffsets.end(), labelByteOffset);

        if (cmdIter != cmdByteOffsets.end()) {
            const uint32_t labelCmdIdx = (uint32_t)(cmdIter - cmdByteOffsets.begin());
            labels.push_back(labelCmdIdx);
        } else {
            throw std::exception("Invalid byte offset for track label! Track data may be corrupt!");
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write an entire track to a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Track::writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false) {
    // Makeup a buffer and byte stream to write all the track data to
    std::vector<std::byte> trackDataBytes;
    trackDataBytes.reserve(cmds.size() * 8);

    const StreamWriteFunc trackDataWrite = [&](const void* const pSrc, const size_t size) noexcept(false) {
        for (size_t i = 0; i < size; ++i) {
            trackDataBytes.push_back(((const std::byte*) pSrc)[i]);
        }
    };

    // Write all of the track commands to the stream and save the byte offset of each command
    std::vector<uint32_t> cmdOffsets;
    cmdOffsets.reserve(cmds.size());

    for (const TrackCmd& cmd : cmds) {
        cmdOffsets.push_back((uint32_t) trackDataBytes.size() + Module::getVarLenQuantLen(cmd.delayQnp));   // Note: labels skip the command delay time bytes
        cmd.writeToWmd(trackDataWrite);
    }

    // Makeup the header for the track, endian correct and write to the file
    {
        WmdTrackHdr hdr = {};
        hdr.driverId = WmdSoundDriverId::PSX;
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
            throw std::exception("Too many labels in a track for a .WMD file!");

        hdr.numLabels = (uint16_t) labels.size();
        hdr.cmdStreamSize = (uint32_t) trackDataBytes.size();

        hdr.endianCorrect();
        streamWrite(&hdr, sizeof(WmdTrackHdr));
    }

    // Write the track labels to the file
    for (uint32_t cmdIdx : labels) {
        // Make sure the label is in range
        if (cmdIdx >= cmds.size())
            throw std::exception("Bad track label which cannot be serialized! References a track command that does not exist (out of range).");

        // Get the byte offset of the command and write
        const uint32_t cmdByteOffset = cmdOffsets[cmdIdx];
        streamWrite(&cmdByteOffset, sizeof(uint32_t));
    }

    // Write the track data to the file
    streamWrite(trackDataBytes.data(), trackDataBytes.size());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read an entire sequence from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::readFromWmd(const StreamReadFunc& streamRead) noexcept(false) {
    // Read the sequence header which tells us how many tracks there are
    WmdSequenceHdr seqHdr = {};
    streamRead(&seqHdr, sizeof(WmdSequenceHdr));
    seqHdr.endianCorrect();

    // Preserve this field for diff purposes against original .WMD files
    unknownWmdField = seqHdr.unknownField;

    // Read all the tracks in the sequence
    for (uint32_t trackIdx = 0; trackIdx < seqHdr.numTracks; ++trackIdx) {
        tracks.emplace_back().readFromWmd(streamRead);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write an entire sequence from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false) {
    // Write the header first
    {
        WmdSequenceHdr hdr = {};

        if (tracks.size() > UINT16_MAX)
            throw std::exception("Too many tracks in a sequence for a .WMD file!");
        
        hdr.numTracks = (uint16_t) tracks.size();
        hdr.unknownField = unknownWmdField;

        hdr.endianCorrect();
        streamWrite(&hdr, sizeof(WmdSequenceHdr));
    }

    // Then write all the tracks
    for (const Track& track : tracks) {
        track.writeToWmd(streamWrite);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the entire module to a json document
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::writeToJson(rapidjson::Document& doc) const noexcept {
    // Add basic module properties
    assert(doc.IsObject());
    rapidjson::Document::AllocatorType& docAlloc = doc.GetAllocator();

    doc.AddMember("maxActiveSequences", maxActiveSequences, docAlloc);
    doc.AddMember("maxActiveTracks", maxActiveTracks, docAlloc);
    doc.AddMember("maxGatesPerSeq", maxGatesPerSeq, docAlloc);
    doc.AddMember("maxItersPerSeq", maxItersPerSeq, docAlloc);
    doc.AddMember("maxCallbacks", maxCallbacks, docAlloc);

    // Write the PSX driver patch group
    {
        rapidjson::Value psxPatchGroupJson(rapidjson::kObjectType);
        psxPatchGroup.writeToJson(psxPatchGroupJson);
        doc.AddMember("psxPatchGroupJson", psxPatchGroupJson, docAlloc);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the entire module from the specified .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::readFromWmd(const StreamReadFunc& streamRead) noexcept(false) {
    // Read the header for the module file, verify correct and save it's basic info
    WmdModuleHdr moduleHdr = {};
    streamRead(&moduleHdr, sizeof(WmdModuleHdr));
    moduleHdr.endianCorrect();

    if (moduleHdr.moduleId != WMD_MODULE_ID)
        throw std::exception("Bad .WMD file ID!");

    if (moduleHdr.moduleVersion != WMD_VERSION)
        throw std::exception("Bad .WMD file version!");

    this->maxActiveSequences = moduleHdr.maxActiveSequences;
    this->maxActiveTracks = moduleHdr.maxActiveTracks;
    this->maxGatesPerSeq = moduleHdr.maxGatesPerSeq;
    this->maxItersPerSeq = moduleHdr.maxItersPerSeq;
    this->maxCallbacks = moduleHdr.maxCallbacks;

    // Read all patch groups
    for (uint32_t patchGrpIdx = 0; patchGrpIdx < moduleHdr.numPatchGroups; ++patchGrpIdx) {
        // Read the header for the patch group
        WmdPatchGroupHdr patchGroupHdr = {};
        streamRead(&patchGroupHdr, sizeof(WmdPatchGroupHdr));
        patchGroupHdr.endianCorrect();

        // If it's a PlayStation format patch group read it, otherwise skip
        if (patchGroupHdr.driverId == WmdSoundDriverId::PSX) {
            psxPatchGroup = {};
            psxPatchGroup.readFromWmd(streamRead, patchGroupHdr);
        } else {
            std::printf("Warning: skipping unsupported format patch group with driver id '%u'!\n", patchGroupHdr.driverId);
            skipReadingWmdPatchGroup(streamRead, patchGroupHdr);
        }
    }

    // Read all sequences
    for (uint32_t seqIdx = 0; seqIdx < moduleHdr.numSequences; ++seqIdx) {
        sequences.emplace_back().readFromWmd(streamRead);
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
void Module::writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false) {
    // Make up the module header, endian correct and serialize it
    {
        WmdModuleHdr hdr = {};
        hdr.moduleId = WMD_MODULE_ID;
        hdr.moduleVersion = WMD_VERSION;

        if (sequences.size() > UINT16_MAX)
            throw new std::exception("Too many sequences for a .WMD file!");

        hdr.numSequences = (uint16_t) sequences.size();
        hdr.numPatchGroups = 1; // Just the PSX patch group to be written
        hdr.maxActiveSequences = maxActiveSequences;
        hdr.maxActiveTracks = maxActiveTracks;
        hdr.maxGatesPerSeq = maxGatesPerSeq;
        hdr.maxItersPerSeq = maxItersPerSeq;
        hdr.maxCallbacks = maxCallbacks;

        hdr.endianCorrect();
        streamWrite(&hdr, sizeof(WmdModuleHdr));
    }

    // Write the PSX driver patch group
    psxPatchGroup.writeToWmd(streamWrite);

    // Write all the sequences
    for (const Sequence& sequence : sequences) {
        sequence.writeToWmd(streamWrite);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skip the data in a .WMD file for a patch group of an unknown type
//------------------------------------------------------------------------------------------------------------------------------------------
void Module::skipReadingWmdPatchGroup(const StreamReadFunc& reader, const WmdPatchGroupHdr& patchGroupHdr) noexcept(false) {
    if (patchGroupHdr.loadFlags & LOAD_PATCHES) {
        reader(nullptr, (size_t) patchGroupHdr.numPatches * patchGroupHdr.patchSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_PATCH_VOICES) {
        reader(nullptr, (size_t) patchGroupHdr.numPatchVoices * patchGroupHdr.patchVoiceSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_PATCH_SAMPLES) {
        reader(nullptr, (size_t) patchGroupHdr.numPatchSamples * patchGroupHdr.patchSampleSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_DRUM_PATCHES) {
        reader(nullptr, (size_t) patchGroupHdr.numDrumPatches * patchGroupHdr.drumPatchSize);
    }

    if (patchGroupHdr.loadFlags & LOAD_EXTRA_DATA) {
        reader(nullptr, patchGroupHdr.extraDataSize);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a variable length quantity from track data, similar to how certain data is encoded in the MIDI standard.
// Returns number of bytes read and the output value, which may be up to 5 bytes.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t Module::readVarLenQuant(const StreamReadFunc& streamRead, uint32_t& valueOut) noexcept(false) {
    // Grab the first byte in the quantity
    uint8_t curByte = {};
    streamRead(&curByte, 1);
    uint32_t numBytesRead = 1;

    // The top bit set on each byte means there is another byte to follow.
    // Each byte can therefore only encode 7 bits, so we need 5 of them to encode 32-bits:
    uint32_t decodedVal = curByte & 0x7Fu;

    while (curByte & 0x80) {
        // Make room for more data
        decodedVal <<= 7;

        // Read the next byte and incorporate into the value
        streamRead(&curByte, 1);
        numBytesRead++;
        decodedVal |= (uint32_t) curByte & 0x7Fu;

        // Sanity check, there should only be at most 5 bytes!
        if (numBytesRead > 5)
            throw std::exception("Read VLQ: too many bytes! Quantity encoding is not valid!");
    }

    valueOut = decodedVal;
    return numBytesRead;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a variable length quantity to track data, similar to how certain data is encoded in the MIDI standard.
// Returns number of bytes written, which may be up to 5 bytes.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t Module::writeVarLenQuant(const StreamWriteFunc& streamWrite, const uint32_t valueIn) noexcept(false) {
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
        streamWrite(pEncodedByte, 1);
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
