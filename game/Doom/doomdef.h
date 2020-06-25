#pragma once

#include <cstdint>

struct mobj_t;
struct mobjinfo_t;
struct player_t;
struct sector_t;
struct state_t;
struct state_t;
struct subsector_t;

enum dirtype_t : int32_t;
enum mobjtype_t : int32_t;
enum statenum_t : int32_t;

// Screen resolution (NTSC)
static constexpr int32_t SCREEN_W = 256;
static constexpr int32_t SCREEN_H = 240;
static constexpr int32_t HALF_SCREEN_W = SCREEN_W / 2;
static constexpr int32_t HALF_SCREEN_H = SCREEN_H / 2;
static constexpr int32_t VIEW_3D_H = 200;
static constexpr int32_t HALF_VIEW_3D_H = VIEW_3D_H / 2;

//------------------------------------------------------------------------------------------------------------------------------------------
// Alias for fixed point numbers in DOOM: mostly in 16.16 format but does not have to be
//------------------------------------------------------------------------------------------------------------------------------------------
typedef int32_t fixed_t;

static constexpr uint32_t   FRACBITS = 16;              // Number of bits in most fixed point numbers in DOOM
static constexpr fixed_t    FRACUNIT = 0x10000;         // 1.0 in 16.16 fixed point format
static constexpr fixed_t    FRACMASK = FRACUNIT - 1;    // Mask for the fractional part of a 16.16 fixed point number

//------------------------------------------------------------------------------------------------------------------------------------------
// Alias for a 32-bit BAM (Binary angular measurement) angle in DOOM and some commonly used angles.
// This type is used for most angles in the game, but bits are truncated to lookup sine, cosine etc. table values.
//------------------------------------------------------------------------------------------------------------------------------------------
typedef uint32_t angle_t;

static constexpr angle_t ANG45  = 0x20000000;
static constexpr angle_t ANG90  = 0x40000000;
static constexpr angle_t ANG180 = 0x80000000;
static constexpr angle_t ANG270 = 0xC0000000;
static constexpr angle_t ANG5   = ANG45 / 9;

// Constants relating to trig lookup tables
static constexpr uint32_t FINEANGLES        = 8192;                 // How many entries in sine, cosine etc. LUTs. The number of 'fine' angles.
static constexpr uint32_t FINEMASK          = FINEANGLES - 1;       // Wrap a 'fine' angle to the LUT tables
static constexpr uint32_t ANGLETOFINESHIFT  = 19;                   // How many bits to chop off when converting a BAM angle to a 'fine' angle for looking up the trig LUTs.
static constexpr uint32_t SLOPERANGE        = 2048;                 // Number of entries (-1) in the 'TanToAngle' table

//------------------------------------------------------------------------------------------------------------------------------------------
// The trig lookup tables.
// Note that the cosine table is just the sine table phase shifted by PI/2.
// The 'gTanToAngle' also is only defined for angles 0-45 degrees, and the lookup index must be obtained by using 'R_SlopeDiv'.
//------------------------------------------------------------------------------------------------------------------------------------------
extern const fixed_t gFineSine[(5 * FINEANGLES) / 4];
extern const fixed_t* const gFineCosine;
extern const angle_t gTanToAngle[SLOPERANGE + 1];           // +1 so we can handle slope 'x/y' where 'x == y' without extra checking.

// Provides a multiplier for a screen y coordinate.
// When multiplied against a plane/flat z-height, yields the distance away that a particular flat span is.
extern const fixed_t gYSlope[VIEW_3D_H];

// Maximum number of players in a multiplayer game
static constexpr int32_t MAXPLAYERS = 2;

