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
    "JRInstHandler.h"
    "PrintUtils.cpp"
    "PrintUtils.h"
    "ProgElem.cpp"
    "ProgElem.h"
    "PseudoCppPrinter.cpp"
    "PseudoCppPrinter.h"
    "PseudoCppPrinter_InstPrint.cpp"
    "PseudoCppPrinter_InstPrint.h"
    "TextIStream.cpp"
    "TextIStream.h"
)

set(OTHER_FILES
)

set(INCLUDE_PATHS
    "."
)

add_library(${REVERSING_COMMON_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${REVERSING_COMMON_TGT_NAME})
target_include_directories(${REVERSING_COMMON_TGT_NAME} PUBLIC INTERFACE ${INCLUDE_PATHS})
