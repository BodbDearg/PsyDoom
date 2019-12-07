# Platform agnostic sources
set(SOURCE_FILES
    "Doom/d_main.cpp"
    "Doom/d_main.h"
    "Doom/p_ceiling.cpp"
    "Doom/p_ceiling.h"
    "Doom/p_change.cpp"
    "Doom/p_change.h"
    "Doom/p_doors.cpp"
    "Doom/p_doors.h"
    "Doom/p_enemy.cpp"
    "Doom/p_enemy.h"
    "Doom/p_floor.cpp"
    "Doom/p_floor.h"
    "Doom/p_inter.cpp"
    "Doom/p_inter.h"
    "Doom/p_lights.cpp"
    "Doom/p_lights.h"
    "Doom/p_plats.cpp"
    "Doom/p_plats.h"
    "Doom/p_pspr.cpp"
    "Doom/p_pspr.h"
    "Doom/p_setup.cpp"
    "Doom/p_setup.h"
    "Doom/s_sound.cpp"
    "Doom/s_sound.h"
    "PsxVm/PsxVm.h"
    "PsyQ/LIBAPI.cpp"
    "PsyQ/LIBAPI.h"
    "PsyQ/LIBC2.cpp"
    "PsyQ/LIBC2.h"
    "PsyQ/LIBCD.cpp"
    "PsyQ/LIBCD.h"
    "PsyQ/LIBCOMB.cpp"
    "PsyQ/LIBCOMB.h"
    "PsyQ/LIBETC.cpp"
    "PsyQ/LIBETC.h"
    "PsyQ/LIBGPU.cpp"
    "PsyQ/LIBGPU.h"
    "PsyQ/LIBGTE.cpp"
    "PsyQ/LIBGTE.h"
    "PsyQ/LIBSN.cpp"
    "PsyQ/LIBSN.h"
    "PsyQ/LIBSPU.cpp"
    "PsyQ/LIBSPU.h"
    "Wess/PSXCD.cpp"
    "Wess/PSXCD.h"
)

set(OTHER_FILES
)

set(INCLUDE_PATHS
    "."
)

# Platform specific sources
if (PLATFORM_WINDOWS)
    list(APPEND SOURCE_FILES "Main_Windows.cpp")
elseif(PLATFORM_MAC)
    list(APPEND SOURCE_FILES "Main_Mac.mm")
else()
    list(APPEND SOURCE_FILES "Main_StandardCpp.cpp")
endif()

# Build the executable and setup IDE folders
add_executable(${GAME_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

# Specify include dirs
include_directories(${INCLUDE_PATHS})

# Setup target compile options
add_common_target_compile_options(${GAME_TGT_NAME})

if (PLATFORM_WINDOWS)
    set_property(TARGET ${GAME_TGT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()

if (PLATFORM_MAC)
    SET_TARGET_PROPERTIES(${GAME_TGT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
endif()

if (PLATFORM_LINUX)
    target_compile_options(${GAME_TGT_NAME} PRIVATE -pthread)
endif()
