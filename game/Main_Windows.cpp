#include "Doom/psx_main.h"
#include "PsyDoom/Launcher/Launcher.h"

#include <clocale>
#include <memory>
#include <string>
#include <vector>
#include <Windows.h>

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds command line arguments for the app in UTF-8 and standard-c type format. Note: movable only!
//------------------------------------------------------------------------------------------------------------------------------------------
struct Args {
    Args() = default;
    Args(Args&& other) = default;

    std::vector<const char*>    argv;       // A list of pointers to all the arguments
    std::string                 argStr;     // The memory for all arguments (each is null separated)
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Appends a command line argument in wide character c-string format to the given 'std::string' (encoded as UTF-8).
// Each argument will also have a null terminator at the end in the output string.
//------------------------------------------------------------------------------------------------------------------------------------------
static void appendCmdLineArg(const wchar_t* const wideStrArg, std::string& utf8ArgStrOut) noexcept {
    // Figure out how much of a buffer is needed for this string in UTF-8 (including the null terminator, by specifying length '-1') and allocate it
    const int bufferSizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideStrArg, -1, nullptr, 0, 0, 0);
    const size_t argStrOldSize = utf8ArgStrOut.size();
    utf8ArgStrOut.resize(argStrOldSize + bufferSizeNeeded);

    // Convert the string to UTF-8 and include the null terminator
    WideCharToMultiByte(CP_UTF8, 0, wideStrArg, -1, utf8ArgStrOut.data() + argStrOldSize, bufferSizeNeeded, 0, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Converts the Windows style single arg list (wide character) to a standard C style argument list encoded in UTF-8
//------------------------------------------------------------------------------------------------------------------------------------------
static Args getCmdLineArgs(const PWSTR lpCmdLine) {
    // Reserve some space for all the output arguments
    Args args = {};
    args.argStr.reserve(1024);

    // Get the name of the .exe: reserve initially MAX_PATH and keep doubling the buffer size until we have enough to hold the .exe path.
    // This will be the first of our output arguments.
    {
        std::wstring exeFileName;
        exeFileName.resize(MAX_PATH);

        while (GetModuleFileNameW(NULL, exeFileName.data(), (DWORD) exeFileName.size()) >= exeFileName.size()) {
            exeFileName.resize(exeFileName.size() * 2);
        }

        appendCmdLineArg(exeFileName.c_str(), args.argStr);
    }

    // Split up the command line string.
    // 
    // Note: if the input string to 'CommandLineToArgvW' is empty then the program path will be returned instead, so don't call it when we have no arguments!
    // We don't need the program/.exe path since we got that already...
    int argc = 0;
    LPWSTR* argvW = nullptr;

    if (lpCmdLine && lpCmdLine[0] != 0) {
        argvW = CommandLineToArgvW(lpCmdLine, &argc);
    }

    // Makeup a combined argument string with all arguments separated by a null character.
    // Note: the program/.exe path has already been added to the string before this, that's always the first argument.
    for (int argIdx = 0; argIdx < argc; ++argIdx) {
        appendCmdLineArg(argvW[argIdx], args.argStr);
    }

    // Now make a list of pointers to all the arguments
    char* const pArgStrChars = args.argStr.data();

    for (size_t charIdx = 0; charIdx < args.argStr.size(); ++charIdx) {
        // Keep skipping nulls until we have a valid start of string
        if (!pArgStrChars[charIdx])
            continue;

        args.argv.push_back(pArgStrChars + charIdx);

        // Goto the end of the string (terminated by a null character)
        while (charIdx < args.argStr.size() && pArgStrChars[charIdx]) {
            ++charIdx;
        }
    }

    // Free the temporary memory allocated by Windows for the separated args and return the result
    if (argvW) {
        LocalFree(argvW);
        argvW = nullptr;
    }

    return args;
}

#if !NDEBUG
//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates a debug console for the application and redirects standard streams to it
//------------------------------------------------------------------------------------------------------------------------------------------
static void openDebugConsoleWindow() noexcept {
    AllocConsole();

    FILE* pDummyFile = {};
    freopen_s(&pDummyFile, "CONIN$", "r", stdin);
    freopen_s(&pDummyFile, "CONOUT$", "w", stderr);
    freopen_s(&pDummyFile, "CONOUT$", "w", stdout);
}
#endif  // #if !NDEBUG

//------------------------------------------------------------------------------------------------------------------------------------------
// Windows entrypoint for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
int WINAPI wWinMain(
    [[maybe_unused]] const HINSTANCE hInstance,
    [[maybe_unused]] const HINSTANCE hPrevInstance,
    const PWSTR lpCmdLine,
    [[maybe_unused]] const int nShowCmd
) {
    // Interpret 'char*' strings as UTF-8 for 'fopen' etc.
    setlocale(LC_ALL, "en_US.UTF-8");

    // In debug builds allocate a console window also and redirect the standard streams to it
    #if !NDEBUG
        openDebugConsoleWindow();
    #endif

    // Get the 'main' function arguments in standard C format and then run the game
    const Args args = getCmdLineArgs(lpCmdLine);
    
    // Run the game!
    #if PSYDOOM_LAUNCHER
        return Launcher::launcherMain((int) args.argv.size(), args.argv.data());
    #else
        return psx_main((int) args.argv.size(), args.argv.data());
    #endif
}
