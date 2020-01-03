#pragma once

#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

struct mobj_t;
struct mobjinfo_t;
struct player_t;
struct sector_t;
struct state_t;
struct state_t;
struct subsector_t;

enum mobjtype_t : uint32_t;

// Alias for fixed point numbers in DOOM: mostly in 16.16 format but does not have to be
typedef int32_t fixed_t;

static constexpr uint32_t   FRACBITS = 16;            // Number of bits in most fixed point numbers in DOOM
static constexpr fixed_t    FRACUNIT = 0x10000;       // 1.0 in 16.16 fixed point format

// Alias for a 32-bit BAM (Binary angular measurement) angle in DOOM and some commonly used angles.
// This type is used for most angles in the game, but bits are truncated to lookup sine, cosine etc. table values.
typedef uint32_t angle_t;

static constexpr angle_t ANG45  = 0x20000000;
static constexpr angle_t ANG90  = 0x40000000;
static constexpr angle_t ANG180 = 0x80000000;
static constexpr angle_t ANG270 = 0xc0000000;

// Some global defines and constants
static constexpr uint32_t MAXPLAYERS = 2;   // Maximum number of players in a multiplayer game

// What type of game is being played
enum gametype_t : uint32_t {
    gt_single,
    gt_coop,
    gt_deathmatch
};

// What skill level the game is running at
enum skill_t : uint32_t {
    sk_baby,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare
};

// Represents a high level result of running a game loop (MiniLoop).
// Determines where the game navigates to next.
enum gameaction_t : uint32_t {
    ga_nothing,     // No game action
    ga_died,        // Player has died
    ga_number2,     // TODO: NAME!
    ga_number3,     // TODO: NAME!
    ga_number4,     // TODO: NAME!
    ga_number5,     // TODO: NAME!
    ga_number6,     // TODO: NAME!
    ga_timeout,     // Player died or menu timed out
    ga_number8,     // TODO: NAME!
    ga_exitdemo,    // Player aborted the demo screens
};

// Coordinate indexes in a bounding box
enum : uint32_t {
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
};

// Maximum number of ticks in a demo.
// The maximum allowed demo size is 16384 ticks (demo size 64 KiB).
const int32_t MAX_DEMO_TICKS = 16384;

// A function which gets called after regular map object updating is done
typedef VmPtr<void (mobj_t* pMObj)> latecall_t;

// Holds state for an object in the game world
struct mobj_t {
    fixed_t                 x;                  // Global position in the world, in 16.16 format
    fixed_t                 y;
    fixed_t                 z;
    VmPtr<subsector_t>      subsector;
    VmPtr<mobj_t>           prev;               // Intrusive fields for the global linked list of things
    VmPtr<mobj_t>           next;
    latecall_t              latecall;
    VmPtr<mobj_t>           snext;              // Intrusive fields for the linked list of things in the current sector
    VmPtr<mobj_t>           sprev;
    angle_t                 angle;              // Direction the thing is facing in
    uint32_t                sprite;             // Current sprite displayed
    uint32_t                frame;              // Current sprite frame displayed. Must use 'FF_FRAMEMASK' to get the actual frame number.
    VmPtr<mobj_t>           bnext;              // Linked list of things in this blockmap block
    VmPtr<mobj_t>           bprev;          
    fixed_t                 floorz;             // Highest floor in contact with map object
    fixed_t                 ceilingz;           // Lowest floor in contact with map object
    fixed_t                 radius;             // For collision detection
    fixed_t                 height;             // For collision detection
    fixed_t                 momx;               // Current XYZ speed
    fixed_t                 momy;
    fixed_t                 momz;
    mobjtype_t              type;               // Type enum
    VmPtr<mobjinfo_t>       info;               // Type data
    int32_t                 tics;               // Tick counter for the current state
    VmPtr<state_t>          state;              // State data
    uint32_t                flags;              // See the MF_XXX series of flags for possible bits.
    int32_t                 health;
    int32_t                 movedir;            // Diagonal directions: 0-7
    int32_t                 movecount;          // When this reaches 0 a new dir is selected
    VmPtr<mobj_t>           target;             // The current map object being chased or attacked (if any)
    int32_t                 reactiontime;       // Time left until an attack is allowed
    int32_t                 threshold;          // Time left chasing the current target
    VmPtr<player_t>         player;             // Associated player, if any
    uint32_t                extradata;          // Used for latecall functions
    int16_t                 spawnx;             // Used for respawns: original spawn params
    int16_t                 spawny;
    uint16_t                spawntype;
    int16_t                 spawnangle;
    VmPtr<mobj_t>           tracer;             // Used by homing missiles
};

