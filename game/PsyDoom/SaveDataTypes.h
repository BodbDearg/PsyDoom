#pragma once

#include "Doom/doomdef.h"
#include "Doom/Game/p_doors.h"
#include "SmallString.h"

#include <memory>

class InputStream;
class OutputStream;
enum bwhere_e : int32_t;
enum ceiling_e : int32_t;
enum class ReadSaveResult : int32_t;
enum floor_e : int32_t;
enum plat_e : int32_t;
enum plattype_e : int32_t;
enum sfxenum_t : int32_t;
enum spclface_e : int32_t;
enum vldoor_e : int32_t;
struct button_t;
struct ceiling_t;
struct delayaction_t;
struct fireflicker_t;
struct floormove_t;
struct glow_t;
struct lightflash_t;
struct plat_t;
struct side_t;
struct stbar_t;
struct strobe_t;
struct vlcustomdoor_t;
struct vldoor_t;

namespace ScriptingEngine {
    struct ScheduledAction;
}

// The current save file format version
static constexpr uint32_t SAVE_FILE_VERSION = 2;

// The expected file ids in little endian format (says 'PSYDSAVF' at the top of the file)
static constexpr uint32_t SAVE_FILE_ID1 = 0x44595350;
static constexpr uint32_t SAVE_FILE_ID2 = 0x46564153;

// Saved state for a sector
struct SavedSectorT {
    fixed_t         floorheight;        // Current floor height for the sector
    fixed_t         ceilingheight;      // Current ceiling height for the sector
    int16_t         floorpic;           // Index of the flat texture used for the sector floor
    int16_t         ceilingpic;         // Index of the flat texture used for the sector ceiling
    uint8_t         colorid;            // Which of the colored light colors (by index) to use for the sector
    uint8_t         ceilColorid;        // PsyDoom: the ceiling color for 2-colored lighting
    int16_t         lightlevel;         // Sector light level (should be 0-255)
    int32_t         special;            // Special action for the sector: damage, secret, light flicker etc.
    int32_t         tag;                // Tag for the sector for use in targetted actions (triggered by switches, line crossings etc.)
    int32_t         soundtargetIdx;     // Index of the thing that last made a noise in the sector to alert monsters (-1 if none)
    uint32_t        flags;              // Sector flags (new addition for PSX)
    fixed_t         floorTexOffsetX;    // PsyDoom: floor texture x offset (can be used to scroll flats)
    fixed_t         floorTexOffsetY;    // PsyDoom: floor texture y offset (can be used to scroll flats)
    fixed_t         ceilTexOffsetX;     // PsyDoom: ceiling texture x offset (can be used to scroll flats)
    fixed_t         ceilTexOffsetY;     // PsyDoom: ceiling texture y offset (can be used to scroll flats)

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const sector_t& sector) noexcept;
    void deserializeTo(sector_t& sector) const noexcept;
};

static_assert(sizeof(SavedSectorT) == 48);

// Saved state for a line
struct SavedLineT {
    uint32_t        flags;              // ML_XXX line flags
    int32_t         special;            // What special action (switch, trigger etc.) the line does
    int32_t         tag;                // Tag for the action: what to affect in some cases, in terms of sectors

    void byteSwap() noexcept;
    void serializeFrom(const line_t& line) noexcept;
    void deserializeTo(line_t& line) const noexcept;
};

static_assert(sizeof(SavedLineT) == 12);

// Saved state for a side
struct SavedSideT {
    fixed_t         textureoffset;      // Horizontal texture offset for the side
    fixed_t         rowoffset;          // Vertical texture offset for the side
    int16_t         toptexture;         // Wall texture index for the side's top texture
    int16_t         bottomtexture;      // Wall texture index for the side's bottom texture
    int16_t         midtexture;         // Wall texture index for the side's mid/wall texture

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const side_t& side) noexcept;
    void deserializeTo(side_t& side) const noexcept;
};

static_assert(sizeof(SavedSideT) == 16);

