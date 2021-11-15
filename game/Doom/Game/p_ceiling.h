#pragma once

#include "Doom/doomdef.h"

#include <vector>

enum sfxenum_t : int32_t;
struct line_t;

// Type for a ceiling
enum ceiling_e : int32_t {
    lowerToFloor            = 0,
    raiseToHighest          = 1,
    lowerAndCrush           = 2,
    crushAndRaise           = 3,
    fastCrushAndRaise       = 4,
    silentCrushAndRaise     = 5,

    // PsyDoom: adding a new 'custom' ceiling type
    #if PSYDOOM_MODS
        customCeiling = 6,
    #endif
};

// Config and state for a ceiling mover
struct ceiling_t {
    thinker_t   thinker;            // Basic thinker properties
    ceiling_e   type;               // What type of behavior the ceiling mover has
    sector_t*   sector;             // Sector affected
    fixed_t     bottomheight;       // Lowest destination height
    fixed_t     topheight;          // Highest destination height
    fixed_t     speed;              // Speed of movement up or down
    bool        crush;              // Does the ceiling damage things when they don't fit?

    #if PSYDOOM_MODS
        bool    bIsCrushing;        // PsyDoom: a flag set to 'true' if the ceiling was crushing on the last frame
        bool    bDoFinishScript;    // PsyDoom: custom crushers: whether to do a 'finish' script action on fully stopping
    #endif

    int32_t     direction;          // 1 = up, 0 = waiting, -1 = down
    int32_t     tag;                // Sector tag for the ceiling mover's sector
    int32_t     olddirection;       // In-stasis ceilings: which way the ceiling was moving before it was paused

    // PsyDoom: new fields used by custom crushers
    #if PSYDOOM_MODS
        fixed_t     crushSpeed;                 // Speed when crushing something (usually 1/8 of normal speed)
        int32_t     dirChangesLeft;             // How many times the crusher can change direction before stopping (if '-1' then no limit)
        sfxenum_t   moveSound;                  // Sound to make when moving ('sfx_None' if none)
        uint32_t    moveSoundFreq;              // How many tics between instances of the move sound playing
        sfxenum_t   changeDirSound;             // Sound to make when changing direction ('sfx_None' if none)
        sfxenum_t   stopSound;                  // Sound to make when stopping ('sfx_None' if none)
        int32_t     finishScriptActionNum;      // If enabled, a script action to execute when the ceiling has come to a complete stop/finished
        int32_t     finishScriptUserdata;       // Userdata to pass to the 'finish' script action
    #endif
};

#if PSYDOOM_LIMIT_REMOVING
    extern std::vector<ceiling_t*> gpActiveCeilings;
#else
    static constexpr int32_t MAXCEILINGS = 30;          // Maximum number of ceiling movers there can be active at once
    extern ceiling_t* gpActiveCeilings[MAXCEILINGS];
#endif

// PsyDoom: definition for a custom ceiling/crusher.
// The constructor tries to construct with reasonable default settings.
#if PSYDOOM_MODS
    struct CustomCeilingDef {
        CustomCeilingDef() noexcept;

        bool        bCrush;                     // Is the ceiling crushing?
        bool        bDoFinishScript;            // Call the finish script action when completed moving?
        fixed_t     minHeight;                  // Minimum ceiling height the crusher reaches
        fixed_t     maxHeight;                  // Maximum ceiling height the crusher reaches
        int32_t     startDir;                   // 1 = up, 0 = paused, -1 = down
        fixed_t     normalSpeed;                // Speed normally when moving
        fixed_t     crushSpeed;                 // Speed when crushing something (usually 1/8 of normal speed)
        int32_t     numDirChanges;              // How many times the crusher can change direction before stopping (if '-1' then no limit)
        sfxenum_t   startSound;                 // Sound to make when starting ('sfx_None' if none)
        sfxenum_t   moveSound;                  // Sound to make when moving ('sfx_None' if none)
        uint32_t    moveSoundFreq;              // How many tics between instances of the move sound playing
        sfxenum_t   changeDirSound;             // Sound to make when changing direction ('sfx_None' if none)
        sfxenum_t   stopSound;                  // Sound to make when stopping ('sfx_None' if none)
        int32_t     finishScriptActionNum;      // If enabled, a script action to execute when the ceiling has come to a complete stop/finished
        int32_t     finishScriptUserdata;       // Userdata to pass to the 'finish' script action
    };
#endif

void T_MoveCeiling(ceiling_t& ceiling) noexcept;
bool EV_DoCeiling(line_t& line, const ceiling_e ceilingType) noexcept;

#if PSYDOOM_MODS
    bool EV_DoCustomCeiling(sector_t& sector, const CustomCeilingDef& ceilDef) noexcept;
#endif

void P_AddActiveCeiling(ceiling_t& ceiling) noexcept;
void P_RemoveActiveCeiling(ceiling_t& ceiling) noexcept;

#if PSYDOOM_MODS
    bool P_ActivateInStasisCeilingsForTag(const int32_t tag) noexcept;
    bool P_ActivateInStasisCeilingForSector(const sector_t& sector) noexcept;
    bool EV_CeilingCrushStopForTag(const int32_t tag) noexcept;
    bool EV_CeilingCrushStopForSector(const sector_t& sector) noexcept;
#endif

bool EV_CeilingCrushStop(line_t& line) noexcept;
