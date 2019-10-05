#include <mednafen/cdrom/CDInterface.h>
#include <mednafen/NativeVFS.h>
#include <mednafen/psx/psx.h>
#include <mednafen/settings.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>

static const MDFNSetting MednafenSettings[] = {
    { "cd.image_memcache", MDFNSF_NOFLAGS, gettext_noop("Cache entire CD images in memory."), gettext_noop("Reads the entire CD image(s) into memory at startup(which will cause a small delay).  Can help obviate emulation hiccups due to emulated CD access.  May cause more harm than good on low memory systems, systems with swap enabled, and/or when the disc images in question are on a fast SSD."), MDFNST_BOOL, "0" },
    { "cheats", MDFNSF_NOFLAGS, "Enable cheats.", NULL, MDFNST_BOOL, "1" },
    { "filesys.fname_movie", MDFNSF_CAT_PATH, gettext_noop("Format string for movie filename."), "", MDFNST_STRING, "%f.%M%p.%x" },
    { "filesys.fname_sav", MDFNSF_CAT_PATH, gettext_noop("Format string for save games filename."), gettext_noop("WARNING: %x should always be included, otherwise you run the risk of overwriting save data for games that create multiple save data files.\n\nSee fname_format.txt for more information.  Edit at your own risk."), MDFNST_STRING, "%f.%M%x" },
    { "filesys.fname_savbackup", MDFNSF_CAT_PATH, gettext_noop("Format string for save game backups filename."), gettext_noop("WARNING: %x and %p should always be included.\n\nSee fname_format.txt for more information.  Edit at your own risk."), MDFNST_STRING, "%f.%m%z%p.%x" },
    { "filesys.fname_snap", MDFNSF_CAT_PATH, gettext_noop("Format string for screen snapshot filenames."), gettext_noop("WARNING: %x or %p should always be included, otherwise there will be a conflict between the numeric counter text file and the image data file.\n\nSee fname_format.txt for more information.  Edit at your own risk."), MDFNST_STRING, "%f-%p.%x" },
    { "filesys.fname_state", MDFNSF_CAT_PATH, gettext_noop("Format string for state filename."), "", MDFNST_STRING, "%f.%M%X" /*"%F.%M%p.%x"*/ },
    { "filesys.path_cheat", MDFNSF_CAT_PATH, gettext_noop("Path to directory for cheats."), NULL, MDFNST_STRING, "cheats" },
    { "filesys.path_firmware", MDFNSF_CAT_PATH, gettext_noop("Path to directory for firmware."), NULL, MDFNST_STRING, "E:\\Darragh\\Desktop\\Doom\\PSX DOOM\\Doom [U] [SLUS-00077]\\" },
    { "filesys.path_movie", MDFNSF_CAT_PATH, gettext_noop("Path to directory for movies."), NULL, MDFNST_STRING, "mcm" },
    { "filesys.path_palette", MDFNSF_CAT_PATH, gettext_noop("Path to directory for custom palettes."), NULL, MDFNST_STRING, "palettes" },
    { "filesys.path_pgconfig", MDFNSF_CAT_PATH, gettext_noop("Path to directory for per-game configuration override files."), NULL, MDFNST_STRING, "pgconfig" },
    { "filesys.path_sav", MDFNSF_CAT_PATH, gettext_noop("Path to directory for save games and nonvolatile memory."), gettext_noop("WARNING: Do not set this path to a directory that contains Famicom Disk System disk images, or you will corrupt them when you load an FDS game and exit Mednafen."), MDFNST_STRING, "sav" },
    { "filesys.path_savbackup", MDFNSF_CAT_PATH, gettext_noop("Path to directory for backups of save games and nonvolatile memory."), NULL, MDFNST_STRING, "b" },
    { "filesys.path_snap", MDFNSF_CAT_PATH, gettext_noop("Path to directory for screen snapshots."), NULL, MDFNST_STRING, "snaps" },
    { "filesys.path_state", MDFNSF_CAT_PATH, gettext_noop("Path to directory for save states."), NULL, MDFNST_STRING, "mcs" },
    { "filesys.untrusted_fip_check", MDFNSF_NOFLAGS, gettext_noop("Enable untrusted file-inclusion path security check."), gettext_noop("When this setting is set to \"1\", the default, paths to files referenced from files like CUE sheets and PSF rips are checked for certain characters that can be used in directory traversal, and if found, loading is aborted.  Set it to \"0\" if you want to allow constructs like absolute paths in CUE sheets, but only if you understand the security implications of doing so(see \"Security Issues\" section in the documentation)."), MDFNST_BOOL, "1" },
#if PSX_DBGPRINT_ENABLE
    { "psx.dbg_level", MDFNSF_NOFLAGS, gettext_noop("Debug printf verbosity level."), NULL, MDFNST_UINT, "0", "0", "4" },
#endif
    { "psx.dbg_exe_cdpath", MDFNSF_SUPPRESS_DOC | MDFNSF_CAT_PATH, gettext_noop("CD image to use with .PSX/.EXE loading."), NULL, MDFNST_STRING, "" },
    { "psx.input.mouse_sensitivity", MDFNSF_NOFLAGS, gettext_noop("Emulated mouse sensitivity."), NULL, MDFNST_FLOAT, "1.00", NULL, NULL },
    { "psx.input.analog_mode_ct", MDFNSF_NOFLAGS, gettext_noop("Enable analog mode combo-button alternate toggle."), gettext_noop("When enabled, instead of the configured Analog mode toggle button for the emulated DualShock, use a combination of buttons held down for one emulated second to toggle it instead.  The specific combination is controlled via the \"psx.input.analog_mode_ct.compare\" setting, which by default is Select, Start, and all four shoulder buttons."), MDFNST_BOOL, "0", NULL, NULL },
    { "psx.input.analog_mode_ct.compare", MDFNSF_NOFLAGS, gettext_noop("Compare value for analog mode combo-button alternate toggle."), gettext_noop("0x0001=SELECT\n0x0002=L3\n0x0004=R3\n0x0008=START\n0x0010=D-Pad UP\n0x0020=D-Pad Right\n0x0040=D-Pad Down\n0x0080=D-Pad Left\n0x0100=L2\n0x0200=R2\n0x0400=L1\n0x0800=R1\n0x1000=△\n0x2000=○\n0x4000=x\n0x8000=□"), MDFNST_UINT, "0x0F09", "0x0000", "0xFFFF" },
    { "psx.input.pport1.multitap", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable multitap on PSX port 1."), gettext_noop("Makes 3 more virtual ports available.\n\nNOTE: Enabling multitap in games that don't fully support it may cause deleterious effects."), MDFNST_BOOL, "0", NULL, NULL }, //MDFNST_ENUM, "auto", NULL, NULL, NULL, NULL, MultiTap_List },
    { "psx.input.pport2.multitap", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable multitap on PSX port 2."), gettext_noop("Makes 3 more virtual ports available.\n\nNOTE: Enabling multitap in games that don't fully support it may cause deleterious effects."), MDFNST_BOOL, "0", NULL, NULL },
    { "psx.input.port1.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memory card on virtual port 1."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
    { "psx.input.port2.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memory card on virtual port 2."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
    { "psx.input.port3.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memory card on virtual port 3."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
    { "psx.input.port4.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memory card on virtual port 4."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
    { "psx.input.port5.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memory card on virtual port 5."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
    { "psx.input.port6.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memory card on virtual port 6."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
    { "psx.input.port7.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memory card on virtual port 7."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
    { "psx.input.port8.memcard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate memory card on virtual port 8."), NULL, MDFNST_BOOL, "1", NULL, NULL, },
    { "psx.input.port1.gun_chairs", MDFNSF_NOFLAGS, gettext_noop("Crosshairs color for lightgun on virtual port 1."), gettext_noop("A value of 0x1000000 disables crosshair drawing."), MDFNST_UINT, "0xFF0000", "0x000000", "0x1000000" },
    { "psx.input.port2.gun_chairs", MDFNSF_NOFLAGS, gettext_noop("Crosshairs color for lightgun on virtual port 2."), gettext_noop("A value of 0x1000000 disables crosshair drawing."), MDFNST_UINT, "0x00FF00", "0x000000", "0x1000000" },
    { "psx.input.port3.gun_chairs", MDFNSF_NOFLAGS, gettext_noop("Crosshairs color for lightgun on virtual port 3."), gettext_noop("A value of 0x1000000 disables crosshair drawing."), MDFNST_UINT, "0xFF00FF", "0x000000", "0x1000000" },
    { "psx.input.port4.gun_chairs", MDFNSF_NOFLAGS, gettext_noop("Crosshairs color for lightgun on virtual port 4."), gettext_noop("A value of 0x1000000 disables crosshair drawing."), MDFNST_UINT, "0xFF8000", "0x000000", "0x1000000" },
    { "psx.input.port5.gun_chairs", MDFNSF_NOFLAGS, gettext_noop("Crosshairs color for lightgun on virtual port 5."), gettext_noop("A value of 0x1000000 disables crosshair drawing."), MDFNST_UINT, "0xFFFF00", "0x000000", "0x1000000" },
    { "psx.input.port6.gun_chairs", MDFNSF_NOFLAGS, gettext_noop("Crosshairs color for lightgun on virtual port 6."), gettext_noop("A value of 0x1000000 disables crosshair drawing."), MDFNST_UINT, "0x00FFFF", "0x000000", "0x1000000" },
    { "psx.input.port7.gun_chairs", MDFNSF_NOFLAGS, gettext_noop("Crosshairs color for lightgun on virtual port 7."), gettext_noop("A value of 0x1000000 disables crosshair drawing."), MDFNST_UINT, "0x0080FF", "0x000000", "0x1000000" },
    { "psx.input.port8.gun_chairs", MDFNSF_NOFLAGS, gettext_noop("Crosshairs color for lightgun on virtual port 8."), gettext_noop("A value of 0x1000000 disables crosshair drawing."), MDFNST_UINT, "0x8000FF", "0x000000", "0x1000000" },
    { "psx.region_autodetect", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Attempt to auto-detect region of game."), NULL, MDFNST_BOOL, "1" },
    { "psx.region_default", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Default region to use."), gettext_noop("Used if region autodetection fails or is disabled."), MDFNST_INT, "1" },
    { "psx.bios_jp", MDFNSF_EMU_STATE | MDFNSF_CAT_PATH, gettext_noop("Path to the Japan SCPH-5500/v3.0J ROM BIOS"), gettext_noop("SHA-256 9c0421858e217805f4abe18698afea8d5aa36ff0727eb8484944e00eb5e7eadb"), MDFNST_STRING, "scph5500.bin" },
    { "psx.bios_na", MDFNSF_EMU_STATE | MDFNSF_CAT_PATH, gettext_noop("Path to the North America SCPH-5501/v3.0A ROM BIOS"), gettext_noop("SHA-256 11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef"), MDFNST_STRING, "scph5501.bin" },
    { "psx.bios_eu", MDFNSF_EMU_STATE | MDFNSF_CAT_PATH, gettext_noop("Path to the Europe SCPH-5502/v3.0E ROM BIOS"), gettext_noop("SHA-256 1faaa18fa820a0225e488d9f086296b8e6c46df739666093987ff7d8fd352c09"), MDFNST_STRING, "scph5502.bin" },
    { "psx.bios_sanity", MDFNSF_NOFLAGS, gettext_noop("Enable BIOS ROM image sanity checks."), gettext_noop("Enables blacklisting of known bad BIOS dumps and known BIOS versions that don't match the region of the hardware being emulated.") , MDFNST_BOOL, "1" },
    { "psx.cd_sanity", MDFNSF_NOFLAGS, gettext_noop("Enable CD (image) sanity checks."), gettext_noop("Sanity checks are only performed on discs detected(via heuristics) to be PS1 discs.  The checks primarily consist of ensuring that Q subchannel data is as expected for a typical commercially-released PS1 disc."), MDFNST_BOOL, "1" },
    { "psx.spu.resamp_quality", MDFNSF_NOFLAGS, gettext_noop("SPU output resampler quality."), gettext_noop("0 is lowest quality and CPU usage, 10 is highest quality and CPU usage.  The resampler that this setting refers to is used for converting from 44.1KHz to the sampling rate of the host audio device Mednafen is using.  Changing Mednafen's output rate, via the \"sound.rate\" setting, to \"44100\" may bypass the resampler, which can decrease CPU usage by Mednafen, and can increase or decrease audio quality, depending on various operating system and hardware factors."), MDFNST_UINT, "5", "0", "10" },
    { "psx.slstart", MDFNSF_NOFLAGS, gettext_noop("First displayed scanline in NTSC mode."), NULL, MDFNST_INT, "0", "0", "239" },
    { "psx.slend", MDFNSF_NOFLAGS, gettext_noop("Last displayed scanline in NTSC mode."), NULL, MDFNST_INT, "239", "0", "239" },
    { "psx.slstartp", MDFNSF_NOFLAGS, gettext_noop("First displayed scanline in PAL mode."), NULL, MDFNST_INT, "0", "0", "287" },	// 14
    { "psx.slendp", MDFNSF_NOFLAGS, gettext_noop("Last displayed scanline in PAL mode."), NULL, MDFNST_INT, "287", "0", "287" },	// 275
    { "psx.h_overscan", MDFNSF_NOFLAGS, gettext_noop("Show horizontal overscan area."), NULL, MDFNST_BOOL, "1" },
    { NULL }
};

int WINAPI wWinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPWSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow
) {
    MDFNI_InitializeModules();
    MDFN_MergeSettings(MednafenSettings);
    MDFN_FinalizeSettings();

    std::unique_ptr<NativeVFS> nativeVFS(new NativeVFS());
    std::unique_ptr<CDInterface> cd(CDInterface::Open(nativeVFS.get(), "E:\\Darragh\\Desktop\\Doom\\PSX DOOM\\Doom [U] [SLUS-00077]\\Doom.cue", true));
    std::vector<CDInterface*> cds = { cd.get() };
    EmulatedPSX.LoadCD(&cds);
    MDFNGameInfo = &EmulatedPSX;

    MDFN_PixelFormat pixelFormat(MDFN_COLORSPACE_RGB, 16, 8, 0, 24);
    std::unique_ptr<MDFN_Surface> surface(new MDFN_Surface(nullptr, 640, 480, 320, pixelFormat, true));
    std::unique_ptr<int32[]> surfaceLineWidths(new int32[480]);

    EmulateSpecStruct emuSpec = {};
    emuSpec.surface = surface.get();
    emuSpec.LineWidths = surfaceLineWidths.get();
    emuSpec.VideoFormatChanged = true;

    for (uint32 i = 0; i < 40; ++i) {
        EmulatedPSX.Emulate(&emuSpec);
    }

    return 0;
}
