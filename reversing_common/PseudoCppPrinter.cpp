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

static void printHexCppInt32Literal(const int32_t valI32, bool bZeroPad, std::ostream& out) noexcept {
    const int64_t valI64 = valI32;

    if (valI64 < 0) {
        out << "-0x";
        PrintUtils::printHexU32((uint32_t)(-valI64), bZeroPad, out);
    } else {
        out << "0x";
        PrintUtils::printHexU32((uint32_t) valI64, bZeroPad, out);
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
// Instruction argument types
//------------------------------------------------------------------------------------------------------------------------------------------
struct GprArg       { uint8_t val;  };
struct DecU8Arg     { uint8_t val;  };
struct HexI16Arg    { int16_t val;  };
struct HexU16Arg    { uint16_t val; };
struct HexI32Arg    { int32_t val;  };
struct HexU32Arg    { uint32_t val; };

//------------------------------------------------------------------------------------------------------------------------------------------
// Printing of instruction arguments
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void printSingleInstArg(std::ostream& out, const T arg);

template <>
void printSingleInstArg(std::ostream& out, const GprArg arg) {
    out << getGprCppMacroName(arg.val);
}

template <>
void printSingleInstArg(std::ostream& out, const DecU8Arg arg) {
    out << (uint32_t) arg.val;
}

template <>
void printSingleInstArg(std::ostream& out, const HexI16Arg arg) {
    printHexCppInt16Literal(arg.val, false, out);
}

template <>
void printSingleInstArg(std::ostream& out, const HexU16Arg arg) {
    printHexCppUint16Literal(arg.val, false, out);
}

template <>
void printSingleInstArg(std::ostream& out, const HexI32Arg arg) {
    printHexCppInt32Literal(arg.val, false, out);
}

template <>
void printSingleInstArg(std::ostream& out, const HexU32Arg arg) {
    printHexCppUint32Literal(arg.val, false, out);
}

template <class... Args>
static void printInstArgs(std::ostream& out, const Args... args);

static void printInstArgs([[maybe_unused]] std::ostream& out) {}

template <class T>
static void printInstArgs(std::ostream& out, const T arg) {
    printSingleInstArg(out, arg);
}

template <class T, class... Args>
static void printInstArgs(std::ostream& out, const T arg, const Args... args) {
    printSingleInstArg(out, arg);
    out << ", ";
    printInstArgs(out, args...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Print a generic instruction
//------------------------------------------------------------------------------------------------------------------------------------------
template <class... Args>
static void printInst(
    std::ostream& out,
    const CpuInstruction& ins,
    const Args... args    
) {
    // Print the destination register, if any
    const uint8_t destGpr = ins.getDestGprIdx();

    if (destGpr < CpuGpr::NUM_GPRS) {
        out << getGprCppMacroName(destGpr);
        out << " = ";
    }

    // Print opcode Mnemonic, braces and function arguments.
    // Note: replace some reserved C++ words also!
    const char* mnemonic = CpuOpcodeUtils::getMnemonic(ins.opcode);

    if (std::strcmp(mnemonic, "break") == 0) {
        mnemonic = "_break";
    }

    out << mnemonic;
    out.put('(');
    printInstArgs(out, args...);
    out << ");";
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
        // OPERATION(regS, regT, (uint16_t) immVal)
        case CpuOpcode::TEQ:
        case CpuOpcode::TGE:
        case CpuOpcode::TGEU:
        case CpuOpcode::TLT:
        case CpuOpcode::TLTU:
        case CpuOpcode::TNE:
            printInst(out, inst, GprArg{ inst.regS }, GprArg{ inst.regT }, HexU16Arg{ (uint16_t) inst.immediateVal });
            break;

        // OPERATION(regS, regT)
        case CpuOpcode::ADD:
        case CpuOpcode::ADDU:
        case CpuOpcode::AND:
        case CpuOpcode::DIV:
        case CpuOpcode::DIVU:
        case CpuOpcode::MULT:
        case CpuOpcode::MULTU:
        case CpuOpcode::NOR:
        case CpuOpcode::OR:
        case CpuOpcode::SLT:
        case CpuOpcode::SLTU:
        case CpuOpcode::SUB:
        case CpuOpcode::SUBU:
        case CpuOpcode::XOR:
            printInst(out, inst, GprArg{ inst.regS }, GprArg{ inst.regT });
            break;

        // OPERATION(regT, regS)
        case CpuOpcode::SLLV:
        case CpuOpcode::SRAV:
        case CpuOpcode::SRLV:
            printInst(out, inst, GprArg{ inst.regT }, GprArg{ inst.regS });
            break;

        // OPERATION(regT, regS, (int16_t) immVal)
        case CpuOpcode::LWL:
        case CpuOpcode::LWR:
        case CpuOpcode::SB:
        case CpuOpcode::SH:
        case CpuOpcode::SW:
        case CpuOpcode::SWL:
        case CpuOpcode::SWR:
            printInst(out, inst, GprArg{ inst.regT }, GprArg{ inst.regS }, HexI16Arg{ (int16_t) inst.immediateVal });
            break;

        // OPERATION(regS, (int32_t) immVal)
        case CpuOpcode::TEQI:
        case CpuOpcode::TGEI:
        case CpuOpcode::TLTI:
        case CpuOpcode::TNEI:
            printInst(out, inst, GprArg{ inst.regS }, HexI32Arg{ (int32_t) inst.immediateVal });
            break;

        // OPERATION(regS, (uint32_t) immVal)
        case CpuOpcode::TGEIU:
        case CpuOpcode::TLTIU:
            printInst(out, inst, GprArg{ inst.regS }, HexU32Arg{ inst.immediateVal });
            break;

        // OPERATION(regS, (int16_t) immVal)
        case CpuOpcode::ADDI:
        case CpuOpcode::ADDIU:
        case CpuOpcode::LB:
        case CpuOpcode::LBU:
        case CpuOpcode::LH:
        case CpuOpcode::LHU:
        case CpuOpcode::LW:
        case CpuOpcode::SLTI:
        case CpuOpcode::SLTIU:
            printInst(out, inst, GprArg{ inst.regS }, HexI16Arg{ (int16_t) inst.immediateVal });
            break;

        // OPERATION(regS, (uint16_t) immVal)
        case CpuOpcode::ANDI:
        case CpuOpcode::ORI:
        case CpuOpcode::XORI:
            printInst(out, inst, GprArg{ inst.regS }, HexU16Arg{ (uint16_t) inst.immediateVal });
            break;

        // OPERATION(regT, (uint16_t) immVal)
        case CpuOpcode::SLL:
        case CpuOpcode::SRA:
        case CpuOpcode::SRL:
            printInst(out, inst, GprArg{ inst.regT }, HexU16Arg{ (uint16_t) inst.immediateVal });
            break;

        // OPERATION(regS)
        case CpuOpcode::MTHI:
        case CpuOpcode::MTLO:
            printInst(out, inst, GprArg{ inst.regS });
            break;

        // OPERATION((uint16_t) immVal)
        case CpuOpcode::LUI:
            printInst(out, inst, HexU16Arg{ (uint16_t) inst.immediateVal });
            break;

        // OPERATION((uint32_t) immVal)
        case CpuOpcode::BREAK:
        case CpuOpcode::COP2:
        case CpuOpcode::SYSCALL:
            printInst(out, inst, HexU32Arg{ inst.immediateVal });
            break;

        // OPERATION()
        case CpuOpcode::RFE:
        case CpuOpcode::TLBP:
        case CpuOpcode::TLBR:
        case CpuOpcode::TLBWI:
        case CpuOpcode::TLBWR:
        case CpuOpcode::MFHI:
        case CpuOpcode::MFLO:
            printInst(out, inst);
            break;

        // Special cases
        case CpuOpcode::CFC2:
        case CpuOpcode::MFC0:
        case CpuOpcode::MFC2:
            printInst(out, inst, DecU8Arg{ inst.regD });
            break;

        case CpuOpcode::CTC2:
        case CpuOpcode::MTC0:
        case CpuOpcode::MTC2:
            printInst(out, inst, GprArg{ inst.regT }, DecU8Arg{ inst.regD });
            break;

        case CpuOpcode::LWC2:
        case CpuOpcode::SWC2:
            printInst(out, inst, DecU8Arg{ inst.regT }, GprArg{ inst.regS }, HexI16Arg{ (int16_t) inst.immediateVal });
            break;

    /*
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
