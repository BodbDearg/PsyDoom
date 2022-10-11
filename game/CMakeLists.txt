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
    "Doom/Base/i_drawcmds.h"
    "Doom/Base/i_file.cpp"
    "Doom/Base/i_file.h"
    "Doom/Base/i_main.cpp"
    "Doom/Base/i_main.h"
    "Doom/Base/i_misc.cpp"
    "Doom/Base/i_misc.h"
    "Doom/Base/i_texcache.cpp"
    "Doom/Base/i_texcache.h"
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
    "Doom/doomdef.cpp"
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
    "Doom/Game/p_info.cpp"
    "Doom/Game/p_info.h"
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
    "Doom/Game/p_weak.cpp"
    "Doom/Game/p_weak.h"
    "Doom/Game/sprinfo.cpp"
    "Doom/Game/sprinfo.h"
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
    "Doom/UI/am_main.cpp"
    "Doom/UI/am_main.h"
    "Doom/UI/cr_main.cpp"
    "Doom/UI/cr_main.h"
    "Doom/UI/errormenu_main.cpp"
    "Doom/UI/errormenu_main.h"
    "Doom/UI/f_finale.cpp"
    "Doom/UI/f_finale.h"
    "Doom/UI/in_main.cpp"
    "Doom/UI/in_main.h"
    "Doom/UI/le_main.cpp"
    "Doom/UI/le_main.h"
    "Doom/UI/loadsave_main.cpp"
    "Doom/UI/loadsave_main.h"
    "Doom/UI/m_main.cpp"
    "Doom/UI/m_main.h"
    "Doom/UI/o_main.cpp"
    "Doom/UI/o_main.h"
    "Doom/UI/pw_main.cpp"
    "Doom/UI/pw_main.h"
    "Doom/UI/saveroot_main.cpp"
    "Doom/UI/saveroot_main.h"
    "Doom/UI/st_main.cpp"
    "Doom/UI/st_main.h"
    "Doom/UI/ti_main.cpp"
    "Doom/UI/ti_main.h"
    "Doom/UI/xoptions_main.cpp"
    "Doom/UI/xoptions_main.h"
    "EngineLimits.h"
    "PsyDoom/AudioCompressor.cpp"
    "PsyDoom/AudioCompressor.h"
    "PsyDoom/BitShift.h"
    "PsyDoom/Cheats.cpp"
    "PsyDoom/Cheats.h"
    "PsyDoom/Config/Config.cpp"
    "PsyDoom/Config/Config.h"
    "PsyDoom/Config/ConfigSerialization.cpp"
    "PsyDoom/Config/ConfigSerialization.h"
    "PsyDoom/Config/ConfigSerialization_Audio.cpp"
    "PsyDoom/Config/ConfigSerialization_Audio.h"
    "PsyDoom/Config/ConfigSerialization_Cheats.cpp"
    "PsyDoom/Config/ConfigSerialization_Cheats.h"
    "PsyDoom/Config/ConfigSerialization_Controls.cpp"
    "PsyDoom/Config/ConfigSerialization_Controls.h"
    "PsyDoom/Config/ConfigSerialization_Game.cpp"
    "PsyDoom/Config/ConfigSerialization_Game.h"
    "PsyDoom/Config/ConfigSerialization_Graphics.cpp"
    "PsyDoom/Config/ConfigSerialization_Graphics.h"
    "PsyDoom/Config/ConfigSerialization_Input.cpp"
    "PsyDoom/Config/ConfigSerialization_Input.h"
    "PsyDoom/Config/ConfigSerialization_Multiplayer.cpp"
    "PsyDoom/Config/ConfigSerialization_Multiplayer.h"
    "PsyDoom/Controls.cpp"
    "PsyDoom/Controls.h"
    "PsyDoom/DemoCommon.cpp"
    "PsyDoom/DemoCommon.h"
    "PsyDoom/DemoPlayer.cpp"
    "PsyDoom/DemoPlayer.h"
    "PsyDoom/DemoRecorder.cpp"
    "PsyDoom/DemoRecorder.h"
    "PsyDoom/DemoResult.cpp"
    "PsyDoom/DemoResult.h"
    "PsyDoom/DevMapAutoReloader.cpp"
    "PsyDoom/DevMapAutoReloader.h"
    "PsyDoom/DiscInfo.cpp"
    "PsyDoom/DiscInfo.h"
    "PsyDoom/DiscReader.cpp"
    "PsyDoom/DiscReader.h"
    "PsyDoom/FixedIndexSet.h"
    "PsyDoom/Game.cpp"
    "PsyDoom/Game.h"
    "PsyDoom/GameConstants.cpp"
    "PsyDoom/GameConstants.h"
    "PsyDoom/GameFileReader.cpp"
    "PsyDoom/GameFileReader.h"
    "PsyDoom/GamepadInput.cpp"
    "PsyDoom/GamepadInput.h"
    "PsyDoom/Input.cpp"
    "PsyDoom/Input.h"
    "PsyDoom/InterpFixedT.cpp"
    "PsyDoom/InterpFixedT.h"
    "PsyDoom/IntroLogos.cpp"
    "PsyDoom/IntroLogos.h"
    "PsyDoom/IsoFileSys.cpp"
    "PsyDoom/IsoFileSys.h"
    "PsyDoom/IVideoBackend.h"
    "PsyDoom/IVideoSurface.h"
    "PsyDoom/LIBGPU_CmdDispatch.cpp"
    "PsyDoom/LIBGPU_CmdDispatch.h"
    "PsyDoom/LogoPlayer.cpp"
    "PsyDoom/LogoPlayer.h"
    "PsyDoom/MapHash.cpp"
    "PsyDoom/MapHash.h"
    "PsyDoom/MapInfo/MapInfo.cpp"
    "PsyDoom/MapInfo/MapInfo.h"
    "PsyDoom/MapInfo/MapInfo_Defaults.cpp"
    "PsyDoom/MapInfo/MapInfo_Defaults.h"
    "PsyDoom/MapInfo/MapInfo_Defaults_Doom.cpp"
    "PsyDoom/MapInfo/MapInfo_Defaults_Doom.h"
    "PsyDoom/MapInfo/MapInfo_Defaults_FinalDoom.cpp"
    "PsyDoom/MapInfo/MapInfo_Defaults_FinalDoom.h"
    "PsyDoom/MapInfo/MapInfo_Defaults_GEC_ME.cpp"
    "PsyDoom/MapInfo/MapInfo_Defaults_GEC_ME.h"
    "PsyDoom/MapInfo/MapInfo_Parse.cpp"
    "PsyDoom/MapInfo/MapInfo_Parse.h"
    "PsyDoom/MapPatcher/MapPatcher.cpp"
    "PsyDoom/MapPatcher/MapPatcher.h"
    "PsyDoom/MapPatcher/MapPatches.h"
    "PsyDoom/MapPatcher/MapPatches_Doom.cpp"
    "PsyDoom/MapPatcher/MapPatches_FinalDoom.cpp"
    "PsyDoom/MapPatcher/MapPatches_GEC_ME_Beta3.cpp"
    "PsyDoom/MobjSpritePrecacher.cpp"
    "PsyDoom/MobjSpritePrecacher.h"
    "PsyDoom/ModMgr.cpp"
    "PsyDoom/ModMgr.h"
    "PsyDoom/MouseButton.h"
    "PsyDoom/Movie/Block.cpp"
    "PsyDoom/Movie/Block.h"
    "PsyDoom/Movie/CDXAFileStreamer.cpp"
    "PsyDoom/Movie/CDXAFileStreamer.h"
    "PsyDoom/Movie/Frame.cpp"
    "PsyDoom/Movie/Frame.h"
    "PsyDoom/Movie/MacroBlockDecoder.cpp"
    "PsyDoom/Movie/MacroBlockDecoder.h"
    "PsyDoom/Movie/MBlockBitStream.cpp"
    "PsyDoom/Movie/MBlockBitStream.h"
    "PsyDoom/Movie/MoviePlayer.cpp"
    "PsyDoom/Movie/MoviePlayer.h"
    "PsyDoom/Movie/XAAdpcmDecoder.cpp"
    "PsyDoom/Movie/XAAdpcmDecoder.h"
    "PsyDoom/NetPacketReader.h"
    "PsyDoom/NetPacketWriter.h"
    "PsyDoom/Network.cpp"
    "PsyDoom/Network.h"
    "PsyDoom/ParserTokenizer.cpp"
    "PsyDoom/ParserTokenizer.h"
    "PsyDoom/PlayerPrefs.cpp"
    "PsyDoom/PlayerPrefs.h"
    "PsyDoom/ProgArgs.cpp"
    "PsyDoom/ProgArgs.h"
    "PsyDoom/PsxPadButtons.h"
    "PsyDoom/PsxVm.cpp"
    "PsyDoom/PsxVm.h"
    "PsyDoom/ResizableBuffer.h"
    "PsyDoom/SaveAndLoad.cpp"
    "PsyDoom/SaveAndLoad.h"
    "PsyDoom/SaveDataTypes.cpp"
    "PsyDoom/SaveDataTypes.h"
    "PsyDoom/ScriptBindings.cpp"
    "PsyDoom/ScriptBindings.h"
    "PsyDoom/ScriptingEngine.cpp"
    "PsyDoom/ScriptingEngine.h"
    "PsyDoom/TexturePatcher.cpp"
    "PsyDoom/TexturePatcher.h"
    "PsyDoom/Utils.cpp"
    "PsyDoom/Utils.h"
    "PsyDoom/Video.cpp"
    "PsyDoom/Video.h"
    "PsyDoom/VideoBackend_SDL.cpp"
    "PsyDoom/VideoBackend_SDL.h"
    "PsyDoom/VideoSurface_SDL.cpp"
    "PsyDoom/VideoSurface_SDL.h"
    "PsyDoom/WadFile.cpp"
    "PsyDoom/WadFile.h"
    "PsyDoom/WadList.cpp"
    "PsyDoom/WadList.h"
    "PsyDoom/WadUtils.cpp"
    "PsyDoom/WadUtils.h"
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

