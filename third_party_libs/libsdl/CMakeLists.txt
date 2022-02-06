set(BASE_DIR        "sdl2")
set(SRC_DIR         "${BASE_DIR}/src")
set(INCLUDE_DIR     "${BASE_DIR}/include")

set(INCLUDE_PATHS
    ${INCLUDE_DIR}
)

set(EXPORTED_INCLUDE_PATHS
    ${INCLUDE_DIR}
)

if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
    list(APPEND EXPORTED_INCLUDE_PATHS
        "${SRC_DIR}/video/khronos"
    )
endif()

set(SOURCE_FILES
    "${INCLUDE_DIR}/begin_code.h"
    "${INCLUDE_DIR}/close_code.h"
    "${INCLUDE_DIR}/SDL.h"
    "${INCLUDE_DIR}/SDL_assert.h"
    "${INCLUDE_DIR}/SDL_atomic.h"
    "${INCLUDE_DIR}/SDL_audio.h"
    "${INCLUDE_DIR}/SDL_bits.h"
    "${INCLUDE_DIR}/SDL_blendmode.h"
    "${INCLUDE_DIR}/SDL_clipboard.h"
    "${INCLUDE_DIR}/SDL_config.h"
    "${INCLUDE_DIR}/SDL_config_wiz.h"
    "${INCLUDE_DIR}/SDL_copying.h"
    "${INCLUDE_DIR}/SDL_cpuinfo.h"
    "${INCLUDE_DIR}/SDL_egl.h"
    "${INCLUDE_DIR}/SDL_endian.h"
    "${INCLUDE_DIR}/SDL_error.h"
    "${INCLUDE_DIR}/SDL_events.h"
    "${INCLUDE_DIR}/SDL_filesystem.h"
    "${INCLUDE_DIR}/SDL_gamecontroller.h"
    "${INCLUDE_DIR}/SDL_gesture.h"
    "${INCLUDE_DIR}/SDL_haptic.h"
    "${INCLUDE_DIR}/SDL_hints.h"
    "${INCLUDE_DIR}/SDL_joystick.h"
    "${INCLUDE_DIR}/SDL_keyboard.h"
    "${INCLUDE_DIR}/SDL_keycode.h"
    "${INCLUDE_DIR}/SDL_loadso.h"
    "${INCLUDE_DIR}/SDL_locale.h"
    "${INCLUDE_DIR}/SDL_log.h"
    "${INCLUDE_DIR}/SDL_main.h"
    "${INCLUDE_DIR}/SDL_messagebox.h"
    "${INCLUDE_DIR}/SDL_misc.h"
    "${INCLUDE_DIR}/SDL_mouse.h"
    "${INCLUDE_DIR}/SDL_mutex.h"
    "${INCLUDE_DIR}/SDL_name.h"
    "${INCLUDE_DIR}/SDL_opengl.h"
    "${INCLUDE_DIR}/SDL_opengl_glext.h"
    "${INCLUDE_DIR}/SDL_pixels.h"
    "${INCLUDE_DIR}/SDL_platform.h"
    "${INCLUDE_DIR}/SDL_power.h"
    "${INCLUDE_DIR}/SDL_quit.h"
    "${INCLUDE_DIR}/SDL_rect.h"
    "${INCLUDE_DIR}/SDL_render.h"
    "${INCLUDE_DIR}/SDL_revision.h"
    "${INCLUDE_DIR}/SDL_rwops.h"
    "${INCLUDE_DIR}/SDL_scancode.h"
    "${INCLUDE_DIR}/SDL_sensor.h"
    "${INCLUDE_DIR}/SDL_shape.h"
    "${INCLUDE_DIR}/SDL_stdinc.h"
    "${INCLUDE_DIR}/SDL_surface.h"
    "${INCLUDE_DIR}/SDL_system.h"
    "${INCLUDE_DIR}/SDL_syswm.h"
    "${INCLUDE_DIR}/SDL_thread.h"
    "${INCLUDE_DIR}/SDL_timer.h"
    "${INCLUDE_DIR}/SDL_touch.h"
    "${INCLUDE_DIR}/SDL_types.h"
    "${INCLUDE_DIR}/SDL_version.h"
    "${INCLUDE_DIR}/SDL_video.h"
    "${SRC_DIR}/atomic/SDL_atomic.c"
    "${SRC_DIR}/atomic/SDL_spinlock.c"
    "${SRC_DIR}/audio/dummy/SDL_dummyaudio.c"
    "${SRC_DIR}/audio/dummy/SDL_dummyaudio.h"
    "${SRC_DIR}/audio/SDL_audio.c"
    "${SRC_DIR}/audio/SDL_audio_c.h"
    "${SRC_DIR}/audio/SDL_audiocvt.c"
    "${SRC_DIR}/audio/SDL_audiodev.c"
    "${SRC_DIR}/audio/SDL_audiodev_c.h"
    "${SRC_DIR}/audio/SDL_audiotypecvt.c"
    "${SRC_DIR}/audio/SDL_mixer.c"
    "${SRC_DIR}/audio/SDL_sysaudio.h"
    "${SRC_DIR}/audio/SDL_wave.c"
    "${SRC_DIR}/audio/SDL_wave.h"
    "${SRC_DIR}/cpuinfo/SDL_cpuinfo.c"
    "${SRC_DIR}/events/blank_cursor.h"
    "${SRC_DIR}/events/default_cursor.h"
    "${SRC_DIR}/events/imKStoUCS.c"
    "${SRC_DIR}/events/imKStoUCS.h"
    "${SRC_DIR}/events/SDL_clipboardevents.c"
    "${SRC_DIR}/events/SDL_clipboardevents_c.h"
    "${SRC_DIR}/events/SDL_displayevents.c"
    "${SRC_DIR}/events/SDL_displayevents_c.h"
    "${SRC_DIR}/events/SDL_dropevents.c"
    "${SRC_DIR}/events/SDL_dropevents_c.h"
    "${SRC_DIR}/events/SDL_events.c"
    "${SRC_DIR}/events/SDL_events_c.h"
    "${SRC_DIR}/events/SDL_gesture.c"
    "${SRC_DIR}/events/SDL_gesture_c.h"
    "${SRC_DIR}/events/SDL_keyboard.c"
    "${SRC_DIR}/events/SDL_keyboard_c.h"
    "${SRC_DIR}/events/SDL_mouse.c"
    "${SRC_DIR}/events/SDL_mouse_c.h"
    "${SRC_DIR}/events/SDL_quit.c"
    "${SRC_DIR}/events/SDL_sysevents.h"
    "${SRC_DIR}/events/SDL_touch.c"
    "${SRC_DIR}/events/SDL_touch_c.h"
    "${SRC_DIR}/events/SDL_windowevents.c"
    "${SRC_DIR}/events/SDL_windowevents_c.h"
    "${SRC_DIR}/file/SDL_rwops.c"
    "${SRC_DIR}/filesystem/dummy/SDL_sysfilesystem.c"
    "${SRC_DIR}/haptic/dummy/SDL_syshaptic.c"
    "${SRC_DIR}/haptic/SDL_haptic.c"
    "${SRC_DIR}/haptic/SDL_haptic_c.h"
    "${SRC_DIR}/haptic/SDL_syshaptic.h"
    "${SRC_DIR}/joystick/controller_type.h"
    "${SRC_DIR}/joystick/dummy/SDL_sysjoystick.c"
    "${SRC_DIR}/joystick/SDL_gamecontroller.c"
    "${SRC_DIR}/joystick/SDL_gamecontrollerdb.h"
    "${SRC_DIR}/joystick/SDL_joystick.c"
    "${SRC_DIR}/joystick/SDL_joystick_c.h"
    "${SRC_DIR}/joystick/SDL_sysjoystick.h"
    "${SRC_DIR}/joystick/steam/SDL_steamcontroller.c"
    "${SRC_DIR}/joystick/steam/SDL_steamcontroller.h"
    "${SRC_DIR}/joystick/windows/SDL_rawinputjoystick.c"
    "${SRC_DIR}/joystick/windows/SDL_rawinputjoystick_c.h"
    "${SRC_DIR}/locale/SDL_locale.c"
    "${SRC_DIR}/locale/SDL_syslocale.h"
    "${SRC_DIR}/misc/SDL_sysurl.h"
    "${SRC_DIR}/misc/SDL_url.c"
    "${SRC_DIR}/render/opengl/SDL_glfuncs.h"
    "${SRC_DIR}/render/opengl/SDL_render_gl.c"
    "${SRC_DIR}/render/opengl/SDL_shaders_gl.c"
    "${SRC_DIR}/render/opengl/SDL_shaders_gl.h"
    "${SRC_DIR}/render/SDL_render.c"
    "${SRC_DIR}/render/SDL_sysrender.h"
    "${SRC_DIR}/SDL.c"
    "${SRC_DIR}/SDL_assert.c"
    "${SRC_DIR}/SDL_assert_c.h"
    "${SRC_DIR}/SDL_dataqueue.c"
    "${SRC_DIR}/SDL_dataqueue.h"
    "${SRC_DIR}/SDL_error.c"
    "${SRC_DIR}/SDL_error_c.h"
    "${SRC_DIR}/SDL_hints.c"
    "${SRC_DIR}/SDL_internal.h"
    "${SRC_DIR}/SDL_log.c"
    "${SRC_DIR}/sensor/dummy/SDL_dummysensor.c"
    "${SRC_DIR}/sensor/dummy/SDL_dummysensor.h"
    "${SRC_DIR}/sensor/SDL_sensor.c"
    "${SRC_DIR}/sensor/SDL_sensor_c.h"
    "${SRC_DIR}/sensor/SDL_syssensor.h"
    "${SRC_DIR}/stdlib/SDL_crc32.c"
    "${SRC_DIR}/stdlib/SDL_getenv.c"
    "${SRC_DIR}/stdlib/SDL_iconv.c"
    "${SRC_DIR}/stdlib/SDL_malloc.c"
    "${SRC_DIR}/stdlib/SDL_qsort.c"
    "${SRC_DIR}/stdlib/SDL_stdlib.c"
    "${SRC_DIR}/stdlib/SDL_string.c"
    "${SRC_DIR}/stdlib/SDL_strtokr.c"
    "${SRC_DIR}/thread/SDL_systhread.h"
    "${SRC_DIR}/thread/SDL_thread.c"
    "${SRC_DIR}/thread/SDL_thread_c.h"
    "${SRC_DIR}/timer/SDL_timer.c"
    "${SRC_DIR}/timer/SDL_timer_c.h"
    "${SRC_DIR}/video/SDL_blit.c"
    "${SRC_DIR}/video/SDL_blit.h"
    "${SRC_DIR}/video/SDL_blit_copy.c"
    "${SRC_DIR}/video/SDL_blit_copy.h"
    "${SRC_DIR}/video/SDL_blit_slow.c"
    "${SRC_DIR}/video/SDL_blit_slow.h"
    "${SRC_DIR}/video/SDL_bmp.c"
    "${SRC_DIR}/video/SDL_clipboard.c"
    "${SRC_DIR}/video/SDL_fillrect.c"
    "${SRC_DIR}/video/SDL_pixels.c"
    "${SRC_DIR}/video/SDL_pixels_c.h"
    "${SRC_DIR}/video/SDL_rect.c"
    "${SRC_DIR}/video/SDL_rect_c.h"
    "${SRC_DIR}/video/SDL_RLEaccel.c"
    "${SRC_DIR}/video/SDL_RLEaccel_c.h"
    "${SRC_DIR}/video/SDL_shape.c"
    "${SRC_DIR}/video/SDL_shape_internals.h"
    "${SRC_DIR}/video/SDL_stretch.c"
    "${SRC_DIR}/video/SDL_surface.c"
    "${SRC_DIR}/video/SDL_sysvideo.h"
    "${SRC_DIR}/video/SDL_video.c"
    "${SRC_DIR}/video/SDL_yuv.c"
    "${SRC_DIR}/video/SDL_yuv_c.h"
)

