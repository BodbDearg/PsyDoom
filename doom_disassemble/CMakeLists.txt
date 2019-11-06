# Platform agnostic sources
set(SOURCE_FILES
    "Main.cpp"
    "ProgElems.h"
    "ProgElems_Doom.cpp"
)

set(OTHER_FILES
)

# Build the executable and setup IDE folders
add_executable(${DOOM_DISASM_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

# Setup target dependencies and compile options
target_link_libraries(${DOOM_DISASM_TGT_NAME} ${REVERSING_COMMON_TGT_NAME})
add_common_target_compile_options(${DOOM_DISASM_TGT_NAME})
