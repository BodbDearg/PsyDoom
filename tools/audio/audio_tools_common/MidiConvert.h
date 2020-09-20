#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(AudioTools)

struct MidiFile;
struct Sequence;

BEGIN_NAMESPACE(MidiConvert)

void sequenceToMidi(const Sequence& sequence, MidiFile& midiFile) noexcept;
void midiToSequence(const MidiFile& midiFile, Sequence& sequence, const bool bIsMusic) noexcept;

END_NAMESPACE(MidiConvert)
END_NAMESPACE(AudioTools)
