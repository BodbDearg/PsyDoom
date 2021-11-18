//------------------------------------------------------------------------------------------------------------------------------------------
// A module containing most of the low-level serialization/deserialization logic for each game data type.
// 
// The meaning of the individual data object functions is as follows:
//  (1) endianCorrect:  Converts from little endian to host endian on reading, or host endian to little on writing.
//                      Basically performs a byte swap if the endianness is not little.
//  (2) validate:       Checks whether loaded save data is actually valid/sane based on certain global game constraints or the current level.
//                      The timing of when this can be called during level loading varies depending on the object type.
//  (3) serializeFrom:  Serialize the data from a runtime object to the save file data structure.
//                      Not all fields may be serialized, depending on the object.
//  (4) deserializeTo:  Deserialize the data from the save file object to a runtime object.
//                      Some fields that are not serialized may be default initialized instead, or recreated somehow.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "SaveDataTypes.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_ceiling.h"
#include "Doom/Game/p_doors.h"
#include "Doom/Game/p_floor.h"
#include "Doom/Game/p_inter.h"
#include "Doom/Game/p_lights.h"
#include "Doom/Game/p_local.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_plats.h"
#include "Doom/Game/p_pspr.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Game/p_switch.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Game/sprinfo.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/pw_main.h"
#include "Doom/UI/st_main.h"
#include "Endian.h"
#include "Game.h"
#include "InputStream.h"
#include "MapHash.h"
#include "OutputStream.h"
#include "SaveAndLoad.h"
#include "ScriptingEngine.h"
#include "Wess/psxcd.h"

#include <type_traits>

// Make sure the global password character buffer is the expected size
static_assert(PW_SEQ_LEN == C_ARRAY_SIZE(SavedGlobals::passwordCharBuffer), "Password char buffer has unexpected size!");

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform an in-place byte swap of a single field
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void byteSwapField(T& toSwap) noexcept {
    toSwap = Endian::byteSwap(toSwap);
}

