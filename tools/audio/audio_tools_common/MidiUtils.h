#pragma once

#include "Macros.h"

#include <cstdint>
#include <string>

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

struct MidiFile;

BEGIN_NAMESPACE(MidiUtils)

bool readMidiFile(InputStream& in, MidiFile& midiFileOut, std::string& errorMsgOut) noexcept;
bool readMidiFile(const char* const filePath, MidiFile& midiFileOut, std::string& errorMsgOut) noexcept;
bool writeMidiFile(OutputStream& out, const MidiFile& midiFile) noexcept;
bool writeMidiFile(const char* const filePath, const MidiFile& midiFile) noexcept;
uint32_t readVarLenQuant(InputStream& in) THROWS;
uint32_t readVarLenQuant(InputStream& in, uint32_t& valueOut) THROWS;
uint32_t writeVarLenQuant(OutputStream& out, const uint32_t valueIn) THROWS;
uint32_t getVarLenQuantLen(const uint32_t valueIn) noexcept;

END_NAMESPACE(MidiUtils)
END_NAMESPACE(AudioTools)
