set(SOURCE_FILES
    "Asserts.h"
    "ByteInputStream.h"
    "ByteVecOutputStream.h"
    "Endian.h"
    "FatalErrors.cpp"
    "FatalErrors.h"
    "FileInputStream.h"
    "FileOutputStream.h"
    "FileUtils.cpp"
    "FileUtils.h"
    "IniUtils.cpp"
    "IniUtils.h"
    "InputStream.h"
    "JsonUtils.h"
    "Macros.h"
    "OutputStream.h"
)

set(OTHER_FILES
)

set(INCLUDE_PATHS
    "."
)

add_library(${BASELIB_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

target_link_libraries(${BASELIB_TGT_NAME}
    ${RAPID_JSON_TGT_NAME}
)

target_include_directories(${BASELIB_TGT_NAME} PUBLIC ${INCLUDE_PATHS})
