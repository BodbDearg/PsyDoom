#include "p_change.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "g_game.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "PsxVm/PsxVm.h"

void P_ThingHeightClip() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    s0 = lw(s1 + 0x8);
    v0 = lw(s1 + 0x38);
    a1 = lw(s1);
    a2 = lw(s1 + 0x4);
    s0 ^= v0;
    s0 = (s0 < 1);
    P_CheckPosition();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E18);                               // Load from: gTmFloorZ (800781E8)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F04);                               // Load from: gTmCeilingZ (80077F04)
    sw(v0, s1 + 0x38);
    sw(v1, s1 + 0x3C);
    if (s0 == 0) goto loc_80014FFC;
    v0 = lw(s1 + 0x38);
    sw(v0, s1 + 0x8);
    goto loc_8001501C;
loc_80014FFC:
    v0 = lw(s1 + 0x8);
    a0 = lw(s1 + 0x44);
    v0 += a0;
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - a0;
        if (bJump) goto loc_8001501C;
    }
    sw(v0, s1 + 0x8);
loc_8001501C:
    v0 = lw(s1 + 0x3C);
    v1 = lw(s1 + 0x38);
    a0 = lw(s1 + 0x44);
    v0 -= v1;
    v0 = (i32(v0) < i32(a0));
    v0 ^= 1;
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

bool PIT_ChangeSector(mobj_t& mobj) noexcept {
    a0 = ptrToVmAddr(&mobj);

    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    a1 = lw(s1);
    s0 = lw(s1 + 0x8);
    v0 = lw(s1 + 0x38);
    a2 = lw(s1 + 0x4);
    s0 ^= v0;
    s0 = (s0 < 1);
    P_CheckPosition();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E18);                               // Load from: gTmFloorZ (800781E8)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F04);                               // Load from: gTmCeilingZ (80077F04)
    sw(v0, s1 + 0x38);
    sw(v1, s1 + 0x3C);
    if (s0 == 0) goto loc_800150A4;
    v0 = lw(s1 + 0x38);
    sw(v0, s1 + 0x8);
    goto loc_800150C4;
loc_800150A4:
    v0 = lw(s1 + 0x8);
    a0 = lw(s1 + 0x44);
    v0 += a0;
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - a0;
        if (bJump) goto loc_800150C4;
    }
    sw(v0, s1 + 0x8);
loc_800150C4:
    v0 = lw(s1 + 0x3C);
    v1 = lw(s1 + 0x38);
    a0 = lw(s1 + 0x44);
    v0 -= v1;
    v0 = (i32(v0) < i32(a0));
    v0 ^= 1;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015220;
    }
    v0 = lw(s1 + 0x68);
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 0x20000;                                   // Result = 00020000
        if (bJump) goto loc_80015158;
    }
    a0 = s1;
    a1 = 0x29F;                                         // Result = 0000029F
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
    a0 = s1;
    a1 = sfx_slop;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = *gCurPlayerIndex;
    a0 = lw(s1 + 0x80);
    sw(0, s1 + 0x44);
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    sw(0, s1 + 0x40);
    if (a0 != v0) goto loc_8001521C;
    v0 = 1;                                             // Result = 00000001
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78CC);                                // Store to: gStatusBar[7] (80098734)
    goto loc_80015220;
loc_80015158:
    v1 = lw(s1 + 0x64);
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 4;
        if (bJump) goto loc_8001517C;
    }
    a0 = s1;
    P_RemoveMobj(*vmAddrToPtr<mobj_t>(a0));
    v0 = 1;                                             // Result = 00000001
    goto loc_80015220;
loc_8001517C:
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001521C;
    }
    v1 = lw(gp + 0x9C0);                                // Load from: gbCrushChange (80077FA0)
    sw(v0, gp + 0x8DC);                                 // Store to: gbNofit (80077EBC)
    if (v1 == 0) goto loc_80015220;
    v0 = *gGameTic;
    v0 &= 3;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80015220;
    }
    a0 = s1;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0xA;                                           // Result = 0000000A
    P_DamageMObj();
    a3 = 0x1C;                                          // Result = 0000001C
    a0 = lw(s1);
    a2 = lw(s1 + 0x44);
    a1 = lw(s1 + 0x4);
    v0 = a2 >> 31;
    a2 += v0;
    v0 = lw(s1 + 0x8);
    a2 = u32(i32(a2) >> 1);
    a2 += v0;
    v0 = ptrToVmAddr(P_SpawnMObj(a0, a1, a2, (mobjtype_t) a3));
    s1 = v0;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 12;
    sw(s0, s1 + 0x48);
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 12;
    sw(s0, s1 + 0x4C);
loc_8001521C:
    v0 = 1;                                             // Result = 00000001
loc_80015220:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return (v0 != 0);
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
