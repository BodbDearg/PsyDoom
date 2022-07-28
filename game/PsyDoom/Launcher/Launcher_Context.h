#pragma once

#if PSYDOOM_LAUNCHER

#include "Macros.h"

#include <memory>
#include <string>

class Fl_Box;
class Fl_Button;
class Fl_Check_Button;
class Fl_Choice;
class Fl_Group;
class Fl_Input;
class Fl_Int_Input;
class Fl_Tabs;
class Fl_Window;

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds all of the widgets and state for the 'Launcher' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Launcher {
    Fl_Group*           pTab;
    Fl_Input*           pInput_cue;
    Fl_Input*           pInput_dataDir;
    Fl_Check_Button*    pCheck_recordDemos;
    Fl_Check_Button*    pCheck_pistolStart;
    Fl_Check_Button*    pCheck_turbo;
    Fl_Check_Button*    pCheck_noMonsters;
    Fl_Box*             pLabel_netHost;
    Fl_Input*           pInput_netHost;
    Fl_Button*          pButton_clearNetHost;
    Fl_Int_Input*       pInput_netPort;
    Fl_Choice*          pChoice_netPeerType;
    Fl_Button*          pButton_launch;
    bool                bLaunchGame;                // If true actually run the game once the launcher exits
    std::string         demoFileToPlay;             // If non empty: path to a demo file to play
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds all of the widgets and state for the 'Graphics' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Graphics {
    Fl_Group*           pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds all of the widgets and state for the 'Game' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Game {
    Fl_Group*           pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds all of the widgets and state for the 'Input' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Input {
    Fl_Group*           pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds all of the widgets and state for the 'Controls' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Controls {
    Fl_Group*           pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds all of the widgets and state for the 'Audio' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Audio {
    Fl_Group*           pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds all of the widgets and state for the 'Cheats' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Cheats {
    Fl_Group*           pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets of interest and state for the launcher
//------------------------------------------------------------------------------------------------------------------------------------------
struct Context {
    std::unique_ptr<Fl_Window>      pWindow;
    Fl_Tabs*                        pTabs;
    Tab_Launcher                    tab_launcher;
    Tab_Graphics                    tab_graphics;
    Tab_Game                        tab_game;
    Tab_Input                       tab_input;
    Tab_Controls                    tab_controls;
    Tab_Audio                       tab_audio;
    Tab_Cheats                      tab_cheats;
};

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
