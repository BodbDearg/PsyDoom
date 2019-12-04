#pragma once

#include <cstdint>
#include <ostream>

//----------------------------------------------------------------------------------------------------------------------
// Module that generates comments on a given instruction.
// Used to annotate the results of constant operations that are known from looking at the disassembly.
//----------------------------------------------------------------------------------------------------------------------
class ConstInstructionEvaluator;
struct CpuInstruction;
struct ExeFile;

namespace InstructionCommenter {
    // Typedef for a function that adds prefixes for instruction comments. (i.e the start of comment character, leading spaces etc.)
    // Takes the current column in the line as a parameter and the output stream to print to.
    typedef void (*CommentPrefixerFunc)(const uint32_t lineCol, std::ostream& out);

    // Attempts to comment the given instruction at the given address.
    // Uses the results of the given constant instruction evaluator to make the comments.
    // Uses the given line column info and the comment prefixer to start the comment.
    void tryCommentInstruction(
        const CpuInstruction& inst,
        const uint32_t instAddr,
        const ExeFile& exe,
        const ConstInstructionEvaluator& constInstEvaluator,        
        const CommentPrefixerFunc pCommentPrefixer,
        const uint32_t lineCol,
        std::ostream& out
    );
}
