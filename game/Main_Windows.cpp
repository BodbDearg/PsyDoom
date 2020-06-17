#include "Doom/psx_main.h"

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
// Converts the Windows style single arg list (wide character) to a standard C style argument list encoded in UTF-8
//------------------------------------------------------------------------------------------------------------------------------------------
static Args getCmdLineArgs(const PWSTR lpCmdLine) {
    // Split up the command line string
    int argc = {};
    Args args = {};
    LPWSTR* const argvW = CommandLineToArgvW(lpCmdLine, &argc);

    // Makeup a combined argument string with all arguments separated by a null character
    args.argStr.reserve(256);

    for (int argIdx = 0; argIdx < argc; ++argIdx) {
        // Figure out how much of a buffer is needed for this string (including null terminator, by specifying length '-1') and allocate it
        const int bufferSizeNeeded = WideCharToMultiByte(CP_UTF8, 0, argvW[argIdx], -1, nullptr, 0, 0, 0);
        const size_t argStrOldSize = args.argStr.size();
        args.argStr.resize(argStrOldSize + bufferSizeNeeded);

        // Convert the string to UTF-8 and include the null terminator
        WideCharToMultiByte(CP_UTF8, 0, argvW[argIdx], -1, args.argStr.data() + argStrOldSize, bufferSizeNeeded, 0, 0);
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

    // Free the temporary memory allocated by Windows for the split args and return the result
    LocalFree(argvW);
    return args;
}

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Windows entrypoint for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
int wWinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    PWSTR lpCmdLine,
    [[maybe_unused]] int nShowCmd
) {
    // Get the arguments in a nice standard C like format
    Args args = getCmdLineArgs(lpCmdLine);

    // In debug builds allocate a console window also and redirect the standard streams to it
    #if !NDEBUG
        openDebugConsoleWindow();
    #endif

    // Run PsyDoom!
    return psx_main((int) args.argv.size(), args.argv.data());
}
