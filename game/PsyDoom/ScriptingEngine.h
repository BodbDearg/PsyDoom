#pragma once

#include "Macros.h"

#include <cstdint>
#include <vector>

struct line_t;
struct mobj_t;
struct sector_t;

BEGIN_NAMESPACE(ScriptingEngine)

// Holds details on an action which is scheduled to run later.
// Note that delayed actions have no triggering line, sector or thing associated with them.
struct ScheduledAction {
    int32_t     actionNum;          // Which action function to execute with
    int32_t     delayTics;          // Game tics left until the action executes
    int32_t     executionsLeft;     // The number of action executions left; '0' if the action will not execute again, or '-1' if infinitely repeating.
    int32_t     repeatDelay;        // The delay in tics between repeats ('0' means execute every tic)
    int32_t     tag;                // User defined tag associated with the action
    int32_t     userdata;           // User defined data associated with the action
    bool        bPaused;            // If 'true' then the action is paused, otherwise it's unpaused
    bool        bPendingExecute;    // If 'true' then the action is pending execution this frame
};

extern std::vector<ScheduledAction>     gScheduledActions;
extern line_t*                          gpCurTriggeringLine;
extern sector_t*                        gpCurTriggeringSector;
extern mobj_t*                          gpCurTriggeringMobj;
extern int32_t                          gCurActionTag;
extern int32_t                          gCurActionUserdata;
extern bool                             gbCurActionAllowed;
extern bool                             gbNeedMobjGC;

void init() noexcept;
void shutdown() noexcept;
void runScheduledActions() noexcept;

bool doAction(
    const int32_t actionNum,
    line_t* const pTrigLine,
    sector_t* const pTrigSector,
    mobj_t* const pTrigMobj,
    const int32_t actionTag,
    const int32_t actionUserdata
) noexcept;

void scheduleAction(
    const int32_t actionNum,
    const int32_t delayTics,
    const int32_t tag,
    const int32_t userdata
) noexcept;

void scheduleRepeatingAction(
    const int32_t actionNum,
    const int32_t initialDelayTics,
    const int32_t numRepeats,
    const int32_t repeatDelay,
    const int32_t tag,
    const int32_t userdata
) noexcept;

void stopAllScheduledActions() noexcept;
void stopScheduledActionsWithTag(const int32_t tag) noexcept;
void pauseAllScheduledActions(const bool bPause) noexcept;
void pauseScheduledActionsWithTag(const int32_t tag, const bool bPause) noexcept;
int32_t getNumScheduledActionsWithTag(const int32_t tag) noexcept;

END_NAMESPACE(ScriptingEngine)
