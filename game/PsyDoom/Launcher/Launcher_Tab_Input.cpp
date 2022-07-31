//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Input' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Config/ConfigSerialization_Input.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Group.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'Mouse' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeMouseSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 400, 30, "Mouse");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 400, 70, "");

    // Mouse turn speed
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 50, 200, 26, "Mouse turn speed");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Input.mouseTurnSpeed.comment);

        const auto pInput = new Fl_Float_Input(x + 260, y + 50, 120, 26);
        bindConfigField<Config::gMouseTurnSpeed, Config::gbNeedSave_Input>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'Gamepad' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeGamepadSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 400, 30, "Gamepad");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 400, 220, "");

    // Dead zone
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 50, 200, 26, "Dead zone");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Input.gamepadDeadZone.comment);

        const auto pInput = new Fl_Float_Input(x + 260, y + 50, 120, 26);
        bindConfigField<Config::gGamepadDeadZone, Config::gbNeedSave_Input>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Fast turn speed: high
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 80, 200, 26, "Fast turn speed: high");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Input.gamepadFastTurnSpeed_High.comment);

        const auto pInput = new Fl_Float_Input(x + 260, y + 80, 120, 26);
        bindConfigField<Config::gGamepadFastTurnSpeed_High, Config::gbNeedSave_Input>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Fast turn speed: low
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 110, 200, 26, "Fast turn speed: low");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Input.gamepadFastTurnSpeed_High.comment);

        const auto pInput = new Fl_Float_Input(x + 260, y + 110, 120, 26);
        bindConfigField<Config::gGamepadFastTurnSpeed_Low, Config::gbNeedSave_Input>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Turn speed: high
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 140, 200, 26, "Turn speed: high");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Input.gamepadFastTurnSpeed_High.comment);

        const auto pInput = new Fl_Float_Input(x + 260, y + 140, 120, 26);
        bindConfigField<Config::gGamepadTurnSpeed_High, Config::gbNeedSave_Input>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Turn speed: low
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 170, 200, 26, "Turn speed: low");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Input.gamepadFastTurnSpeed_High.comment);

        const auto pInput = new Fl_Float_Input(x + 260, y + 170, 120, 26);
        bindConfigField<Config::gGamepadTurnSpeed_Low, Config::gbNeedSave_Input>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Analog to digital threshold
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 200, 200, 26, "Analog to digital threshold");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Input.analogToDigitalThreshold.comment);

        const auto pInput = new Fl_Float_Input(x + 260, y + 200, 120, 26);
        bindConfigField<Config::gAnalogToDigitalThreshold, Config::gbNeedSave_Input>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Input' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Input& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);

    makeMouseSection((tabRect.lx + tabRect.rx) / 2 - 200, tabRect.ty + 20);
    makeGamepadSection((tabRect.lx + tabRect.rx) / 2 - 200, tabRect.ty + 140);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