set(OTHER_FILES
    "${SRC_DIR}/joystick/sort_controllers.py"
    "${SRC_DIR}/video/sdlgenblit.pl"
    "${SRC_DIR}/video/yuv2rgb/LICENSE"
    "${SRC_DIR}/video/yuv2rgb/README.md"
)

if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
    list(APPEND SOURCE_FILES
        "${SRC_DIR}/video/SDL_vulkan_internal.h"
        "${SRC_DIR}/video/SDL_vulkan_utils.c"
    )
endif()

if (PLATFORM_WINDOWS)
    list(APPEND SOURCE_FILES
        "${INCLUDE_DIR}/SDL_config_windows.h"
        "${SRC_DIR}/audio/wasapi/SDL_wasapi.c"
        "${SRC_DIR}/audio/wasapi/SDL_wasapi.h"
        "${SRC_DIR}/audio/wasapi/SDL_wasapi_win32.c"
        "${SRC_DIR}/core/windows/SDL_directx.h"
        "${SRC_DIR}/core/windows/SDL_windows.c"
        "${SRC_DIR}/core/windows/SDL_windows.h"
        "${SRC_DIR}/core/windows/SDL_xinput.c"
        "${SRC_DIR}/core/windows/SDL_xinput.h"
        "${SRC_DIR}/events/scancodes_windows.h"
        "${SRC_DIR}/filesystem/windows/SDL_sysfilesystem.c"
        "${SRC_DIR}/haptic/windows/SDL_dinputhaptic.c"
        "${SRC_DIR}/haptic/windows/SDL_dinputhaptic_c.h"
        "${SRC_DIR}/haptic/windows/SDL_windowshaptic.c"
        "${SRC_DIR}/haptic/windows/SDL_windowshaptic_c.h"
        "${SRC_DIR}/haptic/windows/SDL_xinputhaptic.c"
        "${SRC_DIR}/haptic/windows/SDL_xinputhaptic_c.h"
        "${SRC_DIR}/joystick/windows/SDL_dinputjoystick.c"
        "${SRC_DIR}/joystick/windows/SDL_dinputjoystick_c.h"
        "${SRC_DIR}/joystick/windows/SDL_windowsjoystick.c"
        "${SRC_DIR}/joystick/windows/SDL_windowsjoystick_c.h"
        "${SRC_DIR}/joystick/windows/SDL_xinputjoystick.c"
        "${SRC_DIR}/joystick/windows/SDL_xinputjoystick_c.h"
        "${SRC_DIR}/loadso/windows/SDL_sysloadso.c"
        "${SRC_DIR}/locale/windows/SDL_syslocale.c"
        "${SRC_DIR}/misc/windows/SDL_sysurl.c"
        "${SRC_DIR}/thread/generic/SDL_syscond.c"
        "${SRC_DIR}/thread/windows/SDL_sysmutex.c"
        "${SRC_DIR}/thread/windows/SDL_syssem.c"
        "${SRC_DIR}/thread/windows/SDL_systhread.c"
        "${SRC_DIR}/thread/windows/SDL_systhread_c.h"
        "${SRC_DIR}/thread/windows/SDL_systls.c"
        "${SRC_DIR}/timer/windows/SDL_systimer.c"
        "${SRC_DIR}/video/windows/SDL_msctf.h"
        "${SRC_DIR}/video/windows/SDL_vkeys.h"
        "${SRC_DIR}/video/windows/SDL_windowsclipboard.c"
        "${SRC_DIR}/video/windows/SDL_windowsclipboard.h"
        "${SRC_DIR}/video/windows/SDL_windowsevents.c"
        "${SRC_DIR}/video/windows/SDL_windowsevents.h"
        "${SRC_DIR}/video/windows/SDL_windowsframebuffer.c"
        "${SRC_DIR}/video/windows/SDL_windowsframebuffer.h"
        "${SRC_DIR}/video/windows/SDL_windowskeyboard.c"
        "${SRC_DIR}/video/windows/SDL_windowskeyboard.h"
        "${SRC_DIR}/video/windows/SDL_windowsmessagebox.c"
        "${SRC_DIR}/video/windows/SDL_windowsmessagebox.h"
        "${SRC_DIR}/video/windows/SDL_windowsmodes.c"
        "${SRC_DIR}/video/windows/SDL_windowsmodes.h"
        "${SRC_DIR}/video/windows/SDL_windowsmouse.c"
        "${SRC_DIR}/video/windows/SDL_windowsmouse.h"
        "${SRC_DIR}/video/windows/SDL_windowsopengl.c"
        "${SRC_DIR}/video/windows/SDL_windowsopengl.h"
        "${SRC_DIR}/video/windows/SDL_windowsshape.c"
        "${SRC_DIR}/video/windows/SDL_windowsshape.h"
        "${SRC_DIR}/video/windows/SDL_windowstaskdialog.h"
        "${SRC_DIR}/video/windows/SDL_windowsvideo.c"
        "${SRC_DIR}/video/windows/SDL_windowsvideo.h"
        "${SRC_DIR}/video/windows/SDL_windowswindow.c"
        "${SRC_DIR}/video/windows/SDL_windowswindow.h"
        "${SRC_DIR}/video/windows/wmmsg.h"
    )

    if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
        list(APPEND SOURCE_FILES
            "${SRC_DIR}/video/windows/SDL_windowsvulkan.c"
            "${SRC_DIR}/video/windows/SDL_windowsvulkan.h"
        )
    endif()
