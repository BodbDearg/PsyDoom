set(SOURCE_FILES
    "VagTool.cpp"
)

set(OTHER_FILES
)

add_executable(${VAG_TOOL_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${VAG_TOOL_TGT_NAME})
target_link_libraries(${VAG_TOOL_TGT_NAME} ${AUDIO_TOOLS_COMMON_TGT_NAME})

