#pragma once

#include "Doom/doomdef.h"

#include <vector>

enum sfxenum_t : int32_t;
struct line_t;

// Moving platform type
enum plattype_e : int32_t {
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS,

    // PsyDoom: adding a new 'custom' platform type
    #if PSYDOOM_MODS
        customPlat,
    #endif
};

// Current status for a moving platform
enum plat_e : int32_t {
    up,
    down,
    waiting,
    in_stasis
};

// Config and state for a moving platform
struct plat_t {
    thinker_t       thinker;        // Basic thinker properties
    sector_t*       sector;         // What sector is affected by the moving platform
    fixed_t         speed;          // Speed that the platform moves at
    fixed_t         low;            // Height of the lowest floor surrounding the platform's sector
    fixed_t         high;           // Height of the highest floor surrounding the platform's sector
    int32_t         wait;           // How long the moving platform waits before returning to it's original position
    int32_t         count;          // Tick counter: how long of a wait there is left for the platform before it returns to it's original position
    plat_e          status;         // Current platform state (up/down/wait/paused)
    plat_e          oldstatus;      // Platform state before it was paused or put into stasis
    bool            crush;          // If true then the moving platform damages things which don't fit

    #if PSYDOOM_MODS
        bool bDoFinishScript;       // PsyDoom: custom platforms: whether to do a 'finish' script action on fully stopping
    #endif

    int32_t         tag;            // The tag for the line which activated this platform
    plattype_e      type;           // What type of behavior the moving platform has

    // PsyDoom: new fields used by custom crushers
    #if PSYDOOM_MODS
        plat_e      finishState;                // Which state the platform should finish on, once it's complete
        sfxenum_t   startSound;                 // Sound to make when starting ('sfx_None' if none)
        sfxenum_t   moveSound;                  // Sound to make when moving ('sfx_None' if none)
        uint32_t    moveSoundFreq;              // How many tics between instances of the move sound playing
        sfxenum_t   stopSound;                  // Sound to make when stopping ('sfx_None' if none)
        int32_t     finishScriptActionNum;      // If enabled, a script action to execute when the platform has come to a complete stop/finished
        int32_t     finishScriptUserdata;       // Userdata to pass to the 'finish' script action
    #endif
};

// PsyDoom: removing limits on the number of moving floors
#if PSYDOOM_LIMIT_REMOVING
    extern std::vector<plat_t*> gpActivePlats;
#else
    static constexpr int32_t MAXPLATS = 30;     // Maximum number of platforms there can be active at once
    extern plat_t* gpActivePlats[MAXPLATS];
#endif

// PsyDoom: definition for a custom platform/elevator.
// The constructor tries to construct with reasonable default settings.
#if PSYDOOM_MODS
    struct CustomPlatDef {
        CustomPlatDef() noexcept;

        bool        bCrush;                     // Is the platform crushing?
        bool        bDoFinishScript;            // Call the finish script action when completed moving?
        int32_t     startState;                 // -1 = down, 0 = wait, 1 = up
        int32_t     finishState;                // -1 = down, 0 = wait, 1 = up (ends when this state finishes)
        fixed_t     minHeight;                  // Minimum platform floor height
        fixed_t     maxHeight;                  // Maximum platform floor height
        fixed_t     speed;                      // Speed that the platform moves at
        int32_t     waitTime;                   // How many game tics the platform waits for when in the 'waiting' state
        sfxenum_t   startSound;                 // Sound to make when starting ('sfx_None' if none)
        sfxenum_t   moveSound;                  // Sound to make when moving ('sfx_None' if none)
        uint32_t    moveSoundFreq;              // How many tics between instances of the move sound playing
        sfxenum_t   stopSound;                  // Sound to make when stopping ('sfx_None' if none)
        int32_t     finishScriptActionNum;      // If enabled, a script action to execute when the platform has come to a complete stop/finished
        int32_t     finishScriptUserdata;       // Userdata to pass to the 'finish' script action
    };
#endif

bool EV_DoPlat(line_t& line, const plattype_e platType, const int32_t moveAmount) noexcept;

#if PSYDOOM_MODS
    bool EV_DoCustomPlat(sector_t& sector, const CustomPlatDef platDef) noexcept;
    bool P_ActivateInStasisPlatForTag(const int32_t tag) noexcept;
    bool P_ActivateInStasisPlatForSector(const sector_t& sector) noexcept;
    bool EV_StopPlatForTag(const int32_t tag) noexcept;
    bool EV_StopPlatForSector(const sector_t& sector) noexcept;
#endif

void EV_StopPlat(line_t& line) noexcept;
