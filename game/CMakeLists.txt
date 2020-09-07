# Use the old "I_Error" implementation which hangs the game with a message on the screen?
set(PSYDOOM_USE_NEW_I_ERROR TRUE CACHE BOOL
"If TRUE (default) then use the new implementation of 'I_Error' for showing original game errors such as 'Texture Cache Overflow'.
PsyDoom by default now uses a system error dialog instead of just hanging the app on an error message because it is more user
friendly and easier to quit out of on modern platforms.")

# This setting switches on/off fixes to undefined behavior from the original code
set(PSYDOOM_FIX_UB TRUE CACHE BOOL
"If TRUE (default) then fix certain cases of undefined behavior in the original PSX Doom code.
Examples of fixes to undefined behavior include added bounds checks on certain blockmap operations, which in the original
game could result in memory corruption if out of bounds conditions were triggered - the so called 'linedef deletion' bug.
If undefined behavior fixes are NOT enabled then asserts (in debug builds) will be triggered when certain types of UB is encountered.
These asserts can be useful to identify cases of demo de-sync for demos recorded against the original games which trigger UB.
Such gameplay which triggers UB in the original game is not possible to play back correctly in PsyDoom, because UB behavior
requires the exact same hardware, compiler and execution environment to replicate."
)

# This setting includes old stuff in the project
set(PSYDOOM_INCLUDE_OLD_CODE FALSE CACHE BOOL 
"If TRUE include source files from the 'Old' directory of the PsyDoom project.
These files contain original or older versions of various functions, variables and data structures.
These pieces of code are archived for future reference in case we need to refer back to the way things originally worked.
Most of these functions etc. have either been completely rewritten or so drastically altered that the '#if PSYDOOM_MODS'
mechanism no longer makes sense or is too unwieldly to use."
)

# Platform agnostic sources
set(SOURCE_FILES
    "Doom/Base/d_vsprintf.cpp"
    "Doom/Base/d_vsprintf.h"
    "Doom/Base/i_crossfade.cpp"
    "Doom/Base/i_crossfade.h"
    "Doom/Base/i_drawcmds.cpp"
    "Doom/Base/i_drawcmds.h"
    "Doom/Base/i_file.cpp"
    "Doom/Base/i_file.h"
    "Doom/Base/i_main.cpp"
    "Doom/Base/i_main.h"
    "Doom/Base/i_misc.cpp"
    "Doom/Base/i_misc.h"
    "Doom/Base/m_bbox.cpp"
    "Doom/Base/m_bbox.h"
    "Doom/Base/m_fixed.cpp"
    "Doom/Base/m_fixed.h"
    "Doom/Base/m_random.cpp"
    "Doom/Base/m_random.h"
    "Doom/Base/s_sound.cpp"
    "Doom/Base/s_sound.h"
    "Doom/Base/sounds.h"
    "Doom/Base/tables.cpp"
    "Doom/Base/w_wad.cpp"
    "Doom/Base/w_wad.h"
    "Doom/Base/z_zone.cpp"
    "Doom/Base/z_zone.h"
    "Doom/cdmaptbl.cpp"
    "Doom/cdmaptbl.h"
    "Doom/d_main.cpp"
    "Doom/d_main.h"
    "Doom/doomdef.h"
    "Doom/doomdef.h"
    "Doom/Game/doomdata.h"
    "Doom/Game/g_game.cpp"
    "Doom/Game/g_game.h"
    "Doom/Game/info.cpp"
    "Doom/Game/info.h"
    "Doom/Game/p_base.cpp"
    "Doom/Game/p_base.h"
    "Doom/Game/p_ceiling.cpp"
    "Doom/Game/p_ceiling.h"
    "Doom/Game/p_change.cpp"
    "Doom/Game/p_change.h"
    "Doom/Game/p_doors.cpp"
    "Doom/Game/p_doors.h"
    "Doom/Game/p_enemy.cpp"
    "Doom/Game/p_enemy.h"
    "Doom/Game/p_firesky.cpp"
    "Doom/Game/p_firesky.h"
    "Doom/Game/p_floor.cpp"
    "Doom/Game/p_floor.h"
    "Doom/Game/p_inter.cpp"
    "Doom/Game/p_inter.h"
    "Doom/Game/p_lights.cpp"
    "Doom/Game/p_lights.h"
    "Doom/Game/p_local.h"
    "Doom/Game/p_map.cpp"
    "Doom/Game/p_map.h"
    "Doom/Game/p_maputl.cpp"
    "Doom/Game/p_maputl.h"
    "Doom/Game/p_mobj.cpp"
    "Doom/Game/p_mobj.h"
    "Doom/Game/p_move.cpp"
    "Doom/Game/p_move.h"
    "Doom/Game/p_password.cpp"
    "Doom/Game/p_password.h"
    "Doom/Game/p_plats.cpp"
    "Doom/Game/p_plats.h"
    "Doom/Game/p_pspr.cpp"
    "Doom/Game/p_pspr.h"
    "Doom/Game/p_setup.cpp"
    "Doom/Game/p_setup.h"
    "Doom/Game/p_shoot.cpp"
    "Doom/Game/p_shoot.h"
    "Doom/Game/p_sight.cpp"
    "Doom/Game/p_sight.h"
    "Doom/Game/p_slide.cpp"
    "Doom/Game/p_slide.h"
    "Doom/Game/p_spec.cpp"
    "Doom/Game/p_spec.h"
    "Doom/Game/p_switch.cpp"
    "Doom/Game/p_switch.h"
    "Doom/Game/p_telept.cpp"
    "Doom/Game/p_telept.h"
    "Doom/Game/p_tick.cpp"
    "Doom/Game/p_tick.h"
    "Doom/Game/p_user.cpp"
    "Doom/Game/p_user.h"
    "Doom/psx_main.cpp"
    "Doom/psx_main.h"
    "Doom/Renderer/r_bsp.cpp"
    "Doom/Renderer/r_bsp.h"
    "Doom/Renderer/r_data.cpp"
    "Doom/Renderer/r_data.h"
    "Doom/Renderer/r_draw.cpp"
    "Doom/Renderer/r_draw.h"
    "Doom/Renderer/r_local.h"
    "Doom/Renderer/r_main.cpp"
    "Doom/Renderer/r_main.h"
    "Doom/Renderer/r_plane.cpp"
    "Doom/Renderer/r_plane.h"
    "Doom/Renderer/r_segs.cpp"
    "Doom/Renderer/r_segs.h"
    "Doom/Renderer/r_sky.cpp"
    "Doom/Renderer/r_sky.h"
    "Doom/Renderer/r_things.cpp"
    "Doom/Renderer/r_things.h"
    "Doom/Renderer/sprinfo.cpp"
    "Doom/UI/am_main.cpp"
    "Doom/UI/am_main.h"
    "Doom/UI/cr_main.cpp"
    "Doom/UI/cr_main.h"
    "Doom/UI/f_finale.cpp"
    "Doom/UI/f_finale.h"
    "Doom/UI/in_main.cpp"
    "Doom/UI/in_main.h"
    "Doom/UI/le_main.cpp"
    "Doom/UI/le_main.h"
    "Doom/UI/m_main.cpp"
    "Doom/UI/m_main.h"
    "Doom/UI/o_main.cpp"
    "Doom/UI/o_main.h"
    "Doom/UI/pw_main.cpp"
    "Doom/UI/pw_main.h"
    "Doom/UI/st_main.cpp"
    "Doom/UI/st_main.h"
    "Doom/UI/ti_main.cpp"
    "Doom/UI/ti_main.h"
    "EngineLimits.h"
    "PcPsx/Config.cpp"
    "PcPsx/Config.h"
    "PcPsx/ControllerInput.cpp"
    "PcPsx/ControllerInput.h"
    "PcPsx/Controls.cpp"
    "PcPsx/Controls.h"
    "PcPsx/DemoResult.cpp"
    "PcPsx/DemoResult.h"
    "PcPsx/DiscInfo.cpp"
    "PcPsx/DiscInfo.h"
    "PcPsx/DiscReader.cpp"
    "PcPsx/DiscReader.h"
    "PcPsx/Game.cpp"
    "PcPsx/Game.h"
    "PcPsx/Input.cpp"
    "PcPsx/Input.h"
    "PcPsx/IsoFileSys.cpp"
    "PcPsx/IsoFileSys.h"
    "PcPsx/ModMgr.cpp"
    "PcPsx/ModMgr.h"
    "PcPsx/MouseButton.h"
    "PcPsx/NetPacketReader.h"
    "PcPsx/NetPacketWriter.h"
    "PcPsx/Network.cpp"
    "PcPsx/Network.h"
    "PcPsx/ProgArgs.cpp"
    "PcPsx/ProgArgs.h"
    "PcPsx/PsxPadButtons.h"
    "PcPsx/PsxVm.cpp"
    "PcPsx/PsxVm.h"
    "PcPsx/Types.h"
    "PcPsx/Utils.cpp"
    "PcPsx/Utils.h"
    "PcPsx/Video.cpp"
    "PcPsx/Video.h"
    "PsyQ/LIBAPI.cpp"
    "PsyQ/LIBAPI.h"
    "PsyQ/LIBETC.cpp"
    "PsyQ/LIBETC.h"
    "PsyQ/LIBGPU.cpp"
    "PsyQ/LIBGPU.h"
    "PsyQ/LIBGPU_Resources.cpp"
    "PsyQ/LIBGTE.cpp"
    "PsyQ/LIBGTE.h"
    "PsyQ/LIBSN.cpp"
    "PsyQ/LIBSN.h"
    "PsyQ/LIBSPU.cpp"
    "PsyQ/LIBSPU.h"
    "PsyQ/LIBSPU_Resources.cpp"
    "Wess/lcdload.cpp"
    "Wess/lcdload.h"
    "Wess/psxcd.cpp"
    "Wess/psxcd.h"
    "Wess/psxcmd.cpp"
    "Wess/psxcmd.h"
    "Wess/psxspu.cpp"
    "Wess/psxspu.h"
    "Wess/seqload.cpp"
    "Wess/seqload.h"
    "Wess/seqload_r.cpp"
    "Wess/seqload_r.h"
    "Wess/wessapi.cpp"
    "Wess/wessapi.h"
    "Wess/wessapi_m.cpp"
    "Wess/wessapi_m.h"
    "Wess/wessapi_p.cpp"
    "Wess/wessapi_p.h"
    "Wess/wessapi_t.cpp"
    "Wess/wessapi_t.h"
    "Wess/wessarc.cpp"
    "Wess/wessarc.h"
    "Wess/wessseq.cpp"
    "Wess/wessseq.h"
)

if (PSYDOOM_INCLUDE_OLD_CODE)
    list(APPEND SOURCE_FILES
        "Old/Doom/Base/Old_i_crossfade.cpp"
        "Old/Doom/Base/Old_i_main.cpp"
        "Old/Doom/Base/Old_i_main.h"
        "Old/Doom/Base/Old_z_zone.cpp"
        "Old/Doom/Game/Old_p_setup.cpp"
        "Old/Doom/UI/Old_cn_main.cpp"
        "Old/Doom/UI/Old_cn_main.h"
        "Old/Doom/UI/Old_ti_main.cpp"
        "Old/Old_cdmaptbl.cpp"
        "Old/PsyQ/Old_LIBAPI.cpp"
        "Old/PsyQ/Old_LIBAPI.h"
        "Old/PsyQ/Old_LIBCD.cpp"
        "Old/PsyQ/Old_LIBCD.h"
        "Old/PsyQ/Old_LIBCOMB.cpp"
        "Old/PsyQ/Old_LIBCOMB.h"
        "Old/Wess/Old_lcdload.cpp"
        "Old/Wess/Old_lcdload.h"
        "Old/Wess/Old_psxcd.cpp"
        "Old/Wess/Old_psxcd.h"
    )
endif()

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

target_compile_definitions(${GAME_TGT_NAME} PRIVATE
    -DGAME_VERSION_STR="${GAME_VERSION_STR}"    # Game version string - set in main CMakeLists.txt
    -DPSYDOOM_MODS=1                            # Defined to mark areas where we changed the code from the PSX version
)

# Various tweaks that can be applied
if (PSYDOOM_FIX_UB)
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -DPSYDOOM_FIX_UB=1)
else()
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -DPSYDOOM_FIX_UB=0)
endif()

