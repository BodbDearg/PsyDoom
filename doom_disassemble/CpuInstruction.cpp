#include "CpuInstruction.h"

#include "CpuGpr.h"
#include "PrintUtils.h"

//----------------------------------------------------------------------------------------------------------------------
// Print an instruction that assigns to a GPR and that has two input GPR arguments
//----------------------------------------------------------------------------------------------------------------------
static void printInstGprOutGprInGprIn(
    const CpuInstruction& ins,    
    const uint8_t in1Gpr,
    const uint8_t in2Gpr,
    std::stringstream& out
) noexcept {
    const uint8_t destGpr = ins.getDestGprIdx();
    out << CpuGpr::getName(destGpr);
    out << " = ";
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out << ' ';
    out << CpuGpr::getName(in1Gpr);
    out << ", ";
    out << CpuGpr::getName(in2Gpr);
}

//----------------------------------------------------------------------------------------------------------------------
// Print an instruction that assigns to a GPR and that has an input GPR arg followed by an I16 constant
//----------------------------------------------------------------------------------------------------------------------
static void printInstGprOutGprInI16In(    
    const CpuInstruction& ins,
    const uint8_t in1Gpr,
    const int16_t in2I16,
    std::stringstream& out
) noexcept {
    const uint8_t destGpr = ins.getDestGprIdx();
    out << CpuGpr::getName(destGpr);
    out << " = ";
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out.put(' ');
    out << CpuGpr::getName(in1Gpr);
    out << ", ";
    PrintUtils::printHexI16(in2I16, false, out);
}

//----------------------------------------------------------------------------------------------------------------------
// Print an instruction that assigns to a GPR and that has an input GPR arg followed by an U16 constant
//----------------------------------------------------------------------------------------------------------------------
static void printInstGprOutGprInU16In(    
    const CpuInstruction& ins,
    const uint8_t in1Gpr,
    const uint16_t in2U16,
    std::stringstream& out
) noexcept {
    const uint8_t destGpr = ins.getDestGprIdx();
    out << CpuGpr::getName(destGpr);
    out << " = ";
    out << CpuOpcodeUtils::getMnemonic(ins.opcode);
    out.put(' ');
    out << CpuGpr::getName(in1Gpr);
    out << ", ";
    PrintUtils::printHexI16(in2U16, false, out);
}

static void printI16HexOffsetForInst(const int16_t offset, std::stringstream& out) noexcept {
    // Note: don't print the offset when it is zero
    if (offset > 0) {
        out << " + ";
        PrintUtils::printHexI16(offset, false, out);
    } else if (offset < 0) {
        out << " - ";
        const int32_t offsetI32 = offset;
        const uint32_t offsetAbsU32 = -offsetI32;
        PrintUtils::printHexU16((uint16_t) offsetAbsU32, false, out);
    }
}

