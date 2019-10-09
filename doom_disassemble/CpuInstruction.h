#include "CpuOpcode.h"

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
    inline constexpr bool isIllegal() const {
        return CpuOpcodeUtils::isIllegalOpcode(opcode);
    }

    // Do the hard work of actually decoding the instruction from a 32-bit machine word.
    // The word is assumed to be in little endian format.
    // If decoding fails then the instruction will be made into an illegal instruction and 'false' will be returned.
    // Note: all unused values are also given poison values to detect their misuse!
    bool decode(const uint32_t machineCode) noexcept;
};