//------------------------------------------------------------------------------------------------------------------------------------------
// How many game ticks happen per second, how many draws happen per second and the actual refresh rate (60Hz NTSC).
// PSX DOOM has a 15Hz timebase, similar to Jaguar DOOM. Some operations however update at 30Hz (rendering speed), like player movements.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int32_t TICRATE = 15;              // Rate that most game logic runs at (15 Hz)
static constexpr int32_t DRAWRATE = 30;             // Original game framerate cap (30 Hz)
static constexpr int32_t VBLANKS_PER_SEC = 60;      // Number of vblanks per second (assuming a 60 Hz display)
static constexpr int32_t VBLANKS_PER_TIC = 4;       // How many vblanks there are in a game tic
static constexpr int32_t VBLANK_TO_TIC_SHIFT = 2;   // How many bits to shift right to get from vblanks to to tics

// What type of game is being played
enum gametype_t : int32_t {
    gt_single,
    gt_coop,
    gt_deathmatch,
#if PC_PSX_DOOM_MODS    // PC-PSX: adding for convenience
    NUMGAMETYPES
#endif
};

// What skill level the game is running at
enum skill_t : int32_t {
    sk_baby,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare,
#if PC_PSX_DOOM_MODS    // PC-PSX: adding for convenience
    NUMSKILLS
#endif
};

// Represents a high level result of running a game loop (MiniLoop).
// Determines where the game navigates to next.
enum gameaction_t : int32_t {
    ga_nothing,         // No game action
    ga_died,            // Player has died
    ga_completed,       // Level complete
    ga_secretexit,      // Level complete (secret exit)
    ga_warped,          // Cheat warping to another level
    ga_exitdemo,        // Demo is finished playback or recording
    ga_recorddemo,      // PSX DOOM: save the recorded demo? (not sure about this one, not enough evidence to say for sure)
    ga_timeout,         // PSX DOOM: player died or menu timed out
    ga_restart,         // PSX DOOM: player restarted the level
    ga_exit,            // PSX DOOM: Exit the current screen or demo
#if PC_PSX_DOOM_MODS
    ga_quitapp          // PC-PSX: exit the application entirely
#endif
};

// Coordinate indexes in a bounding box
enum : int32_t {
    BOXTOP,         // Top box coordinate (max y)
    BOXBOTTOM,      // Bottom box coordinate (min y)
    BOXLEFT,        // Left box coordinate (min x)
    BOXRIGHT        // Right box coordinate (max y)
};

// Episode numbers
enum episodenum_t : int32_t {
    episode_doom1 = 1,
    episode_doom2 = 2
};

static constexpr uint32_t MIN_EPISODE = episode_doom1;
static constexpr uint32_t MAX_EPISODE = episode_doom2;

// Number of maps in the game and number of regular (non secret) maps
static constexpr uint32_t NUM_MAPS = 59;
static constexpr uint32_t NUM_REGULAR_MAPS = 54;

// Maximum number of ticks in a demo.
// The maximum allowed demo size in the original game is 16384 ticks (demo size ~64 KiB).
const int32_t MAX_DEMO_TICKS = 16384;

// Format for a thinker function to handle some periodic update for an actor/system.
// Note that most thinker functions take a thinker specific struct, but the first fields of that struct must always match 'thinker_t'.
typedef void (*think_t)(struct thinker_t&) noexcept;

// An element that receives periodic updates
struct thinker_t {
    thinker_t*  prev;       // Previous thinker in the global linked list of thinkers: 'gThinkerCap' when this thinker is at the beginning of the list
    thinker_t*  next;       // Next thinker in the global linked list of thinkers: 'gThinkerCap' when this thinker is at the end of the list
    think_t     function;   // Thinker 'think' function which is called at regular intervals to run thinker update logic
};

