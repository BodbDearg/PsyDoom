set(SOURCE_FILES
    "FuncSignature.cpp"
    "FuncSignature.h"
    "Main.cpp"
    "ObjFileData.h"
    "ObjFileParser.cpp"
    "ObjFileParser.h"
)

set(OTHER_FILES
)

add_executable(${PSXOBJ_SIGGEN_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${PSXOBJ_SIGGEN_TGT_NAME})
target_link_libraries(${PSXOBJ_SIGGEN_TGT_NAME} ${REVERSING_COMMON_TGT_NAME})
