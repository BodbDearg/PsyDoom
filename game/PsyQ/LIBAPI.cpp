#include "LIBAPI.h"

#include "PcPsx/Network.h"
#include <algorithm>
#include <chrono>
#include <cmath>

// Include LAST due to potential conflicts with the MIPS macros
#include "PsxVm/PsxVm.h"

// Shorten this
typedef std::chrono::high_resolution_clock::time_point SysTime;

// Use this dummy id for our root counter 2 event
static constexpr int32_t RCNT2_EVENT_ID = 0x7FFF0001;

// Keeps track of stuff relating to the PlayStation's 'Root Counter 2' (8 system clocks) timer.
// It's used as the heartbeat for the music system in DOOM, and we must generate periodic 'interrupts' to advance the music.
struct RootCounter2 {
    double      interruptIntervalSecs;      // How often we generate interrupts
    double      spilloverTime;              // How much time past when the last interrupt was due elapsed
    SysTime     lastInterruptTime;          // When we last generated an interrupt
    bool        bIsEventOpened;             // Has 'OpenEvent' been called?
    bool        bIsEventEnabled;            // Has 'EnableEvent' been called?
    bool        bIsCounting;                // Has 'StartRCnt' been called?

    // The function that will be called to handle the periodic timer event
    int32_t (*pHandler)();
};

