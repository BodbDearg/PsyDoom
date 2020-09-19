#include "MidiConvert.h"

#include "MidiTypes.h"
#include "Sequence.h"
#include "WmdFileTypes.h"

#include <algorithm>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(MidiConvert)

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a Sequence from a module to a MidiFile structure.
//
// Limitations:
//  (1) Only the following command types are supported:
//      - PitchMod
//      - VolumeMod
//      - PanMod
//      - NoteOn
//      - NoteOff
//      - TrkEnd
//  (2) All tracks are assumed to have the same tempo
//------------------------------------------------------------------------------------------------------------------------------------------
void sequenceToMidi(const Sequence& sequence, MidiFile& midiFile) noexcept {
    // Clear all output tracks and determine the sequence tempo
    midiFile.bpm = 120;
    midiFile.ppq = 120;
    midiFile.tracks.clear();

    // Determine the tempo: just use the tempo from the first track
    for (const Track& track : sequence.tracks) {
        midiFile.bpm = track.initQpm;
        midiFile.ppq = track.initPpq;
        break;
    }

    // Create a single meta track and add a command there to set the tempo
    {
        MidiTrack& metaTrack = midiFile.tracks.emplace_back();

        // Add the tempo command
        {
            MidiCmd& cmd = metaTrack.cmds.emplace_back();
            cmd.type = MidiCmd::Type::SetTempo;
            cmd.arg1 = (uint16_t) std::min<uint32_t>(midiFile.bpm, UINT16_MAX);
            cmd.delayQnp = 0;
        }

        // Add the end of track command
        {
            MidiCmd& cmd = metaTrack.cmds.emplace_back();
            cmd.type = MidiCmd::Type::EndOfTrack;
            cmd.delayQnp = 0;
        }
    }

    // Create all the other tracks
    for (const Track& track : sequence.tracks) {
        // Name the track
        MidiTrack& midiTrack = midiFile.tracks.emplace_back();
        midiTrack.name = std::string("Track ") + std::to_string(midiFile.tracks.size() - 1);

        // Do command conversion
        for (const TrackCmd& trackCmd : track.cmds) {
            MidiCmd& midiCmd = midiTrack.cmds.emplace_back();
            midiCmd.delayQnp = trackCmd.delayQnp;
            
            switch (trackCmd.type) {
                case WmdTrackCmdType::PitchMod:
                    midiCmd.type = MidiCmd::Type::SetPitchBend;
                    midiCmd.arg1 = (uint16_t) std::clamp<int32_t>(trackCmd.arg1 + 8192, 0, 16383);      // Clamp to 14-bits. Note that '8192' is center pitch in MIDI.
                    break;

                case WmdTrackCmdType::PanMod:
                    midiCmd.type = MidiCmd::Type::SetPan;
                    midiCmd.arg1 = (uint16_t) std::clamp<int32_t>(trackCmd.arg1, 0, 127);   //  Note: 0x40 is center
                    break;

                case WmdTrackCmdType::VolumeMod:
                    midiCmd.type = MidiCmd::Type::SetVolume;
                    midiCmd.arg1 = (uint16_t) std::clamp<int32_t>(trackCmd.arg1, 0, 127);   // Volume
                    break;

                case WmdTrackCmdType::NoteOn:
                    midiCmd.type = MidiCmd::Type::NoteOn;
                    midiCmd.arg1 = (uint16_t) std::clamp<int32_t>(trackCmd.arg1, 0, 127);   // Note
                    midiCmd.arg2 = (uint16_t) std::clamp<int32_t>(trackCmd.arg2, 0, 127);   // Velocity
                    break;

                case WmdTrackCmdType::NoteOff:
                    midiCmd.type = MidiCmd::Type::NoteOff;
                    midiCmd.arg1 = (uint16_t) std::clamp<int32_t>(trackCmd.arg1, 0, 127);   // Note
                    midiCmd.arg2 = 64;                                                      // No release velocity! Use a middle value.
                    break;

                case WmdTrackCmdType::TrkEnd:
                    midiCmd.type = MidiCmd::Type::EndOfTrack;
                    break;

                // Unsupported command type, just remove it from the MIDI track
                default:
                    midiTrack.cmds.pop_back();
                    break;
            }
        }

        // Add an end of track command if there isn't already one
        if (midiTrack.cmds.empty() || (midiTrack.cmds.back().type != MidiCmd::Type::EndOfTrack)) {
            MidiCmd& cmd = midiTrack.cmds.emplace_back();
            cmd.type = MidiCmd::Type::EndOfTrack;
        }
    }
}

END_NAMESPACE(MidiConvert)
END_NAMESPACE(AudioTools)
