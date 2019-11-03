set(INCLUDE_PATHS
    "."
)

# Platform agnostic sources
set(SOURCE_FILES
)

set(OTHER_FILES
)

# Platform specific sources
if (PLATFORM_WINDOWS)
    list(APPEND SOURCE_FILES "Main_Windows.cpp")
elseif(PLATFORM_MAC)
    list(APPEND SOURCE_FILES "Main_Mac.mm")
else()
    list(APPEND SOURCE_FILES "Main_StandardCpp.cpp")
endif()

# Platform and compiler specific defines
if (COMPILER_MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # MSVC: Don't complain about using regular 'std::fopen()' etc.
endif()

# Build the executable and setup IDE folders
add_executable(${GAME_TGT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${HEADER_FILES}" "${OTHER_FILES}")

# Specify include dirs
include_directories(${INCLUDE_PATHS})

# Other platform or compiler specific settings, flags or switches
if (PLATFORM_WINDOWS)
    set_property(TARGET ${GAME_TGT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()

if (PLATFORM_MAC)
    SET_TARGET_PROPERTIES(${GAME_TGT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
endif()

if (PLATFORM_LINUX)
    target_compile_options(${GAME_TGT_NAME} PRIVATE -pthread)
endif()

if (COMPILER_MSVC)    
    target_compile_options(${GAME_TGT_NAME} PRIVATE /W4)                # Enable all warnings
elseif(COMPILER_GCC OR COMPILER_CLANG)
    target_compile_options(${GAME_TGT_NAME} PRIVATE -pedantic -Wall)    # Enable all warnings and non standard C++ warnings
endif()
