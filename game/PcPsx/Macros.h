#pragma once

#include "FatalErrors.h"

// Whether or not asserts are enabled
#ifndef NDEBUG
    #define ASSERTS_ENABLED 1
#endif

// Required includes for error handling
#include <cstdio>
#include <cstdlib>

// Regular assert without a message
#if ASSERTS_ENABLED == 1
    #define ASSERT(Condition)\
        do {\
            if (!(Condition)) {\
                FatalErrors::errorWithFormat("Assert failed! Condition: %s\n", #Condition);\
            }\
        } while (0)
#else
    #define ASSERT(Condition)
#endif

// Assert with a simple message on failure
#if ASSERTS_ENABLED == 1
    #define ASSERT_LOG(Condition, Message)\
        do {\
            if (!(Condition)) {\
                FatalErrors::errorWithFormat("Assert failed! Condition: %s\n%s\n", #Condition, Message);\
            }\
        } while (0)
#else
    #define ASSERT_LOG(Condition, Message)
#endif

// Assert with a formatted message on failure
#if ASSERTS_ENABLED == 1
    #define ASSERT_LOG_F(Condition, MessageFormat, ...)\
        do {\
            if (!(Condition)) {\
                FatalErrors::errorWithFormat("Assert failed! Condition: %s\n" ## MessageFormat ## "\n", #Condition, __VA_ARGS__);\
            }\
        } while (0)
#else
    #define ASSERT_LOG_F(Condition, MessageFormat, ...)
#endif

// Raise a failed assertion
#if ASSERTS_ENABLED == 1
    #define ASSERT_FAIL(Message)\
        do {\
            FatalErrors::errorWithFormat("Assert failed!\n%s\n", Message);\
        } while (0)
    
    #define ASSERT_FAIL_F(MessageFormat, ...)\
        do {\
            FatalErrors::errorWithFormat("Assert failed!\n" ## MessageFormat ## "\n", __VA_ARGS__);\
        } while (0)
#else
    #define ASSERT_FAIL(Message)
    #define ASSERT_FAIL_F(MessageFormat, ...)
#endif

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

// Disable certain warnings when including third party headers
#if defined(_MSC_VER)
    // MSVC++
    #define BEGIN_THIRD_PARTY_INCLUDES\
        __pragma(warning(push))             /* Save so we can restore later */\
        __pragma(warning(disable:4201))     /* Non standard extension used */\
        __pragma(warning(disable:4244))     /* Conversion: possible loss of data */
#else
    // Unhandled compiler
    #define BEGIN_THIRD_PARTY_INCLUDES
#endif

#if defined(_MSC_VER)
    // MSVC++
    #define END_THIRD_PARTY_INCLUDES\
        __pragma(warning(pop))
#else
    // Unhandled compiler
    #define END_THIRD_PARTY_INCLUDES
#endif
