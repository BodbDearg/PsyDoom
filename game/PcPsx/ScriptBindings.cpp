#include "ScriptBindings.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/p_floor.h"
#include "Doom/Game/p_inter.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Game/p_switch.h"
#include "Doom/Game/p_telept.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "ScriptingEngine.h"

#include <algorithm>
#include <cstdio>
#include <sol/sol.hpp>

BEGIN_NAMESPACE(ScriptBindings)

//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities to convert back and forth between Doom angles and degrees
//------------------------------------------------------------------------------------------------------------------------------------------
static float AngleToDegrees(const angle_t angle) noexcept {
    const double normalized = (double) angle / (double(UINT32_MAX) + 1.0);
    return (float)(normalized * 360.0);
}

static angle_t DegreesToAngle(const float degrees) noexcept {
    const double range360 = std::fmod((double) degrees, 360.0);
    const double normalized = (range360 < 0.0) ? (range360 + 360.0) / 360.0 : range360 / 360.0;
    const double intRange = normalized * (double(UINT32_MAX) + 1.0);
    return (angle_t) intRange;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: Miscellaneous
//------------------------------------------------------------------------------------------------------------------------------------------
static float FixedToFloat(const fixed_t num) noexcept {
    return (float)(num * (1.0f / 65536.0f));
}

static fixed_t FloatToFixed(const float num) noexcept {
    return (fixed_t)(num * 65536.0f);
}

static int32_t Script_R_TextureNumForName(const char* const name) noexcept {
    return (name) ? R_TextureNumForName(name, false) : -1;
}

static int32_t Script_R_FlatNumForName(const char* const name) noexcept {
    return (name) ? R_FlatNumForName(name, false) : -1;
}

static void P_StatusMessage(const char* const message) noexcept {
    // Use the level startup warning buffer since it's always around
    std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "%s", message);
    player_t& player = gPlayers[gCurPlayerIndex];
    player.message = gLevelStartupWarning;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: Sectors
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t GetNumSectors() noexcept {
    return gNumSectors;
}

static sector_t* GetSector(const int32_t index) noexcept {
    return ((index >= 0) && (index < gNumSectors)) ? gpSectors + index : nullptr;
}

static sector_t* FindSectorWithTag(const int32_t tag) noexcept {
    const int32_t numSectors = gNumSectors;
    sector_t* const pSectors = gpSectors;

    for (int32_t i = 0; i < numSectors; ++i) {
        sector_t& sector = pSectors[i];

        if (sector.tag == tag)
            return &sector;
    }

    return nullptr;
}

static void ForEachSector(const std::function<void (sector_t& sector)>& callback) noexcept {
    if (!callback)
        return;

    const int32_t numSectors = gNumSectors;
    sector_t* const pSectors = gpSectors;

    for (int32_t i = 0; i < numSectors; ++i) {
        callback(pSectors[i]);
    }
}

static void ForEachSectorWithTag(const int32_t tag, const std::function<void (sector_t& sector)>& callback) noexcept {
    if (!callback)
        return;

    const int32_t numSectors = gNumSectors;
    sector_t* const pSectors = gpSectors;

    for (int32_t i = 0; i < numSectors; ++i) {
        sector_t& sector = pSectors[i];

        if (sector.tag == tag) {
            callback(sector);
        }
    }
}

static line_t* GetLineInSector(sector_t& sector, const int32_t index) noexcept {
    return ((index >= 0) && (index < sector.linecount)) ? sector.lines[index] : nullptr;
}

static void ForEachLineInSector(sector_t& sector, const std::function<void (line_t& line)>& callback) noexcept {
    if (!callback)
        return;

    const int32_t numLines = sector.linecount;
    line_t** const ppLines = sector.lines;

    for (int32_t i = 0; i < numLines; ++i) {
        callback(*ppLines[i]);
    }
}

static void ForEachThingInSector(sector_t& sector, const std::function<void (mobj_t& mo)>& callback) noexcept {
    if (!callback)
        return;

    for (mobj_t* pMobj = sector.thinglist; pMobj; pMobj = pMobj->snext) {
        callback(*pMobj);
    }
}

static void ForEachSurroundingSector(sector_t& sector, const std::function<void (sector_t& sector)>& callback) noexcept {
    if (!callback)
        return;

    const int32_t numLines = sector.linecount;
    line_t** const ppLines = sector.lines;

    for (int32_t lineIdx = 0; lineIdx < numLines; ++lineIdx) {
        line_t& line = *ppLines[lineIdx];
        sector_t* const pNextSector = getNextSector(line, sector);

        if (pNextSector) {
            callback(*pNextSector);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: Lines
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t GetNumLines() noexcept {
    return gNumLines;
}

static line_t* GetLine(const int32_t index) noexcept {
    return ((index >= 0) && (index < gNumLines)) ? gpLines + index : nullptr;
}

static line_t* FindLineWithTag(const int32_t tag) noexcept {
    const int32_t numLines = gNumLines;
    line_t* const pLines = gpLines;

    for (int32_t i = 0; i < numLines; ++i) {
        line_t& line = pLines[i];

        if (line.tag == tag)
            return &line;
    }

    return nullptr;
}

static void ForEachLine(const std::function<void (line_t& line)>& callback) noexcept {
    if (!callback)
        return;

    const int32_t numLines = gNumLines;
    line_t* const pLines = gpLines;

    for (int32_t i = 0; i < numLines; ++i) {
        callback(pLines[i]);
    }
}

static void ForEachLineWithTag(const int32_t tag, const std::function<void (line_t& line)>& callback) noexcept {
    if (!callback)
        return;

    const int32_t numLines = gNumLines;
    line_t* const pLines = gpLines;

    for (int32_t i = 0; i < numLines; ++i) {
        line_t& line = pLines[i];

        if (line.tag == tag) {
            callback(line);
        }
    }
}

static void Script_P_CrossSpecialLine(line_t& line, mobj_t* const pActivator) noexcept {
    P_CrossSpecialLine(line, (pActivator) ? *pActivator : *gPlayers[0].mo);
}

static void Script_P_ShootSpecialLine(line_t& line, mobj_t* const pActivator) noexcept {
    P_ShootSpecialLine((pActivator) ? *pActivator : *gPlayers[0].mo, line);
}

static void Script_P_UseSpecialLine(line_t& line, mobj_t* const pActivator) noexcept {
    P_UseSpecialLine((pActivator) ? *pActivator : *gPlayers[0].mo, line);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: Sides
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t GetNumSides() noexcept {
    return gNumSides;
}

static side_t* GetSide(const int32_t index) noexcept {
    return ((index >= 0) && (index < gNumSides)) ? gpSides + index : nullptr;
}

static void ForEachSide(const std::function<void (side_t& sector)>& callback) noexcept {
    if (!callback)
        return;

    const int32_t numSides = gNumSides;
    side_t* const pSides = gpSides;

    for (int32_t i = 0; i < numSides; ++i) {
        callback(pSides[i]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: Things
//------------------------------------------------------------------------------------------------------------------------------------------
static void ForEachThing(const std::function<void (mobj_t& mo)>& callback) noexcept {
    if (!callback)
        return;

    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead; pMobj = pMobj->next) {
        callback(*pMobj);
    }
}

static int32_t FindMobjTypeForDoomEdNum(const int32_t doomEdNum) noexcept {
    const int32_t numMobjTypes = gNumMobjInfo;
    const mobjinfo_t* const pMobjInfo = gMobjInfo;

    for (int32_t i = 0; i < numMobjTypes; ++i) {
        if (pMobjInfo[i].doomednum == doomEdNum)
            return i;
    }

    return -1;
}

mobj_t* Script_P_SpawnMobj(const float x, const float y, const float z, const uint32_t type) noexcept {
    const fixed_t xfrac = FloatToFixed(x);
    const fixed_t yfrac = FloatToFixed(y);
    const fixed_t zfrac = FloatToFixed(z);

    if (type >= (uint32_t) gNumMobjInfo)
        return nullptr;

    return P_SpawnMobj(xfrac, yfrac, zfrac, (mobjtype_t) type);
}

mobj_t* Script_P_SpawnMissile(mobj_t& src, mobj_t& dst, const uint32_t type) noexcept {
    return (type < (uint32_t) gNumMobjInfo) ? P_SpawnMissile(src, dst, (mobjtype_t) type) : nullptr;
}

bool Script_EV_TeleportTo(
    mobj_t& mobj,
    const float dstX,
    const float dstY,
    const float dstAngle,
    const bool bTelefrag,
    const bool bPreserveMomentum,
    const uint32_t fogMobjType,
    const uint32_t fogSoundId
) noexcept {
    return EV_TeleportTo(
        mobj,
        FloatToFixed(dstX),
        FloatToFixed(dstY),
        DegreesToAngle(dstAngle),
        bTelefrag,
        bPreserveMomentum,
        (mobjtype_t) fogMobjType,
        (sfxenum_t) fogSoundId
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: Script execution context
//------------------------------------------------------------------------------------------------------------------------------------------
static line_t* GetTriggeringLine() noexcept { return ScriptingEngine::gpCurTriggeringLine; }
static sector_t* GetTriggeringSector() noexcept { return ScriptingEngine::gpCurTriggeringSector; }
static mobj_t* GetTriggeringThing() noexcept { return ScriptingEngine::gpCurTriggeringMobj; }
static void SetLineActionAllowed(const bool bAllowed) noexcept { ScriptingEngine::gbCurActionAllowed = bAllowed; }

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: moving a floor or ceiling (simplified slightly)
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t T_MoveFloor(sector_t& sector, const float speed, const float destHeight, const bool bCrush) noexcept {
    const fixed_t speedFixed = FloatToFixed(speed);
    const fixed_t destHeightFixed = FloatToFixed(destHeight);
    return (uint32_t) T_MovePlane(sector, speedFixed, destHeightFixed, bCrush, 0, (destHeightFixed >= sector.floorheight) ? +1 : -1);
}

static uint32_t T_MoveCeiling(sector_t& sector, const float speed, const float destHeight, const bool bCrush) noexcept {
    const fixed_t speedFixed = FloatToFixed(speed);
    const fixed_t destHeightFixed = FloatToFixed(destHeight);
    return (uint32_t) T_MovePlane(sector, speedFixed, destHeightFixed, bCrush, 1, (destHeightFixed >= sector.ceilingheight) ? +1 : -1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: sound
//------------------------------------------------------------------------------------------------------------------------------------------
void S_PlaySoundAtThing(mobj_t* const pOrigin, const uint32_t soundId) noexcept {
    S_StartSound(pOrigin, (sfxenum_t) soundId);
}

void S_PlaySoundAtSector(sector_t* const pOrigin, const uint32_t soundId) noexcept {
    S_StartSound(pOrigin ? (mobj_t*) &pOrigin->soundorg : nullptr, (sfxenum_t) soundId);
}

void S_PlaySoundAtPosition(const float x, const float y, const uint32_t soundId) noexcept {
    degenmobj_t dummyMobj = {};
    dummyMobj.x = FloatToFixed(x);
    dummyMobj.y = FloatToFixed(y);
    dummyMobj.subsector = R_PointInSubsector(dummyMobj.x, dummyMobj.y);
    S_StartSound((mobj_t*) &dummyMobj, (sfxenum_t) soundId);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper macros for registering properties
//------------------------------------------------------------------------------------------------------------------------------------------

// Register a getter and setter for a fixed point number and expose it as a floating point number
#define SOL_FIXED_PROPERTY_AS_FLOAT(TypeName, FieldName)\
    sol::property(\
        [](const TypeName& obj) noexcept { return FixedToFloat(obj.FieldName); },\
        [](TypeName& obj, const float value) noexcept { obj.FieldName = FloatToFixed(value); }\
    )

// Register a getter (only) and for a fixed point number and expose it as a floating point number
#define SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(TypeName, FieldName)\
    sol::readonly_property(\
        [](const TypeName& obj) noexcept { return FixedToFloat(obj.FieldName); }\
    )

// Register a getter and setter for a value defined from 0-255.
// This property also clamps the value to be in range, rather than dropping bits.
#define SOL_BYTE_PROPERTY(TypeName, FieldName)\
    sol::property(\
        [](const TypeName& obj) noexcept -> int32_t { return obj.FieldName; },\
        [](TypeName& obj, const int32_t value) noexcept { obj.FieldName = (uint8_t) std::clamp(value, 0, 255); }\
    )

//------------------------------------------------------------------------------------------------------------------------------------------
// Type registration functions
//------------------------------------------------------------------------------------------------------------------------------------------
static void registerType_sector_t(sol::state& lua) noexcept {
    sol::usertype<sector_t> type = lua.new_usertype<sector_t>("sector_t", sol::no_constructor);

    type["index"] = sol::readonly_property([](const sector_t& s) noexcept { return &s - gpSectors; });
    type["floorheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(sector_t, floorheight);
    type["floorheight_fixed"] = &sector_t::floorheight;
    type["ceilingheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(sector_t, ceilingheight);
    type["ceilingheight_fixed"] = &sector_t::ceilingheight;
    type["floorpic"] = &sector_t::floorpic;
    type["ceilingpic"] = &sector_t::ceilingpic;
    type["colorid"] = SOL_BYTE_PROPERTY(sector_t, colorid);
    type["lightlevel"] = SOL_BYTE_PROPERTY(sector_t, lightlevel);
    type["special"] = &sector_t::special;
    type["tag"] = &sector_t::tag;
    type["flags"] = &sector_t::flags;
    type["ceil_colorid"] = SOL_BYTE_PROPERTY(sector_t, ceilColorid);
    type["numlines"] = sol::readonly(&sector_t::linecount);
    type.set_function("GetLine", GetLineInSector);
    type.set_function("ForEachLine", ForEachLineInSector);
    type.set_function("ForEachThing", ForEachThingInSector);
    type.set_function("ForEachSurroundingSector", ForEachSurroundingSector);
}

static void registerType_line_t(sol::state& lua) noexcept {
    sol::usertype<line_t> type = lua.new_usertype<line_t>("line_t", sol::no_constructor);

    type["index"] = sol::readonly_property([](const line_t& s) noexcept { return &s - gpLines; });
    type["v1x"] = sol::readonly_property([](const line_t& line) noexcept { return FixedToFloat(line.vertex1->x); });
    type["v1y"] = sol::readonly_property([](const line_t& line) noexcept { return FixedToFloat(line.vertex1->y); });
    type["v2x"] = sol::readonly_property([](const line_t& line) noexcept { return FixedToFloat(line.vertex2->x); });
    type["v2y"] = sol::readonly_property([](const line_t& line) noexcept { return FixedToFloat(line.vertex2->y); });
    type["angle"] = sol::readonly_property([](const line_t& line) noexcept { return AngleToDegrees((angle_t) line.fineangle << ANGLETOFINESHIFT); });
    type["flags"] = &line_t::flags;
    type["special"] = &line_t::special;
    type["tag"] = &line_t::tag;
    type["frontside"] = sol::readonly_property([](const line_t& line) noexcept { return GetSide(line.sidenum[0]); });
    type["backside"] = sol::readonly_property([](const line_t& line) noexcept { return GetSide(line.sidenum[1]); });
    type["frontsector"] = sol::readonly(&line_t::frontsector);
    type["backsector"] = sol::readonly(&line_t::backsector);
}

static void registerType_side_t(sol::state& lua) noexcept {
    sol::usertype<side_t> type = lua.new_usertype<side_t>("side_t", sol::no_constructor);

    type["index"] = sol::readonly_property([](const side_t& s) noexcept { return &s - gpSides; });
    type["textureoffset"] = SOL_FIXED_PROPERTY_AS_FLOAT(side_t, textureoffset);
    type["textureoffset_fixed"] = &side_t::textureoffset;
    type["rowoffset"] = SOL_FIXED_PROPERTY_AS_FLOAT(side_t, rowoffset);
    type["rowoffset_fixed"] = &side_t::rowoffset;
    type["toptexture"] = &side_t::toptexture;
    type["bottomtexture"] = &side_t::bottomtexture;
    type["midtexture"] = &side_t::midtexture;
    type["sector"] = sol::readonly(&side_t::sector);
}

static void registerType_mobj_t(sol::state& lua) noexcept {
    sol::usertype<mobj_t> type = lua.new_usertype<mobj_t>("mobj_t", sol::no_constructor);

    type["x"] = SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(mobj_t, x);
    type["y"] = SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(mobj_t, y);
    type["z"] = SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(mobj_t, z);
    type["angle"] = sol::property(
        [](const mobj_t& mo) noexcept { return AngleToDegrees(mo.angle); },
        [](mobj_t& mo, const float value) noexcept { mo.angle = DegreesToAngle(value); }
    );
    type["momx"] = SOL_FIXED_PROPERTY_AS_FLOAT(mobj_t, momx);
    type["momy"] = SOL_FIXED_PROPERTY_AS_FLOAT(mobj_t, momy);
    type["momz"] = SOL_FIXED_PROPERTY_AS_FLOAT(mobj_t, momz);
    type["type"] = sol::readonly(&mobj_t::type);
    type["flags"] = sol::readonly(&mobj_t::flags);
    type["radius"] = SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(mobj_t, radius);
    type["height"] = SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(mobj_t, height);
    type["health"] = sol::readonly(&mobj_t::health);
    type["sector"] = sol::readonly_property([](const mobj_t& mo) noexcept { return mo.subsector->sector; });
    type["target"] = sol::readonly(&mobj_t::target);
    type["tracer"] = sol::readonly(&mobj_t::tracer);
    type["player"] = sol::readonly(&mobj_t::player);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Registers all Lua scripting types
//------------------------------------------------------------------------------------------------------------------------------------------
static void registerLuaTypes(sol::state& lua) noexcept {
    registerType_sector_t(lua);
    registerType_line_t(lua);
    registerType_side_t(lua);
    registerType_mobj_t(lua);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Registers all Lua scripting functions
//------------------------------------------------------------------------------------------------------------------------------------------
static void registerLuaFunctions(sol::state& lua) noexcept {
    lua["FixedToFloat"] = FixedToFloat;
    lua["FloatToFixed"] = FloatToFixed;
    lua["P_Random"] = P_Random;
    lua["R_TextureNumForName"] = Script_R_TextureNumForName;
    lua["R_FlatNumForName"] = Script_R_FlatNumForName;
    lua["G_ExitLevel"] = G_ExitLevel;
    lua["G_SecretExitLevel"] = G_SecretExitLevel;
    lua["P_StatusMessage"] = P_StatusMessage;

    lua["GetNumSectors"] = GetNumSectors;
    lua["GetSector"] = GetSector;
    lua["FindSectorWithTag"] = FindSectorWithTag;
    lua["ForEachSector"] = ForEachSector;
    lua["ForEachSectorWithTag"] = ForEachSectorWithTag;
    
    lua["GetNumLines"] = GetNumLines;
    lua["GetLine"] = GetLine;
    lua["FindLineWithTag"] = FindLineWithTag;
    lua["ForEachLine"] = ForEachLine;
    lua["ForEachLineWithTag"] = ForEachLineWithTag;
    lua["P_CrossSpecialLine"] = Script_P_CrossSpecialLine;
    lua["P_ShootSpecialLine"] = Script_P_ShootSpecialLine;
    lua["P_UseSpecialLine"] = Script_P_UseSpecialLine;
    
    lua["GetNumSides"] = GetNumSides;
    lua["GetSide"] = GetSide;
    lua["ForEachSide"] = ForEachSide;

    lua["ForEachThing"] = ForEachThing;
    lua["FindMobjTypeForDoomEdNum"] = FindMobjTypeForDoomEdNum;
    lua["P_SpawnMobj"] = Script_P_SpawnMobj;
    lua["P_SpawnMissile"] = Script_P_SpawnMissile;
    lua["P_DamageMobj"] = P_DamageMobj;
    lua["EV_TeleportTo"] = Script_EV_TeleportTo;

    lua["GetTriggeringLine"] = GetTriggeringLine;
    lua["GetTriggeringSector"] = GetTriggeringSector;
    lua["GetTriggeringThing"] = GetTriggeringThing;
    lua["SetLineActionAllowed"] = SetLineActionAllowed;
    
    lua["T_MoveFloor"] = T_MoveFloor;
    lua["T_MoveCeiling"] = T_MoveCeiling;

    lua["S_PlaySoundAtThing"] = S_PlaySoundAtThing;
    lua["S_PlaySoundAtSector"] = S_PlaySoundAtSector;
    lua["S_PlaySoundAtPosition"] = S_PlaySoundAtPosition;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Registers all Lua scripting constants
//------------------------------------------------------------------------------------------------------------------------------------------
static void registerLuaConstants(sol::state& lua) noexcept {
    lua["T_MOVEPLANE_OK"] = (uint32_t) result_e::ok;
    lua["T_MOVEPLANE_CRUSHED"] = (uint32_t) result_e::crushed;
    lua["T_MOVEPLANE_PASTDEST"] = (uint32_t) result_e::pastdest;

    lua["SF_NO_REVERB"] = SF_NO_REVERB;
    lua["SF_GHOSTPLAT"] = SF_GHOSTPLAT;
    lua["SF_GRAD_CONTRACT"] = SF_GRAD_CONTRACT;
    lua["SF_GRAD_FLOOR_PLUS_1"] = SF_GRAD_FLOOR_PLUS_1;
    lua["SF_GRAD_FLOOR_PLUS_2"] = SF_GRAD_FLOOR_PLUS_2;
    lua["SF_GRAD_CEIL_PLUS_1"] = SF_GRAD_CEIL_PLUS_1;
    lua["SF_GRAD_CEIL_PLUS_2"] = SF_GRAD_CEIL_PLUS_2;

    lua["ML_BLOCKING"] = ML_BLOCKING;
    lua["ML_BLOCKMONSTERS"] = ML_BLOCKMONSTERS;
    lua["ML_TWOSIDED"] = ML_TWOSIDED;
    lua["ML_DONTPEGTOP"] = ML_DONTPEGTOP;
    lua["ML_DONTPEGBOTTOM"] = ML_DONTPEGBOTTOM;
    lua["ML_SECRET"] = ML_SECRET;
    lua["ML_SOUNDBLOCK"] = ML_SOUNDBLOCK;
    lua["ML_DONTDRAW"] = ML_DONTDRAW;
    lua["ML_MAPPED"] = ML_MAPPED;
    lua["ML_MIDMASKED"] = ML_MIDMASKED;
    lua["ML_MIDTRANSLUCENT"] = ML_MIDTRANSLUCENT;
    lua["ML_BLOCKPRJECTILE"] = ML_BLOCKPRJECTILE;
    lua["ML_MIDHEIGHT_128"] = ML_MIDHEIGHT_128;
    lua["ML_VOID"] = ML_VOID;
    lua["ML_ADD_SKY_WALL_HINT"] = ML_ADD_SKY_WALL_HINT;

    lua["MT_PLAYER"] = MT_PLAYER;
    lua["MT_POSSESSED"] = MT_POSSESSED;
    lua["MT_SHOTGUY"] = MT_SHOTGUY;
    lua["MT_UNDEAD"] = MT_UNDEAD;
    lua["MT_TRACER"] = MT_TRACER;
    lua["MT_SMOKE"] = MT_SMOKE;
    lua["MT_FATSO"] = MT_FATSO;
    lua["MT_FATSHOT"] = MT_FATSHOT;
    lua["MT_CHAINGUY"] = MT_CHAINGUY;
    lua["MT_TROOP"] = MT_TROOP;
    lua["MT_SERGEANT"] = MT_SERGEANT;
    lua["MT_HEAD"] = MT_HEAD;
    lua["MT_BRUISER"] = MT_BRUISER;
    lua["MT_KNIGHT"] = MT_KNIGHT;
    lua["MT_SKULL"] = MT_SKULL;
    lua["MT_SPIDER"] = MT_SPIDER;
    lua["MT_BABY"] = MT_BABY;
    lua["MT_CYBORG"] = MT_CYBORG;
    lua["MT_PAIN"] = MT_PAIN;
    lua["MT_BARREL"] = MT_BARREL;
    lua["MT_TROOPSHOT"] = MT_TROOPSHOT;
    lua["MT_HEADSHOT"] = MT_HEADSHOT;
    lua["MT_BRUISERSHOT"] = MT_BRUISERSHOT;
    lua["MT_ROCKET"] = MT_ROCKET;
    lua["MT_PLASMA"] = MT_PLASMA;
    lua["MT_BFG"] = MT_BFG;
    lua["MT_ARACHPLAZ"] = MT_ARACHPLAZ;
    lua["MT_PUFF"] = MT_PUFF;
    lua["MT_BLOOD"] = MT_BLOOD;
    lua["MT_TFOG"] = MT_TFOG;
    lua["MT_IFOG"] = MT_IFOG;
    lua["MT_TELEPORTMAN"] = MT_TELEPORTMAN;
    lua["MT_EXTRABFG"] = MT_EXTRABFG;
    lua["MT_MISC0"] = MT_MISC0;
    lua["MT_MISC1"] = MT_MISC1;
    lua["MT_MISC2"] = MT_MISC2;
    lua["MT_MISC3"] = MT_MISC3;
    lua["MT_MISC4"] = MT_MISC4;
    lua["MT_MISC5"] = MT_MISC5;
    lua["MT_MISC6"] = MT_MISC6;
    lua["MT_MISC7"] = MT_MISC7;
    lua["MT_MISC8"] = MT_MISC8;
    lua["MT_MISC9"] = MT_MISC9;
    lua["MT_MISC10"] = MT_MISC10;
    lua["MT_MISC11"] = MT_MISC11;
    lua["MT_MISC12"] = MT_MISC12;
    lua["MT_INV"] = MT_INV;
    lua["MT_MISC13"] = MT_MISC13;
    lua["MT_INS"] = MT_INS;
    lua["MT_MISC14"] = MT_MISC14;
    lua["MT_MISC15"] = MT_MISC15;
    lua["MT_MISC16"] = MT_MISC16;
    lua["MT_MEGA"] = MT_MEGA;
    lua["MT_CLIP"] = MT_CLIP;
    lua["MT_MISC17"] = MT_MISC17;
    lua["MT_MISC18"] = MT_MISC18;
    lua["MT_MISC19"] = MT_MISC19;
    lua["MT_MISC20"] = MT_MISC20;
    lua["MT_MISC21"] = MT_MISC21;
    lua["MT_MISC22"] = MT_MISC22;
    lua["MT_MISC23"] = MT_MISC23;
    lua["MT_MISC24"] = MT_MISC24;
    lua["MT_MISC25"] = MT_MISC25;
    lua["MT_CHAINGUN"] = MT_CHAINGUN;
    lua["MT_MISC26"] = MT_MISC26;
    lua["MT_MISC27"] = MT_MISC27;
    lua["MT_MISC28"] = MT_MISC28;
    lua["MT_SHOTGUN"] = MT_SHOTGUN;
    lua["MT_SUPERSHOTGUN"] = MT_SUPERSHOTGUN;
    lua["MT_MISC29"] = MT_MISC29;
    lua["MT_MISC30"] = MT_MISC30;
    lua["MT_MISC31"] = MT_MISC31;
    lua["MT_MISC32"] = MT_MISC32;
    lua["MT_MISC33"] = MT_MISC33;
    lua["MT_MISC34"] = MT_MISC34;
    lua["MT_MISC35"] = MT_MISC35;
    lua["MT_MISC36"] = MT_MISC36;
    lua["MT_MISC37"] = MT_MISC37;
    lua["MT_MISC38"] = MT_MISC38;
    lua["MT_MISC39"] = MT_MISC39;
    lua["MT_MISC40"] = MT_MISC40;
    lua["MT_MISC41"] = MT_MISC41;
    lua["MT_MISC42"] = MT_MISC42;
    lua["MT_MISC43"] = MT_MISC43;
    lua["MT_MISC44"] = MT_MISC44;
    lua["MT_MISC45"] = MT_MISC45;
    lua["MT_MISC46"] = MT_MISC46;
    lua["MT_MISC47"] = MT_MISC47;
    lua["MT_MISC48"] = MT_MISC48;
    lua["MT_MISC49"] = MT_MISC49;
    lua["MT_MISC50"] = MT_MISC50;
    lua["MT_MISC51"] = MT_MISC51;
    lua["MT_MISC52"] = MT_MISC52;
    lua["MT_MISC53"] = MT_MISC53;
    lua["MT_MISC54"] = MT_MISC54;
    lua["MT_MISC56"] = MT_MISC56;
    lua["MT_MISC57"] = MT_MISC57;
    lua["MT_MISC58"] = MT_MISC58;
    lua["MT_MISC55"] = MT_MISC55;
    lua["MT_MISC59"] = MT_MISC59;
    lua["MT_MISC60"] = MT_MISC60;
    lua["MT_MISC_BLOODHOOK"] = MT_MISC_BLOODHOOK;
    lua["MT_MISC_HANG_LAMP"] = MT_MISC_HANG_LAMP;
    lua["MT_MISC61"] = MT_MISC61;
    lua["MT_MISC63"] = MT_MISC63;
    lua["MT_MISC64"] = MT_MISC64;
    lua["MT_MISC66"] = MT_MISC66;
    lua["MT_MISC67"] = MT_MISC67;
    lua["MT_MISC68"] = MT_MISC68;
    lua["MT_MISC69"] = MT_MISC69;
    lua["MT_MISC70"] = MT_MISC70;
    lua["MT_MISC73"] = MT_MISC73;
    lua["MT_MISC71"] = MT_MISC71;
    lua["MT_MISC72"] = MT_MISC72;
    lua["MT_MISC74"] = MT_MISC74;
    lua["MT_MISC75"] = MT_MISC75;
    lua["MT_MISC76"] = MT_MISC76;
    lua["MT_MISC77"] = MT_MISC77;
    lua["MT_MISC78"] = MT_MISC78;
    lua["MT_MISC79"] = MT_MISC79;
    lua["MT_MISC80"] = MT_MISC80;
    lua["MT_MISC81"] = MT_MISC81;
    lua["MT_MISC82"] = MT_MISC82;
    lua["MT_MISC83"] = MT_MISC83;
    lua["MT_MISC84"] = MT_MISC84;
    lua["MT_MISC85"] = MT_MISC85;
    lua["MT_MISC86"] = MT_MISC86;
    lua["MT_MISC87"] = MT_MISC87;
    lua["MT_VILE"] = MT_VILE;
    lua["MT_FIRE"] = MT_FIRE;
    lua["MT_WOLFSS"] = MT_WOLFSS;
    lua["MT_KEEN"] = MT_KEEN;
    lua["MT_BOSSBRAIN"] = MT_BOSSBRAIN;
    lua["MT_BOSSSPIT"] = MT_BOSSSPIT;
    lua["MT_BOSSTARGET"] = MT_BOSSTARGET;
    lua["MT_SPAWNSHOT"] = MT_SPAWNSHOT;
    lua["MT_SPAWNFIRE"] = MT_SPAWNFIRE;

    lua["sfx_sgcock"] = sfx_sgcock;
    lua["sfx_punch"] = sfx_punch;
    lua["sfx_itmbk"] = sfx_itmbk;
    lua["sfx_firsht2"] = sfx_firsht2;
    lua["sfx_barexp"] = sfx_barexp;
    lua["sfx_firxpl"] = sfx_firxpl;
    lua["sfx_pistol"] = sfx_pistol;
    lua["sfx_shotgn"] = sfx_shotgn;
    lua["sfx_plasma"] = sfx_plasma;
    lua["sfx_bfg"] = sfx_bfg;
    lua["sfx_sawup"] = sfx_sawup;
    lua["sfx_sawidl"] = sfx_sawidl;
    lua["sfx_sawful"] = sfx_sawful;
    lua["sfx_sawhit"] = sfx_sawhit;
    lua["sfx_rlaunc"] = sfx_rlaunc;
    lua["sfx_rxplod"] = sfx_rxplod;
    lua["sfx_pstart"] = sfx_pstart;
    lua["sfx_pstop"] = sfx_pstop;
    lua["sfx_doropn"] = sfx_doropn;
    lua["sfx_dorcls"] = sfx_dorcls;
    lua["sfx_stnmov"] = sfx_stnmov;
    lua["sfx_swtchn"] = sfx_swtchn;
    lua["sfx_swtchx"] = sfx_swtchx;
    lua["sfx_itemup"] = sfx_itemup;
    lua["sfx_wpnup"] = sfx_wpnup;
    lua["sfx_oof"] = sfx_oof;
    lua["sfx_telept"] = sfx_telept;
    lua["sfx_noway"] = sfx_noway;
    lua["sfx_dshtgn"] = sfx_dshtgn;
    lua["sfx_dbopn"] = sfx_dbopn;
    lua["sfx_dbload"] = sfx_dbload;
    lua["sfx_dbcls"] = sfx_dbcls;
    lua["sfx_plpain"] = sfx_plpain;
    lua["sfx_pldeth"] = sfx_pldeth;
    lua["sfx_slop"] = sfx_slop;
    lua["sfx_posit1"] = sfx_posit1;
    lua["sfx_posit2"] = sfx_posit2;
    lua["sfx_posit3"] = sfx_posit3;
    lua["sfx_podth1"] = sfx_podth1;
    lua["sfx_podth2"] = sfx_podth2;
    lua["sfx_podth3"] = sfx_podth3;
    lua["sfx_posact"] = sfx_posact;
    lua["sfx_popain"] = sfx_popain;
    lua["sfx_dmpain"] = sfx_dmpain;
    lua["sfx_dmact"] = sfx_dmact;
    lua["sfx_claw"] = sfx_claw;
    lua["sfx_bgsit1"] = sfx_bgsit1;
    lua["sfx_bgsit2"] = sfx_bgsit2;
    lua["sfx_bgdth1"] = sfx_bgdth1;
    lua["sfx_bgdth2"] = sfx_bgdth2;
    lua["sfx_bgact"] = sfx_bgact;
    lua["sfx_sgtsit"] = sfx_sgtsit;
    lua["sfx_sgtatk"] = sfx_sgtatk;
    lua["sfx_sgtdth"] = sfx_sgtdth;
    lua["sfx_brssit"] = sfx_brssit;
    lua["sfx_brsdth"] = sfx_brsdth;
    lua["sfx_cacsit"] = sfx_cacsit;
    lua["sfx_cacdth"] = sfx_cacdth;
    lua["sfx_sklatk"] = sfx_sklatk;
    lua["sfx_skldth"] = sfx_skldth;
    lua["sfx_kntsit"] = sfx_kntsit;
    lua["sfx_kntdth"] = sfx_kntdth;
    lua["sfx_pesit"] = sfx_pesit;
    lua["sfx_pepain"] = sfx_pepain;
    lua["sfx_pedth"] = sfx_pedth;
    lua["sfx_bspsit"] = sfx_bspsit;
    lua["sfx_bspdth"] = sfx_bspdth;
    lua["sfx_bspact"] = sfx_bspact;
    lua["sfx_bspwlk"] = sfx_bspwlk;
    lua["sfx_manatk"] = sfx_manatk;
    lua["sfx_mansit"] = sfx_mansit;
    lua["sfx_mnpain"] = sfx_mnpain;
    lua["sfx_mandth"] = sfx_mandth;
    lua["sfx_firsht"] = sfx_firsht;
    lua["sfx_skesit"] = sfx_skesit;
    lua["sfx_skedth"] = sfx_skedth;
    lua["sfx_skeact"] = sfx_skeact;
    lua["sfx_skeatk"] = sfx_skeatk;
    lua["sfx_skeswg"] = sfx_skeswg;
    lua["sfx_skepch"] = sfx_skepch;
    lua["sfx_cybsit"] = sfx_cybsit;
    lua["sfx_cybdth"] = sfx_cybdth;
    lua["sfx_hoof"] = sfx_hoof;
    lua["sfx_metal"] = sfx_metal;
    lua["sfx_spisit"] = sfx_spisit;
    lua["sfx_spidth"] = sfx_spidth;
    lua["sfx_bdopn"] = sfx_bdopn;
    lua["sfx_bdcls"] = sfx_bdcls;
    lua["sfx_getpow"] = sfx_getpow;
    lua["sfx_vilsit"] = sfx_vilsit;
    lua["sfx_vipain"] = sfx_vipain;
    lua["sfx_vildth"] = sfx_vildth;
    lua["sfx_vilact"] = sfx_vilact;
    lua["sfx_vilatk"] = sfx_vilatk;
    lua["sfx_flamst"] = sfx_flamst;
    lua["sfx_flame"] = sfx_flame;
    lua["sfx_sssit"] = sfx_sssit;
    lua["sfx_ssdth"] = sfx_ssdth;
    lua["sfx_keenpn"] = sfx_keenpn;
    lua["sfx_keendt"] = sfx_keendt;
    lua["sfx_bossit"] = sfx_bossit;
    lua["sfx_bospit"] = sfx_bospit;
    lua["sfx_bospn"] = sfx_bospn;
    lua["sfx_bosdth"] = sfx_bosdth;
    lua["sfx_boscub"] = sfx_boscub;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Registers all Lua scripting bindings: types, functions and constants
//------------------------------------------------------------------------------------------------------------------------------------------
void registerAll(sol::state& lua) noexcept {
    registerLuaTypes(lua);
    registerLuaFunctions(lua);
    registerLuaConstants(lua);
}

END_NAMESPACE(ScriptBindings)
