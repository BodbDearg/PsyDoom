#pragma once

#include "Macros.h"

#include <functional>

namespace IniUtils {
    struct IniValue;
}

BEGIN_NAMESPACE(ConfigSerialization)

//------------------------------------------------------------------------------------------------------------------------------------------
// Functions that provide access to global config fields
//------------------------------------------------------------------------------------------------------------------------------------------

// A function that sets the value of a global config field using the specified INI value.
// Note: this is allowed to throw an exception on failure to convert the INI value to the appropriate format.
// If an exception is caught then the field will be initialized to the default value.
typedef std::function<void (const IniUtils::IniValue& value)> SetConfigFieldFn;

// A function that gets the value of a global config field and stores it as a string in the specified INI value
typedef std::function<void (IniUtils::IniValue& valueOut)> GetConfigFieldFn;

// A function that default initializes the value of a global config field
typedef std::function<void ()> DefInitConfigFieldFn;

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a global config field that can be loaded or saved or set to the default value
//------------------------------------------------------------------------------------------------------------------------------------------
struct ConfigField {
    const char*             name;           // The string name/key for the config field
    const char*             comment;        // Documentation comment for the config field: can be set to 'nullptr' or empty if commented elsewhere
    SetConfigFieldFn        setFunc;        // Set the value of the global config field
    GetConfigFieldFn        getFunc;        // Get the value of the global config field
    DefInitConfigFieldFn    defInitFunc;    // Default initializes the global config field
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a list of config fields.
// The fields are stored one after the other in memory.
//------------------------------------------------------------------------------------------------------------------------------------------
struct ConfigFieldList {
    ConfigField*    pFieldList;
    size_t          numFields;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper that makes a config field struct
//------------------------------------------------------------------------------------------------------------------------------------------
static inline ConfigField makeConfigField(
    const char* name,
    const char* comment,
    SetConfigFieldFn setFunc,
    GetConfigFieldFn getFunc,
    DefInitConfigFieldFn defInitFunc
) noexcept {
    return ConfigField{ name, comment, setFunc, getFunc, defInitFunc };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helpers for making config field setters, getters and default initializers
//------------------------------------------------------------------------------------------------------------------------------------------
SetConfigFieldFn makeConfigSetterFn(int32_t& globalCfgValue) noexcept;
SetConfigFieldFn makeConfigSetterFn(float& globalCfgValue) noexcept;
SetConfigFieldFn makeConfigSetterFn(bool& globalCfgValue) noexcept;
SetConfigFieldFn makeConfigSetterFn(std::string& globalCfgValue) noexcept;

GetConfigFieldFn makeConfigGetterFn(const int32_t& globalCfgValue) noexcept;
GetConfigFieldFn makeConfigGetterFn(const float& globalCfgValue) noexcept;
GetConfigFieldFn makeConfigGetterFn(const bool& globalCfgValue) noexcept;
GetConfigFieldFn makeConfigGetterFn(const std::string& globalCfgValue) noexcept;

DefInitConfigFieldFn makeConfigDefInitFn(int32_t& globalCfgValue, const int32_t defaultValue) noexcept;
DefInitConfigFieldFn makeConfigDefInitFn(float& globalCfgValue, const float defaultValue) noexcept;
DefInitConfigFieldFn makeConfigDefInitFn(bool& globalCfgValue, const bool defaultValue) noexcept;
DefInitConfigFieldFn makeConfigDefInitFn(std::string& globalCfgValue, const char* const defaultValue) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Shortened version of the helper to make a config field struct: uses standard setter, getter and default initializer functions
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ValT, class DefT>
static inline ConfigField makeConfigField(
    const char* name,
    const char* comment,
    ValT& globalCfgValue,
    const DefT defaultValue
) noexcept {
    return ConfigField{
        name,
        comment,
        makeConfigSetterFn(globalCfgValue),
        makeConfigGetterFn(globalCfgValue),
        makeConfigDefInitFn(globalCfgValue, defaultValue)
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// General module functionality
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept;
void shutdown() noexcept;

void readAllConfigFiles() noexcept;
void writeAllConfigFiles(const bool bWriteUnchangedConfig) noexcept;

bool readConfigFile(
    const std::string& configFolder,
    const char* fileName,
    const ConfigFieldList& cfgFields
) noexcept;

void writeConfigFile(
    const std::string& cfgFilePath,
    const ConfigFieldList& cfgFields,
    const char* const fileHeader
) noexcept;

END_NAMESPACE(ConfigSerialization)
