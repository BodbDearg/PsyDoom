#pragma once

#include "Macros.h"

#include <cstdint>
#include <string>
#include <vector>

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a single MIDI command
//------------------------------------------------------------------------------------------------------------------------------------------
struct MidiCmd {
    //--------------------------------------------------------------------------------------------------------------------------------------
    // Type for a MIDI command.
    // This is very pruned down list, just the commands I'm interested in for these audio tools...
    //--------------------------------------------------------------------------------------------------------------------------------------
    enum class Type : uint8_t {
        EndOfTrack,         // End of track event
        NoteOn,             // Sound a note: arg1 = key, arg2 = velocity
        NoteOff,            // Release a note: arg1 = key, arg2 = velocity
        SetVolume,          // Change channel volume: arg1 = new volume
        SetPan,             // Change channel pan: arg1 = new pan
        SetPitchBend,       // Change pitch modulation amount: arg1 = new pitch modulation (in 7.7 format)
        SetTempo,           // Set sequence tempo: arg1 = new tempo in beats per minute
        UnhandledMetaCmd,   // A meta command that is unhandled: I preserve these as a placeholder, so we can identify meta info tracks
    };

    Type        type;           // Type of command
    uint8_t     channel;        // What channel the command applies to
    uint16_t    arg1;           // Generic command argument 1
    uint16_t    arg2;           // Generic command Argument 2
    uint32_t    delayQnp;       // Delay until executing the command (in quarter note parts)

    static bool isMetaTrackCmdType(const Type type) noexcept;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a track in a MIDI file
//------------------------------------------------------------------------------------------------------------------------------------------
struct MidiTrack {
    std::string             name;
    std::vector<MidiCmd>    cmds;

    bool isMetaTrack() const noexcept;
    uint64_t getQnpDuration() const noexcept;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Container for the entire MIDI file
//------------------------------------------------------------------------------------------------------------------------------------------
struct MidiFile {
    uint32_t                bpm;        // Tempo: beats per minute
    uint32_t                ppq;        // Timing resolution: parts per quarter note
    std::vector<MidiTrack>  tracks;     // The tracks themselves

    uint32_t getEndMetaTrackIdx() const noexcept;
    uint64_t getQnpDuration() const noexcept;
    bool definesStartTempoInMetaTracks() const noexcept;
};

END_NAMESPACE(AudioTools)
