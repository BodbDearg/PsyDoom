set(BASE_DIR "hash-library")

set(EXPORTED_INCLUDE_PATHS
    ${BASE_DIR}
)

set(SOURCE_FILES    
    "${BASE_DIR}/crc32.cpp"
    "${BASE_DIR}/crc32.h"
    "${BASE_DIR}/hash.h"
    "${BASE_DIR}/hmac.h"
    "${BASE_DIR}/keccak.cpp"
    "${BASE_DIR}/keccak.h"
    "${BASE_DIR}/md5.cpp"
    "${BASE_DIR}/md5.h"
    "${BASE_DIR}/sha1.cpp"
    "${BASE_DIR}/sha1.h"
    "${BASE_DIR}/sha256.cpp"
    "${BASE_DIR}/sha256.h"
    "${BASE_DIR}/sha3.cpp"
    "${BASE_DIR}/sha3.h"
)

set(OTHER_FILES
)

add_library(${HASHLIBRARY_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")
target_include_directories(${HASHLIBRARY_TGT_NAME} INTERFACE ${EXPORTED_INCLUDE_PATHS})
