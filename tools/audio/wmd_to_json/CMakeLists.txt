set(SOURCE_FILES
    "WmdToJson.cpp"
)

set(OTHER_FILES
)

add_executable(${WMD_TO_JSON_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${WMD_TO_JSON_TGT_NAME})
target_link_libraries(${WMD_TO_JSON_TGT_NAME} ${AUDIO_TOOLS_COMMON_TGT_NAME})
add_common_target_compile_options(${WMD_TO_JSON_TGT_NAME})