if (PSYDOOM_INCLUDE_LAUNCHER)
    list(APPEND SOURCE_FILES
        "PsyDoom/Launcher/AddInputPrompt.cpp"
        "PsyDoom/Launcher/AddInputPrompt.h"
        "PsyDoom/Launcher/Launcher.cpp"
        "PsyDoom/Launcher/Launcher.h"
        "PsyDoom/Launcher/Launcher_Context.h"
        "PsyDoom/Launcher/Launcher_Tab_Audio.cpp"
        "PsyDoom/Launcher/Launcher_Tab_Cheats.cpp"
        "PsyDoom/Launcher/Launcher_Tab_Controls.cpp"
        "PsyDoom/Launcher/Launcher_Tab_Game.cpp"
        "PsyDoom/Launcher/Launcher_Tab_Graphics.cpp"
        "PsyDoom/Launcher/Launcher_Tab_Input.cpp"
        "PsyDoom/Launcher/Launcher_Tab_Multiplayer.cpp"
        "PsyDoom/Launcher/Launcher_Tab_Launcher.cpp"
        "PsyDoom/Launcher/Launcher_Utils.cpp"
        "PsyDoom/Launcher/Launcher_Utils.h"
        "PsyDoom/Launcher/LauncherPrefs.cpp"
        "PsyDoom/Launcher/LauncherPrefs.h"
    )    
