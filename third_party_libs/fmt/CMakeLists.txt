set(BASE_DIR "fmt")
set(SRC_DIR "${BASE_DIR}/src")
set(INC_DIR "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INC_DIR}
)

set(SOURCE_FILES
    "${INC_DIR}/fmt/chrono.h"
    "${INC_DIR}/fmt/color.h"
    "${INC_DIR}/fmt/compile.h"
    "${INC_DIR}/fmt/core.h"
    "${INC_DIR}/fmt/format-inl.h"
    "${INC_DIR}/fmt/format.h"
    "${INC_DIR}/fmt/locale.h"
    "${INC_DIR}/fmt/ostream.h"
    "${INC_DIR}/fmt/posix.h"
    "${INC_DIR}/fmt/printf.h"
    "${INC_DIR}/fmt/ranges.h"
    "${SRC_DIR}/format.cc"
    "${SRC_DIR}/posix.cc"
)

set(OTHER_FILES
)

add_library(${FMT_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")
target_include_directories(${FMT_TGT_NAME} PUBLIC ${INCLUDE_PATHS})