// Flags for 'mobj_t'
static constexpr uint32_t MF_SPECIAL            = 0x1;          // Thing is an item which can be picked up
static constexpr uint32_t MF_SOLID              = 0x2;          // Thing is collidable and blocks movement
static constexpr uint32_t MF_SHOOTABLE          = 0x4;          // Thing is targetable (can be hit) and can take damage
static constexpr uint32_t MF_NOSECTOR           = 0x8;          // Thing is not present in sector thing lists
static constexpr uint32_t MF_NOBLOCKMAP         = 0x10;         // Thing is not present in the blockmap
static constexpr uint32_t MF_AMBUSH             = 0x20;         // Monster is deaf and does not react to sound
static constexpr uint32_t MF_JUSTHIT            = 0x40;         // Monster has just been hit and should try to attack or fight back immediately
static constexpr uint32_t MF_JUSTATTACKED       = 0x80;         // Monster has just attacked and should wait for a bit before attacking again
static constexpr uint32_t MF_SPAWNCEILING       = 0x100;        // When spawning on level start, spawn touching the ceiling rather than the floor (used for hanging sprites)
static constexpr uint32_t MF_NOGRAVITY          = 0x200;        // Don't apply gravity every game tick to this monster (can float in the air)
static constexpr uint32_t MF_DROPOFF            = 0x400;        // The thing is allowed to jump/fall off high ledges
static constexpr uint32_t MF_PICKUP             = 0x800;        // Flag used when moving a thing: pickup items that it comes into contact with
static constexpr uint32_t MF_NOCLIP             = 0x1000;       // Cheat that disables collision on the player
static constexpr uint32_t MF_SLIDE              = 0x2000;       // Not used in PSX Doom: Linux Doom says 'Player: keep info about sliding along walls'
static constexpr uint32_t MF_FLOAT              = 0x4000;       // The thing can fly/float up and down (used by Cacodemons etc.)
static constexpr uint32_t MF_TELEPORT           = 0x8000;       // A flag set temporarily when a thing is the process of teleporting. Stops line crossing and height checks on teleport.
static constexpr uint32_t MF_MISSILE            = 0x10000;      // Set by flying projectiles (Rocket, Imp Fireball): don't damage the same species and explode when blocked.
static constexpr uint32_t MF_DROPPED            = 0x20000;      // The pickup was dropped by an enemy that drops ammo; was not spawned in the level originally. These pickups are worth less ammo.
static constexpr uint32_t MF_SHADOW             = 0x40000;      // Not used in PSX: on PC this causes demons to be drawn fuzzy/semi-translucent
static constexpr uint32_t MF_NOBLOOD            = 0x80000;      // Don't bleed when shot; used by barrels
static constexpr uint32_t MF_CORPSE             = 0x100000;     // Thing is dead: don't stop sliding when positioned hanging over a step
static constexpr uint32_t MF_INFLOAT            = 0x200000;     // The thing is floating up or down so it can move to a certain position
static constexpr uint32_t MF_COUNTKILL          = 0x400000;     // Killing this thing counts towards the kill stats at level end
static constexpr uint32_t MF_COUNTITEM          = 0x800000;     // Picking up this item counts towards the item stats at level end
static constexpr uint32_t MF_SKULLFLY           = 0x1000000;    // The thing is a lost soul in flight: special logic is needed for this thing type
static constexpr uint32_t MF_NOTDMATCH          = 0x2000000;    // The thing is not spawned in deathmatch mode (used for keycards)
static constexpr uint32_t MF_SEETARGET          = 0x4000000;    // A flag set on monsters when the monster can see or has a line of sight towards it's intended target
static constexpr uint32_t MF_BLEND_ON           = 0x10000000;   // PSX DOOM: PSX DOOM: if set then blending is enabled for the object (alpha, additive or subtractive)
static constexpr uint32_t MF_BLEND_MODE_BIT1    = 0x20000000;   // PSX DOOM: 1 of 2 bits determining blend mode if blending is enabled. See 'MF_BLEND' flag combos below for more details.
static constexpr uint32_t MF_BLEND_MODE_BIT2    = 0x40000000;   // PSX DOOM: 1 of 2 bits determining blend mode if blending is enabled. See 'MF_BLEND' flag combos below for more details.

