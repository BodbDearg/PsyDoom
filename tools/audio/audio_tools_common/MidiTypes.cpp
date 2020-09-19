#include "MidiTypes.h"

#include <algorithm>

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a command is the type that would only be found in a meta-track or a track only containing metadata and not actual notes
//------------------------------------------------------------------------------------------------------------------------------------------
bool MidiCmd::isMetaTrackCmdType(const Type type) noexcept {
    switch (type) {
        case Type::SetTempo:
        case Type::UnhandledMetaCmd:
            return true;

        default:
            return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the track is regarded as being a meta track, not considering the track's position in the track list.
// Meta tracks should normally be first in the track list.
//------------------------------------------------------------------------------------------------------------------------------------------
bool MidiTrack::isMetaTrack() const noexcept {
    // Regard an empty track as NOT being a meta track.
    // Might be an empty instrumental track?
    if (!cmds.empty()) {
        for (const MidiCmd& cmd : cmds) {
            // Ignore end of track commands, they are in ALL tracks
            if (cmd.type == MidiCmd::Type::EndOfTrack)
                continue;

            // Not a meta track command type?
            if (!MidiCmd::isMetaTrackCmdType(cmd.type))
                return false;
        }

        return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the duration of the track in quarter note parts
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t MidiTrack::getQnpDuration() const noexcept {
    uint64_t duration = 0;

    for (const MidiCmd& cmd : cmds) {
        if (cmd.type == MidiCmd::Type::EndOfTrack)
            break;

        duration += (uint64_t) cmd.delayQnp;
    }

    return duration;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the end index (exclusive) for the meta tracks in the MIDI file.
// The meta tracks are assumed to always be the first in the file, and normally there is just 1.
// FL Studio seems to add in 2 on export though (weird)...
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t MidiFile::getEndMetaTrackIdx() const noexcept {
    uint32_t endIdx = 0;

    for (const MidiTrack& track : tracks) {
        if (track.isMetaTrack()) {
            ++endIdx;
        } else {
            break;
        }
    }

    return endIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the total duration of the midi file in quarter note parts
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t MidiFile::getQnpDuration() const noexcept {
    uint64_t duration = 0;

    for (const MidiTrack& track : tracks) {
        duration = std::max(duration, track.getQnpDuration());
    }

    return duration;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the start tempo is defined in one of the meta tracks before anything plays
//------------------------------------------------------------------------------------------------------------------------------------------
bool MidiFile::definesStartTempoInMetaTracks() const noexcept {
    const uint32_t endMetaTrackIdx = getEndMetaTrackIdx();

    for (uint32_t i = 0; i < endMetaTrackIdx; ++i) {
        const MidiTrack& track = tracks[i];

        for (const MidiCmd& cmd : track.cmds) {
            // A delay means the command is not at the start, therefore no start tempo can be defined from here on in
            if (cmd.delayQnp != 0)
                break;

            if (cmd.type == MidiCmd::Type::SetTempo)
                return true;
        }
    }

    return false;
}

END_NAMESPACE(AudioTools)
