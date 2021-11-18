//------------------------------------------------------------------------------------------------------------------------------------------
// High level saving and loading functionality.
// Does not concern itself too much with the details of how each datatype is serialized, for that see 'SaveDataTypes.cpp'.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "SaveAndLoad.h"

#include "Doom/Game/g_game.h"
#include "Doom/Game/p_ceiling.h"
#include "Doom/Game/p_floor.h"
#include "Doom/Game/p_lights.h"
#include "Doom/Game/p_plats.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Game/p_switch.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_local.h"
#include "InputStream.h"
#include "MapHash.h"
#include "OutputStream.h"
#include "SaveDataTypes.h"
#include "ScriptingEngine.h"

BEGIN_NAMESPACE(SaveAndLoad)

// Save/load accelerator LUT: maps from a map object to it's index in the global linked list of map objects
std::unordered_map<mobj_t*, int32_t> gMobjToIdx;

// Save/load accelerator LUT: maps from a map object index to it's pointer
std::vector<mobj_t*> gMobjList;

// LUTS for thinkers of various types: used during saving
static std::vector<vldoor_t*>           gVlDoors;
static std::vector<vlcustomdoor_t*>     gVlCustomDoors;
static std::vector<floormove_t*>        gFloorMovers;
static std::vector<ceiling_t*>          gCeilings;
static std::vector<plat_t*>             gPlats;
static std::vector<fireflicker_t*>      gFireFlickers;
static std::vector<lightflash_t*>       gLightFlashes;
static std::vector<strobe_t*>           gStrobes;
static std::vector<glow_t*>             gGlows;
static std::vector<delayaction_t*>      gDelayedExits;

