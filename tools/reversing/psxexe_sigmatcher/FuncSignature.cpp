#include "FuncSignature.h"

#include "TextIStream.h"

void FuncSignatureUtils::readSigFromTextStream(TextIStream& in, FuncSignature& out) {
    // Read name and trim end of line marker
    {
        TextIStream line = in.readNextLineAsStream();
        out.name.assign(line.str, line.endOffset);
        in.skipAsciiWhiteSpace();

        while (!out.name.empty()) {
            const char c = out.name.back();

            if (c == '\r' || c == '\n') {
                out.name.pop_back();
            } else {
                break;
            }
        }
    }

    // Read instruction count
    const uint32_t numInstructions = in.readDecimalUint();
    in.skipAsciiWhiteSpace();

    // Read number of wildcard instructions
    const uint32_t numWildcardInstructions = in.readDecimalUint();
    in.skipAsciiWhiteSpace();

    // Read each instruction
    for (uint32_t i = numInstructions; i > 0; --i) {
        const uint32_t instructionWord = in.readHexUint();
        in.skipAsciiWhiteSpace();

        CpuInstruction instruction;
        instruction.decode(instructionWord);
        out.instructions.push_back(instruction);
    }

    // Read wildcard instruction indexes
    out.bInstructionIsPatched.resize(out.instructions.size());

    for (uint32_t i = numWildcardInstructions; i > 0; --i) {
        const uint32_t index = in.readDecimalUint();
        in.skipAsciiWhiteSpace();

        if (index < out.bInstructionIsPatched.size()) {
            out.bInstructionIsPatched[index] = true;
        }
    }
}

void FuncSignatureUtils::readSigsFromTextStream(TextIStream& in, std::vector<FuncSignature>& out) {
    while (!in.isAtEnd()) {
        if (in.skipAsciiWhiteSpace()) {
            continue;
        }

        FuncSignature& sig = out.emplace_back();
        readSigFromTextStream(in, sig);
    }
}
