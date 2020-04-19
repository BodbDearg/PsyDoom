# Header only library: just need to define an include path
add_library(${MAGIC_ENUM_TGT_NAME} INTERFACE)
target_include_directories(${MAGIC_ENUM_TGT_NAME} INTERFACE "magic_enum/include")
