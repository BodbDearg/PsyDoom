//------------------------------------------------------------------------------------------------------------------------------------------
// Manages the Lua scripting engine that new PsyDoom levels can use to execute more advanced line actions with.
// The scripting capabilities are limited, and just extend to offering a little more control like 'Macros' in the original Doom 64 engine.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "ScriptingEngine.h"

#include "Doom/Base/w_wad.h"
#include "Doom/Game/p_setup.h"
#include "Doom/UI/st_main.h"
#include "MapHash.h"
#include "ScriptBindings.h"

#include <cstdio>
#include <memory>
#include <sol/sol.hpp>

// The main Lua VM, managed and wrapped by the 'Sol2' library
static std::unique_ptr<sol::state> gpLuaState;

// A table of actions registered with the scripting engine.
// Each action has an integer identifier associated with it that is referenced by line tags.
static std::unordered_map<int32_t, sol::function> gScriptActions;

BEGIN_NAMESPACE(ScriptingEngine)

// Context for the current script action being executed.
// Which linedef, sector and thing triggered the action, all of which are optional.
// All, some or none of these might be specified depending on the context in which the script action is executed.
line_t*     gpCurTriggeringLine;
sector_t*   gpCurTriggeringSector;
mobj_t*     gpCurTriggeringMobj;

// This flag can be set to 'false' by scripts to indicate action is not currently available/allowed.
// It affects the behavior of switches and whether they can change texture and make a noise or not.
bool gbCurActionAllowed;

//------------------------------------------------------------------------------------------------------------------------------------------
// Issues a user facing error message relating to scripting, which is shown on the in-game status bar.
// These messages must be short since there is not much space on the HUD, more detailed messages must be logged to standard out.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Args>
static void showStatusBarError(const char* const formatStr, Args&&... args) noexcept {
    // Use the level startup warning buffer for this purpose (it's always around)
    std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), formatStr, args...);
    gStatusBar.message = gLevelStartupWarning;
    gStatusBar.messageTicsLeft = 60;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the the script for the current map (if any) and returns the string containing the entire script.
