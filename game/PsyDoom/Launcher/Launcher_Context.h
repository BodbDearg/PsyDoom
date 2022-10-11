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
class Fl_Pixmap;
class Fl_Tabs;
class Fl_Window;

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents the result of executing the launcher.
// Tells what should happen next.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class LauncherResult : int32_t {
    Exit,           // Exit the application entirely
    RunLauncher,    // Re-run the launcher (used after config is reset back to defaults)
    RunGame         // Run the game after running the launcher
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets and state for the 'Launcher' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Launcher {
    Fl_Group*                       pTab;
    Fl_Input*                       pInput_cue;
    Fl_Input*                       pInput_dataDir;
    Fl_Check_Button*                pCheck_recordDemos;
    Fl_Check_Button*                pCheck_pistolStart;
    Fl_Check_Button*                pCheck_turbo;
    Fl_Check_Button*                pCheck_noMonsters;
    Fl_Box*                         pLabel_netHost;
    Fl_Input*                       pInput_netHost;
    Fl_Button*                      pButton_clearNetHost;
    Fl_Int_Input*                   pInput_netPort;
    Fl_Choice*                      pChoice_netPeerType;
    Fl_Choice*                      pChoice_resetCfgType;
    Fl_Button*                      pButton_launch;
    std::unique_ptr<Fl_Pixmap>      pGameLogo;                  // Image data for the game's logo

    // A callback invoked when the net peer type is updated
    void (*onNetPeerTypeUpdated)(Tab_Launcher& tab) noexcept;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets and state for the 'Graphics' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Graphics {
    Fl_Group*               pTab;
    Fl_Box*                 pLabel_settingsPreset;
    Fl_Choice*              pChoice_settingsPreset;
    Fl_Button*              pButton_settingsPreset;
    Fl_Box*                 pLabel_renderHeight;
    Fl_Int_Input*           pInput_renderHeight;
    Fl_Box*                 pLabel_aaMultisamples;
    Fl_Int_Input*           pInput_aaMultisamples;
    Fl_Check_Button*        pCheck_pixelStretch;
    Fl_Check_Button*        pCheck_tripleBuffer;
    Fl_Check_Button*        pCheck_widescreenEnabled;
    Fl_Check_Button*        pCheck_32bitShading;
    Fl_Check_Button*        pCheck_drawExtStatusBar;
    Fl_Check_Button*        pCheck_brightenAutomap;
    Fl_Check_Button*        pCheck_disableVkRenderer;
    Fl_Box*                 pLabel_prefDevicesRegex;
    Fl_Input*               pInput_prefDevicesRegex;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets and state for the 'Game' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Game {
    Fl_Group*           pTab;
    Fl_Input*           pInput_cueFile;
    Fl_Check_Button*    pCheck_uncappedFramerate;
    Fl_Check_Button*    pCheck_interpolateSectors;
    Fl_Check_Button*    pCheck_interpolateMobj;
    Fl_Check_Button*    pCheck_interpolateMonsters;
    Fl_Check_Button*    pCheck_interpolateWeapon;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets and state for the 'Input' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Input {
    Fl_Group*       pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets and state for the 'Controls' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Controls {
    Fl_Group*       pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets and state for the 'Audio' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Audio {
    Fl_Group*       pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets and state for the 'Cheats' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Cheats {
    Fl_Group*       pTab;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds widgets and state for the 'Multiplayer' tab
//------------------------------------------------------------------------------------------------------------------------------------------
struct Tab_Multiplayer {
    Fl_Group*       pTab;
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
    Tab_Multiplayer                 tab_multiplayer;
    LauncherResult                  launcherResult;
    std::string                     demoFileToPlay;     // If non empty: path to a demo file to play
};

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