// Saved version of a map object.
// Note: the map object is initially assumed to NOT be associated with a player.
// Only once a 'SavedPlayerT' is found pointing to it, does the spawned map object become associated with it's player.
struct SavedMobjT {
    fixed_t         x;                  // Global position in the world, in 16.16 format
    fixed_t         y;
    fixed_t         z;
    int32_t         tag;                // PsyDoom: a tag that can be assigned via scripting, for identification purposes
    int32_t         subsectorIdx;       // What subsector (by index) the map object is currently in (and by extension, what sector)
    angle_t         angle;              // Direction the thing is facing in
    int32_t         sprite;             // Current sprite displayed
    int32_t         frame;              // Current sprite frame displayed. Must use 'FF_FRAMEMASK' to get the actual frame number.
    fixed_t         floorz;             // Highest floor in contact with map object
    fixed_t         ceilingz;           // Lowest floor in contact with map object
    fixed_t         radius;             // For collision detection
    fixed_t         height;             // For collision detection
    fixed_t         momx;               // Current velocity/speed: x, y & z
    fixed_t         momy;
    fixed_t         momz;
    mobjtype_t      type;               // Type enum
    int32_t         tics;               // Tick counter for the current state
    int32_t         stateIdx;           // Index of the state that the object is using
    uint32_t        flags;              // See the MF_XXX series of flags for possible bits.
    int32_t         health;             // When this reaches '0' the object is dead
    dirtype_t       movedir;            // For enemy AI, what direction the enemy is moving in
    int32_t         movecount;          // When this reaches 0 a new dir is selected
    int32_t         targetIdx;          // Index of the current map object being chased or attacked (if any), or for missiles the source object. '-1' if nothing.
    int32_t         reactiontime;       // Time left until an attack is allowed
    int32_t         threshold;          // Time left chasing the current target
    int16_t         spawnx;             // Used for respawns: original spawn position (integer) x
    int16_t         spawny;             // Used for respawns: original spawn position (integer) y
    uint16_t        spawntype;          // Used for respawns: item 'DoomEd' type/number
    int16_t         spawnangle;         // Used for respawns: item angle
    int32_t         tracerIdx;          // Used by homing missiles (map object index, or '-1' if none)

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const mobj_t& mobj) noexcept;
    void deserializeTo(mobj_t& mobj) const noexcept;
};

static_assert(sizeof(SavedMobjT) == 112);

// Saved state for a player weapon sprite (gun and muzzle flash)
struct SavedPspdefT {
    int32_t         stateIdx;           // Index of the state that the sprite is using
    int32_t         tics;               // How many tics are left in this state?
    fixed_t         sx;                 // Offset of the sprite, x & y
    fixed_t         sy;

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const pspdef_t& spr) noexcept;
    void deserializeTo(pspdef_t& spr) const noexcept;
};

static_assert(sizeof(SavedPspdefT) == 16);

