#include "p_plats.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_local.h"
#include "g_game.h"
#include "p_floor.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"

// Current status for a moving platform
enum plat_e : uint32_t {
    up,
    down,
    waiting,
    in_stasis
};

// Moving platform type
enum plattype_e : uint32_t {
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS
};

// Status and state for a moving platform
struct plat_t {
    thinker_t           thinker;
    VmPtr<sector_t>     sector;
    fixed_t             speed;
    fixed_t             low;            // TODO: COMMENT
    fixed_t             high;           // TODO: COMMENT
    int32_t             wait;           // TODO: COMMENT
    int32_t             count;          // TODO: COMMENT
    plat_e              status;         // TODO: COMMENT
    plat_e              oldstatus;      // TODO: COMMENT
    bool32_t            crush;
    int32_t             tag;            // TODO: COMMENT
    plattype_e          type;
};

static_assert(sizeof(plat_t) == 56);

// Maximum number of active moving platforms
static constexpr int32_t MAXPLATS = 30;

// Contains all of the active platforms in the level (some slots may be empty)
static const VmPtr<VmPtr<plat_t>[MAXPLATS]> gpActivePlats(0x80097C44);

// Not required externally: making private to this module
static void P_AddActivePlat(plat_t& plat) noexcept;
static void P_RemoveActivePlat(plat_t& plat) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a moving platform: moves the platform, does state transitions and sounds etc.
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_PlatRaise(plat_t& plat) noexcept {
    sector_t& sector = *plat.sector;

    switch (plat.status) {
        // Going up
        case up: {
            const result_e moveResult = T_MovePlane(sector, plat.speed, plat.high, plat.crush, false, 1);

            // Do movement sounds every so often for certain platform types
            if ((plat.type == raiseAndChange) || (plat.type == raiseToNearestAndChange)) {
                if ((gGameTic & 7U) == 0) {
                    S_StartSound((mobj_t*) &sector.soundorg, sfx_stnmov);
                }
            }

            // Decide what to do base on the move result and platform settings
            if ((moveResult == crushed) && (!plat.crush)) {
                // Crushing something and this platform doesn't crush: change direction
                plat.status = down;
                plat.count = plat.wait;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstart);
            }
            else if (moveResult == pastdest) {
                // Reached the destination for the platform: wait by default (overrides below) and play the stopped sound
                plat.status = waiting;
                plat.count = plat.wait;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstop);

                // Certain platform types stop now at this point
                switch (plat.type) {
                    case downWaitUpStay:
                    case raiseAndChange:
                    case blazeDWUS:
                        P_RemoveActivePlat(plat);
                        break;
                }
            }
        }   break;

        // Going down
        case down: {
            const result_e moveResult = T_MovePlane(sector, plat.speed, plat.low, false, 0, -1);
            
            // Time to start waiting before going back up again?
            if (moveResult == pastdest) {
                plat.status = waiting;
                plat.count = plat.wait;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstop);
            }
        }   break;

        // Waiting to go back up (or down) again?
        case waiting: {
            plat.count--;

            // Time to end the wait and begin moving again?
            if (plat.count == 0) {
                plat.status = (sector.floorheight == plat.low) ? up : down;     // Go back in the direction we came from
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstart);
            }
        }   break;

        case in_stasis:
            break;
    }
}

// TODO: REMOVE eventually
void _thunk_T_PlatRaise() noexcept {
    T_PlatRaise(*vmAddrToPtr<plat_t>(*PsxVm::gpReg_a0));
}

void EV_DoPlat() noexcept {
loc_8001F464:
    sp -= 0x38;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s4, sp + 0x20);
    s4 = a1;
    sw(s7, sp + 0x2C);
    s7 = a2;
    sw(s2, sp + 0x18);
    s2 = -1;                                            // Result = FFFFFFFF
    sw(s5, sp + 0x24);
    s5 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x30);
    sw(s6, sp + 0x28);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    if (s4 != 0) goto loc_8001F4B0;
    a0 = lw(s3 + 0x18);
    P_ActivateInStasis(a0);
loc_8001F4B0:
    s6 = 0x2D;                                          // Result = 0000002D
loc_8001F4B4:
    a0 = s3;
loc_8001F4B8:
    a1 = s2;
    v0 = P_FindSectorFromLineTag(*vmAddrToPtr<line_t>(a0), a1);
    s2 = v0;
    v0 = s2 << 1;
    if (i32(s2) < 0) goto loc_8001F72C;
    v0 += s2;
    v0 <<= 3;
    v0 -= s2;
    v1 = *gpSectors;
    v0 <<= 2;
    s1 = v0 + v1;
    v0 = lw(s1 + 0x50);
    a1 = 0x38;                                          // Result = 00000038
    if (v0 != 0) goto loc_8001F4B4;
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0xD80;                                        // Result = T_PlatRaise (8001F280)
    sw(s4, s0 + 0x34);
    sw(s1, s0 + 0xC);
    sw(s0, s1 + 0x50);
    sw(v0, s0 + 0x8);
    sw(0, s0 + 0x2C);
    v0 = lw(s3 + 0x18);
    sw(v0, s0 + 0x30);
    v0 = (s4 < 5);
    s5 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8001F71C;
    v0 = s4 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += 0x8E8;                                        // Result = JumpTable_EV_DoPlat[0] (800108E8)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x8001F6AC: goto loc_8001F6AC;
        case 0x8001F61C: goto loc_8001F61C;
        case 0x8001F5C8: goto loc_8001F5C8;
        case 0x8001F56C: goto loc_8001F56C;
        case 0x8001F664: goto loc_8001F664;
        default: jump_table_err(); break;
    }
