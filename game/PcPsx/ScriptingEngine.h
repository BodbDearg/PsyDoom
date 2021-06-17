#include "Macros.h"

#include <cstdint>

struct line_t;
struct mobj_t;

BEGIN_NAMESPACE(ScriptingEngine)

extern line_t*  gpCurTriggeringLine;
extern mobj_t*  gpCurTriggeringMobj;
extern bool     gbCurActionAllowed;

void init() noexcept;
void shutdown() noexcept;
void doAction(const int32_t actionNum, line_t* const pTrigLine, mobj_t* const pTrigMobj) noexcept;

END_NAMESPACE(ScriptingEngine)
