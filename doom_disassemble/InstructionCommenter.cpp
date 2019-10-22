#include "InstructionCommenter.h"

#include "ConstInstructionEvaluator.h"
#include "CpuInstruction.h"
#include "ExeFile.h"
#include "PrintUtils.h"

static void prefixComment(
    const InstructionCommenter::CommentPrefixerFunc pCommentPrefixer,
    const uint32_t lineCol,
    std::ostream& out
) noexcept {
    if (pCommentPrefixer) {
        pCommentPrefixer(lineCol, out);
    }
}

static void printNameAndAddress(const uint32_t addr, const ExeFile& exe, std::ostream& out) noexcept {
    const bool bPrintedName = exe.printNameOfElemAtAddr(addr, out);

    // If we printed a name then print the address in brackets also
    if (bPrintedName) {
        out << " (";
        PrintUtils::printHexU32(addr, true, out);
        out.put(')');
    }
}

void InstructionCommenter::tryCommentInstruction(
    CpuInstruction& inst,
    const uint32_t instAddr,
    const ExeFile& exe,
    const ConstInstructionEvaluator& constInstEvaluator,
    const CommentPrefixerFunc pCommentPrefixer,
    const uint32_t lineCol,
    std::ostream& out
) noexcept {
    // If the instruction is a NOP then there is nothing to do
    if (inst.isNOP())
        return;
    
    // Get the register states for this instruction
    ConstEvalRegState inRegState;
    ConstEvalRegState outRegState;
    constInstEvaluator.getInRegStateForInstruction(instAddr, inRegState);
    constInstEvaluator.getInRegStateForInstruction(instAddr, outRegState);

    // Try to comment
    switch (inst.opcode) {
        /*
        //------------------------------------------------------------------------------------------------------------------
        // [ADD WORD]
        //      Add the SIGNED registers 'S' and 'T' and save in the output register 'D', I.E:
        //          D = S + T
        //      If an OVERFLOW occurs then an EXCEPTION will be raised and the destination register will be UNCHANGED.
        // 
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100000
        //------------------------------------------------------------------------------------------------------------------
        ADD,
        //------------------------------------------------------------------------------------------------------------------
        // [ADD IMMEDIATE WORD]
        //      Add the 16-bit SIGNED constant 'I' to 'S' and save in the output register 'T', I.E:
        //          T = S + I
        //      If an OVERFLOW occurs then an EXCEPTION will be raised and the destination register will be UNCHANGED.
        //
        // Encoding: 001000 SSSSS TTTTT IIIII IIIII IIIIII
        //------------------------------------------------------------------------------------------------------------------
        ADDI,
        //------------------------------------------------------------------------------------------------------------------
        // [ADD IMMEDIATE WORD UNSIGNED]
        //      Add the 16-bit SIGNED constant 'I' to 'S' and save in the output register 'T', I.E:
        //          T = S + I
        //      OVERFLOWS are IGNORED by this particular operation.
        //
        // Encoding: 001001 SSSSS TTTTT IIIII IIIII IIIIII
        //------------------------------------------------------------------------------------------------------------------
        ADDIU,
        //------------------------------------------------------------------------------------------------------------------
        // [ADD UNSIGNED WORD]
        //      Add the registers 'S' and 'T' and save in the output register 'D', I.E:
        //          D = S + T
        //      OVERFLOWS are IGNORED by this particular operation.
        // 
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100001
        //------------------------------------------------------------------------------------------------------------------
        ADDU,
        //------------------------------------------------------------------------------------------------------------------
        // [AND]
        //      Do a bitwise logical AND of registers 'S' and 'T' and store in register 'D', I.E:
        //          D = S & T
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100100
        //------------------------------------------------------------------------------------------------------------------    
        AND,
        //------------------------------------------------------------------------------------------------------------------
        // [AND IMMEDIATE]
        //      Do a bitwise logical AND of register 'S' with 16-bit constant 'I' and store in register 'T', I.E:
        //          T = S & I
        //
        // Encoding: 001100 SSSSS TTTTT IIIII IIIII IIIIII
        //------------------------------------------------------------------------------------------------------------------
        ANDI,
        //------------------------------------------------------------------------------------------------------------------
        // [DIVIDE WORD]
        //      Divide SIGNED register 'S' by SIGNED register 'T' and save the result in special register 'LO'.
        //      The remainder is stored in special register 'HI'.
        //      If dividing by zero then the result is UNPREDICTABLE but no error occurs.
        //
        // Encoding: 000000 SSSSS TTTTT ----- ----- 011010
        //------------------------------------------------------------------------------------------------------------------
        DIV,
        //------------------------------------------------------------------------------------------------------------------
        // [DIVIDE UNSIGNED WORD]
        //      Divide UNSIGNED register 'S' by UNSIGNED register 'T' and save the result in special register 'LO'.
        //      The remainder is stored in special register 'HI'.
        //      If dividing by zero then the result is UNPREDICTABLE but no error occurs.
        //
        // Encoding: 000000 SSSSS TTTTT ----- ----- 011011
        //------------------------------------------------------------------------------------------------------------------
        DIVU,
        //------------------------------------------------------------------------------------------------------------------
        // [LOAD UPPER IMMEDIATE]
        //      Load the constant 'I' into the MOST SIGNIFICANT bits of register 'T' and zero all other bits.
        //          T = I << 16
        //
        // Encoding: 001111 ----- TTTTT IIIII IIIII IIIIII
        //------------------------------------------------------------------------------------------------------------------
        LUI,
        */

        case CpuOpcode::LB:
        case CpuOpcode::LBU:
        case CpuOpcode::LH:
        case CpuOpcode::LHU:
        case CpuOpcode::LW: 
        case CpuOpcode::LWC2:
        case CpuOpcode::LWL:
        case CpuOpcode::LWR: {
            if (outRegState.bGprValueKnown[inst.regS]) {
                prefixComment(pCommentPrefixer, lineCol, out);
                const uint32_t s = inRegState.gprValue[inst.regS];
                const int32_t i = (int32_t)(int16_t)(uint16_t) inst.immediateVal;
                const uint32_t addr = s + i;
                out << "Load from: ";
                printNameAndAddress(addr, exe, out);
            }
        } break;

        /*
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
        // [MULTIPLY WORD]
        //      Multiply SIGNED register 'S' by SIGNED register 'T' and save the results in special registers 'HI' & 'LO'.
        //      The low 32-bits of the result is stored in 'LO' and the high 32-bits stored in 'HI':
        //          HI, LO = S * T
        //
        // Encoding: 000000 SSSSS TTTTT ----- ----- 011000
        //------------------------------------------------------------------------------------------------------------------
        MULT,
        //------------------------------------------------------------------------------------------------------------------
        // [MULTIPLY UNSIGNED WORD]
        //      Multiply UNSIGNED register 'S' by UNSIGNED register 'T' and save the results in special registers 'HI' & 'LO'.
        //      The low 32-bits of the result is stored in 'LO' and the high 32-bits stored in 'HI':
        //          HI, LO = S * T
        //
        // Encoding: 000000 SSSSS TTTTT ----- ----- 011001
        //------------------------------------------------------------------------------------------------------------------
        MULTU,
        //------------------------------------------------------------------------------------------------------------------
        // [NOT OR]
        //      Does a bitwise logical OR of register 'S' with register 'T' and then inverts the bits of the result.
        //      The output is stored in register 'D'. This is equivalent to:
        //          D = ~(S | T)
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100111
        //------------------------------------------------------------------------------------------------------------------
        NOR,
        //------------------------------------------------------------------------------------------------------------------
        // [OR]
        //      Do a bitwise logical OR of registers 'S' and 'T' and store in register 'D', I.E:
        //          D = S | T
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100101
        //------------------------------------------------------------------------------------------------------------------
        OR,
        //------------------------------------------------------------------------------------------------------------------
        // [OR IMMEDIATE]
        //      Do a bitwise logical OR of register 'S' with 16-bit constant 'I' and store in register 'T', I.E:
        //          T = S | I
        //
        // Encoding: 001101 SSSSS TTTTT IIIII IIIII IIIIII
        //------------------------------------------------------------------------------------------------------------------
        ORI,
        */

        case CpuOpcode::SB:
        case CpuOpcode::SH:
        case CpuOpcode::SW:
        case CpuOpcode::SWC2:
        case CpuOpcode::SWL:
        case CpuOpcode::SWR: {
            if (outRegState.bGprValueKnown[inst.regS]) {
                prefixComment(pCommentPrefixer, lineCol, out);
                const uint32_t s = inRegState.gprValue[inst.regS];
                const int32_t i = (int32_t)(int16_t)(uint16_t) inst.immediateVal;
                const uint32_t addr = s + i;
                out << "Store to: ";
                printNameAndAddress(addr, exe, out);
            }
        } break;

        /*
        //------------------------------------------------------------------------------------------------------------------
        // [SHIFT WORD LEFT LOGICAL]
        //      Do a logical left shift of register 'T' by 5 bit unsigned constant 'I' and store the result in 'D', I.E:
        //          D = T << I
        //
        // Encoding: 000000 ----- TTTTT DDDDD IIIII 000000
        //------------------------------------------------------------------------------------------------------------------
        SLL,
        //------------------------------------------------------------------------------------------------------------------
        // [SHIFT WORD LEFT LOGICAL VARIABLE]
        //      Do a logical left shift of register 'T' by UNSIGNED register 'S' and store in register 'D', I.E:
        //          D = T << S
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 000100
        //------------------------------------------------------------------------------------------------------------------
        SLLV,
        //------------------------------------------------------------------------------------------------------------------
        // [SET ON LESS THAN]
        //      Compare SIGNED registers 'S' and 'T', checking the condition 'S < T'.
        //      If the condition is 'true' then store '1' in register 'D', otherwise '0'. This is equivalent to:
        //          D = (S < T) ? 1 : 0
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 101010
        //------------------------------------------------------------------------------------------------------------------
        SLT,
        //------------------------------------------------------------------------------------------------------------------
        // [SET ON LESS THAN IMMEDIATE]
        //      Compare SIGNED register 'S' with SIGNED 16-bit constant 'I', checking the condition 'S < I'.
        //      If the condition is 'true' then store '1' in register 'T', otherwise '0'. This is equivalent to:
        //          T = (S < I) ? 1 : 0
        //
        // Encoding: 001010 SSSSS TTTTT IIIII IIIII IIIIII
        //------------------------------------------------------------------------------------------------------------------
        SLTI,
        //------------------------------------------------------------------------------------------------------------------
        // [SET ON LESS THAN IMMEDIATE UNSIGNED]
        //      Compare UNSIGNED register 'S' with SIGNED 16-bit constant 'I', checking the condition 'S < I'.
        //      Before comparison, the constant 'I' is SIGN EXTENDED to 32-bits and then interpreted as an unsigned.
        //      If the condition is 'true' then store '1' in register 'T', otherwise '0'. This is equivalent to:
        //          T = (S < sign_extend(I)) ? 1 : 0
        //
        // Encoding: 001011 SSSSS TTTTT IIIII IIIII IIIIII
        //------------------------------------------------------------------------------------------------------------------
        SLTIU,
        //------------------------------------------------------------------------------------------------------------------
        // [SET ON LESS THAN UNSIGNED]
        //      Compare UNSIGNED registers 'S' and 'T', checking the condition 'S < T'.
        //      If the condition is 'true' then store '1' in register 'D', otherwise '0'. This is equivalent to:
        //          D = (S < T) ? 1 : 0
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 101011
        //------------------------------------------------------------------------------------------------------------------
        SLTU,
        //------------------------------------------------------------------------------------------------------------------
        // [SHIFT WORD RIGHT ARITHMETIC]
        //      Do an ARITHMETIC right shift of register 'T' by 5 bit unsigned constant 'I' and store the result in 'D', I.E:
        //          D = T >> I
        //
        // Encoding: 000000 ----- TTTTT DDDDD IIIII 000011
        //------------------------------------------------------------------------------------------------------------------
        SRA,
        //------------------------------------------------------------------------------------------------------------------
        // [SHIFT WORD RIGHT ARITHMETIC VARIABLE]
        //      Do an ARITHMETIC right shift of register 'T' by UNSIGNED register 'S' and store in register 'D', I.E:
        //          D = T >> S
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 000111
        //------------------------------------------------------------------------------------------------------------------
        SRAV,
        //------------------------------------------------------------------------------------------------------------------
        // [SHIFT WORD RIGHT LOGICAL]
        //      Do a LOGICAL right shift of register 'T' by 5 bit unsigned constant 'I' and store the result in 'D', I.E:
        //          D = T >>> I
        //
        // Encoding: 000000 ----- TTTTT DDDDD IIIII 000010
        //------------------------------------------------------------------------------------------------------------------
        SRL,
        //------------------------------------------------------------------------------------------------------------------
        // [SHIFT WORD RIGHT LOGICAL VARIABLE]
        //      Do a LOGICAL right shift of register 'T' by UNSIGNED register 'S' and store in register 'D', I.E:
        //          D = T >>> S
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 000110
        //------------------------------------------------------------------------------------------------------------------
        SRLV,
        //------------------------------------------------------------------------------------------------------------------
        // [SUBTRACT WORD]
        //      Do a SIGNED subtraction of register 'T' from register 'S' and store in register 'D', I.E:
        //          D = S - T
        //      If an OVERFLOW occurs then an EXCEPTION will be raised and the destination register will be UNCHANGED.
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100010
        //------------------------------------------------------------------------------------------------------------------
        SUB,
        //------------------------------------------------------------------------------------------------------------------
        // [SUBTRACT UNSIGNED WORD]
        //      Do a subtraction of register 'T' from register 'S' and store in register 'D', I.E:
        //          D = S - T
        //      OVERFLOWS are IGNORED by this particular operation.
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100011
        //------------------------------------------------------------------------------------------------------------------
        SUBU,

        //------------------------------------------------------------------------------------------------------------------
        // [EXCLUSIVE OR]
        //      Do a bitwise logical exclusive OR of registers 'S' and 'T' and store in register 'D', I.E:
        //          D = S ^ T
        //
        // Encoding: 000000 SSSSS TTTTT DDDDD ----- 100110
        //------------------------------------------------------------------------------------------------------------------
        XOR,
        //------------------------------------------------------------------------------------------------------------------
        // [EXCLUSIVE OR IMMEDIATE]
        //      Do a bitwise logical exclusive OR of register 'S' with 16-bit constant 'I' and store in register 'T', I.E:
        //          T = S ^ I
        //
        // Encoding: 001110 SSSSS TTTTT IIIII IIIII IIIIII
        //------------------------------------------------------------------------------------------------------------------
        XORI,        
        */

        // Instruction types we don't comment deliberately:
        case CpuOpcode::BEQ:
        case CpuOpcode::BGEZ:
        case CpuOpcode::BGEZAL:
        case CpuOpcode::BGTZ:
        case CpuOpcode::BLEZ:
        case CpuOpcode::BLTZ:
        case CpuOpcode::BLTZAL:
        case CpuOpcode::BNE:
        case CpuOpcode::BREAK:
        case CpuOpcode::CFC2:
        case CpuOpcode::COP2:
        case CpuOpcode::CTC2:
        case CpuOpcode::J:
        case CpuOpcode::JAL:
        case CpuOpcode::JALR:
        case CpuOpcode::JR:
        case CpuOpcode::MFC0:
        case CpuOpcode::MFC2:
        case CpuOpcode::MTC0:
        case CpuOpcode::MTC2:
        case CpuOpcode::RFE:
        case CpuOpcode::SYSCALL:
        case CpuOpcode::TEQ:
        case CpuOpcode::TEQI:
        case CpuOpcode::TGE:
        case CpuOpcode::TGEI:
        case CpuOpcode::TGEIU:
        case CpuOpcode::TGEU:
        case CpuOpcode::TLBP:
        case CpuOpcode::TLBR:
        case CpuOpcode::TLBWI:
        case CpuOpcode::TLBWR:
        case CpuOpcode::TLT:
        case CpuOpcode::TLTI:
        case CpuOpcode::TLTIU:
        case CpuOpcode::TLTU:
        case CpuOpcode::TNE:
        case CpuOpcode::TNEI:
        case CpuOpcode::INVALID:
            break;
    }
}
