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
    // Whether it is interpreted as signed or unsigned or whether it even matters depends on the instruction.
    // Note: sign extension will be performed on instruction decoding also where appropriate!
    uint32_t immediateVal;

    // Clear the instruction to be an illegal instruction
    inline constexpr void clear() noexcept  {
        opcode = CpuOpcode::INVALID;
        regS = 0;
        regT = 0;
        regD = 0;
        immediateVal = 0;
    }

    // Tell if this is an illegal/invalid instruction
    inline constexpr bool isIllegal() const {
        return CpuOpcodeUtils::isIllegalOpcode(opcode);
    }

    // Do the hard work of actually decoding the instruction from a 32-bit machine word.
    // The word is assumed to be in little endian format.
    // If decoding fails then the instruction will be made into an illegal instruction and 'false' will be returned.
    bool decode(const uint32_t machineCode) noexcept;
};
