#include "Macros.h"

#include <cstdint>

struct line_t;
struct mobj_t;
struct sector_t;

BEGIN_NAMESPACE(ScriptingEngine)

extern line_t*      gpCurTriggeringLine;
extern sector_t*    gpCurTriggeringSector;
extern mobj_t*      gpCurTriggeringMobj;
extern bool         gbCurActionAllowed;
extern bool         gbNeedMobjGC;

void init() noexcept;
void shutdown() noexcept;

void doAction(
    const int32_t actionNum,
    line_t* const pTrigLine,
    sector_t* const pTrigSector,
    mobj_t* const pTrigMobj
) noexcept;

END_NAMESPACE(ScriptingEngine)
