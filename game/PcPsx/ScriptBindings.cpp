#include "ScriptBindings.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/p_ceiling.h"
#include "Doom/Game/p_doors.h"
#include "Doom/Game/p_floor.h"
#include "Doom/Game/p_inter.h"
#include "Doom/Game/p_local.h"
#include "Doom/Game/p_map.h"
#include "Doom/Game/p_maputl.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_plats.h"
#include "Doom/Game/p_pspr.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_sight.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Game/p_switch.h"
#include "Doom/Game/p_telept.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/st_main.h"
#include "ScriptingEngine.h"

#include <algorithm>
#include <cstdio>
#include <optional>
#include <sol/sol.hpp>

BEGIN_NAMESPACE(ScriptBindings)

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: makes a lua type's table readonly as much as possible
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void makeTypeReadOnly(sol::usertype<T>& typeTable) noexcept {
    typeTable[sol::meta_function::new_index] = [](lua_State* const L) -> int {
        return luaL_error(L, "Value is read only!");
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: visits sectors surrounding a specified sector, invoking the specified lambda on each one
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void visitSurroundingSectors(sector_t& sector, const T& callback) noexcept {
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
// Helper: gets the return type of a sector 'value getter' function
//------------------------------------------------------------------------------------------------------------------------------------------
template <typename GetFnT>
using SectorGetterRetT = decltype(std::declval<GetFnT>()(sector_t{}));

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function: visits sectors surrounding a given sector and extracts a value for each sector.
// The value is filtered using the specified filter/predicate and multiple values are combined using the specified reduce function.
// Can be used to search for minimum or maximum values in surrounding sectors.
//------------------------------------------------------------------------------------------------------------------------------------------
template <typename GetFnT, typename FilterFnT, typename ReduceFnT>
static std::optional<SectorGetterRetT<GetFnT>> surroundingSectorsFilterAndReduceValue(
    sector_t& sector,
    const GetFnT getValue,
    const FilterFnT& filterValue,
    const ReduceFnT& reduceTwoValues
) noexcept {
    typedef SectorGetterRetT<GetFnT> ValT;
    std::optional<ValT> result;

    visitSurroundingSectors(
        sector,
        [&](const sector_t& surroundingSector) noexcept {
            const ValT value = getValue(surroundingSector);

            if (filterValue(value)) {
                if (result.has_value()) {
                    result = reduceTwoValues(result.value(), value);
                } else {
                    result = value;
                }
            }
        }
    );

    return result;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helpers: get the minimum, maximum, next lowest, or next highest value of a sector property from sectors surrounding a sector
//------------------------------------------------------------------------------------------------------------------------------------------
template <typename GetFnT>
static std::optional<SectorGetterRetT<GetFnT>> getSurroundingSectorsMinValue(sector_t& sector, const GetFnT& getValue) noexcept {
    typedef SectorGetterRetT<GetFnT> ValT;

    return surroundingSectorsFilterAndReduceValue(
        sector,
        getValue,
        []([[maybe_unused]] const ValT v) noexcept { return true; },
        [](const ValT v1, const ValT v2) noexcept { return std::min(v1, v2); }
    );
}

template <typename GetFnT>
static std::optional<SectorGetterRetT<GetFnT>> getSurroundingSectorsMaxValue(sector_t& sector, const GetFnT& getValue) noexcept {
    typedef SectorGetterRetT<GetFnT> ValT;

    return surroundingSectorsFilterAndReduceValue(
        sector,
        getValue,
        []([[maybe_unused]] const ValT v) noexcept { return true; },
        [](const ValT v1, const ValT v2) noexcept { return std::max(v1, v2); }
    );
}

template <typename GetFnT>
static std::optional<SectorGetterRetT<GetFnT>> getSurroundingSectorsNextLowestValue(
    sector_t& sector,
    const GetFnT& getValue,
    const SectorGetterRetT<GetFnT> lowerThanValue
) noexcept {
    typedef SectorGetterRetT<GetFnT> ValT;

    return surroundingSectorsFilterAndReduceValue(
        sector,
        getValue,
        [=](const ValT v) noexcept { return (v < lowerThanValue); },
        [](const ValT v1, const ValT v2) noexcept { return std::max(v1, v2); }
    );
}

template <typename GetFnT>
static std::optional<SectorGetterRetT<GetFnT>> getSurroundingSectorsNextHighestValue(
    sector_t& sector,
    const GetFnT& getValue,
    const SectorGetterRetT<GetFnT> higherThanValue
) noexcept {
    typedef SectorGetterRetT<GetFnT> ValT;

    return surroundingSectorsFilterAndReduceValue(
        sector,
        getValue,
        [=](const ValT v) noexcept { return (v > higherThanValue); },
        [](const ValT v1, const ValT v2) noexcept { return std::min(v1, v2); }
    );
}

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
// Helper: does a 'key needed' status bar flash for the specified player.
// N.B: the specified message string must exist for the lifetime of the flash.
//------------------------------------------------------------------------------------------------------------------------------------------
static void doKeyFlash(player_t& player, const card_t keyType, const char* const msg) noexcept {
    player_t& curPlayer = gPlayers[gCurPlayerIndex];
    player.message = msg;

    if (&player == &curPlayer) {
        gStatusBar.tryopen[keyType] = true;
    }
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

static void StatusMessage(const char* const message) noexcept {
    // Use the level startup warning buffer since it's always around
    std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "%s", message);
    player_t& player = gPlayers[gCurPlayerIndex];
    player.message = gLevelStartupWarning;
}

static void KeyFlash_Red(player_t& player) noexcept {
    doKeyFlash(player, gMapRedKeyType, "You need a red key.");
}

static void KeyFlash_Blue(player_t& player) noexcept {
    doKeyFlash(player, gMapBlueKeyType, "You need a blue key.");
}

static void KeyFlash_Yellow(player_t& player) noexcept {
    doKeyFlash(player, gMapYellowKeyType, "You need a yellow key.");
}

static float ApproxLength(const float dx, const float dy) noexcept {
    const fixed_t dxFrac = FloatToFixed(dx);
    const fixed_t dyFrac = FloatToFixed(dy);
    return FixedToFloat(P_AproxDistance(dxFrac, dyFrac));
}

static float ApproxDistance(const float x1, const float y1, const float x2, const float y2) noexcept {
    const fixed_t x1Frac = FloatToFixed(x1);
    const fixed_t y1Frac = FloatToFixed(y1);
    const fixed_t x2Frac = FloatToFixed(x2);
    const fixed_t y2Frac = FloatToFixed(y2);
    const fixed_t dxFrac = x2Frac - x1Frac;
    const fixed_t dyFrac = y2Frac - y1Frac;
    return FixedToFloat(P_AproxDistance(dxFrac, dyFrac));
}

static float AngleToPoint(const float x1, const float y1, const float x2, const float y2) noexcept {
    const fixed_t x1Frac = FloatToFixed(x1);
    const fixed_t y1Frac = FloatToFixed(y1);
    const fixed_t x2Frac = FloatToFixed(x2);
    const fixed_t y2Frac = FloatToFixed(y2);
    return AngleToDegrees(R_PointToAngle2(x1Frac, y1Frac, x2Frac, y2Frac));
}

static void AlertMessage(const char* const message, const uint32_t numTics, const uint32_t soundId) noexcept {
    ST_AlertMessage(message, numTics);

    if (soundId != 0) {
        S_StartSound(nullptr, (sfxenum_t) soundId);
    }
}

static void DoCamera(const float x, const float y, const float z, const float angle, const uint32_t numTics) noexcept {
    gExtCameraX = FloatToFixed(x);
    gExtCameraY = FloatToFixed(y);
    gExtCameraZ = FloatToFixed(z);
    gExtCameraAngle = DegreesToAngle(angle);
    gExtCameraTicsLeft = numTics;
}

static void StopCamera() noexcept {
    gExtCameraTicsLeft = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helpers: extract the floor or ceiling height (float format) or the light level of a sector
//------------------------------------------------------------------------------------------------------------------------------------------
static float getSectorFloorH(const sector_t& sector) noexcept { return FixedToFloat(sector.floorheight); }
static float getSectorCeilingH(const sector_t& sector) noexcept { return FixedToFloat(sector.ceilingheight); }
static int32_t getSectorLightLevel(const sector_t& sector) noexcept { return sector.lightlevel; }

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

static void ForEachMobjInSector(sector_t& sector, const std::function<void (mobj_t& mo)>& callback) noexcept {
    if (!callback)
        return;

    for (mobj_t* pMobj = sector.thinglist; pMobj;) {
        mobj_t* const pNextMobj = pMobj->snext;     // Just to be a bit safer...
        callback(*pMobj);
        pMobj = pNextMobj;
    }
}

static void ForEachSurroundingSector(sector_t& sector, const std::function<void (sector_t& sector)>& callback) noexcept {
    if (callback) {
        visitSurroundingSectors(sector, callback);
    }
}

static float GetLowestSurroundingFloor(sector_t& sector, const float defaultValue) noexcept {
    return getSurroundingSectorsMinValue(sector, getSectorFloorH).value_or(defaultValue);
}

static float GetLowestSurroundingCeiling(sector_t& sector, const float defaultValue) noexcept {
    return getSurroundingSectorsMinValue(sector, getSectorCeilingH).value_or(defaultValue);
}

static int32_t GetLowestSurroundingLightLevel(sector_t& sector, const int32_t defaultValue) noexcept {
    return getSurroundingSectorsMinValue(sector, getSectorLightLevel).value_or(defaultValue);
}

static float GetHighestSurroundingFloor(sector_t& sector, const float defaultValue) noexcept {
    return getSurroundingSectorsMaxValue(sector, getSectorFloorH).value_or(defaultValue);
}

static float GetHighestSurroundingCeiling(sector_t& sector, const float defaultValue) noexcept {
    return getSurroundingSectorsMaxValue(sector, getSectorCeilingH).value_or(defaultValue);
}

static int32_t GetHighestSurroundingLightLevel(sector_t& sector, const int32_t defaultValue) noexcept {
    return getSurroundingSectorsMaxValue(sector, getSectorLightLevel).value_or(defaultValue);
}

static float GetNextLowestSurroundingFloor(sector_t& sector, const float lowerThanValue, const float defaultValue) noexcept {
    return getSurroundingSectorsNextLowestValue(sector, getSectorFloorH, lowerThanValue).value_or(defaultValue);
}

static float GetNextLowestSurroundingCeiling(sector_t& sector, const float lowerThanValue, const float defaultValue) noexcept {
    return getSurroundingSectorsNextLowestValue(sector, getSectorCeilingH, lowerThanValue).value_or(defaultValue);
}

static int32_t GetNextLowestSurroundingLightLevel(sector_t& sector, const int32_t lowerThanValue, const int32_t defaultValue) noexcept {
    return getSurroundingSectorsNextLowestValue(sector, getSectorLightLevel, lowerThanValue).value_or(defaultValue);
}

static float GetNextHighestSurroundingFloor(sector_t& sector, const float higherThanValue, const float defaultValue) noexcept {
    return getSurroundingSectorsNextHighestValue(sector, getSectorFloorH, higherThanValue).value_or(defaultValue);
}

static float GetNextHighestSurroundingCeiling(sector_t& sector, const float higherThanValue, const float defaultValue) noexcept {
    return getSurroundingSectorsNextHighestValue(sector, getSectorCeilingH, higherThanValue).value_or(defaultValue);
}

static int32_t GetNextHighestSurroundingLightLevel(sector_t& sector, const int32_t higherThanValue, const int32_t defaultValue) noexcept {
    return getSurroundingSectorsNextHighestValue(sector, getSectorLightLevel, higherThanValue).value_or(defaultValue);
}

static sector_t* SectorAtPosition(const float x, const float y) noexcept {
    return R_PointInSubsector(FloatToFixed(x), FloatToFixed(y))->sector;
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

int32_t Script_P_PointOnLineSide(const float x, const float y, const line_t& line) noexcept {
    return P_PointOnLineSide(FloatToFixed(x), FloatToFixed(y), line);
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
static void ForEachMobj(const std::function<void (mobj_t& mo)>& callback) noexcept {
    if (!callback)
        return;

    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead;) {
        mobj_t* const pNextMobj = pMobj->next;      // Just to be a bit safer...
        callback(*pMobj);
        pMobj = pNextMobj;
    }
}

static void ForEachMobjInArea(
    const float x1,
    const float y1,
    const float x2,
    const float y2,
    const std::function<void (mobj_t& mo)>& callback
) noexcept {
    if (!callback)
        return;

    // Figure out what cells in the blockmap we will cover
    const fixed_t xmin = FloatToFixed(std::min(x1, x2));
    const fixed_t xmax = FloatToFixed(std::max(x1, x2));
    const fixed_t ymin = FloatToFixed(std::min(y1, y2));
    const fixed_t ymax = FloatToFixed(std::max(y1, y2));

    const int32_t bmapW = gBlockmapWidth;
    const int32_t bmapH = gBlockmapHeight;

    const int32_t bmapTy = std::min(d_rshift<MAPBLOCKSHIFT>(ymax - gBlockmapOriginY), bmapH - 1);
    const int32_t bmapBy = std::max(d_rshift<MAPBLOCKSHIFT>(ymin - gBlockmapOriginY), 0);
    const int32_t bmapLx = std::max(d_rshift<MAPBLOCKSHIFT>(xmin - gBlockmapOriginX), 0);
    const int32_t bmapRx = std::min(d_rshift<MAPBLOCKSHIFT>(xmax - gBlockmapOriginX), bmapW - 1);

    // Go through all of the blockmap cells of interest, calling the callback on each thing found
    for (int32_t bmapY = bmapBy; bmapY <= bmapTy; ++bmapY) {
        for (int32_t bmapX = bmapLx; bmapX <= bmapRx; ++bmapX) {
            mobj_t* pmobj = gppBlockLinks[bmapX + bmapY * bmapW];

            while (pmobj) {
                mobj_t* const pNextMobj = pmobj->bnext;     // Just to be safe
                callback(*pmobj);
                pmobj = pNextMobj;
            }
        }
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

void Script_P_RemoveMobj(mobj_t& mobj) noexcept {
    // Put the thing into the S_NULL state and schedule it to be deleted later (don't delete immediately)
    state_t& nullState = gStates[S_NULL];

    mobj.state = &nullState;
    mobj.tics = nullState.tics;
    mobj.sprite = nullState.sprite;
    mobj.frame = nullState.frame;
    mobj.latecall = P_RemoveMobj;

    // Tell the scripting engine that it needs to clean up some things
    ScriptingEngine::gbNeedMobjGC = true;
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

static bool Script_P_CheckPosition(mobj_t& mobj, const float x, const float y) noexcept {
    return P_CheckPosition(mobj, FloatToFixed(x), FloatToFixed(y));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: Players
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t GetNumPlayers() noexcept {
    return (gNetGame == gt_single) ? 1 : 2;
}

static player_t& GetCurPlayer() noexcept {
    return gPlayers[gCurPlayerIndex];
}

static player_t* GetPlayer(const int32_t index) {
    return ((index >= 0) && (index < GetNumPlayers())) ? &gPlayers[index] : nullptr;
}

static int32_t Player_GetPowerTicsLeft(player_t& player, const int32_t powerIdx) noexcept {
    return ((powerIdx >= 0) && (powerIdx < NUMPOWERS)) ? player.powers[powerIdx] : 0;
}

static bool Player_HasCard(player_t& player, const int32_t cardIdx) noexcept {
    return ((cardIdx >= 0) && (cardIdx < NUMCARDS)) ? player.cards[cardIdx] : false;
}

static bool Player_IsWeaponOwned(player_t& player, const int32_t weaponIdx) noexcept {
    return ((weaponIdx >= 0) && (weaponIdx < NUMWEAPONS)) ? player.weaponowned[weaponIdx] : false;
}

static int32_t Player_GetAmmo(player_t& player, const int32_t ammoTypeIdx) noexcept {
    return ((ammoTypeIdx >= 0) && (ammoTypeIdx < NUMAMMO)) ? player.ammo[ammoTypeIdx] : 0;
}

static int32_t Player_GetMaxAmmo(player_t& player, const int32_t ammoTypeIdx) noexcept {
    return ((ammoTypeIdx >= 0) && (ammoTypeIdx < NUMAMMO)) ? player.maxammo[ammoTypeIdx] : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: Script execution context
//------------------------------------------------------------------------------------------------------------------------------------------
static line_t* GetTriggeringLine() noexcept { return ScriptingEngine::gpCurTriggeringLine; }
static sector_t* GetTriggeringSector() noexcept { return ScriptingEngine::gpCurTriggeringSector; }
static mobj_t* GetTriggeringMobj() noexcept { return ScriptingEngine::gpCurTriggeringMobj; }
static int32_t GetCurActionTag() noexcept { return ScriptingEngine::gCurActionTag; }
static int32_t GetCurActionUserdata() noexcept { return ScriptingEngine::gCurActionUserdata; }
static void SetLineActionAllowed(const bool bAllowed) noexcept { ScriptingEngine::gbCurActionAllowed = bAllowed; }

//------------------------------------------------------------------------------------------------------------------------------------------
// Script API: moving floors, ceilings and platforms
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
void S_PlaySoundAtMobj(mobj_t* const pOrigin, const uint32_t soundId) noexcept {
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
    type["hasthinker"] = sol::readonly_property([](const sector_t& sector){ return (sector.specialdata != nullptr); });
    type.set_function("GetLine", GetLineInSector);
    type.set_function("ForEachLine", ForEachLineInSector);
    type.set_function("ForEachMobj", ForEachMobjInSector);
    type.set_function("ForEachSurroundingSector", ForEachSurroundingSector);
    type.set_function("GetLowestSurroundingFloor", GetLowestSurroundingFloor);
    type.set_function("GetLowestSurroundingCeiling", GetLowestSurroundingCeiling);
    type.set_function("GetLowestSurroundingLightLevel", GetLowestSurroundingLightLevel);
    type.set_function("GetHighestSurroundingFloor", GetHighestSurroundingFloor);
    type.set_function("GetHighestSurroundingCeiling", GetHighestSurroundingCeiling);
    type.set_function("GetHighestSurroundingLightLevel", GetHighestSurroundingLightLevel);
    type.set_function("GetNextLowestSurroundingFloor", GetNextLowestSurroundingFloor);
    type.set_function("GetNextLowestSurroundingCeiling", GetNextLowestSurroundingCeiling);
    type.set_function("GetNextLowestSurroundingLightLevel", GetNextLowestSurroundingLightLevel);
    type.set_function("GetNextHighestSurroundingFloor", GetNextHighestSurroundingFloor);
    type.set_function("GetNextHighestSurroundingCeiling", GetNextHighestSurroundingCeiling);
    type.set_function("GetNextHighestSurroundingLightLevel", GetNextHighestSurroundingLightLevel);

    makeTypeReadOnly(type);
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

    makeTypeReadOnly(type);
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

    makeTypeReadOnly(type);
}

static void registerType_mobj_t(sol::state& lua) noexcept {
    sol::usertype<mobj_t> type = lua.new_usertype<mobj_t>("mobj_t", sol::no_constructor);

    type["x"] = SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(mobj_t, x);
    type["y"] = SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(mobj_t, y);
    type["z"] = SOL_READONLY_FIXED_PROPERTY_AS_FLOAT(mobj_t, z);
    type["tag"] = &mobj_t::tag;
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
    type["target"] = &mobj_t::target;
    type["tracer"] = &mobj_t::tracer;
    type["player"] = sol::readonly(&mobj_t::player);

    makeTypeReadOnly(type);
}

static void registerType_player_t(sol::state& lua) noexcept {
    sol::usertype<player_t> type = lua.new_usertype<player_t>("player_t", sol::no_constructor);

    type["mo"] = sol::readonly(&player_t::mo);
    type["health"] = sol::readonly(&player_t::health);
    type["armorpoints"] = sol::readonly(&player_t::armorpoints);
    type["armortype"] = sol::readonly(&player_t::armortype);
    type["backpack"] = sol::readonly(&player_t::backpack);
    type["frags"] = sol::readonly(&player_t::frags);
    type["killcount"] = sol::readonly(&player_t::killcount);
    type["itemcount"] = sol::readonly(&player_t::itemcount);
    type["secretcount"] = sol::readonly(&player_t::secretcount);
    type["readyweapon"] = sol::readonly_property([](const player_t& p) noexcept { return (uint32_t) p.readyweapon; });
    type["pendingweapon"] = sol::readonly_property([](const player_t& p) noexcept { return (uint32_t) p.pendingweapon; });

    type.set_function("GetPowerTicsLeft", Player_GetPowerTicsLeft);
    type.set_function("HasCard", Player_HasCard);
    type.set_function("IsWeaponOwned", Player_IsWeaponOwned);
    type.set_function("GetAmmo", Player_GetAmmo);
    type.set_function("GetMaxAmmo", Player_GetMaxAmmo);

    makeTypeReadOnly(type);
}

static void registerType_CustomFloorDef(sol::state& lua) noexcept {
    sol::usertype<CustomFloorDef> type = lua.new_usertype<CustomFloorDef>("CustomFloorDef", sol::default_constructor);

    type["crush"] = &CustomFloorDef::bCrush;
    type["dofinishscript"] = &CustomFloorDef::bDoFinishScript;
    type["destheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomFloorDef, destHeight);
    type["speed"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomFloorDef, speed);
    type["startsound"] = &CustomFloorDef::startSound;
    type["movesound"] = &CustomFloorDef::moveSound;
    type["movesoundfreq"] = &CustomFloorDef::moveSoundFreq;
    type["stopsound"] = &CustomFloorDef::stopSound;
    type["finishscript_actionnum"] = &CustomFloorDef::finishScriptActionNum;
    type["finishscript_userdata"] = &CustomFloorDef::finishScriptUserdata;

    makeTypeReadOnly(type);
}

static void registerType_CustomPlatDef(sol::state& lua) noexcept {
    sol::usertype<CustomPlatDef> type = lua.new_usertype<CustomPlatDef>("CustomPlatDef", sol::default_constructor);

    type["crush"] = &CustomPlatDef::bCrush;
    type["dofinishscript"] = &CustomPlatDef::bDoFinishScript;
    type["startstate"] = &CustomPlatDef::startState;
    type["finishstate"] = &CustomPlatDef::finishState;
    type["minheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomPlatDef, minHeight);
    type["maxheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomPlatDef, maxHeight);
    type["speed"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomPlatDef, speed);
    type["waittime"] = &CustomPlatDef::waitTime;
    type["startsound"] = &CustomPlatDef::startSound;
    type["movesound"] = &CustomPlatDef::moveSound;
    type["movesoundfreq"] = &CustomPlatDef::moveSoundFreq;
    type["stopsound"] = &CustomPlatDef::stopSound;
    type["finishscript_actionnum"] = &CustomPlatDef::finishScriptActionNum;
    type["finishscript_userdata"] = &CustomPlatDef::finishScriptUserdata;

    makeTypeReadOnly(type);
}

static void registerType_CustomCeilingDef(sol::state& lua) noexcept {
    sol::usertype<CustomCeilingDef> type = lua.new_usertype<CustomCeilingDef>("CustomCeilingDef", sol::default_constructor);

    type["crush"] = &CustomCeilingDef::bCrush;
    type["dofinishscript"] = &CustomCeilingDef::bDoFinishScript;
    type["minheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomCeilingDef, minHeight);
    type["maxheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomCeilingDef, maxHeight);
    type["startdir"] = &CustomCeilingDef::startDir;
    type["normalspeed"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomCeilingDef, normalSpeed);
    type["crushspeed"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomCeilingDef, crushSpeed);
    type["numdirchanges"] = &CustomCeilingDef::numDirChanges;
    type["startsound"] = &CustomCeilingDef::startSound;
    type["movesound"] = &CustomCeilingDef::moveSound;
    type["movesoundfreq"] = &CustomCeilingDef::moveSoundFreq;
    type["changedirsound"] = &CustomCeilingDef::changeDirSound;
    type["stopsound"] = &CustomCeilingDef::stopSound;
    type["finishscript_actionnum"] = &CustomCeilingDef::finishScriptActionNum;
    type["finishscript_userdata"] = &CustomCeilingDef::finishScriptUserdata;

    makeTypeReadOnly(type);
}

static void registerType_CustomDoorDef(sol::state& lua) noexcept {
    sol::usertype<CustomDoorDef> type = lua.new_usertype<CustomDoorDef>("CustomDoorDef", sol::default_constructor);

    type["open"] = &CustomDoorDef::bOpen;
    type["doreturn"] = &CustomDoorDef::bDoReturn;
    type["blockable"] = &CustomDoorDef::bBlockable;
    type["dofinishscript"] = &CustomDoorDef::bDoFinishScript;
    type["minheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomDoorDef, minHeight);
    type["maxheight"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomDoorDef, maxHeight);
    type["speed"] = SOL_FIXED_PROPERTY_AS_FLOAT(CustomDoorDef, speed);
    type["waittime"] = &CustomDoorDef::waitTime;
    type["opensound"] = &CustomDoorDef::openSound;
    type["closesound"] = &CustomDoorDef::closeSound;
    type["finishscript_actionnum"] = &CustomDoorDef::finishScriptActionNum;
    type["finishscript_userdata"] = &CustomDoorDef::finishScriptUserdata;

    makeTypeReadOnly(type);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Registers all Lua scripting types
//------------------------------------------------------------------------------------------------------------------------------------------
static void registerLuaTypes(sol::state& lua) noexcept {
    registerType_sector_t(lua);
    registerType_line_t(lua);
    registerType_side_t(lua);
    registerType_mobj_t(lua);
    registerType_player_t(lua);
    registerType_CustomFloorDef(lua);
    registerType_CustomPlatDef(lua);
    registerType_CustomCeilingDef(lua);
    registerType_CustomDoorDef(lua);
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
    lua["AlertMessage"] = AlertMessage;
    lua["KeyFlash_Red"] = KeyFlash_Red;
    lua["KeyFlash_Blue"] = KeyFlash_Blue;
    lua["KeyFlash_Yellow"] = KeyFlash_Yellow;
    lua["ApproxLength"] = ApproxLength;
    lua["ApproxDistance"] = ApproxDistance;
    lua["AngleToPoint"] = AngleToPoint;
    lua["IsSinglePlayerGame"] = []{ return (gNetGame == gt_single); };
    lua["IsCoopGame"] = []{ return (gNetGame == gt_coop); };
    lua["IsDeathmatchGame"] = []{ return (gNetGame == gt_deathmatch); };
    lua["StatusMessage"] = StatusMessage;
    lua["DoCamera"] = DoCamera;
    lua["StopCamera"] = StopCamera;

    lua["ScheduleAction"] = ScriptingEngine::scheduleAction;
    lua["ScheduleRepeatingAction"] = ScriptingEngine::scheduleRepeatingAction;
    lua["StopAllScheduledActions"] = ScriptingEngine::stopAllScheduledActions;
    lua["StopScheduledActionsWithTag"] = ScriptingEngine::stopScheduledActionsWithTag;
    lua["PauseAllScheduledActions"] = ScriptingEngine::pauseAllScheduledActions;
    lua["PauseScheduledActionsWithTag"] = ScriptingEngine::pauseScheduledActionsWithTag;
    lua["GetNumScheduledActionsWithTag"] = ScriptingEngine::getNumScheduledActionsWithTag;

    lua["GetNumSectors"] = GetNumSectors;
    lua["GetSector"] = GetSector;
    lua["FindSectorWithTag"] = FindSectorWithTag;
    lua["ForEachSector"] = ForEachSector;
    lua["ForEachSectorWithTag"] = ForEachSectorWithTag;
    lua["SectorAtPosition"] = SectorAtPosition;
    
    lua["GetNumLines"] = GetNumLines;
    lua["GetLine"] = GetLine;
    lua["FindLineWithTag"] = FindLineWithTag;
    lua["ForEachLine"] = ForEachLine;
    lua["ForEachLineWithTag"] = ForEachLineWithTag;
    lua["P_PointOnLineSide"] = Script_P_PointOnLineSide;
    lua["P_CrossSpecialLine"] = Script_P_CrossSpecialLine;
    lua["P_ShootSpecialLine"] = Script_P_ShootSpecialLine;
    lua["P_UseSpecialLine"] = Script_P_UseSpecialLine;
    lua["P_ChangeSwitchTexture"] = P_ChangeSwitchTexture;

    lua["GetNumPlayers"] = GetNumPlayers;
    lua["GetCurPlayer"] = GetCurPlayer;
    lua["GetPlayer"] = GetPlayer;

    lua["GetNumSides"] = GetNumSides;
    lua["GetSide"] = GetSide;
    lua["ForEachSide"] = ForEachSide;

    lua["ForEachMobj"] = ForEachMobj;
    lua["ForEachMobjInArea"] = ForEachMobjInArea;
    lua["FindMobjTypeForDoomEdNum"] = FindMobjTypeForDoomEdNum;
    lua["P_SpawnMobj"] = Script_P_SpawnMobj;
    lua["P_SpawnMissile"] = Script_P_SpawnMissile;
    lua["P_DamageMobj"] = P_DamageMobj;
    lua["P_RemoveMobj"] = Script_P_RemoveMobj;
    lua["EV_TeleportTo"] = Script_EV_TeleportTo;
    lua["P_NoiseAlert"] = P_NoiseAlertToMobj;
    lua["P_CheckSight"] = P_CheckSight;
    lua["P_CheckPosition"] = Script_P_CheckPosition;

    lua["GetTriggeringLine"] = GetTriggeringLine;
    lua["GetTriggeringSector"] = GetTriggeringSector;
    lua["GetTriggeringMobj"] = GetTriggeringMobj;
    lua["GetCurActionTag"] = GetCurActionTag;
    lua["GetCurActionUserdata"] = GetCurActionUserdata;
    lua["SetLineActionAllowed"] = SetLineActionAllowed;
    
    lua["T_MoveFloor"] = T_MoveFloor;
    lua["T_MoveCeiling"] = T_MoveCeiling;
    lua["EV_DoCustomFloor"] = EV_DoCustomFloor;
    lua["EV_DoCustomPlat"] = EV_DoCustomPlat;
    lua["EV_DoCustomCeiling"] = EV_DoCustomCeiling;
    lua["EV_DoCustomDoor"] = EV_DoCustomDoor;
    lua["P_ActivateInStasisPlatForTag"] = P_ActivateInStasisPlatForTag;
    lua["P_ActivateInStasisPlatForSector"] = P_ActivateInStasisPlatForSector;
    lua["EV_StopPlatForTag"] = EV_StopPlatForTag;
    lua["EV_StopPlatForSector"] = EV_StopPlatForSector;
    lua["P_ActivateInStasisCeilingsForTag"] = P_ActivateInStasisCeilingsForTag;
    lua["P_ActivateInStasisCeilingForSector"] = P_ActivateInStasisCeilingForSector;
    lua["EV_CeilingCrushStopForTag"] = EV_CeilingCrushStopForTag;
    lua["EV_CeilingCrushStopForSector"] = EV_CeilingCrushStopForSector;

    lua["S_PlaySoundAtMobj"] = S_PlaySoundAtMobj;
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

    lua["pw_invulnerability"] = (uint32_t) pw_invulnerability;
    lua["pw_strength"] = (uint32_t) pw_strength;
    lua["pw_invisibility"] = (uint32_t) pw_invisibility;
    lua["pw_ironfeet"] = (uint32_t) pw_ironfeet;
    lua["pw_allmap"] = (uint32_t) pw_allmap;
    lua["pw_infrared"] = (uint32_t) pw_infrared;

    lua["it_redcard"] = (uint32_t) it_redcard;
    lua["it_bluecard"] = (uint32_t) it_bluecard;
    lua["it_yellowcard"] = (uint32_t) it_yellowcard;
    lua["it_redskull"] = (uint32_t) it_redskull;
    lua["it_blueskull"] = (uint32_t) it_blueskull;
    lua["it_yellowskull"] = (uint32_t) it_yellowskull;

    lua["wp_fist"] = (uint32_t) wp_fist;
    lua["wp_pistol"] = (uint32_t) wp_pistol;
    lua["wp_shotgun"] = (uint32_t) wp_shotgun;
    lua["wp_supershotgun"] = (uint32_t) wp_supershotgun;
    lua["wp_chaingun"] = (uint32_t) wp_chaingun;
    lua["wp_missile"] = (uint32_t) wp_missile;
    lua["wp_plasma"] = (uint32_t) wp_plasma;
    lua["wp_bfg"] = (uint32_t) wp_bfg;
    lua["wp_chainsaw"] = (uint32_t) wp_chainsaw;
    lua["wp_nochange"] = (uint32_t) wp_nochange;

    lua["am_clip"] = (uint32_t) am_clip;
    lua["am_shell"] = (uint32_t) am_shell;
    lua["am_cell"] = (uint32_t) am_cell;
    lua["am_misl"] = (uint32_t) am_misl;

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
