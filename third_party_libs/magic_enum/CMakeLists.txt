set(BASE_DIR "magic_enum")
set(INC_DIR "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INC_DIR}
)

set(SOURCE_FILES
    "${INC_DIR}/magic_enum.hpp"
)

set(OTHER_FILES
)

add_library(${MAGIC_ENUM_TGT_NAME} INTERFACE)
target_sources(${MAGIC_ENUM_TGT_NAME} INTERFACE ${SOURCE_FILES})
target_include_directories(${MAGIC_ENUM_TGT_NAME} INTERFACE ${INCLUDE_PATHS})
