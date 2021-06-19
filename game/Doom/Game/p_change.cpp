#include "p_change.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "info.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_move.h"

bool gbNofit;           // If 'true' then one or more things in the test sector undergoing height changes do not fit
bool gbCrushChange;     // If 'true' then the current sector undergoing height changes should crush/damage things when they do not fit

//------------------------------------------------------------------------------------------------------------------------------------------
// Clamps the map object's z position to be in a valid range for the sector it is within using the current floor/ceiling height.
// Returns 'true' if the object can fit vertically in it's current sector, 'false' otherwise (should be crushed).
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_ThingHeightClip(mobj_t& mobj) noexcept {
    // Is the thing currently on the floor?
    const fixed_t oldFloorZ = mobj.floorz;
    const bool bWasOnFloor = (oldFloorZ == mobj.z);

    // Get the current floor/ceiling Z values for the thing and update
    P_CheckPosition(mobj, mobj.x, mobj.y);
    mobj.floorz = gTmFloorZ;
    mobj.ceilingz = gTmCeilingZ;

    // PsyDoom: If the thing is the current player, on the floor, and the floor moved up or down then snap the current Z interpolation.
    // The player's viewpoint moves with the sector immediately in this instance because the player is being pushed/pulled:
    #if PSYDOOM_MODS
        if ((oldFloorZ != mobj.floorz) && bWasOnFloor && (mobj.player == &gPlayers[gCurPlayerIndex])) {
            R_SnapViewZInterpolation();
        }
    #endif

    // Things that were on the floor previously rise and fall as the sector floor rises and falls.
    // Otherwise, if a floating thing, clip against the ceiling.
    if (bWasOnFloor) {
        mobj.z = mobj.floorz;
    } else if (mobj.z + mobj.height > mobj.ceilingz) {
        mobj.z = mobj.ceilingz - mobj.height;
    }

    return (mobj.height <= mobj.ceilingz - mobj.floorz);    // Is there enough vertical room for the thing?
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does updates for one map object after the sector it is contained within has had it's floor or ceiling height changed.
// Clips the map object to the valid height range of the sector, and applies crushing where required.
// Sets 'gbNofit' to 'false' also if the thing does not fit in the height range of the sector.
//------------------------------------------------------------------------------------------------------------------------------------------
bool PIT_ChangeSector(mobj_t& mobj) noexcept {
    // Clip the thing to the height range of the sector: if it fits then we have nothing else to do
    if (P_ThingHeightClip(mobj))
        return true;

    // Turn bodies into gibs
    if (mobj.health <= 0) {
        P_SetMobjState(mobj, S_GIBS);
        S_StartSound(&mobj, sfx_slop);

        mobj.height = 0;    // This prevents the height clip test from failing again and triggering more crushing
        mobj.radius = 0;

        // If it is the player being gibbed then make the status bar head gib
        if (mobj.player == &gPlayers[gCurPlayerIndex]) {
            gStatusBar.gotgibbed = true;
        }

        return true;
    }

    // Crush and destroy dropped items
    if (mobj.flags & MF_DROPPED) {
        P_RemoveMobj(mobj);
        return true;
    }

    // If the thing is not shootable then don't do anything more to it.
    // This will be the case for decorative things etc.
    if ((mobj.flags & MF_SHOOTABLE) == 0)
        return true;

    // Things in the current sector do not fit into it's height range
    gbNofit = true;

    // If the sector crushes and it's every 4th tic then do some damage to the thing
    if (gbCrushChange && ((gGameTic & 3) == 0)) {
        P_DamageMobj(mobj, nullptr, nullptr, 10);

        // Spawn some blood and randomly send it off in different directions
        mobj_t& blood = *P_SpawnMobj(mobj.x, mobj.y, mobj.height / 2 + mobj.z, MT_BLOOD);
        blood.momx = P_SubRandom() * (FRACUNIT / 16);
        blood.momy = P_SubRandom() * (FRACUNIT / 16);
    }

    return true;    // Continue iterating through things in the blockmap
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called after the height of the given sector changes, either by it's floor or ceiling moving up or down.
// Clips all things contained in the sector to the new height range, and crushes things also (if desired).
// Returns 'true' if one or more things in the sector did not fit into it's height range.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_ChangeSector(sector_t& sector, const bool bCrunch) noexcept {
    // Force all players to do a noise alert again next time they fire.
    // Because of the height change with this sector, new gaps might have opened...
    for (int32_t i = MAXPLAYERS - 1; i >= 0; --i) {
        gPlayers[i].lastsoundsector = nullptr;
    }

    // Initially everything fits in the sector and save whether to crush for the blockmap iterator
    gbNofit = false;
    gbCrushChange = bCrunch;

    // Clip the heights of all things in the updated sector and crush things where appropriate.
    // Note that this crude test may pull in things in other sectors, which could be included in the results also. Generally that's okay however!
    const int32_t bmapLx = sector.blockbox[BOXLEFT];
    const int32_t bmapRx = sector.blockbox[BOXRIGHT];
    const int32_t bmapTy = sector.blockbox[BOXTOP];
    const int32_t bmapBy = sector.blockbox[BOXBOTTOM];

    for (int32_t x = bmapLx; x <= bmapRx; ++x) {
        for (int32_t y = bmapBy; y <= bmapTy; ++y) {
            P_BlockThingsIterator(x, y, PIT_ChangeSector);
        }
    }

    return gbNofit;     // Did all the things in the sector fit?
}
