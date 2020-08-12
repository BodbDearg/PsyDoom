#pragma once

#include "CpuInstruction.h"

#include <string>
#include <vector>
#include <ostream>

struct TextIStream;

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a function signature read from a signature file
//------------------------------------------------------------------------------------------------------------------------------------------
struct FuncSignature {
    std::string                     name;
    std::vector<CpuInstruction>     instructions;
    std::vector<bool>               bInstructionIsPatched;      // If true then only match on instruction type because linker patches it (wildcard instruction)
};

namespace FuncSignatureUtils {
    void readSigFromTextStream(TextIStream& in, FuncSignature& out);
    void readSigsFromTextStream(TextIStream& in, std::vector<FuncSignature>& out);
}
