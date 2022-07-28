//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Graphics' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Group.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Graphics' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Graphics& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);

    // TODO
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
