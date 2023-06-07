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
    #include <FL/Fl_Choice.H>
    #include <FL/Fl_Group.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Applies the 'Modern, high resolution' Vulkan renderer settings preset
//------------------------------------------------------------------------------------------------------------------------------------------
static void applyVkSettingsPreset_modern(Tab_Graphics& tab) noexcept {
    tab.pInput_renderHeight->value("-1");
    tab.pInput_aaMultisamples->value(std::to_string(Config::gDefaultAntiAliasingMultisamples).c_str());
    tab.pCheck_pixelStretch->value(0);
    tab.pCheck_widescreenEnabled->value(1);
    tab.pCheck_32bitShading->value(0);
    tab.pCheck_drawExtStatusBar->value(1);
    tab.pCheck_brightenAutomap->value(1);

    Config::gVulkanRenderHeight = -1;
    Config::gAAMultisamples = Config::gDefaultAntiAliasingMultisamples;
    Config::gbVulkanPixelStretch = false;
    Config::gbVulkanWidescreenEnabled = true;
    Config::gbUseVulkan32BitShading = false;
    Config::gbVulkanDrawExtendedStatusBar = true;
    Config::gbVulkanBrightenAutomap = true;

    Config::gbNeedSave_Graphics = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Applies the 'Classic, low resolution' Vulkan renderer settings preset
//------------------------------------------------------------------------------------------------------------------------------------------
static void applyVkSettingsPreset_classic(Tab_Graphics& tab) noexcept {
    tab.pInput_renderHeight->value("240");
    tab.pInput_aaMultisamples->value("1");
    tab.pCheck_pixelStretch->value(1);
    tab.pCheck_widescreenEnabled->value(1);
    tab.pCheck_32bitShading->value(0);
    tab.pCheck_drawExtStatusBar->value(1);
    tab.pCheck_brightenAutomap->value(0);

    Config::gVulkanRenderHeight = 240;
    Config::gAAMultisamples = 1;
    Config::gbVulkanPixelStretch = true;
    Config::gbVulkanWidescreenEnabled = true;
    Config::gbUseVulkan32BitShading = false;
    Config::gbVulkanDrawExtendedStatusBar = true;
    Config::gbVulkanBrightenAutomap = false;

    Config::gbNeedSave_Graphics = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Applies the 'Classic, double resolution' Vulkan renderer settings preset
//------------------------------------------------------------------------------------------------------------------------------------------
static void applyVkSettingsPreset_classicDouble(Tab_Graphics& tab) noexcept {
    tab.pInput_renderHeight->value("480");
    tab.pInput_aaMultisamples->value("1");
    tab.pCheck_pixelStretch->value(1);
    tab.pCheck_widescreenEnabled->value(1);
    tab.pCheck_32bitShading->value(0);
    tab.pCheck_drawExtStatusBar->value(1);
    tab.pCheck_brightenAutomap->value(0);

    Config::gVulkanRenderHeight = 480;
    Config::gAAMultisamples = 1;
    Config::gbVulkanPixelStretch = true;
    Config::gbVulkanWidescreenEnabled = true;
    Config::gbUseVulkan32BitShading = false;
    Config::gbVulkanDrawExtendedStatusBar = true;
    Config::gbVulkanBrightenAutomap = false;

    Config::gbNeedSave_Graphics = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Updates which widgets are enabled in the 'Vulkan renderer' settings section
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateVulkanRendererEnabledWidgets(Tab_Graphics& tab) noexcept {
    // Can we use Vulkan at all and is it enabled currently?
    const bool bCanUseVulkan = Config::gbCouldDetermineVulkanConfigDefaults;
    const bool bIsVulkanEnabled = (bCanUseVulkan && (tab.pCheck_disableVkRenderer->value() == 0));

    // Update the activated status of all widgets
    const auto activateWidget = [](Fl_Widget& widget, const bool bActivate) noexcept {
        if (bActivate) {
            widget.activate();
        } else {
            widget.deactivate();
        }
    };

    activateWidget(*tab.pLabel_settingsPreset, bIsVulkanEnabled);
    activateWidget(*tab.pChoice_settingsPreset, bIsVulkanEnabled);
    activateWidget(*tab.pButton_settingsPreset, bIsVulkanEnabled);
    activateWidget(*tab.pLabel_renderHeight, bIsVulkanEnabled);
    activateWidget(*tab.pInput_renderHeight, bIsVulkanEnabled);
    activateWidget(*tab.pLabel_aaMultisamples, bIsVulkanEnabled);
    activateWidget(*tab.pInput_aaMultisamples, bIsVulkanEnabled);
    activateWidget(*tab.pCheck_pixelStretch, bIsVulkanEnabled);
    activateWidget(*tab.pCheck_tripleBuffer, bIsVulkanEnabled);
    activateWidget(*tab.pCheck_widescreenEnabled, bIsVulkanEnabled);
    activateWidget(*tab.pCheck_32bitShading, bIsVulkanEnabled);
    activateWidget(*tab.pCheck_drawExtStatusBar, bIsVulkanEnabled);
    activateWidget(*tab.pCheck_brightenAutomap, bIsVulkanEnabled);
    activateWidget(*tab.pCheck_disableVkRenderer, bCanUseVulkan);
    activateWidget(*tab.pLabel_prefDevicesRegex, bIsVulkanEnabled);
    activateWidget(*tab.pInput_prefDevicesRegex, bIsVulkanEnabled);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for output options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeOutputOptionsSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 300, 30, "Output settings");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 300, 180, "");

    // Fullscreen toggle
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 120, 30, "  Fullscreen");
        bindConfigField<Config::gbFullscreen, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.fullscreen.comment);
    }

    // Vsync toggle
    {
        const auto pCheck = makeFl_Check_Button(x + 150, y + 40, 120, 30, "  Enable vsync");
        bindConfigField<Config::gbEnableVSync, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.enableVSync.comment);
    }

    // Output resolution: width
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 80, 80, 26, "Resolution: width");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.outputResolutionW.comment);

        const auto pInput = new Fl_Int_Input(x + 200, y + 80, 80, 26);
        bindConfigField<Config::gOutputResolutionW, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Output resolution: height
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 110, 80, 26, "Resolution: height");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.outputResolutionW.comment);

        const auto pInput = new Fl_Int_Input(x + 200, y + 110, 80, 26);
        bindConfigField<Config::gOutputResolutionH, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Output display index
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 140, 80, 26, "Output display index");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.outputDisplayIndex.comment);

        const auto pInput = new Fl_Int_Input(x + 200, y + 140, 80, 26);
        bindConfigField<Config::gOutputDisplayIndex, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Exclusive fullscreen mode toggle
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 170, 120, 30, "  Exclusive fullscreen mode");
        bindConfigField<Config::gbExclusiveFullscreenMode, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.exclusiveFullscreenMode.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for picture crop and strecth related options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makePictureCropAndStretchSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 300, 30, "Picture crop & stretch");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 300, 110, "");

    // Top overscan pixels
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 40, 80, 26, "Top overscan pixels");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.topOverscanPixels.comment);

        const auto pInput = new Fl_Int_Input(x + 210, y + 40, 70, 26);
        bindConfigField<Config::gTopOverscanPixels, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Bottom overscan pixels
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 70, 80, 26, "Bottom overscan pixels");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.topOverscanPixels.comment);

        const auto pInput = new Fl_Int_Input(x + 210, y + 70, 70, 26);
        bindConfigField<Config::gBottomOverscanPixels, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }

    // Logical display width
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 100, 80, 26, "Logical display width");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.logicalDisplayWidth.comment);

        const auto pInput = new Fl_Float_Input(x + 210, y + 100, 70, 26);
        bindConfigField<Config::gLogicalDisplayW, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the general settings section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeGeneralSettingsSection(Tab_Graphics& tab, const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 300, 30, "General");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 300, 120, "");

    // VRAM size in MB
    {
        const auto pLabel = new Fl_Box(FL_NO_BOX, x + 20, y + 40, 80, 26, "VRAM size (MiB)");
        pLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        pLabel->tooltip(ConfigSerialization::gConfig_Graphics.vramSizeInMegabytes.comment);

        const auto pInput = new Fl_Int_Input(x + 200, y + 40, 80, 26);
        bindConfigField<Config::gVramSizeInMegabytes, Config::gbNeedSave_Graphics>(*pInput);
        pInput->tooltip(pLabel->tooltip());

        // This setting does nothing in a non-limit removing build
        #if !PSYDOOM_LIMIT_REMOVING
            pLabel->deactivate();
            pInput->deactivate();
        #endif
    }

    // Extended automap colors cheat
    tab.pCheck_useExtendedAutomapColors = makeFl_Check_Button(x + 20, y + 80, 220, 30, "  Use extended automap colors");
    bindConfigField<Config::gbUseExtendedAutomapColors, Config::gbNeedSave_Graphics>(*tab.pCheck_useExtendedAutomapColors);
    tab.pCheck_useExtendedAutomapColors->tooltip(ConfigSerialization::gConfig_Graphics.useExtendedAutomapColors.comment);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the Vulkan renderer settings section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeVulkanRendererSettingsSection(Tab_Graphics& tab, const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 580, 30, (Config::gbCouldDetermineVulkanConfigDefaults) ? "Vulkan renderer" : "Vulkan renderer (unavailable)");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 580, 330, "");

    // Graphic presets
    tab.pLabel_settingsPreset = new Fl_Box(FL_NO_BOX, x + 20, y + 40, 120, 26, "Apply settings preset");
    tab.pLabel_settingsPreset->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    tab.pLabel_settingsPreset->tooltip(
        "Use the 'Apply' button to auto-configure the Vulkan renderer using the chosen settings preset.\n"
        "Available presets are:\n"
        "\n"
        " - Modern, high resolution: run at modern high resolutions.\n"
        " - Classic, low resolution: run at the original PSX resolution but with widescreen enabled.\n"
        " - Classic, double resolution: run at 2x the original PSX resolution and with widescreen.\n"
    );

    tab.pChoice_settingsPreset = new Fl_Choice(x + 210, y + 40, 240, 26);
    tab.pChoice_settingsPreset->tooltip(tab.pLabel_settingsPreset->tooltip());
    tab.pChoice_settingsPreset->add("Modern, high resolution");
    tab.pChoice_settingsPreset->add("Classic, low resolution");
    tab.pChoice_settingsPreset->add("Classic, double resolution");
    tab.pChoice_settingsPreset->value(0);

    tab.pButton_settingsPreset = new Fl_Button(x + 460, y + 40, 100, 26, "Apply");
    tab.pButton_settingsPreset->tooltip(tab.pLabel_settingsPreset->tooltip());
    tab.pButton_settingsPreset->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Graphics& tab = *static_cast<Tab_Graphics*>(pUserData);

            if (tab.pChoice_settingsPreset->value() == 1) {
                applyVkSettingsPreset_classic(tab);
            }
            else if (tab.pChoice_settingsPreset->value() == 2) {
                applyVkSettingsPreset_classicDouble(tab);
            }
            else {
                applyVkSettingsPreset_modern(tab);
            }
        },
        &tab
    );

    // Render height
    tab.pLabel_renderHeight = new Fl_Box(FL_NO_BOX, x + 20, y + 100, 220, 26, "Render height");
    tab.pLabel_renderHeight->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    tab.pLabel_renderHeight->tooltip(ConfigSerialization::gConfig_Graphics.vulkanRenderHeight.comment);

    tab.pInput_renderHeight = new Fl_Int_Input(x + 240, y + 100, 100, 26);
    bindConfigField<Config::gVulkanRenderHeight, Config::gbNeedSave_Graphics>(*tab.pInput_renderHeight);
    tab.pInput_renderHeight->tooltip(tab.pLabel_renderHeight->tooltip());

    // Anti aliasing multisamples
    tab.pLabel_aaMultisamples = new Fl_Box(FL_NO_BOX, x + 20, y + 130, 220, 26, "Anti-aliasing multisamples");
    tab.pLabel_aaMultisamples->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    tab.pLabel_aaMultisamples->tooltip(ConfigSerialization::gConfig_Graphics.antiAliasingMultisamples.comment);

    tab.pInput_aaMultisamples = new Fl_Int_Input(x + 240, y + 130, 100, 26);
    bindConfigField<Config::gAAMultisamples, Config::gbNeedSave_Graphics>(*tab.pInput_aaMultisamples);
    tab.pInput_aaMultisamples->tooltip(tab.pLabel_aaMultisamples->tooltip());

    // Various toggles
    tab.pCheck_pixelStretch = makeFl_Check_Button(x + 20, y + 180, 220, 30, "  Mimic original pixel stretch");
    bindConfigField<Config::gbVulkanPixelStretch, Config::gbNeedSave_Graphics>(*tab.pCheck_pixelStretch);
    tab.pCheck_pixelStretch->tooltip(ConfigSerialization::gConfig_Graphics.vulkanPixelStretch.comment);

    tab.pCheck_tripleBuffer = makeFl_Check_Button(x + 310, y + 180, 220, 30, "  Triple buffer");
    bindConfigField<Config::gbVulkanTripleBuffer, Config::gbNeedSave_Graphics>(*tab.pCheck_tripleBuffer);
    tab.pCheck_tripleBuffer->tooltip(ConfigSerialization::gConfig_Graphics.vulkanTripleBuffer.comment);

    tab.pCheck_widescreenEnabled = makeFl_Check_Button(x + 20, y + 210, 220, 30, "  Widescreen enabled");
    bindConfigField<Config::gbVulkanWidescreenEnabled, Config::gbNeedSave_Graphics>(*tab.pCheck_widescreenEnabled);
    tab.pCheck_widescreenEnabled->tooltip(ConfigSerialization::gConfig_Graphics.vulkanWidescreenEnabled.comment);

    tab.pCheck_32bitShading = makeFl_Check_Button(x + 310, y + 210, 220, 30, "  Use 32-bit shading");
    bindConfigField<Config::gbUseVulkan32BitShading, Config::gbNeedSave_Graphics>(*tab.pCheck_32bitShading);
    tab.pCheck_32bitShading->tooltip(ConfigSerialization::gConfig_Graphics.useVulkan32BitShading.comment);

    tab.pCheck_drawExtStatusBar = makeFl_Check_Button(x + 20, y + 240, 220, 30, "  Draw extended status bar");
    bindConfigField<Config::gbVulkanDrawExtendedStatusBar, Config::gbNeedSave_Graphics>(*tab.pCheck_drawExtStatusBar);
    tab.pCheck_drawExtStatusBar->tooltip(ConfigSerialization::gConfig_Graphics.vulkanDrawExtendedStatusBar.comment);

    tab.pCheck_brightenAutomap = makeFl_Check_Button(x + 310, y + 240, 220, 30, "  Brighten automap");
    bindConfigField<Config::gbVulkanBrightenAutomap, Config::gbNeedSave_Graphics>(*tab.pCheck_brightenAutomap);
    tab.pCheck_brightenAutomap->tooltip(ConfigSerialization::gConfig_Graphics.vulkanBrightenAutomap.comment);

    tab.pCheck_disableVkRenderer = makeFl_Check_Button(x + 20, y + 270, 220, 30, "  Disable Vulkan Renderer");
    tab.pCheck_disableVkRenderer->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Graphics& tab = *static_cast<Tab_Graphics*>(pUserData);
            Config::gbDisableVulkanRenderer = tab.pCheck_disableVkRenderer->value();
            Config::gbNeedSave_Graphics = true;

            // Need to update which widgets are grayed out when this disable toggle is switched
            updateVulkanRendererEnabledWidgets(tab);
        },
        &tab
    );
    tab.pCheck_disableVkRenderer->value(Config::gbDisableVulkanRenderer);
    tab.pCheck_disableVkRenderer->tooltip(ConfigSerialization::gConfig_Graphics.disableVulkanRenderer.comment);

    // Preferred devices regex
    tab.pLabel_prefDevicesRegex = new Fl_Box(FL_NO_BOX, x + 20, y + 320, 200, 26, "Preferred devices regex");
    tab.pLabel_prefDevicesRegex->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    tab.pLabel_prefDevicesRegex->tooltip(ConfigSerialization::gConfig_Graphics.vulkanPreferredDevicesRegex.comment);

    tab.pInput_prefDevicesRegex = new Fl_Input(x + 220, y + 320, 340, 26);
    bindConfigField<Config::gVulkanPreferredDevicesRegex, Config::gbNeedSave_Graphics>(*tab.pInput_prefDevicesRegex);
    tab.pInput_prefDevicesRegex->tooltip(tab.pLabel_prefDevicesRegex->tooltip());

    // Update which wigets are active
    updateVulkanRendererEnabledWidgets(tab);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the classic renderer settings section
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeClassicRendererSettingsSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 580, 30, "Classic renderer");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 580, 120, "");

    // Enhance wall draw precision
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 150, 30, "  Enhance wall draw precision");
        bindConfigField<Config::gbEnhanceWallDrawPrecision, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.enhanceWallDrawPrecision.comment);
    }

    // Floor render gap fix
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 150, 30, "  Use floor render gap fix");
        bindConfigField<Config::gbFloorRenderGapFix, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.floorRenderGapFix.comment);
    }

    // Sky leak fix
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 100, 150, 30, "  Use sky leak fix");
        bindConfigField<Config::gbSkyLeakFix, Config::gbNeedSave_Graphics>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Graphics.skyLeakFix.comment);

        // This setting does nothing in a non-limit removing build
        #if !PSYDOOM_LIMIT_REMOVING
            pCheck->deactivate();
        #endif
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Graphics' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populateGraphicsTab(Context& ctx) noexcept {
    Tab_Graphics& tab = ctx.tab_graphics;
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);

    makeOutputOptionsSection(tabRect.lx + 20, tabRect.ty + 20);
    makePictureCropAndStretchSection(tabRect.lx + 20, tabRect.ty + 240);
    makeGeneralSettingsSection(tab, tabRect.lx + 20, tabRect.ty + 390);
    makeVulkanRendererSettingsSection(tab, tabRect.lx + 340, tabRect.ty + 20);
    makeClassicRendererSettingsSection(tabRect.lx + 340, tabRect.ty + 390);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
