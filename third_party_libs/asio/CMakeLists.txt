set(BASE_DIR "asio")
set(SRC_DIR "${BASE_DIR}/src")
set(INCLUDE_DIR "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INCLUDE_DIR}
)

set(SOURCE_FILES
    "${SRC_DIR}/asio.cpp"
)

set(OTHER_FILES
)

add_library(${ASIO_TGT_NAME} ${SOURCE_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${ASIO_TGT_NAME})
target_compile_definitions(${ASIO_TGT_NAME} PUBLIC -DASIO_SEPARATE_COMPILATION)
target_include_directories(${ASIO_TGT_NAME} PUBLIC ${INCLUDE_PATHS})

if (PLATFORM_WINDOWS)
    target_compile_definitions(${ASIO_TGT_NAME} PRIVATE -D_WIN32_WINNT=0x0603)  # Target Windows 8.1 minimum
endif()