// Saved state for a player
struct SavedPlayerT {
    int32_t         mobjIdx;                    // Index of the map object controlled by the player
    playerstate_t   playerstate;                // Player status
    fixed_t         forwardmove;                // How much forward movement thrust is to be applied this frame to the player's map object
    fixed_t         sidemove;                   // How much side movement thrust is to be applied this frame to the player's map object
    angle_t         angleturn;                  // How much turning is to be applied this frame to the player's map object
    fixed_t         viewz;                      // The current absolute Z position of the player's view (used for rendering)
    fixed_t         viewheight;                 // The height above the floor that the view is positioned at
    fixed_t         deltaviewheight;            // Current change in view height to apply this frame
    fixed_t         bob;                        // How much view bobbing to apply to the view z this frame (goes to 0 once the player stops)
    int32_t         health;                     // Mainly just used to carry forward the player's health between levels (mo->health is used by most game code)
    int32_t         armorpoints;                // How many armor points the player has
    int32_t         armortype;                  // 0 = no armor, 1 = regular armor, 2 = mega armor
    int32_t         powers[NUMPOWERS];          // How many ticks left for each power
    bool            cards[NUMCARDS];            // Which keycards the player has
    bool            backpack;                   // True if the player has a backpack
    weapontype_t    readyweapon;                // The currently equipped weapon
    weapontype_t    pendingweapon;              // The weapon to equip next or 'wp_nochange' if no weapon change is pending
    bool            weaponowned[NUMWEAPONS];    // Whether each of the weapons is owned (PsyDoom: originally these were 32-bit bools, made them 'bool' instead)
    int32_t         ammo[NUMAMMO];              // How much of each ammo type the player has
    int32_t         maxammo[NUMAMMO];           // Current maximum amount of each ammo type for the player
    uint32_t        cheats;                     // Cheat (CF_XXX) flags for the player
    uint32_t        killcount;                  // Intermission stats: monster kill count
    uint32_t        itemcount;                  // Intermission stats: number of items picked up
    uint32_t        secretcount;                // Intermission stats: number of secrets found
    uint32_t        damagecount;                // Screen damage tint amount. Decreases over time, increased when damage taken and according to severity.
    uint32_t        bonuscount;                 // Screen bonus pickup tint amount. Decreases over time and increases after picking up items.
    int32_t         attackerIdx;                // Index of the last thing to damage this player, or '-1' if an environmental thing (crusher etc.) did the damage.
    uint32_t        extralight;                 // Extra light to add to the world on account of weapon firing or muzzle flashes
    SavedPspdefT    psprites[NUMPSPRITES];      // Current state information for the player's weapon sprites (weapon + muzzle flash)
    int32_t         automapx;                   // View position in the automap: x
    int32_t         automapy;                   // View position in the automap: y
    uint32_t        automapscale;               // Render scaling for the automap

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const player_t& player) noexcept;
    void deserializeTo(player_t& player) const noexcept;
};

static_assert(sizeof(SavedPlayerT) == 208);

// Saved state for a vertical door mover
struct SavedVLDoorT {
    vldoor_e        type;                   // What type of door it is
    uint32_t        sectorIdx;              // Index of the sector affected
    fixed_t         topheight;              // Sector ceiling height when opened
    fixed_t         speed;                  // Speed of door movement
    int32_t         direction;              // Current movement direction: 1 = up, 0 = opened, -1 = down
    int32_t         topwait;                // Door setting: total number of tics for the door to wait in the opened state
    int32_t         topcountdown;           // Door state: how many tics before the door starts closing

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const vldoor_t& door) noexcept;
    void deserializeTo(vldoor_t& door) const noexcept;
};

static_assert(sizeof(SavedVLDoorT) == 28);

// Saved state for a custom vertical door mover
struct SavedVLCustomdoorT {
    uint32_t        sectorIdx;              // Index of the sector affected
    CustomDoorDef   def;                    // Settings for the door
    int32_t         direction;              // Current direction of movement (1 = open, 0 = wait, -1 = close)
    int32_t         postWaitDirection;      // Direction that the door will go in after waiting
    int32_t         countdown;              // Current countdown (if waiting)

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const vlcustomdoor_t& door) noexcept;
    void deserializeTo(vlcustomdoor_t& door) const noexcept;
};

static_assert(sizeof(SavedVLCustomdoorT) == 52);