// Blend modes - when thing flags are masked by 'MF_ALL_BLEND_FLAGS':
static constexpr uint32_t MF_BLEND_ALPHA_50 = MF_BLEND_ON;                                              // 50% opacity alpha blend
static constexpr uint32_t MF_BLEND_ADD      = MF_BLEND_ON | MF_BLEND_MODE_BIT1;                         // Additive blend at 100% opacity
static constexpr uint32_t MF_BLEND_SUBTRACT = MF_BLEND_ON | MF_BLEND_MODE_BIT2;                         // Subtractive blend at 100% opacity (also makes monsters 'nightmare' and have 2x hit points)
static constexpr uint32_t MF_BLEND_ADD_25   = MF_BLEND_ON | MF_BLEND_MODE_BIT1 | MF_BLEND_MODE_BIT2;    // Additive blend at 25% opacity

// Convenience flag combo: used for masking out the bits/flags specifying blend mode
static constexpr uint32_t MF_ALL_BLEND_FLAGS = (
    MF_BLEND_ON | MF_BLEND_MODE_BIT1 | MF_BLEND_MODE_BIT2
);

// A function which gets called after regular map object updating is done
typedef void (*latecall_t)(mobj_t& mobj) noexcept;

// Holds state for an object/thing in the game world
struct mobj_t {
    fixed_t         x;                  // Global position in the world, in 16.16 format
    fixed_t         y;
    fixed_t         z;
    subsector_t*    subsector;          // What subsector the map object is currently in (and by extension, what sector)
    mobj_t*         prev;               // Intrusive fields for the global linked list of things
    mobj_t*         next;
    latecall_t      latecall;
    mobj_t*         snext;              // Intrusive fields for the linked list of things in the current sector
    mobj_t*         sprev;
    angle_t         angle;              // Direction the thing is facing in
    uint32_t        sprite;             // Current sprite displayed
    uint32_t        frame;              // Current sprite frame displayed. Must use 'FF_FRAMEMASK' to get the actual frame number.
    mobj_t*         bnext;              // Linked list of things in this blockmap block
    mobj_t*         bprev;              
    fixed_t         floorz;             // Highest floor in contact with map object
    fixed_t         ceilingz;           // Lowest floor in contact with map object
    fixed_t         radius;             // For collision detection
    fixed_t         height;             // For collision detection
    fixed_t         momx;               // Current velocity/speed: x, y & z
    fixed_t         momy;
    fixed_t         momz;
    mobjtype_t      type;               // Type enum
    mobjinfo_t*     info;               // Type data
    int32_t         tics;               // Tick counter for the current state
    state_t*        state;              // State data
    uint32_t        flags;              // See the MF_XXX series of flags for possible bits.
    int32_t         health;             // When this reaches '0' the object is dead
    dirtype_t       movedir;            // For enemy AI, what direction the enemy is moving in
    int32_t         movecount;          // When this reaches 0 a new dir is selected
    mobj_t*         target;             // The current map object being chased or attacked (if any), or for missiles the source object
    int32_t         reactiontime;       // Time left until an attack is allowed
    int32_t         threshold;          // Time left chasing the current target
    player_t*       player;             // Associated player, if any
    uintptr_t       extradata;          // Used for latecall functions
    int16_t         spawnx;             // Used for respawns: original spawn position (integer) x
    int16_t         spawny;             // Used for respawns: original spawn position (integer) y
    uint16_t        spawntype;          // Used for respawns: item 'DoomEd' type/number
    int16_t         spawnangle;         // Used for respawns: item angle
    mobj_t*         tracer;             // Used by homing missiles
};

// A degenerate map object with most of it's fields chopped out to save on memory.
// Used to store a sound origin location within the 'sector_t' struct, so we know where to position sounds when the sector makes a noise (floor move etc.).
struct degenmobj_t {
    fixed_t         x;
    fixed_t         y;
    fixed_t         z;
    subsector_t*    subsector;
};

// Basic player status
enum playerstate_t : int32_t {
    PST_LIVE,       // Player is playing the game
    PST_DEAD,       // Player is dead
    PST_REBORN      // Player is spawning or respawning
};

