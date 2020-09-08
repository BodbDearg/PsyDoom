set(SOURCE_FILES
    "LcdExtract.cpp"
)

set(OTHER_FILES
)

add_executable(${LCD_EXTRACT_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${LCD_EXTRACT_TGT_NAME})
target_link_libraries(${LCD_EXTRACT_TGT_NAME} ${AUDIO_TOOLS_COMMON_TGT_NAME})
add_common_target_compile_options(${LCD_EXTRACT_TGT_NAME})