endif()

if (PLATFORM_MAC)
    list(APPEND SOURCE_FILES
        "${INCLUDE_DIR}/SDL_config_macosx.h"
        "${SRC_DIR}/audio/coreaudio/SDL_coreaudio.h"
        "${SRC_DIR}/audio/coreaudio/SDL_coreaudio.m"
        "${SRC_DIR}/events/scancodes_darwin.h"
        "${SRC_DIR}/file/cocoa/SDL_rwopsbundlesupport.h"
        "${SRC_DIR}/file/cocoa/SDL_rwopsbundlesupport.m"
        "${SRC_DIR}/filesystem/cocoa/SDL_sysfilesystem.m"
        "${SRC_DIR}/haptic/darwin/SDL_syshaptic.c"
        "${SRC_DIR}/haptic/darwin/SDL_syshaptic_c.h"
        "${SRC_DIR}/joystick/darwin/SDL_iokitjoystick.c"
        "${SRC_DIR}/joystick/darwin/SDL_iokitjoystick_c.h"
        "${SRC_DIR}/joystick/iphoneos/SDL_mfijoystick.m"
        "${SRC_DIR}/joystick/iphoneos/SDL_mfijoystick_c.h"
        "${SRC_DIR}/loadso/dlopen/SDL_sysloadso.c"
        "${SRC_DIR}/locale/macosx/SDL_syslocale.m"
        "${SRC_DIR}/misc/macosx/SDL_sysurl.m"
        "${SRC_DIR}/render/metal/SDL_render_metal.m"
        "${SRC_DIR}/render/metal/SDL_shaders_metal_osx.h"
        "${SRC_DIR}/thread/pthread/SDL_syscond.c"
        "${SRC_DIR}/thread/pthread/SDL_sysmutex.c"
        "${SRC_DIR}/thread/pthread/SDL_sysmutex_c.h"
        "${SRC_DIR}/thread/pthread/SDL_syssem.c"
        "${SRC_DIR}/thread/pthread/SDL_systhread.c"
        "${SRC_DIR}/thread/pthread/SDL_systhread_c.h"
        "${SRC_DIR}/thread/pthread/SDL_systls.c"
        "${SRC_DIR}/timer/unix/SDL_systimer.c"
        "${SRC_DIR}/video/cocoa/SDL_cocoaclipboard.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoaclipboard.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoaevents.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoaevents.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoakeyboard.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoakeyboard.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoamessagebox.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoamessagebox.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoametalview.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoametalview.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoamodes.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoamodes.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoamouse.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoamouse.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoamousetap.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoamousetap.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoashape.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoashape.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoavideo.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoavideo.m"
        "${SRC_DIR}/video/cocoa/SDL_cocoawindow.h"
        "${SRC_DIR}/video/cocoa/SDL_cocoawindow.m"
    )

    if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
        list(APPEND SOURCE_FILES
            "${SRC_DIR}/video/cocoa/SDL_cocoavulkan.h"
            "${SRC_DIR}/video/cocoa/SDL_cocoavulkan.m"
        )
    endif()

    list(APPEND OTHER_FILES
        "${SRC_DIR}/render/metal/SDL_shaders_metal.metal"
    )
