# Sources
set(SOURCE_FILES
    "Module.cpp"
    "Module.h"
    "WmdFileTypes.cpp"
    "WmdFileTypes.h"
)

set(OTHER_FILES
)

# Build the library and setup IDE folders
add_library(${AUDIO_TOOLS_COMMON_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

# Setup target compile options and exported include dirs
add_common_target_compile_options(${AUDIO_TOOLS_COMMON_TGT_NAME})
target_include_directories(${AUDIO_TOOLS_COMMON_TGT_NAME} PUBLIC INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
