set(BASE_DIR        "mednafen")
set(SRC_DIR         "${BASE_DIR}/src")
set(INCLUDE_DIR     "${BASE_DIR}/include")

set(INCLUDE_PATHS
    "${INCLUDE_DIR}"
)

set(EXPORTED_INCLUDE_PATHS
    "${INCLUDE_DIR}"
)

# Platform agnostic sources
set(SOURCE_FILES
    "${INCLUDE_DIR}/mednafen/cdrom/CDAccess.h"
    "${INCLUDE_DIR}/mednafen/cdrom/CDInterface.h"
    "${INCLUDE_DIR}/mednafen/cdrom/CDUtility.h"
    "${INCLUDE_DIR}/mednafen/cdrom/SimpleFIFO.h"
    "${INCLUDE_DIR}/mednafen/cheat_formats/psx.h"
    "${INCLUDE_DIR}/mednafen/driver.h"
    "${INCLUDE_DIR}/mednafen/endian.h"
    "${INCLUDE_DIR}/mednafen/error.h"
    "${INCLUDE_DIR}/mednafen/file.h"
    "${INCLUDE_DIR}/mednafen/FileStream.h"
    "${INCLUDE_DIR}/mednafen/general.h"
    "${INCLUDE_DIR}/mednafen/git.h"
    "${INCLUDE_DIR}/mednafen/hash/crc.h"
    "${INCLUDE_DIR}/mednafen/hash/md5.h"
    "${INCLUDE_DIR}/mednafen/hash/sha1.h"
    "${INCLUDE_DIR}/mednafen/hash/sha256.h"
    "${INCLUDE_DIR}/mednafen/math_ops.h"
    "${INCLUDE_DIR}/mednafen/mednafen-driver.h"
    "${INCLUDE_DIR}/mednafen/mednafen.h"
    "${INCLUDE_DIR}/mednafen/MemoryStream.h"
    "${INCLUDE_DIR}/mednafen/mempatcher-driver.h"
    "${INCLUDE_DIR}/mednafen/mempatcher.h"
    "${INCLUDE_DIR}/mednafen/movie-driver.h"
    "${INCLUDE_DIR}/mednafen/MThreading.h"
    "${INCLUDE_DIR}/mednafen/NativeVFS.h"
    "${INCLUDE_DIR}/mednafen/netplay-driver.h"
    "${INCLUDE_DIR}/mednafen/player.h"
    "${INCLUDE_DIR}/mednafen/PSFLoader.h"
    "${INCLUDE_DIR}/mednafen/psx/psx.h"
    "${INCLUDE_DIR}/mednafen/resampler/resampler.h"
    "${INCLUDE_DIR}/mednafen/settings-common.h"
    "${INCLUDE_DIR}/mednafen/settings-driver.h"
    "${INCLUDE_DIR}/mednafen/settings.h"
    "${INCLUDE_DIR}/mednafen/sound/fir_blargg_common.h"
    "${INCLUDE_DIR}/mednafen/sound/fir_blargg_config.h"
    "${INCLUDE_DIR}/mednafen/sound/Fir_Resampler.h"
    "${INCLUDE_DIR}/mednafen/sound/WAVRecord.h"
    "${INCLUDE_DIR}/mednafen/state-common.h"
    "${INCLUDE_DIR}/mednafen/state-driver.h"
    "${INCLUDE_DIR}/mednafen/state.h"
    "${INCLUDE_DIR}/mednafen/Stream.h"
    "${INCLUDE_DIR}/mednafen/string/escape.h"
    "${INCLUDE_DIR}/mednafen/string/string.h"
    "${INCLUDE_DIR}/mednafen/time.h"
    "${INCLUDE_DIR}/mednafen/types.h"
    "${INCLUDE_DIR}/mednafen/video-driver.h"
    "${INCLUDE_DIR}/mednafen/video.h"
    "${INCLUDE_DIR}/mednafen/video/primitives.h"
    "${INCLUDE_DIR}/mednafen/video/surface.h"
    "${INCLUDE_DIR}/mednafen/video/text.h"
    "${INCLUDE_DIR}/mednafen/VirtualFS.h"
    "${INCLUDE_DIR}/mednafen/win32-common.h"
    "${INCLUDE_DIR}/minilzo/minilzo.h"
    "${INCLUDE_DIR}/trio/trio.h"
    "${INCLUDE_DIR}/trio/triodef.h"
    "${INCLUDE_DIR}/trio/trionan.h"
    "${INCLUDE_DIR}/trio/triop.h"
    "${INCLUDE_DIR}/trio/triostr.h"
    "${SRC_DIR}/cdrom/CDAccess.cpp"
    "${SRC_DIR}/cdrom/CDInterface.cpp"
    "${SRC_DIR}/cdrom/CDInterface_MT.cpp"
    "${SRC_DIR}/cdrom/CDInterface_MT.h"
    "${SRC_DIR}/cdrom/CDInterface_ST.cpp"
    "${SRC_DIR}/cdrom/CDInterface_ST.h"
    "${SRC_DIR}/cdrom/CDUtility.cpp"
    "${SRC_DIR}/compress/GZFileStream.cpp"
    "${SRC_DIR}/compress/GZFileStream.h"
    "${SRC_DIR}/compress/ZIPReader.cpp"
    "${SRC_DIR}/compress/ZIPReader.h"
    "${SRC_DIR}/compress/ZLInflateFilter.cpp"
    "${SRC_DIR}/compress/ZLInflateFilter.h"
    "${SRC_DIR}/drivers/main.cpp"
    "${SRC_DIR}/error.cpp"
    "${SRC_DIR}/FileStream.cpp"
    "${SRC_DIR}/general.cpp"
    "${SRC_DIR}/hash/crc.cpp"
    "${SRC_DIR}/hash/md5.cpp"
    "${SRC_DIR}/hash/sha1.cpp"
    "${SRC_DIR}/hash/sha256.cpp"
    "${SRC_DIR}/mednafen.cpp"
    "${SRC_DIR}/MemoryStream.cpp"
    "${SRC_DIR}/minilzo/minilzo.c"
    "${SRC_DIR}/NativeVFS.cpp"
    "${SRC_DIR}/psx/cdc.cpp"
    "${SRC_DIR}/psx/cdc.h"
    "${SRC_DIR}/psx/cpu.cpp"
    "${SRC_DIR}/psx/cpu.h"
    "${SRC_DIR}/psx/debug.cpp"
    "${SRC_DIR}/psx/debug.h"
    "${SRC_DIR}/psx/dis.cpp"
    "${SRC_DIR}/psx/dis.h"
    "${SRC_DIR}/psx/dma.cpp"
    "${SRC_DIR}/psx/dma.h"
    "${SRC_DIR}/psx/FastFIFO.h"
    "${SRC_DIR}/psx/frontio.cpp"
    "${SRC_DIR}/psx/frontio.h"
    "${SRC_DIR}/psx/gpu.cpp"
    "${SRC_DIR}/psx/gpu.h"
    "${SRC_DIR}/psx/gpu_common.inc"
    "${SRC_DIR}/psx/gpu_line.cpp"
    "${SRC_DIR}/psx/gpu_polygon.cpp"
    "${SRC_DIR}/psx/gpu_sprite.cpp"
    "${SRC_DIR}/psx/gte.cpp"
    "${SRC_DIR}/psx/gte.h"
    "${SRC_DIR}/psx/input"
    "${SRC_DIR}/psx/input/dualanalog.cpp"
    "${SRC_DIR}/psx/input/dualanalog.h"
    "${SRC_DIR}/psx/input/dualshock.cpp"
    "${SRC_DIR}/psx/input/dualshock.h"
    "${SRC_DIR}/psx/input/gamepad.cpp"
    "${SRC_DIR}/psx/input/gamepad.h"
    "${SRC_DIR}/psx/input/guncon.cpp"
    "${SRC_DIR}/psx/input/guncon.h"
    "${SRC_DIR}/psx/input/justifier.cpp"
    "${SRC_DIR}/psx/input/justifier.h"
    "${SRC_DIR}/psx/input/memcard.cpp"
    "${SRC_DIR}/psx/input/memcard.h"
    "${SRC_DIR}/psx/input/mouse.cpp"
    "${SRC_DIR}/psx/input/mouse.h"
    "${SRC_DIR}/psx/input/multitap.cpp"
    "${SRC_DIR}/psx/input/multitap.h"
    "${SRC_DIR}/psx/input/negcon.cpp"
    "${SRC_DIR}/psx/input/negcon.h"
    "${SRC_DIR}/psx/irq.cpp"
    "${SRC_DIR}/psx/irq.h"
    "${SRC_DIR}/psx/masmem.h"
    "${SRC_DIR}/psx/mdec.cpp"
    "${SRC_DIR}/psx/mdec.h"
    "${SRC_DIR}/psx/psx.cpp"
    "${SRC_DIR}/psx/psx.h"
    "${SRC_DIR}/psx/sio.cpp"
    "${SRC_DIR}/psx/sio.h"
    "${SRC_DIR}/psx/spu.cpp"
    "${SRC_DIR}/psx/spu.h"
    "${SRC_DIR}/psx/spu_fir_table.inc"
    "${SRC_DIR}/psx/spu_reverb.inc"
    "${SRC_DIR}/psx/timer.cpp"
    "${SRC_DIR}/psx/timer.h"
    "${SRC_DIR}/settings.cpp"
    "${SRC_DIR}/sound/Fir_Resampler.cpp"
    "${SRC_DIR}/sound/WAVRecord.cpp"
    "${SRC_DIR}/state.cpp"
    "${SRC_DIR}/Stream.cpp"
    "${SRC_DIR}/string/escape.cpp"
    "${SRC_DIR}/string/string.cpp"
    "${SRC_DIR}/Time.cpp"
    "${SRC_DIR}/trio/trio.c"
    "${SRC_DIR}/trio/trionan.c"
    "${SRC_DIR}/trio/triostr.c"
    "${SRC_DIR}/VirtualFS.cpp"
)