static RootCounter2 gRootCnt2 = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Not a part of the original LIBAPI: this is called by the host application to generate timer related events (fake 'interrupts')
//------------------------------------------------------------------------------------------------------------------------------------------
void generate_timer_events() noexcept {
    // Don't do anything if we are not counting or if there is no interval defined
    if ((!gRootCnt2.bIsCounting) || (gRootCnt2.interruptIntervalSecs <= 0))
        return;

    // Figure out how much time has elapsed since the last interrupt
    const SysTime now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = now - gRootCnt2.lastInterruptTime;
    const double elapsedSecs = elapsedTime.count() + gRootCnt2.spilloverTime;

    // Only generate an event if enough time has passed
    if (elapsedSecs < gRootCnt2.interruptIntervalSecs)
        return;

    // Generate one event per every single interval passed up to a certain max (skip the rest).
    const int32_t numEvents = std::max((int32_t)(elapsedSecs / gRootCnt2.interruptIntervalSecs), 1);
    const int32_t numEventsToFire = std::min(numEvents, 16);

    // Figure out how far we are towards the next event
    const double spilloverSecs = std::fmod(elapsedSecs, gRootCnt2.interruptIntervalSecs);

    // Save the time of the last interrupt and spillover amount
    gRootCnt2.lastInterruptTime = now;
    gRootCnt2.spilloverTime = spilloverSecs;

    // Generate the interrupts if possible
    if (gRootCnt2.bIsEventOpened && gRootCnt2.bIsEventEnabled) {
        for (int32_t i = 0; i < numEventsToFire; ++i) {
            gRootCnt2.pHandler();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Close the specified hardware event object
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_CloseEvent(const int32_t event) noexcept {
    // Only supporting the root counter 2 event for DOOM
    if (event == RCNT2_EVENT_ID) {
        if (gRootCnt2.bIsEventOpened) {
            gRootCnt2 = {};
            return true;
        }
        else {
            return false;   // Already closed!
        }
    }

    // TODO: REMOVE BIOS CALL
    a0 = event;
    t2 = 0xB0;
    t1 = 9;
    emu_call(t2);
    return (v0 != 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Disables hardware interrupts
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBAPI_EnterCriticalSection() noexcept {
    // Not using interrupts in this port/environment anymore - just ignore the call...
}

int32_t LIBAPI_write([[maybe_unused]] const int32_t fileDesc, const void* const pBuffer, const int32_t numBytes) noexcept {
    // FIXME: IMPLEMENT PROPERLY - return value: bytes written or -1 on error
    return Network::sendBytes(pBuffer, numBytes) ? numBytes : -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable the specified hardware event object.
// Note: timers require an additional step (calling 'StartRCnt') in order to actually start generating event interrupts.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_EnableEvent(const int32_t event) noexcept {
    // Only supporting the root counter 2 event for DOOM
    if (event == RCNT2_EVENT_ID) {
        if (gRootCnt2.bIsEventOpened) {
            gRootCnt2.bIsEventEnabled = true;
            return true;
        } else {
            return false;   // Event not opened!
        }
    }

    // TODO: REMOVE BIOS CALL
    a0 = event;
    t2 = 0xB0;
    t1 = 0xC;
    emu_call(t2);
    return (v0 != 0);
}

void LIBAPI_InitPAD() noexcept {
loc_80049C5C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x12;                                          // Result = 00000012
    emu_call(t2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the target for a root counter
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_SetRCnt(const int32_t cntType, const uint16_t target, const int32_t mode) noexcept {
    // Only supporting the root counter 2 event for DOOM in interrupt mode
    if ((cntType != RCntCNT2) || (mode != RCntMdINTR))
        return false;

    // Figure out how many seconds are in 1 increment of root counter 2.
    // There are 8 CPU clocks per 1 clock of root counter 2.
    constexpr double PS1_CPU_CLOCKS_PER_SEC = 33'868'800.0;
    constexpr double SECS_PER_UNIT = 8.0 / PS1_CPU_CLOCKS_PER_SEC;

    gRootCnt2.interruptIntervalSecs = (double) target * SECS_PER_UNIT;
    gRootCnt2.spilloverTime = 0.0;
    gRootCnt2.lastInterruptTime = std::chrono::high_resolution_clock::now();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Activate the specified root counter
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_StartRCnt(const int32_t cntType) noexcept {
    // Only supporting the root counter 2 event for DOOM
    if (cntType != RCntCNT2)
        return false;

    gRootCnt2.bIsCounting = true;
    gRootCnt2.spilloverTime = 0.0;
    gRootCnt2.lastInterruptTime = std::chrono::high_resolution_clock::now();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Disable the specified hardware event object.
// Note: timers require an additional step (calling 'StartRCnt') in order to actually start generating event interrupts.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_DisableEvent(const int32_t event) noexcept {
    // Only supporting the root counter 2 event for DOOM
    if (event == RCNT2_EVENT_ID) {
        if (gRootCnt2.bIsEventOpened) {
            gRootCnt2.bIsEventEnabled = false;
            return true;
        } else {
            return false;   // Event not opened!
        }
    }

    // TODO: REMOVE BIOS CALL
    a0 = event;
    t2 = 0xB0;
    t1 = 0xD;
    emu_call(t2);
    return (v0 != 0);
}

void LIBAPI_StartPAD() noexcept {
loc_80049DEC:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x13;                                          // Result = 00000013
    emu_call(t2);
}

void LIBAPI_ChangeClearPAD() noexcept {
loc_80049DFC:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x5B;                                          // Result = 0000005B
    emu_call(t2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open a specified hardware event (which can be listened to) and return the event descriptor
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBAPI_OpenEvent(const int32_t cause, const int32_t type, const int32_t mode, int32_t (* const pHandler)()) noexcept {
    // Only supporting interrupts generated by 'root counter 2' for DOOM
    if ((cause == RCntCNT2) && (type == EvSpINT) && (mode == EvMdINTR)) {
        // A handler must be given and the event must not be already opened!
        if (pHandler && (!gRootCnt2.bIsEventOpened)) {
            gRootCnt2 = {};
            gRootCnt2.bIsEventOpened = true;
            gRootCnt2.pHandler = pHandler;
            return RCNT2_EVENT_ID;
        }
        else {
            // Failed to open the event! -1 means failure:
            return -1;
        }
    }

    // TODO: REMOVE THIS
    a0 = cause;
    a1 = type;
    a2 = mode;
    a3 = PsxVm::getNativeFuncVmAddr((void*) pHandler);

    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 8;                                             // Result = 00000008
    emu_call(t2);

    return v0;
}

int32_t LIBAPI_read([[maybe_unused]] const int32_t fileDesc, void* const pBuffer, const int32_t numBytes) noexcept {
    // FIXME: IMPLEMENT PROPERLY - return value: bytes read or -1 on error
    return Network::recvBytes(pBuffer, numBytes) ? numBytes : -1;
}

bool LIBAPI_TestEvent([[maybe_unused]] const uint32_t eventDescriptor) noexcept {
    // FIXME: IMPLEMENT ME
    return true;

#if 0
loc_80049E2C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0xB;                                           // Result = 0000000B
    emu_call(t2);
    v0 = 0;
#endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Re-enables hardware interrupts
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBAPI_ExitCriticalSection() noexcept {
    // Not using interrupts in this port/environment anymore - just ignore the call...
}

void LIBAPI_open() noexcept {
loc_80049E4C:
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x32;                                          // Result = 00000032
    emu_call(t2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flushes the instruction cache
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBAPI_FlushCache() noexcept {
    // Don't need to do anything for this anymore...
}

void LIBAPI_InitHeap() noexcept {
loc_80050884:
    t2 = 0xA0;                                          // Result = 000000A0
    t1 = 0x39;                                          // Result = 00000039
    emu_call(t2);
}
