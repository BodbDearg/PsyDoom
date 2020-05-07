#include "p_doors.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_local.h"
#include "p_floor.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"

static constexpr fixed_t VDOORSPEED = FRACUNIT * 6;     // Regular speed of vertical doors
static constexpr int32_t VDOORWAIT  = 70;               // How long vertical doors normally wait before closing (game tics)

void T_VerticalDoor() noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    v1 = lw(s0 + 0x1C);
    if (v1 == 0) goto loc_80015354;
    a2 = 1;                                             // Result = 00000001
    if (i32(v1) > 0) goto loc_8001533C;
    v0 = -1;                                            // Result = FFFFFFFF
    s1 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_800153E0;
    goto loc_80015528;
loc_8001533C:
    v0 = 2;                                             // Result = 00000002
    if (v1 == a2) goto loc_800154A8;
    if (v1 == v0) goto loc_800153AC;
    goto loc_80015528;
loc_80015354:
    v0 = lw(s0 + 0x24);
    v0--;
    sw(v0, s0 + 0x24);
    if (v0 != 0) goto loc_80015528;
    v1 = lw(s0 + 0xC);
    v0 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_8001548C;
    v0 = 5;                                             // Result = 00000005
    if (v1 == 0) goto loc_80015398;
    a1 = 0x58;                                          // Result = 00000058
    if (v1 != v0) goto loc_80015528;
    a0 = lw(s0 + 0x10);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x1C);
    goto loc_80015498;
loc_80015398:
    a1 = 0x14;                                          // Result = 00000014
    a0 = lw(s0 + 0x10);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x1C);
    goto loc_80015498;
loc_800153AC:
    v0 = lw(s0 + 0x24);
    v0--;
    sw(v0, s0 + 0x24);
    if (v0 != 0) goto loc_80015528;
    v1 = lw(s0 + 0xC);
    v0 = 4;                                             // Result = 00000004
    a1 = 0x13;                                          // Result = 00000013
    if (v1 != v0) goto loc_80015528;
    a0 = lw(s0 + 0x10);
    sw(a2, s0 + 0x1C);
    sw(0, s0 + 0xC);
    goto loc_80015498;
loc_800153E0:
    v1 = lw(s0 + 0x10);
    sw(s1, sp + 0x10);
    v0 = lw(s0 + 0x1C);
    sw(v0, sp + 0x14);
    a0 = lw(s0 + 0x10);
    a1 = lw(s0 + 0x18);
    a2 = lw(v1);
    a3 = 0;                                             // Result = 00000000
    v0 = T_MovePlane(*vmAddrToPtr<sector_t>(a0), a1, a2, a3, lw(sp + 0x10), lw(sp + 0x14));
    v1 = v0;
    v0 = 2;                                             // Result = 00000002
    if (v1 != v0) goto loc_8001546C;
    v1 = lw(s0 + 0xC);
    v0 = (v1 < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80015528;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x78;                                         // Result = JumpTable_T_VerticalDoor_1[0] (80010078)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80015518: goto loc_80015518;
        case 0x80015464: goto loc_80015464;
        case 0x80015528: goto loc_80015528;
        case 0x80015448: goto loc_80015448;
        default: jump_table_err(); break;
    }
loc_80015448:
    v0 = lw(s0 + 0x10);
    a0 = s0;
    sw(0, v0 + 0x50);
    _thunk_P_RemoveThinker();
    a0 = lw(s0 + 0x10);
    a1 = 0x58;                                          // Result = 00000058
    goto loc_80015498;
loc_80015464:
    v0 = 0x1C2;                                         // Result = 000001C2
    goto loc_8001550C;
loc_8001546C:
    if (v1 != s1) goto loc_80015528;
    a0 = lw(s0 + 0xC);
    {
        const bool bJump = (a0 == v0);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80015528;
    }
    if (a0 == v0) goto loc_80015528;
loc_8001548C:
    a0 = lw(s0 + 0x10);
    a1 = sfx_doropn;
    sw(v1, s0 + 0x1C);
loc_80015498:
    a0 += 0x38;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    goto loc_80015528;
