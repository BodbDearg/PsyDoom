#include <mednafen/cdrom/CDInterface.h>
#include <mednafen/NativeVFS.h>
#include <mednafen/psx/psx.h>
#include <mednafen/settings.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>

static const MDFNSetting MednafenSettings[] = {
    {
        "cd.image_memcache",
        MDFNSF_NOFLAGS,
        gettext_noop("Cache entire CD images in memory."),
        gettext_noop("Reads the entire CD image(s) into memory at startup(which will cause a small delay).  Can help obviate emulation hiccups due to emulated CD access.  May cause more harm than good on low memory systems, systems with swap enabled, and/or when the disc images in question are on a fast SSD."),
        MDFNST_BOOL,
        "0"
    },
    {
        "filesys.untrusted_fip_check",
        MDFNSF_NOFLAGS,
        gettext_noop("Enable untrusted file-inclusion path security check."),
	    gettext_noop("When this setting is set to \"1\", the default, paths to files referenced from files like CUE sheets and PSF rips are checked for certain characters that can be used in directory traversal, and if found, loading is aborted.  Set it to \"0\" if you want to allow constructs like absolute paths in CUE sheets, but only if you understand the security implications of doing so(see \"Security Issues\" section in the documentation)."),
        MDFNST_BOOL,
        "1" 
    },
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
    std::unique_ptr<Mednafen::CDInterface> cd(Mednafen::CDInterface::Open(nativeVFS.get(), "E:\\Darragh\\Desktop\\Doom\\PSX DOOM\\Doom [U] [SLUS-00077]\\Doom.cue", true));
    
    return 0;
}
