#include "PseudoCppPrinter.h"

#include "ConstInstructionEvaluator.h"
#include "CpuInstruction.h"
#include "ExeFile.h"
#include "FatalErrors.h"
#include "InstructionCommenter.h"
#include "PrintUtils.h"
#include <algorithm>
#include <set>

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
    out.put('u');
}

static void printHexCppUint32Literal(const uint32_t valU32, bool bZeroPad, std::ostream& out) noexcept {    
    out << "0x";
    PrintUtils::printHexU32(valU32, bZeroPad, out);
    out.put('u');
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
    out << ")";
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
    constexpr uint32_t minCommentStartCol = 56u;
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
// Get the 'jr' instruction handler for a specified instruction and abort with failure if not found
//------------------------------------------------------------------------------------------------------------------------------------------
static const JRInstHandler& getJRInstHandler(const ExeFile& exe, const uint32_t instAddr) noexcept {
    for (const JRInstHandler& handler : exe.jrInstHandlers) {
        if (handler.instAddress == instAddr)
            return handler;
    }

    FATAL_ERROR_F("Failed to find a 'jr' instruction handler for the instruction at address 0x%Xu", instAddr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the given jump table program element or abort with failure if not found
//------------------------------------------------------------------------------------------------------------------------------------------
static const ProgElem& getJumpTableProgElem(const ExeFile& exe, const uint32_t atAddr) noexcept {
    const ProgElem* const pProgElem = exe.findProgElemAtAddr(atAddr);
    const bool bIsJumpTable = (
        pProgElem &&
        (pProgElem->type == ProgElemType::ARRAY) &&
        (pProgElem->arrayElemType == ProgElemType::PTR32)
    );

    if (bIsJumpTable) {
        return *pProgElem;
    } else {
        FATAL_ERROR_F("Missing a valid jump required for a 'jr' instruction. Jump table address is: 0x%Xu", atAddr);
    }
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
    }

    // Terminate instruction
    out.put(';');

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
// Print an instruction that a branch or jump
//------------------------------------------------------------------------------------------------------------------------------------------
static void printBranchOrJumpInstruction(
    const ExeFile& exe,
    const CpuInstruction branchInst,
    const uint32_t branchInstAddr,
    const CpuInstruction nextInst,
    const uint32_t nextInstAddr,
    const uint32_t indent,
    std::ostream& out
) {
    // Print the instruction goto label if it requires it, first
    if (doesInstructionRequireAGotoLabel(exe, branchInstAddr)) {
        out << "loc_";
        PrintUtils::printHexU32(branchInstAddr, true, out);
        out << ":\n";
    }

    // Figure out if this instruction is a branch
    const bool bIsBranch = CpuOpcodeUtils::isBranchOpcode(branchInst.opcode);

    // If the instruction is a branch, also figure out whether we can evaluate the branch condition and jump as the last step.
    // If we can do it, this reordering of instructions allows us to produce shorter and cleaner output.
    // This is only possible however if the next instruction does NOT modify any of the register that the branch condition depends on.
    bool bCanLateEvalBranchCond = false;

    if (bIsBranch) {
        const uint8_t nextInstDestGpr = nextInst.getDestGprIdx();
        bCanLateEvalBranchCond = (
            (nextInstDestGpr >= CpuGpr::NUM_GPRS) ||
            (!branchInst.isInputGprIdx(nextInstDestGpr)) ||
            (nextInst.isNOP())
        );
    }

    // Handle branches that require us to evaluate the branch condition first.
    // This requires creating a scope, and a boolean variable:
    uint32_t instIndent = indent;

    if (bIsBranch && (!bCanLateEvalBranchCond)) {
        // Create a new scope 
        indentByNumChars(indent, out);
        out << "{\n";

        // Evaluate the condition
        instIndent += 4;
        indentByNumChars(instIndent, out);
        out << "const bool bJump = ";

        switch (branchInst.opcode) {
            case CpuOpcode::BEQ:
            case CpuOpcode::BNE:
                printInst(out, branchInst, GprArg{ branchInst.regS }, GprArg{ branchInst.regT });
                break;

            case CpuOpcode::BGEZ:
            case CpuOpcode::BGTZ:
            case CpuOpcode::BLEZ:
            case CpuOpcode::BLTZ:
                printInst(out, branchInst, GprArg{ branchInst.regS });
                break;

            // Note: not supporting 'BGEZAL' or 'BLTZAL'!
            // These are not used in Doom anyway, and probably would not be emitted by a C compiler...
            case CpuOpcode::BGEZAL:
            case CpuOpcode::BLTZAL:
            default:
                FATAL_ERROR("Unhandled or unsupported branch opcode!");
                break;
        }

        out.put('\n');
    }

    // Print the instruction that follows the branch
    printNonBranchOrJumpInstruction(exe, nextInst, nextInstAddr, instIndent, out);

    // Handle the branch or jump itself
    indentByNumChars(instIndent, out);

    if (bIsBranch) {
        if (bCanLateEvalBranchCond) {
            // Branch instruction that can be evaluated late: print the if statement
            out << "if (";

            switch (branchInst.opcode) {
                case CpuOpcode::BEQ:
                case CpuOpcode::BNE:
                    printInst(out, branchInst, GprArg{ branchInst.regS }, GprArg{ branchInst.regT });
                    break;

                case CpuOpcode::BGEZ:
                case CpuOpcode::BGTZ:
                case CpuOpcode::BLEZ:
                case CpuOpcode::BLTZ:
                    printInst(out, branchInst, GprArg{ branchInst.regS });
                    break;

                // Note: not supporting 'BGEZAL' or 'BLTZAL'!
                // These are not used in Doom anyway, and probably would not be emitted by a C compiler...
                case CpuOpcode::BGEZAL:
                case CpuOpcode::BLTZAL:
                default:
                    FATAL_ERROR("Unhandled or unsupported branch opcode!");
                    break;
            }

            // And print the goto logic for the branch
            out << ") goto loc_";
            PrintUtils::printHexU32(branchInst.getBranchInstTargetAddr(branchInstAddr), true, out);
            out << ";\n";
        } else {
            // Branch instruction that can be evaluated late: branch to the specified location if the condition is true
            out << "if (bJump) goto loc_";
            PrintUtils::printHexU32(branchInst.getBranchInstTargetAddr(branchInstAddr), true, out);
            out << ";\n";

            // Close up the scope
            indentByNumChars(indent, out);
            out << "}\n";
        }
    } else {
        // Dealing with a jump instruction.
        // The way we deal with this will vary greatly depending on the instruction...
        if (branchInst.opcode == CpuOpcode::J) {
            // Jump just needs a simple 'goto'
            out << "goto loc_";
            PrintUtils::printHexU32(branchInst.getBranchInstTargetAddr(branchInstAddr), true, out);
            out << ";\n";
        } else if (branchInst.opcode == CpuOpcode::JAL) {
            // A fixed function call is also easy
            exe.printNameOfElemAtAddr(branchInst.getFixedJumpInstTargetAddr(branchInstAddr), out);
            out << "();\n";
        } else if (branchInst.opcode == CpuOpcode::JALR) {
            // JALR is tricker.
            // We need the host program to define the mappings to C++ functions:
            out << "vm_call(";
            getGprCppMacroName(branchInst.regS);
            out << ");\n";
        } else if (branchInst.opcode == CpuOpcode::JR) {
            // JR is the trickiest.
            // It's either a jump table (used for switch statements) or a bios call.
            // In a lot of cases though it might just be 'jr $ra', which is simply 'return;' in C++:
            if (branchInst.regS == CpuGpr::RA) {
                out << "return;\n";
            } else {
                const JRInstHandler& handler = getJRInstHandler(exe, branchInstAddr);

                if (handler.type == JRInstHandler::Type::BIOS_CALL) {
                    // The 'jr' instruction is a bios call
                    out << "bios_call(";
                    out << getGprCppMacroName(branchInst.regS);
                    out << ");\n";
                } else if (handler.type == JRInstHandler::Type::JUMP_TABLE) {
                    // The 'jr' instruction is a jump table or switch statement, begin creating that now
                    out << "switch (";
                    out << getGprCppMacroName(branchInst.regS);
                    out << ") {\n";
                    const ProgElem& jumpTable = getJumpTableProgElem(exe, handler.jumpTableAddr);
                    const uint32_t tableStartWordIdx = (jumpTable.startAddr - exe.baseAddress) / 4;
                    const uint32_t tableEndWordIdx = (jumpTable.endAddr - exe.baseAddress) / 4;

                    // Print each case.
                    // Note that jump tables might have duplicate entries, so only do each one once:
                    std::set<uint32_t> handledJumpTableEntries;

                    for (uint32_t wordIdx = tableStartWordIdx; wordIdx < tableEndWordIdx; ++wordIdx) {
                        assert(wordIdx < exe.sizeInWords);
                        const uint32_t jumpTableDest = exe.words[wordIdx].value;

                        // Only print if we didn't already handle this jump
                        if (handledJumpTableEntries.count(jumpTableDest) <= 0) {
                            indentByNumChars(instIndent + 4, out);
                            out << "case ";
                            printHexCppUint32Literal(jumpTableDest, true, out);
                            out << ": goto loc_";
                            PrintUtils::printHexU32(jumpTableDest, true, out);
                            out << ";\n";

                            // Don't print the same label again
                            handledJumpTableEntries.emplace(jumpTableDest);
                        }
                    }

                    // Print the default case (fatal error) and close
                    indentByNumChars(instIndent + 4, out);
                    out << "default: jump_table_err(); break;\n";

                    indentByNumChars(instIndent, out);
                    out << "}\n";
                } else {
                    FATAL_ERROR("Unknown 'jr' instruction handler type!");
                }
            }
        } else {
            FATAL_ERROR("Illegal jump instruction!");
        }
    }
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
            printBranchOrJumpInstruction(exe, thisInst, thisInstAddr, nextInst, nextInstAddr, 4, out);
            wordIdx += 2;
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
