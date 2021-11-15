//------------------------------------------------------------------------------------------------------------------------------------------
// Manages the Lua scripting engine that new PsyDoom levels can use to execute more advanced line actions with.
// The scripting capabilities are limited, and just extend to offering a little more control like 'Macros' in the original Doom 64 engine.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "ScriptingEngine.h"

#include "Doom/Base/w_wad.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/UI/st_main.h"
#include "MapHash.h"
#include "ScriptBindings.h"

#include <cstdio>
#include <memory>
#include <sol/sol.hpp>

BEGIN_NAMESPACE(ScriptingEngine)

// The main Lua VM, managed and wrapped by the 'Sol2' library
static std::unique_ptr<sol::state> gpLuaState;

// A table of actions registered with the scripting engine.
// Each action has an integer identifier associated with it that is referenced by line tags.
static std::unordered_map<int32_t, sol::protected_function> gScriptActions;

// How many 'doAction' calls are currently active?
// Scripts might invoke other scripts, so the 'doAction' calls can be nested.
static int32_t gNumExecutingScripts = 0;

// The list of actions scheduled to execute later. This list may contain 'holes' or actions that are done executing.
// This is so we preserve the relative order of actions scheduled to occur on the same tic - they should happen in the same order they were scheduled in.
std::vector<ScheduledAction> gScheduledActions;

// Context for the current script action being executed.
// Which linedef, sector and thing triggered the action, all of which are optional.
// All, some or none of these might be specified depending on the context in which the script action is executed.
line_t*     gpCurTriggeringLine;
sector_t*   gpCurTriggeringSector;
mobj_t*     gpCurTriggeringMobj;

// Extra context for the current script action being executed, which is only non-zero for scheduled action.
// The user/script defined tag for the action and userdata for the action.
int32_t     gCurActionTag;
int32_t     gCurActionUserdata;

// This flag can be set to 'false' by scripts to indicate action is not currently available/allowed.
// It affects the behavior of switches and whether they can change texture and make a noise or not.
bool gbCurActionAllowed;

