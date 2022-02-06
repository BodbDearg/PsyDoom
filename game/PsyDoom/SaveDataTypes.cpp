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

#include <algorithm>

// Make sure the global password character buffer is the expected size
static_assert(PW_SEQ_LEN == C_ARRAY_SIZE(SavedGlobals::passwordCharBuffer), "Password char buffer has unexpected size!");

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform an in-place byte swap of a single primitive value, an array of primitive values or an enum
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void byteSwapValue(T& toSwap) noexcept {
    Endian::byteSwapInPlace(toSwap);
}

template <class T, size_t ArraySize>
static void byteSwapValueArray(T (&toSwap)[ArraySize]) noexcept {
    for (T& elem : toSwap) {
        byteSwapValue(elem);
    }
}

template <class T>
static void byteSwapEnumValue(T& toSwap) noexcept {
    Endian::byteSwapEnumInPlace(toSwap);
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

template <class T, uint32_t ArraySize>
static void byteSwapObjects(T (&pObjs)[ArraySize]) noexcept {
    byteSwapObjects(pObjs, ArraySize);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: assign the elements of one array to another
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T, uint32_t ArraySize>
static void arrayAssign(T (&pDstObjs)[ArraySize], const T (&pSrcObjs)[ArraySize]) noexcept {
    for (uint32_t i = 0; i < ArraySize; ++i) {
        pDstObjs[i] = pSrcObjs[i];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: read an object (stored in little endiant format) from the specified stream
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void readObjectLE(InputStream& in, T& obj) THROWS {
    in.read(obj);

    if constexpr (Endian::isBig()) {
        obj.byteSwap();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: allocates an array of objects and then reads those objects from the specified stream.
// The objects in the file are assumed to be little endian.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void readArrayLE(InputStream& in, std::unique_ptr<T[]>& arrayStorage, const uint32_t numObjs) THROWS {
    arrayStorage = std::make_unique<T[]>(numObjs);
    in.readArray(arrayStorage.get(), numObjs);

    if constexpr (Endian::isBig()) {
        byteSwapObjects(arrayStorage.get(), numObjs);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: write an object to the specified stream in little endian format
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void writeObjectLE(OutputStream& out, const T& obj) THROWS {
    if constexpr (Endian::isLittle()) {
        out.write(obj);
    } else {
        T littleObj = obj;
        littleObj.byteSwap();
        out.write(littleObj);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: writes an array objects to the specified stream in little endian format
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void writeArrayLE(OutputStream& out, const T* const pObjs, const uint32_t numObjs) THROWS {
    if constexpr (Endian::isLittle()) {
        out.writeArray(pObjs, numObjs);
    } else {
        for (uint32_t i = 0; i < numObjs; ++i) {
            writeObjectLE(out, pObjs[i]);
        }
    }
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

static bool isValidWeaponType(const weapontype_t weapon) noexcept {
    return ((weapon >= 0) && (weapon < NUMWEAPONS));
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

template <class T, uint32_t ArraySize>
static bool validateObjects(const T (&pObjs)[ArraySize]) noexcept {
    return validateObjects(pObjs, ArraySize);
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

        // Make sure the map object pointer is valid. There are bugs in the DOOM code where sometimes the 'target' and 'tracer'
        // fields end up pointing to deleted objects. I've fixed this issue but keep the sanity checks here just in case.
        // If the pointer is to an invalid object then just pretend it was a null pointer for the purposes of serialization...
        if (iter != SaveAndLoad::gMobjToIdx.end()) {
            return iter->second;
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
    byteSwapValue(floorheight);
    byteSwapValue(ceilingheight);
    byteSwapValue(floorpic);
    byteSwapValue(ceilingpic);
    byteSwapValue(colorid);
    byteSwapValue(ceilColorid);
    byteSwapValue(lightlevel);
    byteSwapValue(special);
    byteSwapValue(tag);
    byteSwapValue(soundtargetIdx);
    byteSwapValue(flags);
    byteSwapValue(floorTexOffsetX);
    byteSwapValue(floorTexOffsetY);
    byteSwapValue(ceilTexOffsetX);
    byteSwapValue(ceilTexOffsetY);
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
    // All things should be removed from the map at this point!
    ASSERT(sector.thinglist == nullptr);

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
    sector.floorDrawH = {};                         // Not serialized, default init
    sector.ceilingDrawH = {};                       // Not serialized, default init
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

    // Don't do interpolations for the first frame!
    R_SnapSectorInterpolation(sector);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedLineT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedLineT::byteSwap() noexcept {
    byteSwapValue(flags);
    byteSwapValue(special);
    byteSwapValue(tag);
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
    line.validcount = 0;            // Not serialized, resets on load
    line.specialdata = nullptr;     // Unused by PSX DOOM - default init
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedSideT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedSideT::byteSwap() noexcept {
    byteSwapValue(textureoffset);
    byteSwapValue(rowoffset);
    byteSwapValue(toptexture);
    byteSwapValue(bottomtexture);
    byteSwapValue(midtexture);
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

    // Don't do interpolations for the first frame!
    R_SnapSideInterpolation(side);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedMobjT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedMobjT::byteSwap() noexcept {
    byteSwapValue(x);
    byteSwapValue(y);
    byteSwapValue(z);
    byteSwapValue(tag);
    byteSwapValue(subsectorIdx);
    byteSwapValue(angle);
    byteSwapValue(sprite);
    byteSwapValue(frame);
    byteSwapValue(floorz);
    byteSwapValue(ceilingz);
    byteSwapValue(radius);
    byteSwapValue(height);
    byteSwapValue(momx);
    byteSwapValue(momy);
    byteSwapValue(momz);
    byteSwapEnumValue(type);
    byteSwapValue(tics);
    byteSwapValue(stateIdx);
    byteSwapValue(flags);
    byteSwapValue(health);
    byteSwapEnumValue(movedir);
    byteSwapValue(movecount);
    byteSwapValue(targetIdx);
    byteSwapValue(reactiontime);
    byteSwapValue(threshold);
    byteSwapValue(spawnx);
    byteSwapValue(spawny);
    byteSwapValue(spawntype);
    byteSwapValue(spawnangle);
    byteSwapValue(tracerIdx);
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
    // Note: don't init the following here deliberately - done elsewhere:
    //  (1) The mobj linked list fields (global mobj list, sector list, blockmap list)
    //  (2) The player field
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
    mobj.extradata = {};        // Not serialized, default init
    mobj.spawnx = spawnx;
    mobj.spawny = spawny;
    mobj.spawntype = spawntype;
    mobj.spawnangle = spawnangle;
    mobj.tracer = getMobjAtIdx(tracerIdx);

    // Don't do interpolations for the first frame!
    R_SnapMobjInterpolation(mobj);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedPspdefT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedPspdefT::byteSwap() noexcept {
    byteSwapValue(stateIdx);
    byteSwapValue(tics);
    byteSwapValue(sx);
    byteSwapValue(sy);
}

bool SavedPspdefT::validate() const noexcept {
    // Note: state is optional for some player sprites!
    return ((stateIdx <= -1) || isValidStateIdx(stateIdx));
}

void SavedPspdefT::serializeFrom(const pspdef_t& spr) noexcept {
    stateIdx = (spr.state) ? (int32_t)(spr.state - gStates) : -1;
    tics = spr.tics;
    sx = spr.sx;
    sy = spr.sy;
}

void SavedPspdefT::deserializeTo(pspdef_t& spr) const noexcept {
    spr.state = (stateIdx >= 0) ? &gStates[stateIdx] : nullptr;
    spr.tics = tics;
    spr.sx = sx;
    spr.sy = sy;

    // Don't do interpolations for the first frame!
    R_SnapPsprInterpolation(spr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedPlayerT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedPlayerT::byteSwap() noexcept {
    byteSwapValue(mobjIdx);
    byteSwapEnumValue(playerstate);
    byteSwapValue(forwardmove);
    byteSwapValue(sidemove);
    byteSwapValue(angleturn);
    byteSwapValue(viewz);
    byteSwapValue(viewheight);
    byteSwapValue(deltaviewheight);
    byteSwapValue(bob);
    byteSwapValue(health);
    byteSwapValue(armorpoints);
    byteSwapValue(armortype);
    byteSwapValueArray(powers);
    byteSwapValueArray(cards);
    byteSwapValue(backpack);
    byteSwapEnumValue(readyweapon);
    byteSwapEnumValue(pendingweapon);
    byteSwapValueArray(weaponowned);
    byteSwapValueArray(ammo);
    byteSwapValueArray(maxammo);
    byteSwapValue(cheats);
    byteSwapValue(killcount);
    byteSwapValue(itemcount);
    byteSwapValue(secretcount);
    byteSwapValue(damagecount);
    byteSwapValue(bonuscount);
    byteSwapValue(attackerIdx);
    byteSwapValue(extralight);
    byteSwapObjects(psprites);
    byteSwapValue(automapx);
    byteSwapValue(automapy);
    byteSwapValue(automapscale);
}

bool SavedPlayerT::validate() const noexcept {
    return (
        isValidMobjIdx(mobjIdx) &&
        isValidWeaponType(readyweapon) &&
        ((pendingweapon == wp_nochange) || isValidWeaponType(pendingweapon)) &&
        isValidMobjIdx(attackerIdx) &&
        validateObjects(psprites)
    );
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
    arrayAssign(powers, player.powers);
    arrayAssign(cards, player.cards);
    backpack = player.backpack;
    readyweapon = player.readyweapon;
    pendingweapon = player.pendingweapon;
    arrayAssign(weaponowned, player.weaponowned);
    arrayAssign(ammo, player.ammo);
    arrayAssign(maxammo, player.maxammo);
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

    automapx = player.automapx;
    automapy = player.automapy;
    automapscale = player.automapscale;
}

void SavedPlayerT::deserializeTo(player_t& player, const bool bSkipMobjAssign) const noexcept {
    // Link the map object to player unless specified otherwise (demos skip this step)
    if (!bSkipMobjAssign) {
        player.mo = getMobjAtIdx(mobjIdx);
        player.mo->player = &player;
    }

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
    arrayAssign(player.powers, powers);
    arrayAssign(player.cards, cards);
    player.backpack = backpack;
    player.frags = {};                      // Not serialized, default init
    player._unused = {};                    // Not serialized, default init
    player.readyweapon = readyweapon;
    player.pendingweapon = pendingweapon;
    arrayAssign(player.weaponowned, weaponowned);
    arrayAssign(player.ammo, ammo);
    arrayAssign(player.maxammo, maxammo);
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
    player.automapx = automapx;
    player.automapy = automapy;
    player.automapscale = automapscale;
    player.automapflags = 0;                // Not serialized, default init
    player.turnheld = {};                   // Not serialized, default init
    player.psxMouseUseCountdown = {};       // Not serialized, default init
    player.psxMouseUse = {};                // Not serialized, default init
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedVLDoorT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedVLDoorT::byteSwap() noexcept {
    byteSwapEnumValue(type);
    byteSwapValue(sectorIdx);
    byteSwapValue(topheight);
    byteSwapValue(speed);
    byteSwapValue(direction);
    byteSwapValue(topwait);
    byteSwapValue(topcountdown);
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
    byteSwapValue(sectorIdx);
    byteSwapValue(def.bOpen);
    byteSwapValue(def.bDoReturn);
    byteSwapValue(def.bBlockable);
    byteSwapValue(def.bDoFinishScript);
    byteSwapValue(def.minHeight);
    byteSwapValue(def.maxHeight);
    byteSwapValue(def.speed);
    byteSwapValue(def.waitTime);
    byteSwapEnumValue(def.openSound);
    byteSwapEnumValue(def.closeSound);
    byteSwapValue(def.finishScriptActionNum);
    byteSwapValue(def.finishScriptUserdata);
    byteSwapValue(direction);
    byteSwapValue(postWaitDirection);
    byteSwapValue(countdown);
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
    byteSwapEnumValue(type);
    byteSwapValue(crush);
    byteSwapValue(bDoFinishScript);
    byteSwapValue(sectorIdx);
    byteSwapValue(direction);
    byteSwapValue(newspecial);
    byteSwapValue(texture);
    byteSwapValue(floordestheight);
    byteSwapValue(speed);
    byteSwapEnumValue(moveSound);
    byteSwapValue(moveSoundFreq);
    byteSwapEnumValue(stopSound);
    byteSwapValue(finishScriptActionNum);
    byteSwapValue(finishScriptUserdata);
}

bool SavedFloorMoveT::validate() const noexcept {
    return (isValidSectorIdx(sectorIdx) && isValidTexNum(texture));
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
    byteSwapEnumValue(type);
    byteSwapValue(sectorIdx);
    byteSwapValue(bottomheight);
    byteSwapValue(topheight);
    byteSwapValue(speed);
    byteSwapValue(crush);
    byteSwapValue(bIsCrushing);
    byteSwapValue(bDoFinishScript);
    byteSwapValue(bIsActive);
    byteSwapValue(direction);
    byteSwapValue(tag);
    byteSwapValue(olddirection);
    byteSwapValue(crushSpeed);
    byteSwapValue(dirChangesLeft);
    byteSwapEnumValue(moveSound);
    byteSwapValue(moveSoundFreq);
    byteSwapEnumValue(changeDirSound);
    byteSwapEnumValue(stopSound);
    byteSwapValue(finishScriptActionNum);
    byteSwapValue(finishScriptUserdata);
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedPlatT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedPlatT::byteSwap() noexcept {
    byteSwapValue(sectorIdx);
    byteSwapValue(speed);
    byteSwapValue(low);
    byteSwapValue(high);
    byteSwapValue(wait);
    byteSwapValue(count);
    byteSwapEnumValue(status);
    byteSwapEnumValue(oldstatus);
    byteSwapValue(crush);
    byteSwapValue(bDoFinishScript);
    byteSwapValue(bIsActive);
    byteSwapValue(tag);
    byteSwapEnumValue(type);
    byteSwapEnumValue(finishState);
    byteSwapEnumValue(startSound);
    byteSwapEnumValue(moveSound);
    byteSwapValue(moveSoundFreq);
    byteSwapEnumValue(stopSound);
    byteSwapValue(finishScriptActionNum);
    byteSwapValue(finishScriptUserdata);
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SavedFireFlickerT
//------------------------------------------------------------------------------------------------------------------------------------------
void SavedFireFlickerT::byteSwap() noexcept {
    byteSwapValue(sectorIdx);
    byteSwapValue(count);
    byteSwapValue(maxlight);
    byteSwapValue(minlight);
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
    byteSwapValue(sectorIdx);
    byteSwapValue(count);
    byteSwapValue(maxlight);
    byteSwapValue(minlight);
    byteSwapValue(maxtime);
    byteSwapValue(mintime);
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
    byteSwapValue(sectorIdx);
    byteSwapValue(count);
    byteSwapValue(minlight);
    byteSwapValue(maxlight);
    byteSwapValue(darktime);
    byteSwapValue(brighttime);
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
    byteSwapValue(sectorIdx);
    byteSwapValue(minlight);
    byteSwapValue(maxlight);
    byteSwapValue(direction);
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
    byteSwapValue(ticsleft);
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
    byteSwapValue(lineIdx);
    byteSwapEnumValue(where);
    byteSwapValue(btexture);
    byteSwapValue(btimer);
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
    byteSwapValue(actionNum);
    byteSwapValue(delayTics);
    byteSwapValue(executionsLeft);
    byteSwapValue(repeatDelay);
    byteSwapValue(tag);
    byteSwapValue(userdata);
    byteSwapValue(bPaused);
    byteSwapValue(bPendingExecute);
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
    byteSwapValue(face);
    byteSwapEnumValue(specialFace);
    byteSwapValue(gotgibbed);
    byteSwapValue(gibframe);
    byteSwapValue(gibframeTicsLeft);
    byteSwapValue(alertMessageTicsLeft);
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
    byteSwapValue(gameMap);
    byteSwapValue(nextMap);
    byteSwapEnumValue(gameSkill);
    byteSwapValue(levelElapsedTime);
    byteSwapValue(totalKills);
    byteSwapValue(totalItems);
    byteSwapValue(totalSecret);
    player.byteSwap();
    byteSwapValue(prndIndex);
    byteSwapValue(mrndIndex);
    byteSwapValue(gameTic);
    byteSwapValue(ticCon);
    byteSwapValue(ticRemainder);
    byteSwapValue(mapBossSpecialFlags);
    byteSwapValue(extCameraTicsLeft);
    byteSwapValue(extCameraX);
    byteSwapValue(extCameraY);
    byteSwapValue(extCameraZ);
    byteSwapValue(extCameraAngle);
    byteSwapValue(numPasswordCharsEntered);
    byteSwapValue(curCDTrack);
    statusBar.byteSwap();
    byteSwapValue(faceTics);
    byteSwapValue(bDrawSBFace);
    byteSwapValue(bGibDraw);
    byteSwapValue(bDoSpclFace);
    byteSwapValue(newFace);
    byteSwapEnumValue(spclFaceType);
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

    if ((numPasswordCharsEntered < 0) || (numPasswordCharsEntered > (int32_t) C_ARRAY_SIZE(passwordCharBuffer)))
        return false;

    if (!statusBar.validate())
        return false;

    if ((newFace < 0) || (newFace >= NUMFACES))
        return false;

    if (gbDoSpclFace && ((spclFaceType < 0) || (spclFaceType >= NUMSPCLFACES)))
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
    player.deserializeTo(gPlayers[0]);
    
    gPRndIndex = prndIndex;
    gMRndIndex = mrndIndex;
    gGameTic = gameTic;
    gTicCon = ticCon;
    gbIsFirstTick = (ticCon == 0);      // Not saved explicitly since it's redundant

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
    gLastTgtGameTicCount = tgtGameTicCount;

    // The count marker gets reset after deserializing
    gValidCount = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SaveFileHdr
//------------------------------------------------------------------------------------------------------------------------------------------
void SaveFileHdr::byteSwap() noexcept {
    // Note: dont byte swap 'mapName' as the characters should always be in the same order, regardless of endianness
    byteSwapValue(fileId1);
    byteSwapValue(fileId2);
    byteSwapValue(version);
    byteSwapValue(mapNum);
    byteSwapValue(secondsPlayed);
    byteSwapValue(mapHashWord1);
    byteSwapValue(mapHashWord2);
    byteSwapValue(numSectors);
    byteSwapValue(numLines);
    byteSwapValue(numSides);
    byteSwapValue(numMobjs);
    byteSwapValue(numVlDoors);
    byteSwapValue(numVlCustomDoors);
    byteSwapValue(numFloorMovers);
    byteSwapValue(numCeilings);
    byteSwapValue(numPlats);
    byteSwapValue(numFireFlickers);
    byteSwapValue(numLightFlashes);
    byteSwapValue(numStrobes);
    byteSwapValue(numGlows);
    byteSwapValue(numDelayedExits);
    byteSwapValue(numButtons);
    byteSwapValue(numScheduledActions);
}

bool SaveFileHdr::validateFileId() const noexcept {
    return ((fileId1 == SAVE_FILE_ID1) && (fileId2 == SAVE_FILE_ID2));
}

bool SaveFileHdr::validateVersion() const noexcept {
    return (version == SAVE_FILE_VERSION);
}

bool SaveFileHdr::validateMapNum() const noexcept {
    return ((mapNum >= 1) && (mapNum <= Game::getNumMaps()));
}

bool SaveFileHdr::validateMapHash() const noexcept {
    return ((mapHashWord1 == MapHash::gWord1) && (mapHashWord2 == MapHash::gWord2));
}

bool SaveFileHdr::validate() const noexcept {
    return (
        validateFileId() &&
        validateVersion() &&
        validateMapNum() &&
        validateMapHash() &&
        // Should not be negative!
        (secondsPlayed >= 0) &&
        // Sanity these counts match the map
        ((int32_t) numSectors == gNumSectors) &&
        ((int32_t) numLines == gNumLines) &&
        ((int32_t) numSides == gNumSides) &&
        // There should be at least one map object! (the player)
        (numMobjs > 0)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// SaveData
//------------------------------------------------------------------------------------------------------------------------------------------
bool SaveData::writeTo(OutputStream& out) const noexcept {
    try {
        writeObjectLE(out, hdr);
        writeObjectLE(out, globals);
        writeArrayLE(out, sectors.get(), hdr.numSectors);
        writeArrayLE(out, lines.get(), hdr.numLines);
        writeArrayLE(out, sides.get(), hdr.numSides);
        writeArrayLE(out, mobjs.get(), hdr.numMobjs);
        writeArrayLE(out, vlDoors.get(), hdr.numVlDoors);
        writeArrayLE(out, vlCustomDoors.get(), hdr.numVlCustomDoors);
        writeArrayLE(out, floorMovers.get(), hdr.numFloorMovers);
        writeArrayLE(out, ceilings.get(), hdr.numCeilings);
        writeArrayLE(out, plats.get(), hdr.numPlats);
        writeArrayLE(out, fireFlickers.get(), hdr.numFireFlickers);
        writeArrayLE(out, lightFlashes.get(), hdr.numLightFlashes);
        writeArrayLE(out, strobes.get(), hdr.numStrobes);
        writeArrayLE(out, glows.get(), hdr.numGlows);
        writeArrayLE(out, delayedExits.get(), hdr.numDelayedExits);
        writeArrayLE(out, buttons.get(), hdr.numButtons);
        writeArrayLE(out, scheduledActions.get(), hdr.numScheduledActions);
        return true;
    }
    catch (...) {
        return false;
    }
}

ReadSaveResult SaveData::readFrom(InputStream& in) noexcept {
    try {
        // Read the header first and do basic validity checks
        readObjectLE(in, hdr);

        if (!hdr.validateFileId())
            return ReadSaveResult::BAD_FILE_ID;

        if (!hdr.validateVersion())
            return ReadSaveResult::BAD_VERSION;

        if (!hdr.validateMapNum())
            return ReadSaveResult::BAD_MAP_NUM;

        // Read the globals and everything else
        readObjectLE(in, globals);
        readArrayLE(in, sectors, hdr.numSectors);
        readArrayLE(in, lines, hdr.numLines);
        readArrayLE(in, sides, hdr.numSides);
        readArrayLE(in, mobjs, hdr.numMobjs);
        readArrayLE(in, vlDoors, hdr.numVlDoors);
        readArrayLE(in, vlCustomDoors, hdr.numVlCustomDoors);
        readArrayLE(in, floorMovers, hdr.numFloorMovers);
        readArrayLE(in, ceilings, hdr.numCeilings);
        readArrayLE(in, plats, hdr.numPlats);
        readArrayLE(in, fireFlickers, hdr.numFireFlickers);
        readArrayLE(in, lightFlashes, hdr.numLightFlashes);
        readArrayLE(in, strobes, hdr.numStrobes);
        readArrayLE(in, glows, hdr.numGlows);
        readArrayLE(in, delayedExits, hdr.numDelayedExits);
        readArrayLE(in, buttons, hdr.numButtons);
        readArrayLE(in, scheduledActions, hdr.numScheduledActions);
        return ReadSaveResult::OK;
    }
    catch (...) {
        return ReadSaveResult::IO_ERROR;
    }
}
