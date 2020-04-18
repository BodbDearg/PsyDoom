set(BASE_DIR "rapidjson")
set(INC_DIR "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INC_DIR}
)

set(SOURCE_FILES
    "${INC_DIR}/rapidjson/allocators.h"
    "${INC_DIR}/rapidjson/cursorstreamwrapper.h"
    "${INC_DIR}/rapidjson/document.h"
    "${INC_DIR}/rapidjson/encodedstream.h"
    "${INC_DIR}/rapidjson/encodings.h"
    "${INC_DIR}/rapidjson/error/en.h"
    "${INC_DIR}/rapidjson/error/error.h"
    "${INC_DIR}/rapidjson/filereadstream.h"
    "${INC_DIR}/rapidjson/filewritestream.h"
    "${INC_DIR}/rapidjson/fwd.h"
    "${INC_DIR}/rapidjson/internal/biginteger.h"
    "${INC_DIR}/rapidjson/internal/clzll.h"
    "${INC_DIR}/rapidjson/internal/diyfp.h"
    "${INC_DIR}/rapidjson/internal/dtoa.h"
    "${INC_DIR}/rapidjson/internal/ieee754.h"
    "${INC_DIR}/rapidjson/internal/itoa.h"
    "${INC_DIR}/rapidjson/internal/meta.h"
    "${INC_DIR}/rapidjson/internal/pow10.h"
    "${INC_DIR}/rapidjson/internal/regex.h"
    "${INC_DIR}/rapidjson/internal/stack.h"
    "${INC_DIR}/rapidjson/internal/strfunc.h"
    "${INC_DIR}/rapidjson/internal/strtod.h"
    "${INC_DIR}/rapidjson/internal/swap.h"
    "${INC_DIR}/rapidjson/istreamwrapper.h"
    "${INC_DIR}/rapidjson/memorybuffer.h"
    "${INC_DIR}/rapidjson/memorystream.h"
    "${INC_DIR}/rapidjson/msinttypes/inttypes.h"
    "${INC_DIR}/rapidjson/msinttypes/stdint.h"
    "${INC_DIR}/rapidjson/ostreamwrapper.h"
    "${INC_DIR}/rapidjson/pointer.h"
    "${INC_DIR}/rapidjson/prettywriter.h"
    "${INC_DIR}/rapidjson/rapidjson.h"
    "${INC_DIR}/rapidjson/reader.h"
    "${INC_DIR}/rapidjson/schema.h"
    "${INC_DIR}/rapidjson/stream.h"
    "${INC_DIR}/rapidjson/stringbuffer.h"
    "${INC_DIR}/rapidjson/writer.h"
)

set(OTHER_FILES
)

add_library(${RAPID_JSON_TGT_NAME} INTERFACE)
target_sources(${RAPID_JSON_TGT_NAME} INTERFACE ${SOURCE_FILES})
target_include_directories(${RAPID_JSON_TGT_NAME} INTERFACE ${INCLUDE_PATHS})
