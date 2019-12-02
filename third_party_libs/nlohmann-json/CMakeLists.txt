set(BASE_DIR "json")
set(INC_DIR "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INC_DIR}
)

set(SOURCE_FILES
    "Dummy.cpp"
    "${INC_DIR}/nlohmann/adl_serializer.hpp"
    "${INC_DIR}/nlohmann/detail/conversions/from_json.hpp"
    "${INC_DIR}/nlohmann/detail/conversions/to_chars.hpp"
    "${INC_DIR}/nlohmann/detail/conversions/to_json.hpp"
    "${INC_DIR}/nlohmann/detail/exceptions.hpp"
    "${INC_DIR}/nlohmann/detail/input/binary_reader.hpp"
    "${INC_DIR}/nlohmann/detail/input/input_adapters.hpp"
    "${INC_DIR}/nlohmann/detail/input/json_sax.hpp"
    "${INC_DIR}/nlohmann/detail/input/lexer.hpp"
    "${INC_DIR}/nlohmann/detail/input/parser.hpp"
    "${INC_DIR}/nlohmann/detail/input/position_t.hpp"
    "${INC_DIR}/nlohmann/detail/iterators/internal_iterator.hpp"
    "${INC_DIR}/nlohmann/detail/iterators/iteration_proxy.hpp"
    "${INC_DIR}/nlohmann/detail/iterators/iterator_traits.hpp"
    "${INC_DIR}/nlohmann/detail/iterators/iter_impl.hpp"
    "${INC_DIR}/nlohmann/detail/iterators/json_reverse_iterator.hpp"
    "${INC_DIR}/nlohmann/detail/iterators/primitive_iterator.hpp"
    "${INC_DIR}/nlohmann/detail/json_pointer.hpp"
    "${INC_DIR}/nlohmann/detail/json_ref.hpp"
    "${INC_DIR}/nlohmann/detail/macro_scope.hpp"
    "${INC_DIR}/nlohmann/detail/macro_unscope.hpp"
    "${INC_DIR}/nlohmann/detail/meta/cpp_future.hpp"
    "${INC_DIR}/nlohmann/detail/meta/detected.hpp"
    "${INC_DIR}/nlohmann/detail/meta/is_sax.hpp"
    "${INC_DIR}/nlohmann/detail/meta/type_traits.hpp"
    "${INC_DIR}/nlohmann/detail/meta/void_t.hpp"
    "${INC_DIR}/nlohmann/detail/output/binary_writer.hpp"
    "${INC_DIR}/nlohmann/detail/output/output_adapters.hpp"
    "${INC_DIR}/nlohmann/detail/output/serializer.hpp"
    "${INC_DIR}/nlohmann/detail/value_t.hpp"
    "${INC_DIR}/nlohmann/json.hpp"
    "${INC_DIR}/nlohmann/json_fwd.hpp"
    "${INC_DIR}/nlohmann/thirdparty/hedley/hedley.hpp"
    "${INC_DIR}/nlohmann/thirdparty/hedley/hedley_undef.hpp"
)

set(OTHER_FILES
)

add_library(${NLOHMANN_JSON_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")
target_include_directories(${NLOHMANN_JSON_TGT_NAME} PUBLIC ${INCLUDE_PATHS})
