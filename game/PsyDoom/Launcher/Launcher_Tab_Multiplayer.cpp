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
// Makes the 'co-op' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeCoopSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 300, 30, "Cooperative");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 300, 70, "");
    
        // Friendly fire toggle
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 150, 30, "  No Friendly Fire");
        bindConfigField<Config::gbNoFriendlyFire, Config::gbNeedSave_Multiplayer>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Multiplayer.noFriendlyFire.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'deathmatch' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeDeathmatchSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 300, 30, "Deathmatch");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 300, 100, "");

        // Death match frag limit
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 40, 140, 26, "Frag Limit");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Multiplayer.fragLimit.comment);

        const auto pInput = new Fl_Int_Input(x + 170, y + 40, 110, 26);
        bindConfigField<Config::gFragLimit, Config::gbNeedSave_Multiplayer>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

        // Disable Exit Switch
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 150, 30, "  Disable Exits");
        bindConfigField<Config::gbExitDisabled, Config::gbNeedSave_Multiplayer>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Multiplayer.exitDisabled.comment);
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
    makeCoopSection((tabRect.lx + tabRect.rx) / 2 - 150, tabRect.ty + 20);
    makeDeathmatchSection((tabRect.lx + tabRect.rx) / 2 - 150, tabRect.ty + 170);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