set(OTHER_FILES
)

# Platform specific sources
if (PLATFORM_WINDOWS)
    list(APPEND SOURCE_FILES 
        "${SRC_DIR}/mthreading/MThreading_Win32.cpp"
    )
else()
    list(APPEND SOURCE_FILES 
        "${SRC_DIR}/mthreading/MThreading_POSIX.cpp"
    )
endif()

# Generate the library
add_library(${MEDNAFEN_TGT_NAME} ${SOURCE_FILES} ${OTHER_FILES})
setup_source_groups("${SOURCE_FILES}" "${OTHER_FILES}")

# Dependencies
target_link_libraries(${MEDNAFEN_TGT_NAME} ${ZLIB_TGT_NAME})

# Setup include dirs
include_directories(${INCLUDE_PATHS})
target_include_directories(${MEDNAFEN_TGT_NAME} INTERFACE ${EXPORTED_INCLUDE_PATHS})

# Need to define endianess for mednafen.
# Assuming a little endian system for now, if we need to support big then this needs to change based on that.
target_compile_definitions(${MEDNAFEN_TGT_NAME} PUBLIC LSB_FIRST)

# Only need the playstation emulator
target_compile_definitions(${MEDNAFEN_TGT_NAME} PRIVATE WANT_PSX_EMU)

# Mednafen version info
target_compile_definitions(${MEDNAFEN_TGT_NAME} PUBLIC -DMEDNAFEN_VERSION="1.22.2")
target_compile_definitions(${MEDNAFEN_TGT_NAME} PUBLIC -DMEDNAFEN_VERSION_NUMERIC=0x00102202)

# Platform and compiler specific settings
if (COMPILER_MSVC)
    target_compile_options(${MEDNAFEN_TGT_NAME} PRIVATE /W1)    # Very limited set of warnings
endif()

if (PLATFORM_WINDOWS)
    target_compile_options(${MEDNAFEN_TGT_NAME} PUBLIC -DPSS_STYLE=2)   # Use back slash path separator
else()
    target_compile_options(${MEDNAFEN_TGT_NAME} PUBLIC -DPSS_STYLE=1)   # Use forward slash path separator
endif()
