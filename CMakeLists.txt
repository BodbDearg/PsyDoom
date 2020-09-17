cmake_minimum_required(VERSION 3.15)
project(PsyDoom)

# Current game version string
set(GAME_VERSION_STR "0.5.2")

# Where CMake will search for .cmake module files
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Causes CMake projects to go into their own folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include common build scripts and do basic build setup
include(HelperFunctions)
include(BuildSetup)

build_setup()

# Global identifiers for each project/target
set(ASIO_TGT_NAME                   Asio)
set(AUDIO_TOOLS_COMMON_TGT_NAME     AudioToolsCommon)
set(AVOCADO_TGT_NAME                Avocado)
set(BASELIB_TGT_NAME                BaseLib)
set(DOOM_DISASM_TGT_NAME            DoomDisassemble)
set(EVENT_BUS_TGT_NAME              EventBus)
set(FMT_TGT_NAME                    Fmt)
set(GAME_TGT_NAME                   PsyDoom)
set(JSON_TO_WMD_TGT_NAME            JsonToWmd)
set(LCD_TOOL_TGT_NAME               LcdTool)
set(LIBSDL_TGT_NAME                 SDL)
set(MAGIC_ENUM_TGT_NAME             MagicEnum)
set(PSXEXE_SIGMATCH_TGT_NAME        PSXExeSigMatcher)
set(PSXOBJ_SIGGEN_TGT_NAME          PSXObjSigGen)
set(RAPID_JSON_TGT_NAME             RapidJson)
set(REVERSING_COMMON_TGT_NAME       ReversingCommon)
set(SIMPLE_SPU_TGT_NAME             SimpleSpu)
set(WMD_TO_JSON_TGT_NAME            WmdToJson)

# Settings to include or exclude optional portions of the project tree
set(PSYDOOM_INCLUDE_REVERSING_TOOLS FALSE CACHE BOOL 
"If TRUE include reverse engineering tools in the project tree.
These were tools which were used during the earlier stages of development."
)

set(PSYDOOM_INCLUDE_AUDIO_TOOLS FALSE CACHE BOOL
"If TRUE include PlayStation Doom audio related tools in the project tree."
)

# Adding individual projects and libraries
add_subdirectory("${PROJECT_SOURCE_DIR}/baselib")
add_subdirectory("${PROJECT_SOURCE_DIR}/game")
add_subdirectory("${PROJECT_SOURCE_DIR}/simple_spu")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/asio")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/avocado")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/event_bus")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/fmt")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/libsdl")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/magic_enum")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/rapidjson")

if (PSYDOOM_INCLUDE_AUDIO_TOOLS)
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/audio/audio_tools_common")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/audio/json_to_wmd")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/audio/lcd_tool")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/audio/wmd_to_json")
endif()

if (PSYDOOM_INCLUDE_REVERSING_TOOLS)
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/reversing/doom_disassemble")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/reversing/psxexe_sigmatcher")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/reversing/psxobj_siggen")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/reversing/reversing_common")
endif()
