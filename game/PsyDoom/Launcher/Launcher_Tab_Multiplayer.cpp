//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Multplayer' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Config/ConfigSerialization_Multiplayer.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Group.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'Settings' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeSettingSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 300, 30, "Multiplayer settings");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 300, 100, "");
    
        // Friendly fire toggle
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 50, 150, 30, "  No Friendly Fire");
        bindConfigField<Config::gbNoFriendlyFire, Config::gbNeedSave_Multiplayer>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Multiplayer.noFriendlyFire.comment);
    }

        // Death match frag limit
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 80, 140, 26, "Frag Limit");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Multiplayer.fragLimit.comment);

        const auto pInput = new Fl_Int_Input(x + 170, y + 80, 110, 26);
        bindConfigField<Config::gFragLimit, Config::gbNeedSave_Multiplayer>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Multiplayer' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populateMultiplayerTab(Context& ctx) noexcept {
    Tab_Audio& tab = ctx.tab_audio;
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);
    makeSettingSection((tabRect.lx + tabRect.rx) / 2 - 150, tabRect.ty + 20);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
