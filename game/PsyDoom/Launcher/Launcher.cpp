//------------------------------------------------------------------------------------------------------------------------------------------
// Starts up and runs the game launcher.
// The game launcher allows program arguments and configuration to be edited via a GUI.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Launcher.h"

#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Doom/psx_main.h"
#include "PsyDoom/Utils.h"

#include <memory>

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl.H>
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Button.H>
    #include <FL/Fl_Double_Window.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Input.H>
    #include <FL/Fl_Native_File_Chooser.H>
    #include <FL/Fl_Tabs.H>
    #include <FL/Fl_Tooltip.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// The command line args to launch the game with
//------------------------------------------------------------------------------------------------------------------------------------------
std::vector<std::string> gProgArgs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Dimensions of the launcher window
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int WINDOW_W = 960;
static constexpr int WINDOW_H = 600;

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds all of the widgets in the Launcher UI
//------------------------------------------------------------------------------------------------------------------------------------------
struct Widgets {
    std::unique_ptr<Fl_Window>      pWindow;
    Fl_Tabs*                        pTabs;

    struct Tab_Launcher {
        Fl_Group*   pTab;
        Fl_Box*     pLabel_cue;
        Fl_Input*   pInput_cue;
        Fl_Button*  pButton_cuePick;
        Fl_Button*  pButton_cueClear;
        Fl_Button*  pButton_launch;
    } tab_launcher;

    struct Tab_Graphics {
        Fl_Group*   pTab;
    } tab_graphics;

    struct Tab_Game {
        Fl_Group*   pTab;
    } tab_game;

    struct Tab_Input {
        Fl_Group*   pTab;
    } tab_input;

    struct Tab_Controls {
        Fl_Group*   pTab;
    } tab_controls;

    struct Tab_Audio {
        Fl_Group*   pTab;
    } tab_audio;

    struct Tab_Cheats {
        Fl_Group*   pTab;
    } tab_cheats;
};

Widgets gWidgets = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// The result of executing the launcher.
// Tells whether the game itself should be run or whether the app should quit.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class LauncherResult {
    Exit,
    RunGame
};

static LauncherResult gLauncherResult = {};