endif()

if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
    list(APPEND SOURCE_FILES
        "Doom/RendererVk/rv_automap.cpp"
        "Doom/RendererVk/rv_automap.h"
        "Doom/RendererVk/rv_bsp.cpp"
        "Doom/RendererVk/rv_bsp.h"
        "Doom/RendererVk/rv_data.cpp"
        "Doom/RendererVk/rv_data.h"
        "Doom/RendererVk/rv_flats.cpp"
        "Doom/RendererVk/rv_flats.h"
        "Doom/RendererVk/rv_main.cpp"
        "Doom/RendererVk/rv_main.h"
        "Doom/RendererVk/rv_occlusion.cpp"
        "Doom/RendererVk/rv_occlusion.h"
        "Doom/RendererVk/rv_sky.cpp"
        "Doom/RendererVk/rv_sky.h"
        "Doom/RendererVk/rv_sprites.cpp"
        "Doom/RendererVk/rv_sprites.h"
        "Doom/RendererVk/rv_utils.cpp"
        "Doom/RendererVk/rv_utils.h"
        "Doom/RendererVk/rv_walls.cpp"
        "Doom/RendererVk/rv_walls.h"
        "PsyDoom/VideoBackend_Vulkan.cpp"
        "PsyDoom/VideoBackend_Vulkan.h"
        "PsyDoom/VideoSurface_Vulkan.cpp"
        "PsyDoom/VideoSurface_Vulkan.h"
        "PsyDoom/Vulkan/IVRenderPath.h"
        "PsyDoom/Vulkan/VCrossfader.cpp"
        "PsyDoom/Vulkan/VCrossfader.h"
        "PsyDoom/Vulkan/VDrawing.cpp"
        "PsyDoom/Vulkan/VDrawing.h"
        "PsyDoom/Vulkan/VMsaaResolver.cpp"
        "PsyDoom/Vulkan/VMsaaResolver.h"
        "PsyDoom/Vulkan/VPipelines.cpp"
        "PsyDoom/Vulkan/VPipelines.h"
        "PsyDoom/Vulkan/VPlaqueDrawer.cpp"
        "PsyDoom/Vulkan/VPlaqueDrawer.h"
        "PsyDoom/Vulkan/VRenderer.cpp"
        "PsyDoom/Vulkan/VRenderer.h"
        "PsyDoom/Vulkan/VRenderPath_Blit.cpp"
        "PsyDoom/Vulkan/VRenderPath_Blit.h"
        "PsyDoom/Vulkan/VRenderPath_Crossfade.cpp"
        "PsyDoom/Vulkan/VRenderPath_Crossfade.h"
        "PsyDoom/Vulkan/VRenderPath_Main.cpp"
        "PsyDoom/Vulkan/VRenderPath_Main.h"
        "PsyDoom/Vulkan/VRenderPath_Psx.cpp"
        "PsyDoom/Vulkan/VRenderPath_Psx.h"
        "PsyDoom/Vulkan/VScreenQuad.cpp"
        "PsyDoom/Vulkan/VScreenQuad.h"
        "PsyDoom/Vulkan/VTypes.h"
        "PsyDoom/Vulkan/VVertexBufferSet.h"
    )
