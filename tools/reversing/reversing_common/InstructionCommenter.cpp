#include "InstructionCommenter.h"

#include "ConstInstructionEvaluator.h"
#include "CpuInstruction.h"
#include "ExeFile.h"
#include "PrintUtils.h"

static void prefixComment(
    const InstructionCommenter::CommentPrefixerFunc pCommentPrefixer,
    const uint32_t lineCol,
    std::ostream& out
) {
    if (pCommentPrefixer) {
        pCommentPrefixer(lineCol, out);
    }
}

static void printNameAndAddress(const uint32_t addr, const ExeFile& exe, std::ostream& out) {
    const bool bPrintedName = exe.printNameOfElemAtAddr(addr, out);

    // If we printed a name then print the address in brackets also
    if (bPrintedName) {
        out << " (";
        PrintUtils::printHexU32(addr, true, out);
        out.put(')');
    }
}

void InstructionCommenter::tryCommentInstruction(
    const CpuInstruction& inst,
    const uint32_t instAddr,
    const ExeFile& exe,
    const ConstInstructionEvaluator& constInstEvaluator,
    const CommentPrefixerFunc pCommentPrefixer,
    const uint32_t lineCol,
    std::ostream& out
) {
    // If the instruction is a NOP then there is nothing to do
    if (inst.isNOP())
        return;

    // Get the register states for this instruction
    ConstEvalRegState inRegState;
    ConstEvalRegState outRegState;
    constInstEvaluator.getInRegStateForInstruction(instAddr, inRegState);
    constInstEvaluator.getOutRegStateForInstruction(instAddr, outRegState);

    // Try to comment
    switch (inst.opcode) {
        // Comment the constant result of various operations that store to a GPR
        case CpuOpcode::ADD:
        case CpuOpcode::ADDI:
        case CpuOpcode::ADDIU:
        case CpuOpcode::ADDU:
        case CpuOpcode::AND:
        case CpuOpcode::ANDI:
        case CpuOpcode::MFHI:
        case CpuOpcode::MFLO:
        case CpuOpcode::NOR:
        case CpuOpcode::OR:
        case CpuOpcode::ORI:
        case CpuOpcode::SLL:
        case CpuOpcode::SLLV:
        case CpuOpcode::SLT:
        case CpuOpcode::SLTI:
        case CpuOpcode::SLTIU:
        case CpuOpcode::SLTU:
        case CpuOpcode::SRA:
        case CpuOpcode::SRAV:
        case CpuOpcode::SRL:
        case CpuOpcode::SRLV:
        case CpuOpcode::SUB:
        case CpuOpcode::SUBU:
        case CpuOpcode::XOR:
        case CpuOpcode::XORI: {
            const uint8_t destRegIdx = inst.getDestGprIdx();

            if (outRegState.bGprValueKnown[destRegIdx]) {
                prefixComment(pCommentPrefixer, lineCol, out);
                const uint32_t result = outRegState.gprValue[destRegIdx];
                out << "Result = ";
                printNameAndAddress(result, exe, out);
            }
        }   break;

        // For LUI just print a result, since it's always just used for the upper pointer of an address.
        // Even if we don't need the lower 16-bits for the address and nothing further follows LUI, we'll still comment the load anyway:
        case CpuOpcode::LUI: {
            const uint8_t destRegIdx = inst.getDestGprIdx();

            if (outRegState.bGprValueKnown[destRegIdx]) {
                prefixComment(pCommentPrefixer, lineCol, out);
                const uint32_t result = outRegState.gprValue[destRegIdx];
                out << "Result = ";
                PrintUtils::printHexU32(result, true, out);
            }
        }   break;

        // Divides and multiplies are special, they store their results in two registers
        case CpuOpcode::DIV:
        case CpuOpcode::DIVU:
        case CpuOpcode::MULT:
        case CpuOpcode::MULTU: {
            if (outRegState.bLoRegValueKnown && outRegState.bHiRegValueKnown) {
                prefixComment(pCommentPrefixer, lineCol, out);
                out << "Result = LO: ";
                printNameAndAddress(outRegState.loRegValue, exe, out);
                out << ", HI: ";
                printNameAndAddress(outRegState.hiRegValue, exe, out);
            }
        }   break;

        // If moving to the '$hi' or '$lo' register then comment what we are moving
        case CpuOpcode::MTHI:
        case CpuOpcode::MTLO: {
            if (inRegState.bGprValueKnown[inst.regS]) {
                const uint32_t result = inRegState.gprValue[inst.regS];
                out << "Result = ";
                printNameAndAddress(result, exe, out);
            }
        }   break;

        // Comment loads
        case CpuOpcode::LB:
        case CpuOpcode::LBU:
        case CpuOpcode::LH:
        case CpuOpcode::LHU:
        case CpuOpcode::LW: 
        case CpuOpcode::LWC2:
        case CpuOpcode::LWL:
        case CpuOpcode::LWR: {
            if (inRegState.bGprValueKnown[inst.regS]) {
                prefixComment(pCommentPrefixer, lineCol, out);
                const uint32_t s = inRegState.gprValue[inst.regS];
                const int32_t i = (int32_t)(int16_t)(uint16_t) inst.immediateVal;
                const uint32_t addr = s + i;
                out << "Load from: ";
                printNameAndAddress(addr, exe, out);
            }
        } break;

        // Comment stores
        case CpuOpcode::SB:
        case CpuOpcode::SH:
        case CpuOpcode::SW:
        case CpuOpcode::SWC2:
        case CpuOpcode::SWL:
        case CpuOpcode::SWR: {
            if (inRegState.bGprValueKnown[inst.regS]) {
                prefixComment(pCommentPrefixer, lineCol, out);
                const uint32_t s = inRegState.gprValue[inst.regS];
                const int32_t i = (int32_t)(int16_t)(uint16_t) inst.immediateVal;
                const uint32_t addr = s + i;
                out << "Store to: ";
                printNameAndAddress(addr, exe, out);
            }
        } break;

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
