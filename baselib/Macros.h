#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// A collection of useful utility macros
//------------------------------------------------------------------------------------------------------------------------------------------

// Used to decorate exception throwing C++ functions
#define THROWS noexcept(false)

// Begin and end a namespace
#define BEGIN_NAMESPACE(Name) namespace Name {
#define END_NAMESPACE(Name) }

// Macro to get the number of elements in a C Array whose size is known at compile time
#define C_ARRAY_SIZE(arrayPtr)\
    (sizeof(arrayPtr) / sizeof(*arrayPtr))

// Mark a variable as unused to avoid warnings
#define MARK_UNUSED(variable)\
    (void) variable

 // Macro that declares a default constructor and copy constructor of the given type and which disallows all forms of assignment.
 // Intended to be used for map data structs that are housed in containers, which we want to allow default and copy construct
 // operations for but which we want to disallow assignment (to prevent accidental assignment).
 #define NON_ASSIGNABLE_STRUCT(Type)\
    Type() noexcept = default;\
    Type(const Type& other) noexcept = default;\
    Type& operator = (const Type& other) noexcept = delete;

// Disable and re-enable certain warnings when including third party headers
#if defined(_MSC_VER)
    // MSVC++
    #define BEGIN_DISABLE_HEADER_WARNINGS\
        __pragma(warning(push))             /* Save so we can restore later */\
        __pragma(warning(disable:4201))     /* Non standard extension used */\
        __pragma(warning(disable:4244))     /* Conversion: possible loss of data */

    #define END_DISABLE_HEADER_WARNINGS\
        __pragma(warning(pop))
#elif defined (__GNUC__)
    // GCC or Clang
    #define BEGIN_DISABLE_HEADER_WARNINGS\
        _Pragma("GCC diagnostic push")\
        _Pragma("GCC diagnostic ignored \"-Wgnu-anonymous-struct\"")    /* Anonymous structs are a GNU extension */
    
    #define END_DISABLE_HEADER_WARNINGS\
        _Pragma("GCC diagnostic pop")
#else
    // Unhandled compiler
    #define BEGIN_DISABLE_HEADER_WARNINGS
    #define END_DISABLE_HEADER_WARNINGS
#endif
