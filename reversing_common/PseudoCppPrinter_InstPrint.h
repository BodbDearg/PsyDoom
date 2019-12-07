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
    void printInst_and(std::ostream& out, const CpuInstruction& inst);
    void printInst_andi(std::ostream& out, const CpuInstruction& inst);
    void printInst_lui(std::ostream& out, const CpuInstruction& inst);
    void printInst_or(std::ostream& out, const CpuInstruction& inst);
    void printInst_ori(std::ostream& out, const CpuInstruction& inst);
    void printInst_sll(std::ostream& out, const CpuInstruction& inst);
    void printInst_sra(std::ostream& out, const CpuInstruction& inst);
    void printInst_srl(std::ostream& out, const CpuInstruction& inst);
    void printInst_subu(std::ostream& out, const CpuInstruction& inst);
    void printInst_xori(std::ostream& out, const CpuInstruction& inst);
}
