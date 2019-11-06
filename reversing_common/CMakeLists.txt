# Platform agnostic sources
set(SOURCE_FILES
    "ConstEvalRegState.h"
    "ConstInstructionEvaluator.cpp"
    "ConstInstructionEvaluator.h"
    "CpuGpr.h"
    "CpuInstruction.cpp"
    "CpuInstruction.h"
    "CpuOpcode.h"
    "DisassemblyPrinter.cpp"
    "DisassemblyPrinter.h"
    "ExeFile.cpp"
    "ExeFile.h"
    "ExeWord.h"
    "FatalErrors.cpp"
    "FatalErrors.h"
    "FileUtils.cpp"
    "FileUtils.h"
    "InstructionCommenter.cpp"
    "InstructionCommenter.h"
    "PrintUtils.cpp"
    "PrintUtils.h"
    "ProgElem.cpp"
    "ProgElem.h"
    "TextIStream.cpp"
    "TextIStream.h"
)

set(OTHER_FILES
)

# Build the executable and setup IDE folders
add_library(${REVERSING_COMMON_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

# Setup target compile options and exported include dirs
add_common_target_compile_options(${REVERSING_COMMON_TGT_NAME})
target_include_directories(${REVERSING_COMMON_TGT_NAME} PUBLIC INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
