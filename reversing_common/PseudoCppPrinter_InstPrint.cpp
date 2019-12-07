#include "PseudoCppPrinter_InstPrint.h"

#include "CpuGpr.h"
#include "CpuInstruction.h"
#include "PseudoCppPrinter.h"
#include <cstdlib>

using namespace PseudoCppPrinter;

//------------------------------------------------------------------------------------------------------------------------------------------
// Print in either hex or decimal, depending on how big the number is.
// The number is printed without padding.
//------------------------------------------------------------------------------------------------------------------------------------------
static void printHexOrDecInt32Literal(const int32_t val, std::ostream& out) {
    if (std::abs(val) < 10) {
        out << val;
    } else {
        printHexCppInt32Literal(val, false, out);
    }
}

static void printHexOrDecUint32Literal(const uint32_t val, std::ostream& out) {
    if (val < 10) {
        out << val;
    } else {
        printHexCppUint32Literal(val, false, out);
    }
}

void PseudoCppPrinter::printInst_addiu(std::ostream& out, const CpuInstruction& inst) {
    const int16_t i16 = (int16_t) inst.immediateVal;
    const int32_t i32 = i16;

    out << getGprCppMacroName(inst.regT);

    if (i32 == 0) {
        // If the immediate is zero, then it is basically a move or assign
        out << " = ";

        if (inst.regS == CpuGpr::ZERO) {
            out << "0";
        } else {
            out << getGprCppMacroName(inst.regS);
        }
    }
    else if (inst.regS == CpuGpr::ZERO) {
        // If the register is zero then we are just assigning an integer literal
        out << " = ";
        printHexOrDecInt32Literal(i32, out);
    }
    else if (inst.regT == inst.regS) {
        // If the source and dest reg are the same then we can use one of '+=', '-=', '++' or '--':
        if (i32 < 0) {
            if (i32 == -1) {
                out << "--";
            } else {
                out << " -= ";
                printHexOrDecInt32Literal(-i32, out);
            }
        } else {
            if (i32 == 1) {
                out << "++";
            } else {
                out << " += ";
                printHexOrDecInt32Literal(i32, out);
            }
        }
    } else {
        // Regular add or subtract
        if (i32 < 0) {
            out << " = ";
            out << getGprCppMacroName(inst.regS);
            out << " - ";
            printHexOrDecInt32Literal(-i32, out);
        } else {
            out << " = ";
            out << getGprCppMacroName(inst.regS);
            out << " + ";
            printHexOrDecInt32Literal(i32, out);
        }
    }
}

void PseudoCppPrinter::printInst_addu(std::ostream& out, const CpuInstruction& inst) {
    out << getGprCppMacroName(inst.regD);

    if (inst.regS == CpuGpr::ZERO && inst.regT == CpuGpr::ZERO) {
        // Zero assign
        out << " = 0";
    }
    else if (inst.regS == CpuGpr::ZERO) {
        // Move instruction
        out << " = ";
        out << getGprCppMacroName(inst.regT);
    }
    else if (inst.regT == CpuGpr::ZERO) {
        // Move instruction
        out << " = ";
        out << getGprCppMacroName(inst.regS);
    }
    else if (inst.regS == inst.regD) {
        // One source reg same as dest: can use '+=' shorthand
        out << " += ";
        out << getGprCppMacroName(inst.regT);
    }
    else if (inst.regT == inst.regD) {
        // One source reg same as dest: can use '+=' shorthand
        out << " += ";
        out << getGprCppMacroName(inst.regS);
    }
    else {
        // Regular add
        out << " = ";
        out << getGprCppMacroName(inst.regS);
        out << " + ";
        out << getGprCppMacroName(inst.regT);
    }
}

void PseudoCppPrinter::printInst_lui(std::ostream& out, const CpuInstruction& inst) {
    out << getGprCppMacroName(inst.regT);
    out << " = ";
    printHexOrDecUint32Literal(inst.immediateVal << 16, out);
}

void PseudoCppPrinter::printInst_or(std::ostream& out, const CpuInstruction& inst) {
    out << getGprCppMacroName(inst.regD);

    if (inst.regS == CpuGpr::ZERO && inst.regT == CpuGpr::ZERO) {
        // Zero assign
        out << " = 0";
    }
    else if (inst.regS == CpuGpr::ZERO) {
        // Move instruction
        out << " = ";
        out << getGprCppMacroName(inst.regT);
    }
    else if (inst.regT == CpuGpr::ZERO) {
        // Move instruction
        out << " = ";
        out << getGprCppMacroName(inst.regS);
    }
    else if (inst.regS == inst.regD) {
        // One source reg same as dest: can use '|=' shorthand
        out << " |= ";
        out << getGprCppMacroName(inst.regT);
    }
    else if (inst.regT == inst.regD) {
        // One source reg same as dest: can use '|=' shorthand
        out << " |= ";
        out << getGprCppMacroName(inst.regS);
    }
    else {
        // Regular bitwise OR
        out << " = ";
        out << getGprCppMacroName(inst.regS);
        out << " | ";
        out << getGprCppMacroName(inst.regT);
    }
}

void PseudoCppPrinter::printInst_ori(std::ostream& out, const CpuInstruction& inst) {
    out << getGprCppMacroName(inst.regT);
    const uint16_t u16 = (uint16_t) inst.immediateVal;

    if (inst.regS == CpuGpr::ZERO) {
        // Assigning a constant
        out << " = ";
        printHexOrDecUint32Literal(u16, out);
    } 
    else if (u16 == 0) {
        // Move instruction
        out << " = ";
        out << getGprCppMacroName(inst.regS);
    }
    else if (inst.regS == inst.regT) {
        // Can use '|=' shorthand
        out << " |= ";
        printHexOrDecUint32Literal(u16, out);
    }
    else {
        // Regular bitwise OR
        out << " = ";
        out << getGprCppMacroName(inst.regS);
        out << " | ";
        printHexOrDecUint32Literal(u16, out);
    }
}

void PseudoCppPrinter::printInst_sra(std::ostream& out, const CpuInstruction& inst) {
    out << getGprCppMacroName(inst.regD);
    const uint32_t shiftAmount = inst.immediateVal & 0x1Fu;

    if (inst.regT == CpuGpr::ZERO) {
        // Zero assign
        out << " = 0";
    }
    else if (shiftAmount == 0) {
        // Move instruction
        out << " = ";
        out << getGprCppMacroName(inst.regT);
    }
    else {
        // Shift instruction
        out << " = u32(i32(";
        out << getGprCppMacroName(inst.regT);
        out << ") >> ";
        out << shiftAmount;
        out << ")";
    }
}

void PseudoCppPrinter::printInst_subu(std::ostream& out, const CpuInstruction& inst) {
    out << getGprCppMacroName(inst.regD);

    if (inst.regS == CpuGpr::ZERO && inst.regT == CpuGpr::ZERO) {
        // Zero assign
        out << " = 0";
    }
    else if (inst.regS == CpuGpr::ZERO) {
        // Assigning negative number
        out << " = -";
        out << getGprCppMacroName(inst.regT);
    }
    else if (inst.regT == CpuGpr::ZERO) {
        // Move instruction
        out << " = ";
        out << getGprCppMacroName(inst.regS);
    }
    else if (inst.regS == inst.regD) {
        // Regular subtract with '-=' shorthand
        out << " -= ";
        out << getGprCppMacroName(inst.regT);
    }
    else {
        // Regular subtract
        out << " = ";
        out << getGprCppMacroName(inst.regS);
        out << " - ";
        out << getGprCppMacroName(inst.regT);
    }
}
