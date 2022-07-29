//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Launcher' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Utils.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Choice.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Int_Input.H>
    #include <FL/Fl_Native_File_Chooser.H>
    #include <FL/Fl_Pixmap.H>
END_DISABLE_HEADER_WARNINGS

#include <cstdlib>

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// The PsyDoom logo in .xpm format
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* const gLauncherLogo_xpm_data[] = {
    #include "Launcher_Logo.bin.h"
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the PsyDoom logo
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLogo(Tab_Launcher& tab, const int lx, const int rx, const int ty) noexcept {
    tab.pGameLogo = std::make_unique<Fl_Pixmap>(gLauncherLogo_xpm_data);
    const auto pImageBox = new Fl_Box(FL_NO_BOX, lx, ty, rx - lx, 0, "");
    pImageBox->image(tab.pGameLogo.get());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the cue file selector
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeCueFileSelector(Tab_Launcher& tab, const int lx, const int rx, const int ty) noexcept {
    const auto pLabel_cue = new Fl_Box(FL_NO_BOX, lx, ty, 150, 30, "Game disc");
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
    pButton_clearCue->tooltip(pLabel_cue->tooltip());
    pButton_clearCue->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;
            tab.pInput_cue->value("");
            tab.pInput_cue->set_changed();
        },
        &tab
    );

    const auto pButton_pickCue = new Fl_Button(rx - 110, ty + 30, 80, 30, "Browse");
    pButton_pickCue->tooltip(pLabel_cue->tooltip());
    pButton_pickCue->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;

            const auto pFileChooser = std::make_unique<Fl_Native_File_Chooser>();
            pFileChooser->filter("CUE Sheet Files\t*.{cue}");
            pFileChooser->type(Fl_Native_File_Chooser::BROWSE_FILE);
            pFileChooser->title("Choose a game disc .cue file");

            if ((pFileChooser->show() == 0) && (pFileChooser->count() == 1)) {
                tab.pInput_cue->value(pFileChooser->filename());
                tab.pInput_cue->set_changed();
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
    const auto pLabel_dataDir = new Fl_Box(FL_NO_BOX, lx, ty, 150, 30, "Mod directory");
    pLabel_dataDir->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel_dataDir->tooltip(
        "Path to a directory containing data for a PsyDoom mod.\n"
        "Use this field to pick which PsyDoom mod is active.\n"
    );

    const auto pButton_clearDataDir = new Fl_Button(rx - 30, ty + 30, 30, 30, "X");
    pButton_clearDataDir->tooltip(pLabel_dataDir->tooltip());
    pButton_clearDataDir->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;
            tab.pInput_dataDir->value("");
            tab.pInput_dataDir->set_changed();
        },
        &tab
    );

    const auto pButton_pickDataDir = new Fl_Button(rx - 110, ty + 30, 80, 30, "Browse");
    pButton_pickDataDir->tooltip(pLabel_dataDir->tooltip());
    pButton_pickDataDir->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;

            const auto pFileChooser = std::make_unique<Fl_Native_File_Chooser>();
            pFileChooser->type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
            pFileChooser->title("Choose a directory containing files for a PsyDoom mod");

            if ((pFileChooser->show() == 0) && (pFileChooser->count() == 1)) {
                tab.pInput_dataDir->value(pFileChooser->filename());
                tab.pInput_dataDir->set_changed();
            }
        },
        &tab
    );

    const int dataDirInputRx = getRectExtents(*pButton_pickDataDir).lx - 10;
    tab.pInput_dataDir = makeFl_Input(lx, ty + 30, dataDirInputRx - lx, 30);
    tab.pInput_dataDir->tooltip(pLabel_dataDir->tooltip());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'Game options' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeGameOptionsSection(Tab_Launcher& tab, const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 170, 30, "Game options");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 170, 180, "");

    // All the check boxes
    tab.pCheck_recordDemos = makeFl_Check_Button(x + 10, y + 40, 150, 30, "  Record demos");
    tab.pCheck_recordDemos->tooltip(
        "If enabled causes demos to be recorded for each map played.\n"
        "Demos will be named `DEMO_MAP??.LMP` after the current map number.\n"
        "All demos are output to the PsyDoom user settings and data directory."
    );

    tab.pCheck_pistolStart = makeFl_Check_Button(x + 10, y + 70, 150, 30, "  Force pistol starts");
    tab.pCheck_pistolStart->tooltip(
        "Forces pistol starts on all levels when enabled.\n"
        "This setting also affects password generation and multiplayer.\n"
    );

    tab.pCheck_turbo = makeFl_Check_Button(x + 10, y + 100, 150, 30, "  Turbo mode");
    tab.pCheck_turbo->tooltip(
        "When enabled allows the player to move and fire 2x as fast.\n"
        "Doors and platforms also move 2x as fast. Monsters are unaffected."
    );

    tab.pCheck_noMonsters = makeFl_Check_Button(x + 10, y + 130, 150, 30, "  No monsters");
    tab.pCheck_noMonsters->tooltip(
        "Removes monsters from all levels.\n"
        "Note: this might make some maps impossible to complete without cheats!"
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called when the net peer type setting is updated.
// Updates whether the host name field is active.
//------------------------------------------------------------------------------------------------------------------------------------------
static void onNetPeerTypeUpdated(Tab_Launcher& tab) noexcept {
    // Enable the host input box when the peer type is 'client'
    if (tab.pChoice_netPeerType->value() == 0) {
        tab.pLabel_netHost->activate();
        tab.pInput_netHost->activate();
        tab.pButton_clearNetHost->activate();
    } else {
        tab.pLabel_netHost->deactivate();
        tab.pInput_netHost->deactivate();
        tab.pButton_clearNetHost->deactivate();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets a flag to say that the game should be launched.
// Then begins the process of launching the game by closing the launcher window.
//------------------------------------------------------------------------------------------------------------------------------------------
static void requestGameBeLaunched(Tab_Launcher& tab) noexcept {
    tab.bLaunchGame = true;
    tab.pTab->window()->hide();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes network related options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeNetworkOptions(Tab_Launcher& tab, const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 400, 30, "Network settings");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 400, 120, "");

    // Host selection
    tab.pLabel_netHost = new Fl_Box(FL_NO_BOX, x + 10, y + 50, 100, 30, "Host");
    tab.pLabel_netHost->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    tab.pLabel_netHost->tooltip(
        "Client: the hostname or IP address of the machine to connect to.\n"
        "If unspecified then the client will try to connect to the local machine (localhost)."
    );

    tab.pInput_netHost = makeFl_Input(x + 60, y + 50, 290, 30);
    tab.pInput_netHost->tooltip(tab.pLabel_netHost->tooltip());

    tab.pButton_clearNetHost = new Fl_Button(x + 350, y + 50, 30, 30, "X");
    tab.pButton_clearNetHost->tooltip(tab.pLabel_netHost->tooltip());
    tab.pButton_clearNetHost->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;
            tab.pInput_netHost->value("");
            tab.pInput_netHost->set_changed();
        },
        &tab
    );

    // Port selection
    const auto pLabel_netPort = new Fl_Box(FL_NO_BOX, x + 10, y + 100, 90, 30, "Port");
    pLabel_netPort->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel_netPort->tooltip(
        "Client: specifies the port to connect to the server on.\n"
        "Server: specifies the port to listen on for incoming connections.\n"
        "Note: if no port is specified then the default value of '666' is used."
    );

    tab.pInput_netPort = makeFl_Int_Input(x + 60, y + 100, 70, 30);
    tab.pInput_netPort->tooltip(pLabel_netPort->tooltip());

    const auto pButton_clearNetPort = new Fl_Button(x + 130, y + 100, 30, 30, "X");
    pButton_clearNetPort->tooltip(pLabel_netPort->tooltip());
    pButton_clearNetPort->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;
            tab.pInput_netPort->value("");
            tab.pInput_netPort->set_changed();
        },
        &tab
    );

    // Peer type selection
    const auto pLabel_netPeerType = new Fl_Box(FL_NO_BOX, x + 200, y + 100, 80, 30, "Peer type");
    pLabel_netPeerType->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel_netPeerType->tooltip(
        "Whether this machine is a client or server in a multiplayer game.\n"
        "The server is the machine that listens and waits for incoming connections.\n"
        "The server also determines the game rules and settings."
    );

    tab.pChoice_netPeerType = new Fl_Choice(x + 280, y + 100, 100, 30);
    tab.pChoice_netPeerType->add("Client");
    tab.pChoice_netPeerType->add("Server");
    tab.pChoice_netPeerType->value(0);
    tab.pChoice_netPeerType->tooltip(pLabel_netPeerType->tooltip());
    tab.pChoice_netPeerType->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;
            onNetPeerTypeUpdated(tab);

            // Note: because we use a non-default callback FLTK clears the 'changed' flag.
            // Set it here instead as a minor workaround, so the launcher preferences are saved.
            tab.pTab->set_changed();
        },
        &tab
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'tools' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeToolsSection(Tab_Launcher& tab, const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 290, 30, "Tools");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 290, 180, "");

    // Button to open the PsyDoom data directory
    const auto pButton_openDataDir = new Fl_Button(x + 20, y + 50, 250, 30, "Open PsyDoom data directory");
    pButton_openDataDir->callback(
        [](Fl_Widget*, void*) noexcept {
            const std::string userDataFolder = Utils::getOrCreateUserDataFolder();

            #if WIN32
                // Windows
                const std::string shellCmd = std::string("explorer \"") + userDataFolder + "\"";
            #elif __APPLE__
                // MacOS
                const std::string shellCmd = std::string("open \"") + userDataFolder + "\"";
            #elif __linux__
                // Linux
                const std::string shellCmd = std::string("xdg-open \"") + userDataFolder + "\"";
            #else
                #error Need to implement opening the data directory for this platform!
            #endif

            std::system(shellCmd.c_str());
        },
        &tab
    );
    pButton_openDataDir->tooltip("Opens the directory that PsyDoom stores preferences, savefiles and recorded demos to.");

    // A button to play a demo file
    const auto pButton_playDemo = new Fl_Button(x + 20, y + 100, 250, 30, "Play a demo file");
    pButton_playDemo->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;

            const std::string userDataFolder = Utils::getOrCreateUserDataFolder();

            const auto pFileChooser = std::make_unique<Fl_Native_File_Chooser>();
            pFileChooser->type(Fl_Native_File_Chooser::BROWSE_FILE);
            pFileChooser->title("Choose a demo file to play");
            pFileChooser->directory(userDataFolder.c_str());

            if ((pFileChooser->show() == 0) && (pFileChooser->count() == 1)) {
                tab.demoFileToPlay = pFileChooser->filename();
                requestGameBeLaunched(tab);
            }
        },
        &tab
    );
    pButton_openDataDir->tooltip("Play a demo file previously recorded by PsyDoom and then exit.");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the launch button
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLaunchButton(Tab_Launcher& tab, const int x, const int y) noexcept {
    tab.pButton_launch = new Fl_Button(x, y, 400, 40, "Launch PsyDoom!");
    tab.pButton_launch->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Launcher& tab = *(Tab_Launcher*) pUserData;
            requestGameBeLaunched(tab);
        },
        &tab
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Launcher' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Launcher& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);
    tab.onNetPeerTypeUpdated = onNetPeerTypeUpdated;

    makeLogo(tab, tabRect.lx, tabRect.rx, tabRect.ty + 120);
    makeCueFileSelector(tab, tabRect.lx + 20, tabRect.rx - 20, 230);
    makeModDataDirSelector(tab, tabRect.lx + 20, tabRect.rx - 20, 290);
    makeGameOptionsSection(tab, tabRect.lx + 330, 360);
    makeNetworkOptions(tab, tabRect.lx + 520, 360);
    makeToolsSection(tab, tabRect.lx + 20, 360);
    makeLaunchButton(tab, tabRect.rx - 420, tabRect.by - 60);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
