#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Assertion macros
//------------------------------------------------------------------------------------------------------------------------------------------
#include "FatalErrors.h"

// Whether or not asserts are enabled
#ifndef NDEBUG
    #define ASSERTS_ENABLED 1
#endif

// Regular assert without a message
#if ASSERTS_ENABLED == 1
    #define ASSERT(Condition)\
        do {\
            if (!(Condition)) {\
                FatalErrors::raiseF("Assert failed! Condition: %s\n", #Condition);\
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
                FatalErrors::raiseF("Assert failed! Condition: %s\n%s\n", #Condition, Message);\
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
                FatalErrors::raiseF("Assert failed! Condition: %s\n" ## MessageFormat ## "\n", #Condition, __VA_ARGS__);\
            }\
        } while (0)
#else
    #define ASSERT_LOG_F(Condition, MessageFormat, ...)
#endif

// Raise a failed assertion
#if ASSERTS_ENABLED == 1
    #define ASSERT_FAIL(Message)\
        do {\
            FatalErrors::raiseF("Assert failed!\n%s\n", Message);\
        } while (0)
    
    #define ASSERT_FAIL_F(MessageFormat, ...)\
        do {\
            FatalErrors::raiseF("Assert failed!\n" ## MessageFormat ## "\n", __VA_ARGS__);\
        } while (0)
#else
    #define ASSERT_FAIL(Message)
    #define ASSERT_FAIL_F(MessageFormat, ...)
#endif