// Saved state for a floor mover
struct SavedFloorMoveT {
    floor_e     type;                       // What type of behavior the floor mover has
    bool        crush;                      // Does the floor movement cause crushing when things don't fit?
    bool        bDoFinishScript;            // PsyDoom: if 'type' is 'customFloor' then this can be 'true' to execute a script action on finish
    uint32_t    sectorIdx;                  // Index of the sector affected
    int32_t     direction;                  // 1 = up, -1 = down
    int32_t     newspecial;                 // For certain floor mover types, a special to assign to the sector when the movement is done
    int16_t     texture;                    // For certain floor mover types, a texture to assign to the sector when the movement is done
    fixed_t     floordestheight;            // Destination height for the floor mover
    fixed_t     speed;                      // Speed that the floor moves at
    sfxenum_t   moveSound;                  // PsyDoom custom floors: sound to make when moving ('sfx_None' if none)
    uint32_t    moveSoundFreq;              // PsyDoom custom floors: how many tics between instances of the move sound playing
    sfxenum_t   stopSound;                  // PsyDoom custom floors: sound to make when stopping ('sfx_None' if none)
    int32_t     finishScriptActionNum;      // PsyDoom custom floors: if enabled, the script action number to execute when the floor is done moving
    int32_t     finishScriptUserdata;       // PsyDoom custom floors: userdata field which will be sent along to the finish action when it executes

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const floormove_t& floor) noexcept;
    void deserializeTo(floormove_t& floor) const noexcept;
};

static_assert(sizeof(SavedFloorMoveT) == 52);

// Saved state for a ceiling mover
struct SavedCeilingT {
    ceiling_e       type;                       // What type of behavior the ceiling mover has
    uint32_t        sectorIdx;                  // Index of the sector affected
    fixed_t         bottomheight;               // Lowest destination height
    fixed_t         topheight;                  // Highest destination height
    fixed_t         speed;                      // Speed of movement up or down
    bool            crush;                      // Does the ceiling damage things when they don't fit?
    bool            bIsCrushing;                // PsyDoom: a flag set to 'true' if the ceiling was crushing on the last frame
    bool            bDoFinishScript;            // PsyDoom: custom crushers: whether to do a 'finish' script action on fully stopping
    bool            bIsActive;                  // Is this ceiling in the 'active ceilings' list?
    int32_t         direction;                  // 1 = up, 0 = waiting, -1 = down
    int32_t         tag;                        // Sector tag for the ceiling mover's sector
    int32_t         olddirection;               // In-stasis ceilings: which way the ceiling was moving before it was paused
    fixed_t         crushSpeed;                 // Speed when crushing something (usually 1/8 of normal speed)
    int32_t         dirChangesLeft;             // How many times the crusher can change direction before stopping (if '-1' then no limit)
    sfxenum_t       moveSound;                  // Sound to make when moving ('sfx_None' if none)
    uint32_t        moveSoundFreq;              // How many tics between instances of the move sound playing
    sfxenum_t       changeDirSound;             // Sound to make when changing direction ('sfx_None' if none)
    sfxenum_t       stopSound;                  // Sound to make when stopping ('sfx_None' if none)
    int32_t         finishScriptActionNum;      // If enabled, a script action to execute when the ceiling has come to a complete stop/finished
    int32_t         finishScriptUserdata;       // Userdata to pass to the 'finish' script action

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const ceiling_t& ceil) noexcept;
    void deserializeTo(ceiling_t& ceil) const noexcept;
};

static_assert(sizeof(SavedCeilingT) == 68);

// Saved state for a moving platform or elevator
struct SavedPlatT {
    uint32_t        sectorIdx;                  // Index of the sector affected
    fixed_t         speed;                      // Speed that the platform moves at
    fixed_t         low;                        // Height of the lowest floor surrounding the platform's sector
    fixed_t         high;                       // Height of the highest floor surrounding the platform's sector
    int32_t         wait;                       // How long the moving platform waits before returning to it's original position
    int32_t         count;                      // Tick counter: how long of a wait there is left for the platform before it returns to it's original position
    plat_e          status;                     // Current platform state (up/down/wait/paused)
    plat_e          oldstatus;                  // Platform state before it was paused or put into stasis
    bool            crush;                      // If true then the moving platform damages things which don't fit
    bool            bDoFinishScript;            // PsyDoom: custom platforms: whether to do a 'finish' script action on fully stopping
    bool            bIsActive;                  // Is this platform in the 'active platforms' list?
    int32_t         tag;                        // The tag for the line which activated this platform
    plattype_e      type;                       // What type of behavior the moving platform has
    plat_e          finishState;                // PsyDoom custom platforms: which state the platform should finish on, once it's complete
    sfxenum_t       startSound;                 // PsyDoom custom platforms: sound to make when starting ('sfx_None' if none)
    sfxenum_t       moveSound;                  // PsyDoom custom platforms: sound to make when moving ('sfx_None' if none)
    uint32_t        moveSoundFreq;              // PsyDoom custom platforms: how many tics between instances of the move sound playing
    sfxenum_t       stopSound;                  // PsyDoom custom platforms: sound to make when stopping ('sfx_None' if none)
    int32_t         finishScriptActionNum;      // PsyDoom custom platforms: if enabled, a script action to execute when the platform has come to a complete stop/finished
    int32_t         finishScriptUserdata;       // PsyDoom custom platforms: userdata to pass to the 'finish' script action

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const plat_t& plat) noexcept;
    void deserializeTo(plat_t& plat) const noexcept;
};