endif()

if (PSYDOOM_INCLUDE_OLD_CODE)
    list(APPEND SOURCE_FILES
        "Old/Doom/Base/Old_i_crossfade.cpp"
        "Old/Doom/Base/Old_i_drawcmds.cpp"
        "Old/Doom/Base/Old_i_drawcmds.h"
        "Old/Doom/Base/Old_i_main.cpp"
        "Old/Doom/Base/Old_i_main.h"
        "Old/Doom/Base/Old_s_sound.cpp"
        "Old/Doom/Base/Old_w_wad.cpp"
        "Old/Doom/Base/Old_w_wad.h"
        "Old/Doom/Base/Old_z_zone.cpp"
        "Old/Doom/Game/Old_g_game.cpp"
        "Old/Doom/Game/Old_p_ceiling.cpp"
        "Old/Doom/Game/Old_p_plats.cpp"
        "Old/Doom/Game/Old_p_setup.cpp"
        "Old/Doom/Game/Old_sprinfo.cpp"
        "Old/Doom/Renderer/Old_r_data.cpp"
        "Old/Doom/UI/Old_cn_main.cpp"
        "Old/Doom/UI/Old_cn_main.h"
        "Old/Doom/UI/Old_ti_main.cpp"
        "Old/Old_cdmaptbl.cpp"
        "Old/Old_cdmaptbl.h"
        "Old/PsyQ/Old_LIBAPI.cpp"
        "Old/PsyQ/Old_LIBAPI.h"
        "Old/PsyQ/Old_LIBCD.cpp"
        "Old/PsyQ/Old_LIBCD.h"
        "Old/PsyQ/Old_LIBCOMB.cpp"
        "Old/PsyQ/Old_LIBCOMB.h"
        "Old/PsyQ/Old_LIBETC.cpp"
        "Old/PsyQ/Old_LIBETC.h"
        "Old/PsyQ/Old_LIBGPU.cpp"
        "Old/PsyQ/Old_LIBGPU.h"
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
    "../vulkan_shaders/compiled"
)