endif()

if (PLATFORM_LINUX)
    list(APPEND SOURCE_FILES
        "${INCLUDE_DIR}/SDL_config_linux.h"
        "${SRC_DIR}/audio/pulseaudio/SDL_pulseaudio.c"
        "${SRC_DIR}/audio/pulseaudio/SDL_pulseaudio.h"
        "${SRC_DIR}/core/linux/SDL_dbus.c"
        "${SRC_DIR}/core/linux/SDL_dbus.h"
        "${SRC_DIR}/core/linux/SDL_evdev.c"
        "${SRC_DIR}/core/linux/SDL_evdev.h"
        "${SRC_DIR}/core/linux/SDL_evdev_capabilities.c"
        "${SRC_DIR}/core/linux/SDL_evdev_capabilities.h"
        "${SRC_DIR}/core/linux/SDL_evdev_kbd.c"
        "${SRC_DIR}/core/linux/SDL_evdev_kbd.h"
        "${SRC_DIR}/core/linux/SDL_evdev_kbd_default_accents.h"
        "${SRC_DIR}/core/linux/SDL_evdev_kbd_default_keymap.h"
        "${SRC_DIR}/core/linux/SDL_ibus.c"
        "${SRC_DIR}/core/linux/SDL_ibus.h"
        "${SRC_DIR}/core/linux/SDL_ime.c"
        "${SRC_DIR}/core/linux/SDL_ime.h"
        "${SRC_DIR}/core/linux/SDL_threadprio.c"
        "${SRC_DIR}/core/linux/SDL_udev.c"
        "${SRC_DIR}/core/linux/SDL_udev.h"
        "${SRC_DIR}/core/unix/SDL_poll.c"
        "${SRC_DIR}/core/unix/SDL_poll.h"
        "${SRC_DIR}/filesystem/unix/SDL_sysfilesystem.c"
        "${SRC_DIR}/haptic/linux/SDL_syshaptic.c"
        "${SRC_DIR}/joystick/linux/SDL_sysjoystick.c"
        "${SRC_DIR}/joystick/linux/SDL_sysjoystick_c.h"
        "${SRC_DIR}/loadso/dlopen/SDL_sysloadso.c"
        "${SRC_DIR}/locale/unix/SDL_syslocale.c"
        "${SRC_DIR}/misc/unix/SDL_sysurl.c"
        "${SRC_DIR}/thread/pthread/SDL_syscond.c"
        "${SRC_DIR}/thread/pthread/SDL_sysmutex.c"
        "${SRC_DIR}/thread/pthread/SDL_sysmutex_c.h"
        "${SRC_DIR}/thread/pthread/SDL_syssem.c"
        "${SRC_DIR}/thread/pthread/SDL_systhread.c"
        "${SRC_DIR}/thread/pthread/SDL_systhread_c.h"
        "${SRC_DIR}/thread/pthread/SDL_systls.c"
        "${SRC_DIR}/timer/unix/SDL_systimer.c"
        "${SRC_DIR}/video/x11/edid-parse.c"
        "${SRC_DIR}/video/x11/edid.h"
        "${SRC_DIR}/video/x11/SDL_x11clipboard.c"
        "${SRC_DIR}/video/x11/SDL_x11clipboard.h"
        "${SRC_DIR}/video/x11/SDL_x11dyn.c"
        "${SRC_DIR}/video/x11/SDL_x11dyn.h"
        "${SRC_DIR}/video/x11/SDL_x11events.c"
        "${SRC_DIR}/video/x11/SDL_x11events.h"
        "${SRC_DIR}/video/x11/SDL_x11framebuffer.c"
        "${SRC_DIR}/video/x11/SDL_x11framebuffer.h"
        "${SRC_DIR}/video/x11/SDL_x11keyboard.c"
        "${SRC_DIR}/video/x11/SDL_x11keyboard.h"
        "${SRC_DIR}/video/x11/SDL_x11messagebox.c"
        "${SRC_DIR}/video/x11/SDL_x11messagebox.h"
        "${SRC_DIR}/video/x11/SDL_x11modes.c"
        "${SRC_DIR}/video/x11/SDL_x11modes.h"
        "${SRC_DIR}/video/x11/SDL_x11mouse.c"
        "${SRC_DIR}/video/x11/SDL_x11mouse.h"
        "${SRC_DIR}/video/x11/SDL_x11opengl.c"
        "${SRC_DIR}/video/x11/SDL_x11opengl.h"
        "${SRC_DIR}/video/x11/SDL_x11opengles.c"
        "${SRC_DIR}/video/x11/SDL_x11opengles.h"
        "${SRC_DIR}/video/x11/SDL_x11shape.c"
        "${SRC_DIR}/video/x11/SDL_x11shape.h"
        "${SRC_DIR}/video/x11/SDL_x11sym.h"
        "${SRC_DIR}/video/x11/SDL_x11touch.c"
        "${SRC_DIR}/video/x11/SDL_x11touch.h"
        "${SRC_DIR}/video/x11/SDL_x11video.c"
        "${SRC_DIR}/video/x11/SDL_x11video.h"
        "${SRC_DIR}/video/x11/SDL_x11vulkan.c"
        "${SRC_DIR}/video/x11/SDL_x11vulkan.h"
        "${SRC_DIR}/video/x11/SDL_x11window.c"
        "${SRC_DIR}/video/x11/SDL_x11window.h"
        "${SRC_DIR}/video/x11/SDL_x11xfixes.c"
        "${SRC_DIR}/video/x11/SDL_x11xfixes.h"
        "${SRC_DIR}/video/x11/SDL_x11xinput2.c"
        "${SRC_DIR}/video/x11/SDL_x11xinput2.h"
    )