//----------------------------------------------------------------------------------------------------------------------
// === MASTER DECODING CHEAT SHEET ===
//
// ADD      000000 SSSSS TTTTT DDDDD ----- 100000
// ADDI     001000 SSSSS TTTTT IIIII IIIII IIIIII
// ADDIU    001001 SSSSS TTTTT IIIII IIIII IIIIII
// ADDU     000000 SSSSS TTTTT DDDDD ----- 100001
// AND      000000 SSSSS TTTTT DDDDD ----- 100100
// ANDI     001100 SSSSS TTTTT IIIII IIIII IIIIII
// BEQ      000100 SSSSS TTTTT IIIII IIIII IIIIII
// BGEZ     000001 SSSSS 00001 IIIII IIIII IIIIII
// BGEZAL   000001 SSSSS 10001 IIIII IIIII IIIIII
// BGTZ     000111 SSSSS ----- IIIII IIIII IIIIII
// BLEZ     000110 SSSSS ----- IIIII IIIII IIIIII
// BLTZ     000001 SSSSS 00000 IIIII IIIII IIIIII
// BLTZAL   000001 SSSSS 10000 IIIII IIIII IIIIII
// BNE      000101 SSSSS TTTTT IIIII IIIII IIIIII
// BREAK    000000 IIIII IIIII IIIII IIIII 001101
// CFC2     010010 00010 TTTTT DDDDD ----- ------
// COP2     010010 1IIII IIIII IIIII IIIII IIIIII
// CTC2     010010 00110 TTTTT DDDDD ----- ------
// DIV      000000 SSSSS TTTTT ----- ----- 011010
// DIVU     000000 SSSSS TTTTT ----- ----- 011011
// J        000010 IIIII IIIII IIIII IIIII IIIIII
// JAL      000011 IIIII IIIII IIIII IIIII IIIIII
// JALR     000000 SSSSS ----- DDDDD ----- 001001
// JR       000000 SSSSS ----- ----- ----- 001000
// LB       100000 SSSSS TTTTT IIIII IIIII IIIIII
// LBU      100100 SSSSS TTTTT IIIII IIIII IIIIII
// LH       100001 SSSSS TTTTT IIIII IIIII IIIIII
// LHU      100101 SSSSS TTTTT IIIII IIIII IIIIII
// LUI      001111 ----- TTTTT IIIII IIIII IIIIII
// LW       100011 SSSSS TTTTT IIIII IIIII IIIIII
// LWC2     110010 SSSSS TTTTT IIIII IIIII IIIIII
// LWL      100010 SSSSS TTTTT IIIII IIIII IIIIII
// LWR      100110 SSSSS TTTTT IIIII IIIII IIIIII
// MFC0     010000 00000 TTTTT DDDDD ----- ------
// MFC2     010010 00000 TTTTT DDDDD ----- ------
// MFHI     000000 ----- ----- DDDDD ----- 010000
// MFLO     000000 ----- ----- DDDDD ----- 010010
// MTC0     010000 00100 TTTTT DDDDD ----- ------
// MTC2     010010 00100 TTTTT DDDDD ----- ------
// MTHI     000000 SSSSS ----- ----- ----- 010001
// MTLO     000000 SSSSS ----- ----- ----- 010011
// MULT     000000 SSSSS TTTTT ----- ----- 011000
// MULTU    000000 SSSSS TTTTT ----- ----- 011001
// NOR      000000 SSSSS TTTTT DDDDD ----- 100111
// OR       000000 SSSSS TTTTT DDDDD ----- 100101
// ORI      001101 SSSSS TTTTT IIIII IIIII IIIIII
// RFE      010000 10000 ----- ----- ----- 010000
// SB       101000 SSSSS TTTTT IIIII IIIII IIIIII
// SH       101001 SSSSS TTTTT IIIII IIIII IIIIII
// SLL      000000 ----- TTTTT DDDDD IIIII 000000
// SLLV     000000 SSSSS TTTTT DDDDD ----- 000100
// SLT      000000 SSSSS TTTTT DDDDD ----- 101010
// SLTI     001010 SSSSS TTTTT IIIII IIIII IIIIII
// SLTIU    001011 SSSSS TTTTT IIIII IIIII IIIIII
// SLTU     000000 SSSSS TTTTT DDDDD ----- 101011
// SRA      000000 ----- TTTTT DDDDD IIIII 000011
// SRAV     000000 SSSSS TTTTT DDDDD ----- 000111
// SRL      000000 ----- TTTTT DDDDD IIIII 000010
// SRLV     000000 SSSSS TTTTT DDDDD ----- 000110
// SUB      000000 SSSSS TTTTT DDDDD ----- 100010
// SUBU     000000 SSSSS TTTTT DDDDD ----- 100011
// SW       101011 SSSSS TTTTT IIIII IIIII IIIIII
// SWC2     111010 SSSSS TTTTT IIIII IIIII IIIIII
// SWL      101010 SSSSS TTTTT IIIII IIIII IIIIII
// SWR      101110 SSSSS TTTTT IIIII IIIII IIIIII
// SYSCALL  000000 IIIII IIIII IIIII IIIII 001100
// TEQ      000000 SSSSS TTTTT IIIII IIIII 110100
// TEQI     000001 SSSSS 01100 IIIII IIIII IIIIII
// TGE      000000 SSSSS TTTTT IIIII IIIII 110000
// TGEI     000001 SSSSS 01000 IIIII IIIII IIIIII
// TGEIU    000001 SSSSS 01001 IIIII IIIII IIIIII
// TGEU     000000 SSSSS TTTTT IIIII IIIII 110001
// TLBP     010000 10000 ----- ----- ----- 001000
// TLBR     010000 10000 ----- ----- ----- 000001
// TLBWI    010000 10000 ----- ----- ----- 000010
// TLBWR    010000 10000 ----- ----- ----- 000110
// TLT      000000 SSSSS TTTTT IIIII IIIII 110010
// TLTI     000001 SSSSS 01010 IIIII IIIII IIIIII
// TLTIU    000001 SSSSS 01011 IIIII IIIII IIIIII
// TLTU     000000 SSSSS TTTTT IIIII IIIII 110011
// TNE      000000 SSSSS TTTTT IIIII IIIII 110110
// TNEI     000001 SSSSS 01110 IIIII IIIII IIIIII
// XOR      000000 SSSSS TTTTT DDDDD ----- 100110
// XORI     001110 SSSSS TTTTT IIIII IIIII IIIIII
//----------------------------------------------------------------------------------------------------------------------

