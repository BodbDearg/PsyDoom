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
#include "LauncherPrefs.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Config/ConfigSerialization.h"
#include "PsyDoom/Controls.h"
#include "PsyDoom/Utils.h"

#include <memory>

#if _WIN32
    #include <Windows.h>
#endif

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl.H>
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Button.H>
    #include <FL/Fl_Double_Window.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Input.H>
    #include <FL/Fl_Int_Input.H>
    #include <FL/Fl_Native_File_Chooser.H>
    #include <FL/Fl_Pixmap.H>
    #include <FL/Fl_Tabs.H>
    #include <FL/Fl_Tooltip.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Linux: embedded icons
//------------------------------------------------------------------------------------------------------------------------------------------
#ifdef __linux__
    #include "Resources/Linux/icon_8.raw_rgb888.c"
    #include "Resources/Linux/icon_16.raw_rgb888.c"
    #include "Resources/Linux/icon_20.raw_rgb888.c"
    #include "Resources/Linux/icon_22.raw_rgb888.c"
    #include "Resources/Linux/icon_24.raw_rgb888.c"
    #include "Resources/Linux/icon_28.raw_rgb888.c"
    #include "Resources/Linux/icon_30.raw_rgb888.c"
    #include "Resources/Linux/icon_32.raw_rgb888.c"
    #include "Resources/Linux/icon_36.raw_rgb888.c"
    #include "Resources/Linux/icon_40.raw_rgb888.c"
    #include "Resources/Linux/icon_42.raw_rgb888.c"
    #include "Resources/Linux/icon_44.raw_rgb888.c"
    #include "Resources/Linux/icon_48.raw_rgb888.c"
    #include "Resources/Linux/icon_56.raw_rgb888.c"
    #include "Resources/Linux/icon_60.raw_rgb888.c"
    #include "Resources/Linux/icon_64.raw_rgb888.c"
    #include "Resources/Linux/icon_72.raw_rgb888.c"
    #include "Resources/Linux/icon_80.raw_rgb888.c"
    #include "Resources/Linux/icon_96.raw_rgb888.c"
    #include "Resources/Linux/icon_128.raw_rgb888.c"
    #include "Resources/Linux/icon_160.raw_rgb888.c"
    #include "Resources/Linux/icon_192.raw_rgb888.c"
    #include "Resources/Linux/icon_256.raw_rgb888.c"
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Dimensions of the launcher window
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int WINDOW_W = 960;
static constexpr int WINDOW_H = 610;

