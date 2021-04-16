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

        // Add a command to initialize the pitch bend to '0' or 'center' for the track.
        // Only do this however if there isn't already a pitch bend command at delay 0.
        bool bHasInitPitchBendCmd = false;

        for (const TrackCmd& trackCmd : track.cmds) {
            if (trackCmd.delayQnp != 0)
                break;

            if (trackCmd.type == WmdTrackCmdType::PitchMod) {
                bHasInitPitchBendCmd = true;
                break;
            }
        }

        if (!bHasInitPitchBendCmd) {
            MidiCmd& midiCmd = midiTrack.cmds.emplace_back();
            midiCmd.type = MidiCmd::Type::SetPitchBend;
            midiCmd.arg1 = 8192;
        }

        // Do command conversion
        uint32_t skippedCmdsDelayQnp = 0;

        for (const TrackCmd& trackCmd : track.cmds) {
            MidiCmd& midiCmd = midiTrack.cmds.emplace_back();
            midiCmd.delayQnp = trackCmd.delayQnp + skippedCmdsDelayQnp;
            skippedCmdsDelayQnp = 0;

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

                // Unsupported command type, just remove it from the MIDI track and skip over
                default:
                    skippedCmdsDelayQnp += midiCmd.delayQnp;
                    midiTrack.cmds.pop_back();
                    break;
            }
        }

        // Add an end of track command if there isn't already one
        if (midiTrack.cmds.empty() || (midiTrack.cmds.back().type != MidiCmd::Type::EndOfTrack)) {
            MidiCmd& cmd = midiTrack.cmds.emplace_back();
            cmd.type = MidiCmd::Type::EndOfTrack;
            cmd.delayQnp = skippedCmdsDelayQnp;
            skippedCmdsDelayQnp = 0;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a MidiFile structure to a sequence.
//
// Limitations:
//  (1) Set tempo commands are ignored, only the tempo on the 'MidiFile' object is used and it stays constant for the sequence.
//------------------------------------------------------------------------------------------------------------------------------------------
void midiToSequence(const MidiFile& midiFile, Sequence& sequence, const bool bIsMusic) noexcept {
    // How many actual music tracks are there in the MIDI file and how long is it? (in quarter note parts)
    const uint32_t endMidiMetaTrack = midiFile.getEndMetaTrackIdx();
    const uint32_t numActualTracks = (uint32_t)(midiFile.tracks.size() - endMidiMetaTrack);
    const uint64_t sequenceQnpLength = midiFile.getQnpDuration();

    // Do basic track initialization.
    // Note: I'm using resize here so existing tracks settings will be preserved.
    sequence.unknownWmdField = {};
    const size_t oldNumTracks = sequence.tracks.size();
    sequence.tracks.resize(numActualTracks);

    for (uint32_t i = 0; i < sequence.tracks.size(); ++i) {
        const bool bIsNewTrack = (i >= oldNumTracks);

        // Set basic track properties
        Track& track = sequence.tracks[i];
        track.driverId = WmdSoundDriverId::PSX;
        track.soundClass = (bIsMusic) ? WmdSoundClass::MUSIC : WmdSoundClass::SNDFX;
        track.initQpm = (uint16_t) std::min<uint32_t>(midiFile.bpm, UINT16_MAX);
        track.initPpq = (uint16_t) std::min<uint32_t>(midiFile.ppq, UINT16_MAX);
        track.priority = (bIsMusic) ? 128 : 100;

        // Initialize these settings if we've just newly created the track
        if (bIsNewTrack) {
            track.initPatchIdx = 0;
            track.initPitchCntrl = 0;
            track.initVolumeCntrl = 100;
            track.initPanCntrl = 64;            // Center pan
            track.initReverb = 0;
            track.initMutegroupsMask = 0;
            track.maxVoices = 24;               // PSX hardware voice limit
            track.locStackSize = 1;             // Not using this at all, just assign a token value
        }

        // Clear track data
        track.labels.clear();
        track.cmds.clear();

        // If it's music then we will be looping, make a location pointing to the start command in the track
        if (bIsMusic) {
            track.labels.push_back(0);
        }
    }

    // Convert each track
    for (size_t i = 0; i < sequence.tracks.size(); ++i) {
        // Get the source and destination track
        const MidiTrack& srcTrack = midiFile.tracks[i + endMidiMetaTrack];
        Track& dstTrack = sequence.tracks[i];

        // This is how many quarter note parts till the end of the sequence
        uint64_t qnpTillSeqEnd = sequenceQnpLength;

        // In case we are skipping a command from the source stream, add the delay to the next command we actually handle
        uint32_t skippedCmdsQnpDelay = 0;

        // Convert the commands
        const auto addDstCmd = [&](const MidiCmd& srcCmd, const WmdTrackCmdType type) -> TrackCmd& {
            TrackCmd& dstCmd = dstTrack.cmds.emplace_back();
            dstCmd.type = type;
            dstCmd.delayQnp = srcCmd.delayQnp + skippedCmdsQnpDelay;
            skippedCmdsQnpDelay = 0;

            if (dstCmd.delayQnp > qnpTillSeqEnd) {
                qnpTillSeqEnd = 0;
            } else {
                qnpTillSeqEnd -= dstCmd.delayQnp;
            }

            return dstCmd;
        };

        for (const MidiCmd& srcCmd : srcTrack.cmds) {
            switch (srcCmd.type) {
                // Ignore the end of track message and stick in our own.
                // Might not end at the same time as the entire sequence, which is what we want...
                case MidiCmd::Type::EndOfTrack: {
                    skippedCmdsQnpDelay += srcCmd.delayQnp;
                }   break;

                case MidiCmd::Type::NoteOn: {
                    TrackCmd& dstCmd = addDstCmd(srcCmd, WmdTrackCmdType::NoteOn);
                    dstCmd.arg1 = srcCmd.arg1;  // Note
                    dstCmd.arg2 = srcCmd.arg2;  // Velocity
                }   break;

                case MidiCmd::Type::NoteOff: {
                    TrackCmd& dstCmd = addDstCmd(srcCmd, WmdTrackCmdType::NoteOff);
                    dstCmd.arg1 = srcCmd.arg1;  // Note
                }   break;

                case MidiCmd::Type::SetVolume: {
                    TrackCmd& dstCmd = addDstCmd(srcCmd, WmdTrackCmdType::VolumeMod);
                    dstCmd.arg1 = srcCmd.arg1;  // Volume
                }   break;

                case MidiCmd::Type::SetPan: {
                    TrackCmd& dstCmd = addDstCmd(srcCmd, WmdTrackCmdType::PanMod);
                    dstCmd.arg1 = srcCmd.arg1;  // Pan
                }   break;

                case MidiCmd::Type::SetPitchBend: {
                    TrackCmd& dstCmd = addDstCmd(srcCmd, WmdTrackCmdType::PitchMod);
                    dstCmd.arg1 = (int32_t) srcCmd.arg1 - 8192;     // Pitch
                }   break;

                // Not supporting 'Set Tempo' commands and also skip unhandled meta commands or unknown command types
                case MidiCmd::Type::SetTempo:
                case MidiCmd::Type::UnhandledMetaCmd:
                default: {
                    skippedCmdsQnpDelay += srcCmd.delayQnp;
                }   break;
            }
        }

        // If music add in a loop command
        if (bIsMusic) {
            TrackCmd& dstCmd = dstTrack.cmds.emplace_back();
            dstCmd.type = WmdTrackCmdType::TrkJump;
            dstCmd.delayQnp = (uint32_t) qnpTillSeqEnd;
            dstCmd.arg1 = 0;    // Destination label index: track start

            qnpTillSeqEnd = 0;
            skippedCmdsQnpDelay = 0;
        }

        // Add in the track end command
        {
            TrackCmd& dstCmd = dstTrack.cmds.emplace_back();
            dstCmd.type = WmdTrackCmdType::TrkEnd;
            dstCmd.delayQnp = (uint32_t) qnpTillSeqEnd;
        }
    }
}

END_NAMESPACE(MidiConvert)
END_NAMESPACE(AudioTools)
