#include "p_ceiling.h"

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

static constexpr int32_t MAXCEILINGS    = 30;               // Maximum size of the 'active ceilings' list
static constexpr fixed_t CEILSPEED      = FRACUNIT * 2;     // Normal move speed for ceilings/crushers

static const VmPtr<VmPtr<ceiling_t>[MAXCEILINGS]>   gpActiveCeilings(0x800A9D18);

// Not required externally: making private to this module
static void P_AddActiveCeiling(ceiling_t& ceiling) noexcept;
static void P_RemoveActiveCeiling(ceiling_t& ceiling) noexcept;
static void P_ActivateInStasisCeiling(line_t& line) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a moving ceiling or crusher: moves the ceiling, does state transitions and sounds etc.
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_MoveCeiling(ceiling_t& ceiling) noexcept {
    sector_t& ceilingSector = *ceiling.sector;

    switch (ceiling.direction) {
        // In stasis
        case 0:
            break;

        // Moving up
        case 1: {
            const result_e moveResult = T_MovePlane(ceilingSector, ceiling.speed, ceiling.topheight, false, 1, ceiling.direction);

            // Do moving sounds
            if ((*gGameTic & 7) == 0) {
                switch (ceiling.type) {
                    case silentCrushAndRaise:
                        break;

                    default:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_stnmov);
                        break;
                }
            }

            // Reached the destination?
            if (moveResult == pastdest) {
                switch (ceiling.type) {
                    case raiseToHighest:
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    case silentCrushAndRaise:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_pstop);
                        ceiling.direction = -1;
                        break;

                    case fastCrushAndRaise:
                    case crushAndRaise:
                        ceiling.direction = -1;
                        break;

                    default:
                        break;
                }
            }
        }   break;

        case -1: {
            const result_e moveResult = T_MovePlane(ceilingSector, ceiling.speed, ceiling.bottomheight, ceiling.crush, 1, ceiling.direction);

            if ((*gGameTic & 7) == 0) {
                switch (ceiling.type) {
                    case silentCrushAndRaise:
                        break;

                    default:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_stnmov);
                        break;
                }
            }

            if (moveResult == pastdest) {
                // Reached the destination
                switch (ceiling.type) {
                    case silentCrushAndRaise:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_pstop);
                    case crushAndRaise:
                        ceiling.speed = CEILSPEED;
                    case fastCrushAndRaise:
                        ceiling.direction = 1;
                        break;

                    case lowerToFloor:
                    case lowerAndCrush:
                        P_RemoveActiveCeiling(ceiling);
                        break;
                }
            }
            else if (moveResult == crushed) {
                // Crushing/hitting something
                switch (ceiling.type) {
                    case lowerAndCrush:
                    case crushAndRaise:
                    case silentCrushAndRaise:
                        ceiling.speed = CEILSPEED / 8;
                        break;

                    default:
                        break;
                }
            }
        }   break;
    }
}

void EV_DoCeiling() noexcept {
loc_80014C44:
    sp -= 0x38;
    sw(s5, sp + 0x24);
    s5 = a0;
    sw(s3, sp + 0x1C);
    s3 = a1;
    sw(s2, sp + 0x18);
    s2 = -1;                                            // Result = FFFFFFFF
    sw(s6, sp + 0x28);
    s6 = 0;                                             // Result = 00000000
    v0 = (s3 < 6);
    sw(ra, sp + 0x30);
    sw(s7, sp + 0x2C);
    sw(s4, sp + 0x20);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_80014C98;
    v0 = (s3 < 3);
    if (v0 != 0) goto loc_80014C98;
    P_ActivateInStasisCeiling(*vmAddrToPtr<line_t>(a0));
loc_80014C98:
    v1 = 0x80010000;                                    // Result = 80010000
    v1 += 0x18;                                         // Result = JumpTable_EV_DoCeiling[0] (80010018)
    v0 = s3 << 2;
    s7 = v0 + v1;
    s4 = 1;                                             // Result = 00000001
loc_80014CAC:
    a0 = s5;
loc_80014CB0:
    a1 = s2;
    v0 = P_FindSectorFromLineTag(*vmAddrToPtr<line_t>(a0), a1);
    s2 = v0;
    v0 = s2 << 1;
    if (i32(s2) < 0) goto loc_80014DE4;
    v0 += s2;
    v0 <<= 3;
    v0 -= s2;
    v1 = *gpSectors;
    v0 <<= 2;
    s1 = v0 + v1;
    v0 = lw(s1 + 0x50);
    a1 = 0x30;                                          // Result = 00000030
    if (v0 != 0) goto loc_80014CAC;
    s6 = 1;                                             // Result = 00000001
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x4A30;                                       // Result = T_MoveCeiling (80014A30)
    sw(s0, s1 + 0x50);
    sw(v0, s0 + 0x8);
    v0 = (s3 < 6);
    sw(s1, s0 + 0x10);
    sw(0, s0 + 0x20);
    if (v0 == 0) goto loc_80014DC8;
    v0 = lw(s7);
    switch (v0) {
        case 0x80014D88: goto loc_80014D88;
        case 0x80014DB0: goto loc_80014DB0;
        case 0x80014D78: goto loc_80014D78;
        case 0x80014D44: goto loc_80014D44;
        default: jump_table_err(); break;
    }
loc_80014D44:
    sw(s4, s0 + 0x20);
    v0 = lw(s1 + 0x4);
    sw(v0, s0 + 0x18);
    v1 = lw(s1);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x24);
    v0 = 0x40000;                                       // Result = 00040000
    sw(v0, s0 + 0x1C);
    v0 = 0x80000;                                       // Result = 00080000
    v1 += v0;
    sw(v1, s0 + 0x14);
    goto loc_80014DC8;
