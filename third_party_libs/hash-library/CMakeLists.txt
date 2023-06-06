set(SRC_DIR "hash-library")

set(SOURCE_FILES
    "${SRC_DIR}/md5.cpp"
    "${SRC_DIR}/md5.h"
)

set(OTHER_FILES
)

add_library(${HASH_LIBRARY_TGT_NAME} ${SOURCE_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${HASH_LIBRARY_TGT_NAME})
target_include_directories(${HASH_LIBRARY_TGT_NAME} INTERFACE ${SRC_DIR})
