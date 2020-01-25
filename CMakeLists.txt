cmake_minimum_required(VERSION 3.14)
project(PsyDoom)

# Current game version string
set(GAME_VERSION_STR "0.0.2")

# Where CMake will search for .cmake module files
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Causes CMake projects to go into their own folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include common build scripts and do basic build setup
include(HelperFunctions)
include(BuildSetup)

build_setup()

# Global identifiers for each project/target
set(AVOCADO_TGT_NAME            Avocado)
set(CEREAL_TGT_NAME             Cereal)
set(DOOM_DISASM_TGT_NAME        DoomDisassemble)
set(EVENT_BUS_TGT_NAME          EventBus)
set(FMT_TGT_NAME                Fmt)
set(GAME_TGT_NAME               PsyDoom)
set(GLM_TGT_NAME                GLM)
set(LIBSDL_TGT_NAME             SDL)
set(MAGIC_ENUM_TGT_NAME         MagicEnum)
set(NLOHMANN_JSON_TGT_NAME      NlohmannJson)
set(PSXEXE_SIGMATCH_TGT_NAME    PSXExeSigMatcher)
set(PSXOBJ_SIGGEN_TGT_NAME      PSXObjSigGen)
set(REVERSING_COMMON_TGT_NAME   ReversingCommon)

# Adding individual projects and libraries
add_subdirectory("${PROJECT_SOURCE_DIR}/doom_disassemble")
add_subdirectory("${PROJECT_SOURCE_DIR}/game")
add_subdirectory("${PROJECT_SOURCE_DIR}/psxexe_sigmatcher")
add_subdirectory("${PROJECT_SOURCE_DIR}/psxobj_siggen")
add_subdirectory("${PROJECT_SOURCE_DIR}/reversing_common")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/avocado")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/cereal")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/event_bus")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/fmt")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/glm")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/libsdl")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/magic_enum")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/nlohmann-json")
