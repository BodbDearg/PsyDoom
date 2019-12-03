set(BASE_DIR "magic_enum")
set(INC_DIR "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INC_DIR}
)

set(SOURCE_FILES
    "Dummy.cpp"
    "${INC_DIR}/magic_enum.hpp"
)

set(OTHER_FILES
)

add_library(${MAGIC_ENUM_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")
target_include_directories(${MAGIC_ENUM_TGT_NAME} PUBLIC ${INCLUDE_PATHS})
