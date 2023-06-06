set(SOURCE_FILES
    "Main.cpp"
    "ProgElems.h"
    "ProgElems_Doom.cpp"
    "ProgElems_DestructionDerby.cpp"
)

set(OTHER_FILES
)

add_executable(${DOOM_DISASM_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_psydoom_common_target_compile_options(${DOOM_DISASM_TGT_NAME})
target_link_libraries(${DOOM_DISASM_TGT_NAME} ${REVERSING_COMMON_TGT_NAME})
