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
set(DOOM_DISASM_TGT_NAME    DoomDisassemble)
set(GAME_TGT_NAME           PC_PSX_Doom)
set(LIBSDL_TGT_NAME         SDL)
set(MEDNAFEN_TGT_NAME       Mednafen)
set(ZLIB_TGT_NAME           ZLib)

# Adding individual projects and libraries
add_subdirectory("${PROJECT_SOURCE_DIR}/doom_disassemble")
add_subdirectory("${PROJECT_SOURCE_DIR}/game")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/libsdl")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/mednafen")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/zlib")