static bool decodeMainOpcode0Ins(CpuInstruction& ins, const uint32_t machineCode26Bit) noexcept {
    // -----------------------------------------------------------------------------------------------------------------
    // === MAIN OPCODE '0' INSTRUCTIONS (low 26-bits) ===
    // 
    // ADD      SSSSS TTTTT DDDDD ----- 100000
    // ADDU     SSSSS TTTTT DDDDD ----- 100001
    // AND      SSSSS TTTTT DDDDD ----- 100100
    // BREAK    IIIII IIIII IIIII IIIII 001101
    // DIV      SSSSS TTTTT ----- ----- 011010
    // DIVU     SSSSS TTTTT ----- ----- 011011
    // JALR     SSSSS ----- DDDDD ----- 001001
    // JR       SSSSS ----- ----- ----- 001000
    // MFHI     ----- ----- DDDDD ----- 010000
    // MFLO     ----- ----- DDDDD ----- 010010
    // MTHI     SSSSS ----- ----- ----- 010001
    // MTLO     SSSSS ----- ----- ----- 010011
    // MULT     SSSSS TTTTT ----- ----- 011000
    // MULTU    SSSSS TTTTT ----- ----- 011001
    // NOR      SSSSS TTTTT DDDDD ----- 100111
    // OR       SSSSS TTTTT DDDDD ----- 100101
    // SLL      ----- TTTTT DDDDD IIIII 000000
    // SLLV     SSSSS TTTTT DDDDD ----- 000100
    // SLT      SSSSS TTTTT DDDDD ----- 101010
    // SLTU     SSSSS TTTTT DDDDD ----- 101011
    // SRA      ----- TTTTT DDDDD IIIII 000011
    // SRAV     SSSSS TTTTT DDDDD ----- 000111
    // SRL      ----- TTTTT DDDDD IIIII 000010
    // SRLV     SSSSS TTTTT DDDDD ----- 000110
    // SUB      SSSSS TTTTT DDDDD ----- 100010
    // SUBU     SSSSS TTTTT DDDDD ----- 100011
    // SYSCALL  IIIII IIIII IIIII IIIII 001100
    // TEQ      SSSSS TTTTT IIIII IIIII 110100
    // TGE      SSSSS TTTTT IIIII IIIII 110000
    // TGEU     SSSSS TTTTT IIIII IIIII 110001
    // TLT      SSSSS TTTTT IIIII IIIII 110010
    // TLTU     SSSSS TTTTT IIIII IIIII 110011
    // TNE      SSSSS TTTTT IIIII IIIII 110110
    // XOR      SSSSS TTTTT DDDDD ----- 100110
    // -----------------------------------------------------------------------------------------------------------------

    // Decode the secondary opcode and some of the most commonly used parameters upfront
    const uint32_t secondaryOpcode = machineCode26Bit & 0x3Fu;      // RETRIEVE: ----- ----- ----- ----- CCCCCC
    const uint8_t decodedRegS = (machineCode26Bit >> 21) & 0x1Fu;   // RETRIEVE: SSSSS ----- ----- ----- ------
    const uint8_t decodedRegT = (machineCode26Bit >> 16) & 0x1Fu;   // RETRIEVE: ----- TTTTT ----- ----- ------
    const uint8_t decodedRegD = (machineCode26Bit >> 11) & 0x1Fu;   // RETRIEVE: ----- ----- DDDDD ----- ------

    switch (secondaryOpcode) {
        case 0b000000:  // SLL      ----- TTTTT DDDDD IIIII 000000
            ins.opcode = CpuOpcode::SLL;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x1Fu;
            return true;

        case 0b000010:  // SRL      ----- TTTTT DDDDD IIIII 000010
            ins.opcode = CpuOpcode::SRL;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x1Fu;
            return true;

        case 0b000011:  // SRA      ----- TTTTT DDDDD IIIII 000011
            ins.opcode = CpuOpcode::SRA;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x1Fu;
            return true;

        case 0b000100:  // SLLV     SSSSS TTTTT DDDDD ----- 000100
            ins.opcode = CpuOpcode::SRL;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b000110:  // SRLV     SSSSS TTTTT DDDDD ----- 000110
            ins.opcode = CpuOpcode::SRLV;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b000111:  // SRAV     SSSSS TTTTT DDDDD ----- 000111
            ins.opcode = CpuOpcode::SRAV;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b001000:  // JR       SSSSS ----- ----- ----- 001000
            ins.opcode = CpuOpcode::JR;
            ins.regS = decodedRegS;
            return true;

        case 0b001001:  // JALR     SSSSS ----- DDDDD ----- 001001
            ins.opcode = CpuOpcode::JR;
            ins.regS = decodedRegS;
            ins.regD = decodedRegD;
            return true;

        case 0b001100:  // SYSCALL  IIIII IIIII IIIII IIIII 001100
            ins.opcode = CpuOpcode::SYSCALL;
            ins.immediateVal = (machineCode26Bit >> 6);
            return true;

        case 0b001101:  // BREAK    IIIII IIIII IIIII IIIII 001101
            ins.opcode = CpuOpcode::BREAK;
            ins.immediateVal = (machineCode26Bit >> 6);
            return true;

        case 0b010000:  // MFHI     ----- ----- DDDDD ----- 010000
            ins.opcode = CpuOpcode::MFHI;
            ins.regD = decodedRegD;
            return true;

        case 0b010001:  // MTHI     SSSSS ----- ----- ----- 010001
            ins.opcode = CpuOpcode::MTHI;
            ins.regS = decodedRegS;
            return true;

        case 0b010010:  // MFLO     ----- ----- DDDDD ----- 010010
            ins.opcode = CpuOpcode::MFLO;
            ins.regD = decodedRegD;
            return true;

        case 0b010011:  // MTLO     SSSSS ----- ----- ----- 010011
            ins.opcode = CpuOpcode::MTLO;
            ins.regS = decodedRegS;
            return true;

        case 0b011000:  // MULT     SSSSS TTTTT ----- ----- 011000
            ins.opcode = CpuOpcode::MULT;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            return true;

        case 0b011001:  // MULTU    SSSSS TTTTT ----- ----- 011001
            ins.opcode = CpuOpcode::MULTU;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            return true;

        case 0b011010:  // DIV      SSSSS TTTTT ----- ----- 011010
            ins.opcode = CpuOpcode::DIV;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            return true;

        case 0b011011:  // DIVU     SSSSS TTTTT ----- ----- 011011
            ins.opcode = CpuOpcode::DIVU;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            return true;

        case 0b100000:  // ADD      SSSSS TTTTT DDDDD ----- 100000
            ins.opcode = CpuOpcode::ADD;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b100001:  // ADDU     SSSSS TTTTT DDDDD ----- 100001
            ins.opcode = CpuOpcode::ADDU;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b100010:  // SUB      SSSSS TTTTT DDDDD ----- 100010
            ins.opcode = CpuOpcode::SUB;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b100011:  // SUBU     SSSSS TTTTT DDDDD ----- 100011
            ins.opcode = CpuOpcode::SUBU;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b100100:  // AND      SSSSS TTTTT DDDDD ----- 100100
            ins.opcode = CpuOpcode::AND;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b100101:  // OR       SSSSS TTTTT DDDDD ----- 100101
            ins.opcode = CpuOpcode::OR;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b100110:  // XOR      SSSSS TTTTT DDDDD ----- 100110
            ins.opcode = CpuOpcode::XOR;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b100111:  // NOR      SSSSS TTTTT DDDDD ----- 100111
            ins.opcode = CpuOpcode::NOR;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b101010:  // SLT      SSSSS TTTTT DDDDD ----- 101010
            ins.opcode = CpuOpcode::SLT;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b101011:  // SLTU     SSSSS TTTTT DDDDD ----- 101011
            ins.opcode = CpuOpcode::SLTU;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.regD = decodedRegD;
            return true;

        case 0b110000:  // TGE      SSSSS TTTTT IIIII IIIII 110000
            ins.opcode = CpuOpcode::TGE;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x3FFu;
            return true;

        case 0b110001:  // TGEU     SSSSS TTTTT IIIII IIIII 110001
            ins.opcode = CpuOpcode::TGEU;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x3FFu;
            return true;

        case 0b110010:  // TLT      SSSSS TTTTT IIIII IIIII 110010
            ins.opcode = CpuOpcode::TLT;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x3FFu;
            return true;

        case 0b110011:  // TLTU     SSSSS TTTTT IIIII IIIII 110011
            ins.opcode = CpuOpcode::TLTU;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x3FFu;
            return true;

        case 0b110100:  // TEQ      SSSSS TTTTT IIIII IIIII 110100
            ins.opcode = CpuOpcode::TEQ;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x3FFu;
            return true;

        case 0b110110:  // TNE      SSSSS TTTTT IIIII IIIII 110110
            ins.opcode = CpuOpcode::TNE;
            ins.regS = decodedRegS;
            ins.regT = decodedRegT;
            ins.immediateVal = (machineCode26Bit >> 6) & 0x3FFu;
            return true;

        // Illegal secondary opcode
        default: break;
    }

    // If we get to here then it's because decoding has failed
    return false;
}