static_assert(sizeof(SavedPlatT) == 72);

// Saved state for a flicker lighting effect
struct SavedFireFlickerT {
    uint32_t    sectorIdx;      // Index of the sector affected
    int32_t     count;
    int32_t     maxlight;
    int32_t     minlight;

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const fireflicker_t& light) noexcept;
    void deserializeTo(fireflicker_t& light) const noexcept;
};

static_assert(sizeof(SavedFireFlickerT) == 16);

// Saved state for a flashing light effect
struct SavedLightFlashT {
    uint32_t    sectorIdx;      // Index of the sector affected
    int32_t     count;
    int32_t     maxlight;
    int32_t     minlight;
    int32_t     maxtime;
    int32_t     mintime;

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const lightflash_t& light) noexcept;
    void deserializeTo(lightflash_t& light) const noexcept;
};

static_assert(sizeof(SavedLightFlashT) == 24);

// Saved state for a strobe light effect
struct SavedStrobeT {
    uint32_t    sectorIdx;      // Index of the sector affected
    int32_t     count;
    int32_t     minlight;
    int32_t     maxlight;
    int32_t     darktime;
    int32_t     brighttime;

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const strobe_t& light) noexcept;
    void deserializeTo(strobe_t& light) const noexcept;
};

static_assert(sizeof(SavedStrobeT) == 24);

// Saved state for a glowing light effect
struct SavedGlowT {
    uint32_t    sectorIdx;      // Index of the sector affected
    int32_t     minlight;
    int32_t     maxlight;
    int32_t     direction;

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const glow_t& light) noexcept;
    void deserializeTo(glow_t& light) const noexcept;
};

static_assert(sizeof(SavedGlowT) == 16);

// Saved state for a delayed exit action
struct SavedDelayedExitT {
    int32_t     ticsleft;       // How many tics until we perform the action

    void byteSwap() noexcept;
    void serializeFrom(const delayaction_t& action) noexcept;
    void deserializeTo(delayaction_t& action) const noexcept;
};

static_assert(sizeof(SavedDelayedExitT) == 4);

// Saved state for a button that will switch back to it's prior state after a while
struct SavedButtonT {
    uint32_t    lineIdx;        // Index of the linedef which has the button/switch
    bwhere_e    where;          // What part of the line was activated
    int32_t     btexture;       // The texture to switch the line back to
    int32_t     btimer;         // The countdown for when the button reverts to it's former state; reverts when it reaches '0'

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const button_t& btn) noexcept;
    void deserializeTo(button_t& btn) const noexcept;
};

static_assert(sizeof(SavedButtonT) == 16);

// Saved state for a scheduled script action.
// This is presently identical to the runtime struct but we keep the types separate in case they need to differ.
struct SavedScheduledAction {
    int32_t     actionNum;              // Which action function to execute with
    int32_t     delayTics;              // Game tics left until the action executes
    int32_t     executionsLeft;         // The number of action executions left; '0' if the action will not execute again, or '-1' if infinitely repeating.
    int32_t     repeatDelay;            // The delay in tics between repeats ('0' means execute every tic)
    int32_t     tag;                    // User defined tag associated with the action
    int32_t     userdata;               // User defined data associated with the action
    bool        bPaused;                // If 'true' then the action is paused, otherwise it's unpaused
    bool        bPendingExecute;        // If 'true' then the action is pending execution this frame