static_assert(sizeof(mobj_t) == 148);

// A degenerate map object with most of it's fields chopped out.
// Used to store a sound origin within sector structures.
struct degenmobj_t {            
    fixed_t                 x;
    fixed_t                 y;
    fixed_t                 z;
    VmPtr<subsector_t>      subsector;      // TODO: CONFIRM LAYOUT
};

static_assert(sizeof(degenmobj_t) == 16);

// Basic player status
enum playerstate_t : uint32_t {
    PST_LIVE,       // Player is playing the game           (TODO: CONFIRM!)
    PST_DEAD,       // Player is dead                       (TODO: CONFIRM!)
    PST_REBORN      // Player is spawning or respawning     (TODO: CONFIRM!)
};

// Player sprite type, just a weapon and a flash in DOOM
enum psprnum_t : uint32_t {
    ps_weapon,
    ps_flash,
    NUMPSPRITES
};

// State for a player sprite
struct pspdef_t {
    VmPtr<state_t>  state;      // Pointer to state structure
    int32_t         tics;       // How many tics are left in this state?
    fixed_t         sx;         // Offset of the sprite
    fixed_t         sy;
};

static_assert(sizeof(pspdef_t) == 16);

// Keycard types
enum card_t : uint32_t {
    // TODO: name these labels
    it_0,
    it_1,
    it_2,
    it_3,
    it_4,
    it_5,
    NUMCARDS
};

// Player weapon types
enum weapontype_t : uint32_t {
    // TODO: name these labels
    wp_0,
    wp_1,
    wp_2,
    wp_3,
    wp_4,
    wp_5,
    wp_6,
    wp_7,
    wp_8,
    NUMWEAPONS,
    wp_nochange     // Used to represent no weapon change (TODO: CONFIRM)
};

// Player ammo types
enum ammotype_t : uint32_t {
    // TODO: name these labels
    am_0,
    am_1,
    am_2,
    am_3,
    NUMAMMO,
    am_noammo       // The 'no' ammo type used for ammoless weapons (TODO: CONFIRM)
};

// Player powerup types
enum powertype_t : uint32_t {
    // TODO: name these labels
    pw_0,
    pw_1,
    pw_2,
    pw_3,
    pw_4,
    pw_5,
    NUMPOWERS
};

// Holds state specific to each player
struct player_t {
    VmPtr<mobj_t>       mo;                                 // The map object controlled by the player
    playerstate_t       playerstate;                        // Player status
    fixed_t             forwardmove;
    fixed_t             sidemove;
    angle_t             angleturn;
    fixed_t             viewz;
    fixed_t             viewheight;
    fixed_t             deltaviewheight;
    fixed_t             bob;
    uint32_t            health;
    uint32_t            armorpoints;
    uint32_t            armortype;        
    int32_t             powers[NUMPOWERS];                  // How many ticks left for each power
    bool32_t            cards[NUMCARDS];                    // Which keycards the player has
    bool32_t            backpack;
    uint32_t            frags;
    uint32_t            __padding;
    weapontype_t        readyweapon;
    weapontype_t        pendingweapon;
    bool32_t            weaponowned[NUMWEAPONS];
    uint32_t            ammo[NUMAMMO];
    uint32_t            maxammo[NUMAMMO];
    uint32_t            attackdown;
    uint32_t            usedown;
    uint32_t            cheats;
    uint32_t            refire;
    uint32_t            killcount;
    uint32_t            itemcount;
    uint32_t            secretcount;
    VmPtr<char>         message;
    uint32_t            damagecount;
    uint32_t            bonuscount;
    VmPtr<mobj_t>       attacker;
    uint32_t            extralight;
    uint32_t            fixedcolormap;
    uint32_t            colormap;
    pspdef_t            psprites[NUMPSPRITES];
    bool32_t            didsecret;
    VmPtr<sector_t>     lastsoundsector;
    int32_t             automapx;
    int32_t             automapy;
    uint32_t            automapscale;
    uint32_t            automapflags;
    uint32_t            turnheld;
};

static_assert(sizeof(player_t) == 300);
