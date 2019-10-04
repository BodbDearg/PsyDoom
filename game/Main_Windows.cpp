#include <mednafen/cdrom/CDInterface.h>
#include <mednafen/psx/psx.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int WINAPI wWinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPWSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow
) {
    //D_DoomMain();
    
    Mednafen::CDInterface* cd = Mednafen::CDInterface::Open(nullptr, "PSXDoom.bin", false);
    return 0;
}
