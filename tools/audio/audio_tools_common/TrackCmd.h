#pragma once

#include "Macros.h"

#include <cstdint>
#include <rapidjson/document.h>

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

enum class WmdTrackCmdType : uint8_t;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a sequencer command in a track.
// Depending on the command type, none or all of the arguments may be used and also the range allowed might be capped.
//------------------------------------------------------------------------------------------------------------------------------------------
struct TrackCmd {
    WmdTrackCmdType     type;           // What type of command this is?
    uint32_t            delayQnp;       // Time until the command executes in quarter note parts (QNP). The actual delay in seconds depends on quarter note parts per minute count and parts per quarter note.
    int32_t             arg1;           // Command argument 1: meaning (if any) depends on the command
    int32_t             arg2;           // Command argument 2: meaning (if any) depends on the command
    int32_t             arg3;           // Command argument 3: meaning (if any) depends on the command

    void readFromJson(const rapidjson::Value& jsonRoot) noexcept;
    void writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept;
    uint32_t readFromWmd(InputStream& in) THROWS;
    uint32_t writeToWmd(OutputStream& out) const THROWS;
};

END_NAMESPACE(AudioTools)
