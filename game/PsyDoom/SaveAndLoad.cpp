//------------------------------------------------------------------------------------------------------------------------------------------
// High level saving and loading functionality.
// Does not concern itself too much with the details of how each datatype is serialized, for that see 'SaveDataTypes.cpp'.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "SaveAndLoad.h"

#include "Doom/Game/p_tick.h"

BEGIN_NAMESPACE(SaveAndLoad)

std::unordered_map<mobj_t*, int32_t>    gMobjToIdx;     // Save/load accelerator LUT: maps from a map object to it's index in the global linked list of map objects
std::vector<mobj_t*>                    gMobjList;      // Save/load accelerator LUT: maps from a map object index to it's pointer

//------------------------------------------------------------------------------------------------------------------------------------------
// Builds the LUTs for accelerating map object lookups
//------------------------------------------------------------------------------------------------------------------------------------------
static void buildMobjLUTs() noexcept {
    gMobjToIdx.clear();
    gMobjToIdx.reserve(8192);
    gMobjList.clear();
    gMobjList.reserve(8192);

    int32_t mobjIdx = 0;

    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead; pMobj = pMobj->next, ++mobjIdx) {
        gMobjToIdx[pMobj] = mobjIdx;
        gMobjList.push_back(pMobj);
    }
}

END_NAMESPACE(SaveAndLoad)
