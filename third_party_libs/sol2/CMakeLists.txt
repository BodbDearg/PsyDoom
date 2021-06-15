# Header only library: just defines an include path basically
add_library(${SOL2_TGT_NAME} INTERFACE)
target_include_directories(${SOL2_TGT_NAME} INTERFACE "sol2/include")
