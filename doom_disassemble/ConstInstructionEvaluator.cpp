#include "ConstInstructionEvaluator.h"

#include "ExeFile.h"
#include "FatalErrors.h"
#include "ProgElem.h"

// Maximum number of times we are allowed to re-execute an instruction.
// This is used to make us stop evaluating loops forever and is set > 1 so we can find constant stuff within loops.
static constexpr uint32_t MAX_INSTRUCTION_EVALUATIONS = 3;

//----------------------------------------------------------------------------------------------------------------------
// Does the actual work of evaluating a particular instruction
//----------------------------------------------------------------------------------------------------------------------
static void evalInstImpl(CpuInstruction& inst, const ConstEvalRegState& regIn, ConstEvalRegState& regOut) noexcept {
    const uint8_t resultGpr = inst.getDestGprIdx();

    // Evaluate all the different instruction types.
    // Note that if ANY inputs to an operation are unknown as a constant then the result will be unknown too:
    switch (inst.opcode) {
        case CpuOpcode::ADD: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const int64_t s = (int64_t)(int32_t) regIn.gprValue[inst.regS];
                const int64_t t = (int64_t)(int32_t) regIn.gprValue[inst.regT];
                const int64_t result64 = s + t;

                if (result64 >= INT32_MIN && result64 <= INT32_MAX) {
                    regOut.setGpr(resultGpr, (uint32_t) result64);
                }
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::ADDI: {
            if (regIn.bGprValueKnown[inst.regS]) {
                const int64_t s = (int64_t)(int32_t) regIn.gprValue[inst.regS];
                const int64_t i = (int64_t)(int16_t)(uint16_t) inst.immediateVal;
                const int64_t result64 = s + i;

                if (result64 >= INT32_MIN && result64 <= INT32_MAX) {
                    regOut.setGpr(resultGpr, (uint32_t) result64);
                }
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::ADDIU: {
            if (regIn.bGprValueKnown[inst.regS]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t i = inst.immediateVal & 0xFFFFu;
                regOut.setGpr(resultGpr, s + i);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::ADDU: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, s + t);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::AND: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, s & t);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::ANDI: {
            if (regIn.bGprValueKnown[inst.regS]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t i = inst.immediateVal & 0xFFFFu;
                regOut.setGpr(resultGpr, s & i);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        // Moving a control word from coprocessor 2 is not simulated, hence output register is undefined:
        case CpuOpcode::CFC2: {
            regOut.clearGpr(resultGpr);
        }   break;

        case CpuOpcode::DIV: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const int32_t s = (int32_t) regIn.gprValue[inst.regS];
                const int32_t t = (int32_t) regIn.gprValue[inst.regT];

                if (t != 0) {
                    regOut.setLo((uint32_t)(s / t));                        // The actual result
                    regOut.setHi((uint32_t)(s - s * regOut.loRegValue));    // Remainder
                } else {
                    regOut.clearHiAndLo();  // Div by '0' is UNDEFINED
                }
            } else {
                regOut.clearHiAndLo();
            }
        }   break;

        case CpuOpcode::DIVU: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];

                if (t != 0) {
                    regOut.setLo(s / t);                        // The actual result
                    regOut.setHi(s - s * regOut.loRegValue);    // Remainder
                } else {
                    regOut.clearHiAndLo();  // Div by '0' is UNDEFINED
                }
            } else {
                regOut.clearHiAndLo();
            }
        }   break;

        case CpuOpcode::LUI: {
            regOut.setGpr(resultGpr, inst.immediateVal << 16);
        }   break;

        // Loads invalidate the destination since memory locations cannot be constant evaluated
        case CpuOpcode::LB:
        case CpuOpcode::LBU:
        case CpuOpcode::LH:
        case CpuOpcode::LHU:
        case CpuOpcode::LW:
        case CpuOpcode::LWL:
        case CpuOpcode::LWR: {
            regOut.clearGpr(resultGpr);
        }   break;

        // Moving words from coprocessor 0 or 2 is not simulated, hence output register is undefined:
        case CpuOpcode::MFC0:
        case CpuOpcode::MFC2: {
            regOut.clearGpr(resultGpr);
        }   break;

        case CpuOpcode::MFHI: {
            if (regIn.bHiRegValueKnown) {
                regOut.setGpr(resultGpr, regIn.hiRegValue);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::MFLO: {
            if (regIn.bLoRegValueKnown) {
                regOut.setGpr(resultGpr, regIn.loRegValue);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::MTHI: {
            if (regIn.bGprValueKnown[inst.regS]) {
                regOut.setHi(regIn.gprValue[inst.regS]);
            } else {
                regOut.clearHi();
            }
        }   break;

        case CpuOpcode::MTLO: {
            if (regIn.bGprValueKnown[inst.regS]) {
                regOut.setLo(regIn.gprValue[inst.regS]);
            } else {
                regOut.clearLo();
            }
        }   break;

        case CpuOpcode::MULT: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const int64_t s = (int64_t)(int32_t) regIn.gprValue[inst.regS];
                const int64_t t = (int64_t)(int32_t) regIn.gprValue[inst.regT];
                const int64_t result64 = s * t;
                regOut.setLo((uint32_t) result64);
                regOut.setHi((uint32_t)(((uint64_t) result64) >> 32));
            } else {
                regOut.clearHiAndLo();
            }
        }   break;

        case CpuOpcode::MULTU: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint64_t s = regIn.gprValue[inst.regS];
                const uint64_t t = regIn.gprValue[inst.regT];
                const uint64_t result64 = s * t;
                regOut.setLo((uint32_t) result64);
                regOut.setHi((uint32_t)(result64 >> 32));
            } else {
                regOut.clearHiAndLo();
            }
        }   break;

        case CpuOpcode::NOR: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, ~(s | t));
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::OR: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, s | t);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::ORI: {
            if (regIn.bGprValueKnown[inst.regS]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t i = inst.immediateVal & 0xFFFFu;
                regOut.setGpr(resultGpr, s | i);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SLL: {
            if (regIn.bGprValueKnown[inst.regT]) {
                const uint32_t t = regIn.gprValue[inst.regT];
                const uint32_t i = inst.immediateVal & 0x1Fu;
                regOut.setGpr(resultGpr, t << i);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SLLV: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, t << s);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SLT: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const int32_t s = (int32_t) regIn.gprValue[inst.regS];
                const int32_t t = (int32_t) regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, (s < t) ? 1 : 0);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SLTI: {
            if (regIn.bGprValueKnown[inst.regS]) {
                const int32_t s = (int32_t) regIn.gprValue[inst.regS];
                const int32_t i = (int32_t)(int16_t)(uint16_t) inst.immediateVal;
                regOut.setGpr(resultGpr, (s < i) ? 1 : 0);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SLTIU: {
            if (regIn.bGprValueKnown[inst.regS]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t i = inst.immediateVal & 0xFFFFu;
                regOut.setGpr(resultGpr, (s < i) ? 1 : 0);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SLTU: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, (s < t) ? 1 : 0);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        // Note: assuming a signed shift will be ARITHMETIC, but it depends on compiler.
        // Most (all?) known major compilers should behave in this sane way.
        // If we get it wrong however then it's not the end of the world, since the evaluator
        // is just used for annotating disassembly instructions!
        case CpuOpcode::SRA: {
            if (regIn.bGprValueKnown[inst.regT]) {
                const int32_t t = (int32_t) regIn.gprValue[inst.regT];
                const uint32_t i = inst.immediateVal & 0x1Fu;
                regOut.setGpr(resultGpr, (uint32_t)(t >> i));
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SRAV: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const int32_t t = (int32_t) regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, (uint32_t)(t >> s));
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SRL: {
            if (regIn.bGprValueKnown[inst.regT]) {
                const uint32_t t = regIn.gprValue[inst.regT];
                const uint32_t i = inst.immediateVal & 0x1Fu;
                regOut.setGpr(resultGpr, t >> i);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SRLV: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, t >> s);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SUB: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const int64_t s = (int64_t)(int32_t) regIn.gprValue[inst.regS];
                const int64_t t = (int64_t)(int32_t) regIn.gprValue[inst.regT];
                const int64_t result64 = s - t;

                if (result64 >= INT32_MIN && result64 <= INT32_MAX) {
                    regOut.setGpr(resultGpr, (uint32_t) result64);
                }
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::SUBU: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, s - t);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::XOR: {
            if (regIn.bGprValueKnown[inst.regS] && regIn.bGprValueKnown[inst.regT]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t t = regIn.gprValue[inst.regT];
                regOut.setGpr(resultGpr, s ^ t);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        case CpuOpcode::XORI: {
            if (regIn.bGprValueKnown[inst.regS]) {
                const uint32_t s = regIn.gprValue[inst.regS];
                const uint32_t i = inst.immediateVal & 0xFFFFu;
                regOut.setGpr(resultGpr, s ^ i);
            } else {
                regOut.clearGpr(resultGpr);
            }
        }   break;

        // I don't know what these operatons do, so just wipe everything if we encounter them
        case CpuOpcode::BREAK:
        case CpuOpcode::SYSCALL: 
        case CpuOpcode::TLBP:
        case CpuOpcode::TLBR:
        case CpuOpcode::TLBWI:
        case CpuOpcode::TLBWR: {
            regOut.clear();
        }   break;

        // These trap instructions are not valid in MIPS I so do nothing if we encounter them
        case CpuOpcode::TEQ:
        case CpuOpcode::TEQI:
        case CpuOpcode::TGE:
        case CpuOpcode::TGEI:
        case CpuOpcode::TGEIU:
        case CpuOpcode::TGEU:
        case CpuOpcode::TLT:
        case CpuOpcode::TLTI:
        case CpuOpcode::TLTIU:
        case CpuOpcode::TLTU:
        case CpuOpcode::TNE:
        case CpuOpcode::TNEI:
            break;

        // These opcodes do not affect any GPRs or the HI/LO register:
        case CpuOpcode::BEQ:
        case CpuOpcode::BGEZ:
        case CpuOpcode::BGEZAL:
        case CpuOpcode::BGTZ:
        case CpuOpcode::BLEZ:
        case CpuOpcode::BLTZ:
        case CpuOpcode::BLTZAL:
        case CpuOpcode::BNE:        
        case CpuOpcode::COP2:
        case CpuOpcode::CTC2:
        case CpuOpcode::J:
        case CpuOpcode::JAL:
        case CpuOpcode::JALR:
        case CpuOpcode::JR:
        case CpuOpcode::LWC2:
        case CpuOpcode::MTC0:
        case CpuOpcode::MTC2:
        case CpuOpcode::RFE:
        case CpuOpcode::SB:
        case CpuOpcode::SH:
        case CpuOpcode::SW:
        case CpuOpcode::SWC2:
        case CpuOpcode::SWL:
        case CpuOpcode::SWR:
            break;

        default:
            FATAL_ERROR("Unhandled instruction type!");
            break;
    } 
}

ConstInstructionEvaluator::ConstInstructionEvaluator() noexcept
    : mFuncStartAddr(0)
    , mFuncEndAddr(0)
    , mInstructions()
    , mBranchPathsToExec()
{
}

ConstInstructionEvaluator::~ConstInstructionEvaluator() noexcept {
    // Nothing to do...
}

void ConstInstructionEvaluator::constEvalFunction(const ExeFile& exe, const ProgElem& progElem, const ConstEvalRegState& inputRegState) noexcept {
    initEvaluator(exe, progElem, inputRegState);

    while (true) {
        // See if there are branch paths to execute.
        // If there are some then execute the next one:
        if (!mBranchPathsToExec.empty()) {
            BranchPath branchPath = mBranchPathsToExec.back();
            mBranchPathsToExec.pop_back();
            evalBranchPath(branchPath);
        } else {
            // No branch paths to execute.
            // Check to see if there are any unevaluated instructions and if there are none then we are done:
            if (!checkForUnevaluatedInstructions())
                break;

            // We found unevaluated instructions.
            // A branch path SHOULD have been created at this point:
            assert(!mBranchPathsToExec.empty());
        }
    }
}

void ConstInstructionEvaluator::getInRegStateForInstruction(const uint32_t instAddr, ConstEvalRegState& regState) const noexcept {
    const uint32_t instructionIdx = getInstructionIdx(instAddr);

    if (instructionIdx < mInstructions.size()) {
        regState = mInstructions[instructionIdx].regIn;
    } else {
        regState.clear();
    }
}

void ConstInstructionEvaluator::getOutRegStateForInstruction(const uint32_t instAddr, ConstEvalRegState& regState) const noexcept {
    const uint32_t instructionIdx = getInstructionIdx(instAddr);

    if (instructionIdx < mInstructions.size()) {
        regState = mInstructions[instructionIdx].regOut;
    } else {
        regState.clear();
    }
}

void ConstInstructionEvaluator::initEvaluator(const ExeFile& exe, const ProgElem& progElem, const ConstEvalRegState& inputRegState) noexcept {
    // Sanity checks
    assert(progElem.type == ProgElemType::FUNCTION);
    assert(progElem.startAddr >= exe.baseAddress && progElem.endAddr <= exe.baseAddress + exe.sizeInWords * sizeof(uint32_t));
    assert(progElem.startAddr % 4 == 0);
    assert(progElem.endAddr % 4 == 0);

    // Clear out previous state
    mInstructions.clear();
    mBranchPathsToExec.clear();

    // Save start and end address
    mFuncStartAddr = progElem.startAddr;
    mFuncEndAddr = progElem.endAddr;

    // Reserve enough memory for the words
    const uint32_t startExeWord = (progElem.startAddr - exe.baseAddress) / sizeof(uint32_t);
    const uint32_t endExeWord = (progElem.endAddr - exe.baseAddress) / sizeof(uint32_t);
    const uint32_t numFuncWords = endExeWord - startExeWord;
    mInstructions.reserve(numFuncWords);

    // Decode each instruction in the function and initialize them
    for (uint32_t wordIdx = startExeWord; wordIdx < endExeWord; ++wordIdx) {
        const uint32_t word = exe.words[wordIdx].value;

        FuncInstruction& instruction = mInstructions[wordIdx];
        instruction.instruction.decode(word);
        instruction.regIn.clear();
        instruction.regOut.clear();
        instruction.execCount = 0;
    }
    
    // We start off by evaluating the first instruction in the function with the given input registers
    BranchPath& branchPath = mBranchPathsToExec.emplace_back();
    branchPath.instructionIdx = 0;
    branchPath.regStates = inputRegState;
}

bool ConstInstructionEvaluator::checkForUnevaluatedInstructions() noexcept {
    const uint32_t numInstructions = (uint32_t) mInstructions.size();

    for (uint32_t i = 0; i < numInstructions; ++i) {
        FuncInstruction& instruction = mInstructions[i];

        if (instruction.execCount <= 0) {
            // Unevaluated instruction.
            // Create a branch path for it to be executed with unknown input registers:
            BranchPath& branchPath = mBranchPathsToExec.emplace_back();
            branchPath.instructionIdx = i;
            branchPath.regStates.clear();

            // There were unevaluated instructions
            return true;
        }
    }

    return false;
}

uint32_t ConstInstructionEvaluator::getInstructionIdx(const uint32_t instructionAddr) const noexcept {
    const bool bIsValidAddr = (
        (instructionAddr % 4 == 0) &&
        (instructionAddr >= mFuncStartAddr) &&
        (instructionAddr < mFuncEndAddr)
    );

    if (bIsValidAddr) {
        return (instructionAddr - mFuncStartAddr) / sizeof(uint32_t);
    } else {
        return UINT32_MAX;
    }
}

void ConstInstructionEvaluator::evalBranchPath(const BranchPath& branchPath) noexcept {    
    // Ensure the instruction is in range, if not just ignore
    const uint32_t numInstructions = (uint32_t) mInstructions.size();
    uint32_t instructionIdx = branchPath.instructionIdx;

    if (instructionIdx >= numInstructions)
        return;
    
    // Our inital input for all instructions will be the register state stored in the branch path
    ConstEvalRegState inputRegState = branchPath.regStates;

    // Continue executing until we have to stop
    while (instructionIdx < numInstructions) {
        // Firstly evaluate this instruction
        FuncInstruction& thisInst = mInstructions[instructionIdx];

        if (!evalInstruction(thisInst, inputRegState))
            break;
        
        // If the instruction is a branch or jump then the next instruction gets executed also.
        // Determine what the next input state will be based on whether there is a branch or not:
        const CpuOpcode thisInstOpcode = thisInst.instruction.opcode;
        const bool bIsBranchOrJump = CpuOpcodeUtils::isBranchOrJumpOpcode(thisInstOpcode);

        if (bIsBranchOrJump) {
            if (instructionIdx + 1 < numInstructions) {
                FuncInstruction& nextInst = mInstructions[instructionIdx + 1];

                if (!evalInstruction(nextInst, inputRegState))
                    break;

                inputRegState = nextInst.regOut;
            } else {
                // There should always be an instruction following a branch/jump instruction, due to the branch delay slot.
                // If there isn't for some insane reason then I don't know what to do? Just quit:
                break;
            }
        } else {
            inputRegState = thisInst.regOut;
        }

        // Determine where to go next based on whether the instruction is a branch or not
        if (!bIsBranchOrJump) {
            // Most frequent case: a regular (non branch or jump) instruction was executed.
            // Just move onto the next instruction:
            ++instructionIdx;
        } else {
            // Dealing with a branch or jump, see first if a branch or jump
            const uint32_t thisInstAddr = mFuncStartAddr + instructionIdx * sizeof(uint32_t);

            if (CpuOpcodeUtils::isBranchOpcode(thisInstOpcode)) {
                // Branch: always goes to a fixed location.
                // Figure out which instruction we would go to if the branch is taken and if not:
                const uint32_t branchTgtAddr = thisInst.instruction.getBranchInstTargetAddr(thisInstAddr);
                const uint32_t instIdxAfterBranch = instructionIdx + 2;
                const uint32_t branchTgtInstIdx = (branchTgtAddr - mFuncStartAddr) / sizeof(uint32_t);

                // Make two branch paths for both sides of the branch
                {
                    BranchPath& newBranchPath = mBranchPathsToExec.emplace_back();
                    newBranchPath.instructionIdx = instIdxAfterBranch;
                    newBranchPath.regStates = inputRegState;
                }

                {
                    BranchPath& newBranchPath = mBranchPathsToExec.emplace_back();
                    newBranchPath.instructionIdx = branchTgtInstIdx;
                    newBranchPath.regStates = inputRegState;
                }
            } else {
                // Jump instruction: handle differently depending on jump type
                switch (thisInstOpcode) {
                    // Regular jump: just make a branch path to the new location
                    case CpuOpcode::J: {
                        const uint32_t branchTgtAddr = thisInst.instruction.getBranchInstTargetAddr(thisInstAddr);
                        const uint32_t branchTgtInstIdx = (branchTgtAddr - mFuncStartAddr) / sizeof(uint32_t);

                        BranchPath& newBranchPath = mBranchPathsToExec.emplace_back();
                        newBranchPath.instructionIdx = branchTgtInstIdx;
                        newBranchPath.regStates = inputRegState;
                    }   break;

                    // Jump and link and jump and link register - a function call basically:
                    // Make a new branch path starting at the instruction AFTER the jump's branch delay slot.
                    // Clear any temporaries that might be overriden by the subroutine also:
                    case CpuOpcode::JAL:
                    case CpuOpcode::JALR: {
                        inputRegState.clearFuncCallTransientRegisters();

                        BranchPath& newBranchPath = mBranchPathsToExec.emplace_back();
                        newBranchPath.instructionIdx = instructionIdx + 2;
                        newBranchPath.regStates = inputRegState;
                    }   break;

                    // For jump register this might either be a switch statement or the actual return instruction for the function.
                    // Since we have no idea where the jump might go to, just reset the entire register state and continue executing
                    // instructions after it's branch delay slot:
                    case CpuOpcode::JR: {
                        inputRegState.clear();

                        BranchPath& newBranchPath = mBranchPathsToExec.emplace_back();
                        newBranchPath.instructionIdx = instructionIdx + 2;
                        newBranchPath.regStates = inputRegState;
                    }   break;

                    default:
                        FATAL_ERROR("Unhandled jump instruction type!");
                        break;
                }
            }

            // A branch or jump instruction always finishes executing the current branch path
            break;
        }
    }
}

bool ConstInstructionEvaluator::evalInstruction(FuncInstruction& inst, const ConstEvalRegState& inRegState) noexcept {
    // Have we executed the instruction too much already? If so then stop:
    if (inst.execCount >= MAX_INSTRUCTION_EVALUATIONS)
        return false;   

    // Set the input register state for the instruction.
    // If it's the first time executing we just copy the current input state, otherwise we merge and set any
    // conflicting registers to 'unknown' constant values:
    if (inst.execCount <= 0) {
        inst.regIn = inRegState;
    } else {
        inst.regIn.mergeWith(inRegState);
    }

    // The initial output register state for the instruction is just the input register state
    inst.regOut = inst.regIn;

    // If the instruction is invalid then stop now and mark all registers as unknown and the instruction as 'executed'
    if (CpuOpcodeUtils::isIllegalOpcode(inst.instruction.opcode)) {
        inst.regOut.clear();
        ++inst.execCount;
        return false;
    }

    // Actually evaluate the instruction
    evalInstImpl(inst.instruction, inst.regIn, inst.regOut);

    // This instruction has now been executed
    ++inst.execCount;
    return true;
}
