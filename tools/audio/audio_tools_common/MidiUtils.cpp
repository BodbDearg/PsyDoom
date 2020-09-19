#include "MidiUtils.h"

#include "Asserts.h"
#include "ByteInputStream.h"
#include "Endian.h"
#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "MidiTypes.h"

#include "ByteVecOutputStream.h"

#include <algorithm>
#include <vector>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(MidiUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// MIDI file data structures and constants
//------------------------------------------------------------------------------------------------------------------------------------------

// Id for chunks in a MIDI file
enum class MidiFileChunkId : uint32_t {
    MThd    = 0x6468544D,       // Header chunk id
    MTrk    = 0x6B72544D,       // Track chunk id
};

// MIDI Meta command types: just the ones we are handling
enum class MidiMetaCmdType : uint8_t {
    SequenceTrackName   = 0x03,     // Contains an ASCII string identifying the track (or sequence) name
    EndOfTrack          = 0x2F,     // Marks the end of the track, not an optional command
    SetTempo            = 0x51,     // Set the tempo: 24-bit arg is microseconds per quarter note
};

// Controller change command or channel mode command: just the ones we are handling
enum class MidiCcOrModeCmdType : uint8_t {
    VolumeMsb   = 0x07,     // Channel volume: most significant byte
    PanMsb      = 0x0A,     // Channel pan: most significant byte
};

// The default tempo for a MIDI file
static constexpr uint32_t MIDI_DEFAULT_BPM = 120;

// Header for a chunk in a MIDI file
struct MidiFileChunkHdr {
    MidiFileChunkId     id;         // What chunk is it?
    uint32_t            size;       // How many bytes of data there are

    void endianCorrect() noexcept {
        id = Endian::littleToHost(id);      // Note: it's really ASCII, but I read as if it's a little endian uint32_t
        size = Endian::bigToHost(size);     // This like most other MIDI file fields is big endian
    }
};

static_assert(sizeof(MidiFileChunkHdr) == 8);

// MIDI file header payload (version 1.0 MIDI file)
struct MidiFileHdr {
    uint16_t    format;         // 0 = Single Track, 1 = Multi-track / single-sequence, 2 = Multi-track / multi-sequence
    uint16_t    numTracks;      // How many tracks there are
    uint16_t    timingInfo;     // If high bit is '0', bits 14-0 = parts per quarter note. Otherwise bits 14-8 = FPS and bits 7-0 = parts per frame.

    void endianCorrect() noexcept {
        format = Endian::bigToHost(format);
        numTracks = Endian::bigToHost(numTracks);
        timingInfo = Endian::bigToHost(timingInfo);
    }
};