template <class T>
static void byteSwapEnumField(T& toSwap) noexcept {
    typedef std::underlying_type_t<T> EnumT;
    toSwap = (T) Endian::byteSwap((EnumT) toSwap);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Call 'byteSwap' on an array of objects
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void byteSwapObjects(T* const pObjs, const uint32_t numObjs) noexcept {
    for (uint32_t i = 0; i < numObjs; ++i) {
        pObjs[i].byteSwap();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: allocates an array of objects and then reads those objects from the specified file
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void readArray(InputStream& file, std::unique_ptr<T[]>& arrayStorage, const uint32_t numElems) THROWS {
    arrayStorage = std::make_unique<T[]>(numElems);
    file.readArray(arrayStorage.get(), numElems);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks to see if various object indexes are valid
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isValidFlatNum(const int32_t flatNum) noexcept {
    return (
        ((flatNum >= 0) && (flatNum < gNumFlatLumps)) ||    // Valid regular flat
        (flatNum == -1)                                     // Sky flat
    );
}

static bool isValidTexNum(const int32_t texNum) noexcept {
    return (
        ((texNum >= 0) && (texNum < gNumTexLumps)) ||       // Valid regular texture
        (texNum == -1)                                      // A texture that is not rendered
    );
}

static bool isValidMobjIdx(const int32_t idx) noexcept {
    return (
        ((idx >= 0) && (idx < (int32_t) SaveAndLoad::gMobjList.size())) ||      // A normal (valid) map object index
        (idx == -1)                                                             // Special index representing no map object
    );
}

static bool isValidSectorIdx(const int32_t idx) noexcept {
    return ((idx >= 0) && (idx < gNumSectors));
}

static bool isValidSubsectorIdx(const int32_t idx) noexcept {
    return ((idx >= 0) && (idx < gNumSubsectors));
}

static bool isValidLineIdx(const int32_t idx) noexcept {
    return ((idx >= 0) && (idx < gNumLines));
}

static bool isValidSpriteIdx(const int32_t idx) noexcept {
    return ((idx >= 0) && (idx < gNumSpriteLumps));
}

static bool isValidMobjType(const mobjtype_t type) noexcept {
    return ((type >= 0) && (type < gNumMobjInfo));
}

static bool isValidStateIdx(const int32_t idx) noexcept {
    return ((idx >= 0) && (idx < gNumStates));
}

static bool isValidMoveDir(const dirtype_t dir) noexcept {
    return ((dir >= 0) && (dir < NUMDIRS));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Validate an array of objects by calling 'validate()'
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static bool validateObjects(T* const pObjs, const uint32_t numObjs) noexcept {
    for (uint32_t i = 0; i < numObjs; ++i) {
        if (!pObjs[i].validate())
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks to see if a ceiling or platform is active
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isActiveCeil(const ceiling_t& ceil) noexcept {
    return (std::find(gpActiveCeilings.begin(), gpActiveCeilings.end(), &ceil) != gpActiveCeilings.end());
}

static bool isActivePlat(const plat_t& plat) noexcept {
    return (std::find(gpActivePlats.begin(), gpActivePlats.end(), &plat) != gpActivePlats.end());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out the index of the specified map object.
// Returns '-1' if the map object is not in the global map objects list, or is null.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t getMobjIndex(mobj_t* const pMobj) noexcept {
    if (pMobj) {
        const auto iter = SaveAndLoad::gMobjToIdx.find(pMobj);

        if (iter != SaveAndLoad::gMobjToIdx.end()) {
            return iter->second;
        } else {
            // This should never happen, if it does then it might indicate some bad problems elsewhere...
            I_Error("SaveData: getMobjIndex: can't serialize invalid map object with no index!");
        }
    }

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the map object at the specified index in the map objects list.
// If a negative index is passed returns 'nullptr'.
//------------------------------------------------------------------------------------------------------------------------------------------
static mobj_t* getMobjAtIdx(const int32_t idx) noexcept {
    if (idx >= 0) {
        if (idx < (int32_t) SaveAndLoad::gMobjList.size()) {
            return SaveAndLoad::gMobjList[idx];
        } else {
            I_Error("SaveData: getMobjAtIdx: bad map object index specified!");
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedSectorT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedSectorT::byteSwap() noexcept {
    byteSwapField(floorheight);
    byteSwapField(ceilingheight);
    byteSwapField(floorpic);
    byteSwapField(ceilingpic);
    byteSwapField(colorid);
    byteSwapField(ceilColorid);
    byteSwapField(lightlevel);
    byteSwapField(special);
    byteSwapField(tag);
    byteSwapField(soundtargetIdx);
    byteSwapField(flags);
    byteSwapField(floorTexOffsetX);
    byteSwapField(floorTexOffsetY);
    byteSwapField(ceilTexOffsetX);
    byteSwapField(ceilTexOffsetY);
}

bool SavedSectorT::validate() const noexcept {
    return (isValidFlatNum(floorpic) && isValidFlatNum(ceilingpic) && isValidMobjIdx(soundtargetIdx));
}

void SavedSectorT::serializeFrom(const sector_t& sector) noexcept {
    floorheight = sector.floorheight;
    ceilingheight = sector.ceilingheight;
    floorpic = (int16_t) sector.floorpic;
    ceilingpic = (int16_t) sector.ceilingpic;
    colorid = (uint8_t) sector.colorid;
    ceilColorid = (uint8_t) sector.ceilColorid;
    lightlevel = sector.lightlevel;
    special = sector.special;
    tag = sector.tag;
    soundtargetIdx = getMobjIndex(sector.soundtarget);
    flags = sector.flags;
    floorTexOffsetX = sector.floorTexOffsetX;
    floorTexOffsetY = sector.floorTexOffsetY;
    ceilTexOffsetX = sector.ceilTexOffsetX;
    ceilTexOffsetY = sector.ceilTexOffsetY;
}

void SavedSectorT::deserializeTo(sector_t& sector) const noexcept {
    // Note: we don't init the thing list deliberately here - that's done elsewhere
    sector.floorheight = floorheight;
    sector.ceilingheight = ceilingheight;
    sector.floorpic = floorpic;
    sector.ceilingpic = ceilingpic;
    sector.colorid = colorid;
    sector.lightlevel = lightlevel;
    sector.special = special;
    sector.tag = tag;
    sector.soundtraversed = 0;
    sector.soundtarget = getMobjAtIdx(soundtargetIdx);
    sector.flags = flags;
    sector.floorDrawHeight = {};                    // Not serialized, default init
    sector.validcount = 0;                          // Not serialized, resets on load
    sector.ceilColorid = ceilColorid;
    sector.lowerColorZ = {};                        // Not serialized, default init
    sector.upperColorZ = {};                        // Not serialized, default init
    sector.shadeHeightDiv = {};                     // Not serialized, default init
    sector.floorTexOffsetX = floorTexOffsetX;
    sector.specialdata = nullptr;                   // Set later once thinkers are deserialized...
    sector.floorTexOffsetY = floorTexOffsetY;
    sector.ceilTexOffsetX = ceilTexOffsetX;
    sector.ceilTexOffsetY = ceilTexOffsetY;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedLineT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedLineT::byteSwap() noexcept {
    byteSwapField(flags);
    byteSwapField(special);
    byteSwapField(tag);
}

void SavedLineT::serializeFrom(const line_t& line) noexcept {
    flags = line.flags;
    special = line.special;
    tag = line.tag;
}

void SavedLineT::deserializeTo(line_t& line) const noexcept {
    line.flags = flags;
    line.special = special;
    line.tag = tag;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedSideT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedSideT::byteSwap() noexcept {
    byteSwapField(textureoffset);
    byteSwapField(rowoffset);
    byteSwapField(toptexture);
    byteSwapField(bottomtexture);
    byteSwapField(midtexture);
}

bool SavedSideT::validate() const noexcept {
    return (isValidTexNum(toptexture) && isValidTexNum(bottomtexture) && isValidTexNum(midtexture));
}

void SavedSideT::serializeFrom(const side_t& side) noexcept {
    textureoffset = side.textureoffset;
    rowoffset = side.rowoffset;
    toptexture = (int16_t) side.toptexture;
    bottomtexture = (int16_t) side.bottomtexture;
    midtexture = (int16_t) side.midtexture;
}

void SavedSideT::deserializeTo(side_t& side) const noexcept {
    side.textureoffset = textureoffset;
    side.rowoffset = rowoffset;
    side.toptexture = toptexture;
    side.bottomtexture = bottomtexture;
    side.midtexture = midtexture;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedMobjT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedMobjT::byteSwap() noexcept {
    byteSwapField(x);
    byteSwapField(y);
    byteSwapField(z);
    byteSwapField(tag);
    byteSwapField(subsectorIdx);
    byteSwapField(angle);
    byteSwapField(sprite);
    byteSwapField(frame);
    byteSwapField(floorz);
    byteSwapField(ceilingz);
    byteSwapField(radius);
    byteSwapField(height);
    byteSwapField(momx);
    byteSwapField(momy);
    byteSwapField(momz);
    byteSwapEnumField(type);
    byteSwapField(tics);
    byteSwapField(stateIdx);
    byteSwapField(flags);
    byteSwapField(health);
    byteSwapEnumField(movedir);
    byteSwapField(movecount);
    byteSwapField(targetIdx);
    byteSwapField(reactiontime);
    byteSwapField(threshold);
    byteSwapField(spawnx);
    byteSwapField(spawny);
    byteSwapField(spawntype);
    byteSwapField(spawnangle);
    byteSwapField(tracerIdx);
}

bool SavedMobjT::validate() const noexcept {
    // Validate most fields that can be directly validated
    const bool bValidFields = (
        isValidSubsectorIdx(subsectorIdx) &&
        isValidSpriteIdx(sprite) &&
        isValidMobjType(type) &&
        isValidStateIdx(stateIdx) &&
        isValidMoveDir(movedir) &&
        isValidMobjIdx(targetIdx) &&
        isValidMobjIdx(tracerIdx)
    );

    if (!bValidFields)
        return false;

    // Validate the sprite frame (once the sprite is confirmed valid)
    const spritedef_t& spriteDef = gSprites[sprite];
    const int32_t sprFrameIdx = frame & FF_FRAMEMASK;
    return ((sprFrameIdx >= 0) && (sprFrameIdx < spriteDef.numframes));
}

void SavedMobjT::serializeFrom(const mobj_t& mobj) noexcept {
    x = mobj.x;
    y = mobj.y;
    z = mobj.z;
    tag = mobj.tag;
    subsectorIdx = (int32_t)(mobj.subsector - gpSubsectors);
    angle = mobj.angle;
    sprite = mobj.sprite;
    frame = mobj.frame;
    floorz = mobj.floorz;
    ceilingz = mobj.ceilingz;
    radius = mobj.radius;
    height = mobj.height;
    momx = mobj.momx;
    momy = mobj.momy;
    momz = mobj.momz;
    type = mobj.type;
    tics = mobj.tics;
    stateIdx = (int32_t)(mobj.state - gStates);
    flags = mobj.flags;
    health = mobj.health;
    movedir = mobj.movedir;
    movecount = mobj.movecount;
    targetIdx = getMobjIndex(mobj.target);
    reactiontime = mobj.reactiontime;
    threshold = mobj.threshold;
    spawnx = mobj.spawnx;
    spawny = mobj.spawny;
    spawntype = mobj.spawntype;
    spawnangle = mobj.spawnangle;
    tracerIdx = getMobjIndex(mobj.tracer);
}

void SavedMobjT::deserializeTo(mobj_t& mobj) const noexcept {
    // Note: not initializing the mobj linked list fields here deliberately; that happens elsewhere
    mobj.x = x;
    mobj.y = y;
    mobj.z = z;
    mobj.tag = tag;
    mobj.subsector = &gpSubsectors[subsectorIdx];
    mobj.latecall = {};         // Not serialized, default init
    mobj.angle = angle;
    mobj.sprite = sprite;
    mobj.frame = frame;
    mobj.floorz = floorz;
    mobj.ceilingz = ceilingz;
    mobj.radius = radius;
    mobj.height = height;
    mobj.momx = momx;
    mobj.momy = momy;
    mobj.momz = momz;
    mobj.type = type;
    mobj.info = &gMobjInfo[type];
    mobj.tics = tics;
    mobj.state = &gStates[stateIdx];
    mobj.flags = flags;
    mobj.health = health;
    mobj.movedir = movedir;
    mobj.movecount = movecount;
    mobj.target = getMobjAtIdx(targetIdx);
    mobj.reactiontime = reactiontime;
    mobj.threshold = threshold;
    mobj.player = nullptr;      // Initialized later
    mobj.extradata = {};        // Not serialized, default init
    mobj.spawnx = spawnx;
    mobj.spawny = spawny;
    mobj.spawntype = spawntype;
    mobj.spawnangle = spawnangle;
    mobj.tracer = getMobjAtIdx(tracerIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedPspdefT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedPspdefT::byteSwap() noexcept {
    byteSwapField(stateIdx);
    byteSwapField(tics);
    byteSwapField(sx);
    byteSwapField(sy);
}

bool SavedPspdefT::validate() const noexcept {
    return isValidStateIdx(stateIdx);
}

void SavedPspdefT::serializeFrom(const pspdef_t& spr) noexcept {
    stateIdx = (int32_t)(spr.state - gStates);
    tics = spr.tics;
    sx = spr.sx;
    sy = spr.sy;
}

void SavedPspdefT::deserializeTo(pspdef_t& spr) const noexcept {
    spr.state = &gStates[stateIdx];
    spr.tics = tics;
    spr.sx = sx;
    spr.sy = sy;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedPlayerT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedPlayerT::byteSwap() noexcept {
    byteSwapField(mobjIdx);
    byteSwapEnumField(playerstate);
    byteSwapField(forwardmove);
    byteSwapField(sidemove);
    byteSwapField(angleturn);
    byteSwapField(viewz);
    byteSwapField(viewheight);
    byteSwapField(deltaviewheight);
    byteSwapField(bob);
    byteSwapField(health);
    byteSwapField(armorpoints);
    byteSwapField(armortype);

    for (int32_t& powerAmt : powers) {
        byteSwapField(powerAmt);
    }

    for (bool& bHaveCard : cards) {
        byteSwapField(bHaveCard);
    }

    byteSwapField(backpack);
    byteSwapEnumField(readyweapon);
    byteSwapEnumField(pendingweapon);

    for (bool& bWeaponOwned : weaponowned) {
        byteSwapField(bWeaponOwned);
    }

    for (int32_t& ammoAmt : ammo) {
        byteSwapField(ammoAmt);
    }

    for (int32_t& maxAmmoAmt : maxammo) {
        byteSwapField(maxAmmoAmt);
    }

    byteSwapField(cheats);
    byteSwapField(killcount);
    byteSwapField(itemcount);
    byteSwapField(secretcount);
    byteSwapField(damagecount);
    byteSwapField(bonuscount);
    byteSwapField(attackerIdx);
    byteSwapField(extralight);

    for (SavedPspdefT& spr : psprites) {
        spr.byteSwap();
    }
}

bool SavedPlayerT::validate() const noexcept {
    if (!isValidMobjIdx(mobjIdx))
        return false;

    for (const SavedPspdefT& spr : psprites) {
        if (!spr.validate())
            return false;
    }

    return true;
}

void SavedPlayerT::serializeFrom(const player_t& player) noexcept {
    mobjIdx = getMobjIndex(player.mo);
    playerstate = player.playerstate;
    forwardmove = player.forwardmove;
    sidemove = player.sidemove;
    angleturn = player.angleturn;
    viewz = player.viewz;
    viewheight = player.viewheight;
    deltaviewheight = player.deltaviewheight;
    bob = player.bob;
    health = player.health;
    armorpoints = player.armorpoints;
    armortype = player.armortype;

    for (uint32_t i = 0; i < C_ARRAY_SIZE(powers); ++i) {
        powers[i] = player.powers[i];
    }

    for (uint32_t i = 0; i < C_ARRAY_SIZE(cards); ++i) {
        cards[i] = player.cards[i];
    }

    backpack = player.backpack;
    readyweapon = player.readyweapon;
    pendingweapon = player.pendingweapon;

    for (uint32_t i = 0; i < C_ARRAY_SIZE(weaponowned); ++i) {
        weaponowned[i] = player.weaponowned[i];
    }

    for (uint32_t i = 0; i < C_ARRAY_SIZE(ammo); ++i) {
        ammo[i] = player.ammo[i];
    }

    for (uint32_t i = 0; i < C_ARRAY_SIZE(maxammo); ++i) {
        maxammo[i] = player.maxammo[i];
    }

    cheats = player.cheats;
    killcount = player.killcount;
    itemcount = player.itemcount;
    secretcount = player.secretcount;
    damagecount = player.damagecount;
    bonuscount = player.bonuscount;
    attackerIdx = getMobjIndex(player.attacker);
    extralight = player.extralight;

    for (uint32_t i = 0; i < C_ARRAY_SIZE(psprites); ++i) {
        psprites[i].serializeFrom(player.psprites[i]);
    }
}

void SavedPlayerT::deserializeTo(player_t& player) const noexcept {
    player.mo = getMobjAtIdx(mobjIdx);
    player.playerstate = playerstate;
    player.forwardmove = forwardmove;
    player.sidemove = sidemove;
    player.angleturn = angleturn;
    player.viewz = viewz;
    player.viewheight = viewheight;
    player.deltaviewheight = deltaviewheight;
    player.bob = bob;
    player.health = health;
    player.armorpoints = armorpoints;
    player.armortype = armortype;

    for (uint32_t i = 0; i < C_ARRAY_SIZE(powers); ++i) {
        player.powers[i] = powers[i];
    }

    for (uint32_t i = 0; i < C_ARRAY_SIZE(cards); ++i) {
        player.cards[i] = cards[i];
    }

    player.backpack = backpack;
    player.frags = {};                      // Not serialized, default init
    player._unused = {};                    // Not serialized, default init
    player.readyweapon = readyweapon;
    player.pendingweapon = pendingweapon;

    for (uint32_t i = 0; i < C_ARRAY_SIZE(weaponowned); ++i) {
        player.weaponowned[i] = weaponowned[i];
    }

    for (uint32_t i = 0; i < C_ARRAY_SIZE(ammo); ++i) {
        player.ammo[i] = ammo[i];
    }

    for (uint32_t i = 0; i < C_ARRAY_SIZE(maxammo); ++i) {
        player.maxammo[i] = maxammo[i];
    }

    player.attackdown = 0;                  // Not serialized, default init
    player.usedown = false;                 // Not serialized, default init
    player.cheats = cheats;
    player.refire = 0;                      // Not serialized, default init
    player.killcount = killcount;
    player.itemcount = itemcount;
    player.secretcount = secretcount;
    player.message = nullptr;               // Not serialized, default init
    player.damagecount = damagecount;
    player.bonuscount = bonuscount;
    player.attacker = getMobjAtIdx(attackerIdx);
    player.extralight = extralight;
    player.fixedcolormap = {};              // Not serialized, default init
    player.colormap = {};                   // Not serialized, default init

    for (uint32_t i = 0; i < C_ARRAY_SIZE(psprites); ++i) {
        psprites[i].deserializeTo(player.psprites[i]);
    }

    player.didsecret = {};                  // Not serialized, default init
    player.lastsoundsector = nullptr;       // Not serialized, default init
    player.automapx = {};                   // Not serialized, default init
    player.automapy = {};                   // Not serialized, default init
    player.automapscale = {};               // Not serialized, default init
    player.automapflags = {};               // Not serialized, default init
    player.turnheld = {};                   // Not serialized, default init
    player.psxMouseUseCountdown = {};       // Not serialized, default init
    player.psxMouseUse = {};                // Not serialized, default init
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedVLDoorT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedVLDoorT::byteSwap() noexcept {
    byteSwapEnumField(type);
    byteSwapField(sectorIdx);
    byteSwapField(topheight);
    byteSwapField(speed);
    byteSwapField(direction);
    byteSwapField(topwait);
    byteSwapField(topcountdown);
}

bool SavedVLDoorT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedVLDoorT::serializeFrom(const vldoor_t& door) noexcept {
    type = door.type;
    sectorIdx = (int32_t)(door.sector - gpSectors);
    topheight = door.topheight;
    speed = door.speed;
    direction = door.direction;
    topwait = door.topwait;
    topcountdown = door.topcountdown;
}

void SavedVLDoorT::deserializeTo(vldoor_t& door) const noexcept {
    door.thinker.function = (think_t) &T_VerticalDoor;
    door.type = type;
    door.sector = &gpSectors[sectorIdx];
    door.topheight = topheight;
    door.speed = speed;
    door.direction = direction;
    door.topwait = topwait;
    door.topcountdown = topcountdown;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedVLCustomdoorT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedVLCustomdoorT::byteSwap() noexcept {
    byteSwapField(sectorIdx);
    byteSwapField(def.minHeight);
    byteSwapField(def.maxHeight);
    byteSwapField(def.speed);
    byteSwapField(def.waitTime);
    byteSwapEnumField(def.openSound);
    byteSwapEnumField(def.closeSound);
    byteSwapField(def.finishScriptActionNum);
    byteSwapField(def.finishScriptUserdata);
    byteSwapField(direction);
    byteSwapField(postWaitDirection);
    byteSwapField(countdown);
}

bool SavedVLCustomdoorT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedVLCustomdoorT::serializeFrom(const vlcustomdoor_t& door) noexcept {
    sectorIdx = (int32_t)(door.sector - gpSectors);
    def = door.def;
    direction = door.direction;
    postWaitDirection = door.postWaitDirection;
    countdown = door.countdown;
}

void SavedVLCustomdoorT::deserializeTo(vlcustomdoor_t& door) const noexcept {
    door.thinker.function = (think_t) &T_CustomDoor;
    door.sector = &gpSectors[sectorIdx];
    door.def = def;
    door.direction = direction;
    door.postWaitDirection = postWaitDirection;
    door.countdown = countdown;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedFloorMoveT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedFloorMoveT::byteSwap() noexcept {
    byteSwapEnumField(type);
    byteSwapField(crush);
    byteSwapField(bDoFinishScript);
    byteSwapField(sectorIdx);
    byteSwapField(direction);
    byteSwapField(newspecial);
    byteSwapField(texture);
    byteSwapField(floordestheight);
    byteSwapField(speed);
    byteSwapEnumField(moveSound);
    byteSwapField(moveSoundFreq);
    byteSwapEnumField(stopSound);
    byteSwapField(finishScriptActionNum);
    byteSwapField(finishScriptUserdata);
}

bool SavedFloorMoveT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedFloorMoveT::serializeFrom(const floormove_t& floor) noexcept {
    type = floor.type;
    crush = floor.crush;
    bDoFinishScript = floor.bDoFinishScript;
    sectorIdx = (int32_t)(floor.sector - gpSectors);
    direction = floor.direction;
    newspecial = floor.newspecial;
    texture = floor.texture;
    floordestheight = floor.floordestheight;
    speed = floor.speed;
    moveSound = floor.moveSound;
    moveSoundFreq = floor.moveSoundFreq;
    stopSound = floor.stopSound;
    finishScriptActionNum = floor.finishScriptActionNum;
    finishScriptUserdata = floor.finishScriptUserdata;
}

void SavedFloorMoveT::deserializeTo(floormove_t& floor) const noexcept {
    floor.thinker.function = (think_t) &T_MoveFloor;
    floor.type = type;
    floor.crush = crush;
    floor.bDoFinishScript = bDoFinishScript;
    floor.sector = &gpSectors[sectorIdx];
    floor.direction = direction;
    floor.newspecial = newspecial;
    floor.texture = texture;
    floor.floordestheight = floordestheight;
    floor.speed = speed;
    floor.moveSound = moveSound;
    floor.moveSoundFreq = moveSoundFreq;
    floor.stopSound = stopSound;
    floor.finishScriptActionNum = finishScriptActionNum;
    floor.finishScriptUserdata = finishScriptUserdata;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedCeilingT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedCeilingT::byteSwap() noexcept {
    byteSwapEnumField(type);
    byteSwapField(sectorIdx);
    byteSwapField(bottomheight);
    byteSwapField(topheight);
    byteSwapField(speed);
    byteSwapField(crush);
    byteSwapField(bIsCrushing);
    byteSwapField(bDoFinishScript);
    byteSwapField(bIsActive);
    byteSwapField(direction);
    byteSwapField(tag);
    byteSwapField(olddirection);
    byteSwapField(crushSpeed);
    byteSwapField(dirChangesLeft);
    byteSwapEnumField(moveSound);
    byteSwapField(moveSoundFreq);
    byteSwapEnumField(changeDirSound);
    byteSwapEnumField(stopSound);
    byteSwapField(finishScriptActionNum);
    byteSwapField(finishScriptUserdata);
}

bool SavedCeilingT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedCeilingT::serializeFrom(const ceiling_t& ceil) noexcept {
    type = ceil.type;
    sectorIdx = (int32_t)(ceil.sector - gpSectors);
    bottomheight = ceil.bottomheight;
    topheight = ceil.topheight;
    speed = ceil.speed;
    crush = ceil.crush;
    bIsCrushing = ceil.bIsCrushing;
    bDoFinishScript = ceil.bDoFinishScript;
    bIsActive = isActiveCeil(ceil);
    direction = ceil.direction;
    tag = ceil.tag;
    olddirection = ceil.olddirection;
    crushSpeed = ceil.crushSpeed;
    dirChangesLeft = ceil.dirChangesLeft;
    moveSound = ceil.moveSound;
    moveSoundFreq = ceil.moveSoundFreq;
    changeDirSound = ceil.changeDirSound;
    stopSound = ceil.stopSound;
    finishScriptActionNum = ceil.finishScriptActionNum;
    finishScriptUserdata = ceil.finishScriptUserdata;
}

void SavedCeilingT::deserializeTo(ceiling_t& ceil) const noexcept {
    ceil.thinker.function = (think_t) &T_MoveCeiling;
    ceil.type = type;
    ceil.sector = &gpSectors[sectorIdx];
    ceil.bottomheight = bottomheight;
    ceil.topheight = topheight;
    ceil.speed = speed;
    ceil.crush = crush;
    ceil.bIsCrushing = bIsCrushing;
    ceil.bDoFinishScript = bDoFinishScript;
    ceil.direction = direction;
    ceil.tag = tag;
    ceil.olddirection = olddirection;
    ceil.crushSpeed = crushSpeed;
    ceil.dirChangesLeft = dirChangesLeft;
    ceil.moveSound = moveSound;
    ceil.moveSoundFreq = moveSoundFreq;
    ceil.changeDirSound = changeDirSound;
    ceil.stopSound = stopSound;
    ceil.finishScriptActionNum = finishScriptActionNum;
    ceil.finishScriptUserdata = finishScriptUserdata;

    // TODO: REMOVE - handle this in the load/save module.
    if (bIsActive) {
        P_AddActiveCeiling(ceil);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedPlatT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedPlatT::byteSwap() noexcept {
    byteSwapField(sectorIdx);
    byteSwapField(speed);
    byteSwapField(low);
    byteSwapField(high);
    byteSwapField(wait);
    byteSwapField(count);
    byteSwapEnumField(status);
    byteSwapEnumField(oldstatus);
    byteSwapField(crush);
    byteSwapField(bDoFinishScript);
    byteSwapField(bIsActive);
    byteSwapField(tag);
    byteSwapEnumField(type);
    byteSwapEnumField(finishState);
    byteSwapEnumField(startSound);
    byteSwapEnumField(moveSound);
    byteSwapField(moveSoundFreq);
    byteSwapEnumField(stopSound);
    byteSwapField(finishScriptActionNum);
    byteSwapField(finishScriptUserdata);
}

bool SavedPlatT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedPlatT::serializeFrom(const plat_t& plat) noexcept {
    sectorIdx = (int32_t)(plat.sector - gpSectors);
    speed = plat.speed;
    low = plat.low;
    high = plat.high;
    wait = plat.wait;
    count = plat.count;
    status = plat.status;
    oldstatus = plat.oldstatus;
    crush = plat.crush;
    bDoFinishScript = plat.bDoFinishScript;
    bIsActive = isActivePlat(plat);
    tag = plat.tag;
    type = plat.type;
    finishState = plat.finishState;
    startSound = plat.startSound;
    moveSound = plat.moveSound;
    moveSoundFreq = plat.moveSoundFreq;
    stopSound = plat.stopSound;
    finishScriptActionNum = plat.finishScriptActionNum;
    finishScriptUserdata = plat.finishScriptUserdata;
}

void SavedPlatT::deserializeTo(plat_t& plat) const noexcept {
    plat.thinker.function = (think_t) &T_PlatRaise;
    plat.sector = &gpSectors[sectorIdx];
    plat.speed = speed;
    plat.low = low;
    plat.high = high;
    plat.wait = wait;
    plat.count = count;
    plat.status = status;
    plat.oldstatus = oldstatus;
    plat.crush = crush;
    plat.bDoFinishScript = bDoFinishScript;
    plat.tag = tag;
    plat.type = type;
    plat.finishState = finishState;
    plat.startSound = startSound;
    plat.moveSound = moveSound;
    plat.moveSoundFreq = moveSoundFreq;
    plat.stopSound = stopSound;
    plat.finishScriptActionNum = finishScriptActionNum;
    plat.finishScriptUserdata = finishScriptUserdata;

    // TODO: REMOVE - handle this in the load/save module.
    if (bIsActive) {
        P_AddActivePlat(plat);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedFireFlickerT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedFireFlickerT::byteSwap() noexcept {
    byteSwapField(sectorIdx);
    byteSwapField(count);
    byteSwapField(maxlight);
    byteSwapField(minlight);
}

bool SavedFireFlickerT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedFireFlickerT::serializeFrom(const fireflicker_t& light) noexcept {
    sectorIdx = (int32_t)(light.sector - gpSectors);
    count = light.count;
    maxlight = light.maxlight;
    minlight = light.minlight;
}

void SavedFireFlickerT::deserializeTo(fireflicker_t& light) const noexcept {
    light.thinker.function = (think_t) &T_FireFlicker;
    light.sector = &gpSectors[sectorIdx];
    light.count = count;
    light.maxlight = maxlight;
    light.minlight = minlight;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedLightFlashT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedLightFlashT::byteSwap() noexcept {
    byteSwapField(sectorIdx);
    byteSwapField(count);
    byteSwapField(maxlight);
    byteSwapField(minlight);
    byteSwapField(maxtime);
    byteSwapField(mintime);
}

bool SavedLightFlashT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedLightFlashT::serializeFrom(const lightflash_t& light) noexcept {
    sectorIdx = (int32_t)(light.sector - gpSectors);
    count = light.count;
    maxlight = light.maxlight;
    minlight = light.minlight;
    maxtime = light.maxtime;
    mintime = light.mintime;
}

void SavedLightFlashT::deserializeTo(lightflash_t& light) const noexcept {
    light.thinker.function = (think_t) &T_LightFlash;
    light.sector = &gpSectors[sectorIdx];
    light.count = count;
    light.maxlight = maxlight;
    light.minlight = minlight;
    light.maxtime = maxtime;
    light.mintime = mintime;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedStrobeT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedStrobeT::byteSwap() noexcept {
    byteSwapField(sectorIdx);
    byteSwapField(count);
    byteSwapField(minlight);
    byteSwapField(maxlight);
    byteSwapField(darktime);
    byteSwapField(brighttime);
}

bool SavedStrobeT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedStrobeT::serializeFrom(const strobe_t& light) noexcept {
    sectorIdx = (int32_t)(light.sector - gpSectors);
    count = light.count;
    minlight = light.minlight;
    maxlight = light.maxlight;
    darktime = light.darktime;
    brighttime = light.brighttime;
}

void SavedStrobeT::deserializeTo(strobe_t& light) const noexcept {
    light.thinker.function = (think_t) &T_StrobeFlash;
    light.sector = &gpSectors[sectorIdx];
    light.count = count;
    light.minlight = minlight;
    light.maxlight = maxlight;
    light.darktime = darktime;
    light.brighttime = brighttime;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedGlowT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedGlowT::byteSwap() noexcept {
    byteSwapField(sectorIdx);
    byteSwapField(minlight);
    byteSwapField(maxlight);
    byteSwapField(direction);
}

bool SavedGlowT::validate() const noexcept {
    return isValidSectorIdx(sectorIdx);
}

void SavedGlowT::serializeFrom(const glow_t& light) noexcept {
    sectorIdx = (int32_t)(light.sector - gpSectors);
    minlight = light.minlight;
    maxlight = light.maxlight;
    direction = light.direction;
}

void SavedGlowT::deserializeTo(glow_t& light) const noexcept {
    light.thinker.function = (think_t) &T_Glow;
    light.sector = &gpSectors[sectorIdx];
    light.minlight = minlight;
    light.maxlight = maxlight;
    light.direction = direction;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedDelayedExitT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedDelayedExitT::byteSwap() noexcept {
    byteSwapField(ticsleft);
}

void SavedDelayedExitT::serializeFrom(const delayaction_t& action) noexcept {
    ticsleft = action.ticsleft;
}

void SavedDelayedExitT::deserializeTo(delayaction_t& action) const noexcept {
    action.thinker.function = (think_t) &T_DelayedAction;
    action.ticsleft = ticsleft;
    action.actionfunc = &G_CompleteLevel;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedButtonT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedButtonT::byteSwap() noexcept {
    byteSwapField(lineIdx);
    byteSwapEnumField(where);
    byteSwapField(btexture);
    byteSwapField(btimer);
}

bool SavedButtonT::validate() const noexcept {
    return (isValidLineIdx(lineIdx) && isValidTexNum(btexture));
}

void SavedButtonT::serializeFrom(const button_t& btn) noexcept {
    lineIdx = (int32_t)(btn.line - gpLines);
    where = btn.where;
    btexture = btn.btexture;
    btimer = btn.btimer;
}

void SavedButtonT::deserializeTo(button_t& btn) const noexcept {
    line_t& line = gpLines[lineIdx];

    btn.line = &line;
    btn.where = where;
    btn.btexture = btexture;
    btn.btimer = btimer;
    btn.soundorg = (mobj_t*) &line.frontsector->soundorg;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedScheduledAction
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedScheduledAction::byteSwap() noexcept {
    byteSwapField(actionNum);
    byteSwapField(delayTics);
    byteSwapField(executionsLeft);
    byteSwapField(repeatDelay);
    byteSwapField(tag);
    byteSwapField(userdata);
    byteSwapField(bPaused);
    byteSwapField(bPendingExecute);
}

void SavedScheduledAction::serializeFrom(const ScriptingEngine::ScheduledAction& action) noexcept {
    actionNum = action.actionNum;
    delayTics = action.delayTics;
    executionsLeft = action.executionsLeft;
    repeatDelay = action.repeatDelay;
    tag = action.tag;
    userdata = action.userdata;
    bPaused = action.bPaused;
    bPendingExecute = action.bPendingExecute;
}

void SavedScheduledAction::deserializeTo(ScriptingEngine::ScheduledAction& action) const noexcept {
    action.actionNum = actionNum;
    action.delayTics = delayTics;
    action.executionsLeft = executionsLeft;
    action.repeatDelay = repeatDelay;
    action.tag = tag;
    action.userdata = userdata;
    action.bPaused = bPaused;
    action.bPendingExecute = bPendingExecute;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedSTBarT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedSTBarT::byteSwap() noexcept {
    byteSwapField(face);
    byteSwapEnumField(specialFace);
    byteSwapField(gotgibbed);
    byteSwapField(gibframe);
    byteSwapField(gibframeTicsLeft);
    byteSwapField(alertMessageTicsLeft);
}

bool SavedSTBarT::validate() const noexcept {
    return ((face < NUMFACES) && (specialFace < NUMSPCLFACES));
}

void SavedSTBarT::serializeFrom(const stbar_t& sbar) noexcept {
    face = sbar.face;
    specialFace = sbar.specialFace;
    gotgibbed = sbar.gotgibbed;
    gibframe = sbar.gibframe;
    gibframeTicsLeft = sbar.gibframeTicsLeft;
    std::memcpy(alertMessage, sbar.alertMessage, sizeof(sbar.alertMessage));
    alertMessageTicsLeft = sbar.alertMessageTicsLeft;
}

void SavedSTBarT::deserializeTo(stbar_t& sbar) const noexcept {
    sbar.face = face;
    sbar.specialFace = specialFace;
    std::memset(sbar.tryopen, 0, sizeof(sbar.tryopen));     // Not serialized, default init
    sbar.gotgibbed = gotgibbed;
    sbar.gibframe = gibframe;
    sbar.gibframeTicsLeft = gibframeTicsLeft;
    sbar.message = nullptr;         // Not serialized, default init
    sbar.messageTicsLeft = 0;       // Not serialized, default init
    std::memcpy(sbar.alertMessage, alertMessage, sizeof(sbar.alertMessage));
    sbar.alertMessageTicsLeft = alertMessageTicsLeft;

    // This global is redundant, hence not serialized in 'SavedGlobals'
    gpCurSBFaceSprite = &gFaceSprites[face];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedGlobals
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedGlobals::byteSwap() noexcept {
    byteSwapField(gameMap);
    byteSwapField(nextMap);
    byteSwapEnumField(gameSkill);
    byteSwapField(levelElapsedTime);
    byteSwapField(totalKills);
    byteSwapField(totalItems);
    byteSwapField(totalSecret);
    player.byteSwap();
    byteSwapField(prndIndex);
    byteSwapField(mrndIndex);
    byteSwapField(gameTic);
    byteSwapField(ticCon);
    byteSwapField(ticRemainder);
    byteSwapField(mapBossSpecialFlags);
    byteSwapField(extCameraTicsLeft);
    byteSwapField(extCameraX);
    byteSwapField(extCameraY);
    byteSwapField(extCameraZ);
    byteSwapField(extCameraAngle);
    byteSwapField(numPasswordCharsEntered);
    byteSwapField(curCDTrack);
    statusBar.byteSwap();
    byteSwapField(faceTics);
    byteSwapField(bDrawSBFace);
    byteSwapField(bGibDraw);
    byteSwapField(bDoSpclFace);
    byteSwapField(newFace);
    byteSwapEnumField(spclFaceType);
}

bool SavedGlobals::validate() const noexcept {
    if ((gameMap < 1) || (gameMap > Game::getNumMaps()))
        return false;

    if ((gameSkill < sk_baby) || (gameSkill >= NUMSKILLS))
        return false;

    if (levelElapsedTime < 0)
        return false;

    if ((totalKills < 0) || (totalItems < 0) || (totalSecret < 0))
        return false;

    if (!player.validate())
        return false;

    if ((numPasswordCharsEntered < 0) || (numPasswordCharsEntered > C_ARRAY_SIZE(passwordCharBuffer)))
        return false;

    if (!statusBar.validate())
        return false;

    return true;
}

void SavedGlobals::serializeFromGlobals() noexcept {
    gameMap = gGameMap;
    nextMap = gNextMap;
    gameSkill = gGameSkill;
    levelElapsedTime = Game::getLevelElapsedTimeMicrosecs();
    totalKills = gTotalKills;
    totalItems = gTotalItems;
    totalSecret = gTotalSecret;

    player.serializeFrom(gPlayers[0]);
    
    prndIndex = gPRndIndex;
    mrndIndex = gMRndIndex;
    gameTic = gGameTic;
    ticCon = gTicCon;
    
    ticRemainder = gTicRemainder[0];
    
    mapBossSpecialFlags = gMapBossSpecialFlags;
    extCameraTicsLeft = gExtCameraTicsLeft;
    extCameraX = gExtCameraX;
    extCameraY = gExtCameraY;
    extCameraZ = gExtCameraZ;
    extCameraAngle = gExtCameraAngle;
    numPasswordCharsEntered = gNumPasswordCharsEntered;
    std::memcpy(passwordCharBuffer, gPasswordCharBuffer, sizeof(passwordCharBuffer));
    curCDTrack = psxcd_get_playing_track();
    statusBar.serializeFrom(gStatusBar);
    faceTics = gFaceTics;
    bDrawSBFace = gbDrawSBFace;
    bGibDraw = gbGibDraw;
    bDoSpclFace = gbDoSpclFace;
    newFace = gNewFace;
    spclFaceType = gSpclFaceType;
}

void SavedGlobals::deserializeToGlobals() const noexcept {
    // Handle the globals we serialize
    gGameMap = gameMap;
    gNextMap = nextMap;
    gGameSkill = gameSkill;
    Game::setLevelElapsedTimeMicrosecs(levelElapsedTime);
    gTotalKills = totalKills;
    gTotalItems = totalItems;
    gTotalSecret = totalSecret;
    
    std::memset(gPlayers, 0, sizeof(gPlayers));     // Zero-init the bits we don't use
    player.deserializeTo(gPlayers[0]);;
    
    gPRndIndex = prndIndex;
    gMRndIndex = mrndIndex;
    gGameTic = gameTic;
    gTicCon = ticCon;

    std::memset(gTicRemainder, 0, sizeof(gTicRemainder));   // Zero-init the bits we don't use
    gTicRemainder[0] = ticRemainder;

    gMapBossSpecialFlags = mapBossSpecialFlags;
    gExtCameraTicsLeft = extCameraTicsLeft;
    gExtCameraX = extCameraX;
    gExtCameraY = extCameraY;
    gExtCameraZ = extCameraZ;
    gExtCameraAngle = extCameraAngle;
    gNumPasswordCharsEntered = numPasswordCharsEntered;
    std::memcpy(gPasswordCharBuffer, passwordCharBuffer, sizeof(passwordCharBuffer));
    statusBar.deserializeTo(gStatusBar);
    gpCurSBFaceSprite = &gFaceSprites[statusBar.face];      // This redundant info is not serialized, derived from 'statusBar' instead
    gFaceTics = faceTics;
    gbDrawSBFace = bDrawSBFace;
    gbGibDraw = bGibDraw;
    gbDoSpclFace = bDoSpclFace;
    gNewFace = newFace;
    gSpclFaceType = spclFaceType;

    // Switch to playing CD music if not already playing it
    // TODO: REMOVE - handle this in the load/save module.
    if (curCDTrack > 0) {
        const int32_t playingCdTrack = psxcd_get_playing_track();

        if (playingCdTrack != curCDTrack) {
            S_StopMusic();
            psxcd_play_at_andloop(curCDTrack, gCdMusicVol, 0, 0, curCDTrack, gCdMusicVol, 0, 0);
        }
    }

    // Default init the item respawn queue and dead player removal queue.
    // Don't serialize these because the save file is for single player only...
    gItemRespawnQueueHead = 0;
    gItemRespawnQueueTail = 0;
    std::memset(gItemRespawnTime, 0, sizeof(gItemRespawnTime));
    std::memset(gItemRespawnQueue, 0, sizeof(gItemRespawnQueue));

    gDeadPlayerRemovalQueueIdx = 0;
    std::memset(gDeadPlayerMobjRemovalQueue, 0, sizeof(gDeadPlayerMobjRemovalQueue));
    
    // There's just one player in the game and it's single player
    std::memset(gbPlayerInGame, 0, sizeof(gbPlayerInGame));
    gbPlayerInGame[0] = true;
    gNetGame = gt_single;
    
    // No card flashes
    std::memset(gFlashCards, 0, sizeof(gFlashCards));

    // Default init some timing related stuff
    const int32_t totalVBlanks = I_GetTotalVBlanks();
    const int32_t tgtGameTicCount = (Game::gSettings.bUsePalTimings) ? ticCon / 3 : d_rshift<VBLANK_TO_TIC_SHIFT>(ticCon);

    gPrevGameTic = gameTic;
    gLastTotalVBlanks = totalVBlanks;
    gTotalVBlanks = totalVBlanks;
    gElapsedVBlanks = 0;
    std::memset(gPlayersElapsedVBlanks, 0, sizeof(gPlayersElapsedVBlanks));
    gbIsFirstTick = false;
    gLastTgtGameTicCount = tgtGameTicCount;

    // The count marker gets reset after deserializing
    gValidCount = 0;

    // Clear these for good measure, deserializing an active ceiling or plat will add to these lists
    // TODO: REMOVE - handle this in the load/save module.
    gpActiveCeilings.clear();
    gpActivePlats.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SaveFileHdr
//------------------------------------------------------------------------------------------------------------------------------------------
void SaveFileHdr::byteSwap() noexcept {
    byteSwapField(fileId1);
    byteSwapField(fileId2);
    byteSwapField(version);
    byteSwapField(_reserved1);
    byteSwapField(mapHashWord1);
    byteSwapField(mapHashWord2);
    byteSwapField(numSectors);
    byteSwapField(numLines);
    byteSwapField(numSides);
    byteSwapField(numMobjs);
    byteSwapField(numVlDoors);
    byteSwapField(numVlCustomDoors);
    byteSwapField(numFloorMovers);
    byteSwapField(numCeilings);
    byteSwapField(numPlats);
    byteSwapField(numFireFlickers);
    byteSwapField(numLightFlashes);
    byteSwapField(numStrobes);
    byteSwapField(numGlows);
    byteSwapField(numDelayedExits);
    byteSwapField(numButtons);
    byteSwapField(numScheduledActions);
}

bool SaveFileHdr::validateFileId() const noexcept {
    return ((fileId1 == SAVE_FILE_ID1) && (fileId2 == SAVE_FILE_ID2));
}

bool SaveFileHdr::validateVersion() const noexcept {
    return (version == SAVE_FILE_VERSION);
}

bool SaveFileHdr::validateMapHash() const noexcept {
    return ((mapHashWord1 == MapHash::gWord1) && (mapHashWord2 == MapHash::gWord2));
}

bool SaveFileHdr::validate() const noexcept {
    return (
        validateFileId() &&
        validateVersion() &&
        validateMapHash() &&
        // Sanity these counts match the map
        ((int32_t) numSectors == gNumSectors) &&
        ((int32_t) numLines == gNumLines) &&
        ((int32_t) numSides == gNumSides)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SaveData
//------------------------------------------------------------------------------------------------------------------------------------------
void SaveData::byteSwap() noexcept {
    hdr.byteSwap();
    globals.byteSwap();
    byteSwapObjects(sectors.get(), hdr.numSectors);
    byteSwapObjects(lines.get(), hdr.numLines);
    byteSwapObjects(sides.get(), hdr.numSides);
    byteSwapObjects(mobjs.get(), hdr.numMobjs);
    byteSwapObjects(vlDoors.get(), hdr.numVlDoors);
    byteSwapObjects(vlCustomDoors.get(), hdr.numVlCustomDoors);
    byteSwapObjects(floorMovers.get(), hdr.numFloorMovers);
    byteSwapObjects(ceilings.get(), hdr.numCeilings);
    byteSwapObjects(plats.get(), hdr.numPlats);
    byteSwapObjects(fireFlickers.get(), hdr.numFireFlickers);
    byteSwapObjects(lightFlashes.get(), hdr.numLightFlashes);
    byteSwapObjects(strobes.get(), hdr.numStrobes);
    byteSwapObjects(glows.get(), hdr.numGlows);
    byteSwapObjects(delayedExits.get(), hdr.numDelayedExits);
    byteSwapObjects(buttons.get(), hdr.numButtons);
    byteSwapObjects(scheduledActions.get(), hdr.numScheduledActions);
}

bool SaveData::writeTo(OutputStream& out) const noexcept {
    try {
        out.write(hdr);
        out.write(globals);
        out.writeArray(sectors.get(), hdr.numSectors);
        out.writeArray(lines.get(), hdr.numLines);
        out.writeArray(sides.get(), hdr.numSides);
        out.writeArray(mobjs.get(), hdr.numMobjs);
        out.writeArray(vlDoors.get(), hdr.numVlDoors);
        out.writeArray(vlCustomDoors.get(), hdr.numVlCustomDoors);
        out.writeArray(floorMovers.get(), hdr.numFloorMovers);
        out.writeArray(ceilings.get(), hdr.numCeilings);
        out.writeArray(plats.get(), hdr.numPlats);
        out.writeArray(fireFlickers.get(), hdr.numFireFlickers);
        out.writeArray(lightFlashes.get(), hdr.numLightFlashes);
        out.writeArray(strobes.get(), hdr.numStrobes);
        out.writeArray(glows.get(), hdr.numGlows);
        out.writeArray(delayedExits.get(), hdr.numDelayedExits);
        out.writeArray(buttons.get(), hdr.numButtons);
        out.writeArray(scheduledActions.get(), hdr.numScheduledActions);
        return true;
    }
    catch (...) {
        return false;
    }
}

SaveData::ReadFromFileResult SaveData::readFrom(InputStream& in) noexcept {
    try {
        // Read the header first and do basic validity checks
        in.read(hdr);

        if (!hdr.validateFileId())
            return ReadFromFileResult::BAD_FILE_ID;

        if (!hdr.validateVersion())
            return ReadFromFileResult::BAD_VERSION;

        // Read everything else
        in.read(globals);
        readArray(in, sectors, hdr.numSectors);
        readArray(in, lines, hdr.numLines);
        readArray(in, sides, hdr.numSides);
        readArray(in, mobjs, hdr.numMobjs);
        readArray(in, vlDoors, hdr.numVlDoors);
        readArray(in, vlCustomDoors, hdr.numVlCustomDoors);
        readArray(in, floorMovers, hdr.numFloorMovers);
        readArray(in, ceilings, hdr.numCeilings);
        readArray(in, plats, hdr.numPlats);
        readArray(in, fireFlickers, hdr.numFireFlickers);
        readArray(in, lightFlashes, hdr.numLightFlashes);
        readArray(in, strobes, hdr.numStrobes);
        readArray(in, glows, hdr.numGlows);
        readArray(in, delayedExits, hdr.numDelayedExits);
        readArray(in, buttons, hdr.numButtons);
        readArray(in, scheduledActions, hdr.numScheduledActions);
        return ReadFromFileResult::OK;
    }
    catch (...) {
        return ReadFromFileResult::READ_ERROR;
    }
}