loc_800154A8:
    sw(v1, sp + 0x10);
    v0 = lw(s0 + 0x1C);
    sw(v0, sp + 0x14);
    a0 = lw(s0 + 0x10);
    a1 = lw(s0 + 0x18);
    a2 = lw(s0 + 0x14);
    a3 = 0;                                             // Result = 00000000
    v0 = T_MovePlane(*vmAddrToPtr<sector_t>(a0), a1, a2, a3, lw(sp + 0x10), lw(sp + 0x14));
    v1 = 2;                                             // Result = 00000002
    if (v0 != v1) goto loc_80015528;
    v1 = lw(s0 + 0xC);
    v0 = (v1 < 7);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80015528;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x98;                                         // Result = JumpTable_T_VerticalDoor_2[0] (80010098)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80015508: goto loc_80015508;
        case 0x80015518: goto loc_80015518;
        case 0x80015528: goto loc_80015528;
        default: jump_table_err(); break;
    }
loc_80015508:
    v0 = lw(s0 + 0x20);
loc_8001550C:
    sw(0, s0 + 0x1C);
    sw(v0, s0 + 0x24);
    goto loc_80015528;
loc_80015518:
    v0 = lw(s0 + 0x10);
    a0 = s0;
    sw(0, v0 + 0x50);
    _thunk_P_RemoveThinker();
loc_80015528:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void EV_DoLockedDoor() noexcept {
loc_80015540:
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = lw(a1 + 0x80);
    if (s0 != 0) goto loc_80015564;
loc_8001555C:
    v0 = 0;                                             // Result = 00000000
    goto loc_80015750;
loc_80015564:
    v0 = lw(a0 + 0x14);
    v1 = v0 - 0x1A;
    v0 = (v1 < 0x70);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8001555C;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0xB8;                                         // Result = JumpTable_EV_DoLockedDoor[0] (800100B8)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80015598: goto loc_80015598;
        case 0x8001562C: goto loc_8001562C;
        case 0x800156C0: goto loc_800156C0;
        case 0x8001555C: goto loc_8001555C;
        default: jump_table_err(); break;
    }
