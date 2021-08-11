#pragma once

#include "Macros.h"

#include <cstdint>

struct line_t;
struct mobj_t;
struct sector_t;

BEGIN_NAMESPACE(ScriptingEngine)

extern line_t*      gpCurTriggeringLine;
extern sector_t*    gpCurTriggeringSector;
extern mobj_t*      gpCurTriggeringMobj;
extern int32_t      gCurActionTag;
extern int32_t      gCurActionUserdata;
extern bool         gbCurActionAllowed;
extern bool         gbNeedMobjGC;

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
