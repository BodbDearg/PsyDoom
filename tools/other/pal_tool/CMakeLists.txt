set(SOURCE_FILES
    "PalTool.cpp"
)

set(OTHER_FILES
)

add_executable(${PAL_TOOL_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${PAL_TOOL_TGT_NAME})
target_link_libraries(${PAL_TOOL_TGT_NAME} ${BASELIB_TGT_NAME})
