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

target_link_libraries(${SIMPLE_SPU_TGT_NAME}
	${BASELIB_TGT_NAME}
)

target_include_directories(${SIMPLE_SPU_TGT_NAME} PUBLIC INTERFACE ${INCLUDE_PATHS})
