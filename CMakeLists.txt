cmake_minimum_required(VERSION 3.14)
project(PC_PSX_Doom)

# Where CMake will search for .cmake module files
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Causes CMake projects to go into their own folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include common build scripts and do basic build setup
include(HelperFunctions)
include(BuildSetup)

build_setup()

# Global identifiers for each project/target
set(GAME_NAME       PC_PSX_Doom)
set(LIBSDL_NAME     SDL)
set(MEDNAFEN_NAME   Mednafen)

# Adding individual projects and libraries
set(GAME_SRC_DIR            "${PROJECT_SOURCE_DIR}/game")
set(THIRD_PARTY_LIBS_DIR    "${PROJECT_SOURCE_DIR}/third_party_libs")
set(LIBSDL_DIR              "${THIRD_PARTY_LIBS_DIR}/libsdl")
set(MEDNAFEN_DIR            "${THIRD_PARTY_LIBS_DIR}/mednafen")

#add_subdirectory("${LIBSDL_DIR}")
add_subdirectory("${MEDNAFEN_DIR}")
add_subdirectory("${GAME_SRC_DIR}")
