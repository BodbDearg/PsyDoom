cmake_minimum_required(VERSION 3.15)
project(PsyDoom)

# Current game version string
set(GAME_VERSION_STR "0.7.2")

# Where CMake will search for .cmake module files
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# MacOS: target ARM and Intel 64-bit
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# Causes CMake projects to go into their own folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include common build scripts and do basic build setup
include(HelperFunctions)
include(BuildSetup)

build_setup()

# Global identifiers for each project/target
set(ASIO_TGT_NAME                   Asio)
set(AUDIO_TOOLS_COMMON_TGT_NAME     AudioToolsCommon)
set(BASELIB_TGT_NAME                BaseLib)
set(DOOM_DISASM_TGT_NAME            DoomDisassemble)
set(GAME_TGT_NAME                   PsyDoom)
set(LCD_TOOL_TGT_NAME               LcdTool)
set(LIBSDL_TGT_NAME                 SDL)
set(PSXEXE_SIGMATCH_TGT_NAME        PSXExeSigMatcher)
set(PSXOBJ_SIGGEN_TGT_NAME          PSXObjSigGen)
set(RAPID_JSON_TGT_NAME             RapidJson)
set(REVERSING_COMMON_TGT_NAME       ReversingCommon)
set(SIMPLE_GPU_TGT_NAME             SimpleGpu)
set(SIMPLE_SPU_TGT_NAME             SimpleSpu)
set(VAG_TOOL_TGT_NAME               VagTool)
set(VULKAN_GL_TGT_NAME              VulkanGL)
set(WMD_TOOL_TGT_NAME               WmdTool)

# Compile in support for the Vulkan renderer?
set(PSYDOOM_INCLUDE_VULKAN_RENDERER TRUE CACHE BOOL
"If TRUE (default) then compile in support for the new Vulkan renderer.
Disabling this setting may make it easier to port to platforms which don't support Vulkan.")

# Enable certain limit removing features?
set(PSYDOOM_LIMIT_REMOVING TRUE CACHE BOOL
"If TRUE (default) then various limit removing or increasing features will be compiled into the engine.
This also includes extensions to the classic renderer to support high walls, and fix bugs where you can
see the geometry of adjacent rooms through sky ceilings.")

# Settings to include or exclude optional portions of the project tree
set(PSYDOOM_INCLUDE_GAME TRUE CACHE BOOL 
"If TRUE include the game itself in the project tree.
Optionally, you could remove the game if you just wanted to compile the audio tools for example.")

set(PSYDOOM_INCLUDE_REVERSING_TOOLS FALSE CACHE BOOL 
"If TRUE include reverse engineering tools in the project tree.
These were tools which were used during the earlier stages of development.")

set(PSYDOOM_INCLUDE_AUDIO_TOOLS FALSE CACHE BOOL
"If TRUE include PlayStation Doom audio related tools in the project tree.")

# Adding individual projects and libraries
add_subdirectory("${PROJECT_SOURCE_DIR}/baselib")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/rapidjson")

if (PSYDOOM_INCLUDE_GAME)
    add_subdirectory("${PROJECT_SOURCE_DIR}/game")
    add_subdirectory("${PROJECT_SOURCE_DIR}/simple_gpu")
    add_subdirectory("${PROJECT_SOURCE_DIR}/simple_spu")
    add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/asio")
    add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/libsdl")

    if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
        add_subdirectory("${PROJECT_SOURCE_DIR}/vulkan_gl")
    endif()
endif()

if (PSYDOOM_INCLUDE_AUDIO_TOOLS)
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/audio/audio_tools_common")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/audio/lcd_tool")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/audio/vag_tool")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/audio/wmd_tool")
endif()

if (PSYDOOM_INCLUDE_REVERSING_TOOLS)
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/reversing/doom_disassemble")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/reversing/psxexe_sigmatcher")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/reversing/psxobj_siggen")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools/reversing/reversing_common")
endif()
