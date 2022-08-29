//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Controls' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "IniUtils.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Config/ConfigSerialization_Controls.h"
#include "AddInputPrompt.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Button.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Scroll.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Prompts the user to add an input for the specified control related config field.
// If the user inputs something then the config field is updated along with the specified 'Fl_Input' displaying it.
//------------------------------------------------------------------------------------------------------------------------------------------
static void doAddInputPrompt(Fl_Input& input, ConfigSerialization::ConfigField& configField) noexcept {
    // Get the input from the user via the 'Add Input' prompt
    const Controls::InputSrc inputSrc = AddInputPrompt::show();

    // If no input was received then stop now:
    if (inputSrc.device == Controls::InputSrc::NULL_DEVICE)
        return;

    // Append the new input to the control binding string and save the result to the config field
    IniUtils::IniValue configValue;
    configField.getFunc(configValue);

    if (!configValue.strValue.empty()) {
        configValue.strValue += ',';
        configValue.strValue += ' ';
    }

    Controls::appendInputSrcToStr(inputSrc, configValue.strValue);
    configField.setFunc(configValue);

    // Assign the updated list of inputs back to the UI text box for the control binding
    input.value(configValue.strValue.c_str());

    // We need to save controls config since it's been modified!
    Config::gbNeedSave_Controls = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: makes a control binding field
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeBindingField(
    const char* const name,
    ConfigSerialization::ConfigField& configField,
    const char* const tooltip,
    const int lx,
    const int rx,
    const int y
) noexcept {
    // Get the current value of the control binding
    IniUtils::IniValue configValue = {};
    configField.getFunc(configValue);

    // Make the label
    const auto pLabel = new Fl_Box(FL_NO_BOX, lx, y, 170, 26, name);
    pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel->tooltip(tooltip);

    // Make the button for adding an input to this control binding (assign the callback later)
    const auto pButton = new Fl_Button(rx - 40, y, 40, 26, "+");
    pButton->tooltip(tooltip);

    // How big should the text input field be for this control binding?
    const int inputLx = pLabel->x() + pLabel->w() + 20;
    const int inputRx = pButton->x() - 10;
    const int inputW = inputRx - inputLx;

    // Make the text input field for the control binding and assign logic for when the user edits the text
    const auto pInput = makeFl_Input(inputLx, y, inputW, 26);
    pInput->tooltip(tooltip);
    pInput->value(configValue.strValue.c_str());
    pInput->callback(
        [](Fl_Widget* const pWidget, void* const pUserData) noexcept {
            // Get the input changed and the associated config field.
            ASSERT(pWidget);
            ASSERT(pUserData);
            Fl_Input& input = static_cast<Fl_Input&>(*pWidget);
            ConfigSerialization::ConfigField& configField = *static_cast<ConfigSerialization::ConfigField*>(pUserData);

            // Update the config field
            IniUtils::IniValue configValue = {};
            configValue.strValue = (input.value()) ? input.value() : "";
            configField.setFunc(configValue);

            // Convert the list of inputs for the control binding back to a string again.
            // This converstion ensures the text field has a valid format string inside it.
            // Invalid stuff will be removed through via conversion process:
            configField.getFunc(configValue);
            input.value(configValue.strValue.c_str());

            // We need to save controls config since it's been modified!
            Config::gbNeedSave_Controls = true;
        },
        &configField
    );

    // Add the action for the button which allows binding an input
    pButton->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Fl_Input& input = *static_cast<Fl_Input*>(pUserData);

            ASSERT(input.user_data());
            ConfigSerialization::ConfigField& configField = *static_cast<ConfigSerialization::ConfigField*>(input.user_data());
            doAddInputPrompt(input, configField);
        },
        pInput
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: makes a section title field and box
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeSectionTitleAndBox(const char* const text, const int lx, const int rx, const int ty, const int by) noexcept {
    const auto pLabel = new Fl_Box(FL_NO_BOX, lx, ty, 300, 26, text);
    pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    new Fl_Box(FL_THIN_DOWN_BOX, lx, ty + 30, rx - lx, by - ty - 30, "");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the 'Analog movement and turning' controls section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeAnalogMoveAndTurnSection(const int secLx, const int secRx, const int secY) noexcept {
    makeSectionTitleAndBox("Analog movement and turning", secLx, secRx, secY, secY + 240);

    auto& cfg = ConfigSerialization::gConfig_Controls;
    const char* const tooltip = cfg.analog_moveForward.comment;
    const int fieldY = secY + 50;
    const int fieldLx = secLx + 20;
    const int fieldRx = secRx - 20;

    makeBindingField("Move forward", cfg.analog_moveForward, tooltip, fieldLx, fieldRx, fieldY);
    makeBindingField("Move backward", cfg.analog_moveBackward, tooltip, fieldLx, fieldRx, fieldY + 30);
    makeBindingField("Strafe left", cfg.analog_strafeLeft, tooltip, fieldLx, fieldRx, fieldY + 60);
    makeBindingField("Strafe right", cfg.analog_strafeRight, tooltip, fieldLx, fieldRx, fieldY + 90);
    makeBindingField("Turn left", cfg.analog_turnLeft, tooltip, fieldLx, fieldRx, fieldY + 120);
    makeBindingField("Turn right", cfg.analog_turnRight, tooltip, fieldLx, fieldRx, fieldY + 150);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the 'Digital movement and turning' controls section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeDigitalMoveAndTurnSection(const int secLx, const int secRx, const int secY) noexcept {
    makeSectionTitleAndBox("Digital movement and turning", secLx, secRx, secY, secY + 240);

    auto& cfg = ConfigSerialization::gConfig_Controls;
    const char* const tooltip = cfg.digital_moveForward.comment;
    const int fieldY = secY + 50;
    const int fieldLx = secLx + 20;
    const int fieldRx = secRx - 20;

    makeBindingField("Move forward", cfg.digital_moveForward, tooltip, fieldLx, fieldRx, fieldY);
    makeBindingField("Move backward", cfg.digital_moveBackward, tooltip, fieldLx, fieldRx, fieldY + 30);
    makeBindingField("Strafe left", cfg.digital_strafeLeft, tooltip, fieldLx, fieldRx, fieldY + 60);
    makeBindingField("Strafe right", cfg.digital_strafeRight, tooltip, fieldLx, fieldRx, fieldY + 90);
    makeBindingField("Turn left", cfg.digital_turnLeft, tooltip, fieldLx, fieldRx, fieldY + 120);
    makeBindingField("Turn right", cfg.digital_turnRight, tooltip, fieldLx, fieldRx, fieldY + 150);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the 'In-game actions & modifiers' controls section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeInGameActionsAndModifiersSection(const int secLx, const int secRx, const int secY) noexcept {
    makeSectionTitleAndBox("In-game actions & modifiers", secLx, secRx, secY, secY + 300);

    auto& cfg = ConfigSerialization::gConfig_Controls;
    const char* const tooltip = nullptr;
    const int fieldY = secY + 50;
    const int fieldLx = secLx + 20;
    const int fieldRx = secRx - 20;

    makeBindingField("Use", cfg.action_use, tooltip, fieldLx, fieldRx, fieldY);
    makeBindingField("Attack", cfg.action_attack, tooltip, fieldLx, fieldRx, fieldY + 30);
    makeBindingField("Respawn", cfg.action_respawn, tooltip, fieldLx, fieldRx, fieldY + 60);
    makeBindingField("Run modifier", cfg.modifier_run, tooltip, fieldLx, fieldRx, fieldY + 90);
    makeBindingField("Strafe modifier", cfg.modifier_strafe, tooltip, fieldLx, fieldRx, fieldY + 120);
    makeBindingField("Toggle autorun", cfg.toggle_autorun, tooltip, fieldLx, fieldRx, fieldY + 150);
    makeBindingField("Quick save", cfg.quicksave, tooltip, fieldLx, fieldRx, fieldY + 180);
    makeBindingField("Quick load", cfg.quickload, tooltip, fieldLx, fieldRx, fieldY + 210);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the 'Miscellaneous toggles' controls section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeMiscellaneousTogglesSection(const int secLx, const int secRx, const int secY) noexcept {
    makeSectionTitleAndBox("Miscellaneous toggles", secLx, secRx, secY, secY + 180);

    auto& cfg = ConfigSerialization::gConfig_Controls;
    const char* const tooltip = cfg.toggle_pause.comment;
    const int fieldY = secY + 50;
    const int fieldLx = secLx + 20;
    const int fieldRx = secRx - 20;

    makeBindingField("Toggle pause", cfg.toggle_pause, tooltip, fieldLx, fieldRx, fieldY);
    makeBindingField("Toggle map", cfg.toggle_map, tooltip, fieldLx, fieldRx, fieldY + 30);
    makeBindingField("Toggle renderer", cfg.toggle_renderer, tooltip, fieldLx, fieldRx, fieldY + 60);
    makeBindingField("Toggle demo player", cfg.toggle_viewPlayer, tooltip, fieldLx, fieldRx, fieldY + 90);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the 'Weapon switching' controls section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeWeaponSwitchingSection(const int secLx, const int secRx, const int secY) noexcept {
    makeSectionTitleAndBox("Weapon switching", secLx, secRx, secY, secY + 420);

    auto& cfg = ConfigSerialization::gConfig_Controls;
    const char* const tooltip = cfg.weapon_scrollUp.comment;
    const int fieldY = secY + 50;
    const int fieldLx = secLx + 20;
    const int fieldRx = secRx - 20;

    makeBindingField("Weapon scroll up", cfg.weapon_scrollUp, tooltip, fieldLx, fieldRx, fieldY);
    makeBindingField("Weapon scroll down", cfg.weapon_scrollDown, tooltip, fieldLx, fieldRx, fieldY + 30);
    makeBindingField("Previous weapon", cfg.weapon_previous, tooltip, fieldLx, fieldRx, fieldY + 60);
    makeBindingField("Next weapon", cfg.weapon_next, tooltip, fieldLx, fieldRx, fieldY + 90);
    makeBindingField("Equip Fist/Chainsaw", cfg.weapon_fistChainsaw, tooltip, fieldLx, fieldRx, fieldY + 120);
    makeBindingField("Equip Pistol", cfg.weapon_pistol, tooltip, fieldLx, fieldRx, fieldY + 150);
    makeBindingField("Equip Shotgun", cfg.weapon_shotgun, tooltip, fieldLx, fieldRx, fieldY + 180);
    makeBindingField("Equip Super Shotgun", cfg.weapon_superShotgun, tooltip, fieldLx, fieldRx, fieldY + 210);
    makeBindingField("Equip Chaingun", cfg.weapon_chaingun, tooltip, fieldLx, fieldRx, fieldY + 240);
    makeBindingField("Equip Rocket Launcher", cfg.weapon_rocketLauncher, tooltip, fieldLx, fieldRx, fieldY + 270);
    makeBindingField("Equip Plasma Rifle", cfg.weapon_plasmaRifle, tooltip, fieldLx, fieldRx, fieldY + 300);
    makeBindingField("Equip BFG", cfg.weapon_bfg, tooltip, fieldLx, fieldRx, fieldY + 330);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the 'Menu & UI controls' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeMenuAndUIControlsSection(const int secLx, const int secRx, const int secY) noexcept {
    makeSectionTitleAndBox("Menu & UI controls", secLx, secRx, secY, secY + 330);

    auto& cfg = ConfigSerialization::gConfig_Controls;
    const char* const tooltip = nullptr;
    const int fieldY = secY + 50;
    const int fieldLx = secLx + 20;
    const int fieldRx = secRx - 20;

    makeBindingField("Up", cfg.menu_up, tooltip, fieldLx, fieldRx, fieldY);
    makeBindingField("Down", cfg.menu_down, tooltip, fieldLx, fieldRx, fieldY + 30);
    makeBindingField("Left", cfg.menu_left, tooltip, fieldLx, fieldRx, fieldY + 60);
    makeBindingField("Right", cfg.menu_right, tooltip, fieldLx, fieldRx, fieldY + 90);
    makeBindingField("Ok", cfg.menu_ok, tooltip, fieldLx, fieldRx, fieldY + 120);
    makeBindingField("Back", cfg.menu_back, tooltip, fieldLx, fieldRx, fieldY + 150);
    makeBindingField("Start", cfg.menu_start, tooltip, fieldLx, fieldRx, fieldY + 180);
    makeBindingField("Enter password char", cfg.menu_enterPasswordChar, tooltip, fieldLx, fieldRx, fieldY + 210);
    makeBindingField("Delete password char", cfg.menu_deletePasswordChar, tooltip, fieldLx, fieldRx, fieldY + 240);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the 'Automap controls' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeAutomapControlsSection(const int secLx, const int secRx, const int secY) noexcept {
    makeSectionTitleAndBox("Automap controls", secLx, secRx, secY, secY + 270);

    auto& cfg = ConfigSerialization::gConfig_Controls;
    const char* const tooltip = nullptr;
    const int fieldY = secY + 50;
    const int fieldLx = secLx + 20;
    const int fieldRx = secRx - 20;

    makeBindingField("Zoom in", cfg.automap_zoomIn, tooltip, fieldLx, fieldRx, fieldY);
    makeBindingField("Zoom out", cfg.automap_zoomOut, tooltip, fieldLx, fieldRx, fieldY + 30);
    makeBindingField("Move up", cfg.automap_moveUp, tooltip, fieldLx, fieldRx, fieldY + 60);
    makeBindingField("Move down", cfg.automap_moveDown, tooltip, fieldLx, fieldRx, fieldY + 90);
    makeBindingField("Move left", cfg.automap_moveLeft, tooltip, fieldLx, fieldRx, fieldY + 120);
    makeBindingField("Move right", cfg.automap_moveRight, tooltip, fieldLx, fieldRx, fieldY + 150);
    makeBindingField("Pan", cfg.automap_pan, tooltip, fieldLx, fieldRx, fieldY + 180);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the 'PSX cheat code buttons' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makePSXCheatCodeButtonsSection(const int secLx, const int secRx, const int secY) noexcept {
    makeSectionTitleAndBox("PSX cheat code buttons", secLx, secRx, secY, secY + 420);

    auto& cfg = ConfigSerialization::gConfig_Controls;
    const char* const tooltip = cfg.psxCheatCode_up.comment;
    const int fieldY = secY + 50;
    const int fieldLx = secLx + 20;
    const int fieldRx = secRx - 20;

    makeBindingField("Up", cfg.psxCheatCode_up, tooltip, fieldLx, fieldRx, fieldY);
    makeBindingField("Down", cfg.psxCheatCode_down, tooltip, fieldLx, fieldRx, fieldY + 30);
    makeBindingField("Left", cfg.psxCheatCode_left, tooltip, fieldLx, fieldRx, fieldY + 60);
    makeBindingField("Right", cfg.psxCheatCode_right, tooltip, fieldLx, fieldRx, fieldY + 90);
    makeBindingField("Triangle", cfg.psxCheatCode_triangle, tooltip, fieldLx, fieldRx, fieldY + 120);
    makeBindingField("Circle", cfg.psxCheatCode_circle, tooltip, fieldLx, fieldRx, fieldY + 150);
    makeBindingField("Cross", cfg.psxCheatCode_cross, tooltip, fieldLx, fieldRx, fieldY + 180);
    makeBindingField("Square", cfg.psxCheatCode_square, tooltip, fieldLx, fieldRx, fieldY + 210);
    makeBindingField("L1", cfg.psxCheatCode_l1, tooltip, fieldLx, fieldRx, fieldY + 240);
    makeBindingField("R1", cfg.psxCheatCode_r1, tooltip, fieldLx, fieldRx, fieldY + 270);
    makeBindingField("L2", cfg.psxCheatCode_l2, tooltip, fieldLx, fieldRx, fieldY + 300);
    makeBindingField("R2", cfg.psxCheatCode_r2, tooltip, fieldLx, fieldRx, fieldY + 330);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Controls' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populateControlsTab(Context& ctx) noexcept {
    Tab_Controls& tab = ctx.tab_controls;
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    // Make the scroll view
    const RectExtents tabRect = getRectExtents(*tab.pTab);
    const auto pScroll = new Fl_Scroll(tabRect.lx + 1, tabRect.ty + 5, tabRect.rx - tabRect.lx - 1, tabRect.by - tabRect.ty - 6, "");
    pScroll->scrollbar.linesize(30 * 4);
    tab.pTab->resizable(pScroll);

    // Make all the control sections
    makeAnalogMoveAndTurnSection(tabRect.lx + 20, tabRect.rx - 30, tabRect.ty + 20);
    makeDigitalMoveAndTurnSection(tabRect.lx + 20, tabRect.rx - 30, tabRect.ty + 280);
    makeInGameActionsAndModifiersSection(tabRect.lx + 20, tabRect.rx - 30, tabRect.ty + 540);
    makeMiscellaneousTogglesSection(tabRect.lx + 20, tabRect.rx - 30, tabRect.ty + 860);
    makeWeaponSwitchingSection(tabRect.lx + 20, tabRect.rx - 30, tabRect.ty + 1060);
    makeMenuAndUIControlsSection(tabRect.lx + 20, tabRect.rx - 30, tabRect.ty + 1500);
    makeAutomapControlsSection(tabRect.lx + 20, tabRect.rx - 30, tabRect.ty + 1850);
    makePSXCheatCodeButtonsSection(tabRect.lx + 20, tabRect.rx - 30, tabRect.ty + 2140);
    
    // Add a small bit of padding at the end and finish up making the scroll view
    new Fl_Box(tabRect.lx + 20, tabRect.ty + 2560, 100, 20);
    pScroll->end();
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
