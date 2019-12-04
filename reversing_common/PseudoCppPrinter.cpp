#include "PseudoCppPrinter.h"

#include "ConstInstructionEvaluator.h"
#include "CpuInstruction.h"
#include "ExeFile.h"
#include "FatalErrors.h"
#include "InstructionCommenter.h"
#include "PrintUtils.h"
#include <algorithm>

static ConstInstructionEvaluator gConstInstructionEvaluator;

//------------------------------------------------------------------------------------------------------------------------------------------
// Printing C++ literals of various integer types
//------------------------------------------------------------------------------------------------------------------------------------------
static void printHexCppInt16Literal(const int16_t valI16, bool bZeroPad, std::ostream& out) noexcept {
    const int64_t valI64 = valI16;

    if (valI64 < 0) {
        out << "-0x";
        PrintUtils::printHexU16((uint16_t)(-valI64), bZeroPad, out);
    } else {
        out << "0x";
        PrintUtils::printHexU16((uint16_t) valI64, bZeroPad, out);
    }
}

static void printHexCppUint16Literal(const uint16_t valU16, bool bZeroPad, std::ostream& out) noexcept {    
    out << "0x";
    PrintUtils::printHexU16(valU16, bZeroPad, out);
}

static void printHexCppUint32Literal(const uint32_t valU32, bool bZeroPad, std::ostream& out) noexcept {    
    out << "0x";
    PrintUtils::printHexU32(valU32, bZeroPad, out);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the C++ macro name for the given general purpose register
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* getGprCppMacroName(const uint8_t gprIdx) noexcept {
    switch (gprIdx) {
        case CpuGpr::ZERO:  return "zero";
        case CpuGpr::AT:    return "at";
        case CpuGpr::V0:    return "v0";
        case CpuGpr::V1:    return "v1";
        case CpuGpr::A0:    return "a0";
        case CpuGpr::A1:    return "a1";
        case CpuGpr::A2:    return "a2";
        case CpuGpr::A3:    return "a3";
        case CpuGpr::T0:    return "t0";
        case CpuGpr::T1:    return "t1";
        case CpuGpr::T2:    return "t2";
        case CpuGpr::T3:    return "t3";
        case CpuGpr::T4:    return "t4";
        case CpuGpr::T5:    return "t5";
        case CpuGpr::T6:    return "t6";
        case CpuGpr::T7:    return "t7";
        case CpuGpr::S0:    return "s0";
        case CpuGpr::S1:    return "s1";
        case CpuGpr::S2:    return "s2";
        case CpuGpr::S3:    return "s3";
        case CpuGpr::S4:    return "s4";
        case CpuGpr::S5:    return "s5";
        case CpuGpr::S6:    return "s6";
        case CpuGpr::S7:    return "s7";
        case CpuGpr::T8:    return "t8";
        case CpuGpr::T9:    return "t9";
        case CpuGpr::K0:    return "k0";
        case CpuGpr::K1:    return "k1";
        case CpuGpr::GP:    return "gp";
        case CpuGpr::SP:    return "sp";
        case CpuGpr::FP:    return "fp";
        case CpuGpr::RA:    return "ra";

        default: return "INVALID_REG";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction that assigns to a GPR and that has two input GPR arguments
//------------------------------------------------------------------------------------------------------------------------------------------
static void printInstGprOutGprInGprIn(
    const CpuInstruction& ins,    
    const uint8_t in1Gpr,
    const uint8_t in2Gpr,
    std::ostream& out
) {
    const uint8_t destGpr = ins.getDestGprIdx();
    out << getGprCppMacroName(destGpr);
    out << " = ";
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out << "(";
    out << getGprCppMacroName(in1Gpr);
    out << ", ";
    out << getGprCppMacroName(in2Gpr);
    out << ");";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction that assigns to a GPR and that has an input GPR arg followed by an I16 constant
//------------------------------------------------------------------------------------------------------------------------------------------
static void printInstGprOutGprInI16In(    
    const CpuInstruction& ins,
    const uint8_t in1Gpr,
    const int16_t in2I16,
    std::ostream& out
) {
    const uint8_t destGpr = ins.getDestGprIdx();
    out << getGprCppMacroName(destGpr);
    out << " = ";
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out.put('(');
    out << getGprCppMacroName(in1Gpr);
    out << ", ";
    printHexCppInt16Literal(in2I16, false, out);
    out << ");";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction that assigns to a GPR and that has an input GPR arg followed by an U16 constant
//------------------------------------------------------------------------------------------------------------------------------------------
static void printInstGprOutGprInU16In(    
    const CpuInstruction& ins,
    const uint8_t in1Gpr,
    const uint16_t in2U16,
    std::ostream& out
) {
    const uint8_t destGpr = ins.getDestGprIdx();
    out << getGprCppMacroName(destGpr);
    out << " = ";
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out.put('(');
    out << getGprCppMacroName(in1Gpr);
    out << ", ";
    printHexCppUint16Literal(in2U16, false, out);
    out << ");";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction that assigns to a GPR and that an input U16 constant
//------------------------------------------------------------------------------------------------------------------------------------------
static void printInstGprOutU16In(    
    const CpuInstruction& ins,
    const uint16_t inU16,
    std::ostream& out
) {
    const uint8_t destGpr = ins.getDestGprIdx();
    out << getGprCppMacroName(destGpr);
    out << " = ";
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out.put('(');
    printHexCppUint16Literal(inU16, false, out);
    out << ");";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction has two input GPRs and an input I16 constant
//------------------------------------------------------------------------------------------------------------------------------------------
static void printInstGprInGprInI16In(
    const CpuInstruction& ins,    
    const uint8_t in1Gpr,
    const uint8_t in2Gpr,
    const int16_t in3I16,
    std::ostream& out
) {
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out << "(";
    out << getGprCppMacroName(in1Gpr);
    out << ", ";
    out << getGprCppMacroName(in2Gpr);
    out << ", ";
    printHexCppInt16Literal(in3I16, false, out);
    out << ");";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction has two input GPRs and no output
//------------------------------------------------------------------------------------------------------------------------------------------
static void printInstGprInGprIn(
    const CpuInstruction& ins,    
    const uint8_t in1Gpr,
    const uint8_t in2Gpr,
    std::ostream& out
) {
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out << "(";
    out << getGprCppMacroName(in1Gpr);
    out << ", ";
    out << getGprCppMacroName(in2Gpr);
    out << ");";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction that takes a single U32 constant as input and has no outputs
//------------------------------------------------------------------------------------------------------------------------------------------
static void printInstU32In(    
    const CpuInstruction& ins,
    const uint32_t inU32,
    std::ostream& out
) {
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out.put('(');
    printHexCppUint32Literal(inU32, false, out);
    out << ");";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction that has no inputs and no output
//------------------------------------------------------------------------------------------------------------------------------------------
static void printInstNoParamNoReturn(const CpuInstruction& ins, std::ostream& out) {
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out << "();";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Indent by a specified number of characters
//------------------------------------------------------------------------------------------------------------------------------------------
static void indentByNumChars(const uint32_t numChars, std::ostream& out) {
    for (uint32_t i = 0; i < numChars; ++i) {
        out.put(' ');
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add the prefix for the beginning of a comment
//------------------------------------------------------------------------------------------------------------------------------------------
static void prefixInstructionComment(const uint32_t lineCol, std::ostream& out) {
    // Figure out the start column for the comment
    constexpr uint32_t minCommentStartCol = 48u;
    uint32_t commentStartCol = std::max(minCommentStartCol, lineCol);

    while (commentStartCol % 4 != 0) {
        ++commentStartCol;
    }

    // Insert the required number of spaces up until the start column
    uint32_t numSpacesToInsert = commentStartCol - lineCol;

    while (numSpacesToInsert > 0) {
        out.put(' ');
        --numSpacesToInsert;
    }

    // Start of comment
    out << "// ";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print the name of a program element
//------------------------------------------------------------------------------------------------------------------------------------------
static void printProgElemName(const ProgElem& progElem, const char* const defaultNamePrefix, std::ostream& out) {
    if (progElem.name != nullptr && progElem.name[0] != 0) {
        out << progElem.name;
    } else {
        out << defaultNamePrefix;
        PrintUtils::printHexU32(progElem.startAddr, true, out);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verifies that 'nextInst' is not dependent on the load being performed by 'thisInst' being delayed.
// By this I mean we check to make sure that 'nextInst' does not require the old/stale value of the destination GPR loaded to by 'thisInst'.
// I don't handle MIPS I load delay slots in the conversion to C++, and this verifies that we basically don't have to worry about those.
//------------------------------------------------------------------------------------------------------------------------------------------
static void verifyNoDependencyOnDelayedLoad(const CpuInstruction& thisInst, const CpuInstruction& nextInst) noexcept {
    // This the current instruction one that loads a value from RAM? (has a load delay slot)
    // If not then we do not have to worry about it...
    if (!CpuOpcodeUtils::isLoadDelaySlotOpcode(thisInst.opcode))
        return;

    // Get the destination GPR for the load instruction, and verify it is not an input GPR for the next instruction.
    // If the next instruciton requres this GPR that is being loaded to, then it should have the OLD value according
    // to how MIPS I behaves with regards to it's load delay slots. We don't handle this situation, so raise an error
    // when it is detected....
    const uint8_t loadGpr = thisInst.getDestGprIdx();

    if (nextInst.isInputGprIdx(loadGpr)) {
        FATAL_ERROR(
            "Detected a dependency on delayed load being performed! "
            "We don't support this in the conversion to C++ currently... "
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verifies that both 'thisInst' and 'nextInst' are not both branch or jump instructions.
// I don't know how MIPS handles a branch/jump instruction in the branch delay slot of another branch/jump instruction - perhaps that is
// illegal and not allowed? In any case, this function will sanity check that this is not occuring in the code...
//------------------------------------------------------------------------------------------------------------------------------------------
static void verifyNoAdjacentBranchesOrJumps(const CpuInstruction& thisInst, const CpuInstruction& nextInst) noexcept {
    const bool bDisallowedInstructionPair = (
        CpuOpcodeUtils::isBranchOrJumpOpcode(thisInst.opcode) &&
        CpuOpcodeUtils::isBranchOrJumpOpcode(nextInst.opcode)
    );

    if (bDisallowedInstructionPair) {
        FATAL_ERROR(
            "Found adjacent branch or jump instructions! Having a branch or jump instruction in the branch delay slot "
            "of another branch or jump instruction is NOT supported!"
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given instruction requires a label for a 'goto' statement.
// This will be the case for instructions that are branch targets or referenced by jump tables etc.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool doesInstructionRequireAGotoLabel(const ExeFile& exe, const uint32_t instAddr) noexcept {
    const uint32_t exeWordIdx = (instAddr - exe.baseAddress) / 4;
    assert(exeWordIdx < exe.sizeInWords);
    const ExeWord& word = exe.words[exeWordIdx];
    return (word.bIsBranchTarget || word.bIsDataReferenced || word.bIsJumpTarget);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print an instruction that is NOT a branch or jump instruction
//------------------------------------------------------------------------------------------------------------------------------------------
static void printNonBranchOrJumpInstruction(
    const ExeFile& exe,
    const CpuInstruction inst,
    const uint32_t instAddr,
    const uint32_t indent,
    std::ostream& out
) {
    // Print the instruction goto label if it requires it, first
    if (doesInstructionRequireAGotoLabel(exe, instAddr)) {
        out << "loc_";
        PrintUtils::printHexU32(instAddr, true, out);
        out << ":\n";
    }

    // If the instruction is a nop then stop here
    if (inst.isNOP())
        return;
    
    // Log where we are in the stream (so we can tell how long the instruction was when printed)
    const int64_t instructionStartStreamPos = out.tellp();   

    // Indent the instruction
    indentByNumChars(indent, out);

    // Print the instruction firstly
    switch (inst.opcode) {
        // REG_OUT = OPERATION(regS, regT)
        case CpuOpcode::ADD:
        case CpuOpcode::ADDU:
        case CpuOpcode::AND:
        case CpuOpcode::NOR:
        case CpuOpcode::OR:
        case CpuOpcode::SLT:
        case CpuOpcode::SLTU:
        case CpuOpcode::SUB:
        case CpuOpcode::SUBU:
        case CpuOpcode::XOR:
            printInstGprOutGprInGprIn(inst, inst.regS, inst.regT, out);
            break;

        // REG_OUT = OPERATION(regT, regS)
        case CpuOpcode::SLLV:
        case CpuOpcode::SRAV:
        case CpuOpcode::SRLV:
            printInstGprOutGprInGprIn(inst, inst.regT, inst.regS, out);
            break;

        // REG_OUT = OPERATION(regS, (int16_t) immVal)
        case CpuOpcode::ADDI:
        case CpuOpcode::ADDIU:
        case CpuOpcode::SLTI:
        case CpuOpcode::LB:
        case CpuOpcode::LBU:
        case CpuOpcode::LH:
        case CpuOpcode::LHU:
        case CpuOpcode::LW:
            printInstGprOutGprInI16In(inst, inst.regS, (int16_t) inst.immediateVal, out);
            break;

        // REG_OUT = OPERATION(regS, (uint16_t) immVal)
        case CpuOpcode::ANDI:
        case CpuOpcode::ORI:
        case CpuOpcode::SLTIU:
        case CpuOpcode::XORI:
            printInstGprOutGprInU16In(inst, inst.regS, (uint16_t) inst.immediateVal, out);
            break;

        // REG_OUT = OPERATION(regT, (uint16_t) immVal)
        case CpuOpcode::SLL:
        case CpuOpcode::SRA:
        case CpuOpcode::SRL:
            printInstGprOutGprInU16In(inst, inst.regT, (uint16_t) inst.immediateVal, out);
            break;

        // REG_OUT = OPERATION((uint16_t) immVal)
        case CpuOpcode::LUI:
            printInstGprOutU16In(inst, (uint16_t) inst.immediateVal, out);
            break;

        // OPERATION(regS, regT)
        case CpuOpcode::DIV:
        case CpuOpcode::DIVU:
        case CpuOpcode::MULT:
        case CpuOpcode::MULTU:
            printInstGprInGprIn(inst, inst.regS, inst.regT, out);
            break;

        // OPERATION(regT, regS, (int16_t) immVal)
        case CpuOpcode::SB:
        case CpuOpcode::SH:
        case CpuOpcode::SW:
            printInstGprInGprInI16In(inst, inst.regT, inst.regS, (int16_t) inst.immediateVal, out);
            break;

        // OPERATION((uint32_t) immVal)
        case CpuOpcode::BREAK:
        case CpuOpcode::COP2:
        case CpuOpcode::SYSCALL:
            printInstU32In(inst, inst.immediateVal, out);
            break;

        // Instructions with no inputs or outputs
        case CpuOpcode::RFE:
        case CpuOpcode::TLBP:
        case CpuOpcode::TLBR:
        case CpuOpcode::TLBWI:
        case CpuOpcode::TLBWR:
            printInstNoParamNoReturn(inst, out);
            break;

        // Special cases
        case CpuOpcode::CFC2:
            printInstGprOutU16In(inst, (uint16_t) inst.regD, out);
            break;

    /*
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE CONTROL WORD TO COPROCESSOR 2]
    //      Move a value from register 'T' and save to coprocessor 2 control register 'D':
    //          COP2_C[D] = T
    //
    // Encoding: 010010 00110 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    CTC2,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD WORD TO COPROCESSOR 2]
    //      Load the contents of the ALIGNED 32-bit word pointed to by register 'S' plus the 16-bit SIGNED constant
    //      offset 'I' and store in coprocessor 2 register 'T':
    //          COP2[T] = S[I]
    //
    // Encoding: 110010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LWC2,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD WORD LEFT]
    //      Makeup a POTENTIALLY UNALIGNED memory address 'A' by adding the address in register 'S' plus the 16-bit
    //      SIGNED constant offset 'I'. Then forcefully word align 'A' by truncating 2 bits to form address 'AL'.
    //      Replace the MOST SIGNIFICANT bytes in output register 'T' which are in the word pointed to by 'AL' and
    //      which are <= address 'A'. All other bytes remain unchanged.
    //      
    //      Pseudocode:
    //          A = S + I
    //          AL = A & FFFFFFFC
    //
    //          switch (A % 4)
    //              case 0:     T = (T & 00FFFFFF) | (AL[0] << 24)
    //              case 1:     T = (T & 0000FFFF) | (AL[0] << 16)
    //              case 2:     T = (T & 000000FF) | (AL[0] << 8)
    //              case 3:     T = (T & 00000000) | (AL[0] << 0)
    //
    // Encoding: 100010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LWL,
    //------------------------------------------------------------------------------------------------------------------
    // [LOAD WORD RIGHT]
    //      Makeup a POTENTIALLY UNALIGNED memory address 'A' by adding the address in register 'S' plus the 16-bit
    //      SIGNED constant offset 'I'. Then forcefully word align 'A' by truncating 2 bits to form address 'AL'.
    //      Replace the LEAST SIGNIFICANT bytes in output register 'T' which are in the word pointed to by 'AL' and
    //      which are >= address 'A'. All other bytes remain unchanged.
    //
    //      Pseudocode:
    //          A = S + I
    //          AL = A & FFFFFFFC
    //
    //          switch (A % 4)
    //              case 0:     T = (T & 00000000) | (AL[0] >> 0)
    //              case 1:     T = (T & FF000000) | (AL[0] >> 8)
    //              case 2:     T = (T & FFFF0000) | (AL[0] >> 16)
    //              case 3:     T = (T & FFFFFF00) | (AL[0] >> 24)
    //
    // Encoding: 100110 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    LWR,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE FROM COPROCESSOR 0]
    //      Move a value from a coprocessor 0 register 'D' and save the result in register 'T':
    //          T = COP0[D]
    //
    // Encoding: 010000 00000 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    MFC0,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE FROM COPROCESSOR 2]
    //      Move a value from a coprocessor 2 register 'D' and save the result in register 'T':
    //          T = COP2[D]
    //
    // Encoding: 010010 00000 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    MFC2,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE FROM HI REGISTER]
    //      Copy the value in the special purpose 'HI' register to the given register 'D':
    //          D = HI
    //
    //      IMPORTANT: there are MANY restrictions to when values can be moved TO and FROM this register and what
    //      causes unpredictable behavior. See the MIPS instruction set reference for more details.
    //
    // Encoding: 000000 ----- ----- DDDDD ----- 010000
    //------------------------------------------------------------------------------------------------------------------
    MFHI,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE FROM LO REGISTER]
    //      Copy the value in the special purpose 'LO' register to the given register 'D':
    //          D = LO
    //
    //      IMPORTANT: there are MANY restrictions to when values can be moved TO and FROM this register and what
    //      causes unpredictable behavior. See the MIPS instruction set reference for more details.
    //
    // Encoding: 000000 ----- ----- DDDDD ----- 010010
    //------------------------------------------------------------------------------------------------------------------
    MFLO,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE TO COPROCESSOR 0]
    //      Move a value to a coprocessor 0 register 'D' from register 'T':
    //          COP0[D] = T
    //
    // Encoding: 010000 00100 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    MTC0,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE TO COPROCESSOR 2]
    //      Move a value to a coprocessor 2 register 'D' from register 'T':
    //          COP2[D] = T
    //
    // Encoding: 010010 00100 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------
    MTC2,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE TO HI REGISTER]
    //      Move a value from register 'S' to the special purpose 'HI' register:
    //          HI = S
    //
    //      IMPORTANT: there are MANY restrictions to when values can be moved TO and FROM this register and what
    //      causes unpredictable behavior. See the MIPS instruction set reference for more details.
    //
    // Encoding: 000000 SSSSS ----- ----- ----- 010001
    //------------------------------------------------------------------------------------------------------------------
    MTHI,
    //------------------------------------------------------------------------------------------------------------------
    // [MOVE TO LO REGISTER]
    //      Move a value from register 'S' to the special purpose 'LO' register:
    //          LO = S
    //
    //      IMPORTANT: there are MANY restrictions to when values can be moved TO and FROM this register and what
    //      causes unpredictable behavior. See the MIPS instruction set reference for more details.
    //
    // Encoding: 000000 SSSSS ----- ----- ----- 010011
    //------------------------------------------------------------------------------------------------------------------
    MTLO,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE WORD FROM COPROCESSOR 2]
    //      Store the contents of coprocessor 2 register 'T' to the ALIGNED 32-bit word pointed to by register 'S' plus
    //      the 16-bit SIGNED constant offset 'I':
    //          S[I] = COP2[T]
    //
    // Encoding: 111010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SWC2,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE WORD LEFT]
    //      Makeup a POTENTIALLY UNALIGNED memory address 'A' by adding the address in register 'S' plus the 16-bit
    //      SIGNED constant offset 'I'. Then forcefully word align 'A' by truncating 2 bits to form address 'AL'.
    //      Store a varying number of the MOST SIGNIFICANT bytes in register 'T' to the LEAST SIGNIFICANT bytes of
    //      address 'AL' based on the alignment of address 'A'. Note: all other bytes remain unchanged.
    //
    //      The 4 possible cases are outlined below:
    //          A = S + I
    //          AL = A & FFFFFFFC
    //
    //          switch (A % 4)
    //              case 0:     AL[0] = (AL[0] & FFFFFF00) | (T >> 24)
    //              case 1:     AL[0] = (AL[0] & FFFF0000) | (T >> 16)
    //              case 2:     AL[0] = (AL[0] & FF000000) | (T >> 8)
    //              case 3:     AL[0] = (AL[0] & 00000000) | (T >> 0)
    //
    // Encoding: 101010 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SWL,
    //------------------------------------------------------------------------------------------------------------------
    // [STORE WORD RIGHT]
    //      Makeup a POTENTIALLY UNALIGNED memory address 'A' by adding the address in register 'S' plus the 16-bit
    //      SIGNED constant offset 'I'. Then forcefully word align 'A' by truncating 2 bits to form address 'AL'.
    //      Store a varying number of the LEAST SIGNIFICANT bytes in register 'T' to the MOST SIGNIFICANT bytes of
    //      address 'AL' based on the alignment of address 'A'. Note: all other bytes remain unchanged.
    //
    //      The 4 possible cases are outlined below:
    //          A = S + I
    //          AL = A & FFFFFFFC
    //
    //          switch (A % 4)
    //              case 0:     AL[0] = (AL[0] & 00000000) | (T << 0)
    //              case 1:     AL[0] = (AL[0] & 000000FF) | (T << 8)
    //              case 2:     AL[0] = (AL[0] & 0000FFFF) | (T << 16)
    //              case 3:     AL[0] = (AL[0] & 00FFFFFF) | (T << 24)
    //
    // Encoding: 101110 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    SWR,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF EQUAL]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED registers 'S' == SIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110100
    //------------------------------------------------------------------------------------------------------------------
    TEQ,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF EQUAL IMMEDIATE]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED register 'S' == SIGNED constant 'I'
    //
    // Encoding: 000001 SSSSS 01100 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TEQI,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF GREATER OR EQUAL]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED register 'S' >= SIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110000
    //------------------------------------------------------------------------------------------------------------------
    TGE,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF GREATER OR EQUAL IMMEDIATE]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED Register 'S' >= SIGNED constant 'I'
    //
    // Encoding: 000001 SSSSS 01000 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TGEI,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF GREATER OR EQUAL IMMEDIATE UNSIGNED]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          UNSIGNED Register 'S' >= UNSIGNED, SIGN EXTENDED constant 'I'
    //
    // Encoding: 000001 SSSSS 01001 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TGEIU,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF GREATER OR EQUAL UNSIGNED]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          UNSIGNED register 'S' >= UNSIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110001
    //------------------------------------------------------------------------------------------------------------------
    TGEU,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF LESS THAN]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED register 'S' < SIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110010
    //------------------------------------------------------------------------------------------------------------------
    TLT,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF LESS THAN IMMEDIATE]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED Register 'S' < SIGNED constant 'I'
    //
    // Encoding: 000001 SSSSS 01010 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TLTI,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF LESS THAN IMMEDIATE UNSIGNED]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          UNSIGNED Register 'S' < UNSIGNED, SIGN EXTENDED constant 'I'
    //
    // Encoding: 000001 SSSSS 01011 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TLTIU,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF LESS THAN UNSIGNED]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          UNSIGNED register 'S' < UNSIGNED register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110011
    //------------------------------------------------------------------------------------------------------------------
    TLTU,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF NOT EQUAL]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          Register 'S' != register 'T' and uses code 'I' as an execption param.
    //
    // Encoding: 000000 SSSSS TTTTT IIIII IIIII 110110
    //------------------------------------------------------------------------------------------------------------------
    TNE,
    //------------------------------------------------------------------------------------------------------------------
    // [TRAP IF NOT EQUAL IMMEDIATE]
    //      NOTE: *NOT* a valid MIPS I instruction on the R3000 CPU. However Sony's compilers appear to insert TRAP
    //      instructions in unreachable code regions, presumably to clearly mark them as such and cause an illegal
    //      instruction errors upon execution. I'm handling this instruction just so disassembly is more readable.
    //
    //      In MIPS II traps if:
    //          SIGNED Register 'S' != SIGNED constant 'I'
    //
    // Encoding: 000001 SSSSS 01110 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    TNEI,




    //------------------------------------------------------------------------------------------------------------------
    // [JUMP]
    //      Branch to the given program 32-bit WORD index 'I' (note: NOT byte!) within the current 256 MB memory region.
    //
    // Encoding: 000010 IIIII IIIII IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    J,
    //------------------------------------------------------------------------------------------------------------------
    // [JUMP AND LINK]
    //      Branch to the given program 32-bit WORD index 'I' (note: NOT byte!) within the current 256 MB memory region.
    //      The address of the 2nd (note: NOT 1st!) instruction following the branch is also saved in register 'RA' (R31).
    //
    // Encoding: 000011 IIIII IIIII IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    JAL,
    //------------------------------------------------------------------------------------------------------------------
    // [JUMP AND LINK REGISTER]
    //      Branch to the location specified in register 'S' and place the return address in register 'D'.
    //      Note: the return address is the address of the 2nd (note: NOT 1st!) instruction following the branch.
    //
    // Encoding: 000000 SSSSS ----- DDDDD ----- 001001
    //------------------------------------------------------------------------------------------------------------------
    JALR,
    //------------------------------------------------------------------------------------------------------------------
    // [JUMP REGISTER]
    //      Branch to the location specified in register 'S'.
    //
    // Encoding: 000000 SSSSS ----- ----- ----- 001000
    //------------------------------------------------------------------------------------------------------------------
    JR,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON EQUAL]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset 'I' if 'S' == 'T' where 'S', 'T' are registers.
    //
    // Encoding: 000100 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BEQ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON GREATER THAN OR EQUAL TO ZERO]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset 'I' if register 'S' is >= 0.
    //
    // Encoding: 000001 SSSSS 00001 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BGEZ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON GREATER THAN OR EQUAL TO ZERO AND LINK]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset if register 'S' is >= 0 and 'link'.
    //      The address of the 2nd (note: NOT 1st!) instruction following the branch is saved in register 'RA' (R31).
    //
    // Encoding: 000001 SSSSS 10001 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BGEZAL,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON GREATER THAN ZERO]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset 'I' if register 'S' > 0.
    //
    // Encoding: 000111 SSSSS ----- IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BGTZ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON LESS THAN OR EQUAL TO ZERO]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset if register 'S' is <= 0.
    //
    // Encoding: 000110 SSSSS ----- IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BLEZ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON LESS THAN ZERO]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset if register 'S' is < 0.
    //
    // Encoding: 000001 SSSSS 00000 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BLTZ,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON LESS THAN ZERO AND LINK]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset if register 'S' is < 0 and 'link'.
    //      The address of the 2nd (note: NOT 1st!) instruction following the branch is saved in register 'RA' (R31).
    //
    // Encoding: 000001 SSSSS 10000 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BLTZAL,
    //------------------------------------------------------------------------------------------------------------------
    // [BRANCH ON NOT EQUAL]
    //      Branch to the given SIGNED 16-bit WORD (not byte!) offset 'I' if 'S' != 'T' where 'S', 'T' are registers.
    //
    // Encoding: 000101 SSSSS TTTTT IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------
    BNE,
    */
    }

    // Figure out the printed length of the instruction
    const int64_t instructionEndStreamPos = out.tellp();
    assert(instructionEndStreamPos >= instructionStartStreamPos);
    const uint32_t instructionPrintedLen = (uint32_t)(instructionEndStreamPos - instructionStartStreamPos);

    // Comment on the instruction using the constant instruction evaluator.
    // This should have already been evaluated for the current function!
    InstructionCommenter::tryCommentInstruction(
        inst,
        instAddr,
        exe,
        gConstInstructionEvaluator,
        prefixInstructionComment,
        instructionPrintedLen,
        out
    );

    // Next line!
    out.put('\n');
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print the C++ code for the specified function
//------------------------------------------------------------------------------------------------------------------------------------------
static void printFunction(const ExeFile& exe, const ProgElem& progElem, std::ostream& out) {
    // Do constant evaluation for the function so we can comment some lines
    {
        ConstEvalRegState constEvalRegState;
        constEvalRegState.clear();
        constEvalRegState.setGpr(CpuGpr::GP, exe.assumedGpRegisterValue);

        gConstInstructionEvaluator.constEvalFunction(exe, progElem, constEvalRegState);
    }

    // Start of the function
    out << "void ";
    printProgElemName(progElem, "unnamed_func_", out);
    out << "() noexcept {\n";

    // Function body
    const uint32_t funcBegWordIdx = (progElem.startAddr - exe.baseAddress) / 4;
    const uint32_t funcEndWordIdx = (progElem.endAddr - exe.baseAddress) / 4;

    for (uint32_t wordIdx = funcBegWordIdx; wordIdx < funcEndWordIdx;) {
        // The address for this instruction and the next
        const uint32_t thisInstAddr = wordIdx * 4 + exe.baseAddress;
        const uint32_t nextInstAddr = thisInstAddr + 4;
        
        // Decode this instruction
        CpuInstruction thisInst;
        thisInst.CpuInstruction::decode(exe.words[wordIdx].value);

        // Decode the next instruction if there, otherwise make it a nop
        CpuInstruction nextInst;

        if (wordIdx + 1 < funcEndWordIdx) {
            nextInst.CpuInstruction::decode(exe.words[wordIdx + 1].value);
        } else {
            nextInst = CpuInstruction::makeNOP();
        }

        // Check for illegal instruction pairs
        verifyNoDependencyOnDelayedLoad(thisInst, nextInst);
        verifyNoAdjacentBranchesOrJumps(thisInst, nextInst);

        // See if we are dealing with a branch or jump or just an ordinary instruction.
        // For branches/jumps we need to reorder instructions to account for the branch delay slot:
        if (CpuOpcodeUtils::isBranchOrJumpOpcode(thisInst.opcode)) {
            // TODO
            wordIdx += 1;
        } else {
            // Simple case, print a single instruction and move along by 1 instruction
            printNonBranchOrJumpInstruction(exe, thisInst, thisInstAddr, 4, out);
            wordIdx += 1;
        }
    }

    // End of the function
    out << "}\n";
    out << "\n";
}

static void printFunctionPrototype(const ProgElem& progElem, std::ostream& out) {
    out << "void ";
    printProgElemName(progElem, "unnamed_func_", out);
    out << "() noexcept;\n";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verify the given function definition is SANE and raise a fatal error if not
//------------------------------------------------------------------------------------------------------------------------------------------
static void validateFuncElemRange(const ExeFile& exe, const ProgElem& progElem) noexcept {
    // Must be a function
    assert(progElem.type == ProgElemType::FUNCTION);
    
    // Is this function outside of the .EXE image range?
    // Also make sure it is properly aligned...
    const uint32_t exeStart = exe.baseAddress;
    const uint32_t exeEnd = exeStart + exe.sizeInWords * 4;

    const bool bIsInvalidFuncRange = (
        (progElem.startAddr % 4 != 0) ||
        (progElem.endAddr % 4 != 0) ||
        (progElem.startAddr < exeStart) ||
        (progElem.endAddr < exeStart) ||
        (progElem.startAddr >= exeEnd) ||
        (progElem.endAddr > exeEnd)
    );

    if (bIsInvalidFuncRange) {
        FATAL_ERROR("Invalid function element! The range defined is not valid for the .EXE or is misaligned!");
    }
}

void PseudoCppPrinter::printCpp(const ExeFile& exe, std::ostream& out) {
    // The app must define this header with all of the required macros
    out << "#include \"PsxVmMacros.h\"\n";
    out << "\n";

    // Validate and print the declarations for all the functions
    for (const ProgElem& progElem : exe.progElems) {        
        if (progElem.type == ProgElemType::FUNCTION) {
            validateFuncElemRange(exe, progElem);
            printFunctionPrototype(progElem, out);
        }
    }

    out << "\n";

    // Print all of the functions
    for (const ProgElem& progElem : exe.progElems) {        
        if (progElem.type == ProgElemType::FUNCTION) {
            printFunction(exe, progElem, out);
        }
    }
}
