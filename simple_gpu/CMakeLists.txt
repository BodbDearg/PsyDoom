set(SOURCE_FILES
    "Gpu.h"
    "Gpu.cpp"
)

set(OTHER_FILES
)

set(INCLUDE_PATHS
    "."
)

add_library(${SIMPLE_GPU_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_psydoom_common_target_compile_options(${SIMPLE_GPU_TGT_NAME})
target_link_libraries(${SIMPLE_GPU_TGT_NAME} ${BASELIB_TGT_NAME})
target_include_directories(${SIMPLE_GPU_TGT_NAME} PUBLIC INTERFACE ${INCLUDE_PATHS})
