# This setting controls whether the SPU uses floating point mixing or not
set(PSYDOOM_FLOAT_SPU TRUE CACHE BOOL
"If TRUE then PsyDoom's PlayStation SPU uses floating point mixing of audio.
This can improve audio quality slightly and help prevent clipping artifacts."
)

set(SOURCE_FILES
    "Spu.h"
    "Spu.cpp"
)

set(OTHER_FILES
)

set(INCLUDE_PATHS
    "."
)

add_library(${SIMPLE_SPU_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_psydoom_common_target_compile_options(${SIMPLE_SPU_TGT_NAME})
target_link_libraries(${SIMPLE_SPU_TGT_NAME} ${BASELIB_TGT_NAME})
target_include_directories(${SIMPLE_SPU_TGT_NAME} PUBLIC INTERFACE ${INCLUDE_PATHS})

if (PSYDOOM_FLOAT_SPU)
    target_compile_definitions(${SIMPLE_SPU_TGT_NAME} PUBLIC -DSIMPLE_SPU_FLOAT_SPU=1)
else()
    target_compile_definitions(${SIMPLE_SPU_TGT_NAME} PUBLIC -DSIMPLE_SPU_FLOAT_SPU=0)
endif()