// Player sprite type, just a weapon and a muzzle flash sprite in Doom
enum psprnum_t : int32_t {
    ps_weapon,      // Main weapon sprite
    ps_flash,       // Weapon muzzle flash sprite
    NUMPSPRITES
};

// State for a player sprite
struct pspdef_t {
    const state_t*  state;      // Pointer to state structure
    int32_t         tics;       // How many tics are left in this state?
    fixed_t         sx;         // Offset of the sprite, x & y
    fixed_t         sy;
};

// Keycard types: skull keys and keycards
enum card_t : int32_t {
    it_redcard,
    it_bluecard,
    it_yellowcard,
    it_redskull,
    it_blueskull,
    it_yellowskull,
    NUMCARDS
};

// Player weapon types: pretty self explanatory
enum weapontype_t : int32_t {
    wp_fist,
    wp_pistol,
    wp_shotgun,
    wp_supershotgun,    // PSX DOOM: note the new position for this, no longer after 'wp_chainsaw'!
    wp_chaingun,
    wp_missile,
    wp_plasma,
    wp_bfg,
    wp_chainsaw,
    NUMWEAPONS,
    wp_nochange         // Used to represent no weapon change
};

// Player ammo types
enum ammotype_t : int32_t {
    am_clip,        // Clip for pistol and chaingun
    am_shell,       // Shells for shotgun and super shotgun
    am_cell,        // Cells for BFG and plasma guns
    am_misl,        // Rockets for rocket launcher
    NUMAMMO,
    am_noammo       // The 'no' ammo type used for ammoless weapons
};

// Describes the state transitions for a weapon and the ammo it uses
struct weaponinfo_t {
    ammotype_t  ammo;           // Type of ammo used by the weapon
    statenum_t  upstate;        // State to go into when raising the weapon
    statenum_t  downstate;      // State to go into when lowering the weapon
    statenum_t  readystate;     // State to go into when the weapon is ready to fire
    statenum_t  atkstate;       // State to go into when the weapon is firing
    statenum_t  flashstate;     // The state to use for the weapon's muzzle flash or S_NULL if there is no muzzle flash
};

// Descriptions for all of the weapons in the game
extern const weaponinfo_t gWeaponInfo[NUMWEAPONS];

// Player powerup types
enum powertype_t : int32_t {
    pw_invulnerability,     // Invulnerability powerup
    pw_strength,            // Berserk powerup
    pw_invisibility,        // Partial invisibility powerup
    pw_ironfeet,            // Radiation suit powerup
    pw_allmap,              // Computer area map powerup
    pw_infrared,            // Light amplification visor (fullbright) powerup
    NUMPOWERS
};

// How many game ticks each of the powerups last for
static constexpr int32_t INVULNTICS = 30 * TICRATE;         // Duration of invulnerability powerup
static constexpr int32_t INVISTICS = 60 * TICRATE;          // Duration of the partial invisibility powerup
static constexpr int32_t INFRATICS = 120 * TICRATE;         // Duration of light amplification visor (fullbright) powerup
static constexpr int32_t IRONTICS = 60 * TICRATE;           // Duration of the radiation suit powerup

// Player cheat flags
static constexpr uint32_t CF_GODMODE        = 0x2;      // Invulnerability
static constexpr uint32_t CF_ALLLINES       = 0x4;      // Show all map lines
static constexpr uint32_t CF_ALLMOBJ        = 0x8;      // Show all map objects
static constexpr uint32_t CF_VRAMVIEWER     = 0x10;     // Showing the vram viewer
static constexpr uint32_t CF_WARPMENU       = 0x20;     // Showing the warp to map menu
static constexpr uint32_t CF_XRAYVISION     = 0x80;     // Do 'xray vision' or transparent walls
static constexpr uint32_t CF_NOPAUSEMSG     = 0x100;    // Don't draw the 'paused' plaque when paused: never set anywhere?

