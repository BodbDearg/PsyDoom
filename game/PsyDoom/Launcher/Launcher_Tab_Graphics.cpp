//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Graphics' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Config/ConfigSerialization_Graphics.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Group.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for output options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeOutputOptionsSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 210, 30, "Output settings");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 210, 150, "");

    // Fullscreen toggle
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 150, 30, "  Fullscreen");
        bindConfigField<Config::gbFullscreen, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.fullscreen.comment);
    }

    // Vsync toggle
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 150, 30, "  Enable vsync");
        bindConfigField<Config::gbEnableVSync, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.enableVSync.comment);
    }

    // Output resolution: width
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 110, 80, 26, "Output width");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.outputResolutionW.comment);

        const auto pInput = new Fl_Int_Input(x + 130, y + 110, 60, 26);
        bindConfigField<Config::gOutputResolutionW, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Output resolution: height
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 140, 80, 26, "Output height");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.outputResolutionW.comment);

        const auto pInput = new Fl_Int_Input(x + 130, y + 140, 60, 26);
        bindConfigField<Config::gOutputResolutionH, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for picture crop and strecth related options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makePictureCropAndStretchSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 270, 30, "Picture crop & stretch");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 270, 110, "");

    // Top overscan pixels
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 40, 80, 26, "Top overscan pixels");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.topOverscanPixels.comment);

        const auto pInput = new Fl_Int_Input(x + 190, y + 40, 60, 26);
        bindConfigField<Config::gTopOverscanPixels, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Bottom overscan pixels
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 70, 80, 26, "Bottom overscan pixels");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.topOverscanPixels.comment);

        const auto pInput = new Fl_Int_Input(x + 190, y + 70, 60, 26);
        bindConfigField<Config::gBottomOverscanPixels, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Logical display width
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 100, 80, 26, "Logical display width");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.logicalDisplayWidth.comment);

        const auto pInput = new Fl_Float_Input(x + 190, y + 100, 60, 26);
        bindConfigField<Config::gLogicalDisplayW, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the general settings section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeGeneralSettingsSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 250, 30, "General");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 240, 50, "");

    // VRAM size in MB
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 40, 80, 26, "VRAM size (MiB)");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.vramSizeInMegabytes.comment);

        const auto pInput = new Fl_Int_Input(x + 160, y + 40, 60, 26);
        bindConfigField<Config::gVramSizeInMegabytes, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the classic renderer settings section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeClassicRendererSettingsSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 250, 30, "Classic renderer");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 240, 80, "");

    // Floor render gap fix
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 150, 30, "  Use floor render gap fix");
        bindConfigField<Config::gbFloorRenderGapFix, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.floorRenderGapFix.comment);
    }

    // Sky leak fix
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 150, 30, "  Use sky leak fix");
        bindConfigField<Config::gbSkyLeakFix, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.skyLeakFix.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Graphics' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Graphics& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);

    makeOutputOptionsSection(tabRect.lx + 20, tabRect.ty + 20);
    makePictureCropAndStretchSection(tabRect.lx + 20, tabRect.ty + 210);
    makeGeneralSettingsSection(tabRect.lx + 20, tabRect.ty + 370);
    makeClassicRendererSettingsSection(tabRect.lx + 310, tabRect.ty + 20);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
