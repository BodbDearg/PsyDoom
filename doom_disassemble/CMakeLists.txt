# Platform agnostic sources
set(SOURCE_FILES
    "CpuInstruction.cpp"
    "CpuInstruction.h"
    "CpuOpcode.h"
    "ExeFile.cpp"
    "ExeFile.h"
    "ExeWord.h"
    "FatalErrors.cpp"
    "FatalErrors.h"
    "Main.cpp"
)

set(OTHER_FILES
)

# Platform and compiler specific defines
if (COMPILER_MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # MSVC: Don't complain about using regular 'std::fopen()' etc.
endif()

# Build the executable and setup IDE folders
add_executable(${DOOM_DISASM_TGT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${HEADER_FILES}" "${OTHER_FILES}")

# Other platform or compiler specific settings, flags or switches
if (COMPILER_MSVC)    
    target_compile_options(${DOOM_DISASM_TGT_NAME} PRIVATE /W4)                 # Enable all warnings
elseif(COMPILER_GCC OR COMPILER_CLANG)
    target_compile_options(${DOOM_DISASM_TGT_NAME} PRIVATE -pedantic -Wall)     # Enable all warnings and non standard C++ warnings
endif()
