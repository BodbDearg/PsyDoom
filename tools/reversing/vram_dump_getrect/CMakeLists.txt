set(SOURCE_FILES
    "Main.cpp"
)

set(OTHER_FILES
)

add_executable(${VRAM_DUMP_GETRECT_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_psydoom_common_target_compile_options(${VRAM_DUMP_GETRECT_TGT_NAME})
target_link_libraries(${VRAM_DUMP_GETRECT_TGT_NAME} ${REVERSING_COMMON_TGT_NAME})
