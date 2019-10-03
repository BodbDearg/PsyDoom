set(SRC_DIR "zlib")

set(EXPORTED_INCLUDE_PATHS
    "${SRC_DIR}"
)

set(SOURCE_FILES
    "${SRC_DIR}/adler32.c"
    "${SRC_DIR}/compress.c"
    "${SRC_DIR}/crc32.c"
    "${SRC_DIR}/crc32.h"
    "${SRC_DIR}/deflate.c"
    "${SRC_DIR}/deflate.h"
    "${SRC_DIR}/gzclose.c"
    "${SRC_DIR}/gzguts.h"
    "${SRC_DIR}/gzlib.c"
    "${SRC_DIR}/gzread.c"
    "${SRC_DIR}/gzwrite.c"
    "${SRC_DIR}/infback.c"
    "${SRC_DIR}/inffast.c"
    "${SRC_DIR}/inffast.h"
    "${SRC_DIR}/inffixed.h"
    "${SRC_DIR}/inflate.c"
    "${SRC_DIR}/inflate.h"
    "${SRC_DIR}/inftrees.c"
    "${SRC_DIR}/inftrees.h"
    "${SRC_DIR}/trees.c"
    "${SRC_DIR}/trees.h"
    "${SRC_DIR}/uncompr.c"
    "${SRC_DIR}/zconf.h"
    "${SRC_DIR}/zlib.h"
    "${SRC_DIR}/zutil.c"
    "${SRC_DIR}/zutil.h"
)

set(OTHER_FILES
)

# Platform and compiler specific defines
if (COMPILER_MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # MSVC: Don't complain about using regular 'std::fopen()' etc.
endif()

# Generate the library
add_library(${ZLIB_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

# Setup include dirs
target_include_directories(${ZLIB_TGT_NAME} INTERFACE ${EXPORTED_INCLUDE_PATHS})

# Platform and compiler specific settings
if (COMPILER_MSVC)
    target_compile_options(${ZLIB_TGT_NAME} PRIVATE /W2)    # More limited set of warnings
endif()