//------------------------------------------------------------------------------------------------------------------------------------------
// Stores pixel min/max extents on the x and y axes for a rectangular area
//------------------------------------------------------------------------------------------------------------------------------------------
struct RectExtents {
    int lx;
    int rx;
    int ty;
    int by;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells whether the launcher should be run.
// The command line arguments to the program are passed as parameters.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool shouldRunLauncher(const int argc, [[maybe_unused]] const char* const* const argv) noexcept {
    // Show the launcher in all situations unless program arguments (besides the executable name) are passed
    return (argc <= 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the extents for a specified FLTK widget
//------------------------------------------------------------------------------------------------------------------------------------------
static RectExtents getRectExtents(const Fl_Widget& widget) noexcept {
    RectExtents extents = {};
    extents.lx = widget.x();
    extents.rx = extents.lx + widget.w();
    extents.ty = widget.y();
    extents.by = extents.ty + widget.h();
    return extents;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the rectangular area occupied by text drawn for the given widgets label.
// This can be used to help layout other widgets.
//------------------------------------------------------------------------------------------------------------------------------------------
static RectExtents getTextDrawExtents(const Fl_Widget& widget) noexcept {
    // Get the text size
    int textW = {};
    int textH = {};
    widget.measure_label(textW, textH);

    // Figure out where that text appears on both axes.
    // Start with the x-axis:
    const Fl_Align align = widget.align();
    int x = widget.x();
    int y = widget.y();

    if (align & FL_ALIGN_LEFT) {
        if ((align & FL_ALIGN_INSIDE) == 0) {
            x -= textW;
        }
    }
    else if (align & FL_ALIGN_RIGHT) {
        x += widget.w();

        if (align & FL_ALIGN_INSIDE) {
            x -= textW;
        }
    }
    else {
        // The text is centered on the x-axis
        x += (widget.w() - textW) / 2;
    }

    // Text y-axis position:
    if (align & FL_ALIGN_TOP) {
        if ((align & FL_ALIGN_INSIDE) == 0) {
            y -= textH;
        }
    }
    else if (align & FL_ALIGN_BOTTOM) {
        y += widget.h();

        if (align & FL_ALIGN_INSIDE) {
            y -= textH;
        }
    }
    else {
        // The text is centered on the y-axis
        y += (widget.h() - textH) / 2;
    }

    // Return the computed extents
    RectExtents extents = {};
    extents.lx = x;
    extents.rx = x + textW;
    extents.ty = y;
    extents.by = y + textH;

    return extents;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes FLTK global style settings
//------------------------------------------------------------------------------------------------------------------------------------------
static void initFltkGlobalStyleSettings() noexcept {
    // High level theme
    Fl::scheme("gtk+");

    // Color scheme
    Fl::set_color(FL_BACKGROUND_COLOR, 60, 60, 60);
    Fl::set_color(FL_WHITE, 90, 90, 90);
    Fl::set_color(FL_BLACK, 20, 20, 20);
    Fl::set_color(FL_DARK1, 50, 50, 50);
    Fl::set_color(FL_DARK2, 40, 40, 40);
    Fl::set_color(FL_LIGHT1, 70, 70, 70);
    Fl::set_color(FL_LIGHT2, 80, 80, 80);
    Fl::set_color(FL_FOREGROUND_COLOR, 220, 220, 220);

    // Don't draw dotted lines around focused widgets
    Fl::visible_focus(0);

    // Request hardware double buffering
    Fl::visual(FL_DOUBLE | FL_INDEX);

    // Tooltip style
    Fl_Tooltip::wrap_width(800);
    Fl_Tooltip::hidedelay(30.0f);
    Fl_Tooltip::color(FL_LIGHT2);
    Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Launcher' tab
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateLauncherTab(Widgets::Tab_Launcher& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabArea = getRectExtents(*tab.pTab);

    // Cue file selection ('-cue' parameter)
    tab.pLabel_cue = new Fl_Box(FL_NO_BOX, tabArea.lx + 20, tabArea.ty + 20, 150, 30, "Game disc .cue file");
    tab.pLabel_cue->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    tab.pLabel_cue->tooltip(
        "Path to the game disc's .cue file.\n"
        "The game disc supplied must be PlayStation 'Doom', 'Final Doom', or any other supported mod.\n"
        "See PsyDoom's main README.md file for a list of all supported game discs.\n"
        "If this field is unspecified then the default .cue file will be used instead.\n"
        "This default .cue file can be changed under the 'Game' config tab.\n"
        "\n"
        "Command line argument:\n"
        "   -cue <CUE_FILE_PATH>"
    );

    tab.pButton_cuePick = new Fl_Button(tabArea.rx - 120, tabArea.ty + 20, 80, 30, "Browse");
    tab.pButton_cuePick->callback(
        []([[maybe_unused]] Fl_Widget* const pWidget, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Widgets::Tab_Launcher& tab = *(Widgets::Tab_Launcher*) pUserData;

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

    tab.pButton_cueClear = new Fl_Button(tabArea.rx - 40, tabArea.ty + 20, 30, 30, "X");
    tab.pButton_cueClear->callback(
        []([[maybe_unused]] Fl_Widget* const pWidget, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Widgets::Tab_Launcher& tab = *(Widgets::Tab_Launcher*) pUserData;
            tab.pInput_cue->value("");
        },
        &tab
    );

    const int cueInputLx = getTextDrawExtents(*tab.pLabel_cue).rx + 20;
    const int cueInputRx = getRectExtents(*tab.pButton_cuePick).lx - 10;

    tab.pInput_cue = new Fl_Input(cueInputLx, tabArea.ty + 20, cueInputRx - cueInputLx, 30);
    tab.pInput_cue->color(FL_DARK1, FL_DARK1);
    tab.pInput_cue->selection_color(FL_LIGHT1);
    tab.pInput_cue->tooltip(tab.pLabel_cue->tooltip());

    // Launch button
    tab.pButton_launch = new Fl_Button(tabArea.rx - 210, tabArea.by - 50, 200, 40, "Launch!");
    tab.pButton_launch->callback([](Fl_Widget* const pWidget, [[maybe_unused]] void* const pUserData) noexcept {
        gLauncherResult = LauncherResult::RunGame;
        pWidget->window()->hide();
    });
}

static void populateGraphicsTab(Widgets::Tab_Graphics& tab) noexcept {
    // TODO
}

static void populateGameTab(Widgets::Tab_Game& tab) noexcept {
    // TODO
}

static void populateInputTab(Widgets::Tab_Input& tab) noexcept {
    // TODO
}

static void populateControlsTab(Widgets::Tab_Controls& tab) noexcept {
    // TODO
}

static void populateAudioTab(Widgets::Tab_Audio& tab) noexcept {
    // TODO
}

static void populateCheatsTab(Widgets::Tab_Cheats& tab) noexcept {
    // TODO
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes all the tabs for the launcher window
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLauncherTabs(Widgets& widgets) noexcept {
    // N.B: the window must be the 'current' group or the widgets below will leak
    ASSERT(Fl_Group::current() == widgets.pWindow.get());

    // Create the tab container
    Fl_Window& window = *widgets.pWindow;
    const int winW = window.w();
    const int winH = window.h();

    widgets.pTabs = new Fl_Tabs(10, 10, winW - 20, winH - 20);
    widgets.pTabs->color(FL_BACKGROUND_COLOR, FL_LIGHT1);

    // Create and populate each individual tab (these will be added to the tab container)
    const auto makeTab = [=](auto& tabWidgets, const auto& populateTabFunc, const char* const label) noexcept {
        tabWidgets.pTab = new Fl_Group(10, 40, winW - 20, winH - 50, label);
        populateTabFunc(tabWidgets);
        tabWidgets.pTab->end();
    };

    makeTab(widgets.tab_launcher, populateLauncherTab, "  Launcher  ");
    makeTab(widgets.tab_graphics, populateGraphicsTab, "  Graphics  ");
    makeTab(widgets.tab_game, populateGameTab, "  Game  ");
    makeTab(widgets.tab_input, populateInputTab, "  Input  ");
    makeTab(widgets.tab_controls, populateControlsTab, "  Controls  ");
    makeTab(widgets.tab_audio, populateAudioTab, "  Audio  ");
    makeTab(widgets.tab_cheats, populateCheatsTab, "  Cheats  ");

    // Done adding tabs to the tab container
    widgets.pTabs->end();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the window for the launcher and saves it inside the given Launcher widgets struct
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLauncherWindow(Widgets& widgets, const int winW, const int winH) noexcept {
    // Get the screen dimensions
    int screenX = {}, screenY = {};
    int screenW = {}, screenH = {};
    Fl::screen_work_area(screenX, screenY, screenW, screenH);

    // Make the window center in the middle of the screen
    widgets.pWindow = std::make_unique<Fl_Double_Window>((screenW - winW) / 2, (screenH - winH) / 2, winW, winH);
    widgets.pWindow->label(Utils::getGameVersionString());

    // Make all the tabs and finish up creating the window
    makeLauncherTabs(widgets);
    widgets.pWindow->end();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the launcher dialog.
// Returns 'true' if the main program should proceed afterwards, 'false' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
static void runLauncher(const int argc, const char* const* const argv) noexcept {
    // We will exit the game unless the 'Run Game!' button is pressed
    gLauncherResult = LauncherResult::Exit;

    // Some global FLTK setup
    initFltkGlobalStyleSettings();

    // Create and run the launcher window
    gWidgets = {};
    makeLauncherWindow(gWidgets, WINDOW_W, WINDOW_H);
    gWidgets.pWindow->show();
    Fl::run();

    // TODO: makeup new program launch arguments (based on chosen settings)
    // TODO: move this into a proper function
    if (const char* const pCuePath = gWidgets.tab_launcher.pInput_cue->value()) {
        // TODO: trim this argument
        if (pCuePath[0]) {
            gProgArgs.push_back("-cue");
            gProgArgs.push_back(pCuePath);
        }
    }

    // Cleanup
    gWidgets = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the game launcher if applicable, followed by the game itself afterwards (if appropriate).
// Returns the error code resulting from this execution.
//------------------------------------------------------------------------------------------------------------------------------------------
int launcherMain(const int argc, const char* const* const argv) noexcept {
    // Copy the input arguments to the program arguments list.
    // The launcher may add to this list if it wants.
    gProgArgs = {};

    for (int i = 0; i < argc; ++i) {
        if (argv[i]) {
            gProgArgs.push_back(argv[i]);
        }
    }

    // Run the launcher (if appropriate) and exit if it says so
    if (shouldRunLauncher(argc, argv)) {
        runLauncher(argc, argv);

        if (gLauncherResult == LauncherResult::Exit)
            return 0;
    }

    // Get the pointers for all arguments so we can pass them in standard C format
    std::vector<const char*> progArgPtrs;
    progArgPtrs.reserve(gProgArgs.size());

    for (const std::string& arg : gProgArgs) {
        progArgPtrs.push_back(arg.data());
    }

    // Run the game!
    return psx_main((int) progArgPtrs.size(), progArgPtrs.data());
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
