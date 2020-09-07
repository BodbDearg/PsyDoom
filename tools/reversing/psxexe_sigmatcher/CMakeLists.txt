set(SOURCE_FILES
    "FuncSignature.cpp"
    "FuncSignature.h"
    "Main.cpp"
)

set(OTHER_FILES
)

add_executable(${PSXEXE_SIGMATCH_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${PSXEXE_SIGMATCH_TGT_NAME})
target_link_libraries(${PSXEXE_SIGMATCH_TGT_NAME} ${REVERSING_COMMON_TGT_NAME})