loc_80015598:
    v0 = lw(s0 + 0x4C);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015750;
    }
    v0 = lw(s0 + 0x58);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015750;
    }
    a0 = a1;
    a1 = sfx_oof;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x30;                                         // Result = STR_BlueKeyNeededMsg[0] (80010030)
    sw(v0, s0 + 0xD4);
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    {
        const bool bJump = (s0 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80015750;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7E9C);                               // Load from: gMapBlueKeyType (80077E9C)
    v1 = 1;                                             // Result = 00000001
    v0 <<= 2;
    at = 0x800A0000;                                    // Result = 800A0000
    at -= 0x78E4;                                       // Result = gStatusBar[1] (8009871C)
    at += v0;
    sw(v1, at);
    v0 = 0;                                             // Result = 00000000
    goto loc_80015750;
loc_8001562C:
    v0 = lw(s0 + 0x50);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015750;
    }
    v0 = lw(s0 + 0x5C);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015750;
    }
    a0 = a1;
    a1 = sfx_oof;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x48;                                         // Result = STR_YellowKeyNeededMsg[0] (80010048)
    sw(v0, s0 + 0xD4);
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    {
        const bool bJump = (s0 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80015750;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F60);                               // Load from: gMapYellowKeyType (800780A0)
    v1 = 1;                                             // Result = 00000001
    v0 <<= 2;
    at = 0x800A0000;                                    // Result = 800A0000
    at -= 0x78E4;                                       // Result = gStatusBar[1] (8009871C)
    at += v0;
    sw(v1, at);
    v0 = 0;                                             // Result = 00000000
    goto loc_80015750;
loc_800156C0:
    v0 = lw(s0 + 0x48);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015750;
    }
    v0 = lw(s0 + 0x54);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015750;
    }
    a0 = a1;
    a1 = sfx_oof;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x60;                                         // Result = STR_RedKeyNeededMsg[0] (80010060)
    sw(v0, s0 + 0xD4);
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    {
        const bool bJump = (s0 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80015750;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7DE4);                               // Load from: gMapRedKeyType (8007821C)
    v1 = 1;                                             // Result = 00000001
    v0 <<= 2;
    at = 0x800A0000;                                    // Result = 800A0000
    at -= 0x78E4;                                       // Result = gStatusBar[1] (8009871C)
    at += v0;
    sw(v1, at);
    v0 = 0;                                             // Result = 00000000
loc_80015750:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// For each sector matching the given line's tag, spawn a door thinker/process of the given door type.
// Note that if a sector already has some special action going on, then a door thinker will NOT be added to the sector.
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoDoor(line_t& line, const vldoor_e doorType) noexcept {
    bool bActivatedADoor = false;

    // Create door thinkers for all sectors matching the given line tag
    for (int32_t secIdx = P_FindSectorFromLineTag(line, -1); secIdx >= 0; secIdx = P_FindSectorFromLineTag(line, secIdx)) {
        // Only spawn the door if there isn't already a special operating on this sector
        sector_t& sector = gpSectors->get()[secIdx];

        if (sector.specialdata)
            continue;

        // Create the door thinker and populate its state/settings
        bActivatedADoor = true;

        vldoor_t& door = *(vldoor_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(vldoor_t), PU_LEVSPEC, nullptr);
        P_AddThinker(door.thinker);

        door.thinker.function = PsxVm::getNativeFuncVmAddr(T_VerticalDoor);
        door.type = doorType;
        door.sector = &sector;
        door.speed = VDOORSPEED;
        door.topwait = VDOORWAIT;

        // This thinker is now the special for the sector
        sector.specialdata = ptrToVmAddr(&door);

        // Door specific setup and sounds
        switch (doorType) {
            case BlazeClose:
                door.topheight = P_FindLowestCeilingSurrounding(sector) -4 * FRACUNIT;
                door.direction = -1;
                door.speed = VDOORSPEED * 4;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_bdcls);
                break;

            case Close:
                door.topheight = P_FindLowestCeilingSurrounding(sector) -4 * FRACUNIT;
                door.direction = -1;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_dorcls);
                break;

            case Close30ThenOpen:
                door.topheight = sector.ceilingheight;
                door.direction = -1;
                S_StartSound((mobj_t*) &door.sector->soundorg, sfx_dorcls);
                break;

            case BlazeRaise:
            case BlazeOpen: {
                door.topheight = P_FindLowestCeilingSurrounding(sector) -4 * FRACUNIT;
                door.direction = 1;
                door.speed = VDOORSPEED * 4;

                // Only play a sound if not already fully open
                if (door.topheight != sector.ceilingheight) {
                    S_StartSound((mobj_t*) &sector.soundorg, sfx_bdopn);
                }
            }   break;

            case Normal:
            case Open: {
                door.topheight = P_FindLowestCeilingSurrounding(sector) -4 * FRACUNIT;
                door.direction = 1;

                // Only play a sound if not already fully open
                if (door.topheight != sector.ceilingheight) {
                    S_StartSound((mobj_t*) &sector.soundorg, sfx_doropn);
                }
            }   break;

            default:
                break;
        }
    }

    return bActivatedADoor;
}

void EV_VerticalDoor() noexcept {
loc_80015988:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x20);
    v1 = v0 << 1;
    v1 += v0;
    v0 = *gpSides;
    v1 <<= 3;
    v1 += v0;
    s2 = lw(v1 + 0x14);
    v0 = lw(s2 + 0x50);
    s0 = v0;
    if (v0 == 0) goto loc_80015A40;
    v1 = lw(s1 + 0x14);
    v0 = (i32(v1) < 0x1D);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0x1A);
        if (bJump) goto loc_80015A04;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015A10;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0x77);
        if (bJump) goto loc_80015A10;
    }
    goto loc_80015A4C;
loc_80015A04:
    v0 = 0x75;                                          // Result = 00000075
    if (v1 != v0) goto loc_80015A40;
loc_80015A10:
    v0 = lw(s0 + 0x1C);
    v1 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != v1);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015A28;
    }
    sw(v0, s0 + 0x1C);
    goto loc_80015B68;
loc_80015A28:
    v0 = lw(a1 + 0x80);
    if (v0 == 0) goto loc_80015B68;
    sw(v1, s0 + 0x1C);
    goto loc_80015B68;
loc_80015A40:
    v1 = lw(s1 + 0x14);
    v0 = (i32(v1) < 0x77);
loc_80015A4C:
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0x75);
        if (bJump) goto loc_80015A64;
    }
    a1 = 0x57;                                          // Result = 00000057
    if (v0 != 0) goto loc_80015A64;
    a0 = s2 + 0x38;
    goto loc_80015A6C;