static_assert(sizeof(MidiFileHdr) == 6);

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine the start tempo for the entire midi sequence
//------------------------------------------------------------------------------------------------------------------------------------------
static void determineStartTempo(MidiFile& midiFile) noexcept {
    // Run through all tracks and find set tempo commands that have a delay of '0'.
    // These are the initial tempo values:
    for (MidiTrack& track : midiFile.tracks) {
        auto cmdIter = track.cmds.begin();
        auto cmdEndIter = track.cmds.end();

        for (const MidiCmd& cmd : track.cmds) {
            // Stop if the command is delayed
            if (cmd.delayQnp != 0)
                break;

            // Set tempo command? If so then set the sequence initial tempo:
            if (cmd.type == MidiCmd::Type::SetTempo) {
                midiFile.bpm = cmd.arg1;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Optimize the given track and remove pitch control, volume and pan commands that aren't neccessary.
// Assumes all these commands for the track will be going to the same instrument.
//------------------------------------------------------------------------------------------------------------------------------------------
static void optimizeTrackCmds(MidiTrack& track) noexcept {
    // Initially nothing is set
    bool bSetVol = false;
    bool bSetPan = false;
    bool bSetPitchBend = false;
    uint16_t vol = {};
    uint16_t pan = {};
    uint16_t pitchBend = {};

    // Find the redundant track commands
    auto cmdIter = track.cmds.begin();

    while (cmdIter != track.cmds.end()) {
        const MidiCmd cmd = *cmdIter;
        bool bRemoveCmd = false;

        // Only optimize the command if it doesn't introduce a delay.
        // Otherwise it might be important for timing:
        if (cmd.delayQnp == 0) {
            if (cmd.type == MidiCmd::Type::SetVolume) {
                if ((!bSetVol) || (vol != cmd.arg1)) {
                    bSetVol = true;
                    vol = cmd.arg1;
                } else {
                    bRemoveCmd = true;
                }
            }
            else if (cmd.type == MidiCmd::Type::SetPan) {
                if ((!bSetPan) || (pan != cmd.arg1)) {
                    bSetPan = true;
                    pan = cmd.arg1;
                } else {
                    bRemoveCmd = true;
                }
            }
            else if (cmd.type == MidiCmd::Type::SetPitchBend) {
                if ((!bSetPitchBend) || (pitchBend != cmd.arg1)) {
                    bSetPitchBend = true;
                    pitchBend = cmd.arg1;
                } else {
                    bRemoveCmd = true;
                }
            }
        }

        if (bRemoveCmd) {
            cmdIter = track.cmds.erase(cmdIter);
        } else {
            ++cmdIter;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the commands for a track
//------------------------------------------------------------------------------------------------------------------------------------------
static void readTrackCmds(InputStream& in, MidiTrack& track) THROWS {
    // Continue reading until we reach the end of the stream
    while (!in.isAtEnd()) {
        // Create the command and read the delay in ticks (normally quarter note parts) to the command
        MidiCmd& cmd = track.cmds.emplace_back();
        cmd.delayQnp = readVarLenQuant(in);

        // Get the MIDI command status byte and high/low nibbles.
        // The high nibble is the high level command type, low nibble is usually the MIDI channel affected.
        const uint8_t statusByte = in.read<uint8_t>();
        const uint8_t statusByteHiNib = statusByte >> 4;
        const uint8_t statusByteLoNib = statusByte & 0xF;

        // See what we are dealing with a high level
        if (statusByteHiNib == 0x8) {
            // Note off message:
            cmd.type = MidiCmd::Type::NoteOff;
            cmd.channel = statusByteLoNib;
            cmd.arg1 = in.read<uint8_t>();          // Key
            cmd.arg2 = in.read<uint8_t>();          // Velocity
        }
        else if (statusByteHiNib == 0x9) {
            // Note on message:
            cmd.type = MidiCmd::Type::NoteOn;
            cmd.channel = statusByteLoNib;
            cmd.arg1 = in.read<uint8_t>();          // Key
            cmd.arg2 = in.read<uint8_t>();          // Velocity
        }
        else if (statusByteHiNib == 0xA) {
            // Polyphonic key pressure (aftertouch): ignore this command and skip it's payload
            track.cmds.pop_back();
            in.skipBytes(2);
        }
        else if (statusByteHiNib == 0xB) {
            // Controller change or channel 'mode' message: the first byte is the type, second byte data:
            const MidiCcOrModeCmdType cmdType = in.read<MidiCcOrModeCmdType>();
            const uint8_t cmdData = in.read<uint8_t>();

            if (cmdType == MidiCcOrModeCmdType::VolumeMsb) {
                cmd.type = MidiCmd::Type::SetVolume;
                cmd.arg1 = cmdData;
            }
            else if (cmdType == MidiCcOrModeCmdType::PanMsb) {
                cmd.type = MidiCmd::Type::SetPan;
                cmd.arg1 = cmdData;
            }
            else {
                // Ignore this controller change: not supported
                track.cmds.pop_back();
            }
        }
        else if (statusByteHiNib == 0xC) {
            // Program change: ignore this command and skip it's payload
            track.cmds.pop_back();
            in.skipBytes(1);
        }
        else if (statusByteHiNib == 0xD) {
            // Channel key pressure (aftertouch): ignore this command and skip it's payload
            track.cmds.pop_back();
            in.skipBytes(1);
        }
        else if (statusByteHiNib == 0xE) {
            // Pitch bend command:
            cmd.type = MidiCmd::Type::SetPitchBend;
            cmd.channel = statusByteLoNib;

            const uint16_t amtLoBits = in.read<uint8_t>() & 0x7F;
            const uint16_t amtHiBits = in.read<uint8_t>() & 0x7F;
            cmd.arg1 = (amtHiBits << 7) | amtLoBits;
        }
        else if ((statusByte == 0xF0) || (statusByte == 0xF7)) {
            // A 'sysex' command: read it's size, ignore the command and skip over
            track.cmds.pop_back();
            const uint32_t cmdSize = readVarLenQuant(in);
            in.skipBytes(cmdSize);
        }
        else if (statusByte == 0xFF) {
            // A meta info command lies ahead, read the command type and size
            const MidiMetaCmdType cmdType = in.read<MidiMetaCmdType>();
            const uint32_t cmdSize = readVarLenQuant(in);

            if (cmdType == MidiMetaCmdType::SequenceTrackName) {
                // Track name command: consume this command and embed the data in the track 'name' field
                track.cmds.pop_back();
                track.name.resize(cmdSize);
                in.readArray<char>(track.name.data(), cmdSize);
            }
            else if (cmdType == MidiMetaCmdType::EndOfTrack) {
                // End of track command: save and stop reading the track here
                cmd.type = MidiCmd::Type::EndOfTrack;

                if (cmdSize != 0)
                    throw "Bad/unexpected meta command size found in the MIDI file! File may be corrupt!";

                break;
            }
            else if (cmdType == MidiMetaCmdType::SetTempo) {
                // Set tempo command
                cmd.type = MidiCmd::Type::SetTempo;

                if (cmdSize != 3)
                    throw "Bad/unexpected meta command size found in the MIDI file! File may be corrupt!";

                // Note: this code has the capability to lose resolution but generally in a DAW we are always setting tempo as an integer BPM
                const uint32_t byte1 = in.read<uint8_t>();
                const uint32_t byte2 = in.read<uint8_t>();
                const uint32_t byte3 = in.read<uint8_t>();
                const uint32_t microSecsPerQuarterNote = (byte1 << 16) | (byte2 << 8) | byte3;
                const uint32_t bpm = 60'500'000 / microSecsPerQuarterNote;  // Note: adding 500,000 microseconds to round the result up
                cmd.arg1 = (uint16_t) std::clamp<uint32_t>(bpm, 0, UINT16_MAX);
            }
            else {
                // Unhandled meta command type: save it just as a placeholder so we can identify meta-only tracks
                cmd.type = MidiCmd::Type::UnhandledMetaCmd;
                in.skipBytes(cmdSize);
            }
        }
        else {
            throw "Invalid/unrecognized command type found in the MIDI file! File may be corrupt!";
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a MIDI file from the given input stream
//------------------------------------------------------------------------------------------------------------------------------------------
bool readMidiFile(InputStream& in, MidiFile& midiFileOut, std::string& errorMsgOut) noexcept {
    // Default the midi file initially
    midiFileOut.tracks.clear();
    midiFileOut.bpm = MIDI_DEFAULT_BPM;
    midiFileOut.ppq = 120;

    // Begin reading
    bool bReadOk = false;

    try {
        // Get the chunk header for the MIDI file header and validate
        MidiFileChunkHdr fileHdrChunkHdr = {};
        in.read(fileHdrChunkHdr);
        fileHdrChunkHdr.endianCorrect();

        if (fileHdrChunkHdr.id != MidiFileChunkId::MThd)
            throw "File is not a valid MIDI file! File header is invalid!";

        if (fileHdrChunkHdr.size < sizeof(MidiFileHdr))
            throw "File is not a valid MIDI file! File header is invalid!";

        // Read the file header
        MidiFileHdr fileHdr = {};
        in.read(fileHdr);
        fileHdr.endianCorrect();

        // Skip over any other bytes in the header chunk (in case of newer MIDI file format)
        in.skipBytes(fileHdrChunkHdr.size - sizeof(MidiFileHdr));

        // Set file timing settings
        midiFileOut.bpm = MIDI_DEFAULT_BPM;

        if (fileHdr.timingInfo & 0x8000u) {
            // SMTPE format timing: FPS and subdivisions of a frame
            const uint16_t framesPerSec = (fileHdr.timingInfo >> 8) & 0x7Fu;
            const uint16_t partsPerFrame = fileHdr.timingInfo & 0xFFu;
            midiFileOut.bpm = (uint32_t) framesPerSec * 60u;
            midiFileOut.ppq = partsPerFrame;
        } else {
            // More normal case - just parts per quarter note is specified.
            // Tempo for the sequence is specified in 1st track, defaulting otherwise if not given:
            midiFileOut.bpm = MIDI_DEFAULT_BPM;
            midiFileOut.ppq = fileHdr.timingInfo;
        }

        // Next read all of the track chunks in the MIDI file and skip over any unrecognized chunks
        std::vector<std::byte> chunkBuffer;
        chunkBuffer.reserve(64 * 1024);

        for (uint16_t trackIdx = 0; trackIdx < fileHdr.numTracks;) {
            // Grab the chunk header
            MidiFileChunkHdr chunkHdr = {};
            in.read(chunkHdr);
            chunkHdr.endianCorrect();

            // Skip the chunk if it's not a track one
            if (chunkHdr.id != MidiFileChunkId::MTrk) {
                in.skipBytes(chunkHdr.size);
                continue;
            }

            // Read the entire data for the track chunk into a buffer
            ++trackIdx;
            chunkBuffer.resize(chunkHdr.size);
            in.readBytes(chunkBuffer.data(), chunkBuffer.size());

            // Now read that track data into a series of commands
            ByteInputStream chunkInputStream(chunkBuffer.data(), (uint32_t) chunkBuffer.size());
            MidiTrack& track = midiFileOut.tracks.emplace_back();
            readTrackCmds(chunkInputStream, track);
        }

        // Post processing of the midi file
        determineStartTempo(midiFileOut);

        for (MidiTrack& track : midiFileOut.tracks) {
            optimizeTrackCmds(track);
        }

        // All good if we got to here!
        bReadOk = true;
    }
    catch (const char* const exceptionMsg) {
        errorMsgOut = "An error occurred while reading the MIDI file! It may not be a valid or in a supported format. Error message: ";
        errorMsgOut += exceptionMsg;
    }
    catch (...) {
        errorMsgOut = "An error occurred while reading the MIDI file! It may not be a valid or in a supported format.";
    }

    return bReadOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: read a MIDI file from a file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool readMidiFile(const char* const filePath, MidiFile& midiFileOut, std::string& errorMsgOut) noexcept {
    bool bReadFileOk = false;

    try {
        FileInputStream in(filePath);
        bReadFileOk = readMidiFile(in, midiFileOut, errorMsgOut);

        // If reading failed add the file name as additional context
        if (!bReadFileOk) {
            std::string errorPrefix = "Failed to read MIDI file '";
            errorPrefix += filePath;
            errorPrefix += "'! ";
            errorMsgOut.insert(errorMsgOut.begin(), errorPrefix.begin(), errorPrefix.end());
        }
    } catch (...) {
        errorMsgOut = "Failed to open MIDI file '";
        errorMsgOut += filePath;
        errorMsgOut += "' for reading! Does the file path exist and is it accessible?";
    }

    return bReadFileOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a MIDI set tempo command to the given output stream
//------------------------------------------------------------------------------------------------------------------------------------------
static void writeMidiSetTempoCmd(OutputStream& out, const uint32_t tempo) THROWS {
    out.write<uint8_t>(0xFFu);                                  // MIDI meta event
    out.write<MidiMetaCmdType>(MidiMetaCmdType::SetTempo);      // Set tempo meta command
    out.write<uint8_t>(3);                                      // 3 bytes of payload

    // Write the number of microseconds per quater note (24-bit value, big endian)
    const uint32_t microSecsPerQuarterNote = std::min<uint32_t>(60'000'000u / tempo, 0xFFFFFFu);
    out.write<uint8_t>((uint8_t)(microSecsPerQuarterNote >> 16));
    out.write<uint8_t>((uint8_t)((microSecsPerQuarterNote >> 8) & 0xFFu));
    out.write<uint8_t>((uint8_t)(microSecsPerQuarterNote & 0xFFu));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write the given track command to the specified output stream.
// If 'setTempoTo' is given a value other than '0' then a set tempo command will be written to the start of the track.
//------------------------------------------------------------------------------------------------------------------------------------------
static void writeMidiTrack(OutputStream& out, const MidiTrack& track, const uint32_t setTempoTo) THROWS {
    // Write the track name, if there is one
    if (!track.name.empty()) {
        writeVarLenQuant(out, 0);                                           // Zero delay to this command
        out.write<uint8_t>(0xFFu);                                          // MIDI meta event
        out.write<MidiMetaCmdType>(MidiMetaCmdType::SequenceTrackName);     // Meta command type
        writeVarLenQuant(out, (uint32_t) track.name.length());              // Command payload size
        out.writeArray<char>(track.name.data(), track.name.size());         // Command payload
    }

    // Do we need to write a command to set the tempo?
    if (setTempoTo > 0) {
        writeVarLenQuant(out, 0);                   // Zero delay to this command
        writeMidiSetTempoCmd(out, setTempoTo);      // Write the set tempo command itself
    }

    // Write each command
    for (const MidiCmd& cmd : track.cmds) {
        // Do not write unhandled meta command types
        if (cmd.type == MidiCmd::Type::UnhandledMetaCmd)
            continue;

        // Write the delay until the command in quarter note parts
        writeVarLenQuant(out, cmd.delayQnp);

        // Write the command itself
        if (cmd.type == MidiCmd::Type::EndOfTrack) {
            out.write<uint8_t>(0xFFu);                                      // MIDI meta event
            out.write<MidiMetaCmdType>(MidiMetaCmdType::EndOfTrack);        // Meta command type
            out.write<uint8_t>(0);                                          // Payload size
        }
        else if (cmd.type == MidiCmd::Type::NoteOn) {
            out.write<uint8_t>(uint8_t(0x90) | (cmd.channel & 0x0F));           // Command type and channel
            out.write<uint8_t>((uint8_t) std::min<uint16_t>(cmd.arg1, 127));    // Note
            out.write<uint8_t>((uint8_t) std::min<uint16_t>(cmd.arg2, 127));    // Velocity
        }
        else if (cmd.type == MidiCmd::Type::NoteOff) {
            out.write<uint8_t>(uint8_t(0x80) | (cmd.channel & 0x0F));           // Command type and channel
            out.write<uint8_t>((uint8_t) std::min<uint16_t>(cmd.arg1, 127));    // Note
            out.write<uint8_t>((uint8_t) std::min<uint16_t>(cmd.arg2, 127));    // Velocity
        }
        else if (cmd.type == MidiCmd::Type::SetVolume) {
            out.write<uint8_t>(uint8_t(0xB0) | (cmd.channel & 0x0F));           // Control change or channel 'mode' command + channel number
            out.write<MidiCcOrModeCmdType>(MidiCcOrModeCmdType::VolumeMsb);     // Set volume command (MSB)
            out.write<uint8_t>((uint8_t) std::min<uint16_t>(cmd.arg1, 127));    // The volume level
        }
        else if (cmd.type == MidiCmd::Type::SetPan) {
            out.write<uint8_t>(uint8_t(0xB0) | (cmd.channel & 0x0F));           // Control change or channel 'mode' command + channel number
            out.write<MidiCcOrModeCmdType>(MidiCcOrModeCmdType::PanMsb);        // Set pan command (MSB)
            out.write<uint8_t>((uint8_t) std::min<uint16_t>(cmd.arg1, 127));    // The pan level
        }
        else if (cmd.type == MidiCmd::Type::SetPitchBend) {
            out.write<uint8_t>(uint8_t(0xE0) | (cmd.channel & 0x0F));           // Pitch bend command + channel number

            // Write the pitch bend amount, oddly the least significant byte is first...
            const uint16_t pitchBend = std::min<uint16_t>(cmd.arg1, 0x3FFFu);
            out.write<uint8_t>((uint8_t)(pitchBend & 0x7Fu));
            out.write<uint8_t>((uint8_t)((pitchBend >> 7) & 0x7Fu));
        }
        else if (cmd.type == MidiCmd::Type::SetTempo) {
            writeMidiSetTempoCmd(out, cmd.arg1);
        }
        else {
            throw "Unable to write unknown command type to the stream!";
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a MIDI file to the given output stream
//------------------------------------------------------------------------------------------------------------------------------------------
bool writeMidiFile(OutputStream& out, const MidiFile& midiFile) noexcept {
    bool bWroteFileOk = false;

    try {
        // Firstly make up and write the header chunk
        {
            // Write the header for the chunk
            MidiFileChunkHdr hdrChunkHdr = {};
            hdrChunkHdr.id = MidiFileChunkId::MThd;
            hdrChunkHdr.size = sizeof(MidiFileHdr);
            hdrChunkHdr.endianCorrect();

            out.write(hdrChunkHdr);

            // Sanity check
            if (midiFile.tracks.size() > UINT16_MAX)
                return false;

            // Write the chunk payload - the file header
            MidiFileHdr fileHdr = {};
            fileHdr.format = 1;
            fileHdr.numTracks = (uint16_t) midiFile.tracks.size();
            fileHdr.timingInfo = (uint16_t) std::min<uint32_t>(midiFile.ppq, 0x7FFFu);
            fileHdr.endianCorrect();

            out.write(fileHdr);
        }

        // Is there a tempo command written at the start of one of the meta tracks?
        // If not then we will need to write one into the very first track...
        bool bNeedToSetTempo = (!midiFile.definesStartTempoInMetaTracks());

        // Write all tracks
        ByteVecOutputStream trackOut;
        trackOut.getBytes().reserve(64 * 1024);

        for (const MidiTrack& track : midiFile.tracks) {
            // Write the track data to the byte buffer
            trackOut.reset();
            writeMidiTrack(trackOut, track, (bNeedToSetTempo) ? midiFile.bpm : 0);
            bNeedToSetTempo = false;

            // Make up the header for this track chunk and write it
            MidiFileChunkHdr chunkHdr = {};
            chunkHdr.id = MidiFileChunkId::MTrk;
            chunkHdr.size = (uint32_t) trackOut.getBytes().size();
            chunkHdr.endianCorrect();

            out.write(chunkHdr);

            // Write the track data itself
            out.writeBytes(trackOut.getBytes().data(), trackOut.tell());
        }
        
        // Ensure all pending writes have completed to finish up
        out.flush();
        bWroteFileOk = true;
    } catch (...) {
        // Ignore...
    }

    return bWroteFileOk;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: write to a MIDI file on disk
//------------------------------------------------------------------------------------------------------------------------------------------
bool writeMidiFile(const char* const filePath, const MidiFile& midiFile) noexcept {
    try {
        FileOutputStream out(filePath, false);
        return writeMidiFile(out, midiFile);
    } catch (...) {
        return false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a MIDI variable length quantity from the given input stream.
// This version returns the quantity directly and does not tell how many bytes were read.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t readVarLenQuant(InputStream& in) THROWS {
    uint32_t quantity = {};
    readVarLenQuant(in, quantity);
    return quantity;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a MIDI variable length quantity from the given input stream.
// Returns number of bytes read and the output value, which may be up to 5 bytes.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t readVarLenQuant(InputStream& in, uint32_t& valueOut) THROWS {
    // Grab the first byte in the quantity
    uint8_t curByte = in.read<uint8_t>();
    uint32_t numBytesRead = 1;

    // The top bit set on each byte means there is another byte to follow.
    // Each byte can therefore only encode 7 bits, so we need 5 of them to encode 32-bits:
    uint32_t decodedVal = curByte & 0x7Fu;

    while (curByte & 0x80) {
        // Make room for more data
        decodedVal <<= 7;

        // Read the next byte and incorporate into the value
        curByte = in.read<uint8_t>();
        numBytesRead++;
        decodedVal |= (uint32_t) curByte & 0x7Fu;

        // Sanity check, there should only be at most 5 bytes!
        if (numBytesRead > 5)
            throw "Read VLQ: too many bytes! Quantity encoding is not valid!";
    }

    valueOut = decodedVal;
    return numBytesRead;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Write a MIDI variable length quantity to the given output stream.
// Returns number of bytes written, which may be up to 5 bytes.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t writeVarLenQuant(OutputStream& out, const uint32_t valueIn) THROWS {
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
        out.write(*pEncodedByte);
        numBytesWritten++;
    } while (*pEncodedByte & 0x80);

    return numBytesWritten;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells how many bytes would be used to encode a 32-bit value using MIDI variable length quantity encoding.
// The returned answer will be between 1-5 bytes.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t getVarLenQuantLen(const uint32_t valueIn) noexcept {
    uint32_t numEncodedBytes = 1;
    uint32_t bitsLeftToEncode = valueIn >> 7;

    while (bitsLeftToEncode != 0) {
        numEncodedBytes++;
        bitsLeftToEncode >>= 7;
    }

    return numEncodedBytes;
}

END_NAMESPACE(MidiUtils)
END_NAMESPACE(AudioTools)
