#include "TrackCmd.h"

#include "InputStream.h"
#include "JsonUtils.h"
#include "MidiUtils.h"
#include "OutputStream.h"
#include "WmdFileTypes.h"

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a single track command from json
//------------------------------------------------------------------------------------------------------------------------------------------
void TrackCmd::readFromJson(const rapidjson::Value& jsonRoot) noexcept {
    type = stringToTrackCmdType(JsonUtils::getOrDefault<const char*>(jsonRoot, "type", ""));
    delayQnp = JsonUtils::clampedGetOrDefault<uint32_t>(jsonRoot, "delayQnp", 0);
    arg1 = JsonUtils::clampedGetOrDefault<int32_t>(jsonRoot, "arg1", 0);
    arg2 = JsonUtils::clampedGetOrDefault<int32_t>(jsonRoot, "arg2", 0);
    arg3 = JsonUtils::clampedGetOrDefault<int32_t>(jsonRoot, "arg3", 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a single track command to json
//------------------------------------------------------------------------------------------------------------------------------------------
void TrackCmd::writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept {
    jsonRoot.AddMember("type", rapidjson::StringRef(toString(type)), jsonAlloc);
    jsonRoot.AddMember("delayQnp", delayQnp, jsonAlloc);

    const int32_t numCmdArgs = getNumWmdTrackCmdArgs(type);

    if (numCmdArgs >= 1) {
        jsonRoot.AddMember("arg1", arg1, jsonAlloc);
    }

    if (numCmdArgs >= 2) {
        jsonRoot.AddMember("arg2", arg2, jsonAlloc);
    }

    if (numCmdArgs >= 3) {
        jsonRoot.AddMember("arg3", arg3, jsonAlloc);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a single track command from a .WMD file and return how many bytes were read
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t TrackCmd::readFromWmdFile(InputStream& in) THROWS {
    // First read the delay (in quarter note parts) until the command
    uint32_t numBytesRead = MidiUtils::readVarLenQuant(in, delayQnp);

    // Next grab the command id
    in.read(type);
    numBytesRead += sizeof(WmdTrackCmdType);

    // Read the data for the command into this stack buffer (which is more than big enough)
    std::byte cmdData[8] = {};
    const uint32_t cmdSize = getWmdTrackCmdSize(type);

    if (cmdSize <= 0)
        throw "Unexpected command in the track's command stream!";

    const uint32_t cmdDataSize = cmdSize - 1;

    if (cmdDataSize > 8)
        throw "Bad track command payload size!";

    in.readBytes(cmdData, cmdDataSize);
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
            throw "Unexpected command type in the track's command stream!";
    }

    return numBytesRead;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a single track command to a .WMD file and return how many bytes were written
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t TrackCmd::writeToWmdFile(OutputStream& out) const THROWS {
    // First write the delay (in quarter note parts) until the command
    uint32_t numBytesWritten = MidiUtils::writeVarLenQuant(out, delayQnp);

    // Next Write the command id
    out.write(type);
    numBytesWritten += sizeof(WmdTrackCmdType);

    // Put the data for the command into this stack buffer, which is more than big enough.
    // First verify however that it is a command that we can write to the .WMD:
    std::byte cmdData[8] = {};
    const uint32_t cmdSize = getWmdTrackCmdSize(type);

    if (cmdSize <= 0)
        throw "Unexpected command in the track's command stream which cannot be serialized to a .WMD file!";

    const uint32_t cmdDataSize = cmdSize - 1;

    if (cmdDataSize > 8)
        throw "Bad track command payload size!";

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
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte) arg1;
        }   break;

        case WmdTrackCmdType::PatchChg:
        case WmdTrackCmdType::SeqTempo:
        case WmdTrackCmdType::TrkTempo: {
            if ((arg1 < 0) || (arg1 > UINT16_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte)(arg1);
            cmdData[1] = (std::byte)(arg1 >> 8);
        }   break;

        case WmdTrackCmdType::PitchMod:
        case WmdTrackCmdType::SeqGosub:
        case WmdTrackCmdType::TrkGosub:
        case WmdTrackCmdType::SeqJump:
        case WmdTrackCmdType::TrkJump: {
            if ((arg1 < INT16_MIN) || (arg1 > INT16_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte)((uint32_t) arg1);
            cmdData[1] = (std::byte)((uint32_t) arg1 >> 8);
        }   break;

        case WmdTrackCmdType::NoteOn:
        case WmdTrackCmdType::WriteIterBox: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            if ((arg2 < 0) || (arg2 > UINT8_MAX))
                throw "Track cmd has arg2 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte) arg1;
            cmdData[1] = (std::byte) arg2;
        }   break;

        case WmdTrackCmdType::StatusMark: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            if ((arg2 < INT16_MIN) || (arg2 > INT16_MAX))
                throw "Track cmd has arg2 which is out of range for a .WMD file!";

            cmdData[0] = (std::byte)(arg1);
            cmdData[1] = (std::byte)((uint32_t) arg2);
            cmdData[2] = (std::byte)((uint32_t) arg2 >> 8);
        }   break;

        case WmdTrackCmdType::GateJump:
        case WmdTrackCmdType::IterJump: {
            if ((arg1 < 0) || (arg1 > UINT8_MAX))
                throw "Track cmd has arg1 which is out of range for a .WMD file!";

            if ((arg2 < 0) || (arg2 > UINT8_MAX))
                throw "Track cmd has arg2 which is out of range for a .WMD file!";

            if ((arg3 < INT16_MIN) || (arg3 > INT16_MAX))
                throw "Track cmd has arg3 which is out of range for a .WMD file!";

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
            throw "Unexpected command in the track's command stream which cannot be serialized to a .WMD file!";
    }

    // Serialize the bytes for the command data
    out.writeBytes(cmdData, cmdDataSize);
    numBytesWritten += cmdDataSize;
    return numBytesWritten;
}

END_NAMESPACE(AudioTools)