loc_8001F56C:
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s0 + 0x10);
    v1 = lw(s3 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v1 = *gpSides;
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x14);
    a1 = lw(s1);
    v0 = lw(v0 + 0x8);
    a0 = s1;
    sw(v0, s1 + 0x8);
    v0 = P_FindNextHighestFloor(*vmAddrToPtr<sector_t>(a0), a1);
    a0 = s1 + 0x38;
    a1 = 0x15;                                          // Result = 00000015
    sw(v0, s0 + 0x18);
    sw(0, s0 + 0x1C);
    sw(0, s0 + 0x24);
    sw(0, s1 + 0x14);
    goto loc_8001F714;
loc_8001F5C8:
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s0 + 0x10);
    v1 = lw(s3 + 0x1C);
    a0 = s1 + 0x38;
    v0 = v1 << 1;
    v0 += v1;
    v1 = *gpSides;
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x14);
    v1 = lw(s1);
    v0 = lw(v0 + 0x8);
    a1 = 0x15;                                          // Result = 00000015
    sw(v0, s1 + 0x8);
    v0 = s7 << 16;
    v0 += v1;
    sw(v0, s0 + 0x18);
    sw(0, s0 + 0x1C);
    sw(0, s0 + 0x24);
    goto loc_8001F714;
loc_8001F61C:
    a0 = s1;
    v0 = 0x80000;                                       // Result = 00080000
    sw(v0, s0 + 0x10);
    v0 = P_FindLowestFloorSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x14);
    v1 = lw(s1);
    v0 = (i32(v1) < i32(v0));
    a0 = s1 + 0x38;
    if (v0 == 0) goto loc_8001F648;
    sw(v1, s0 + 0x14);
loc_8001F648:
    a1 = 0x11;                                          // Result = 00000011
    v1 = lw(s1);
    v0 = 1;                                             // Result = 00000001
    sw(s6, s0 + 0x1C);
    sw(v0, s0 + 0x24);
    sw(v1, s0 + 0x18);
    goto loc_8001F714;
loc_8001F664:
    a0 = s1;
    v0 = 0x100000;                                      // Result = 00100000
    sw(v0, s0 + 0x10);
    v0 = P_FindLowestFloorSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x14);
    v1 = lw(s1);
    v0 = (i32(v1) < i32(v0));
    a0 = s1 + 0x38;
    if (v0 == 0) goto loc_8001F690;
    sw(v1, s0 + 0x14);
loc_8001F690:
    a1 = 0x11;                                          // Result = 00000011
    v1 = lw(s1);
    v0 = 1;                                             // Result = 00000001
    sw(s6, s0 + 0x1C);
    sw(v0, s0 + 0x24);
    sw(v1, s0 + 0x18);
    goto loc_8001F714;
loc_8001F6AC:
    a0 = s1;
    v0 = 0x20000;                                       // Result = 00020000
    sw(v0, s0 + 0x10);
    v0 = P_FindLowestFloorSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x14);
    v1 = lw(s1);
    v0 = (i32(v1) < i32(v0));
    if (v0 == 0) goto loc_8001F6D8;
    sw(v1, s0 + 0x14);
loc_8001F6D8:
    a0 = s1;
    v0 = P_FindHighestFloorSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x18);
    v1 = lw(s1);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001F6FC;
    sw(v1, s0 + 0x18);
loc_8001F6FC:
    sw(s6, s0 + 0x1C);
    _thunk_P_Random();
    a0 = s1 + 0x38;
    a1 = sfx_pstart;
    v0 &= 1;
    sw(v0, s0 + 0x24);
loc_8001F714:
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_8001F71C:
    a0 = s0;
    P_AddActivePlat(*vmAddrToPtr<plat_t>(a0));
    a0 = s3;
    goto loc_8001F4B8;
loc_8001F72C:
    v0 = s5;
    ra = lw(sp + 0x30);
    s7 = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x38;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reactivates moving platforms that were paused which match the given sector tag
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ActivateInStasis(const int32_t tag) noexcept {
    for (int32_t i = 0; i < MAXPLATS; ++i) {
        plat_t* pPlat = gpActivePlats[i].get();

        if (pPlat && (pPlat->tag == tag) && (pPlat->status == in_stasis)) {
            pPlat->status = pPlat->oldstatus;
            pPlat->thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_PlatRaise);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops moving platforms that are moving that have a sector tag matching the given line's tag
//------------------------------------------------------------------------------------------------------------------------------------------
void EV_StopPlat(line_t& line) noexcept {
    for (int32_t i = 0; i < MAXPLATS; ++i) {
        plat_t* const pPlat = gpActivePlats[i].get();

        if (pPlat && (pPlat->status != in_stasis) && (pPlat->tag == line.tag)) {
            // Stop this moving platform: remember the status before stopping and put into stasis
            pPlat->oldstatus = pPlat->status;
            pPlat->status = in_stasis;
            pPlat->thinker.function = nullptr;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add the given moving platform to a free slot in the 'active platforms' list
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_AddActivePlat(plat_t& plat) noexcept {
    for (int32_t i = 0; i < MAXPLATS; ++i) {
        VmPtr<plat_t>& pPlat = gpActivePlats[i];

        if (!pPlat) {
            pPlat = &plat;
            return;
        }
    }

    I_Error("P_AddActivePlat: no more plats!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remove the given moving platform from the active platforms list; also dissociates it with it's sector and deallocs it's thinker
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_RemoveActivePlat(plat_t& plat) noexcept {
    for (int32_t i = 0; i < MAXPLATS; ++i) {
        VmPtr<plat_t>& pPlat = gpActivePlats[i];

        if (pPlat.get() == &plat) {
            plat.sector->specialdata = nullptr;
            P_RemoveThinker(plat.thinker);
            pPlat = nullptr;
            return;
        }
    }

    I_Error("P_RemoveActivePlat: can\'t find plat!");
}
