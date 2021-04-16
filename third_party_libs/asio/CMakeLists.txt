set(BASE_DIR "asio")
set(SRC_DIR "${BASE_DIR}/src")
set(INCLUDE_DIR "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INCLUDE_DIR}
)

set(SOURCE_FILES
    "${SRC_DIR}/asio.cpp"
)

add_library(${ASIO_TGT_NAME} ${SOURCE_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

target_compile_definitions(${ASIO_TGT_NAME} PUBLIC -DASIO_SEPARATE_COMPILATION)
target_include_directories(${ASIO_TGT_NAME} PUBLIC ${INCLUDE_PATHS})
