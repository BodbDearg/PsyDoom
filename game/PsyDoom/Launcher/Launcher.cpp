//------------------------------------------------------------------------------------------------------------------------------------------
// Starts up and runs the game launcher.
// The game launcher allows program arguments and configuration to be edited via a GUI.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Launcher.h"

#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Doom/psx_main.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Utils.h"

#include <memory>

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl.H>
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Button.H>
    #include <FL/Fl_Check_Button.H>
    #include <FL/Fl_Double_Window.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Input.H>
    #include <FL/Fl_Int_Input.H>
    #include <FL/Fl_Native_File_Chooser.H>
    #include <FL/Fl_Tabs.H>
    #include <FL/Fl_Tooltip.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Dimensions of the launcher window
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int WINDOW_W = 960;
static constexpr int WINDOW_H = 600;

//------------------------------------------------------------------------------------------------------------------------------------------
// These are defined in the various launcher tab .cpp files
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Launcher& tab) noexcept;
void populate(Tab_Graphics& tab) noexcept;
void populate(Tab_Game& tab) noexcept;
void populate(Tab_Input& tab) noexcept;
void populate(Tab_Controls& tab) noexcept;
void populate(Tab_Audio& tab) noexcept;
void populate(Tab_Cheats& tab) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// The result of executing the launcher.
// Tells whether the game itself should be run or whether the app should quit.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class LauncherResult {
    Exit,
    RunGame
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies the given list of C-style program arguments, returning them in std::vector format
//------------------------------------------------------------------------------------------------------------------------------------------
static std::vector<std::string> copyCProgramArgs(const int argc, const char* const* const argv) noexcept {
    ASSERT(argc >= 0);
    std::vector<std::string> programArgs;
    programArgs.reserve((size_t) argc);

    for (int i = 0; i < argc; ++i) {
        if (argv[i]) {
            programArgs.push_back(argv[i]);
        }
    }

    return programArgs;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Converts the given list of program arguments to a C-style list of argument pointers
//------------------------------------------------------------------------------------------------------------------------------------------
static std::vector<const char*> toCProgramArgs(const std::vector<std::string>& programArgs) noexcept {
    std::vector<const char*> cProgramArgs;
    cProgramArgs.reserve(programArgs.size());

    for (const std::string& arg : programArgs) {
        if (!arg.empty()) {
            cProgramArgs.push_back(arg.data());
        }
    }

    return cProgramArgs;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells whether the launcher should be run.
// The command line arguments to the program are passed as parameters.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool shouldRunLauncher(const std::vector<std::string>& progArgs) noexcept {
    // Show the launcher in all situations unless program arguments (besides the executable name) are passed
    return (progArgs.size() <= 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes FLTK global style settings
//------------------------------------------------------------------------------------------------------------------------------------------
static void initFltkGlobalStyleSettings() noexcept {
    // High level theme
    Fl::scheme("gtk+");

    // Color scheme
    Fl::set_color(FL_BACKGROUND_COLOR, 60, 60, 60);
    Fl::set_color(FL_BACKGROUND2_COLOR, 50, 50, 50);
    Fl::set_color(FL_WHITE, 90, 90, 90);
    Fl::set_color(FL_BLACK, 20, 20, 20);
    Fl::set_color(FL_DARK1, 50, 50, 50);
    Fl::set_color(FL_DARK2, 40, 40, 40);
    Fl::set_color(FL_LIGHT1, 70, 70, 70);
    Fl::set_color(FL_LIGHT2, 80, 80, 80);
    Fl::set_color(FL_FOREGROUND_COLOR, 220, 220, 220);
    Fl::set_color(FL_SELECTION_COLOR, 220, 220, 220);
    
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
// Makes all the tabs for the launcher window
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLauncherTabs(Context& ctx) noexcept {
    // N.B: the window must be the 'current' group or the widgets below will leak
    ASSERT(Fl_Group::current() == ctx.pWindow.get());

    // Create the tab container
    Fl_Window& window = *ctx.pWindow;
    const int winW = window.w();
    const int winH = window.h();

    ctx.pTabs = new Fl_Tabs(10, 10, winW - 20, winH - 20);
    ctx.pTabs->color(FL_BACKGROUND_COLOR, FL_LIGHT1);

    // Create and populate each individual tab (these will be added to the tab container)
    const auto makeTab = [=](auto& tabWidgets, const char* const label) noexcept {
        tabWidgets.pTab = new Fl_Group(10, 40, winW - 20, winH - 50, label);
        populate(tabWidgets);
        tabWidgets.pTab->end();
    };

    makeTab(ctx.tab_launcher, "  Launcher  ");
    makeTab(ctx.tab_graphics, "  Graphics  ");
    makeTab(ctx.tab_game, "  Game  ");
    makeTab(ctx.tab_input, "  Input  ");
    makeTab(ctx.tab_controls,  "  Controls  ");
    makeTab(ctx.tab_audio, "  Audio  ");
    makeTab(ctx.tab_cheats, "  Cheats  ");

    // Done adding tabs to the tab container
    ctx.pTabs->end();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the window for the launcher and saves it inside the given Launcher widgets struct
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLauncherWindow(Context& ctx, const int winW, const int winH) noexcept {
    // Get the screen dimensions
    int screenX = {}, screenY = {};
    int screenW = {}, screenH = {};
    Fl::screen_work_area(screenX, screenY, screenW, screenH);

    // Make the window center in the middle of the screen
    ctx.pWindow = std::make_unique<Fl_Double_Window>((screenW - winW) / 2, (screenH - winH) / 2, winW, winH);
    ctx.pWindow->label(Utils::getGameVersionString());

    // Make all the tabs and finish up creating the window
    makeLauncherTabs(ctx);
    ctx.pWindow->end();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds the program arguments specified by the user (via the launcher) to the given list of program arguments
//------------------------------------------------------------------------------------------------------------------------------------------
static void addLauncherProgramArgs(Context& ctx, std::vector<std::string>& programArgs) noexcept {
    const std::string cuePath = trimText(ctx.tab_launcher.pInput_cue->value());
    const std::string dataDirPath = trimText(ctx.tab_launcher.pInput_dataDir->value());
    const std::string netHost = trimText(ctx.tab_launcher.pInput_netHost->value());
    const std::string netPort = trimText(ctx.tab_launcher.pInput_netPort->value());
    const std::string demoToPlay = trimText(ctx.tab_launcher.demoFileToPlay.c_str());

    if (!cuePath.empty()) {
        programArgs.push_back("-cue");
        programArgs.push_back(cuePath);
    }

    if (!dataDirPath.empty()) {
        programArgs.push_back("-datadir");
        programArgs.push_back(dataDirPath);
    }

    if (ctx.tab_launcher.pCheck_pistolStart->value()) {
        programArgs.push_back("-pistolstart");
    }

    if (ctx.tab_launcher.pCheck_turbo->value()) {
        programArgs.push_back("-turbo");
    }

    if (ctx.tab_launcher.pCheck_noMonsters->value()) {
        programArgs.push_back("-nomonsters");
    }

    if (ctx.tab_launcher.pChoice_netPeerType->value() != 0) {
        // Server peer
        programArgs.push_back("-server");

        if (!netPort.empty()) {
            programArgs.push_back(netPort);
        }
    }
    else {
        // Client peer: a host or port must be specified however
        if ((netHost.length() > 0) || (netPort.length() > 0)) {
            programArgs.push_back("-client");

            if (netPort.empty()) {
                programArgs.push_back(netHost);
            } else {
                if (netHost.empty()) {
                    programArgs.push_back("localhost:" + netPort);
                } else {
                    programArgs.push_back(netHost + ":" + netPort);
                }
            }
        }
    }

    if (ctx.tab_launcher.pCheck_recordDemos->value() != 0) {
        programArgs.push_back("-record");
    }

    if (demoToPlay.length() > 0) {
        programArgs.push_back("-playdemo");
        programArgs.push_back(demoToPlay);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the launcher dialog
//------------------------------------------------------------------------------------------------------------------------------------------
static LauncherResult runLauncher(std::vector<std::string>& programArgs) noexcept {
    // Some global FLTK setup
    initFltkGlobalStyleSettings();

    // Create and run the launcher window
    Context ctx = {};
    makeLauncherWindow(ctx, WINDOW_W, WINDOW_H);
    ctx.pWindow->show();
    Fl::run();

    // Add launch arguments specified by the user via the UI
    addLauncherProgramArgs(ctx, programArgs);

    // TODO: save edits to settings made in the launcher

    // Return whether to run the game or not
    return (ctx.tab_launcher.bLaunchGame) ? LauncherResult::RunGame : LauncherResult::Exit;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the game launcher if applicable, followed by the game itself afterwards (if appropriate).
// Returns the error code resulting from this execution.
//------------------------------------------------------------------------------------------------------------------------------------------
int launcherMain(const int argc, const char* const* const argv) noexcept {
    // Copy the input program arguments: the launcher will append to this list if appropriate
    std::vector<std::string> programArgs = copyCProgramArgs(argc, argv);

    // Run the launcher (if appropriate) and exit if it says so
    if (shouldRunLauncher(programArgs)) {
        if (runLauncher(programArgs) == LauncherResult::Exit)
            return 0;
    }

    // Convert the program arguments back to a C-style format
    const std::vector<const char*> cProgramArgs = toCProgramArgs(programArgs);

    // Run the game using the program arguments determined!
    return psx_main((int) cProgramArgs.size(), cProgramArgs.data());
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
