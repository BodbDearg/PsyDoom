#pragma once

#include <string>
#include <vector>
#include <ostream>

struct ObjFile;

//----------------------------------------------------------------------------------------------------------------------
// Describes a function signature for exporting to a file
//----------------------------------------------------------------------------------------------------------------------
struct FuncSignature {
    std::string             name;
    std::vector<uint32_t>   instructions;
    std::vector<uint32_t>   wildcardInstructionIndexes;
};

namespace FuncSignatureUtils {
    // Build a list of function signatures to export from the given parsed obj file dump
    void buildSigList(ObjFile& objFile, std::vector<FuncSignature>& signatures) noexcept;

    // Print each function signature to the given output stream.
    // Each signature is separated by a new line.
    void printSigList(const std::vector<FuncSignature>& signatures, std::ostream& out) noexcept;
}