//------------------------------------------------------------------------------------------------------------------------------------------
// These are defined in the various launcher tab .cpp files
//------------------------------------------------------------------------------------------------------------------------------------------
void populateLauncherTab(Context& ctx) noexcept;
void populateGraphicsTab(Context& ctx) noexcept;
void populateGameTab(Context& ctx) noexcept;
void populateInputTab(Context& ctx) noexcept;
void populateControlsTab(Context& ctx) noexcept;
void populateAudioTab(Context& ctx) noexcept;
void populateCheatsTab(Context& ctx) noexcept;
void populateMultiplayerTab(Context& ctx) noexcept;

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
    ctx.pTabs->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            // MacOS: workaround an FLTK bug where the logo does not appear sometimes after switching tabs.
            // Force FLTK to redraw everything after a tab switch.
            ASSERT(pUserData);
            Context& ctx = *static_cast<Context*>(pUserData);
            ctx.pWindow->damage(FL_DAMAGE_ALL);
        },
        &ctx
    );

    // Create and populate each individual tab (these will be added to the tab container)
    const auto makeTab = [=](
        Context& ctx,
        auto& tabWidgets,
        const auto& populateTabFunc,
        const char* const label
    ) noexcept {
        tabWidgets.pTab = new Fl_Group(10, 40, winW - 20, winH - 50, label);
        populateTabFunc(ctx);
        tabWidgets.pTab->end();
    };

    makeTab(ctx, ctx.tab_launcher, populateLauncherTab, "  Launcher  ");
    makeTab(ctx, ctx.tab_graphics, populateGraphicsTab, "  Graphics  ");
    makeTab(ctx, ctx.tab_game, populateGameTab, "  Game  ");
    makeTab(ctx, ctx.tab_input, populateInputTab, "  Input  ");
    makeTab(ctx, ctx.tab_controls, populateControlsTab,  "  Controls  ");
    makeTab(ctx, ctx.tab_audio, populateAudioTab, "  Audio  ");
    makeTab(ctx, ctx.tab_cheats, populateCheatsTab, "  Cheats  ");
    makeTab(ctx, ctx.tab_multiplayer, populateMultiplayerTab, "  Multiplayer  ");

    // Done adding tabs to the tab container
    ctx.pTabs->end();
    
    // MacOS: workaround an FLTK bug where the logo does not appear sometimes on startup.
    // Force FLTK to redraw everything.
    ctx.pWindow->damage(FL_DAMAGE_ALL);
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

    // Windows: set the icon for the window using the resource file
    #if _WIN32
        if (ctx.pWindow) {
            ctx.pWindow->icon(LoadIcon(GetModuleHandle(nullptr), L"IDI_ICON1"));
        }
    #endif

    // Linux: set the icons for the window using embedded icon files.
    // Use a ton of different sizes and let FLTK handle the details of chosing the best ones.
    #ifdef __linux__
        if (ctx.pWindow) {
            const Fl_RGB_Image icon8(gIcon_8_raw_rgb888, 8, 8, 3);
            const Fl_RGB_Image icon16(gIcon_16_raw_rgb888, 16, 16, 3);
            const Fl_RGB_Image icon20(gIcon_20_raw_rgb888, 20, 20, 3);
            const Fl_RGB_Image icon22(gIcon_22_raw_rgb888, 22, 22, 3);
            const Fl_RGB_Image icon24(gIcon_24_raw_rgb888, 24, 24, 3);
            const Fl_RGB_Image icon28(gIcon_28_raw_rgb888, 28, 28, 3);
            const Fl_RGB_Image icon30(gIcon_30_raw_rgb888, 30, 30, 3);
            const Fl_RGB_Image icon32(gIcon_32_raw_rgb888, 32, 32, 3);
            const Fl_RGB_Image icon36(gIcon_36_raw_rgb888, 36, 36, 3);
            const Fl_RGB_Image icon40(gIcon_40_raw_rgb888, 40, 40, 3);
            const Fl_RGB_Image icon42(gIcon_42_raw_rgb888, 42, 42, 3);
            const Fl_RGB_Image icon44(gIcon_44_raw_rgb888, 44, 44, 3);
            const Fl_RGB_Image icon48(gIcon_48_raw_rgb888, 48, 48, 3);
            const Fl_RGB_Image icon56(gIcon_56_raw_rgb888, 56, 56, 3);
            const Fl_RGB_Image icon60(gIcon_60_raw_rgb888, 60, 60, 3);
            const Fl_RGB_Image icon64(gIcon_64_raw_rgb888, 64, 64, 3);
            const Fl_RGB_Image icon72(gIcon_72_raw_rgb888, 72, 72, 3);
            const Fl_RGB_Image icon80(gIcon_80_raw_rgb888, 80, 80, 3);
            const Fl_RGB_Image icon96(gIcon_96_raw_rgb888, 96, 96, 3);
            const Fl_RGB_Image icon128(gIcon_128_raw_rgb888, 128, 128, 3);
            const Fl_RGB_Image icon160(gIcon_160_raw_rgb888, 160, 160, 3);
            const Fl_RGB_Image icon192(gIcon_192_raw_rgb888, 192, 192, 3);
            const Fl_RGB_Image icon256(gIcon_256_raw_rgb888, 256, 256, 3);

            const Fl_RGB_Image* iconPtrs[] = {
                &icon8, &icon16, &icon20, &icon22,
                &icon24, &icon28, &icon30, &icon32,
                &icon36, &icon40, &icon42, &icon44,
                &icon48, &icon56, &icon60, &icon64,
                &icon72, &icon80, &icon96, &icon128,
                &icon160, &icon192, &icon256
            };

            ctx.pWindow->icons(iconPtrs, C_ARRAY_SIZE(iconPtrs));
        }
    #endif

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
    const std::string demoToPlay = trimText(ctx.demoFileToPlay.c_str());

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

    // Note: '-playdemo' and '-record' are conflicting arguments.
    // If playing a demo is specified then don't pass '-record', regardless of the UI toggle state.
    if (demoToPlay.length() > 0) {
        programArgs.push_back("-playdemo");
        programArgs.push_back(demoToPlay);
    }
    else if (ctx.tab_launcher.pCheck_recordDemos->value() != 0) {
        programArgs.push_back("-record");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the launcher dialog
//------------------------------------------------------------------------------------------------------------------------------------------
static LauncherResult runLauncher(std::vector<std::string>& programArgs) noexcept {
    // Some global FLTK setup
    initFltkGlobalStyleSettings();

    // Initialize controls and config subsystems first since they feed into the UI state
    Controls::init();
    Config::init();

    // Initialize the launcher's context and initially assume we will exit the game unless a command is issued otherwise
    Context ctx = {};
    ctx.launcherResult = LauncherResult::Exit;

    // Create the launcher window
    makeLauncherWindow(ctx, WINDOW_W, WINDOW_H);

    // Load previous launcher settings and apply them to the 'Launcher' page
    LauncherPrefs::load(ctx.tab_launcher);

    // Run the launcher
    ctx.pWindow->show();
    Fl::run();

    // This fixes a bug on Linux where the Window doesn't go away after being closed
    Fl::flush();

    // Save any changes to launcher preferences and to game config itself
    if (LauncherPrefs::shouldSave(ctx.tab_launcher)) {
        LauncherPrefs::save(ctx.tab_launcher);
    }

    ConfigSerialization::writeAllConfigFiles(false);

    // Add launch arguments specified by the user via the UI
    addLauncherProgramArgs(ctx, programArgs);

    // Cleanup the controls and config systems if we are not going to launch the game.
    // Normally the game handles cleaning up these, but if we are not launching the game then this module needs to handle that.
    if (ctx.launcherResult != LauncherResult::RunGame) {
        Config::shutdown();
        Controls::shutdown();
    }

    return ctx.launcherResult;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the game and returns the exit code from it
//------------------------------------------------------------------------------------------------------------------------------------------
static int runGame(const std::vector<std::string>& programArgs) noexcept {
    // Need to convert the program arguments to C-style program arguments first!
    const std::vector<const char*> cProgramArgs = toCProgramArgs(programArgs);
    return psx_main((int) cProgramArgs.size(), cProgramArgs.data());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the game launcher if applicable, followed by the game itself afterwards (if appropriate).
// Returns the error code resulting from this execution.
//------------------------------------------------------------------------------------------------------------------------------------------
int launcherMain(const int argc, const char* const* const argv) noexcept {
    // Copy the input program arguments: the launcher will append to this list if appropriate
    const std::vector<std::string> origProgramArgs = copyCProgramArgs(argc, argv);

    // Run the game directly without showing the launcher?
    if (!shouldRunLauncher(origProgramArgs))
        return runGame(origProgramArgs);

    // Keep running the launcher until starting the game is requested or quit is requested.
    // The launcher can restart itself multiple times if config is reset back to 'factory defaults' via the UI.
    while (true) {
        // Run the launcher: note that it might append it's own arguments to the original args list
        std::vector<std::string> programArgs = origProgramArgs;
        const LauncherResult result = runLauncher(programArgs);

        // If the launcher wants to quit the app then do so now
        if (result == LauncherResult::Exit)
            return 0;
        
        // Run the game if that was requested, and finish up by returning the result of execution:
        if (result == LauncherResult::RunGame)
            return runGame(programArgs);

        // Otherwise we run the launcher again
        ASSERT(result == LauncherResult::RunLauncher);
    }

    // Unreachable: if we do end up here then something has gone horribly wrong!
    return -1;
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
