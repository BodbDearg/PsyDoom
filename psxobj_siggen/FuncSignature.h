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

    // Utilities to print function signatures to the given output stream.
    // Each signature is separated by a new line.
    void printSig(const FuncSignature& sig, std::ostream& out) noexcept;
    void printSigList(const std::vector<FuncSignature>& sigs, std::ostream& out) noexcept;

    // Print disassembly for each function signature to the given output stream.
    // Each piece of disassembly is separated by a new line.
    void printSigDisassembly(const FuncSignature& sig, std::ostream& out) noexcept;
    void printSigListDisassembly(const std::vector<FuncSignature>& sigs, std::ostream& out) noexcept;
}
