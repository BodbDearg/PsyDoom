//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Audio' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Config/ConfigSerialization_Audio.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Group.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the 'Settings' section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeSettingSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 300, 30, "Audio settings");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 300, 100, "");

    // Audio buffer size
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 50, 140, 26, "Audio buffer size");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Audio.audioBufferSize.comment);

        const auto pInput = new Fl_Int_Input(x + 170, y + 50, 110, 26);
        bindConfigField<Config::gAudioBufferSize, Config::gbNeedSave_Audio>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // SPU RAM size
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 80, 140, 26, "SPU RAM size");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Audio.spuRamSize.comment);

        const auto pInput = new Fl_Int_Input(x + 170, y + 80, 110, 26);
        bindConfigField<Config::gSpuRamSize, Config::gbNeedSave_Audio>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Audio' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populateAudioTab(Context& ctx) noexcept {
    Tab_Audio& tab = ctx.tab_audio;
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);
    makeSettingSection((tabRect.lx + tabRect.rx) / 2 - 150, tabRect.ty + 20);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