// Player automap flags
static constexpr uint32_t AF_ACTIVE = 0x1;      // Automap is displaying
static constexpr uint32_t AF_FOLLOW = 0x2;      // If set then do not follow the player in the automap (manual automap movement)

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds state specific to each player.
// Note: all 'bool' fields in this struct were 32-bit but were changed to regular 'bool' here (normally 8-bit in most compilers).
//------------------------------------------------------------------------------------------------------------------------------------------
struct player_t {
    mobj_t*         mo;                             // The map object controlled by the player
    playerstate_t   playerstate;                    // Player status
    fixed_t         forwardmove;                    // How much forward movement thrust is to be applied this frame to the player's map object
    fixed_t         sidemove;                       // How much side movement thrust is to be applied this frame to the player's map object
    angle_t         angleturn;                      // How much turning is to be applied this frame to the player's map object
    fixed_t         viewz;                          // The current absolute Z position of the player's view (used for rendering)
    fixed_t         viewheight;                     // The height above the floor that the view is positioned at
    fixed_t         deltaviewheight;                // Current change in view height to apply this frame
    fixed_t         bob;                            // How much view bobbing to apply to the view z this frame (goes to 0 once the player stops)
    int32_t         health;                         // Mainly just used to carry forward the player's health between levels (mo->health is used by most game code)
    int32_t         armorpoints;                    // How many armor points the player has
    int32_t         armortype;                      // 0 = no armor, 1 = regular armor, 2 = mega armor
    int32_t         powers[NUMPOWERS];              // How many ticks left for each power
    bool            cards[NUMCARDS];                // Which keycards the player has
    bool            backpack;                       // True if the player has a backpack
    uint32_t        frags;                          // Player's frag count for deathmatch games
    uint32_t        _unused;                        // An unknown/unused field: can't tell what it is for because there are no uses of it :(
    weapontype_t    readyweapon;                    // The currently equipped weapon
    weapontype_t    pendingweapon;                  // The weapon to equip next or 'wp_nochange' if no weapon change is pending
    bool            weaponowned[NUMWEAPONS];        // Whether each of the weapons is owned (PC-PSX: originally these were 32-bit bools, made them 'bool' instead)
    int32_t         ammo[NUMAMMO];                  // How much of each ammo type the player has
    int32_t         maxammo[NUMAMMO];               // Current maximum amount of each ammo type for the player
    uint32_t        attackdown;                     // Number of ticks the player has had the fire/attack button pressed
    bool            usedown;                        // If true the player has just pressed the use key to try and use doors, switches etc.
    uint32_t        cheats;                         // Cheat (CF_XXX) flags for the player
    uint32_t        refire;                         // The number of times the player has re-fired after firing initially (affects accuracy in some cases)
    uint32_t        killcount;                      // Intermission stats: monster kill count
    uint32_t        itemcount;                      // Intermission stats: number of items picked up
    uint32_t        secretcount;                    // Intermission stats: number of secrets found
    const char*     message;                        // A message to show on the status bar at the next available opportunity (string must be valid at all times)
    uint32_t        damagecount;                    // Screen damage tint amount. Decreases over time, increased when damage taken and according to severity.
    uint32_t        bonuscount;                     // Screen bonus pickup tint amount. Decreases over time and increases after picking up items.
    mobj_t*         attacker;                       // The last thing to damage this player, or null if an environmental thing (crusher etc.) did the damage
    uint32_t        extralight;                     // Extra light to add to the world on account of weapon firing or muzzle flashes
    uint32_t        fixedcolormap;                  // Not used in PSX Doom other than being set to '0'; holdover from PC code
    uint32_t        colormap;                       // Not used in PSX Doom ever; holdover from PC code
    pspdef_t        psprites[NUMPSPRITES];          // Current state information for the player's weapon sprites (weapon + muzzle flash)
    bool            didsecret;                      // Doesn't appear to be used at all in PSX Doom; holdover from PC code
    sector_t*       lastsoundsector;                // The last sector the player made noise from (used to early out from flood filling)
    int32_t         automapx;                       // View position in the automap: x
    int32_t         automapy;                       // View position in the automap: y
    uint32_t        automapscale;                   // Render scaling for the automap
    uint32_t        automapflags;                   // Automap related (AF_XXX) flags
    int32_t         turnheld;                       // How many ticks one of the turn buttons has been pressed: used for turn acceleration
};