static bool decodeMainOpcode1Ins(CpuInstruction& ins, const uint32_t machineCode26Bit) noexcept {
    // -----------------------------------------------------------------------------------------------------------------
    // === MAIN OPCODE '1' INSTRUCTIONS (low 26-bits) ===
    //
    // BGEZ     SSSSS 00001 IIIII IIIII IIIIII
    // BGEZAL   SSSSS 10001 IIIII IIIII IIIIII
    // BLTZ     SSSSS 00000 IIIII IIIII IIIIII
    // BLTZAL   SSSSS 10000 IIIII IIIII IIIIII
    // TEQI     SSSSS 01100 IIIII IIIII IIIIII
    // TGEI     SSSSS 01000 IIIII IIIII IIIIII
    // TGEIU    SSSSS 01001 IIIII IIIII IIIIII
    // TLTI     SSSSS 01010 IIIII IIIII IIIIII
    // TLTIU    SSSSS 01011 IIIII IIIII IIIIII
    // TNEI     SSSSS 01110 IIIII IIIII IIIIII
    //------------------------------------------------------------------------------------------------------------------

    // Decode the secondary opcode and instruction params.
    // Assign optimistically to the instruciton for now:
    const uint8_t secondaryOpcode = (uint8_t)((machineCode26Bit >> 16) & 0x1Fu);    // RETRIEVE: ----- CCCCC ----- ----- ------
    ins.regS = (uint8_t)((machineCode26Bit >> 21) & 0x1Fu);                         // RETRIEVE: SSSSS ----- ----- ----- ------
    ins.immediateVal = machineCode26Bit & 0xFFFFu;                                  // RETRIEVE: ----- ----- IIIII IIIII IIIIII

    switch (secondaryOpcode) {
        case 0b00000:   // BLTZ     SSSSS 00000 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::BLTZ;
            return true;

        case 0b00001:   // BGEZ     SSSSS 00001 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::BGEZ;
            return true;

        case 0b01000:   // TGEI     SSSSS 01000 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::TGEI;
            return true;

        case 0b01001:   // TGEIU    SSSSS 01001 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::TGEIU;
            return true;
        
        case 0b01010:   // TLTI     SSSSS 01010 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::TLTI;
            return true;

        case 0b01011:   // TLTIU    SSSSS 01011 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::TLTIU;
            return true;
            
        case 0b01100:   // TEQI     SSSSS 01100 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::TEQI;
            return true;

        case 0b01110:   // TNEI     SSSSS 01110 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::TNEI;
            return true;

        case 0b10000:   // BLTZAL   SSSSS 10000 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::BLTZAL;
            return true;

        case 0b10001:   // BGEZAL   SSSSS 10001 IIIII IIIII IIIIII
            ins.opcode = CpuOpcode::BGEZAL;
            return true;
                
        // Illegal secondary opcode
        default: break;
    }

    // If we get to here then it's because decoding has failed.
    // Need to clear the instruction also because we assigned optimistically.
    ins.clear();
    return false;
}

