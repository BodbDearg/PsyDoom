#include "FatalErrors.h"

#include "Video.h"
#include "Macros.h"
#include <cstring>
#include <SDL.h>
#include <vector>

BEGIN_NAMESPACE(FatalErrors)

// Fallback string to use if null pointers are given for some reason.
// Normally most code does not tolerate nulls, but error reporting should be more robust.
static constexpr const char* const UNSPECIFIED_ERROR_STR = "An unspecified/unknown error has occurred!";

[[noreturn]] static void fatalError(const char* const msg) noexcept {
    // Always print to the console (standard out) and in debug builds
    if (msg) {
        std::printf("[FATAL ERROR] %s\n", msg);
    } else {
        std::printf("[FATAL ERROR] %s\n", UNSPECIFIED_ERROR_STR);
    }

    // Show a GUI error box
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "A fatal error has occurred!",
        (msg != nullptr) ? msg : UNSPECIFIED_ERROR_STR,
        PcPsx::getWindow()
    );
    
    // Finish up!
    std::terminate();
}

[[noreturn]] void outOfMemory() noexcept {
    fatalError("A memory allocation has failed - out of memory!");
}

[[noreturn]] void error(const char* const pStr) noexcept {
    if (pStr == nullptr) {
        fatalError(UNSPECIFIED_ERROR_STR);
    } else {
        fatalError(pStr);
    }
}

[[noreturn]] void errorWithFormat(const char* const pFormatStr, ...) noexcept {
    if (pFormatStr == nullptr) {
        fatalError(UNSPECIFIED_ERROR_STR);
    }
    else {
        const size_t guessBufferLength = (std::strlen(pFormatStr) + 1) * 2;
        std::vector<char> buffer;
        buffer.resize(guessBufferLength);

        va_list va_args;
        va_start(va_args, pFormatStr);
        int numCharsWrittenOrRequired = vsnprintf(buffer.data(), buffer.size(), pFormatStr, va_args);
        va_end(va_args);

        if (numCharsWrittenOrRequired < 0) {
            // A return value of < 0 from 'vsnprintf' indicates an error!
            fatalError(UNSPECIFIED_ERROR_STR);
        }

        if ((unsigned int) numCharsWrittenOrRequired >= buffer.size()) {
            buffer.resize((size_t) numCharsWrittenOrRequired + 1);

            va_start(va_args, pFormatStr);
            numCharsWrittenOrRequired = vsnprintf(buffer.data(), buffer.size(), pFormatStr, va_args);
            va_end(va_args);

            if (numCharsWrittenOrRequired < 0) {
                // A return value of < 0 from 'vsnprintf' indicates an error!
                fatalError(UNSPECIFIED_ERROR_STR);
            }
        }

        va_end(va_args);
        fatalError(buffer.data());
    }
}

END_NAMESPACE(FatalErrors)