if (PSYDOOM_USE_NEW_I_ERROR)
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -DPSYDOOM_USE_NEW_I_ERROR=1)
else()
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -DPSYDOOM_USE_NEW_I_ERROR=0)
endif()

# Specify include dirs
include_directories(${INCLUDE_PATHS})

# Required libraries
target_link_libraries(${GAME_TGT_NAME}
    ${ASIO_TGT_NAME}
    ${AVOCADO_TGT_NAME}
    ${BASELIB_TGT_NAME}
    ${RAPID_JSON_TGT_NAME}
    ${SIMPLE_SPU_TGT_NAME}
)

# MSVC use a lower warning level for this project temporarily
if (COMPILER_MSVC)
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4102)     # No unreferenced label warnings
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4146)     # Negating unsigned integer
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4245)     # Conversion from signed to unsigned
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4702)     # Unreachable code
    target_compile_options(${GAME_TGT_NAME} PUBLIC /W4)         # Enable all warnings (except the ones disabled above)
    
    # MSVC: Don't complain about using regular 'std::fopen()' etc.
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
else()
    add_common_target_compile_options(${GAME_TGT_NAME})
endif()

# Setup target compile options
if (PLATFORM_WINDOWS)
    set_property(TARGET ${GAME_TGT_NAME} PROPERTY WIN32_EXECUTABLE true)        # Win32 GUI APP
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -DUNICODE -D_UNICODE)   # Use Unicode WinMain()
elseif(PLATFORM_MAC)
    # Fixme: compile as a .app eventually
    # SET_TARGET_PROPERTIES(${GAME_TGT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
elseif (PLATFORM_LINUX)
    target_compile_options(${GAME_TGT_NAME} PRIVATE -pthread)
endif()
