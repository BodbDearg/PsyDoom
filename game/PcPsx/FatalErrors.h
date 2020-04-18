#pragma once

// TODO: use BEGIN_NAMESPACE
namespace FatalErrors {
    // Functions that can be called to handle fatal errors like out of memory.
    // The program will terminate after each fatal error type.
    [[noreturn]] void outOfMemory() noexcept;

    // Issue a generic fatal error message with any message.
    // This is useful for very specific error messages for a certain part of the application.
    [[noreturn]] void error(const char* const pStr) noexcept;

    // Same as above, but with 'printf' style formatting of the error
    [[noreturn]] void errorWithFormat(const char* const pFormatStr, ...) noexcept;
}
