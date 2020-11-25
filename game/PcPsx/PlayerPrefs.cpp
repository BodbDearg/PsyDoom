//------------------------------------------------------------------------------------------------------------------------------------------
// Management of some in-game player preferences and saving passwords for the last level completed so it can be restored on relaunch
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PlayerPrefs.h"

#include "Doom/Base/s_sound.h"
#include "Doom/UI/o_main.h"
#include "Doom/UI/pw_main.h"
#include "FileUtils.h"
#include "IniUtils.h"
#include "Utils.h"
#include "Wess/psxspu.h"

#include <algorithm>
#include <cstdio>

BEGIN_NAMESPACE(PlayerPrefs)

// Name of the user prefs file: it can reside in either the writable user data folder (default) or in the current working directory.
// We save to the current working directory if the file is found existing there on launch.
static constexpr char* const PREFS_FILE_NAME = "saved_prefs.ini";

int32_t     gSoundVol;                      // Option for sound volume
int32_t     gMusicVol;                      // Option for music volume
char        gLastPassword[PASSWORD_LEN];    // Password for the current level the player is on
int32_t     gTurnSpeedMult100;              // In-game tweakable turn speed multiplier expressed in integer percentage points (0-500)
bool        gbAlwaysRun;                    // If set then the player runs by default and the run action causes slower walking