// Note: the script data contributes towards the hash for the map also, so a slight change in the script might invalidate the hash.
//------------------------------------------------------------------------------------------------------------------------------------------
static std::unique_ptr<char[]> readCurrentMapScript() noexcept {
    // See if there is a scripts lump available for the current map; if there is none then don't bother initializing the scripting engine
    const int32_t scriptsLumpIdx = W_MapCheckNumForName("SCRIPTS");

    if (scriptsLumpIdx < 0)
        return {};

    // Read the map's script into a null terminated string
    const int32_t scriptsLumpLen = W_MapLumpLength(scriptsLumpIdx);
    std::unique_ptr<char[]> mapScript(new char[scriptsLumpLen + 1]);
    W_ReadMapLump(scriptsLumpIdx, mapScript.get(), true);
    mapScript[scriptsLumpLen] = 0;

    // The script counts towards the map hash: count it before returning
    MapHash::addData(mapScript.get(), scriptsLumpLen);
    return mapScript;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: makes a lua table readonly as much as possible
//------------------------------------------------------------------------------------------------------------------------------------------
template <bool TopLevel>
static void makeTableReadOnly(sol::table_core<TopLevel> table) noexcept {
    // Create a new meta-table for the object to deny new members being added.
    // This doesn't stop existing members from being modified however; sadly this is the best form of 'read-only' we can do with Lua currently.
    sol::table metatable = gpLuaState->create_table_with();

    metatable[sol::meta_function::index] = metatable;
    metatable[sol::meta_function::new_index] = [](lua_State* const L) -> int {
        return luaL_error(L, "Value is read only!");
    };

    table[sol::metatable_key] = metatable;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the initial Lua environment for registering script actions
//------------------------------------------------------------------------------------------------------------------------------------------
static void setupActionRegisterLuaEnv() noexcept {
    // Initialize the Lua VM and load required libraries
    gpLuaState.reset(new sol::state());
    sol::state& lua = *gpLuaState;
    lua.open_libraries(sol::lib::base, sol::lib::math);

    // Remove unwanted functions from the 'base' library.
    // Available functions after this pruning are (as of Lua 5.4):
    //  
    //  ipairs      select
    //  tonumber    tostring
    //  print       error
    //  type
    //
    constexpr const char* const UNWANTED_BASE_FUNCS[] = {
        "assert", "warn",
        "load", "dofile", "loadfile",
        "rawequal", "rawget", "rawset", "rawlen",
        "setmetatable", "getmetatable",
        "pairs", "next",
        "pcall", "xpcall",
        "collectgarbage",
        "_VERSION",
    };

    for (const char* unwantedFuncName : UNWANTED_BASE_FUNCS) {
        lua[unwantedFuncName] = nullptr;
    }

    // Make the 'math' library table read-only to prevent globals being written to this table
    if (auto tbl = lua.get<std::optional<sol::table>>("math"); tbl) {
        makeTableReadOnly(tbl.value());
    }

    // Provide a function to register script actions with
    lua["SetAction"] = [](const int32_t actionNumber, const sol::function func) noexcept {
        gScriptActions[actionNumber] = func;
    };

    // Register all scripting bindings
    ScriptBindings::registerAll(lua);

    // Make the global table read-only.
    // In order to ensure correct serialization, scripts should NOT set any globals!
    makeTableReadOnly(lua.globals());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the Lua environment for executing script actions, after all actions have been registered
//------------------------------------------------------------------------------------------------------------------------------------------
static void setupActionExecuteLuaEnv() noexcept {
    // Unregister the 'SetAction' script function
    sol::state& lua = *gpLuaState;
    lua["SetAction"] = nullptr;

    // Do garbage collection at this point to clean up, scripts should only be using locals variables after this
    lua.collect_gc();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the scripting engine for the current level.
// Loads and compiles the script lump for the level, if any is present.
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Read the current map script (if any)
    std::unique_ptr<char[]> mapScript = readCurrentMapScript();

    if (!mapScript)
        return;

    // If a script exists then setup a Lua environment for registering script actions and then execute the map script to register them
    setupActionRegisterLuaEnv();

    try {
        gpLuaState->script(mapScript.get());
    }
    catch (const std::exception& e) {
        std::printf("PsyDoom: error executing the map script! Details follow:\n%s\n", e.what());
        showStatusBarError("Script error! See stdout.");
    }

    // Setup the environment for executing script actions
    setupActionExecuteLuaEnv();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down the scripting engine for the current level
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gbCurActionAllowed = false;
    gScriptActions.clear();
    gpLuaState.reset();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the specified script action number.
// The line and map object which triggered the script action (both optional) are passed in as additional context for scripts.
//------------------------------------------------------------------------------------------------------------------------------------------
void doAction(
    const int32_t actionNum,
    line_t* const pTrigLine,
    sector_t* const pTrigSector,
    mobj_t* const pTrigMobj
) noexcept {
    // Set context for scripts
    gpCurTriggeringLine = pTrigLine;
    gpCurTriggeringSector = pTrigSector;
    gpCurTriggeringMobj = pTrigMobj;

    // Assume the current action is allowed until scripts indicate otherwise
    gbCurActionAllowed = true;

    // Try and execute the action
    const auto actionIter = gScriptActions.find(actionNum);

    if (actionIter != gScriptActions.end()) {
        try {
            actionIter->second();
        }
        catch (const std::exception& e) {
            showStatusBarError("Script #%d error! See stdout.", actionNum);
            std::printf("PsyDoom: error executing map script action #%d! Details follow:\n%s\n", actionNum, e.what());
        }
    } else {
        showStatusBarError("No script action #%d!", actionNum);
        std::printf("PsyDoom: no scripting action #%d is available to execute!\n", actionNum);
    }

    // Clear script context
    gpCurTriggeringLine = nullptr;
    gpCurTriggeringSector = nullptr;
    gpCurTriggeringMobj = nullptr;
}

END_NAMESPACE(ScriptingEngine)
