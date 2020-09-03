#include "Module.h"

#include <algorithm>

using namespace AudioTools;

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
            arg1 = ((int16_t) cmdData[0]) | ((int16_t) cmdData[1] << 8);
            break;

        case WmdTrackCmdType::NoteOn:
        case WmdTrackCmdType::WriteIterBox:
            arg1 = (uint8_t) cmdData[0];
            arg2 = (uint8_t) cmdData[1];
            break;

        case WmdTrackCmdType::StatusMark:
            arg1 = (uint8_t) cmdData[0];
            arg2 = ((int16_t) cmdData[1]) | ((int16_t) cmdData[2] << 8);
            break;

        case WmdTrackCmdType::GateJump:
        case WmdTrackCmdType::IterJump:
            arg1 = (uint8_t) cmdData[0];
            arg2 = (uint8_t) cmdData[1];
            arg3 = ((int16_t) cmdData[2]) | ((int16_t) cmdData[3] << 8);
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
        cmdByteOffsets.push_back(curCmdByteOffset);
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
// Read an entire sequence from a .WMD file
//------------------------------------------------------------------------------------------------------------------------------------------
void Sequence::readFromWmd(const StreamReadFunc& streamRead) noexcept(false) {
    // Read the sequence header which tells us how many tracks there are
    WmdSequenceHdr seqHdr = {};
    streamRead(&seqHdr, sizeof(WmdSequenceHdr));
    seqHdr.endianCorrect();

    // Read all the tracks in the sequence
    for (uint32_t trackIdx = 0; trackIdx < seqHdr.numTracks; ++trackIdx) {
        tracks.emplace_back().readFromWmd(streamRead);
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
uint32_t Module::readVarLenQuant(const StreamReadFunc& reader, uint32_t& valueOut) noexcept(false) {
    // Grab the first byte in the quantity
    uint8_t curByte = {};
    reader(&curByte, 1);
    uint32_t numBytesRead = 1;

    // The top bit set on each byte means there is another byte to follow.
    // Each byte can therefore only encode 7 bits, so we need 5 of them to encode 32-bits:
    uint32_t decodedVal = curByte & 0x7Fu;

    while (curByte & 0x80) {
        // Make room for more data
        decodedVal <<= 7;

        // Read the next byte and incorporate into the value
        reader(&curByte, 1);
        numBytesRead++;
        decodedVal |= (uint32_t) curByte & 0x7Fu;

        // Sanity check, there should only be at most 5 bytes!
        if (numBytesRead > 5)
            throw std::exception("Read VLQ: too many bytes! Quantity encoding is not valid!");
    }

    valueOut = decodedVal;
    return numBytesRead;
}
