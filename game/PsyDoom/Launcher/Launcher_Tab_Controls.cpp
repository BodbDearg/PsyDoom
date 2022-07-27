﻿//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Controls' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Utils.h"
#include "Launcher_Widgets.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Group.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Controls' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Controls& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);

    // TODO
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
