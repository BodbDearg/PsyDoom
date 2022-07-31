//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Cheats' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Config/ConfigSerialization_Cheats.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Group.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: binds a cheat key sequence config value to the specified input
//------------------------------------------------------------------------------------------------------------------------------------------
template <Config::CheatKeySequence& configValue>
void bindConfigField(Fl_Input& input) noexcept {
    input.callback(
        [](Fl_Widget* const pWidget, void*) noexcept {
            Fl_Input* const pInput = static_cast<Fl_Input*>(pWidget);
            Config::setCheatKeySequence(configValue, pInput->value());
            Config::gbNeedSave_Cheats = true;
        }
    );

    std::string keySequenceStr;
    Config::getCheatKeySequence(configValue, keySequenceStr);
    input.value(keySequenceStr.c_str());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'General' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeGeneralSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 500, 30, "General");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 500, 120, "");

    // Various toggles
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 120, 30, "  Enable developer cheat shortcuts");
        bindConfigField<Config::gbEnableDevCheatShortcuts, Config::gbNeedSave_Cheats>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Cheats.enableDevCheatShortcuts.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 120, 30, "  Enable developer in-place map reload function key");
        bindConfigField<Config::gbEnableDevInPlaceReloadFunctionKey, Config::gbNeedSave_Cheats>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Cheats.enableDevInPlaceReloadFunctionKey.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 100, 120, 30, "  Enable developer auto in-place map reload");
        bindConfigField<Config::gbEnableDevMapAutoReload, Config::gbNeedSave_Cheats>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Cheats.enableDevMapAutoReload.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'Cheat key sequences' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeCheatKeySequencesSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 500, 30, "Cheat key sequences");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 500, 310, "");

    // God mode
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 50, 80, 26, "God mode");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 50, 200, 26);
        bindConfigField<Config::gCheatKeys_GodMode>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // No clip
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 80, 80, 26, "No clip");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 80, 200, 26);
        bindConfigField<Config::gCheatKeys_NoClip>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Level warp
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 110, 80, 26, "Level warp");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 110, 200, 26);
        bindConfigField<Config::gCheatKeys_LevelWarp>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Weapons, keys and armor
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 140, 80, 26, "Weapons, keys and armor");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 140, 200, 26);
        bindConfigField<Config::gCheatKeys_WeaponsKeysAndArmor>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // All map lines on
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 170, 80, 26, "All map lines on");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 170, 200, 26);
        bindConfigField<Config::gCheatKeys_AllMapLinesOn>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // All map things on
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 200, 80, 26, "All map things on");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 200, 200, 26);
        bindConfigField<Config::gCheatKeys_AllMapThingsOn>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // X-ray vision
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 230, 80, 26, "X-ray vision");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 230, 200, 26);
        bindConfigField<Config::gCheatKeys_XRayVision>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // VRAM viewer
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 260, 80, 26, "VRAM viewer");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 260, 200, 26);
        bindConfigField<Config::gCheatKeys_VramViewer>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // No-target
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 290, 80, 26, "No-target");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Cheats.cheatKeys_godMode.comment);

        const auto pInput = new Fl_Input(x + 270, y + 290, 200, 26);
        bindConfigField<Config::gCheatKeys_NoTarget>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Cheats' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Cheats& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);

    makeGeneralSection((tabRect.lx + tabRect.rx) / 2 - 250, tabRect.ty + 20);
    makeCheatKeySequencesSection((tabRect.lx + tabRect.rx) / 2 - 250, tabRect.ty + 190);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
