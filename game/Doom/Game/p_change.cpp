#include "p_change.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "info.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_move.h"
#include "PsxVm/PsxVm.h"

const VmPtr<bool32_t>   gbNofit(0x80077EBC);            // If 'true' then one or more things in the test sector undergoing height changes do not fit
const VmPtr<bool32_t>   gbCrushChange(0x80077FA0);      // If 'true' then the current sector undergoing height changes should crush/damage things when they do not fit

//------------------------------------------------------------------------------------------------------------------------------------------
// Clamps the map object's z position to be in a valid range for the sector it is within using the current floor/ceiling height.
// Returns 'true' if the object can fit vertically in it's current sector, 'false' otherwise (should be crushed).
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_ThingHeightClip(mobj_t& mobj) noexcept {
    // Is the thing currently on the floor?
    const bool bWasOnFloor = (mobj.floorz == mobj.z);

    // Get the current floor/ceiling Z values for the thing and update
    P_CheckPosition(mobj, mobj.x, mobj.y);
    mobj.floorz = *gTmFloorZ;
    mobj.ceilingz = *gTmCeilingZ;

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
        P_SetMObjState(mobj, S_GIBS);
        S_StartSound(&mobj, sfx_slop);

        mobj.height = 0;    // This prevents the height clip test from failing again and triggering more crushing
        mobj.radius = 0;

        // If it is the player being gibbed then make the status bar head gib
        if (mobj.player.get() == &gPlayers[*gCurPlayerIndex]) {
            gStatusBar->gotgibbed = true;
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
    *gbNofit = true;

    // If the sector crushes and it's every 4th tic then do some damage to the thing
    if (*gbCrushChange && ((*gGameTic & 3) == 0)) {
        P_DamageMObj(mobj, nullptr, nullptr, 10);

        // Spawn some blood and randomly send it off in different directions
        mobj_t& blood = *P_SpawnMobj(mobj.x, mobj.y, mobj.height / 2 + mobj.z, MT_BLOOD);
        blood.momx = (P_Random() - P_Random()) * (FRACUNIT / 16);
        blood.momy = (P_Random() - P_Random()) * (FRACUNIT / 16);
    }

    return true;    // Continue iterating through things in the blockmap
}

void P_ChangeSector() noexcept {
loc_80015238:
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    v0 = 0x12C;                                         // Result = 0000012C
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
loc_80015254:
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7700;                                       // Result = gPlayer1[45] (800A8900)
    at += v0;
    sw(0, at);
    v0 -= 0x12C;
    if (i32(v0) >= 0) goto loc_80015254;
    s1 = lw(s2 + 0x30);
    v0 = lw(s2 + 0x34);
    sw(0, gp + 0x8DC);                                  // Store to: gbNofit (80077EBC)
    sw(a1, gp + 0x9C0);                                 // Store to: gbCrushChange (80077FA0)
    v0 = (i32(v0) < i32(s1));
    if (v0 != 0) goto loc_800152DC;
loc_8001528C:
    s0 = lw(s2 + 0x2C);
    v0 = lw(s2 + 0x28);
    v0 = (i32(v0) < i32(s0));
    a0 = s1;
    if (v0 != 0) goto loc_800152C8;
loc_800152A4:
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x504C;                                       // Result = PIT_ChangeSector (8001504C)
    a1 = s0;
    P_BlockThingsIterator(a0, a1, PIT_ChangeSector);
    v0 = lw(s2 + 0x28);
    s0++;
    v0 = (i32(v0) < i32(s0));
    a0 = s1;
    if (v0 == 0) goto loc_800152A4;
loc_800152C8:
    v0 = lw(s2 + 0x34);
    s1++;
    v0 = (i32(v0) < i32(s1));
    if (v0 == 0) goto loc_8001528C;
loc_800152DC:
    v0 = lw(gp + 0x8DC);                                // Load from: gbNofit (80077EBC)
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
