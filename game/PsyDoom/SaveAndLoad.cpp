//------------------------------------------------------------------------------------------------------------------------------------------
// High level saving and loading functionality.
// Does not concern itself too much with the details of how each datatype is serialized, for that see 'SaveDataTypes.cpp'.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "SaveAndLoad.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_ceiling.h"
#include "Doom/Game/p_floor.h"
#include "Doom/Game/p_lights.h"
#include "Doom/Game/p_maputl.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_plats.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Game/p_switch.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
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

// LUTS for thinkers of various types: used during saving and loading
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

// Used during loading, the input save data loaded into memory
static SaveData gSaveDataIn;

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes all map objects from the game
//------------------------------------------------------------------------------------------------------------------------------------------
static void removeAllMobj() noexcept {
    mobj_t* pMobj = gMobjHead.next;

    while (pMobj != &gMobjHead) {
        mobj_t* const pNextMobj = pMobj->next;
        pMobj->flags = 0;       // So it doesn't get added to any item respawn queues
        P_RemoveMobj(*pMobj);
        pMobj = pNextMobj;
    }

    // Sanity check it all went OK
    ASSERT(gMobjHead.next == &gMobjHead);
    ASSERT(gMobjHead.prev == &gMobjHead);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes all thinkers from the game
//------------------------------------------------------------------------------------------------------------------------------------------
static void removeAllThinkers() noexcept {
    // Remove all thinkers
    thinker_t* pThinker = gThinkerCap.next;

    while (pThinker != &gThinkerCap) {
        thinker_t* const pNextThinker = pThinker->next;
        Z_Free2(*gpMainMemZone, pThinker);
        pThinker = pNextThinker;
    }

    gThinkerCap.next = &gThinkerCap;
    gThinkerCap.prev = &gThinkerCap;

    // Sectors are no longer associated with any thinkers
    const int32_t numSectors = gNumSectors;
    sector_t* const pSectors = gpSectors;

    for (int32_t i = 0; i < numSectors; ++i) {
        pSectors[i].specialdata = nullptr;
    }

    // Clear out active ceilings and platforms
    for (ceiling_t*& pCeil : gpActiveCeilings) {
        pCeil = nullptr;
    }

    for (plat_t*& pPlat : gpActivePlats) {
        pPlat = nullptr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes all active buttons (switch status changes) from the game
//------------------------------------------------------------------------------------------------------------------------------------------
static void removeActiveButtons() noexcept {
    for (button_t& btn : gButtonList) {
        btn = {};
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates all of the map objects to be loaded; assumes there no map objects currently existing in the game
//------------------------------------------------------------------------------------------------------------------------------------------
static void allocMobjsToLoad() noexcept {
    // Sanity check, there should be no map objects in the game at this point
    ASSERT(gMobjHead.next == &gMobjHead);

    const uint32_t numMobjs = gSaveDataIn.hdr.numMobjs;
    gMobjToIdx.clear();
    gMobjList.clear();
    gMobjToIdx.reserve(numMobjs);
    gMobjList.reserve(numMobjs);

    mobj_t* pMobjTail = &gMobjHead;

    for (uint32_t i = 0; i < numMobjs; ++i) {
        // Alloc the map object and zero init
        mobj_t& mobj = *(mobj_t*) Z_Malloc(*gpMainMemZone, sizeof(mobj_t), PU_LEVEL, nullptr);
        std::memset(&mobj, 0, sizeof(mobj_t));

        // Keep track of it for later loading logic
        gMobjToIdx[&mobj] = i;
        gMobjList.push_back(&mobj);

        // Link the new object into the map objects list at the end
        mobj.prev = pMobjTail;
        mobj.next = &gMobjHead;
        pMobjTail->next = &mobj;
        gMobjHead.prev = &mobj;
        pMobjTail = &mobj;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates thinkers of the specified type that are to be loaded and places pointers to them in the specified list.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ThinkerT>
static void allocThinkersToLoad(std::vector<ThinkerT*>& outputList, const uint32_t amt) noexcept {
    outputList.clear();
    outputList.reserve(amt);

    for (uint32_t i = 0; i < amt; ++i) {
        ThinkerT& thinker = *(ThinkerT*) Z_Malloc(*gpMainMemZone, sizeof(ThinkerT), PU_LEVSPEC, nullptr);
        std::memset(&thinker, 0, sizeof(ThinkerT));
        P_AddThinker(thinker.thinker);
        outputList.push_back(&thinker);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates room for all of the buttons to be loaded
//------------------------------------------------------------------------------------------------------------------------------------------
static void allocButtonsToLoad() noexcept {
    // This can only be done in limit removing builds
    const uint32_t numBtns = gSaveDataIn.hdr.numButtons;

    #if PSYDOOM_LIMIT_REMOVING
        gButtonList.clear();
        gButtonList.resize(numBtns);
    #else
        // In non limit removing builds if there are too many buttons then just immediately revert the switch texture
        if (numBtns > MAXBUTTONS) {
            for (uint32_t i = MAXBUTTONS; i < numBtns; ++i) {
                const SavedButtonT& savedBtn = gSaveDataIn.buttons[i];

                if (savedBtn.lineIdx < gNumLines) {
                    line_t& line = gpLines[savedBtn.lineIdx];
                    side_t& side = gpSides[line.sidenum[0]];

                    switch (savedBtn.where) {
                        case top:       side.toptexture     = savedBtn.btexture;  break;
                        case middle:    side.midtexture     = savedBtn.btexture;  break;
                        case bottom:    side.bottomtexture  = savedBtn.btexture;  break;
                    }
                }
            }
        }
    #endif
}

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
// Validates a list of save file objects
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ObjT>
static bool validateObjects(const std::unique_ptr<ObjT[]>& pObjs, const uint32_t numObjs) noexcept {
    for (uint32_t i = 0; i < numObjs; ++i) {
        if (!pObjs[i].validate())
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Serializes an array of objects
//------------------------------------------------------------------------------------------------------------------------------------------
template <class SrcT, class DstT>
static void serializeObjects(const SrcT* const pSrcObjs, std::unique_ptr<DstT[]>& dstObjs, const uint32_t numObjs) noexcept {
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
// De-serializes an array of save data objects to another array of objects.
// Note: does not zero-init, assumes destination objects have already had this done upon allocation.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class SrcT, class DstT>
static void deserializeObjects(const std::unique_ptr<SrcT>& pSrcObjs, DstT* const pDstObjs, const uint32_t numObjs) noexcept {
    for (uint32_t i = 0; i < numObjs; ++i) {
        pSrcObjs[i].deserializeTo(pDstObjs[i]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// De-serializes an array of save data objects to another list of objects stored by pointer.
// Note: does not zero-init, assumes destination objects have already had this done upon allocation.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class SrcT, class DstT>
static void deserializeObjects(const std::unique_ptr<SrcT>& pSrcObjs, const std::vector<DstT*>& dstObjs) noexcept {
    const uint32_t numObjs = (uint32_t) dstObjs.size();

    for (uint32_t i = 0; i < numObjs; ++i) {
        pSrcObjs[i].deserializeTo(*dstObjs[i]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load helper: plays the specified CD track if not already playing
//------------------------------------------------------------------------------------------------------------------------------------------
static void playCdTrackIfNeeded(const int32_t cdTrackNum) noexcept {
    if (cdTrackNum > 0) {
        const int32_t playingCdTrack = psxcd_get_playing_track();

        if (playingCdTrack != cdTrackNum) {
            S_StopMusic();
            psxcd_play_at_andloop(cdTrackNum, gCdMusicVol, 0, 0, cdTrackNum, gCdMusicVol, 0, 0);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load helper: add loaded active ceilings and plats to the active lists.
// Note: expects the lists of output ceilings and plats to be allocated.
//------------------------------------------------------------------------------------------------------------------------------------------
static void addActiveCeilingsAndPlats() noexcept {
    const uint32_t numCeils = gSaveDataIn.hdr.numCeilings;
    const uint32_t numPlats = gSaveDataIn.hdr.numPlats;
    ASSERT(numCeils == gCeilings.size());
    ASSERT(numPlats == gPlats.size());

    for (uint32_t i = 0; i < numCeils; ++i) {
        if (gSaveDataIn.ceilings[i].bIsActive) {
            P_AddActiveCeiling(*gCeilings[i]);
        }
    }

    for (uint32_t i = 0; i < numPlats; ++i) {
        if (gSaveDataIn.plats[i].bIsActive) {
            P_AddActivePlat(*gPlats[i]);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load helper: add all map objects into the world (registers with sectors and the blockmap)
//------------------------------------------------------------------------------------------------------------------------------------------
static void addMobjsToSectors() noexcept {
    for (mobj_t* const pMobj : gMobjList) {
        P_SetThingPosition(*pMobj);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load helper: associates the specified list of thinkers with their corresponding sectors
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ThinkerT>
static void associateThinkersWithSectors(const std::vector<ThinkerT*>& thinkers) noexcept {
    for (ThinkerT* const pThinker : thinkers) {
        pThinker->sector->specialdata = pThinker;
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
    serializeObjects(gpSectors, saveData.sectors, hdr.numSectors);
    serializeObjects(gpLines, saveData.lines, hdr.numLines);
    serializeObjects(gpSides, saveData.sides, hdr.numSides);
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
    serializeObjects(ScriptingEngine::gScheduledActions.data(), saveData.scheduledActions, hdr.numScheduledActions);

    // Cleanup and finish up by writing it all out to a file
    clearTempLuts();
    return saveData.writeTo(out);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a save file from the specified input stream and performs basic validation.
// This step doesn't actually begin the process of loading the level, just buffers all the data.
//------------------------------------------------------------------------------------------------------------------------------------------
ReadSaveResult read(InputStream& in) noexcept {
    gSaveDataIn = {};
    return gSaveDataIn.readFrom(in);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads the current save file that has been read into memory, after the map file used by the save has been loaded
//------------------------------------------------------------------------------------------------------------------------------------------
LoadSaveResult load() noexcept {
    // Do some very basic validations first
    const SaveData& saveData = gSaveDataIn;
    const SaveFileHdr& hdr = saveData.hdr;

    if (!hdr.validateMapHash())
        return LoadSaveResult::BAD_MAP_HASH;

    if (!hdr.validate())
        return LoadSaveResult::BAD_DATA;

    // Clear out stuff from the map
    removeAllMobj();
    removeAllThinkers();
    removeActiveButtons();
    ScriptingEngine::gScheduledActions.clear();

    // Allocate objects that the save file calls for
    allocMobjsToLoad();
    allocThinkersToLoad(gVlDoors, hdr.numVlDoors);
    allocThinkersToLoad(gVlCustomDoors, hdr.numVlCustomDoors);
    allocThinkersToLoad(gFloorMovers, hdr.numFloorMovers);
    allocThinkersToLoad(gCeilings, hdr.numCeilings);
    allocThinkersToLoad(gPlats, hdr.numPlats);
    allocThinkersToLoad(gFireFlickers, hdr.numFireFlickers);
    allocThinkersToLoad(gLightFlashes, hdr.numLightFlashes);
    allocThinkersToLoad(gStrobes, hdr.numStrobes);
    allocThinkersToLoad(gGlows, hdr.numGlows);
    allocThinkersToLoad(gDelayedExits, hdr.numDelayedExits);
    allocButtonsToLoad();
    ScriptingEngine::gScheduledActions.resize(hdr.numScheduledActions);

    // Validate everything that needs to be validated
    const bool bAllValid = (
        saveData.globals.validate() &&
        validateObjects(saveData.sectors, hdr.numSectors) &&
        validateObjects(saveData.sides, hdr.numSides) &&
        validateObjects(saveData.mobjs, hdr.numMobjs) &&
        validateObjects(saveData.vlDoors, hdr.numVlDoors) &&
        validateObjects(saveData.vlCustomDoors, hdr.numVlCustomDoors) &&
        validateObjects(saveData.floorMovers, hdr.numFloorMovers) &&
        validateObjects(saveData.ceilings, hdr.numCeilings) &&
        validateObjects(saveData.plats, hdr.numPlats) &&
        validateObjects(saveData.fireFlickers, hdr.numFireFlickers) &&
        validateObjects(saveData.lightFlashes, hdr.numLightFlashes) &&
        validateObjects(saveData.strobes, hdr.numStrobes) &&
        validateObjects(saveData.glows, hdr.numGlows) &&
        validateObjects(saveData.buttons, hdr.numButtons)
    );

    if (!bAllValid)
        return LoadSaveResult::BAD_DATA;

    // Deserialize all objects
    saveData.globals.deserializeToGlobals();
    deserializeObjects(saveData.sectors, gpSectors, hdr.numSectors);
    deserializeObjects(saveData.lines, gpLines, hdr.numLines);
    deserializeObjects(saveData.sides, gpSides, hdr.numSides);
    deserializeObjects(saveData.mobjs, gMobjList);
    deserializeObjects(saveData.vlDoors, gVlDoors);
    deserializeObjects(saveData.vlCustomDoors, gVlCustomDoors);
    deserializeObjects(saveData.floorMovers, gFloorMovers);
    deserializeObjects(saveData.ceilings, gCeilings);
    deserializeObjects(saveData.plats, gPlats);
    deserializeObjects(saveData.fireFlickers, gFireFlickers);
    deserializeObjects(saveData.lightFlashes, gLightFlashes);
    deserializeObjects(saveData.strobes, gStrobes);
    deserializeObjects(saveData.glows, gGlows);
    deserializeObjects(saveData.delayedExits, gDelayedExits);
    deserializeObjects(saveData.buttons, gActiveButtons);
    deserializeObjects(saveData.scheduledActions, ScriptingEngine::gScheduledActions.data(), hdr.numScheduledActions);

    // Post load actions: adding map objects into the blockmap and sector lists, and associating thinkers with their sectors
    addMobjsToSectors();
    associateThinkersWithSectors(gVlDoors);
    associateThinkersWithSectors(gVlCustomDoors);
    associateThinkersWithSectors(gFloorMovers);
    associateThinkersWithSectors(gCeilings);
    associateThinkersWithSectors(gPlats);
    addActiveCeilingsAndPlats();

    // Post load actions: play CD music if required and kill any interpolation for the next frame (snap to the new position)
    playCdTrackIfNeeded(saveData.globals.curCDTrack);
    R_NextInterpolation();

    // Finish up and cleanup
    clearTempLuts();
    return LoadSaveResult::OK;
}

END_NAMESPACE(SaveAndLoad)
