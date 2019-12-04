#include "PseudoCppPrinter.h"

#include "ConstInstructionEvaluator.h"
#include "CpuInstruction.h"
#include "ExeFile.h"
#include "FatalErrors.h"
#include "InstructionCommenter.h"
#include "PrintUtils.h"
#include <algorithm>

static ConstInstructionEvaluator gConstInstructionEvaluator;

static void prefixInstructionComment(const uint32_t lineCol, std::ostream& out) {
    // Figure out the start column for the comment
    constexpr uint32_t minCommentStartCol = 60u;
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
    out << "; ";
}

static void printAddressForLine(const uint32_t addr, std::ostream& out) {
    PrintUtils::printHexU32(addr, true, out);
    out << ":";
}

static void printProgElemName(const ProgElem& progElem, const char* const defaultNamePrefix, std::ostream& out) {
    if (progElem.name != nullptr && progElem.name[0] != 0) {
        out << progElem.name;
    } else {
        out << defaultNamePrefix;
        PrintUtils::printHexU32(progElem.startAddr, true, out);
    }
}

static void printProgInstruction(const ExeFile& exe, const ProgElem* const pParentFunc, const uint32_t instAddr, const uint32_t instWord, std::ostream& out) {
    // Log where we are in the stream (so we can tell how long the instruction was when printed)
    const int64_t instructionStartStreamPos = out.tellp();

    // Decode the instruction
    CpuInstruction inst;
    inst.decode(instWord);

    // Print 'nop' if the instruction is a nop, otherwise print the instruction
    if (inst.isNOP()) {
        out << "  nop";
    } else {
        // If the instruction is NOT a branch or jump then indent it slightly.
        // This makes branches and jumps stand out more.
        // Control flow instructions are important for detecting function boundaries etc. !
        if (!CpuOpcodeUtils::isBranchOrJumpOpcode(inst.opcode)) {
            out << "  ";
        }

        inst.print(exe, instAddr, pParentFunc, out);
    }

    // Figure out the printed length of the instruction
    const int64_t instructionEndStreamPos = out.tellp();
    assert(instructionEndStreamPos >= instructionStartStreamPos);
    const uint32_t instructionPrintedLen = (uint32_t)(instructionEndStreamPos - instructionStartStreamPos);

    // Comment on the instruction if there is a parent function
    if (pParentFunc) {
        InstructionCommenter::tryCommentInstruction(
            inst,
            instAddr,
            exe,
            gConstInstructionEvaluator,
            prefixInstructionComment,
            instructionPrintedLen,
            out
        );
    }
}

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


    // End of the function
    out << "}\n";
    out << "\n";
}

static void printFunctionPrototype(const ProgElem& progElem, std::ostream& out) {
    out << "void ";
    printProgElemName(progElem, "unnamed_func_", out);
    out << "() noexcept;\n";
}

//----------------------------------------------------------------------------------------------------------------------
// Verify the given function definition is SANE and raise a fatal error if not
//----------------------------------------------------------------------------------------------------------------------
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
    out << "#include \"PsxVm.h\"\n";
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