    void byteSwap() noexcept;
    void serializeFrom(const ScriptingEngine::ScheduledAction& action) noexcept;
    void deserializeTo(ScriptingEngine::ScheduledAction& action) const noexcept;
};

static_assert(sizeof(SavedScheduledAction) == 28);

// Saved state for the status bar
struct SavedSTBarT {
    uint32_t        face;                   // Index of the face sprite to currently use
    spclface_e      specialFace;            // What special face to do next
    bool            gotgibbed;              // True if the player just got gibbed
    int32_t         gibframe;               // What frame of the gib animation is currently showing
    int32_t         gibframeTicsLeft;       // How many game ticks left in the current gib animation frame
    char            alertMessage[32];       // PsyDoom: a message displayed near the center of the screen which is not interrupted by pickups
    int32_t         alertMessageTicsLeft;   // PsyDoom: how many tics left before the alert message is done displaying

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFrom(const stbar_t& sbar) noexcept;
    void deserializeTo(stbar_t& sbar) const noexcept;
};

static_assert(sizeof(SavedSTBarT) == 56);

// Save data globals (non object lists)
struct SavedGlobals {
    int32_t             gameMap;                        // The map being played
    int32_t             nextMap;                        // If exiting which map to go to next
    skill_t             gameSkill;                      // Skill level the game is being played with
    int64_t             levelElapsedTime;               // How long ago the level started (in microseconds)
    int32_t             totalKills;                     // Player stats: kills made so far in the level
    int32_t             totalItems;                     // Player stats: items picked up so far in the level
    int32_t             totalSecret;                    // Player stats: secrets found so far in the level
    SavedPlayerT        player;                         // Player state    
    uint32_t            prndIndex;                      // Current position in the RNG table for main game
    uint32_t            mrndIndex;                      // Current position in the RNG table for UI
    int32_t             gameTic;                        // Current game tick count (15 Hz ticks)
    int32_t             ticCon;                         // The current number of 1 vblank ticks
    int32_t             ticRemainder;                   // How many unsimulated player sprite vblanks there are
    int32_t             mapBossSpecialFlags;            // PSX addition: What types of boss specials (triggers) are active on the current map
    uint32_t            extCameraTicsLeft;              // PsyDoom 'external' camera: how many tics it has left, the camera position and angle.
    fixed_t             extCameraX;
    fixed_t             extCameraY;
    fixed_t             extCameraZ;
    angle_t             extCameraAngle;
    int32_t             numPasswordCharsEntered;        // How many characters have been input for the current password sequence
    uint8_t             passwordCharBuffer[10];         // The password input buffer
    int32_t             curCDTrack;                     // If non zero play the specified cd-audio track (handles Club Doom music activated)
    SavedSTBarT         statusBar;                      // The main state for the status bar
    int32_t             faceTics;                       // Ticks left for current face
    bool                bDrawSBFace;                    // Draw the face sprite?
    bool                bGibDraw;                       // Are we animating the face being gibbed?
    bool                bDoSpclFace;                    // Should we do a special face next?
    int32_t             newFace;                        // Which normal face to use next
    spclface_e          spclFaceType;                   // Which special face to use next

    void byteSwap() noexcept;
    bool validate() const noexcept;
    void serializeFromGlobals() noexcept;
    void deserializeToGlobals() const noexcept;
};

static_assert(sizeof(SavedGlobals) == 384);