static bool decodeMainOpcode16Ins(CpuInstruction& ins, const uint32_t machineCode26Bit) noexcept {
    // -----------------------------------------------------------------------------------------------------------------
    // === MAIN OPCODE '16' INSTRUCTIONS (low 26-bits) ===
    //
    // MFC0     00000 TTTTT DDDDD ----- ------
    // MTC0     00100 TTTTT DDDDD ----- ------
    // TLBR     10000 ----- ----- ----- 000001
    // TLBWI    10000 ----- ----- ----- 000010
    // TLBWR    10000 ----- ----- ----- 000110
    // TLBP     10000 ----- ----- ----- 001000
    // RFE      10000 ----- ----- ----- 010000
    //------------------------------------------------------------------------------------------------------------------

    // Retrieve the secondary opcode (top 5 of the 26 bits) and then decide what to do based on that
    const uint8_t secondaryOpcode = (uint8_t)(machineCode26Bit >> 21);

    if (secondaryOpcode == 0b00000 || secondaryOpcode == 0b00100) {
        // Either 'MFC0' or 'MTC0'.
        // Decode the parameters and save the instruction opcode:
        ins.opcode = (secondaryOpcode == 0b00000) ? CpuOpcode::MFC0 : CpuOpcode::MTC0;
        ins.regT = (machineCode26Bit >> 16) & 0x1Fu;
        ins.regD = (machineCode26Bit >> 11) & 0x1Fu;
        return true;
    }
    else if (secondaryOpcode == 0b10000) {
        // Possibly one of the 'TLBx' instructions or 'RFE'.
        // Decode the 3rd opcode (last 6-bits) to decide which:
        const uint8_t tertiaryOpcode = (uint8_t)(machineCode26Bit & 0x3F);

        switch (tertiaryOpcode) {
            case 0b000001: ins.opcode = CpuOpcode::TLBR;    return true;
            case 0b000010: ins.opcode = CpuOpcode::TLBWI;   return true;
            case 0b000110: ins.opcode = CpuOpcode::TLBWR;   return true;
            case 0b001000: ins.opcode = CpuOpcode::TLBP;    return true;
            case 0b010000: ins.opcode = CpuOpcode::RFE;     return true;

            // Invalid tertiary opcode
            default: break;
        }
    }

    // Decoding has failed if we have got to here
    return false;
}

static bool decodeMainOpcode18Ins(CpuInstruction& ins, const uint32_t machineCode26Bit) noexcept {
    // -----------------------------------------------------------------------------------------------------------------
    // === MAIN OPCODE '18' INSTRUCTIONS (low 26-bits) ===
    //
    // CFC2     00010 TTTTT DDDDD ----- ------
    // COP2     1IIII IIIII IIIII IIIII IIIIII
    // CTC2     00110 TTTTT DDDDD ----- ------
    // MFC2     00000 TTTTT DDDDD ----- ------
    // MTC2     00100 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------

    // Decode the top 5 bits, which is probably the opcode for this instruction.
    // If the top bit is set however then we are decoding the 'COP2' instruction:
    const uint8_t secondaryOpcode = (uint8_t)(machineCode26Bit >> 21);

    if ((secondaryOpcode & 0x10) != 0) {
        // This is the 'COP2' opcode.
        // 25-bits are used for a parameter for the instruction:
        ins.opcode = CpuOpcode::COP2;
        ins.immediateVal = machineCode26Bit & 0x1FFFFFF;
    } else {
        // Maybe one of the other opcodes listed above.
        // Decode the parameters optimistically and see which secondary opcode matches:
        ins.regT = (uint8_t)((machineCode26Bit >> 16) & 0x1Fu);
        ins.regD = (uint8_t)((machineCode26Bit >> 11) & 0x1Fu);

        switch (secondaryOpcode) {
            case 0b00000: ins.opcode = CpuOpcode::MFC2;     return true;
            case 0b00010: ins.opcode = CpuOpcode::CFC2;     return true;
            case 0b00100: ins.opcode = CpuOpcode::MTC2;     return true;
            case 0b00110: ins.opcode = CpuOpcode::CTC2;     return true;

            // Invalid secondary opcode
            default: break;
        }
    }

    // Decoding has failed if we have got to here.
    // Need to clear the instruction also because we assigned optimistically.
    ins.clear();
    return false;
}

bool CpuInstruction::decode(const uint32_t machineCode) noexcept {
    // Clear all opcode fields before we start
    clear();

    // Get the first 6 most significant bits first since that is the main opcode.
    // Also remove the top 6 bits from the instruction word for later decoding.
    // Also pre-decode some commonly used instruction parameters here once.
    const uint8_t mainOpcode = (uint8_t)(machineCode >> 26);                    // RETRIEVE: CCCCCC ----- ----- ----- ----- ------
    const uint32_t machineCode26Bit = machineCode & 0x03FFFFFF;                 // RETRIEVE: ------ XXXXX XXXXX XXXXX XXXXX XXXXXX
    const uint8_t decodedRegS = (uint8_t)((machineCode26Bit >> 21) & 0x1Fu);    // RETRIEVE: ------ SSSSS ----- ----- ----- ------
    const uint8_t decodedRegT = (uint8_t)((machineCode26Bit >> 16) & 0x1Fu);    // RETRIEVE: ------ ----- TTTTT ----- ----- ------
    const uint16_t decodedImm16 = (uint16_t) machineCode26Bit;                  // RETRIEVE: ------ ----- ----- IIIII IIIII IIIIII

    switch (mainOpcode) {
        case 0b000000: return decodeMainOpcode0Ins(*this, machineCode26Bit);
        case 0b000001: return decodeMainOpcode1Ins(*this, machineCode26Bit);
        
        case 0b000010:  // J        000010 IIIII IIIII IIIII IIIII IIIIII
            opcode = CpuOpcode::J;
            immediateVal = machineCode26Bit;
            return true;

        case 0b000011:  // JAL      000011 IIIII IIIII IIIII IIIII IIIIII
            opcode = CpuOpcode::JAL;
            immediateVal = machineCode26Bit;
            return true;
        
        case 0b000100:  // BEQ      000100 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::BEQ;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;

        case 0b000101:  // BNE      000101 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::BNE;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b000110:  // BLEZ     000110 SSSSS ----- IIIII IIIII IIIIII
            opcode = CpuOpcode::BLEZ;
            regS = decodedRegS;
            immediateVal = decodedImm16;
            return true;

        case 0b000111:  // BGTZ     000111 SSSSS ----- IIIII IIIII IIIIII
            opcode = CpuOpcode::BGTZ;
            regS = decodedRegS;
            immediateVal = decodedImm16;
            return true;
        
        case 0b001000:  // ADDI     001000 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::ADDI;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b001001:  // ADDIU    001001 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::ADDIU;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b001010:  // SLTI     001010 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::SLTI;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b001011:  // SLTIU    001011 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::SLTIU;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b001100:  // ANDI     001100 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::ANDI;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b001101:  // ORI      001101 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::ORI;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b001110:  // XORI     001110 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::XORI;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;

        case 0b001111:  // LUI      001111 ----- TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LUI;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b010000: return decodeMainOpcode16Ins(*this, machineCode26Bit);
        case 0b010010: return decodeMainOpcode18Ins(*this, machineCode26Bit);

        case 0b100000:  // LB       100000 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LB;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;

        case 0b100001:  // LH       100001 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LH;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b100010:  // LWL      100010 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LWL;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b100011:  // LW       100011 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LW;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b100100:  // LBU      100100 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LBU;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;

        case 0b100101:  // LHU      100101 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LHU;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;

        case 0b100110:  // LWR      100110 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LWR;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
 
        case 0b101000:  // SB       101000 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::SB;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b101001:  // SH       101001 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::SH;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b101010:  // SWL      101010 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::SWL;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b101011:  // SW       101011 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::SW;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b101110:  // SWR      101110 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::SWR;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b110010:  // LWC2     110010 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::LWC2;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;
        
        case 0b111010:  // SWC2     111010 SSSSS TTTTT IIIII IIIII IIIIII
            opcode = CpuOpcode::SWC2;
            regS = decodedRegS;
            regT = decodedRegT;
            immediateVal = decodedImm16;
            return true;

        // Illegal main opcode
        default: break;
    }

    // If we get to here then it's because decoding has failed
    return false;
}

