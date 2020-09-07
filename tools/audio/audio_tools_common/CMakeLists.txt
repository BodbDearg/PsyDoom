set(SOURCE_FILES
    "Module.cpp"
    "Module.h"
    "ModuleFileUtils.cpp"
    "ModuleFileUtils.h"
    "WmdFileTypes.cpp"
    "WmdFileTypes.h"
)

set(OTHER_FILES
)

set(INCLUDE_PATHS
    "."
)

add_library(${AUDIO_TOOLS_COMMON_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

add_common_target_compile_options(${AUDIO_TOOLS_COMMON_TGT_NAME})

target_link_libraries(${AUDIO_TOOLS_COMMON_TGT_NAME}
	${BASELIB_TGT_NAME}
	${RAPID_JSON_TGT_NAME}
)

target_include_directories(${AUDIO_TOOLS_COMMON_TGT_NAME} PUBLIC INTERFACE ${INCLUDE_PATHS})
