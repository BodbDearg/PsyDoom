#pragma once

#include "CpuOpcode.h"
#include <ostream>

struct ExeFile;
struct ProgElem;

//----------------------------------------------------------------------------------------------------------------------
// Represents a decoded MIPS instruction
//----------------------------------------------------------------------------------------------------------------------
struct CpuInstruction {
    // What the instruction operation is.
    // Determines the meaning of the registers and immediate values also.
    CpuOpcode opcode;

    // Registers 'S', 'T' and 'D' for the instruction.
    // Depending on the opcode, some or all of these may be unused.
    uint8_t regS;
    uint8_t regT;
    uint8_t regD;

    // A 32-bit immediate value which may or may not be used by the opcode.
    // A varying number of bits will also be used in this immediate value, depending on the instruction.
    // Note that sign extension and other adjustments are NOT performed during the decoding stage, this
    // just holds the raw value of what was decoded:
    uint32_t immediateVal;

    // Clear the instruction to be an illegal instruction.
    // All values are given poison values so if they are used then we might see problems.
    inline constexpr void clear() noexcept  {
        opcode = CpuOpcode::INVALID;
        regS = 0xDBu;
        regT = 0xDBu;
        regD = 0xDBu;
        immediateVal = 0xDEADBEEF;
    }

    // Tell if this is an illegal/invalid instruction
    inline constexpr bool isIllegal() const noexcept {
        return CpuOpcodeUtils::isIllegalOpcode(opcode);
    }

    // Make and return a NOP instruction.
    // Mips doesn't have a dedicated NOP instruction, however 'SLL $zero $zero 0' is commonly used for that purpose.
    static CpuInstruction makeNOP() noexcept {
        CpuInstruction inst;
        inst.opcode = CpuOpcode::SLL;
        inst.regS = 0;
        inst.regT = 0;
        inst.regD = 0;
        inst.immediateVal = 0;

        return inst;
    }

    // Do the hard work of actually decoding the instruction from a 32-bit machine word.
    // The word is assumed to be in little endian format.
    // If decoding fails then the instruction will be made into an illegal instruction and 'false' will be returned.
    // Note: all unused values are also given poison values to detect their misuse!
    bool decode(const uint32_t machineCode) noexcept;

    // Get the index of the destination general purpose register (GPR) for the instruction, from 0-31.
    // Returns '0xFF' if the instruction does not output to a general purpose register.
    uint8_t getDestGprIdx() const noexcept;

    // Get the index of the 1st input general purpose register (GPR) for the instruction, from 0-31.
    // Returns '0xFF' if the instruction does not have at least 1 input general purpose register.
    uint8_t getInputGprIdx1() const noexcept;

    // Get the index of the 2nd input general purpose register (GPR) for the instruction, from 0-31.
    // Returns '0xFF' if the instruction does not have at least 2 input general purpose registers.
    uint8_t getInputGprIdx2() const noexcept;

    // Tell if the given general purpose register (specified by index) is one of the input general purpose registers for the instruction.
    // Useful for analyzing register dependencies between instructions.
    bool isInputGprIdx(const uint8_t gprIdx) const noexcept;

    // Tell if this instruction is effectively a NOP.
    // Certain instructions like 'SLL r0, r0, 0' (encoded as binary '0') are used as NOP type instructions.
    // NOPs are used in some places in the code to get around issues with branch and load delay slots by delaying the CPU.
    bool isNOP() const noexcept;

    // Get the target address for this instruction if it is a branch type instruction.
    // The instruction must be given it's address in the program in order to calculate this.
    uint32_t getBranchInstTargetAddr(const uint32_t thisInstAddr) const noexcept;
    
    // Get the target address for this instruction if it is a jump with a constant/fixed address target.
    // This CANNOT be used obviously for 'jump to address in register' type instructions.
    uint32_t getFixedJumpInstTargetAddr(const uint32_t thisInstAddr) const noexcept;

    // Print the instruction to the given string buffer.
    // The instruction must be given it's address in the program in order to print (for relative jumps etc.)
    void print(
        const ExeFile& exe,
        const uint32_t thisInstAddr,
        const ProgElem* const pParentFunc,
        std::ostream& out
    ) const;

    inline int compare(const CpuInstruction& other) const noexcept {
        if (opcode != other.opcode) {
            return (opcode < other.opcode) ? -1 : +1;
        }

        if (regS != other.regS) {
            return (regS < other.regS) ? -1 : +1;
        }

        if (regT != other.regT) {
            return (regT < other.regT) ? -1 : +1;
        }

        if (regD != other.regD) {
            return (regD < other.regD) ? -1 : +1;
        }

        if (immediateVal != other.immediateVal) {
            return (immediateVal < other.immediateVal) ? -1 : +1;
        }

        return 0;
    }

    inline bool operator < (const CpuInstruction& other) const noexcept {
        return (compare(other) < 0);
    }

    inline bool operator <= (const CpuInstruction& other) const noexcept {
        return (compare(other) <= 0);
    }

    inline bool operator > (const CpuInstruction& other) const noexcept {
        return (compare(other) > 0);
    }

    inline bool operator >= (const CpuInstruction& other) const noexcept {
        return (compare(other) >= 0);
    }

    inline bool operator != (const CpuInstruction& other) const noexcept {
        return (compare(other) != 0);
    }

    inline bool operator == (const CpuInstruction& other) const noexcept {
        return (compare(other) == 0);
    }
};
