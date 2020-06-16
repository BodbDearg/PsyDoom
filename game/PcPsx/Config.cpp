#include "Config.h"

#include "FileUtils.h"
#include "Finally.h"
#include "IniUtils.h"
#include "Utils.h"

#include <SDL.h>
#include <functional>
#include <vector>

BEGIN_NAMESPACE(Config)

// MSVC: 't' file open option will cause '\n' to become '\r\n' (platform native EOL).
// This is useful because it allows us to use '\n' in the code and have it converted transparently.
#if _MSC_VER
    static constexpr const char* FOPEN_WRITE_APPEND_TEXT = "at";
#else
    static constexpr const char* FOPEN_WRITE_APPEND_TEXT = "a";
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines functionality and data pertaining to a particular config field
//------------------------------------------------------------------------------------------------------------------------------------------
struct ConfigFieldHandler {
    // The string key for the config field
    const char* name;
    
    // What text to insert into the config .ini file when the config value is not present
    const char* iniDefaultStr;
    
    // Logic for parsing the config value
    std::function<void (const IniUtils::Entry& iniEntry)> parseFunc;

    // Logic to set the config value to its default when it's not available in the .ini file
    std::function<void ()> setValueToDefaultFunc;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Graphics config settings
//------------------------------------------------------------------------------------------------------------------------------------------
static const ConfigFieldHandler GRAPHICS_CFG_INI_HANDLERS[] = {
    {
        "Fullscreen",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Fullscreen or windowed mode toggle.\n"
        "# Set to '1' cause the PsyDoom to launch in fullscreen mode, and '0' to use windowed mode.\n"
        "#---------------------------------------------------------------------------------------------------\n"
        "Fullscreen = 1\n",
        [](const IniUtils::Entry& iniEntry) { gbFullscreen = iniEntry.getBoolValue(true); },
        []() { gbFullscreen = true; }
    }
};

bool    gbFullscreen;

//------------------------------------------------------------------------------------------------------------------------------------------
// Game config settings
//------------------------------------------------------------------------------------------------------------------------------------------
static const ConfigFieldHandler GAME_CFG_INI_HANDLERS[] = {
    {
        "UncapFramerate",
        "#---------------------------------------------------------------------------------------------------\n"
        "# Uncapped framerate toggle.\n"
        "# Setting to '1' allows PsyDoom to run beyond the original 30 FPS cap of PSX Doom.\n"
        "# Frames in between the original 30 FPS keyframes will have movements and rotations interpolated.\n"
        "#---------------------------------------------------------------------------------------------------\n"
        "UncapFramerate = 1\n",
        [](const IniUtils::Entry& iniEntry) { gbUncapFramerate = iniEntry.getBoolValue(true); },
        []() { gbUncapFramerate = true; }
    }
};

bool    gbUncapFramerate;

//------------------------------------------------------------------------------------------------------------------------------------------
// Other config parser related state
//------------------------------------------------------------------------------------------------------------------------------------------

// Set to true if a new config file or new config fields were generated
static bool gbDidGenerateNewConfig;

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse a given config file using the given config field handlers
//------------------------------------------------------------------------------------------------------------------------------------------
static void parseConfigFile(
    const std::string& configFolder,
    const char* fileName,
    const ConfigFieldHandler* configFieldHandlers,
    const size_t numConfigFieldHandlers
) noexcept {
    // Set all values to their initial defaults (until we parse otherwise)
    for (size_t i = 0; i < numConfigFieldHandlers; ++i) {
        configFieldHandlers[i].setValueToDefaultFunc();
    }

    // Store which config field handlers have parsed config here
    std::vector<bool> executedConfigHandler;
    executedConfigHandler.resize(numConfigFieldHandlers);

    // Read and parse the ini file (if it exists).
    // Allow the file to exist in two different locations: (1) The current working directory and (2) The normal config folder.
    // The configuration file in the current working directory takes precedence over the one in the config folder.
    std::string configFilePath;
    bool bCfgFileExists;
    
    if (FileUtils::fileExists(fileName)) {
        configFilePath = fileName;
        bCfgFileExists = true;
    } else {
        configFilePath = configFolder + fileName;
        bCfgFileExists = FileUtils::fileExists(configFilePath.c_str());
    }

    if (bCfgFileExists) {
        const FileData fileData = FileUtils::getContentsOfFile(configFilePath.c_str(), 8, std::byte(0));

        if (fileData.bytes) {
            IniUtils::parseIniFromString(
                (const char*) fileData.bytes.get(),
                fileData.size,
                [&](const IniUtils::Entry& iniEntry) noexcept {
                    // Try to find a matching config field handler.
                    // This is not an especially smart or fast way of doing it, but performance isn't an issue here:
                    for (size_t i = 0; i < numConfigFieldHandlers; ++i) {
                        const ConfigFieldHandler& handler = configFieldHandlers[i];

                        if (iniEntry.key != handler.name)
                            continue;
                        
                        handler.parseFunc(iniEntry);
                        executedConfigHandler[i] = true;
                        break;
                    }
                }
            );
        }
    }

    // If we are missing expected config fields then we need to reopen the config and append to it
    size_t numMissingConfigFields = 0;

    for (bool executed : executedConfigHandler) {
        if (!executed) {
            numMissingConfigFields++;
        }
    }

    // Do we need to append any new config fields?
    if (numMissingConfigFields > 0) {
        gbDidGenerateNewConfig = true;
        std::FILE* pFile = std::fopen(configFilePath.c_str(), FOPEN_WRITE_APPEND_TEXT);

        if (pFile) {
            // Only append a starting newline if it's an existing file...
            if (bCfgFileExists) {
                std::fwrite("\n", 1, 1, pFile);
            }

            for (size_t i = 0; i < numConfigFieldHandlers; ++i) {
                if (!executedConfigHandler[i]) {
                    const ConfigFieldHandler& handler = configFieldHandlers[i];
                    numMissingConfigFields--;

                    std::fwrite(handler.iniDefaultStr, std::strlen(handler.iniDefaultStr), 1, pFile);

                    // Only append a newline between the fields if we're not on the last one
                    if (numMissingConfigFields > 0) {
                        std::fwrite("\n", 1, 1, pFile);
                    }
                }
            }

            std::fclose(pFile);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse all config files
//------------------------------------------------------------------------------------------------------------------------------------------
static void parseAllConfigFiles(const std::string& configFolder) noexcept {
    parseConfigFile(configFolder, "graphics_cfg.ini",   GRAPHICS_CFG_INI_HANDLERS,  C_ARRAY_SIZE(GRAPHICS_CFG_INI_HANDLERS));
    parseConfigFile(configFolder, "game_cfg.ini",       GAME_CFG_INI_HANDLERS,      C_ARRAY_SIZE(GAME_CFG_INI_HANDLERS));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read all config for the app, and generate config files for the user if it's the first launch.
// If new config keys are missing then they will be appended to the existing config files.
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    const std::string configFolder = Utils::getOrCreateUserDataFolder();
    parseAllConfigFiles(configFolder);

    // If we generated new config inform the user so changes can be made if required
    if (gbDidGenerateNewConfig) {
        std::string cfgFileMessage =
            "Hey, just a heads up! PsyDoom has generated and defaulted some new configuration settings in one or more .ini files.\n"
            "If you would like to review or edit PsyDoom's settings, you can normally find the .ini files at the following location:\n\n";

        cfgFileMessage.append(configFolder, 0, configFolder.length() - 1);
        cfgFileMessage += "\n\n";
        cfgFileMessage += "Change these files before proceeding to customize game settings.\n";
        cfgFileMessage += "\n";
        cfgFileMessage += "Note: if you wish, you can copy the .ini files to the application's working directory and these .ini\n";
        cfgFileMessage += "files will be recognized and take precedence over the ones in the folder mentioned above.";

        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION,
            "Configuring PsyDoom: new settings available",
            cfgFileMessage.c_str(),
            nullptr
        );

        // Re-parse the config once more after showing the message, just in case the user made changes
        parseAllConfigFiles(configFolder);
    }
}

void shutdown() noexcept {
    // Nothing to do here yet...
}

END_NAMESPACE(Config)