uint8_t CpuInstruction::getDestGprIdx() const noexcept {
    switch (opcode) {
        case CpuOpcode::ADD:        return regD;
        case CpuOpcode::ADDI:       return regT;
        case CpuOpcode::ADDIU:      return regT;
        case CpuOpcode::ADDU:       return regD;
        case CpuOpcode::AND:        return regD;
        case CpuOpcode::ANDI:       return regT;
        case CpuOpcode::CFC2:       return regT;
        case CpuOpcode::LB:         return regT;
        case CpuOpcode::LBU:        return regT;
        case CpuOpcode::LH:         return regT;
        case CpuOpcode::LHU:        return regT;
        case CpuOpcode::LUI:        return regT;
        case CpuOpcode::LW:         return regT;
        case CpuOpcode::LWL:        return regT;
        case CpuOpcode::LWR:        return regT;
        case CpuOpcode::MFC0:       return regT;
        case CpuOpcode::MFC2:       return regT;
        case CpuOpcode::MFHI:       return regD;
        case CpuOpcode::MFLO:       return regD;
        case CpuOpcode::NOR:        return regD;
        case CpuOpcode::OR:         return regD;
        case CpuOpcode::ORI:        return regT;
        case CpuOpcode::SLL:        return regD;
        case CpuOpcode::SLLV:       return regD;
        case CpuOpcode::SLT:        return regD;
        case CpuOpcode::SLTI:       return regT;
        case CpuOpcode::SLTIU:      return regT;
        case CpuOpcode::SLTU:       return regD;
        case CpuOpcode::SRA:        return regD;
        case CpuOpcode::SRAV:       return regD;
        case CpuOpcode::SRL:        return regD;
        case CpuOpcode::SRLV:       return regD;
        case CpuOpcode::SUB:        return regD;
        case CpuOpcode::SUBU:       return regD;
        case CpuOpcode::XOR:        return regD;
        case CpuOpcode::XORI:       return regT;

        default: break;
    }
    
    // No destination GPR for this instruction
    return 0xFFu;
}

bool CpuInstruction::isNOP() const noexcept {
    // If the instruction outputs to a register and that register is the special '$(ZERO)' register then this is a NOP.
    // Writes to '$(ZERO)' have no effect, because it is not a real register:
    const uint8_t destGprIdx = getDestGprIdx();

    if (destGprIdx < CpuGpr::NUM_GPRS) {
        if (destGprIdx == CpuGpr::ZERO) {
            // I am a NOP because I write to '$(ZERO)'!
            // All instructions that write directly to a GPR shouldn't have any side effects other than that, so this check works.
            return true;
        }
    }

    // NOP based on operation type
    switch (opcode) {
        // NOPs for the case of adding '0' to a register and storing the result in the same register:
        case CpuOpcode::ADD:    return ((regT == CpuGpr::ZERO) && (regS == regD));
        case CpuOpcode::ADDI:   return ((immediateVal == 0) && (regS == regT));
        case CpuOpcode::ADDIU:  return ((immediateVal == 0) && (regS == regT));
        case CpuOpcode::ADDU:   return ((regT == CpuGpr::ZERO) && (regS == regD));
        
        // NOPs for branch conditions that can never be true (0 < 0 and 0 > 0)
        case CpuOpcode::BGTZ:       return (regS == CpuGpr::ZERO);
        case CpuOpcode::BLTZ:       return (regS == CpuGpr::ZERO);
        case CpuOpcode::BLTZAL:     return (regS == CpuGpr::ZERO);

        // NOP if testing the same register is NOT EQUAL to itself
        case CpuOpcode::BNE:
            return (regS == regT);
        
        // NOPs for OR-ing a register with '0' and storing the result to the same register
        case CpuOpcode::OR:
            return (
                ((regS == CpuGpr::ZERO) && (regD == regT)) |
                ((regT == CpuGpr::ZERO) && (regD == regS))
            );

        case CpuOpcode::ORI:
            return ((immediateVal == 0) && (regS == regT));

        // NOPs for when doing shifts of '0' of a register and storing the result in the same register
        case CpuOpcode::SLL:    return ((immediateVal == 0) && (regD == regT));
        case CpuOpcode::SLLV:   return ((regS == CpuGpr::ZERO) && (regD == regT));
        case CpuOpcode::SRA:    return ((immediateVal == 0) && (regD == regT));        
        case CpuOpcode::SRAV:   return ((regS == CpuGpr::ZERO) && (regD == regT));
        case CpuOpcode::SRL:    return ((immediateVal == 0) && (regD == regT));
        case CpuOpcode::SRLV:   return ((regS == CpuGpr::ZERO) && (regD == regT));

        // NOPs for the case of subtracting '0' from a register and storing the result in the same register:
        case CpuOpcode::SUB:    return ((regT == CpuGpr::ZERO) && (regS == regD));
        case CpuOpcode::SUBU:   return ((regT == CpuGpr::ZERO) && (regS == regD));

        // NOPs for XOR-ing a register with '0' and storing the result in the same register:
        case CpuOpcode::XOR:    return ((regT == CpuGpr::ZERO) && (regS == regD));
        case CpuOpcode::XORI:   return ((immediateVal == 0) && (regS == regT));

        // Instruction does not generate NOPs!
        default: break;
    }

    return false;   // Not a NOP
}

