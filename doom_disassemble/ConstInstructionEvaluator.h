#include "CpuInstruction.h"
#include "ConstEvalRegState.h"
#include <vector>

struct ExeFile;
struct ProgElem;

//----------------------------------------------------------------------------------------------------------------------
// Constant instruction evaluator: allows constant evaluation of instructions within the context of a single function.
//
// The aim of constant evaluation is to discover the known constant value of registers at various points in the code,
// so the assembly code can be automatically commented accordingly. This allows loads/stores from/to various known
// globals etc. to be commented and annotated, and makes reading disassembly much easier.
//----------------------------------------------------------------------------------------------------------------------
class ConstInstructionEvaluator {
public:
    ConstInstructionEvaluator() noexcept;
    ~ConstInstructionEvaluator() noexcept;

    // Do constant instruction evaluation for the given function and save the results in the evaluator.
    // The results of previous evaluations are wiped.
    void constEvalFunction(const ExeFile& exe, const ProgElem& progElem, const ConstEvalRegState& inputRegState) noexcept;

    // Get the input and output register state for the instruction at the given address.
    // Will return an undefined state if the address is not valid.
    void getInRegStateForInstruction(const uint32_t instAddr, ConstEvalRegState& regState) const noexcept;
    void getOutRegStateForInstruction(const uint32_t instAddr, ConstEvalRegState& regState) const noexcept;

private:
    // Represents an instruction in the function.
    // Stores the instruction itself, and the evaluated input and output register states.
    struct FuncInstruction {
        CpuInstruction      instruction;    // The instruction itself
        ConstEvalRegState   regIn;          // The input register state to the instruction
        ConstEvalRegState   regOut;         // The output register state after the instruction has executed
        uint32_t            execCount;      // Number of times the instruction has been executed by the const instruction evaluator
    };

    // Represents a branch path to be executed.
    // The evaluator evaluates both sides of conditional branches in order to try and figure out constant values.
    // Holds the index of the instruction to execute after the branch as well as the register state going into that instruction.
    struct BranchPath {
        uint32_t            instructionIdx;
        ConstEvalRegState   regStates;
    };

    // Initialize the const evaluator with the given input register state for the given function in the given exe
    void initEvaluator(const ExeFile& exe, const ProgElem& progElem, const ConstEvalRegState& inputRegState) noexcept;

    // Checks for unevaluated instructions.
    // Returns true if there are unevaluated instructions and creates a new branch path to execute those instructions.
    bool checkForUnevaluatedInstructions() noexcept;

    // Get the instruction index for the given address.
    // Returns UINT32_MAX if the given address is not valid and pointing to a valid instruction.
    uint32_t getInstructionIdx(const uint32_t instructionAddr) const noexcept;

    // Evaluate the given branch path starting at the given instruction with the given register state
    void evalBranchPath(const BranchPath& branchPath) noexcept;

    // Evaluate the given single instruction.
    // Returns 'false' if no further instructions should be executed.
    bool evalInstruction(FuncInstruction& inst, const ConstEvalRegState& inRegState) noexcept;

    uint32_t                        mFuncStartAddr;         // Start address of the function being evaluated
    uint32_t                        mFuncEndAddr;           // End address of the function being evaluated
    std::vector<FuncInstruction>    mInstructions;          // Instructions in the function
    std::vector<BranchPath>         mBranchPathsToExec;     // Branch paths that remain to be executed by the evaluator
};
