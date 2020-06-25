#include "LIBAPI.h"

#include "PcPsx/Assert.h"
#include "PcPsx/ProgArgs.h"
#include "PcPsx/PsxVm.h"

#include <algorithm>
#include <chrono>
#include <cmath>

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
// Not a part of the original LIBAPI: this is called by the host application to generate hardware timer related events (fake 'interrupts').
// This is what drives the music sequencer.
//------------------------------------------------------------------------------------------------------------------------------------------
void PsxVm::generateTimerEvents() noexcept {
    // Don't do anything in headless mode: no audio
    if (ProgArgs::gbHeadlessMode)
        return;

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
    // PC-PSX: only supporting the root counter 2 event here for Doom - that's all we need
    if (event == RCNT2_EVENT_ID) {
        if (gRootCnt2.bIsEventOpened) {
            gRootCnt2 = {};
            return true;
        }
        else {
            return false;   // Already closed!
        }
    }

    return false;   // Failed to close the event!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Disables PlayStation hardware interrupts; not using PSX interrupts anymore in PsyDoom, so this call now does nothing...
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBAPI_EnterCriticalSection() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable the specified hardware event object.
// Note: timers require an additional step (calling 'StartRCnt') in order to actually start generating event interrupts.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_EnableEvent(const int32_t event) noexcept {
    // PC-PSX: only supporting the root counter 2 event here for Doom - that's all we need.
    // Originally this was called to enable SIO (Serial I/O) read/write done events but we don't need that functionality anymore.
    if (event == RCNT2_EVENT_ID) {
        if (gRootCnt2.bIsEventOpened) {
            gRootCnt2.bIsEventEnabled = true;
            return true;
        } else {
            return false;   // Event not opened!
        }
    }

    return false;   // Failed to enable the event!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the controller driver.
// Two buffers are provided to receive controller data, which must be both at least 34 bytes in size.
// Always returns 'true' for success.
//
// On the PlayStation this function originally called into the BIOS to do it's work, but in PsyDoom we don't need to do anything here.
// We just ignore the given buffers also in PsyDoom, they're not needed any longer.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_InitPAD(
    [[maybe_unused]] uint8_t pPadInputBuffer1[34],
    [[maybe_unused]] uint8_t pPadInputBuffer2[34],
    [[maybe_unused]] const int32_t inputBuffer1Size,
    [[maybe_unused]] const int32_t inputBuffer2Size
) noexcept {
    // Nothing to do here for PsyDoom...
    return true;
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
    // PC-PSX: only supporting the root counter 2 event here for Doom - that's all we need.
    if (event == RCNT2_EVENT_ID) {
        if (gRootCnt2.bIsEventOpened) {
            gRootCnt2.bIsEventEnabled = false;
            return true;
        } else {
            return false;   // Event not opened!
        }
    }

    return false;   // Failed to disable the event!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyQ BIOS function which starts reading controller data on a regular basis; always returns 'true'.
// Not implemented for PsyDoom because we don't need it.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBAPI_StartPAD() noexcept { return true; }

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyQ BIOS function which determines how controller input is read from interrupts.
// I don't know the exact meaning of the values passed in - we don't need this function in PsyDoom anyway.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBAPI_ChangeClearPAD([[maybe_unused]] const int32_t val) noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open a specified hardware event (which can be listened to) and return the event descriptor
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBAPI_OpenEvent(const int32_t cause, const int32_t type, const int32_t mode, int32_t (* const pHandler)()) noexcept {
    // Only supporting interrupts generated by 'root counter 2' for Doom - that's all we need.
    // Originally this was called to open SIO (Serial I/O) read/write done events but we don't need that functionality anymore.
    if ((cause == RCntCNT2) && (type == EvSpINT) && (mode == EvMdINTR)) {
        // A handler must be given and the event must not be already opened!
        if (pHandler && (!gRootCnt2.bIsEventOpened)) {
            gRootCnt2 = {};
            gRootCnt2.bIsEventOpened = true;
            gRootCnt2.pHandler = pHandler;
            return RCNT2_EVENT_ID;
        }
    }

    return -1;  // Didn't open an event!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Re-enables PlayStation hardware interrupts; not using PSX interrupts anymore in PsyDoom, so this call now does nothing...
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBAPI_ExitCriticalSection() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flushes the instruction cache: doesn't need to do anything in PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBAPI_FlushCache() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bios implemented function which initializes the PsyQ SDK heap, so that functions like malloc() can work.
// PSX Doom didn't use this heap at all, so might have gotten away with not calling this.
// For PsyDoom we don't need to implement this either, so it's now just blank instead.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBAPI_InitHeap([[maybe_unused]] void* const pHeapMem, [[maybe_unused]] const uint32_t heapSize) noexcept {}
