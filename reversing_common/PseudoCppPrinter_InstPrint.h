#pragma once

#include <cstdint>
#include <ostream>

struct CpuInstruction;

//----------------------------------------------------------------------------------------------------------------------
// Manually handled logic for various instructions for the pseudo c++ printer.
// For instructions we want to simplify/cleanup as much as possible.
//----------------------------------------------------------------------------------------------------------------------
namespace PseudoCppPrinter {
    void printInst_addiu(std::ostream& out, const CpuInstruction& inst);
    void printInst_addu(std::ostream& out, const CpuInstruction& inst);
}