# Platform specific sources
if (PLATFORM_WINDOWS)
    # Windows
    list(APPEND SOURCE_FILES
        "Main_Windows.cpp"
        "Resources/Windows/PsyDoom.rc"
    )

    # Generating the version info resource
    configure_file(
        "Resources/Windows/version.template.rc"
        "${CMAKE_CURRENT_BINARY_DIR}/version.generated.rc"
    )    

    set(GENERATED_SOURCE_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/version.generated.rc"
    )
elseif(PLATFORM_MAC)
    # macOS
    list(APPEND SOURCE_FILES
        "Main_Mac.mm"
        "Resources/macOS/icon.icns"
    )

    # Set the app icon and ensure it is bundled with the app
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set_source_files_properties("Resources/macOS/icon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
else()
    # Linux or other platforms
    list(APPEND SOURCE_FILES
        "Main_StandardCpp.cpp"
    )
endif()

# Build the executable and setup IDE folders
add_executable(${GAME_TGT_NAME} ${SOURCE_FILES} ${GENERATED_SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

target_compile_definitions(${GAME_TGT_NAME} PRIVATE
    -DGAME_VERSION_STR="${GAME_VERSION_STR}"    # Game version string - set in main CMakeLists.txt
    -DPSYDOOM_MODS=1                            # Defined to mark areas where we changed the code from the PSX version
    -DSOL_ALL_SAFETIES_ON=1                     # Lua scripting: full error checking of function params etc. (don't just crash)
) 

# Various tweaks that can be applied
target_bool_compile_definition(${GAME_TGT_NAME} PRIVATE PSYDOOM_FIX_UB                  ${PSYDOOM_FIX_UB})
target_bool_compile_definition(${GAME_TGT_NAME} PRIVATE PSYDOOM_LAUNCHER                ${PSYDOOM_INCLUDE_LAUNCHER})
target_bool_compile_definition(${GAME_TGT_NAME} PRIVATE PSYDOOM_LIMIT_REMOVING          ${PSYDOOM_LIMIT_REMOVING})
target_bool_compile_definition(${GAME_TGT_NAME} PRIVATE PSYDOOM_MISSING_TEX_WARNINGS    ${PSYDOOM_EMIT_MISSING_TEX_WARNINGS})
target_bool_compile_definition(${GAME_TGT_NAME} PRIVATE PSYDOOM_USE_NEW_I_ERROR         ${PSYDOOM_USE_NEW_I_ERROR})
target_bool_compile_definition(${GAME_TGT_NAME} PRIVATE PSYDOOM_VULKAN_RENDERER         ${PSYDOOM_INCLUDE_VULKAN_RENDERER})

# Specify include dirs
include_directories(${INCLUDE_PATHS})

# Required libraries
target_link_libraries(${GAME_TGT_NAME}
    ${ASIO_TGT_NAME}
    ${BASELIB_TGT_NAME}
    ${HASH_LIBRARY_TGT_NAME}
    ${LIBSDL_TGT_NAME}
    ${LUA_TGT_NAME}
    ${SIMPLE_GPU_TGT_NAME}
    ${SIMPLE_SPU_TGT_NAME}
    ${SOL2_TGT_NAME}
)

if (PSYDOOM_INCLUDE_LAUNCHER)
    target_link_libraries(${GAME_TGT_NAME} ${FLTK_TGT_NAME})
endif()

if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
    target_link_libraries(${GAME_TGT_NAME} ${VULKAN_GL_TGT_NAME})
endif()

# MSVC specific stuff
if (COMPILER_MSVC)
    # Need this flag to prevent the linker hitting object file limits on some modules
    target_compile_options(${GAME_TGT_NAME} PUBLIC /bigobj)

    # Warnings
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4102)     # Disable: no unreferenced label warnings
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4146)     # Disable: negating unsigned integer
    target_compile_options(${GAME_TGT_NAME} PUBLIC /wd4702)     # Disable: unreachable code
    target_compile_options(${GAME_TGT_NAME} PUBLIC /W4)         # Enable all warnings (except the ones disabled above)

    # MSVC: Don't complain about using regular 'std::fopen()' etc.
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
else()
    add_common_target_compile_options(${GAME_TGT_NAME})
endif()

# Clang or GCC specific
if (COMPILER_CLANG OR COMPILER_GCC)
    # Warnings
    target_compile_options(${GAME_TGT_NAME} PUBLIC -Wno-format-security)        # Disable: format string is not a string literal (potentially insecure)
endif()

# Setup target compile options
if (PLATFORM_WINDOWS)
    set_property(TARGET ${GAME_TGT_NAME} PROPERTY WIN32_EXECUTABLE true)        # Win32 GUI APP
    set_property(TARGET ${GAME_TGT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")    # Make the game DPI-aware so it correctly detects screen resolution
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -DUNICODE -D_UNICODE)   # Use Unicode WinMain()
    target_compile_definitions(${GAME_TGT_NAME} PRIVATE -D_WIN32_WINNT=0x0603)  # Target Windows 8.1 minimum
elseif (PLATFORM_MAC)
    set_target_properties(${GAME_TGT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "PsyDoom"
        MACOSX_BUNDLE_BUNDLE_VERSION ${GAME_VERSION_MAJOR}.${GAME_VERSION_MINOR}.${GAME_VERSION_BUILD}
        MACOSX_BUNDLE_INFO_STRING "${GAME_VERSION_STR}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${GAME_VERSION_STR}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${GAME_VERSION_STR}"
    )
elseif (PLATFORM_LINUX)
    target_compile_options(${GAME_TGT_NAME} PRIVATE -pthread)
endif()

# MacOS: linking against the installed Vulkan SDK and copying required Vulkan libraries and files to the app bundle
if (PLATFORM_MAC AND PSYDOOM_INCLUDE_VULKAN_RENDERER)
    find_package(Vulkan REQUIRED)
    target_link_libraries(${GAME_TGT_NAME} ${Vulkan_LIBRARIES})
    target_link_options(${GAME_TGT_NAME} PRIVATE -rpath @executable_path/../Frameworks) # So the runtime binary knows where to load .dylibs
    get_filename_component(VULKAN_LIBS_DIR ${Vulkan_LIBRARIES} DIRECTORY)

    add_custom_command(
        TARGET ${GAME_TGT_NAME} POST_BUILD
        COMMENT "Copy Vulkan dylibs and resources to app bundle"
        COMMAND mkdir -p "$<TARGET_FILE_DIR:${GAME_TGT_NAME}>/../Frameworks"
        COMMAND mkdir -p "$<TARGET_FILE_DIR:${GAME_TGT_NAME}>/../Resources/vulkan/icd.d"
        COMMAND cp "${VULKAN_LIBS_DIR}/libvulkan.1.dylib" "$<TARGET_FILE_DIR:${GAME_TGT_NAME}>/../Frameworks"
        COMMAND cp "${VULKAN_LIBS_DIR}/libMoltenVK.dylib" "$<TARGET_FILE_DIR:${GAME_TGT_NAME}>/../Frameworks"
        COMMAND cp "${PROJECT_SOURCE_DIR}/Game/Resources/macOS/MoltenVK_icd.json" "$<TARGET_FILE_DIR:${GAME_TGT_NAME}>/../Resources/vulkan/icd.d"
    )
endif()

# Linux specific
if (PLATFORM_LINUX)
    # Static link the standard c++ and gcc libraries to make the binary more portable between Linux versions
    target_link_options(${GAME_TGT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()
