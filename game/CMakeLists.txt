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
    "Doom/UI/cn_main.cpp"
    "Doom/UI/cn_main.h"
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
    "PcPsx/DemoResult.cpp"
    "PcPsx/DemoResult.h"
    "PcPsx/Endian.h"
    "PcPsx/FatalErrors.cpp"
    "PcPsx/FatalErrors.h"
    "PcPsx/FileUtils.cpp"
    "PcPsx/FileUtils.h"
    "PcPsx/Finally.h"
    "PcPsx/IniUtils.cpp"
    "PcPsx/IniUtils.h"
    "PcPsx/Macros.h"
    "PcPsx/ModMgr.cpp"
    "PcPsx/ModMgr.h"
    "PcPsx/Network.cpp"
    "PcPsx/Network.h"
    "PcPsx/ProgArgs.cpp"
    "PcPsx/ProgArgs.h"
    "PcPsx/Types.h"
    "PcPsx/Utils.cpp"
    "PcPsx/Utils.h"
    "PcPsx/Video.cpp"
    "PcPsx/Video.h"
    "PsxVm/PsxVm.h"
    "PsxVm/PsxVm_VmControl.cpp"
    "PsxVm/PsxVm_VmInterface.cpp"
    "PsyQ/LIBAPI.cpp"
    "PsyQ/LIBAPI.h"
    "PsyQ/LIBCD.cpp"
    "PsyQ/LIBCD.h"
    "PsyQ/LIBCOMB.cpp"
    "PsyQ/LIBCOMB.h"
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
    -DPC_PSX_DOOM_MODS=1                        # Defined to mark areas where we changed the code from the PSX version
)

# Specify include dirs
include_directories(${INCLUDE_PATHS})
    
# Required libraries
target_link_libraries(${GAME_TGT_NAME}
    ${AVOCADO_TGT_NAME}
    ${RAPID_JSON_TGT_NAME}
    ${ASIO_TGT_NAME}
)

# MSVC use a lower warning level for this project temporarily
if (COMPILER_MSVC)
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4102)     # No unreferenced label warnings
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4146)     # Negating unsigned integer
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4245)     # Conversion from signed to unsigned
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4702)     # Unreachable code
    target_compile_options(${GAME_TGT_NAME} PUBLIC /W4)         # Enable all warnings (except the ones disabled above)
    
    # MSVC: Don't complain about using regular 'std::fopen()' etc.
    target_compile_definitions(${GAME_TGT_NAME} PUBLIC -D_CRT_SECURE_NO_WARNINGS)
else()
    add_common_target_compile_options(${GAME_TGT_NAME})
endif()

# Setup target compile options
if (PLATFORM_WINDOWS)
    # Disable for now!
    # set_property(TARGET ${GAME_TGT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()

if (PLATFORM_MAC)
    # Disable for now!
    # SET_TARGET_PROPERTIES(${GAME_TGT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
endif()

if (PLATFORM_LINUX)
    target_compile_options(${GAME_TGT_NAME} PRIVATE -pthread)
endif()
