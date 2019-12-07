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
    void printInst_mfhi(std::ostream& out, const CpuInstruction& inst);
    void printInst_mflo(std::ostream& out, const CpuInstruction& inst);
    void printInst_nor(std::ostream& out, const CpuInstruction& inst);
    void printInst_or(std::ostream& out, const CpuInstruction& inst);
    void printInst_ori(std::ostream& out, const CpuInstruction& inst);
    void printInst_sll(std::ostream& out, const CpuInstruction& inst);
    void printInst_sllv(std::ostream& out, const CpuInstruction& inst);
    void printInst_slt(std::ostream& out, const CpuInstruction& inst);
    void printInst_slti(std::ostream& out, const CpuInstruction& inst);
    void printInst_sltiu(std::ostream& out, const CpuInstruction& inst);
    void printInst_sltu(std::ostream& out, const CpuInstruction& inst);
    void printInst_sra(std::ostream& out, const CpuInstruction& inst);
    void printInst_srav(std::ostream& out, const CpuInstruction& inst);
    void printInst_srl(std::ostream& out, const CpuInstruction& inst);
    void printInst_srlv(std::ostream& out, const CpuInstruction& inst);
    void printInst_subu(std::ostream& out, const CpuInstruction& inst);
    void printInst_xor(std::ostream& out, const CpuInstruction& inst);
    void printInst_xori(std::ostream& out, const CpuInstruction& inst);
}
