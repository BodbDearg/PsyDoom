#include "ConfigSerialization_Audio.h"

#include "Config.h"
#include "IniUtils.h"

#include <string>

BEGIN_NAMESPACE(ConfigSerialization)

using namespace Config;

//------------------------------------------------------------------------------------------------------------------------------------------
// Config field storage
//------------------------------------------------------------------------------------------------------------------------------------------
Config_Audio gConfig_Audio = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the config serializers for audio related config
//------------------------------------------------------------------------------------------------------------------------------------------
void initCfgSerialization_Audio() noexcept {
    auto& cfg = gConfig_Audio;

    cfg.audioBufferSize = makeConfigField(
        "AudioBufferSize",
        "Audio buffer size, in 44.1 KHz sound samples.\n"
        "Lower values reduce sound latency and improve music timing precision.\n"
        "\n"
        "Setting the buffer size too low however may cause audio instability or stutter on some systems.\n"
        "If set to '0' (auto) then PsyDoom will use a default value, which is '256' samples currently.\n"
        "Mostly this setting can be left alone but if you are experiencing sound issues, try adjusting.\n"
        "\n"
        "Some example values and their corresponding added sound latency (MS):\n"
        " 64   = ~1.45 MS\n"
        " 128  = ~2.9 MS\n"
        " 256  = ~5.8 MS\n"
        " 512  = ~11.6 MS",
        gAudioBufferSize,
        0
    );

    cfg.spuRamSize = makeConfigField(
        "SpuRamSize",
        "The size of available SPU RAM for loading sounds and sampled music instruments, in bytes.\n"
        "\n"
        "If <= 0 then PsyDoom will auto-configure the amount to 16 MiB, which is 32x the original 512 KiB\n"
        "that the PlayStation allowed. This greatly extended limit fixes a few bugs with sounds not playing\n"
        "on some maps and provides ample room for custom user maps and music.\n"
        "\n"
        "Note that values lower than 512 KiB will be ignored for compatibility reasons. This setting also\n"
        "has no effect if you build PsyDoom without limit removing features.",
        gSpuRamSize,
        -1
    );
}

END_NAMESPACE(ConfigSerialization)
