//------------------------------------------------------------------------------------------------------------------------------------------
// Stores settings made in the 'Launcher' page of the PsyDoom launcher.
// Enables the choice of game etc. to be remembered the next time the launcher is used.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LauncherPrefs.h"

#include "FileUtils.h"
#include "IniUtils.h"
#include "Launcher_Context.h"
#include "Launcher_Utils.h"
#include "PsyDoom/Utils.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Check_Button.H>
    #include <FL/Fl_Choice.H>
    #include <FL/Fl_Group.H>
    #include <FL/Fl_Int_Input.H>
END_DISABLE_HEADER_WARNINGS

#include <cstdio>

BEGIN_NAMESPACE(LauncherPrefs)

// Name of the user prefs file: it can reside in either the writable user data folder (default) or in the current working directory.
// We save to the current working directory if the file is found existing there on launch.
static constexpr const char* const PREFS_FILE_NAME = "launcher.ini";

// If true then we save the prefs file to the current working directory rather than to the user data folder
static bool gbUseWorkingDirPrefsFile = false;

// Context: the current launcher tab.
// Used when loading launcher preferences.
static Tab_Launcher* pCurLauncherTab = nullptr;

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the path to the ini file used to hold launcher preferences
//------------------------------------------------------------------------------------------------------------------------------------------
static std::string getPrefsFilePath() noexcept {
    if (gbUseWorkingDirPrefsFile) {
        return PREFS_FILE_NAME;
    } else {
        const std::string userDataFolder = Utils::getOrCreateUserDataFolder();
        return userDataFolder + PREFS_FILE_NAME;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle the loading of an entry in the launcher preferences INI file
//------------------------------------------------------------------------------------------------------------------------------------------
static void loadPrefsFileIniEntry(const IniUtils::IniEntry& entry) noexcept {
    if (entry.key == "input_cue") {
        pCurLauncherTab->pInput_cue->value(entry.value.strValue.c_str());
    }
    else if (entry.key == "input_data_dir") {
        pCurLauncherTab->pInput_dataDir->value(entry.value.strValue.c_str());
    }
    else if (entry.key == "check_record_demos") {
        pCurLauncherTab->pCheck_recordDemos->value(entry.value.tryGetAsInt(0));
    }
    else if (entry.key == "check_pistol_start") {
        pCurLauncherTab->pCheck_pistolStart->value(entry.value.tryGetAsInt(0));
    }
    else if (entry.key == "check_turbo") {
        pCurLauncherTab->pCheck_turbo->value(entry.value.tryGetAsInt(0));
    }
    else if (entry.key == "check_no_monsters") {
        pCurLauncherTab->pCheck_noMonsters->value(entry.value.tryGetAsInt(0));
    }
    else if (entry.key == "check_nm_boss_fixup") {
        pCurLauncherTab->pCheck_nmBossFixup->value(entry.value.tryGetAsInt(0));
    }
    else if (entry.key == "input_net_host") {
        pCurLauncherTab->pInput_netHost->value(entry.value.strValue.c_str());
    }
    else if (entry.key == "input_net_port") {
        pCurLauncherTab->pInput_netPort->value(entry.value.strValue.c_str());
    }
    else if (entry.key == "choice_net_peer_type") {
        pCurLauncherTab->pChoice_netPeerType->value(entry.value.tryGetAsInt(0));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to loads the preferences for the launcher from the INI file.
// If successful then the preferences are applied to the 'Launcher' tab in the launcher UI.
//------------------------------------------------------------------------------------------------------------------------------------------
void load(Tab_Launcher& tab) noexcept {
    // Should we use a prefs file in the current working directory?
    gbUseWorkingDirPrefsFile = FileUtils::fileExists(PREFS_FILE_NAME);

    // Set some context for loading
    pCurLauncherTab = &tab;

    // Read the .ini file if it exists
    const std::string prefsFilePath = getPrefsFilePath();

    if (FileUtils::fileExists(prefsFilePath.c_str())) {
        const FileData prefsFileData = FileUtils::getContentsOfFile(prefsFilePath.c_str(), 1, std::byte(0));
        IniUtils::parseIniFromString((const char*) prefsFileData.bytes.get(), prefsFileData.size - 1, loadPrefsFileIniEntry);
    }

    // Clear loading context and update the UI in case the net peer type updated from the default
    pCurLauncherTab = nullptr;

    if (tab.onNetPeerTypeUpdated) {
        tab.onNetPeerTypeUpdated(tab);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Saves the preferences for the launcher
//------------------------------------------------------------------------------------------------------------------------------------------
void save(Tab_Launcher& tab) noexcept {
    // Open up the prefs file
    const std::string prefsFilePath = getPrefsFilePath();
    std::FILE* const pFile = std::fopen(prefsFilePath.c_str(), "wt");

    if (!pFile)
        return;

    // Make sure all the string based preferences are trimmed
    const std::string input_cue = Launcher::trimText(tab.pInput_cue->value());
    const std::string input_data_dir = Launcher::trimText(tab.pInput_dataDir->value());
    const std::string input_net_host = Launcher::trimText(tab.pInput_netHost->value());
    const std::string input_net_port = Launcher::trimText(tab.pInput_netPort->value());

    // Write out the preferences
    std::fprintf(pFile, "; WARNING: this file is auto-generated by PsyDoom, it may be overwritten at any time!\n");
    std::fprintf(pFile, "input_cue = %s\n", input_cue.c_str());
    std::fprintf(pFile, "input_data_dir = %s\n", input_data_dir.c_str());
    std::fprintf(pFile, "check_record_demos = %d\n", (tab.pCheck_recordDemos->value()) ? 1 : 0);
    std::fprintf(pFile, "check_pistol_start = %d\n", (tab.pCheck_pistolStart->value()) ? 1 : 0);
    std::fprintf(pFile, "check_turbo = %d\n", (tab.pCheck_turbo->value()) ? 1 : 0);
    std::fprintf(pFile, "check_no_monsters = %d\n", (tab.pCheck_noMonsters->value()) ? 1 : 0);
    std::fprintf(pFile, "check_nm_boss_fixup = %d\n", (tab.pCheck_nmBossFixup->value()) ? 1 : 0);
    std::fprintf(pFile, "input_net_host = %s\n", input_net_host.c_str());
    std::fprintf(pFile, "input_net_port = %s\n", input_net_port.c_str());
    std::fprintf(pFile, "choice_net_peer_type = %d\n", tab.pChoice_netPeerType->value());

    // Flush and close to finish up
    std::fflush(pFile);
    std::fclose(pFile);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if launcher preferences should be saved
//------------------------------------------------------------------------------------------------------------------------------------------
bool shouldSave(Tab_Launcher& tab) noexcept {
    if (tab.pTab->changed())
        return true;

    const int numChildren = tab.pTab->children();

    for (int i = 0; i < numChildren; ++i) {
        const Fl_Widget* const pWidget = tab.pTab->child(i);

        if (pWidget && pWidget->changed())
            return true;
    }

    return false;
}

END_NAMESPACE(LauncherPrefs)