// Header for a save file, comes first in the file
struct SaveFileHdr {
    uint32_t    fileId1;                // Should match 'SAVE_FILE_ID1'
    uint32_t    fileId2;                // Should match 'SAVE_FILE_ID2'
    uint32_t    version;                // Should match 'SAVE_FILE_VERSION'
    int32_t     mapNum;                 // Map number
    int64_t     secondsPlayed;          // Number of seconds the player has been playing the map
    String32    mapName;                // Name of the map
    uint64_t    mapHashWord1;           // Hash of all the map data: word 1 (used to verify the same map is being played)
    uint64_t    mapHashWord2;           // Hash of all the map data: word 2 (used to verify the same map is being played)
    uint32_t    numSectors;             // Number of 'SavedSectorT' in the save file
    uint32_t    numLines;               // Number of 'SavedLineT' in the save file
    uint32_t    numSides;               // Number of 'SavedSideT' in the save file
    uint32_t    numMobjs;               // Number of 'SavedMobjT' in the save file
    uint32_t    numVlDoors;             // Number of 'SavedVLDoorT' in the save file
    uint32_t    numVlCustomDoors;       // Number of 'SavedVLCustomdoorT' in the save file
    uint32_t    numFloorMovers;         // Number of 'SavedFloorMoveT' in the save file
    uint32_t    numCeilings;            // Number of 'SavedCeilingT' in the save file
    uint32_t    numPlats;               // Number of 'SavedPlatT' in the save file
    uint32_t    numFireFlickers;        // Number of 'SavedFireFlickerT' in the save file
    uint32_t    numLightFlashes;        // Number of 'SavedLightFlashT' in the save file
    uint32_t    numStrobes;             // Number of 'SavedStrobeT' in the save file
    uint32_t    numGlows;               // Number of 'SavedGlowT' in the save file
    uint32_t    numDelayedExits;        // Number of 'SavedDelayedExitT' in the save file
    uint32_t    numButtons;             // Number of 'SavedButtonT' in the save file
    uint32_t    numScheduledActions;    // Number of 'SavedScheduledAction' in the save file

    void byteSwap() noexcept;
    bool validateFileId() const noexcept;
    bool validateVersion() const noexcept;
    bool validateMapNum() const noexcept;
    bool validateMapHash() const noexcept;
    bool validate() const noexcept;
};

static_assert(sizeof(SaveFileHdr) == 136);

// Save data for the game in it's entirety, in order of how it appears in the file.
// Just encapsulates state for a single player game, does NOT support multiplayer.
// 
// Notes:
//  (1) The data is always read and written in little endian format.
//      If the host machine is big endian then byte order corrections are performed automatically.
//  (2) Unlike other types this object deliberately does not handle validation, serialization and deserialization.
//      Orchestrating those operations at a high level is handled by the 'SaveAndLoad' module.
//
struct SaveData {
    SaveFileHdr                                 hdr;
    SavedGlobals                                globals;
    std::unique_ptr<SavedSectorT[]>             sectors;
    std::unique_ptr<SavedLineT[]>               lines;
    std::unique_ptr<SavedSideT[]>               sides;
    std::unique_ptr<SavedMobjT[]>               mobjs;
    std::unique_ptr<SavedVLDoorT[]>             vlDoors;
    std::unique_ptr<SavedVLCustomdoorT[]>       vlCustomDoors;
    std::unique_ptr<SavedFloorMoveT[]>          floorMovers;
    std::unique_ptr<SavedCeilingT[]>            ceilings;
    std::unique_ptr<SavedPlatT[]>               plats;
    std::unique_ptr<SavedFireFlickerT[]>        fireFlickers;
    std::unique_ptr<SavedLightFlashT[]>         lightFlashes;
    std::unique_ptr<SavedStrobeT[]>             strobes;
    std::unique_ptr<SavedGlowT[]>               glows;
    std::unique_ptr<SavedDelayedExitT[]>        delayedExits;
    std::unique_ptr<SavedButtonT[]>             buttons;
    std::unique_ptr<SavedScheduledAction[]>     scheduledActions;

    bool writeTo(OutputStream& out) const noexcept;
    [[nodiscard]] ReadSaveResult readFrom(InputStream& in) noexcept;
};