loc_80015A64:
    a0 = s2 + 0x38;
    a1 = sfx_doropn;
loc_80015A6C:
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    a1 = 0x28;                                          // Result = 00000028
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x52FC;                                       // Result = T_VerticalDoor (800152FC)
    a0 = 1;                                             // Result = 00000001
    sw(s0, s2 + 0x50);
    sw(v0, s0 + 0x8);
    v0 = 0x60000;                                       // Result = 00060000
    sw(v0, s0 + 0x18);
    v0 = 0x46;                                          // Result = 00000046
    sw(s2, s0 + 0x10);
    sw(a0, s0 + 0x1C);
    sw(v0, s0 + 0x20);
    v1 = lw(s1 + 0x14);
    v0 = (i32(v1) < 0x23);
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(v1) < 0x1F);
        if (bJump) goto loc_80015AF8;
    }
    v0 = 0x75;                                          // Result = 00000075
    {
        const bool bJump = (v1 == v0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_80015B38;
    }
    v0 = 0x76;                                          // Result = 00000076
    {
        const bool bJump = (v1 == v0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_80015B44;
    }
    goto loc_80015B54;
loc_80015AF8:
    if (v0 == 0) goto loc_80015B28;
    if (v1 == a0) goto loc_80015B20;
    v0 = (i32(v1) < 0x1D);
    if (i32(v1) <= 0) goto loc_80015B54;
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0x1A);
        if (bJump) goto loc_80015B54;
    }
    if (v0 != 0) goto loc_80015B54;
loc_80015B20:
    sw(0, s0 + 0xC);
    goto loc_80015B54;
loc_80015B28:
    v0 = 3;                                             // Result = 00000003
    sw(v0, s0 + 0xC);
    sw(0, s1 + 0x14);
    goto loc_80015B54;
loc_80015B38:
    sw(v0, s0 + 0xC);
    v0 = 0x180000;                                      // Result = 00180000
    goto loc_80015B50;
loc_80015B44:
    sw(v0, s0 + 0xC);
    v0 = 0x180000;                                      // Result = 00180000
    sw(0, s1 + 0x14);
loc_80015B50:
    sw(v0, s0 + 0x18);
loc_80015B54:
    a0 = s2;
    v0 = P_FindLowestCeilingSurrounding(*vmAddrToPtr<sector_t>(a0));
    v1 = 0xFFFC0000;                                    // Result = FFFC0000
    v0 += v1;
    sw(v0, s0 + 0x14);
loc_80015B68:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_SpawnDoorCloseIn30() noexcept {
loc_80015B84:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    a1 = 0x28;                                          // Result = 00000028
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x52FC;                                       // Result = T_VerticalDoor (800152FC)
    sw(s0, s1 + 0x50);
    sw(0, s1 + 0x14);
    sw(v0, s0 + 0x8);
    v0 = 0x60000;                                       // Result = 00060000
    sw(v0, s0 + 0x18);
    v0 = 0x1C2;                                         // Result = 000001C2
    sw(s1, s0 + 0x10);
    sw(0, s0 + 0x1C);
    sw(0, s0 + 0xC);
    sw(v0, s0 + 0x24);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_SpawnDoorRaiseIn5Mins() noexcept {
loc_80015C04:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    a1 = 0x28;                                          // Result = 00000028
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    a0 = s1;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x52FC;                                       // Result = T_VerticalDoor (800152FC)
    sw(s0, a0 + 0x50);
    sw(0, a0 + 0x14);
    sw(v0, s0 + 0x8);
    v0 = 2;                                             // Result = 00000002
    sw(v0, s0 + 0x1C);
    v0 = 4;                                             // Result = 00000004
    sw(v0, s0 + 0xC);
    v0 = 0x60000;                                       // Result = 00060000
    sw(a0, s0 + 0x10);
    sw(v0, s0 + 0x18);
    v0 = P_FindLowestCeilingSurrounding(*vmAddrToPtr<sector_t>(a0));
    v1 = 0xFFFC0000;                                    // Result = FFFC0000
    v0 += v1;
    sw(v0, s0 + 0x14);
    v0 = 0x46;                                          // Result = 00000046
    sw(v0, s0 + 0x20);
    v0 = 0x1194;                                        // Result = 00001194
    sw(v0, s0 + 0x24);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
