#pragma once

namespace FatalErrors {
    // Issue a generic fatal error message with any message.
    // This is useful for very specific error messages for a certain part of the application.
    [[noreturn]] void error(const char* const pStr) noexcept;

    // Same as above, but with 'printf' style formatting of the error
    [[noreturn]] void errorWithFormat(const char* const pFormatStr, ...) noexcept;
}

// Raise a fatal error message
#define FATAL_ERROR(Message)\
    do {\
        FatalErrors::errorWithFormat("%s\n", Message);\
    } while (0)

// Raise a formatted Fatal error message
#define FATAL_ERROR_F(MessageFormat, ...)\
    do {\
        FatalErrors::errorWithFormat(MessageFormat, __VA_ARGS__);\
    } while (0)