uint32_t CpuInstruction::getBranchInstTargetAddr(const uint32_t thisInstAddr) const noexcept {
    // All branch instructions use a signed 16-bit WORD (note: not byte!) index as their immediate value.
    // This plus the instruction address tells us where to go.
    // Note that offset is relative to the address FOLLOWING the branch instruction, hence we add '4' bytes:
    const int32_t effectiveOffset = ((int32_t)(int16_t) immediateVal) * sizeof(uint32_t);
    return thisInstAddr + (uint32_t) effectiveOffset + 4;
}

uint32_t CpuInstruction::getFixedJumpInstTargetAddr(const uint32_t thisInstAddr) const noexcept {
    // The jump instruction encodes a 28-bit (256 MB) jump within 26-bits by shifting left 2-bits.
    // Therefore the jump must be within the current 256 MB region.
    // Use the top 4-bits of the instruction address as the region we are jumping within:
    const uint32_t baseAddr = thisInstAddr & 0xF0000000;
    const uint32_t offset = immediateVal << 2;
    return baseAddr + offset;
}

void CpuInstruction::print(const uint32_t thisInstAddr, std::stringstream& out) const noexcept {
    // Easy case: handling illegal instructions
    if (isIllegal()) {
        out << "<ILLEGAL INSTRUCTION>";
        return;
    }

    // Printing each opcode type
    switch (opcode) {
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
            printInstGprOutGprInGprIn(*this, regS, regT, out);
            break;

        // REG_OUT = OPERATION(regT, regS)
        case CpuOpcode::SLLV:
        case CpuOpcode::SRAV:
        case CpuOpcode::SRLV:
            printInstGprOutGprInGprIn(*this, regT, regS, out);
            break;

        // REG_OUT = OPERATION(regS, (int16_t) immVal)
        case CpuOpcode::ADDI:
        case CpuOpcode::SLTI:
            printInstGprOutGprInI16In(*this, regS, (int16_t) immediateVal, out);
            break;

        // REG_OUT = OPERATION(regS, (uint16_t) immVal)
        case CpuOpcode::ADDIU:
        case CpuOpcode::ANDI:
        case CpuOpcode::ORI:
        case CpuOpcode::SLTIU:
        case CpuOpcode::XORI:
            printInstGprOutGprInU16In(*this, regS, (uint16_t) immediateVal, out);
            break;

        // REG_OUT = OPERATION(regT, (uint16_t) immVal)
        case CpuOpcode::SLL:
        case CpuOpcode::SRA:
        case CpuOpcode::SRL:
            printInstGprOutGprInU16In(*this, regT, (uint16_t) immediateVal, out);
            break;

        // Instructions that just print their mnemonic
        case CpuOpcode::RFE:
        case CpuOpcode::TLBP:
        case CpuOpcode::TLBR:
        case CpuOpcode::TLBWI:
        case CpuOpcode::TLBWR:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            break;

        // All the other mini-groups and individual instructions that can't be covered by the above:
        case CpuOpcode::BEQ:
        case CpuOpcode::BNE:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            out << CpuGpr::getName(regS);
            out << ", ";
            out << CpuGpr::getName(regT);
            out << ", ";
            PrintUtils::printHexU32(getBranchInstTargetAddr(thisInstAddr), false, out);
            break;
        
        case CpuOpcode::BGEZ:
        case CpuOpcode::BGEZAL:
        case CpuOpcode::BGTZ:
        case CpuOpcode::BLEZ:
        case CpuOpcode::BLTZ:
        case CpuOpcode::BLTZAL:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            out << CpuGpr::getName(regS);
            out << ", ";
            PrintUtils::printHexU32(getBranchInstTargetAddr(thisInstAddr), false, out);
            break;

        case CpuOpcode::BREAK:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            PrintUtils::printHexU32(immediateVal, false, out);
            break;

        case CpuOpcode::CFC2:
            out << CpuGpr::getName(getDestGprIdx());
            out << " = $cop2_ctrl[";
            out << (uint32_t) regD;
            out.put(']');
            break;

        case CpuOpcode::COP2:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            PrintUtils::printHexU32(immediateVal, true, out);
            break;

        case CpuOpcode::CTC2:
            out << "$cop2_ctrl[";
            out << (uint32_t) regD;
            out << "] = ";
            out << CpuGpr::getName(regT);
            break;

        case CpuOpcode::DIV:
        case CpuOpcode::DIVU:
        case CpuOpcode::MULT:
        case CpuOpcode::MULTU:
            out << "$hi, $lo = ";
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            out << CpuGpr::getName(regS);
            out << ", ";
            out << CpuGpr::getName(regT);
            break;

        case CpuOpcode::J:
        case CpuOpcode::JAL:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            PrintUtils::printHexU32(getFixedJumpInstTargetAddr(thisInstAddr), false, out);
            break;

        case CpuOpcode::JALR:
        case CpuOpcode::JR:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            out << CpuGpr::getName(regS);
            break;

        case CpuOpcode::LB:
        case CpuOpcode::LBU:
        case CpuOpcode::LH:
        case CpuOpcode::LHU:
        case CpuOpcode::LW:
            out << CpuGpr::getName(getDestGprIdx());
            out << " = ";
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out << " [";
            out << CpuGpr::getName(regS);
            printI16HexOffsetForInst((int16_t) immediateVal, out);
            out.put(']');
            break;

        case CpuOpcode::LUI:
            out << CpuGpr::getName(getDestGprIdx());
            out << " = ";
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            PrintUtils::printHexU16((uint16_t) immediateVal, true, out);
            break;

        case CpuOpcode::LWC2:
            out << "$cop2_data[";
            out << (uint32_t) regT;
            out << "] = ";
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out << " [";
            out << CpuGpr::getName(regS);
            printI16HexOffsetForInst((int16_t) immediateVal, out);
            out.put(']');
            break;

        case CpuOpcode::LWL:
        case CpuOpcode::LWR:
            out << CpuGpr::getName(getDestGprIdx());
            out << " = ";
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out << ' ';
            out << CpuGpr::getName(getDestGprIdx());
            out << ", [";
            out << CpuGpr::getName(regS);
            printI16HexOffsetForInst((int16_t) immediateVal, out);
            out.put(']');
            break;

        case CpuOpcode::MFC0:
        case CpuOpcode::MFC2:
            out << CpuGpr::getName(getDestGprIdx());
            out << ((opcode == CpuOpcode::MFC0) ? " = $cop0_data[" : " = $cop2_data[");     // N.B: Outer parens required due to op precedence!
            out << (uint32_t) regD;
            out.put(']');
            break;

        case CpuOpcode::MFHI:
        case CpuOpcode::MFLO:
            out << CpuGpr::getName(getDestGprIdx());
            out << ((opcode == CpuOpcode::MFHI) ? " = $hi" : " = $hi");     // N.B: Outer parens required due to op precedence!
            break;

        case CpuOpcode::MTC0:
        case CpuOpcode::MTC2:
            out << ((opcode == CpuOpcode::MTC0) ? "$cop0_data[" : "$cop2_data[");   // N.B: Outer parens required due to op precedence!
            out << (uint32_t) regD;
            out << "] = ";
            out << CpuGpr::getName(regT);
            break;

        case CpuOpcode::MTHI:
        case CpuOpcode::MTLO:
            out << ((opcode == CpuOpcode::MTHI) ? "$hi = " : "$lo = ");     // N.B: Outer parens required due to op precedence!
            out << CpuGpr::getName(regS);
            break;

        case CpuOpcode::SB:
        case CpuOpcode::SH:
        case CpuOpcode::SW:
        case CpuOpcode::SWL:
        case CpuOpcode::SWR:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            out << CpuGpr::getName(regT);
            out << ", [";
            out << CpuGpr::getName(regS);
            printI16HexOffsetForInst((int16_t) immediateVal, out);
            out.put(']');
            break;

        case CpuOpcode::SWC2:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out << " $cop2_data[";
            out << (uint32_t) regT;
            out << "], [";
            out << CpuGpr::getName(regS);
            printI16HexOffsetForInst((int16_t) immediateVal, out);
            out.put(']');
            break;

        case CpuOpcode::SYSCALL:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            PrintUtils::printHexU32(immediateVal, false, out);
            break;

        case CpuOpcode::TEQ:
        case CpuOpcode::TGE:
        case CpuOpcode::TGEU:
        case CpuOpcode::TLT:
        case CpuOpcode::TLTU:
        case CpuOpcode::TNE:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            out << CpuGpr::getName(regS);
            out << ", ";
            out << CpuGpr::getName(regT);
            out << ", ";
            PrintUtils::printHexU32(immediateVal, false, out);
            break;
        
        case CpuOpcode::TEQI:
        case CpuOpcode::TGEI:
        case CpuOpcode::TLTI:
        case CpuOpcode::TNEI:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            out << CpuGpr::getName(regS);
            out << ", ";
            PrintUtils::printHexI16((int16_t) immediateVal, false, out);
            break;

        case CpuOpcode::TGEIU:
        case CpuOpcode::TLTIU:
            out << CpuOpcodeUtils::getMnemonic(opcode);
            out.put(' ');
            out << CpuGpr::getName(regS);
            out << ", ";
            PrintUtils::printHexU32((uint32_t)(int32_t)(int16_t) immediateVal, false, out);
            break;

        default:
            out << "<NO INSTRUCTION PRINT IMPLEMENTED FOR OPCODE #";
            out << (uint32_t) opcode;
            out << "!>";
            break;
    }
}
