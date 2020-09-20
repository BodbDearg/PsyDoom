#pragma once

#include "TrackCmd.h"

#include <vector>

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

enum class WmdSoundClass : uint8_t;
enum class WmdSoundDriverId : uint8_t;
struct TrackCmd;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents an individual track in a sequence
//------------------------------------------------------------------------------------------------------------------------------------------
struct Track {
    WmdSoundDriverId        driverId;               // What sound driver the track should play with
    WmdSoundClass           soundClass;             // What rough class of sounds the track contains
    uint16_t                initPpq;                // Parts per quarter note: how many parts to subdivide each quarter note into for timing purposes. Affects timing precision.
    uint16_t                initQpm;                // The quarter notes per minute (beats per minute) value to initialize the track status with, unless manually overridden.
    uint16_t                initPatchIdx;           // Which patch index to initially use for the track unless manually overridden
    int16_t                 initPitchCntrl;         // What pitch shift value to initially use for the track unless manually overridden
    uint8_t                 initVolumeCntrl;        // What volume to initially use for the track unless manually overridden
    uint8_t                 initPanCntrl;           // What pan setting to initially use for the track unless manually overridden
    uint8_t                 initReverb;             // The reverb level to initialize the track with
    uint8_t                 initMutegroupsMask;     // A bit mask defining what 'mute groups' the track is a part of: used for bulk muting of tracks by group
    uint8_t                 maxVoices;              // Maximum number of voices that the track claims to use
    uint8_t                 locStackSize;           // The maximum number of track data locations that can be remembered in the track's location stack (for save + return control flow)
    uint8_t                 priority;               // Used for prioritizing voices when we are out of hardware voices
    std::vector<uint32_t>   labels;                 // Locations to jump to in the track as a command index
    std::vector<TrackCmd>   cmds;                   // The commands for the track

    void readFromJson(const rapidjson::Value& jsonRoot) noexcept;
    void writeToJson(rapidjson::Value& jsonRoot, rapidjson::Document::AllocatorType& jsonAlloc) const noexcept;
    void readFromWmdFile(InputStream& in) THROWS;
    void writeToWmdFile(OutputStream& out) const THROWS;
};

END_NAMESPACE(AudioTools)
