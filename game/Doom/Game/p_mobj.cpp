#include "p_mobj.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/pw_main.h"
#include "Doom/UI/st_main.h"
#include "doomdata.h"
#include "g_game.h"
#include "info.h"
#include "p_local.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_password.h"
#include "p_pspr.h"
#include "p_setup.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"
#include <algorithm>

// Item respawn queue
static constexpr int32_t ITEMQUESIZE = 64;
static constexpr int32_t ITEMQUESIZE_MASK = ITEMQUESIZE - 1;    // Convenience constant for wrapping

const VmPtr<int32_t>    gItemRespawnQueueHead(0x80078138);      // Head of the circular queue
const VmPtr<int32_t>    gItemRespawnQueueTail(0x80078180);      // Tail of the circular queue

static const VmPtr<int32_t[ITEMQUESIZE]>        gItemRespawnTime(0x80097910);       // When each item in the respawn queue began the wait to respawn
static const VmPtr<mapthing_t[ITEMQUESIZE]>     gItemRespawnQueue(0x8008612C);      // Details for the things to be respawned

// Object kill tracking
const VmPtr<int32_t> gNumMObjKilled(0x80078010);

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes the given map object from the game
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RemoveMobj(mobj_t& mobj) noexcept {
    // Respawn the item later it's the right type
    const bool bRespawn = (
        (mobj.flags & MF_SPECIAL) &&
        ((mobj.flags & MF_DROPPED) == 0) &&
        (mobj.type != MT_INV) &&
        (mobj.type != MT_INS)
    );

    if (bRespawn) {
        // Remember the item details for later respawning and occupy one queue slot
        const int32_t slotIdx = (*gItemRespawnQueueHead) & ITEMQUESIZE_MASK;

        gItemRespawnTime[slotIdx] = *gTicCon;
        gItemRespawnQueue[slotIdx].x = mobj.spawnx;
        gItemRespawnQueue[slotIdx].y = mobj.spawny;
        gItemRespawnQueue[slotIdx].type = mobj.spawntype;
        gItemRespawnQueue[slotIdx].angle = mobj.spawnangle;

        *gItemRespawnQueueHead += 1;
    }

    // Remove the thing from sector thing lists and the blockmap
    P_UnsetThingPosition(mobj);

    // Remove from the global linked list of things and deallocate
    mobj.next->prev = mobj.prev;
    mobj.prev->next = mobj.next;
    Z_Free2(*gpMainMemZone->get(), &mobj);
}

