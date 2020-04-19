# Header only library: just defines an include path basically
add_library(${RAPID_JSON_TGT_NAME} INTERFACE)
target_include_directories(${RAPID_JSON_TGT_NAME} INTERFACE "rapidjson/include")
