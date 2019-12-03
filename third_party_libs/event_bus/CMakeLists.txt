set(BASE_DIR "EventBus/lib")
set(SRC_DIR "${BASE_DIR}/src")
set(INC_DIR "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INC_DIR}
)

set(SOURCE_FILES
    "${INC_DIR}/eventbus/AsyncEventBus.h"
    "${INC_DIR}/eventbus/EventBus.h"
    "${INC_DIR}/eventbus/EventCollector.h"
    "${INC_DIR}/eventbus/internal/AsyncCallbackVector.h"
    "${INC_DIR}/eventbus/internal/CallbackVector.h"
    "${INC_DIR}/eventbus/internal/common.h"
    "${INC_DIR}/eventbus/internal/TransactionCallbackVector.h"
    "${INC_DIR}/eventbus/TokenHolder.h"
    "${SRC_DIR}/eventbus/AsyncEventBus.cpp"
)

set(OTHER_FILES
)

add_library(${EVENT_BUS_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")
target_include_directories(${EVENT_BUS_TGT_NAME} PUBLIC ${INCLUDE_PATHS})

if (COMPILER_MSVC)
    # Fixes the 'not' keyword not being recognized
    target_compile_options(${EVENT_BUS_TGT_NAME} PRIVATE /permissive-)
endif()
