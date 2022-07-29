//------------------------------------------------------------------------------------------------------------------------------------------
// Setup code and event handling logic for the 'Game' tab
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LAUNCHER

#include "Asserts.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Config/ConfigSerialization_Game.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Box.H>
    #include <FL/Fl_Check_Button.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Native_File_Chooser.H>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for motion related options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeMotionSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 200, 30, "Motion");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 200, 170, "");

    // Various toggles
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 120, 30, "  Uncapped framerate");
        bindConfigField<Config::gbUncapFramerate, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.uncapFramerate.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 150, 30, "  Interpolate sectors");
        bindConfigField<Config::gbInterpolateSectors, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.interpolateSectors.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 100, 120, 30, "  Interpolate things");
        bindConfigField<Config::gbInterpolateMobj, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.interpolateMobj.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 130, 120, 30, "  Interpolate monsters");
        bindConfigField<Config::gbInterpolateMonsters, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.interpolateMonsters.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 160, 120, 30, "  Interpolate weapon");
        bindConfigField<Config::gbInterpolateWeapon, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.interpolateWeapon.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for counter related options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeCountersSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 200, 30, "Counters");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 200, 80, "");

    // Various toggles
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 120, 30, "  Enable level timer");
        bindConfigField<Config::gbEnableSinglePlayerLevelTimer, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.enableSinglePlayerLevelTimer.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 150, 30, "  Show perf counters");
        bindConfigField<Config::gbShowPerfCounters, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.showPerfCounters.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the input for setting heap size
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeHeapSizeInput(const int x, const int y) noexcept {
    const auto pLabel_heapSize = new Fl_Box(FL_NO_BOX, x, y + 40, 80, 30, "Heap Size");
    pLabel_heapSize->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel_heapSize->tooltip(ConfigSerialization::gConfig_Game.mainMemoryHeapSize.comment);

    const auto pInput_heapSize = new Fl_Int_Input(x + 90, y + 40, 110, 30);
    bindConfigField<Config::gMainMemoryHeapSize, Config::gbNeedSave_Game>(*pInput_heapSize);
    pInput_heapSize->tooltip(pLabel_heapSize->tooltip());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for bug fix related gameplay options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeBugFixesSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 470, 30, "Bug fixes to apply");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 470, 230, "");

    // Various toggles
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 120, 30, "  Fix line activation");
        bindConfigField<Config::gbFixLineActivation, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.fixLineActivation.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 120, 30, "  Item pickup fix");
        bindConfigField<Config::gbUseItemPickupFix, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.useItemPickupFix.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 100, 120, 30, "  Fix multi-line special crossing");
        bindConfigField<Config::gbFixMultiLineSpecialCrossing, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.fixMultiLineSpecialCrossing.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 130, 120, 30, "  Fix kill count");
        bindConfigField<Config::gbFixKillCount, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.fixKillCount.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 160, 120, 30, "  Player rocket blast fix");
        bindConfigField<Config::gbUsePlayerRocketBlastFix, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.usePlayerRocketBlastFix.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 190, 120, 30, "  Fix view bob strength");
        bindConfigField<Config::gbFixViewBobStrength, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.fixViewBobStrength.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 250, y + 40, 120, 30, "  Fix gravitty strength");
        bindConfigField<Config::gbFixGravityStrength, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.fixGravityStrength.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 250, y + 70, 120, 30, "  Lost soul spawn fix");
        bindConfigField<Config::gbUseLostSoulSpawnFix, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.useLostSoulSpawnFix.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 250, y + 100, 120, 30, "  Line of sight overflow fix");
        bindConfigField<Config::gbUseLineOfSightOverflowFix, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.useLineOfSightOverflowFix.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 250, y + 130, 120, 30, "  Fix outdoor bullet puffs");
        bindConfigField<Config::gbFixOutdoorBulletPuffs, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.fixOutdoorBulletPuffs.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 250, y + 160, 120, 30, "  Fix blocking gibs bug");
        bindConfigField<Config::gbFixBlockingGibsBug, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.fixBlockingGibsBug.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 250, y + 190, 120, 30, "  Fix sound propagation");
        bindConfigField<Config::gbFixSoundPropagation, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.fixSoundPropagation.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for various gameplay tweaks
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeTweaksSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 470, 30, "Tweaks");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 470, 150, "");

    // Various toggles
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 120, 30, "  Use input latency tweak");
        bindConfigField<Config::gbUseMoveInputLatencyTweak, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.useMoveInputLatencyTweak.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 250, y + 40, 120, 30, "  Use extended shoot range");
        bindConfigField<Config::gbUseExtendedPlayerShootRange, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.useExtendedPlayerShootRange.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 120, 30, "  Use SSG delay tweak");
        bindConfigField<Config::gbUseSuperShotgunDelayTweak, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.useSuperShotgunDelayTweak.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 250, y + 70, 120, 30, "  Allow turning cancellation");
        bindConfigField<Config::gbAllowTurningCancellation, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.allowTurningCancellation.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for which map patches to apply
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeMapPatchesSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 190, 30, "Map patches to apply");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 190, 110, "");

    // Various toggles
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 120, 30, "  Gameplay fixes");
        bindConfigField<Config::gbEnableMapPatches_GamePlay, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.enableMapPatches_GamePlay.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 120, 30, "  Visual fixes");
        bindConfigField<Config::gbEnableMapPatches_Visual, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.enableMapPatches_Visual.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 100, 120, 30, "  Fixes for PsyDoom");
        bindConfigField<Config::gbEnableMapPatches_PsyDoom, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.enableMapPatches_PsyDoom.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for loading options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeLoadingSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 190, 30, "Loading");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 190, 80, "");

    // Various toggles
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 40, 150, 30, "  Use fast loading");
        bindConfigField<Config::gbUseFastLoading, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.useFastLoading.comment);
    }

    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 70, 150, 30, "  Skip intros");
        bindConfigField<Config::gbSkipIntros, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.skipIntros.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the section for timing related options
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeTimingsSection(const int x, const int y) noexcept {
    // Container frame
    new Fl_Box(FL_NO_BOX, x, y, 190, 30, "Game tick rate");
    new Fl_Box(FL_THIN_DOWN_BOX, x, y + 30, 190, 90, "");

    // Tick rate mode
    const auto pLabel_mode = new Fl_Box(FL_NO_BOX, x + 20, y + 40, 80, 30, "Mode");
    pLabel_mode->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel_mode->tooltip(ConfigSerialization::gConfig_Game.usePalTimings.comment);

    const auto pChoice_mode = new Fl_Choice(x + 80, y + 40, 90, 30);
    pChoice_mode->add("NTSC");
    pChoice_mode->add("PAL");
    pChoice_mode->add("Auto");
    pChoice_mode->value(0);
    pChoice_mode->tooltip(pLabel_mode->tooltip());
    pChoice_mode->callback(
        [](Fl_Widget* const pWidget, void*) noexcept {
            Fl_Choice* const pChoice = static_cast<Fl_Choice*>(pWidget);

            switch (pChoice->value()) {
                case 0: Config::gUsePalTimings =  0; break;
                case 1: Config::gUsePalTimings = +1; break;
                case 2: Config::gUsePalTimings = -1; break;
            }

            Config::gbNeedSave_Game = true;
        }
    );

    if (Config::gUsePalTimings < 0) {
        pChoice_mode->value(2);
    } else if (Config::gUsePalTimings > 0) {
        pChoice_mode->value(1);
    } else {
        pChoice_mode->value(0);
    }

    // Whether to use demo timings
    {
        const auto pCheck = makeFl_Check_Button(x + 20, y + 80, 150, 30, "  Use demo timings");
        bindConfigField<Config::gbUseDemoTimings, Config::gbNeedSave_Game>(*pCheck);
        pCheck->tooltip(ConfigSerialization::gConfig_Game.useDemoTimings.comment);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the default cue file selector
//------------------------------------------------------------------------------------------------------------------------------------------
static void makeDefaultCueFileSelector(Tab_Game& tab, const int lx, const int rx, const int ty) noexcept {
    const auto pLabel_cueFile = new Fl_Box(FL_NO_BOX, lx, ty, 150, 30, "Default game disc");
    pLabel_cueFile->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pLabel_cueFile->tooltip(ConfigSerialization::gConfig_Game.cueFilePath.comment);

    const auto pButton_clearCueFile = new Fl_Button(rx - 30, ty + 30, 30, 30, "X");
    pButton_clearCueFile->tooltip(pLabel_cueFile->tooltip());
    pButton_clearCueFile->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Game& tab = *(Tab_Game*) pUserData;
            tab.pInput_cueFile->value("");
            Config::gCueFilePath.clear();
            Config::gbNeedSave_Game = true;
        },
        &tab
    );

    const auto pButton_pickCueFile = new Fl_Button(rx - 110, ty + 30, 80, 30, "Browse");
    pButton_pickCueFile->tooltip(pLabel_cueFile->tooltip());
    pButton_pickCueFile->callback(
        [](Fl_Widget*, void* const pUserData) noexcept {
            ASSERT(pUserData);
            Tab_Game& tab = *(Tab_Game*) pUserData;

            const auto pFileChooser = std::make_unique<Fl_Native_File_Chooser>();
            pFileChooser->filter("CUE Sheet Files\t*.{cue}");
            pFileChooser->type(Fl_Native_File_Chooser::BROWSE_FILE);
            pFileChooser->title("Choose a default game disc .cue file");

            if ((pFileChooser->show() == 0) && (pFileChooser->count() == 1)) {
                tab.pInput_cueFile->value(pFileChooser->filename());
                Config::gCueFilePath = pFileChooser->filename();
                Config::gbNeedSave_Game = true;
            }
        },
        &tab
    );

    const int cueFileInputRx = getRectExtents(*pButton_pickCueFile).lx - 10;
    tab.pInput_cueFile = makeFl_Input(lx, ty + 30, cueFileInputRx - lx, 30);
    tab.pInput_cueFile->value(Config::gCueFilePath.c_str());
    tab.pInput_cueFile->tooltip(pLabel_cueFile->tooltip());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the 'Game' tab
//------------------------------------------------------------------------------------------------------------------------------------------
void populate(Tab_Game& tab) noexcept {
    ASSERT(tab.pTab);
    ASSERT(Fl_Group::current() == tab.pTab);

    const RectExtents tabRect = getRectExtents(*tab.pTab);

    makeMotionSection(tabRect.lx + 20, tabRect.ty + 20);
    makeCountersSection(tabRect.lx + 20, tabRect.ty + 230);
    makeHeapSizeInput(tabRect.lx + 20, tabRect.ty + 330);
    makeBugFixesSection(tabRect.lx + 240, tabRect.ty + 20);
    makeTweaksSection(tabRect.lx + 240, tabRect.ty + 290);
    makeMapPatchesSection(tabRect.lx + 730, tabRect.ty + 20);
    makeLoadingSection(tabRect.lx + 730, tabRect.ty + 170);
    makeTimingsSection(tabRect.lx + 730, tabRect.ty + 290);
    makeDefaultCueFileSelector(tab, tabRect.lx + 20, tabRect.rx - 20, 510);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER
