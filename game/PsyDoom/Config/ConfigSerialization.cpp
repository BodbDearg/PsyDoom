#include "ConfigSerialization.h"

#include "Asserts.h"
#include "Config.h"
#include "ConfigSerialization_Audio.h"
#include "ConfigSerialization_Cheats.h"
#include "ConfigSerialization_Controls.h"
#include "ConfigSerialization_Game.h"
#include "ConfigSerialization_Graphics.h"
#include "ConfigSerialization_Input.h"
#include "ConfigSerialization_Multiplayer.h"
#include "FileUtils.h"
#include "IniUtils.h"
#include "PsyDoom/Utils.h"

#include <cstring>
#include <vector>

BEGIN_NAMESPACE(ConfigSerialization)

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains the full path for a config file and whether it exists or not
//------------------------------------------------------------------------------------------------------------------------------------------
struct CfgFileInfo {
    std::string     path;
    bool            bFileExists;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------------------------------------------------------------

// MSVC: 't' file open option will cause '\n' to become '\r\n' (platform native EOL).
// This is useful because it allows us to use '\n' in the code and have it converted transparently.
#if _MSC_VER
    static constexpr const char* FOPEN_WRITE_TEXT = "wt";
#else
    static constexpr const char* FOPEN_WRITE_TEXT = "w";
#endif

// A modification warning placed at the top of every config file
static constexpr const char* const FILE_MODIFICATION_WARNING = 
R"(####################################################################################################
# WARNING: this file is periodically re-generated and overwritten by PsyDoom.
# If you have anything other than the value of a setting to save, do not put it in this file!
####################################################################################################
)";