// TODO: remove eventually. Needed at the minute due to 'latecall' function pointer invocations of this function.
void _thunk_P_RemoveMobj() noexcept {
    P_RemoveMobj(*vmAddrToPtr<mobj_t>(*PsxVm::gpReg_a0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Respawns a single item (if one is due to respawn) in deathmatch mode
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RespawnSpecials() noexcept {
    // Only respawn in deathmatch
    if (*gNetGame != gt_deathmatch)
        return;
    
    // No respawning if there is nothing to respawn
    if (*gItemRespawnQueueHead == *gItemRespawnQueueTail)
        return;

    // If the queue has overflowed in (stated) size then adjust it's size back to the limit
    if (*gItemRespawnQueueHead - *gItemRespawnQueueTail > ITEMQUESIZE) {
        *gItemRespawnQueueTail = *gItemRespawnQueueHead - ITEMQUESIZE;
    }

    // Wait 120 seconds before respawning things
    const int32_t slotIdx = (*gItemRespawnQueueTail) & ITEMQUESIZE_MASK;
    
    if (*gTicCon - gItemRespawnTime[slotIdx] < 120 * TICRATE)
        return;

    // Get the spawn location and sector
    const mapthing_t& mapthing = gItemRespawnQueue[slotIdx];
    const fixed_t x = (fixed_t) mapthing.x << FRACBITS;
    const fixed_t y = (fixed_t) mapthing.y << FRACBITS;

    subsector_t& subsec = *R_PointInSubsector(x, y);
    sector_t& sec = *subsec.sector;

    // Spawn an item appear fog (like teleport fog)
    {
        mobj_t& mobj = *P_SpawnMobj(x, y, sec.floorheight, MT_IFOG);
        S_StartSound(&mobj, sfx_itmbk);
    }
    
    // Try to find the type enum for the thing to be spawned
    int32_t mobjTypeIdx = 0;
    
    for (; mobjTypeIdx < NUMMOBJTYPES; mobjTypeIdx++) {
        if (mapthing.type == gMObjInfo[mobjTypeIdx].doomednum)  // Is this the mobj definition we want?
            break;
    }

    // Decide on the z position for the thing being spawned, either spawn it on the floor or ceiling
    const mobjtype_t mobjType = (mobjtype_t) mobjTypeIdx;
    const mobjinfo_t& mobjinfo = gMObjInfo[mobjType];
    const fixed_t z = (mobjinfo.flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ;

    // Spawn the item itself
    mobj_t& mobj = *P_SpawnMobj(x, y, z, mobjType);
    mobj.angle = (mapthing.angle / 45) * ANG45;
    mobj.spawnx = mapthing.x;
    mobj.spawny = mapthing.y;
    mobj.spawntype = mapthing.type;
    mobj.spawnangle = mapthing.angle;

    // This thing has now been spawned, free up the queue slot
    *gItemRespawnQueueTail += 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the state for the given map object.
// Returns 'true' if the map object is still present and hasn't been removed - it's removed if the state switch is to 'S_NULL'.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_SetMObjState(mobj_t& mobj, const statenum_t stateNum) noexcept {
    // Remove the map object if the state is null
    if (stateNum == S_NULL) {
        mobj.state = nullptr;
        P_RemoveMobj(mobj);
        return false;
    }

    // Set the new state and call the action function for the state (if any)
    state_t& state = gStates[stateNum];

    mobj.state = &state;
    mobj.tics = state.tics;
    mobj.sprite = state.sprite;
    mobj.frame = state.frame;

    if (state.action) {
        // FIXME: convert to native call
        const VmFunc func = PsxVm::getVmFuncForAddr(state.action);
        a0 = ptrToVmAddr(&mobj);
        func();
    }

    // This request gets cleared on state switch
    mobj.latecall = nullptr;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Explodes the given missile: sends it into the death state, plays the death sound and removes the MF_MISSILE flag
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ExplodeMissile(mobj_t& mobj) noexcept {
    // Kill all momentum and enter death state
    mobj.momx = 0;
    mobj.momy = 0;
    mobj.momz = 0;

    mobjinfo_t& mobjInfo = *mobj.info;
    P_SetMObjState(mobj, mobjInfo.deathstate);

    // Some random state tic variation and make no longer a missile
    mobj.tics = std::max(mobj.tics - (P_Random() & 1), 1);
    mobj.flags &= (~MF_MISSILE);

    // Stop the missile sound and start the explode sound
    if (mobjInfo.deathsound != sfx_None) {
        S_StopSound(mobj.target);
        S_StartSound(&mobj, mobjInfo.deathsound);
    }
}

// TODO: remove eventually. Needed at the minute due to 'latecall' function pointer invocations of this function.
void _thunk_P_ExplodeMissile() noexcept {
    P_ExplodeMissile(*vmAddrToPtr<mobj_t>(*PsxVm::gpReg_a0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a thing with the specified type at the given location in space
//------------------------------------------------------------------------------------------------------------------------------------------
mobj_t* P_SpawnMobj(const fixed_t x, const fixed_t y, const fixed_t z, const mobjtype_t type) noexcept {
    // Alloc and zero initialize the map object
    mobj_t& mobj = *(mobj_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(mobj_t), PU_LEVEL, nullptr);
    D_memset(&mobj, std::byte(0), sizeof(mobj_t));

    // Fill in basic fields
    mobjinfo_t& info = gMObjInfo[type];
    mobj.type = type;
    mobj.info = &info;
    mobj.x = x;
    mobj.y = y;
    mobj.radius = info.radius;
    mobj.height = info.height;
    mobj.flags = info.flags;
    mobj.health = info.spawnhealth;
    mobj.reactiontime = info.reactiontime;

    // Set initial state and state related stuff.
    // Note: can't set state with P_SetMobjState, because actions can't be called yet (thing not fully initialized).
    state_t& state = gStates[info.spawnstate];
    mobj.state = &state;
    mobj.tics = state.tics;
    mobj.sprite = state.sprite;
    mobj.frame = state.frame;
    
    // Add to the sector thing list and the blockmap
    P_SetThingPosition(mobj);
    
    // Decide on the z position for the thing (specified z, or floor/ceiling)
    sector_t& sec = *mobj.subsector->sector.get();
    mobj.floorz = sec.floorheight;
    mobj.ceilingz = sec.ceilingheight;
    
    if (z == ONFLOORZ) {
        mobj.z = mobj.floorz;
    } else if (z == ONCEILINGZ) {
        mobj.z = sec.ceilingheight - mobj.info->height;
    } else {
        mobj.z = z;
    }

    // Add into the linked list of things
    gMObjHead->prev->next = &mobj;
    mobj.next = gMObjHead.get();
    mobj.prev = gMObjHead->prev;
    gMObjHead->prev = &mobj;
    return &mobj;
}

void P_SpawnPlayer() noexcept {
loc_8001CE40:
    sp -= 0x38;
    sw(s4, sp + 0x28);
    s4 = a0;
    sw(ra, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    a0 = lh(s4 + 0x6);
    v1 = a0 << 2;
    at = *gpSectors;
    at += v1;
    v0 = lw(at);
    v1 += a0;
    if (v0 != 0) goto loc_8001CE94;
    v0 = 0;                                             // Result = 00000000
    goto loc_8001D15C;
loc_8001CE94:
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7940;                                       // Result = gTmpWadLumpBuffer[3FDE] (800A86C0)
    s3 = v0 + v1;
    v0 = lw(s3 + 0x4);
    s5 = 2;                                             // Result = 00000002
    a1 = 0x94;                                          // Result = 00000094
    if (v0 != s5) goto loc_8001CEC8;
    a0--;
    G_PlayerReborn(a0);
    a1 = 0x94;                                          // Result = 00000094
loc_8001CEC8:
    a2 = 2;                                             // Result = 00000002
    a3 = 0;                                             // Result = 00000000
    a0 = *gpMainMemZone;
    s1 = lh(s4);
    s2 = lh(s4 + 0x2);
    s1 <<= 16;
    s2 <<= 16;
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x1FC4;                                       // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    sw(0, s0 + 0x54);
    sw(v0, s0 + 0x58);
    sw(s1, s0);
    sw(s2, s0 + 0x4);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1F84);                               // Load from: MObjInfo_MT_PLAYER[10] (8005E07C)
    sw(v0, s0 + 0x40);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1F80);                               // Load from: MObjInfo_MT_PLAYER[11] (8005E080)
    sw(v0, s0 + 0x44);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1F70);                               // Load from: MObjInfo_MT_PLAYER[15] (8005E090)
    sw(v0, s0 + 0x64);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1FBC);                               // Load from: MObjInfo_MT_PLAYER[2] (8005E044)
    sw(v0, s0 + 0x68);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1FB0);                               // Load from: MObjInfo_MT_PLAYER[5] (8005E050)
    sw(v0, s0 + 0x78);
    v1 = 0x80060000;                                    // Result = 80060000
    v1 = lw(v1 - 0x1FC0);                               // Load from: MObjInfo_MT_PLAYER[1] (8005E040)
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x5C);
    v1 = lw(v0);
    sw(v1, s0 + 0x28);
    v0 = lw(v0 + 0x4);
    a0 = s0;
    sw(v0, s0 + 0x2C);
    P_SetThingPosition(*vmAddrToPtr<mobj_t>(a0));
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v1 = lw(s0 + 0xC);
    v0 = lw(v0);
    sw(v0, s0 + 0x38);
    v0 = lw(v1);
    v1 = lw(s0 + 0x38);
    v0 = lw(v0 + 0x4);
    sw(v1, s0 + 0x8);
    sw(v0, s0 + 0x3C);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(s0, v0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    sw(v0, s0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    a1 = 0xB60B0000;                                    // Result = B60B0000
    sw(v0, s0 + 0x10);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s0, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    v1 = lhu(s4 + 0x4);
    a1 |= 0x60B7;                                       // Result = B60B60B7
    v1 <<= 16;
    a0 = u32(i32(v1) >> 16);
    mult(a0, a1);
    sw(s3, s0 + 0x80);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 += a0;
    v0 = u32(i32(v0) >> 5);
    v0 -= v1;
    v0 <<= 29;
    sw(v0, s0 + 0x24);
    v0 = lw(s3 + 0x24);
    v1 = 0x290000;                                      // Result = 00290000
    sw(v0, s0 + 0x68);
    v0 = 0x24;                                          // Result = 00000024
    sw(s0, s3);
    sw(0, s3 + 0x4);
    sw(0, s3 + 0xC4);
    sw(0, s3 + 0xD4);
    sw(0, s3 + 0xD8);
    sw(0, s3 + 0xDC);
    sw(0, s3 + 0xE4);
    sw(0, s3 + 0xE8);
    sw(v1, s3 + 0x18);
    sw(v0, s3 + 0x120);
    v0 = lw(s0 + 0x8);
    v0 += v1;
    sw(v0, s3 + 0x14);
    a0 = lh(s4 + 0x6);
    a0--;
    P_SetupPsprites();
    v0 = *gNetGame;
    if (v0 != s5) goto loc_8001D0DC;
    a0 = 1;                                             // Result = 00000001
    v1 = 5;                                             // Result = 00000005
    v0 = s3 + 0x14;
loc_8001D0C4:
    sw(a0, v0 + 0x48);
    v1--;
    v0 -= 4;
    if (i32(v1) >= 0) goto loc_8001D0C4;
    v0 = *gNetGame;
loc_8001D0DC:
    if (v0 != 0) goto loc_8001D130;
    v0 = lh(s4 + 0x6);
    v1 = *gCurPlayerIndex;
    v0--;
    {
        const bool bJump = (v0 != v1);
        v0 = s0;
        if (bJump) goto loc_8001D15C;
    }
    v0 = *gbUsingAPassword;
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_8001D130;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x6560;                                       // Result = gPasswordCharBuffer[0] (80096560)
    a2 = a1;
    a3 = s3;
    P_ProcessPassword();
    *gbUsingAPassword = false;
loc_8001D130:
    v0 = lh(s4 + 0x6);
    v1 = *gCurPlayerIndex;
    v0--;
    {
        const bool bJump = (v0 != v1);
        v0 = s0;
        if (bJump) goto loc_8001D15C;
    }
    ST_InitEveryLevel();
    I_UpdatePalette();
    v0 = s0;
loc_8001D15C:
    ra = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x38;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawns a thing using the information in the given 'mapthing_t' struct
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnMapThing(const mapthing_t& mapthing) noexcept {
    // Remember player starts for single player and co-op games
    if (mapthing.type <= MAXPLAYERS) {
        gPlayerStarts[mapthing.type - 1] = mapthing;
        return;
    }

    // Remember deathmatch starts for deathmatch games
    if (mapthing.type == 11) {
        if (gpDeathmatchP->get() < &gDeathmatchStarts[MAX_DEATHMATCH_STARTS]) {
            D_memcpy(gpDeathmatchP->get(), &mapthing, sizeof(mapthing_t));
            *gpDeathmatchP += 1;
        }

        return;
    }
    
    // Ignore if it's a deathmatch only thing and this is not deathmatch
    if ((mapthing.options & MTF_DEATHMATCH) && (*gNetGame != gt_deathmatch))
        return;
    
    // Ignore the thing if it's not for this skill level.
    // Note: 'bSkillMatch' was undefined in the original code if the game skill was greater than nightmare but I'm defaulting it here.
    bool bSkillMatch = false;

    if (*gGameSkill <= sk_easy) {
        bSkillMatch = (mapthing.options & MTF_EASY);
    } else if (*gGameSkill == sk_medium) {
        bSkillMatch = (mapthing.options & MTF_NORMAL);
    } else if (*gGameSkill <= sk_nightmare) {
        bSkillMatch = (mapthing.options & MTF_HARD);
    }
    
    if (!bSkillMatch)
        return;
    
    // Try to figure out the thing type using the DoomEd num.
    // If that fails then issue a fatal error.
    mobjtype_t thingType = NUMMOBJTYPES;

    for (int32_t thingTypeIdx = 0; thingTypeIdx < NUMMOBJTYPES; ++thingTypeIdx) {
        if (gMObjInfo[thingTypeIdx].doomednum == mapthing.type) {
            thingType = (mobjtype_t) thingTypeIdx;
            break;
        }
    }
    
    if (thingType == NUMMOBJTYPES) {
        I_Error("P_SpawnMapThing: Unknown doomednum %d at (%d, %d)", (int) mapthing.type, (int) mapthing.x, (int) mapthing.y);
        return;
    }

    // Do not spawn monsters and keycards in deathmatch
    const mobjinfo_t& info = gMObjInfo[thingType];

    if ((*gNetGame == gt_deathmatch) && info.flags & (MF_NOTDMATCH|MF_COUNTKILL))
        return;

    // Decide whether the thing spawns on the ceiling or floor and spawn it
    const fixed_t z = (info.flags & MF_SPAWNCEILING) ? ONCEILINGZ : ONFLOORZ;

    mobj_t& mobj = *P_SpawnMobj(
        (fixed_t) mapthing.x << FRACBITS,
        (fixed_t) mapthing.y << FRACBITS,
        z,
        thingType
    );

    // Randomly vary starting state tics amount
    if (mobj.tics > 0) {
        mobj.tics = 1 + (P_Random() % mobj.tics);
    }

    // Include the thing in kill and item stats if applicable
    if (mobj.flags & MF_COUNTKILL) {
        *gTotalKills += 1;
    }
    
    if (mobj.flags & MF_COUNTITEM) {
        *gTotalItems += 1;
    }
    
    // Set thing angle
    mobj.angle = ANG45 * (mapthing.angle / 45);

    // Save the mapthing fields in case we want to respawn (for items)
    mobj.spawnx = mapthing.x;
    mobj.spawny = mapthing.y;
    mobj.spawntype = mapthing.type;
    mobj.spawnangle = mapthing.angle;

    // Set the ambush flag (no activate on sound) if specified
    mobj.flags |= (mapthing.options & MTF_AMBUSH) ? MF_AMBUSH : 0;

    // PSX specific blending flags: set them on the thing if specified
    mobj.flags |= (mapthing.options & MTF_BLEND_ON) ? MF_BLEND_ON : 0;
    mobj.flags |= (mapthing.options & MTF_BLEND_MODE_BIT1) ? MF_BLEND_MODE_BIT1 : 0;
    mobj.flags |= (mapthing.options & MTF_BLEND_MODE_BIT2) ? MF_BLEND_MODE_BIT2 : 0;

    // Double health for nightmare blended monsters
    if ((mobj.flags & MF_ALL_BLEND_FLAGS) == MF_BLEND_SUBTRACT) {
        mobj.health *= 2;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawns a puff particle effect at the given location
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnPuff(const fixed_t x, const fixed_t y, const fixed_t z) noexcept {
    // Spawn the puff and randomly adjust its height
    const fixed_t spawnZ = z + ((P_Random() - P_Random()) << 10);
    mobj_t& mobj = *P_SpawnMobj(x, y, spawnZ, MT_PUFF);

    // Give some upward momentum and randomly adjust tics left
    mobj.momz = FRACUNIT;
    mobj.tics -= P_Random() & 1;

    if (mobj.tics < 1) {
        mobj.tics = 1;
    }
    
    // Don't do sparks if punching the wall
    if (*gAttackRange == MELEERANGE) {
        P_SetMObjState(mobj, S_PUFF3);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawns a blood particle effect at the given location
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnBlood(const fixed_t x, const fixed_t y, const fixed_t z, const int32_t damage) noexcept {
    // Spawn the puff and randomly adjust its height
    const fixed_t spawnZ = z + ((P_Random() - P_Random()) << 10);
    mobj_t& mobj = *P_SpawnMobj(x, y, spawnZ, MT_BLOOD);

    // Give some upward momentum and randomly adjust tics left
    mobj.momz = 2 * FRACUNIT;
    mobj.tics -= P_Random() & 1;
    
    if (mobj.tics < 1) {
        mobj.tics = 1;
    }
    
    // Adjust the type of blood, based on the damage amount
    if ((damage >= 9) && (damage <= 12)) {
        P_SetMObjState(mobj, S_BLOOD2);
    } else if (damage < 9) {
        P_SetMObjState(mobj, S_BLOOD3);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Moves a missile a little after spawning to see if it spawned in a wall or something, and explodes it immediately if so
//------------------------------------------------------------------------------------------------------------------------------------------
void P_CheckMissileSpawn(mobj_t& mobj) noexcept {
    // Note: using division by '2' here yields a slightly different result in some cases, such as with the number '0x80000001'.
    // Shifts are required for demo accurate behavior!
    mobj.x += mobj.momx >> 1;
    mobj.y += mobj.momy >> 1;
    mobj.z += mobj.momz >> 1;
    
    if (!P_TryMove(mobj, mobj.x, mobj.y)) {
        P_ExplodeMissile(mobj);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a missile of the specified type going from the given source object to the destination object.
// Returns the missile spawned.
//------------------------------------------------------------------------------------------------------------------------------------------
mobj_t* P_SpawnMissile(mobj_t& source, mobj_t& dest, const mobjtype_t type) noexcept {
    // Spawn the missile
    mobj_t& missile = *P_SpawnMobj(source.x, source.y, source.z + 32 * FRACUNIT, type);
    mobjinfo_t& missileInfo = *missile.info;

    // Play the initial spawning sound
    if (missileInfo.seesound != sfx_None) {
        S_StartSound(&source, missileInfo.seesound);
    }
    
    // Remember who fired the missile (for enemy AI, damage logic etc.)
    missile.target = &source;

    // Decide on the angle/direction to fire the missile in.
    // If the target thing has the invsibility powerup also, then randomize it's direction a bit for bad aim.
    angle_t angle = R_PointToAngle2(source.x, source.y, dest.x, dest.y);
    
    if (dest.flags & MF_ALL_BLEND_FLAGS) {
        angle += (P_Random() - P_Random()) << 20;
    }
    
    missile.angle = angle;

    // Set the missile velocity based on the direction it's going in
    const uint32_t fineAngle = angle >> ANGLETOFINESHIFT;
    const int32_t speedInt = missileInfo.speed >> FRACBITS;

    missile.momx = gFineCosine[fineAngle] * speedInt;
    missile.momy = gFineSine[fineAngle] * speedInt;

    // Figure out the z velocity based on how many tics it would take to reach the destination and z delta
    const fixed_t distToTarget = P_AproxDistance(dest.x - source.x, dest.y - source.y);
    const int32_t flyTics = std::max(distToTarget / missileInfo.speed, 1);
    const fixed_t deltaZ = dest.z - source.z;

    missile.momz = deltaZ / flyTics;

    // Explode the missile intially if it's already in a wall, and return the missile created
    P_CheckMissileSpawn(missile);
    return &missile;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a missile for the player and try to do a little auto-aim
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnPlayerMissile(mobj_t& source, const mobjtype_t missileType) noexcept {
    // Figure out the vertical aim slope.
    // If we don't hit a thing then do a bit of auto-aim and wiggle the aim a bit to try and hit something.
    angle_t aimAngle = source.angle;
    fixed_t aimSlope = P_AimLineAttack(source, aimAngle, 1024 * FRACUNIT);

    if (!gpLineTarget->get()) {
        constexpr angle_t AIM_WIGGLE = ANG45 / 8;
        
        aimAngle = source.angle + AIM_WIGGLE;
        aimSlope = P_AimLineAttack(source, aimAngle, 1024 * FRACUNIT);
        
        if (!gpLineTarget->get()) {
            aimAngle = source.angle - AIM_WIGGLE;
            aimSlope = P_AimLineAttack(source, aimAngle, 1024 * FRACUNIT);

            // If we still haven't hit a thing after all these attempts then just shoot dead level ahead
            if (!gpLineTarget->get()) {
                aimAngle = source.angle;
                aimSlope = 0;
            }
        }
    }

    // Spawn the missile and make the fire sound
    mobj_t& missile = *P_SpawnMobj(source.x, source.y, source.z + 32 * FRACUNIT, missileType);
    
    if (missile.info->seesound != sfx_None) {
        S_StartSound(&source, missile.info->seesound);
    }

    // Set the missile velocity and angle and save the firer (for damage blame) 
    const int32_t missileSpeed = missile.info->speed >> FRACBITS;
    
    missile.target = &source;
    missile.angle = aimAngle;
    missile.momx = gFineCosine[aimAngle >> ANGLETOFINESHIFT] * missileSpeed;
    missile.momy = gFineSine[aimAngle >> ANGLETOFINESHIFT] * missileSpeed;
    missile.momz = aimSlope * missileSpeed;

    // If the missile is already in collision with something then explode it
    P_CheckMissileSpawn(missile);
}