// A list of buttons that are active: used during saving
static std::vector<button_t*> gActiveButtons;

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears all temporary lists, including map object LUTs and lists of thinkers
//------------------------------------------------------------------------------------------------------------------------------------------
static void clearTempLuts() noexcept {
    gMobjToIdx.clear();
    gMobjList.clear();
    gVlDoors.clear();
    gVlCustomDoors.clear();
    gFloorMovers.clear();
    gCeilings.clear();
    gPlats.clear();
    gFireFlickers.clear();
    gLightFlashes.clear();
    gStrobes.clear();
    gGlows.clear();
    gDelayedExits.clear();
    gActiveButtons.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gathers thinkers of a specified type and think function, saving them in the given list
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ThinkerT>
static void gatherThinkersOfType(void (* const pThinkerFn)(ThinkerT& thinker), std::vector<ThinkerT*>& outputList, const uint32_t reserveAmt) noexcept {
    outputList.clear();
    outputList.reserve(reserveAmt);

    for (thinker_t* pThinker = gThinkerCap.next; pThinker != &gThinkerCap; pThinker = pThinker->next) {
        if ((void*) pThinker->function == (void*) pThinkerFn) {
            outputList.push_back(reinterpret_cast<ThinkerT*>(pThinker));
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gathers delayed actions of the specified type, saving them in the given list
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ThinkerT>
static void gatherDelayedActionsOfType(const delayed_actionfn_t pActionFn, std::vector<ThinkerT*>& outputList, const uint32_t reserveAmt) noexcept {
    outputList.clear();
    outputList.reserve(reserveAmt);

    for (thinker_t* pThinker = gThinkerCap.next; pThinker != &gThinkerCap; pThinker = pThinker->next) {
        if ((void*) pThinker->function == (void*) &T_DelayedAction) {
            delayaction_t& delayedAction = reinterpret_cast<delayaction_t&>(*pThinker);

            if (delayedAction.actionfunc == pActionFn) {
                outputList.push_back(&delayedAction);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gathers a list of currently active buttons, saving them in the specified list
//------------------------------------------------------------------------------------------------------------------------------------------
static void gatherActiveButtons(std::vector<button_t*>& outputList, const uint32_t reserveAmt) noexcept {
    outputList.clear();
    outputList.reserve(reserveAmt);

    for (button_t& btn : gButtonList) {
        if (btn.btimer != 0) {
            outputList.push_back(&btn);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Builds the LUTs for accelerating map object lookups; this is a prerequisite step for saving and loading.
//------------------------------------------------------------------------------------------------------------------------------------------
static void buildMobjLuts(const uint32_t reserveAmt) noexcept {
    gMobjToIdx.clear();
    gMobjList.clear();
    gMobjToIdx.reserve(reserveAmt);
    gMobjList.reserve(reserveAmt);

    int32_t mobjIdx = 0;

    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead; pMobj = pMobj->next, ++mobjIdx) {
        gMobjToIdx[pMobj] = mobjIdx;
        gMobjList.push_back(pMobj);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the header for the save file
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateSaveHeader(SaveFileHdr& hdr) noexcept {
    ASSERT_LOG(gMobjList.size() > 0, "Mobj LUT must be built first!");

    hdr.fileId1 = SAVE_FILE_ID1;
    hdr.fileId2 = SAVE_FILE_ID2;
    hdr.version = SAVE_FILE_VERSION;
    hdr.mapHashWord1 = MapHash::gWord1;
    hdr.mapHashWord2 = MapHash::gWord2;
    hdr.numSectors = (uint32_t) gNumSectors;
    hdr.numLines = (uint32_t) gNumLines;
    hdr.numSides = (uint32_t) gNumSides;
    hdr.numMobjs = (uint32_t) gMobjList.size();
    hdr.numVlDoors = (uint32_t) gVlDoors.size();
    hdr.numVlCustomDoors = (uint32_t) gVlCustomDoors.size();
    hdr.numFloorMovers = (uint32_t) gFloorMovers.size();
    hdr.numCeilings = (uint32_t) gCeilings.size();
    hdr.numPlats = (uint32_t) gPlats.size();
    hdr.numFireFlickers = (uint32_t) gFireFlickers.size();
    hdr.numLightFlashes = (uint32_t) gLightFlashes.size();
    hdr.numStrobes = (uint32_t) gStrobes.size();
    hdr.numGlows = (uint32_t) gGlows.size();
    hdr.numDelayedExits = (uint32_t) gDelayedExits.size();
    hdr.numButtons = (uint32_t) gActiveButtons.size();
    hdr.numScheduledActions = (uint32_t) ScriptingEngine::gScheduledActions.size();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Serializes an array of objects
//------------------------------------------------------------------------------------------------------------------------------------------
template <class SrcT, class DstT>
static void serializeObjects(const SrcT* const pSrcObjs, const uint32_t numObjs, std::unique_ptr<DstT[]>& dstObjs) noexcept {
    // Alloc the output array and ensure even padding bytes are zero initialized
    dstObjs = std::make_unique<DstT[]>(numObjs);
    std::memset(dstObjs.get(), 0, sizeof(DstT) * numObjs);

    // Serialize everything
    for (uint32_t i = 0; i < numObjs; ++i) {
        dstObjs[i].serializeFrom(pSrcObjs[i]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Serializes a list of pointers to objects
//------------------------------------------------------------------------------------------------------------------------------------------
template <class SrcT, class DstT>
static void serializeObjects(const std::vector<SrcT*>& srcObjs, std::unique_ptr<DstT[]>& dstObjs) noexcept {
    // Alloc the output array and ensure even padding bytes are zero initialized
    dstObjs = std::make_unique<DstT[]>(srcObjs.size());
    const size_t numObjs = srcObjs.size();
    std::memset(dstObjs.get(), 0, sizeof(DstT) * numObjs);

    // Serialize everything
    for (size_t i = 0; i < numObjs; ++i) {
        dstObjs[i].serializeFrom(*srcObjs[i]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to save the game to the specified output file
//------------------------------------------------------------------------------------------------------------------------------------------
bool save(OutputStream& out) noexcept {
    // Build required LUTs
    buildMobjLuts(8192);
    gatherThinkersOfType(T_VerticalDoor, gVlDoors, 128);
    gatherThinkersOfType(T_CustomDoor, gVlCustomDoors, 128);
    gatherThinkersOfType(T_MoveFloor, gFloorMovers, 512);
    gatherThinkersOfType(T_MoveCeiling, gCeilings, 512);
    gatherThinkersOfType(T_PlatRaise, gPlats, 512);
    gatherThinkersOfType(T_FireFlicker, gFireFlickers, 512);
    gatherThinkersOfType(T_LightFlash, gLightFlashes, 512);
    gatherThinkersOfType(T_StrobeFlash, gStrobes, 512);
    gatherThinkersOfType(T_Glow, gGlows, 512);
    gatherDelayedActionsOfType(G_CompleteLevel, gDelayedExits, 0);      // Don't expect to ever save this in practice...
    gatherActiveButtons(gActiveButtons, 32);

    // Populate the save header, globals and all the lists of objects
    SaveData saveData = {};
    SaveFileHdr& hdr = saveData.hdr;

    populateSaveHeader(hdr);
    saveData.globals.serializeFromGlobals();
    serializeObjects(gpSectors, hdr.numSectors, saveData.sectors);
    serializeObjects(gpLines, hdr.numLines, saveData.lines);
    serializeObjects(gpSides, hdr.numSides, saveData.sides);
    serializeObjects(gMobjList, saveData.mobjs);
    serializeObjects(gVlDoors, saveData.vlDoors);
    serializeObjects(gVlCustomDoors, saveData.vlCustomDoors);
    serializeObjects(gFloorMovers, saveData.floorMovers);
    serializeObjects(gCeilings, saveData.ceilings);
    serializeObjects(gPlats, saveData.plats);
    serializeObjects(gFireFlickers, saveData.fireFlickers);
    serializeObjects(gLightFlashes, saveData.lightFlashes);
    serializeObjects(gStrobes, saveData.strobes);
    serializeObjects(gGlows, saveData.glows);
    serializeObjects(gDelayedExits, saveData.delayedExits);
    serializeObjects(gActiveButtons, saveData.buttons);
    serializeObjects(ScriptingEngine::gScheduledActions.data(), hdr.numScheduledActions, saveData.scheduledActions);

    // Cleanup and finish up by writing it all out to a file
    clearTempLuts();
    return saveData.writeTo(out);
}

END_NAMESPACE(SaveAndLoad)