// The names of all the config files
static constexpr const char* const CFG_FILE_AUDIO       = "audio_cfg.ini";
static constexpr const char* const CFG_FILE_CHEATS      = "cheats_cfg.ini";
static constexpr const char* const CFG_FILE_CONTROLS    = "control_bindings.ini";
static constexpr const char* const CFG_FILE_GAME        = "game_cfg.ini";
static constexpr const char* const CFG_FILE_GRAPHICS    = "graphics_cfg.ini";
static constexpr const char* const CFG_FILE_INPUT       = "input_cfg.ini";
static constexpr const char* const CFG_FILE_MULTIPLAYER = "multiplayer_cfg.ini";

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the config field defines a comment
//------------------------------------------------------------------------------------------------------------------------------------------
static bool hasComment(const ConfigField& field) noexcept {
    return (field.comment && field.comment[0]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Writes a comment to the specified file
//------------------------------------------------------------------------------------------------------------------------------------------
static void writeComment(const char* const comment, FILE* const pFile) noexcept {
    ASSERT(pFile && comment);
    std::fprintf(pFile, "#---------------------------------------------------------------------------------------------------\n");

    // Write the comment line by line
    for (const char* pLineStart = comment; *pLineStart; ) {
        // Find the end of the comment line
        const char* pLineEnd = std::strchr(pLineStart, '\n');
        const bool bFoundNewline = (pLineEnd != nullptr);

        if (!bFoundNewline) {
            pLineEnd = pLineStart + std::strlen(pLineStart);
        }

        // Write the comment line
        std::fprintf(pFile, "# ");
        std::fwrite(pLineStart, pLineEnd - pLineStart, 1, pFile);
        std::fprintf(pFile, "\n");

        // Move onto the next line and skip the newline character if there was one
        pLineStart = pLineEnd;
        
        if (bFoundNewline) {
            pLineStart++;
        }
    }

    std::fprintf(pFile, "#---------------------------------------------------------------------------------------------------\n");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: determines the file path to use for the specified config file and whether it exists
//------------------------------------------------------------------------------------------------------------------------------------------
static CfgFileInfo getCfgFileInfo(const std::string& configFolder, const char* fileName) noexcept {
    // Allow the file to exist in two different locations: (1) The current working directory and (2) The normal config folder.
    // The configuration file in the current working directory takes precedence over the one in the config folder.
    CfgFileInfo info = {};

    if (FileUtils::fileExists(fileName)) {
        info.path = fileName;
        info.bFileExists = true;
    } else {
        info.path = configFolder + fileName;
        info.bFileExists = FileUtils::fileExists(info.path.c_str());
    }

    return info;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: removes trailing zeros from a floating point number that has been converted to a string
//------------------------------------------------------------------------------------------------------------------------------------------
static void trimFloatStringTrailingZeroes(std::string& s) noexcept {
    if (s.empty())
        return;

    // Only trim if there is a decimal point, otherwise those zeros matter!
    if (std::strchr(s.c_str(), '.')) {
        while ((!s.empty()) && (s.back() == '0')) {
            s.pop_back();
        }

        // If the string now ends in a decimal point then it's useless and can be chopped
        if ((!s.empty()) && (s.back() == '.')) {
            s.pop_back();
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Config setter helpers
//------------------------------------------------------------------------------------------------------------------------------------------
SetConfigFieldFn makeConfigSetterFn(int32_t& globalCfgValue) noexcept {
    return [&globalCfgValue](const IniUtils::IniValue& value) THROWS {
        globalCfgValue = value.getAsInt();
    };
}

SetConfigFieldFn makeConfigSetterFn(float& globalCfgValue) noexcept {
    return [&globalCfgValue](const IniUtils::IniValue& value) THROWS {
        globalCfgValue = value.getAsFloat();
    };
}

SetConfigFieldFn makeConfigSetterFn(bool& globalCfgValue) noexcept {
    return [&globalCfgValue](const IniUtils::IniValue& value) THROWS {
        globalCfgValue = value.getAsBool();
    };
}

SetConfigFieldFn makeConfigSetterFn(std::string& globalCfgValue) noexcept {
    return [&globalCfgValue](const IniUtils::IniValue& value) noexcept {
        globalCfgValue = value.strValue;
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Config getter helpers
//------------------------------------------------------------------------------------------------------------------------------------------
GetConfigFieldFn makeConfigGetterFn(const int32_t& globalCfgValue) noexcept {
    return [&globalCfgValue](IniUtils::IniValue& valueOut) noexcept {
        valueOut.strValue = std::to_string(globalCfgValue);
    };
}

GetConfigFieldFn makeConfigGetterFn(const float& globalCfgValue) noexcept {
    return [&globalCfgValue](IniUtils::IniValue& valueOut) noexcept {
        valueOut.strValue = std::to_string(globalCfgValue);
        trimFloatStringTrailingZeroes(valueOut.strValue);
    };
}

GetConfigFieldFn makeConfigGetterFn(const bool& globalCfgValue) noexcept {
    return [&globalCfgValue](IniUtils::IniValue& valueOut) noexcept {
        valueOut.strValue = (globalCfgValue) ? "1" : "0";
    };
}

GetConfigFieldFn makeConfigGetterFn(const std::string& globalCfgValue) noexcept {
    return [&globalCfgValue](IniUtils::IniValue& valueOut) noexcept {
        valueOut.strValue = globalCfgValue;
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Config default initializer helpers
//------------------------------------------------------------------------------------------------------------------------------------------
DefInitConfigFieldFn makeConfigDefInitFn(int32_t& globalCfgValue, const int32_t defaultValue) noexcept {
    return [&globalCfgValue, defaultValue]() noexcept {
        globalCfgValue = defaultValue;
    };
}

DefInitConfigFieldFn makeConfigDefInitFn(float& globalCfgValue, const float defaultValue) noexcept {
    return [&globalCfgValue, defaultValue]() noexcept {
        globalCfgValue = defaultValue;
    };
}

DefInitConfigFieldFn makeConfigDefInitFn(bool& globalCfgValue, const bool defaultValue) noexcept {
    return [&globalCfgValue, defaultValue]() noexcept {
        globalCfgValue = defaultValue;
    };
}

DefInitConfigFieldFn makeConfigDefInitFn(std::string& globalCfgValue, const char* const defaultValue) noexcept {
    return [&globalCfgValue, defaultValue]() noexcept {
        globalCfgValue = defaultValue;
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes all config serializers
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    initCfgSerialization_Audio();
    initCfgSerialization_Cheats();
    initCfgSerialization_Controls();
    initCfgSerialization_Game();
    initCfgSerialization_Graphics();
    initCfgSerialization_Input();
    initCfgSerialization_Multiplayer();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleans up all config serializers
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gConfig_Audio = {};
    gConfig_Cheats = {};
    gConfig_Controls = {};
    gConfig_Game = {};
    gConfig_Graphics = {};
    gConfig_Input = {};
    gConfig_Multiplayer = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads all config files
//------------------------------------------------------------------------------------------------------------------------------------------
void readAllConfigFiles() noexcept {
    const std::string cfgDir = Utils::getOrCreateUserDataFolder();

    const bool bNeedSave_Audio          = readConfigFile(cfgDir, CFG_FILE_AUDIO,        gConfig_Audio.getFieldList());
    const bool bNeedSave_Cheats         = readConfigFile(cfgDir, CFG_FILE_CHEATS,       gConfig_Cheats.getFieldList());
    const bool bNeedSave_Controls       = readConfigFile(cfgDir, CFG_FILE_CONTROLS,     gConfig_Controls.getFieldList());
    const bool bNeedSave_Game           = readConfigFile(cfgDir, CFG_FILE_GAME,         gConfig_Game.getFieldList());
    const bool bNeedSave_Graphics       = readConfigFile(cfgDir, CFG_FILE_GRAPHICS,     gConfig_Graphics.getFieldList());
    const bool bNeedSave_Input          = readConfigFile(cfgDir, CFG_FILE_INPUT,        gConfig_Input.getFieldList());
    const bool bNeedSave_Multiplayer    = readConfigFile(cfgDir, CFG_FILE_MULTIPLAYER,  gConfig_Multiplayer.getFieldList());

    Config::gbNeedSave_Audio        |= bNeedSave_Audio;
    Config::gbNeedSave_Cheats       |= bNeedSave_Cheats;
    Config::gbNeedSave_Controls     |= bNeedSave_Controls;
    Config::gbNeedSave_Game         |= bNeedSave_Game;
    Config::gbNeedSave_Graphics     |= bNeedSave_Graphics;
    Config::gbNeedSave_Input        |= bNeedSave_Input;
    Config::gbNeedSave_Multiplayer  |= bNeedSave_Multiplayer;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Writes all config files to disk, optionally writing only changed files
//------------------------------------------------------------------------------------------------------------------------------------------
void writeAllConfigFiles(const bool bWriteUnchangedConfig) noexcept {
    const std::string cfgDir = Utils::getOrCreateUserDataFolder();

    const auto maybeWriteConfig = [&](
        const char* const cfgFileName,
        const ConfigFieldList& cfgFields,
        bool& bNeedSaveFlag,
        const char* const fileHeader
    ) noexcept {
        if (bNeedSaveFlag || bWriteUnchangedConfig) {
            const CfgFileInfo cfgFileInfo = getCfgFileInfo(cfgDir, cfgFileName);
            writeConfigFile(cfgFileInfo.path, cfgFields, fileHeader);
            bNeedSaveFlag = false;
        }
    };

    maybeWriteConfig(CFG_FILE_AUDIO,       gConfig_Audio.getFieldList(),       Config::gbNeedSave_Audio,       nullptr);
    maybeWriteConfig(CFG_FILE_CHEATS,      gConfig_Cheats.getFieldList(),      Config::gbNeedSave_Cheats,      nullptr);
    maybeWriteConfig(CFG_FILE_CONTROLS,    gConfig_Controls.getFieldList(),    Config::gbNeedSave_Controls,    CONTROL_BINDINGS_INI_HEADER);
    maybeWriteConfig(CFG_FILE_GAME,        gConfig_Game.getFieldList(),        Config::gbNeedSave_Game,        nullptr);
    maybeWriteConfig(CFG_FILE_GRAPHICS,    gConfig_Graphics.getFieldList(),    Config::gbNeedSave_Graphics,    nullptr);
    maybeWriteConfig(CFG_FILE_INPUT,       gConfig_Input.getFieldList(),       Config::gbNeedSave_Input,       nullptr);
    maybeWriteConfig(CFG_FILE_MULTIPLAYER, gConfig_Multiplayer.getFieldList(), Config::gbNeedSave_Multiplayer, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a given config file and tries to the map the data to the specified config fields.
// 
// Returns 'true' if the config file should be saved again (dirty flag) because the final runtime config differs to the data in the file.
// This happens when there are new fields added that were not in the config file, or which have failed to parse.
//------------------------------------------------------------------------------------------------------------------------------------------
bool readConfigFile(
    const std::string& configFolder,
    const char* fileName,
    const ConfigFieldList& cfgFields
) noexcept {
    ASSERT(cfgFields.pFieldList || (cfgFields.numFields == 0));

    // Store which config fields have been read successfully here
    std::vector<bool> readConfigField;
    readConfigField.resize(cfgFields.numFields);

    // Read and parse the ini file (if it exists)
    const CfgFileInfo cfgFileInfo = getCfgFileInfo(configFolder, fileName);
    const FileData cfgFileData = (cfgFileInfo.bFileExists) ? FileUtils::getContentsOfFile(cfgFileInfo.path.c_str(), 8, std::byte(0)) : FileData();

    if (cfgFileData.bytes) {
        IniUtils::parseIniFromString(
            (const char*) cfgFileData.bytes.get(),
            cfgFileData.size,
            [&](const IniUtils::IniEntry& iniEntry) noexcept {
                // Try to find a matching config field.
                // This is not an especially smart or fast way of doing it, but performance isn't an issue here:
                for (size_t i = 0; i < cfgFields.numFields; ++i) {
                    const ConfigField& field = cfgFields.pFieldList[i];
                    ASSERT(field.setFunc);

                    if (iniEntry.key != field.name)
                        continue;

                    try {
                        field.setFunc(iniEntry.value);
                        readConfigField[i] = true;
                    } catch (...) {
                        // Ignore, will default the field later...
                    }
                    
                    break;
                }
            }
        );
    }

    // Assign default values to any fields that were missing or which failed to parse
    size_t numDefaultedCfgFields = 0;
   
    for (size_t i = 0; i < cfgFields.numFields; ++i) {
        if (!readConfigField[i]) {
            // Did not read this config field successfully: set this field to it's default value
            const ConfigField& field = cfgFields.pFieldList[i];
            ASSERT(field.defInitFunc);
            field.defInitFunc();

            numDefaultedCfgFields++;
        }
    }

    // Do we need to re-save the config file?
    // This will be the case if any of the fields were defaulted because the INI didn't provide a value.
    return (numDefaultedCfgFields > 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Saves a list of config fields to the specified INI file.
// An optional header can be added to the top of the file; this can be used for documentation comments/instructions for the entire file.
//------------------------------------------------------------------------------------------------------------------------------------------
void writeConfigFile(
    const std::string& cfgFilePath,
    const ConfigFieldList& cfgFields,
    const char* const fileHeader
) noexcept {
    // Open the config file for writing
    std::FILE* const pFile = std::fopen(cfgFilePath.c_str(), FOPEN_WRITE_TEXT);

    if (!pFile)
        return;

    // Write the modification warning and file header if existing
    std::fprintf(pFile, "%s\n", FILE_MODIFICATION_WARNING);

    if (fileHeader && fileHeader[0]) {
        std::fprintf(pFile, "%s\n", fileHeader);
    }

    // Re-use this buffer for all serialization
    IniUtils::IniValue fieldValue;
    fieldValue.strValue.reserve(256);

    // Write all config fields out to the file
    for (size_t i = 0; i < cfgFields.numFields; ++i) {
        const ConfigField& field = cfgFields.pFieldList[i];
        ASSERT(field.getFunc);

        // Write the comment for the field if it has one
        if (hasComment(field)) {
            writeComment(field.comment, pFile);
        }

        // Serialize the field to a string and then write the key and value for the field to the file
        field.getFunc(fieldValue);

        if (fieldValue.strValue.empty()) {
            std::fprintf(pFile, "%s = \n", field.name);
        } else {
            std::fprintf(pFile, "%s = %s\n", field.name, fieldValue.strValue.c_str());
        }

        // If the next field has a comment then separate the fields with an additional newline
        if (i + 1 < cfgFields.numFields) {
            if (hasComment(cfgFields.pFieldList[i + 1])) {
                std::fprintf(pFile, "\n");
            }
        }
    }

    // Close to flush the stream and finish up
    std::fclose(pFile);
}

END_NAMESPACE(ConfigSerialization)
