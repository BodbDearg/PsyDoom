#include "CpuInstruction.h"

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
// CFC2     010010 00010 TTTTT SSSSS ----- ------
// COP2     010010 1IIII IIIII IIIII IIIII IIIIII
// CTC2     010010 00110 TTTTT SSSSS ----- ------
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
// TLBP     010000 1---- ----- ----- ----- 001000
// TLBR     010000 1---- ----- ----- ----- 000001
// TLBWI    010000 1---- ----- ----- ----- 000010
// TLBWR    010000 1---- ----- ----- ----- 000110
// TLT      000000 SSSSS TTTTT IIIII IIIII 110010
// TLTI     000001 SSSSS 01010 IIIII IIIII IIIIII
// TLTIU    000001 SSSSS 01011 IIIII IIIII IIIIII
// TLTU     000000 SSSSS TTTTT IIIII IIIII 110011
// TNE      000000 SSSSS TTTTT IIIII IIIII 110110
// TNEI     000001 SSSSS 01110 IIIII IIIII IIIIII
// XOR      000000 SSSSS TTTTT DDDDD ----- 100110
// XORI     001110 SSSSS TTTTT IIIII IIIII IIIIII
//----------------------------------------------------------------------------------------------------------------------

static uint32_t signExtend16To32Bit(uint16_t val16) noexcept {
    return (uint32_t)(int32_t)(int16_t) val16;
}

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

    // TODO
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

    // TODO
    return false;
}

static bool decodeMainOpcode16Ins(CpuInstruction& ins, const uint32_t machineCode26Bit) noexcept {
    // -----------------------------------------------------------------------------------------------------------------
    // === MAIN OPCODE '16' INSTRUCTIONS (low 26-bits) ===
    //
    // MFC0     00000 TTTTT DDDDD ----- ------
    // MTC0     00100 TTTTT DDDDD ----- ------
    // TLBR     1---- ----- ----- ----- 000001
    // TLBWI    1---- ----- ----- ----- 000010
    // TLBWR    1---- ----- ----- ----- 000110
    // TLBP     1---- ----- ----- ----- 001000
    // RFE      10000 ----- ----- ----- 010000
    //------------------------------------------------------------------------------------------------------------------

    // TODO
    return false;
}

static bool decodeMainOpcode18Ins(CpuInstruction& ins, const uint32_t machineCode26Bit) noexcept {
    // -----------------------------------------------------------------------------------------------------------------
    // === MAIN OPCODE '18' INSTRUCTIONS (low 26-bits) ===
    //
    // CFC2     00010 TTTTT SSSSS ----- ------
    // COP2     1IIII IIIII IIIII IIIII IIIIII
    // CTC2     00110 TTTTT SSSSS ----- ------
    // MFC2     00000 TTTTT DDDDD ----- ------
    // MTC2     00100 TTTTT DDDDD ----- ------
    //------------------------------------------------------------------------------------------------------------------

    // TODO
    return false;
}

bool CpuInstruction::decode(const uint32_t machineCode) noexcept {
    // Clear all opcode fields before we start
    clear();

    // Get the first 6 most significant bits first since that is the main opcode.
    // Also remove the top 6 bits from the machine word for the convenience of decoding functions.
    const uint8_t mainOpcode = (uint8_t)(machineCode >> 26);
    const uint32_t machineCode26Bit = machineCode & 0x03FFFFFF;

    // Pre-decode some commonly used instruction parameters here once
    const uint8_t decodedRegS = (machineCode26Bit >> 21) & 0x1Fu;           // RETRIEVE: ------ SSSSS ----- ----- ----- ------
    const uint8_t decodedRegT = (machineCode26Bit >> 16) & 0x1Fu;           // RETRIEVE: ------ ----- TTTTT ----- ----- ------
    const uint16_t decodedImm16 = (uint16_t) machineCode26Bit;              // RETRIEVE: ------ ----- ----- IIIII IIIII IIIIII

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
            immediateVal = decodedRegS;
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
