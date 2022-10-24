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
    #include <FL/Fl_Native_File_Chooser.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'co-op' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeCoopSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 336, 30, "Cooperative");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 336, 160, "");
    
        // Friendly fire toggle
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 150, 30, "  No Friendly Fire");
        bindConfigField<Config::gbNoFriendlyFire, Config::gbNeedSave_Multiplayer>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Multiplayer.noFriendlyFire.comment);
    }

    // Enable multiplayer-only things
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 150, 30, "  Spawn Multiplayer-Only Things");
        bindConfigField<Config::gbMPThings, Config::gbNeedSave_Multiplayer>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Multiplayer.mpThings.comment);
    }

        // Preserve keys on respawn
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 100, 150, 30, "  Preserve Keys on Respawn");
        bindConfigField<Config::gbPreserveKeys, Config::gbNeedSave_Multiplayer>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Multiplayer.preserveKeys.comment);
    }

        // Preserve ammo on respawn
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 140, 120, 26, "Preserve Ammo on Respawn");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Multiplayer.preserveAmmoFactor.comment);

        const auto pChoice = new Fl_Choice(x + 210, y + 140, 100, 26);
        pChoice->add("None");
        pChoice->add("All");
        pChoice->add("Half");
        pChoice->tooltip(pLabel->tooltip());
        pChoice->callback(
            [](Fl_Widget* const pWidget, void*) noexcept {
                Fl_Choice* const pChoice = static_cast<Fl_Choice*>(pWidget);

                switch (pChoice->value()) {
                    case 0: Config::gPreserveAmmoFactor = 0; break;
                    case 1: Config::gPreserveAmmoFactor = 1; break;
                    case 2: Config::gPreserveAmmoFactor = 2; break;
                }

                Config::gbNeedSave_Multiplayer = true;
            }
        );

        if (Config::gPreserveAmmoFactor == 2) {
            pChoice->value(2);
        } else if (Config::gPreserveAmmoFactor == 1) {
            pChoice->value(1);
        } else {
            pChoice->value(0);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'deathmatch' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeDeathmatchSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 336, 30, "Deathmatch");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 336, 100, "");

        // Frag limit
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 50, 140, 26, "Frag Limit");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Multiplayer.fragLimit.comment);

        const auto pInput = new Fl_Int_Input(x + 200, y + 50, 110, 26);
        bindConfigField<Config::gFragLimit, Config::gbNeedSave_Multiplayer>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

        // Disable Exit Switch
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 80, 150, 30, "  Disable Exits");
        bindConfigField<Config::gbExitDisabled, Config::gbNeedSave_Multiplayer>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Multiplayer.exitDisabled.comment);
    }

    new Fl_Box(FL_NO_BOX, x, y + 160, 336, 30, "Note: These settings are ignored for client");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Multiplayer' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populateMultiplayerTab(Context& ctx) noexcept {
    Tab_Audio& tab = ctx.tab_audio;
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);
    makeCoopSection((tabRect.lx + tabRect.rx) / 2 - 168, tabRect.ty + 20);
    makeDeathmatchSection((tabRect.lx + tabRect.rx) / 2 - 168, tabRect.ty + 220);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
