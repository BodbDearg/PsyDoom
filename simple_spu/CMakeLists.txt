set(SOURCE_FILES
    "Spu.h"
    "Spu.cpp"
)

set(OTHER_FILES
)

add_library(${SIMPLE_SPU_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")