// A flag set to true if we need to delete things after the current script has finished executing.
// This will be triggered if scripts do a call to 'P_RemoveMobj()'.
bool gbNeedMobjGC;

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

    // Make the 'math' library table read-only to prevent globals being written to this table.
    // Also remove unwanted functions. Available functions after this pruning are (as of Lua 5.4):
    //
    // math.abs     math.tointeger
    // math.ceil    math.floor
    // math.min     math.max
    // math.fmod
    //
    if (auto optTbl = lua.get<std::optional<sol::table>>("math"); optTbl) {
        constexpr const char* const UNWANTED_MATH_FUNCS[] = {
            "sqrt", "sin", "mininteger", "maxinteger",
            "huge", "exp", "pi", "acos",
            "randomseed", "type", "random", "cos",
            "tan", "ult", "atan", "asin",
            "log", "rad", "modf", "deg",
        };

        sol::table& tbl = optTbl.value();

        for (const char* unwantedFuncName : UNWANTED_MATH_FUNCS) {
            tbl[unwantedFuncName] = nullptr;
        }

        makeTableReadOnly(tbl);
    }

    // Provide a function to register script actions with
    lua["SetAction"] = [](const int32_t actionNumber, const sol::protected_function func) noexcept {
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
// Deletes/frees any map objects that have been marked for deletion.
// This action will be triggered by scripts calling 'P_RemoveMobj'.
//------------------------------------------------------------------------------------------------------------------------------------------
static void doMobjGC() noexcept {
    if (!gbNeedMobjGC) 
        return;

    gbNeedMobjGC = false;

    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead;) {
        mobj_t* const pNextMobj = pMobj->next;
        const latecall_t lateCall = pMobj->latecall;

        // N.B: only run the 'P_RemoveMobj' late call - leave other late calls alone
        if (lateCall && (lateCall == P_RemoveMobj)) {
            lateCall(*pMobj);
        }

        pMobj = pNextMobj;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates and zero initializes a 'ScheduledAction' structure.
// Tries to find a free one in the list first, otherwise adds a new one to the end of the list.
//------------------------------------------------------------------------------------------------------------------------------------------
static ScheduledAction& allocScheduledAction() noexcept {
    for (ScheduledAction& action : gScheduledActions) {
        // If there are no more executions left then it is free
        if (action.executionsLeft == 0) {
            action = {};
            return action;
        }
    }

    return gScheduledActions.emplace_back();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to reduce the size of the scheduled actions list by popping elements off the end.
// Leaves alone elements in the middle, in order to preserve relative action order.
//------------------------------------------------------------------------------------------------------------------------------------------
static void compactScheduledActionList() noexcept {
    while (!gScheduledActions.empty()) {
        const ScheduledAction& last = gScheduledActions.back();

        if (last.executionsLeft == 0) {
            gScheduledActions.pop_back();
        } else {
            break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the scripting engine for the current level.
// Loads and compiles the script lump for the level, if any is present.
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    ASSERT(gNumExecutingScripts == 0);

    // Read the current map script (if any)
    std::unique_ptr<char[]> mapScript = readCurrentMapScript();

    if (!mapScript)
        return;

    // Prealloc some memory
    gScheduledActions.reserve(64);

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
    ASSERT_LOG(gNumExecutingScripts == 0, "Shutdown should only be done when no scripts are executing!");

    gScheduledActions.clear();
    gScriptActions.clear();
    gpLuaState.reset();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs all actions that are scheduled for execution
//------------------------------------------------------------------------------------------------------------------------------------------
void runScheduledActions() noexcept {
    // Flag which actions are pending execution for this frame.
    // If any actions schedule any other actions then they will be delayed by 1 frame at least.
    const int32_t numActions = (int32_t) gScheduledActions.size();

    for (int32_t i = 0; i < numActions; ++i) {
        // Skip the action if paused or if there are no more executions left (stopped)
        ScheduledAction& action = gScheduledActions[i];

        if (action.bPaused || (action.executionsLeft == 0))
            continue;

        // Reduce the time until the action executes or schedule it to execute if executing now
        const int32_t delayTics = action.delayTics;

        if (delayTics > 0) {
            action.delayTics = delayTics - 1;
        } else {
            action.bPendingExecute = true;
        }
    }

    // Execute all pending actions and ignore any new ones that have been added
    for (int32_t i = 0; i < numActions; ++i) {
        // Only execute the action if it's pending execution
        ScheduledAction& action = gScheduledActions[i];

        if (!action.bPendingExecute)
            continue;

        // Otherwise setup the next repeat (if any) and execute the action
        const int32_t executionsLeft = action.executionsLeft;

        if (executionsLeft > 0) {
            action.executionsLeft = executionsLeft - 1;     // Finite number of repeats
        }

        action.delayTics = action.repeatDelay;
        action.bPendingExecute = false;
        doAction(action.actionNum, nullptr, nullptr, nullptr, action.tag, action.userdata);
    }

    // Compact the actions list as much as possible if there's stuff on the end that can be removed
    compactScheduledActionList();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the specified script action number and returns whether the script specifies the action was allowed (used by switches).
// The line and map object which triggered the script action (both optional) are passed in as additional context for scripts.
//------------------------------------------------------------------------------------------------------------------------------------------
bool doAction(
    const int32_t actionNum,
    line_t* const pTrigLine,
    sector_t* const pTrigSector,
    mobj_t* const pTrigMobj,
    const int32_t actionTag,
    const int32_t actionUserdata
) noexcept {
    // Set context for scripts
    gNumExecutingScripts++;

    gpCurTriggeringLine = pTrigLine;
    gpCurTriggeringSector = pTrigSector;
    gpCurTriggeringMobj = pTrigMobj;
    gCurActionTag = actionTag;
    gCurActionUserdata = actionUserdata;

    // Assume the current action is allowed until scripts indicate otherwise
    gbCurActionAllowed = true;

    // Try and execute the action
    const auto actionIter = gScriptActions.find(actionNum);

    if (actionIter != gScriptActions.end()) {
        try {
            const sol::protected_function_result result = actionIter->second();

            if (!result.valid()) {
                const sol::error error = result;
                showStatusBarError("Script #%d error! See stdout.", actionNum);
                std::printf("PsyDoom: error executing map script action #%d! Details follow:\n%s\n", actionNum, error.what());
            }
        }
        catch (const std::exception& e) {
            showStatusBarError("Script #%d error! See stdout.", actionNum);
            std::printf("PsyDoom: error executing map script action #%d! Details follow:\n%s\n", actionNum, e.what());
        }
    } else {
        showStatusBarError("No script action #%d!", actionNum);
        std::printf("PsyDoom: no scripting action #%d is available to execute!\n", actionNum);
    }

    const bool bWasActionAllowed = gbCurActionAllowed;

    // Clear script context
    gpCurTriggeringLine = nullptr;
    gpCurTriggeringSector = nullptr;
    gpCurTriggeringMobj = nullptr;
    gCurActionTag = 0;
    gCurActionUserdata = 0;
    gbCurActionAllowed = false;

    gNumExecutingScripts--;

    // Delete any things that are pending for delete if this is the last script in the stack
    if (gNumExecutingScripts == 0) {
        doMobjGC();
    }

    return bWasActionAllowed;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedule a single delayed action to execute.
// The action is guaranteed to execute at least 1 frame later.
//------------------------------------------------------------------------------------------------------------------------------------------
void scheduleAction(
    const int32_t actionNum,
    const int32_t delayTics,
    const int32_t tag,
    const int32_t userdata
) noexcept {
    ScheduledAction& action = allocScheduledAction();
    action.actionNum = actionNum;
    action.delayTics = std::max(delayTics, 0);
    action.executionsLeft = 1;
    action.tag = tag;
    action.userdata = userdata;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedule a potentially repeating action to execute.
// The action is guaranteed to execute at least 1 frame later if the number of repeats is valid (not zero).
//------------------------------------------------------------------------------------------------------------------------------------------
void scheduleRepeatingAction(
    const int32_t actionNum,
    const int32_t initialDelayTics,
    const int32_t numRepeats,
    const int32_t repeatDelay,
    const int32_t tag,
    const int32_t userdata
) noexcept {
    ScheduledAction& action = allocScheduledAction();
    action.actionNum = actionNum;
    action.delayTics = std::max(initialDelayTics, 0);
    action.executionsLeft = (numRepeats < 0) ? -1 : numRepeats + 1;    // Note: use '-1' always for infinite
    action.repeatDelay = std::max(repeatDelay, 0);
    action.tag = tag;
    action.userdata = userdata;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cancels/stops all scheduled actions
//------------------------------------------------------------------------------------------------------------------------------------------
void stopAllScheduledActions() noexcept {
    // Note: this can be called while executing and iterating through the scheduled action list.
    // Therefore just flag the action as stopped rather than trying to cleanup or compact the list.
    for (ScheduledAction& action : gScheduledActions) {
        action.executionsLeft = 0;
        action.bPendingExecute = false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cancels/stops all scheduled actions with the specified tag
//------------------------------------------------------------------------------------------------------------------------------------------
void stopScheduledActionsWithTag(const int32_t tag) noexcept {
    // Note: this can be called while executing and iterating through the scheduled action list.
    // Therefore just flag the action as stopped rather than trying to cleanup or compact the list.
    for (ScheduledAction& action : gScheduledActions) {
        if (action.tag == tag) {
            action.executionsLeft = 0;
            action.bPendingExecute = false;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pauses or unpaused all scheduled actions
//------------------------------------------------------------------------------------------------------------------------------------------
void pauseAllScheduledActions(const bool bPause) noexcept {
    for (ScheduledAction& action : gScheduledActions) {
        action.bPaused = bPause;
        action.bPendingExecute = false;     // Action must wait another frame if it is unpaused
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pauses or unpaused all scheduled actions with the specified tag
//------------------------------------------------------------------------------------------------------------------------------------------
void pauseScheduledActionsWithTag(const int32_t tag, const bool bPause) noexcept {
    for (ScheduledAction& action : gScheduledActions) {
        if (action.tag == tag) {
            action.bPaused = bPause;
            action.bPendingExecute = false;     // Action must wait another frame if it is unpaused
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the number of actions that are pending execution with the specified tag.
// This count includes any actions that have been paused, but not actions that are stopped/finished.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getNumScheduledActionsWithTag(const int32_t tag) noexcept {
    int32_t actionCount = 0;

    for (ScheduledAction& action : gScheduledActions) {
        if ((action.tag == tag) && (action.executionsLeft != 0)) {
            actionCount++;
        }
    }

    return actionCount;
}

END_NAMESPACE(ScriptingEngine)
