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
    #include <FL/Fl_Button.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Tabs.H>
    #include <FL/Fl_Tooltip.H>
    #include <FL/Fl_Window.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

// Dimensions of the launcher window
static constexpr int WINDOW_W = 760;
static constexpr int WINDOW_H = 600;

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
// Initializes FLTK global style settings
//------------------------------------------------------------------------------------------------------------------------------------------
static void initFltkGlobalStyleSettings() noexcept {
    // High level theme
    Fl::scheme("gtk+");

    // Color scheme
    Fl::set_color(FL_BACKGROUND_COLOR, 60, 60, 60);
    Fl::set_color(FL_WHITE, 90, 90, 90);
    Fl::set_color(FL_BLACK, 20, 20, 20);
    Fl::set_color(FL_LIGHT1, 70, 70, 70);
    Fl::set_color(FL_LIGHT2, 80, 80, 80);
    Fl::set_color(FL_FOREGROUND_COLOR, 220, 220, 220);

    // Don't draw dotted lines around focused widgets
    Fl::visible_focus(0);

    // Tooltip style
    Fl_Tooltip::wrap_width(800);
    Fl_Tooltip::color(FL_LIGHT2);
    Fl_Tooltip::textcolor(FL_FOREGROUND_COLOR);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Launcher' tab
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateLauncherTab(Fl_Group& tab) noexcept {
    ASSERT(Fl_Group::current() == &tab);
    const RectExtents tabArea = getRectExtents(tab);

    Fl_Button* const pBtn_Launch = new Fl_Button(tabArea.rx - 210, tabArea.by - 50, 200, 40, "Launch!");
    pBtn_Launch->callback([](Fl_Widget* const pWidget, [[maybe_unused]] void* const pUserData){
        gLauncherResult = LauncherResult::RunGame;
        pWidget->window()->hide();
    });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes all the tabs for the launcher window
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLauncherTabs(Fl_Window& window) noexcept {
    // N.B: the window must be the 'current' group or the widgets below will leak
    ASSERT(Fl_Group::current() == &window);

    const int winW = window.w();
    const int winH = window.h();

    Fl_Tabs* const pTabs = new Fl_Tabs(10, 10, winW - 20, winH - 20);
    pTabs->color(FL_BACKGROUND_COLOR, FL_LIGHT1);

    {
        Fl_Group* const pTab_Launcher = new Fl_Group(10, 40, winW - 20, winH - 50, "  Launcher  ");
        populateLauncherTab(*pTab_Launcher);
        pTab_Launcher->end();

        Fl_Group* const pTab_Graphics = new Fl_Group(10, 40, winW - 20, winH - 50, "  Graphics  ");
        pTab_Graphics->end();
    
        Fl_Group* const pTab_Game = new Fl_Group(10, 40, winW - 20, winH - 50, "  Game  ");
        pTab_Game->end();
    
        Fl_Group* const pTab_Input = new Fl_Group(10, 40, winW - 20, winH - 50, "  Input  ");
        pTab_Input->end();
    
        Fl_Group* const pTab_Controls = new Fl_Group(10, 40, winW - 20, winH - 50, "  Controls  ");
        pTab_Controls->end();
    
        Fl_Group* const pTab_Audio = new Fl_Group(10, 40, winW - 20, winH - 50, "  Audio  ");
        pTab_Audio->end();

        Fl_Group* const pTab_Cheats = new Fl_Group(10, 40, winW - 20, winH - 50, "  Cheats  ");
        pTab_Cheats->end();
    }

    pTabs->end();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the window for the launcher
//------------------------------------------------------------------------------------------------------------------------------------------
static std::unique_ptr<Fl_Window> makeLauncherWindow(const int winW, const int winH) noexcept {
    // Get the screen dimensions
    int screenX = {}, screenY = {};
    int screenW = {}, screenH = {};
    Fl::screen_work_area(screenX, screenY, screenW, screenH);

    // Make the window center in the middle of the screen
    auto pWindow = std::make_unique<Fl_Window>((screenW - winW) / 2, (screenH - winH) / 2, winW, winH);
    pWindow->label(Utils::getGameVersionString());

    // Make all the tabs and finish up creating the window
    makeLauncherTabs(*pWindow);
    pWindow->end();

    return pWindow;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the launcher dialog.
// Returns 'true' if the main program should proceed afterwards, 'false' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
static void runLauncher(const int argc, const char* const* const argv) noexcept {
    // We will exit the game unless the 'Run Game!' button is pressed
    gLauncherResult = LauncherResult::Exit;

    // Create and run the launcher window
    initFltkGlobalStyleSettings();
    const auto pWindow = makeLauncherWindow(WINDOW_W, WINDOW_H);
    pWindow->show();
    Fl::run();

    // TODO: makeup new program launch arguments (based on chosen settings)
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the game launcher if applicable, followed by the game itself afterwards (if appropriate).
// Returns the error code resulting from this execution.
//------------------------------------------------------------------------------------------------------------------------------------------
int launcherMain(const int argc, const char* const* const argv) noexcept {
    if (shouldRunLauncher(argc, argv)) {
        runLauncher(argc, argv);

        if (gLauncherResult == LauncherResult::Exit)
            return 0;
    }

    return psx_main(argc, argv);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
