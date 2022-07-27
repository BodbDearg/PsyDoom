//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Launcher' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Utils.h"
#include "Launcher_Widgets.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Native_File_Chooser.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the cue file selector
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeCueFileSelector(Tab_Launcher& tab, const int lx, const int rx, const int ty) noexcept {
    const auto pLabel_cue = new Fl_Box(FL_NO_BOX, lx, ty, 150, 30, "Game disc .cue file");
    pLabel_cue->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel_cue->tooltip(
        "Path to the game disc's .cue file.\n"
        "\n"
        "The game disc supplied must be PlayStation 'Doom', 'Final Doom', or any other supported disc.\n"
        "See PsyDoom's main README.md file for a list of all supported game discs.\n"
        "If this field is unspecified then the default .cue file will be used instead.\n"
        "This default .cue file can be changed under the 'Game' config tab.\n"
    );

    const auto pButton_clearCue = new Fl_Button(rx - 30, ty + 30, 30, 30, "X");
    pButton_clearCue->callback(
        []([[maybe_unused]] Fl_Widget* const pWidget, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;
            tab.pInput_cue->value("");
        },
        &tab
    );

    const auto pButton_pickCue = new Fl_Button(rx - 110, ty + 30, 80, 30, "Browse");
    pButton_pickCue->callback(
        []([[maybe_unused]] Fl_Widget* const pWidget, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;

            const auto pFileChooser = std::make_unique<Fl_Native_File_Chooser>();
            pFileChooser->filter("CUE Sheet Files\t*.{cue}");
            pFileChooser->type(Fl_Native_File_Chooser::BROWSE_FILE);
            pFileChooser->title("Choose a game disc .cue file");

            if ((pFileChooser->show() == 0) && (pFileChooser->count() == 1)) {
                tab.pInput_cue->value(pFileChooser->filename());
            }
        },
        &tab
    );

    const int cueInputRx = getRectExtents(*pButton_pickCue).lx - 10;
    tab.pInput_cue = makeFl_Input(lx, ty + 30, cueInputRx - lx, 30);
    tab.pInput_cue->tooltip(pLabel_cue->tooltip());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the mod data dir selector
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeModDataDirSelector(Tab_Launcher& tab, const int lx, const int rx, const int ty) noexcept {
    const auto pLabel_dataDir = new Fl_Box(FL_NO_BOX, lx, ty, 150, 30, "Mod data dir");
    pLabel_dataDir->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel_dataDir->tooltip(
        "Path to a directory containing files for a PsyDoom mod.\n"
        "Use this to pick which mod is active.\n"
    );

    const auto pButton_clearDataDir = new Fl_Button(rx - 30, ty + 30, 30, 30, "X");
    pButton_clearDataDir->callback(
        []([[maybe_unused]] Fl_Widget* const pWidget, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;
            tab.pInput_dataDir->value("");
        },
        &tab
    );

    const auto pButton_pickDataDir = new Fl_Button(rx - 110, ty + 30, 80, 30, "Browse");
    pButton_pickDataDir->callback(
        []([[maybe_unused]] Fl_Widget* const pWidget, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;

            const auto pFileChooser = std::make_unique<Fl_Native_File_Chooser>();
            pFileChooser->type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
            pFileChooser->title("Choose a directory containing files for a PsyDoom mod");

            if ((pFileChooser->show() == 0) && (pFileChooser->count() == 1)) {
                tab.pInput_dataDir->value(pFileChooser->filename());
            }
        },
        &tab
    );

    const int dataDirInputRx = getRectExtents(*pButton_pickDataDir).lx - 10;
    tab.pInput_dataDir = makeFl_Input(lx, ty + 30, dataDirInputRx - lx, 30);
    tab.pInput_dataDir->tooltip(pLabel_dataDir->tooltip());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'tweaks and cheats' widgets
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeTweaksAndCheatsWidgets(Tab_Launcher& tab, const int x, const int y) noexcept {
    new Fl_Box(FL_NO_BOX, x, y, 170, 30, "Tweaks & cheats");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 170, 110, "");

    tab.pCheck_pistolStart = makeFl_Check_Button(x + 10, y + 40, 150, 30, "  Force pistol starts");
    tab.pCheck_pistolStart->tooltip(
        "Forces pistol starts on all levels when enabled.\n"
        "This setting also affects password generation and multiplayer.\n"
    );

    tab.pCheck_turbo = makeFl_Check_Button(x + 10, y + 70, 150, 30, "  Turbo mode");
    tab.pCheck_turbo->tooltip(
        "When enabled allows the player to move and fire 2x as fast.\n"
        "Doors and platforms also move 2x as fast. Monsters are unaffected."
    );

    tab.pCheck_noMonsters = makeFl_Check_Button(x + 10, y + 100, 150, 30, "  No monsters");
    tab.pCheck_noMonsters->tooltip(
        "Removes monsters from all levels.\n"
        "Note: this might make some maps impossible to complete without cheats!"
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the launch button
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLaunchButton(Tab_Launcher& tab, const int lx, const int rx, const int ty) noexcept {
    tab.pButton_launch = new Fl_Button(lx, ty, rx - lx, 40, "Launch!");
    tab.pButton_launch->callback([](Fl_Widget* const pWidget, [[maybe_unused]] void* const pUserData) noexcept {
        // Note: set the button user data to '1' to signify that the game should be launched
        pWidget->user_data((void*) 1);
        pWidget->window()->hide();
    });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Launcher' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Launcher& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);

    makeCueFileSelector(tab, tabRect.lx + 20, tabRect.rx - 20, 50);
    makeModDataDirSelector(tab, tabRect.lx + 20, tabRect.rx - 20, 110);
    makeTweaksAndCheatsWidgets(tab, tabRect.lx + 20, 180);
    makeLaunchButton(tab, tabRect.rx - 210, tabRect.rx - 20, tabRect.by - 50);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