// If true then we save the prefs file to the current working directory rather than to the user data folder
static bool gbUseWorkingDirPrefsFile = false;

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the path to the ini file used to hold player prefs
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
// Converts a character to a PSX Doom password character index.
// Returns -1 if there is no valid conversion.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t charToPwCharIndex(const char c) noexcept {
    const char cUpper = (char) std::toupper(c);

    switch (cUpper) {
        case 'B': return 0;     case 'L': return 8;     case 'V': return 16;    case '3': return 24;
        case 'C': return 1;     case 'M': return 9;     case 'W': return 17;    case '4': return 25;
        case 'D': return 2;     case 'N': return 10;    case 'X': return 18;    case '5': return 26;
        case 'F': return 3;     case 'P': return 11;    case 'Y': return 19;    case '6': return 27;
        case 'G': return 4;     case 'Q': return 12;    case 'Z': return 20;    case '7': return 28;
        case 'H': return 5;     case 'R': return 13;    case '0': return 21;    case '8': return 29;
        case 'J': return 6;     case 'S': return 14;    case '1': return 22;    case '9': return 30;
        case 'K': return 7;     case 'T': return 15;    case '2': return 23;    case '!': return 31;
    }

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Converts PSX Doom password character index to a character.
// Returns 'NUL' if there is no valid conversion.
//------------------------------------------------------------------------------------------------------------------------------------------
static char pwCharIndexToChar(const int32_t pwCharIdx) noexcept {
    switch (pwCharIdx) {
        case 0: return 'B';     case 8:  return 'L';    case 16: return 'V';    case 24: return '3';
        case 1: return 'C';     case 9:  return 'M';    case 17: return 'W';    case 25: return '4';
        case 2: return 'D';     case 10: return 'N';    case 18: return 'X';    case 26: return '5';
        case 3: return 'F';     case 11: return 'P';    case 19: return 'Y';    case 27: return '6';
        case 4: return 'G';     case 12: return 'Q';    case 20: return 'Z';    case 28: return '7';
        case 5: return 'H';     case 13: return 'R';    case 21: return '0';    case 29: return '8';
        case 6: return 'J';     case 14: return 'S';    case 22: return '1';    case 30: return '9';
        case 7: return 'K';     case 15: return 'T';    case 23: return '2';    case 31: return '!';
    }

    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle the loading of an entry in the .ini prefs file
//------------------------------------------------------------------------------------------------------------------------------------------
static void loadPrefsFileIniEntry(const IniUtils::Entry& entry) noexcept {
    if (entry.key == "soundVol") {
        gSoundVol = std::clamp(entry.getIntValue(gSoundVol), VOLUME_MIN, VOLUME_MAX);
    } 
    else if (entry.key == "musicVol") {
        gMusicVol = std::clamp(entry.getIntValue(gMusicVol), VOLUME_MIN, VOLUME_MAX);
    }
    else if (entry.key == "lastPassword") {
        std::memset(gLastPassword, 0, sizeof(gLastPassword));
        std::memcpy(gLastPassword, entry.value.c_str(), std::min((int32_t) entry.value.length(), PASSWORD_LEN));    // Ignore chars over the length limit

        // Sanitize the password chars: set unrecognised ones to null and uppercase everything
        for (char& c : gLastPassword) {
            c = (char) std::toupper(c);
            c = (charToPwCharIndex(c) >= 0) ? c : 0;
        }
    }
    else if (entry.key == "turnSpeedPercentMultiplier") {
        gTurnSpeedMult100 = std::clamp(entry.getIntValue(gTurnSpeedMult100), TURN_SPEED_MULT_MIN, TURN_SPEED_MULT_MAX);
    }
    else if (entry.key == "alwaysRun") {
        gbAlwaysRun = entry.getBoolValue(gbAlwaysRun);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set all preferences to the defaults
//------------------------------------------------------------------------------------------------------------------------------------------
void setToDefaults() noexcept {
    // Note: make sound volume 85 by default (Final Doom volume level) to make the music pop a bit more
    gSoundVol = 85;
    gMusicVol = 100;

    // Password is empty by default
    std::memset(gLastPassword, 0, sizeof(gLastPassword));

    // Turn speed is normal by default and auto-run off
    gTurnSpeedMult100 = 100;
    gbAlwaysRun = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the player preferences from the preferences file.
// If the file does not exist then the preferences are defaulted.
//------------------------------------------------------------------------------------------------------------------------------------------
void load() noexcept {
    // Firstly set everything to the defaults and determine whether we use a prefs file found in the current working directory.
    // We only do that if the .ini file is found there on launch!
    setToDefaults();
    gbUseWorkingDirPrefsFile = FileUtils::fileExists(PREFS_FILE_NAME);
    
    // Read the .ini file if it exists, otherwise stop here
    const std::string prefsFilePath = getPrefsFilePath();

    if (!FileUtils::fileExists(prefsFilePath.c_str()))
        return;
    
    const FileData prefsFileData = FileUtils::getContentsOfFile(prefsFilePath.c_str(), 1, std::byte(0));
    IniUtils::parseIniFromString((const char*) prefsFileData.bytes.get(), prefsFileData.size - 1, loadPrefsFileIniEntry);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Save the player preferences the preferences file
//------------------------------------------------------------------------------------------------------------------------------------------
void save() noexcept {
    // Open up the prefs file
    const std::string prefsFilePath = getPrefsFilePath();
    std::FILE* const pFile = std::fopen(prefsFilePath.c_str(), "wt");

    if (!pFile)
        return;

    // Make up a null terminated string for last password
    char lastPassword[PASSWORD_LEN + 1];
    std::memcpy(lastPassword, gLastPassword, sizeof(gLastPassword));
    lastPassword[PASSWORD_LEN] = 0;

    // Write out the preferences
    std::fprintf(pFile, "; WARNING: this file is auto-generated by PsyDoom, it may be overwritten at any time!\n");
    std::fprintf(pFile, "soundVol = %d\n", gSoundVol);
    std::fprintf(pFile, "musicVol = %d\n", gMusicVol);
    std::fprintf(pFile, "lastPassword = %s\n", lastPassword);
    std::fprintf(pFile, "turnSpeedPercentMultiplier = %d\n", gTurnSpeedMult100);
    std::fprintf(pFile, "alwaysRun = %d\n", (int) gbAlwaysRun);

    // Flush and close to finish up
    std::fflush(pFile);
    std::fclose(pFile);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the turn speed multiplier expressed as a floating point fraction where 1.0 is 100%
//------------------------------------------------------------------------------------------------------------------------------------------
float getTurnSpeedMultiplier() noexcept {
    return (float) gTurnSpeedMult100 / 100.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Apply the current sound and music preferences to the options sound and music preferences
//------------------------------------------------------------------------------------------------------------------------------------------
void pushSoundAndMusicPrefs() noexcept {
    gOptionsSndVol = gSoundVol;
    gOptionsMusVol = gMusicVol;
    gCdMusicVol = (gMusicVol * PSXSPU_MAX_CD_VOL) / S_MAX_VOL;      // Calculation copied from the options menu volume adjust
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remember the current sound and music preferences stored in the options menu
//------------------------------------------------------------------------------------------------------------------------------------------
void pullSoundAndMusicPrefs() noexcept {
    gSoundVol = gOptionsSndVol;
    gMusicVol = gOptionsMusVol;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Apply the saved last password to the password system
//------------------------------------------------------------------------------------------------------------------------------------------
void pushLastPassword() noexcept {
    // Clear the current password
    gNumPasswordCharsEntered = 0;
    std::memset(gPasswordCharBuffer, 0, sizeof(gPasswordCharBuffer));

    // Add in password characters to the buffer until we encounter an invalid one
    static_assert(C_ARRAY_SIZE(gPasswordCharBuffer) == C_ARRAY_SIZE(gLastPassword));

    for (const char pwChar : gLastPassword) {
        const int32_t pwCharIdx = charToPwCharIndex(pwChar);

        if (pwCharIdx < 0)
            break;

        gPasswordCharBuffer[gNumPasswordCharsEntered] = (uint8_t) pwCharIdx;
        gNumPasswordCharsEntered++;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remember current password input in the password system, so that it may be saved later to player preferences
//------------------------------------------------------------------------------------------------------------------------------------------
void pullLastPassword() noexcept {
    std::memset(gLastPassword, 0, sizeof(gLastPassword));

    for (int32_t i = 0; i < gNumPasswordCharsEntered; ++i) {
        gLastPassword[i] = pwCharIndexToChar(gPasswordCharBuffer[i]);
    }
}

END_NAMESPACE(PlayerPrefs)
