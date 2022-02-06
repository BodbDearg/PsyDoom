macro(detect_compiler)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set(COMPILER_CLANG TRUE)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(COMPILER_GCC TRUE)
    elseif (MSVC)
        set(COMPILER_MSVC TRUE)
    else()
        message(FATAL_ERROR "Unknown or currently unsupported compiler '${CMAKE_CXX_COMPILER_ID}'!")
    endif()
endmacro()

macro(detect_platform)
    if (WIN32)
        set(PLATFORM_WINDOWS TRUE)

        if ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win64")
            set(PLATFORM_WINDOWS_64 TRUE)
        else()
            set(PLATFORM_WINDOWS_32 TRUE)
        endif()
    elseif (APPLE)    
        set(PLATFORM_MAC TRUE)  # Note: could also be iOS etc. but not targetting those - can just assume Mac...
    elseif (UNIX AND NOT APPLE)
        set(PLATFORM_LINUX TRUE)
    else()
        message(FATAL_ERROR "Unknown or currently unsupported platform!")
    endif()
endmacro()

macro(compiler_agnostic_setup)
    set(CMAKE_C_STANDARD    11)
    set(CMAKE_CXX_STANDARD  17)
endmacro()

macro(compiler_specific_setup)
    # Clang, Apple Clang, or GCC - enable debugging
    if (COMPILER_CLANG OR COMPILER_GCC)
        set(CMAKE_C_FLAGS -g)
        set(CMAKE_CXX_FLAGS -g)
    endif()

    # MSVC: statically link against the CRT.
    # Doing this for end user convenience to try and avoid missing CRT dll issues.
    if (COMPILER_MSVC)
        # Use only debug when required, manually enable if need be - release much faster.
        # TODO: make this configurable
        if (true)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()

        # Disabling 'security check' and 'basic runtime checks' for debug.
        # These slow down the debug build too much, especially 'basic runtime checks'.
        # TODO: make these configurable
        add_compile_options(/GS-)
        STRING (REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")

        # Disable a warning coming often from 'winbase.h': "macro expansion producing 'defined' has undefined behavior"
        add_compile_options(/wd5105)
    endif()
endmacro()

macro(build_setup)
    detect_compiler()
    detect_platform()
    compiler_agnostic_setup()
    compiler_specific_setup()
endmacro()