loc_80014D78:
    sw(s4, s0 + 0x20);
    v0 = lw(s1 + 0x4);
    sw(v0, s0 + 0x18);
loc_80014D88:
    v1 = lw(s1);
    sw(v1, s0 + 0x14);
    if (s3 == 0) goto loc_80014DA0;
    v0 = 0x80000;                                       // Result = 00080000
    v0 += v1;
    sw(v0, s0 + 0x14);
loc_80014DA0:
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x24);
    v0 = 0x20000;                                       // Result = 00020000
    goto loc_80014DC4;
loc_80014DB0:
    a0 = s1;
    v0 = P_FindHighestCeilingSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x18);
    v0 = 0x20000;                                       // Result = 00020000
    sw(s4, s0 + 0x24);
loc_80014DC4:
    sw(v0, s0 + 0x1C);
loc_80014DC8:
    v0 = lw(s1 + 0x18);
    a0 = s0;
    sw(s3, a0 + 0xC);
    sw(v0, a0 + 0x28);
    P_AddActiveCeiling(*vmAddrToPtr<ceiling_t>(a0));
    a0 = s5;
    goto loc_80014CB0;
loc_80014DE4:
    v0 = s6;
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

// TODO: REMOVE eventually
void _thunk_T_MoveCeiling() noexcept {
    T_MoveCeiling(*vmAddrToPtr<ceiling_t>(a0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add the given ceiling mover to a free slot in the 'active ceilings' list.
// Note: does NOT get added if there are no free slots.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_AddActiveCeiling(ceiling_t& ceiling) noexcept {
    for (int32_t i = 0; i < MAXCEILINGS; ++i) {
        VmPtr<ceiling_t>& pCeiling = gpActiveCeilings[i];

        if (!pCeiling) {
            pCeiling = &ceiling;
            return;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remove the given ceiling from the active ceilings list, and also disassociate with it's sector and dealloc it's thinker
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_RemoveActiveCeiling(ceiling_t& ceiling) noexcept {
    for (int32_t i = 0; i < MAXCEILINGS; ++i) {
        VmPtr<ceiling_t>& pCeiling = gpActiveCeilings[i];

        if (pCeiling.get() == &ceiling) {
            ceiling.sector->specialdata = nullptr;
            P_RemoveThinker(ceiling.thinker);
            pCeiling = nullptr;
            return;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpauses ceiling movers which have the same tag as the given line and which are in stasis (paused)
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ActivateInStasisCeiling(line_t& line) noexcept {
    for (int32_t i = 0; i < MAXCEILINGS; ++i) {
        ceiling_t* const pCeiling = gpActiveCeilings[i].get();

        if (pCeiling && (pCeiling->tag == line.tag) && (pCeiling->direction == 0)) {    // Direction 0 = in stasis
            pCeiling->direction = pCeiling->olddirection;
            pCeiling->thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_MoveCeiling);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pauses active ceiling movers (crushers) with the same sector tag as the given line's tag
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_CeilingCrushStop(line_t& line) noexcept {
    bool bPausedACrusher = false;

    for (int32_t i = 0; i < MAXCEILINGS; ++i) {
        ceiling_t* const pCeiling = gpActiveCeilings[i].get();

        if (pCeiling && (pCeiling->tag == line.tag) && (pCeiling->direction != 0)) {
            pCeiling->olddirection = pCeiling->direction;       // Remember which direction it was moving in for unpause
            pCeiling->direction = 0;                            // Now in stasis
            pCeiling->thinker.function = nullptr;               // Remove the thinker function until unpaused
            bPausedACrusher = true;
        }
    }

    return bPausedACrusher;
}