endif()

add_library(${LIBSDL_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

include_directories(${INCLUDE_PATHS})
target_include_directories(${LIBSDL_TGT_NAME} INTERFACE ${EXPORTED_INCLUDE_PATHS})
target_compile_definitions(${LIBSDL_TGT_NAME} PUBLIC -DSDL_LEAN_AND_MEAN=1)

if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
    target_compile_definitions(${LIBSDL_TGT_NAME} PUBLIC -DPSYDOOM_VULKAN_RENDERER=1 -DVK_NO_PROTOTYPES)
else()
    target_compile_definitions(${LIBSDL_TGT_NAME} PUBLIC -DPSYDOOM_VULKAN_RENDERER=0)
endif()

if (PLATFORM_WINDOWS)
    target_compile_definitions(${LIBSDL_TGT_NAME} PUBLIC SDL_DISABLE_WINDOWS_IME)
    target_compile_definitions(${LIBSDL_TGT_NAME} PUBLIC HAVE_LIBC)     # Use standard C math libraries etc.

    target_link_libraries(${LIBSDL_TGT_NAME}
        Imm32.lib
        User32.lib
        Version.lib
        Winmm.lib
    )
endif()

if (PLATFORM_MAC)
    FIND_LIBRARY(APP_KIT_FRAMEWORK              AppKit)
    FIND_LIBRARY(AUDIO_TOOLBOX_FRAMEWORK        AudioToolbox)
    FIND_LIBRARY(CARBON_FRAMEWORK               Carbon)
    FIND_LIBRARY(COCOA_FRAMEWORK                Cocoa)
    FIND_LIBRARY(CORE_AUDIO_FRAMEWORK           CoreAudio)
    FIND_LIBRARY(CORE_FOUNDATION_FRAMEWORK      CoreFoundation)
    FIND_LIBRARY(CORE_HAPTICS_FRAMEWORK         CoreHaptics)
    FIND_LIBRARY(CORE_VIDEO_FRAMEWORK           CoreVideo)
    FIND_LIBRARY(FORCE_FEEDBACK_FRAMEWORK       ForceFeedback)
    FIND_LIBRARY(GAME_CONTROLLER_FRAMEWORK      GameController)
    FIND_LIBRARY(IO_KIT_FRAMEWORK               IOKit)
    FIND_LIBRARY(METAL_FRAMEWORK                Metal)

    target_link_libraries(${LIBSDL_TGT_NAME}
        ${APP_KIT_FRAMEWORK}
        ${AUDIO_TOOLBOX_FRAMEWORK}
        ${CARBON_FRAMEWORK}
        ${COCOA_FRAMEWORK}
        ${CORE_AUDIO_FRAMEWORK}
        ${CORE_FOUNDATION_FRAMEWORK}
        ${CORE_HAPTICS_FRAMEWORK}
        ${CORE_VIDEO_FRAMEWORK}
        ${FORCE_FEEDBACK_FRAMEWORK}
        ${GAME_CONTROLLER_FRAMEWORK}
        ${IO_KIT_FRAMEWORK}
        ${METAL_FRAMEWORK}
    )

    target_compile_definitions(${LIBSDL_TGT_NAME} PUBLIC -DSDL_VIDEO_METAL=1)           # Enable Metal Video backend
    target_compile_options(${LIBSDL_TGT_NAME} PRIVATE -fno-objc-arc)                    # ARC must be disabled for SDL
    target_compile_options(${LIBSDL_TGT_NAME} PRIVATE -Wno-deprecated-declarations)     # Ignore deprecation warnings
    
    if (PSYDOOM_INCLUDE_VULKAN_RENDERER)
        target_compile_definitions(${LIBSDL_TGT_NAME} PUBLIC -DVK_USE_PLATFORM_METAL_EXT=1)
    endif()
endif()

if (PLATFORM_LINUX)
    include_directories("${BASE_DIR}/wayland-protocols")

    target_link_libraries(${LIBSDL_TGT_NAME}
        dl
        pthread
        pulse
        X11
        Xcursor
        Xext
        Xfixes
        Xi
        Xinerama
        Xrandr
        Xxf86vm
    )

    target_compile_options(${LIBSDL_TGT_NAME} PRIVATE -Wno-deprecated-declarations)     # No warnings about using deprecated library functions
endif()