#if PC_PSX_DOOM_MODS
    //--------------------------------------------------------------------------------------------------------------------------------------
    // A data structure used to hold all of the inputs and actions for a player for a particular frame.
    // This is now used in networking and as well as single player to drive all game inputs, instead of raw button codes.
    // It's advantages are that it supports analog motions and doesn't care about control sources or bindings at all - simplifying usage.
    // Note: most fields are only used depending on certain contexts, like when you are in a menu or in-game.
    //--------------------------------------------------------------------------------------------------------------------------------------
    struct TickInputs {
        // In-game: movement (0-1 range) and turning delta from analog sources and mouse input
        fixed_t analogForwardMove;
        fixed_t analogSideMove;
        angle_t analogTurn;

        // In-game: which weapon the player wants to try and directly switch to ('wp_nochange' if not switching).
        // This is used to allow direct switching to weapons for PC.
        uint8_t directSwitchToWeapon;

        // In-game: whether various action buttons are pressed
        uint8_t bTurnLeft : 1;
        uint8_t bTurnRight : 1;
        uint8_t bMoveForward : 1;
        uint8_t bMoveBackward : 1;
        uint8_t bStrafeLeft : 1;
        uint8_t bStrafeRight : 1;
        uint8_t bUse : 1;
        uint8_t bAttack : 1;

        uint8_t bRun : 1;
        uint8_t bStrafe : 1;
        uint8_t bPrevWeapon : 1;
        uint8_t bNextWeapon : 1;
        uint8_t bTogglePause : 1;
        uint8_t bToggleMap : 1;             // Toggle the automap on/off
        uint8_t bAutomapZoomIn : 1;
        uint8_t bAutomapZoomOut : 1;

        uint8_t bAutomapMoveLeft : 1;
        uint8_t bAutomapMoveRight : 1;
        uint8_t bAutomapMoveUp : 1;
        uint8_t bAutomapMoveDown : 1;
        uint8_t bAutomapPan : 1;            // Manually pan the automap
        uint8_t bRespawn : 1;               // Respawn in deathmatch
        uint8_t _unused1 : 2;

        // UI: whether various action buttons are pressed
        uint8_t bMenuUp : 1;
        uint8_t bMenuDown : 1;
        uint8_t bMenuLeft : 1;
        uint8_t bMenuRight : 1;
        uint8_t bMenuOk : 1;
        uint8_t bMenuStart : 1;
        uint8_t bMenuBack : 1;
        uint8_t bEnterPasswordChar : 1;

        uint8_t bDeletePasswordChar : 1;
        uint8_t _unused2 : 7;

        // Unused bytes to pad the struct out to 24 bytes: can be repurposed later if need be
        uint8_t _unused3;
        uint8_t _unused4;
    };

    // Packet sent/received by all players when connecting to a game
    struct NetPacket_Connect {
        uint32_t    gameId;             // Must match the expected game id
        gametype_t  startGameType;      // Only sent by the server for the game: what type of game will be played
        skill_t     startGameSkill;     // Only sent by the server for the game: what skill level will be used
        int32_t     startMap;           // Only sent by the server for the game: what starting map will be used
    };

    // Packet sent/received by all players to share per-tick updates for a network game
    struct NetPacket_Tick {
        uint32_t    gameId;             // Must match the expected game id
        uint32_t    errorCheck;         // Error checking bits for detecting if all players are in sync: populated using the current position and angle for all players
        int32_t     elapsedVBlanks;     // How many vblanks have elapsed for the player sending the update
        TickInputs  inputs;             // Inputs for the player sending this update
    };
#endif
