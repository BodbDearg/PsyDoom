set(SOURCE_FILES
    "Asserts.h"
    "Endian.h"
    "FatalErrors.cpp"
    "FatalErrors.h"
    "FileUtils.cpp"
    "FileUtils.h"
    "IniUtils.cpp"
    "IniUtils.h"
    "Macros.h"
)

set(OTHER_FILES
)

set(INCLUDE_PATHS
    "."
)

add_library(${BASELIB_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")
target_include_directories(${BASELIB_TGT_NAME} PUBLIC ${INCLUDE_PATHS})
