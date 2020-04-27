#include "p_user.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_main.h"
#include "g_game.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_slide.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"

void P_PlayerMove() noexcept {
    v0 = *gPlayerNum;
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v1 = lw(s0 + 0x48);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    v1 = u32(i32(v1) >> 2);
    mult(v1, a0);
    v0 = lw(s0 + 0x4C);
    s2 = lo;
    v0 = u32(i32(v0) >> 2);
    mult(v0, a0);
    sw(s0, gp + 0x8F8);                                 // Store to: gpSlideThing (80077ED8)
    s1 = lo;
    P_SlideMove();
    s3 = 0x80070000;                                    // Result = 80070000
    s3 = lw(s3 + 0x7F9C);                               // Load from: gpSpecialLine (80077F9C)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7F90);                               // Load from: gSlideX (80077F90)
    v0 = lw(s0);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7F94);                               // Load from: gSlideY (80077F94)
    if (a1 != v0) goto loc_80029838;
    v0 = lw(s0 + 0x4);
    {
        const bool bJump = (a2 == v0);
        v0 = 0x100000;                                  // Result = 00100000
        if (bJump) goto loc_80029848;
    }
loc_80029838:
    a0 = s0;
    P_TryMove();
    {
        const bool bJump = (v0 != 0);
        v0 = 0x100000;                                  // Result = 00100000
        if (bJump) goto loc_800298E8;
    }
loc_80029848:
    v0 = (i32(v0) < i32(s2));
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFF00000;                                // Result = FFF00000
        if (bJump) goto loc_8002985C;
    }
    s2 = 0x100000;                                      // Result = 00100000
    goto loc_8002986C;
loc_8002985C:
    v0 = (i32(s2) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 0x100000;                                  // Result = 00100000
        if (bJump) goto loc_80029870;
    }
    s2 = 0xFFF00000;                                    // Result = FFF00000
loc_8002986C:
    v0 = 0x100000;                                      // Result = 00100000
loc_80029870:
    v0 = (i32(v0) < i32(s1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFF00000;                                // Result = FFF00000
        if (bJump) goto loc_80029884;
    }
    s1 = 0x100000;                                      // Result = 00100000
    goto loc_80029894;
loc_80029884:
    v0 = (i32(s1) < i32(v0));
    a0 = s0;
    if (v0 == 0) goto loc_80029898;
    s1 = 0xFFF00000;                                    // Result = FFF00000
loc_80029894:
    a0 = s0;
loc_80029898:
    a2 = lw(s0 + 0x4);
    a1 = lw(s0);
    a2 += s1;
    P_TryMove();
    a0 = s0;
    if (v0 == 0) goto loc_800298BC;
    sw(0, s0 + 0x48);
    sw(s1, s0 + 0x4C);
    goto loc_800298E8;
loc_800298BC:
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a1 += s2;
    P_TryMove();
    if (v0 == 0) goto loc_800298E0;
    sw(s2, s0 + 0x48);
    sw(0, s0 + 0x4C);
    goto loc_800298E8;
loc_800298E0:
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
loc_800298E8:
    a0 = s3;
    if (s3 == 0) goto loc_800298F8;
    a1 = s0;
    P_CrossSpecialLine();
loc_800298F8:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_PlayerXYMovement() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    P_PlayerMove();
    v0 = lw(s0 + 0x8);
    a0 = lw(s0 + 0x38);
    v0 = (i32(a0) < i32(v0));
    v1 = 0x100000;                                      // Result = 00100000
    if (v0 != 0) goto loc_800299F4;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    if (v0 == 0) goto loc_80029978;
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v0 = lw(v0);
    if (a0 != v0) goto loc_800299F4;
loc_80029978:
    v0 = lw(s0 + 0x48);
    v0 += 0xFFF;
    v0 = (v0 < 0x1FFF);
    if (v0 == 0) goto loc_800299B4;
    v0 = lw(s0 + 0x4C);
    v0 += 0xFFF;
    v0 = (v0 < 0x1FFF);
    if (v0 == 0) goto loc_800299B4;
    sw(0, s0 + 0x48);
    sw(0, s0 + 0x4C);
    goto loc_800299F4;
loc_800299B4:
    v0 = lw(s0 + 0x48);
    a0 = lw(s0 + 0x4C);
    v0 = u32(i32(v0) >> 8);
    v1 = v0 << 3;
    v1 -= v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 1;
    a0 = u32(i32(a0) >> 8);
    v1 = a0 << 3;
    v1 -= a0;
    sw(v0, s0 + 0x48);
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 1;
    sw(v0, s0 + 0x4C);
loc_800299F4:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_PlayerZMovement() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = lw(s0 + 0x8);
    v1 = lw(s0 + 0x38);
    v0 = (i32(a1) < i32(v1));
    v1 -= a1;
    if (v0 == 0) goto loc_80029A64;
    a0 = lw(s0 + 0x80);
    v0 = lw(a0 + 0x18);
    v0 -= v1;
    sw(v0, a0 + 0x18);
    a0 = lw(s0 + 0x80);
    v1 = lw(a0 + 0x18);
    v0 = 0x290000;                                      // Result = 00290000
    v0 -= v1;
    v0 = u32(i32(v0) >> 2);
    sw(v0, a0 + 0x1C);
loc_80029A64:
    v0 = lw(s0 + 0x8);
    a0 = lw(s0 + 0x50);
    v1 = lw(s0 + 0x38);
    v0 += a0;
    v1 = (i32(v1) < i32(v0));
    sw(v0, s0 + 0x8);
    if (v1 != 0) goto loc_80029AC0;
    a0 = lw(s0 + 0x50);
    v0 = 0xFFF80000;                                    // Result = FFF80000
    if (i32(a0) >= 0) goto loc_80029AB4;
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = u32(i32(a0) >> 3);
        if (bJump) goto loc_80029AB0;
    }
    a0 = s0;
    v1 = lw(s0 + 0x80);
    a1 = sfx_oof;
    sw(v0, v1 + 0x1C);
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_80029AB0:
    sw(0, s0 + 0x50);
loc_80029AB4:
    v0 = lw(s0 + 0x38);
    sw(v0, s0 + 0x8);
    goto loc_80029AE0;
loc_80029AC0:
    v1 = lw(s0 + 0x50);
    v0 = 0xFFFE0000;                                    // Result = FFFE0000
    if (v1 != 0) goto loc_80029AD8;
    v0 = 0xFFFC0000;                                    // Result = FFFC0000
    goto loc_80029ADC;
loc_80029AD8:
    v0 += v1;
loc_80029ADC:
    sw(v0, s0 + 0x50);
loc_80029AE0:
    v0 = lw(s0 + 0x8);
    a0 = lw(s0 + 0x44);
    v1 = lw(s0 + 0x3C);
    v0 += a0;
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80029B24;
    v0 = lw(s0 + 0x50);
    if (i32(v0) <= 0) goto loc_80029B10;
    sw(0, s0 + 0x50);
loc_80029B10:
    v0 = lw(s0 + 0x3C);
    v1 = lw(s0 + 0x44);
    v0 -= v1;
    sw(v0, s0 + 0x8);
loc_80029B24:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_PlayerMobjThink() noexcept {
loc_80029B38:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lw(s0 + 0x48);
    if (v0 != 0) goto loc_80029B68;
    v0 = lw(s0 + 0x4C);
    if (v0 == 0) goto loc_80029C38;
loc_80029B68:
    a0 = s0;
    P_PlayerMove();
    v0 = lw(s0 + 0x8);
    a0 = lw(s0 + 0x38);
    v0 = (i32(a0) < i32(v0));
    v1 = 0x100000;                                      // Result = 00100000
    if (v0 != 0) goto loc_80029C38;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    if (v0 == 0) goto loc_80029BBC;
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v0 = lw(v0);
    if (a0 != v0) goto loc_80029C38;
loc_80029BBC:
    v0 = lw(s0 + 0x48);
    v0 += 0xFFF;
    v0 = (v0 < 0x1FFF);
    if (v0 == 0) goto loc_80029BF8;
    v0 = lw(s0 + 0x4C);
    v0 += 0xFFF;
    v0 = (v0 < 0x1FFF);
    if (v0 == 0) goto loc_80029BF8;
    sw(0, s0 + 0x48);
    sw(0, s0 + 0x4C);
    goto loc_80029C38;
loc_80029BF8:
    v0 = lw(s0 + 0x48);
    a0 = lw(s0 + 0x4C);
    v0 = u32(i32(v0) >> 8);
    v1 = v0 << 3;
    v1 -= v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 1;
    a0 = u32(i32(a0) >> 8);
    v1 = a0 << 3;
    v1 -= a0;
    sw(v0, s0 + 0x48);
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 1;
    sw(v0, s0 + 0x4C);
loc_80029C38:
    a1 = lw(s0 + 0x8);
    v1 = lw(s0 + 0x38);
    v0 = (i32(a1) < i32(v1));
    if (a1 != v1) goto loc_80029C5C;
    v0 = lw(s0 + 0x50);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a1) < i32(v1));
        if (bJump) goto loc_80029D58;
    }
loc_80029C5C:
    v1 -= a1;
    if (v0 == 0) goto loc_80029C98;
    a0 = lw(s0 + 0x80);
    v0 = lw(a0 + 0x18);
    v0 -= v1;
    sw(v0, a0 + 0x18);
    a0 = lw(s0 + 0x80);
    v1 = lw(a0 + 0x18);
    v0 = 0x290000;                                      // Result = 00290000
    v0 -= v1;
    v0 = u32(i32(v0) >> 2);
    sw(v0, a0 + 0x1C);
loc_80029C98:
    v0 = lw(s0 + 0x8);
    a0 = lw(s0 + 0x50);
    v1 = lw(s0 + 0x38);
    v0 += a0;
    v1 = (i32(v1) < i32(v0));
    sw(v0, s0 + 0x8);
    if (v1 != 0) goto loc_80029CF4;
    a1 = lw(s0 + 0x50);
    v0 = 0xFFF80000;                                    // Result = FFF80000
    if (i32(a1) >= 0) goto loc_80029CE8;
    v0 = (i32(a1) < i32(v0));
    a0 = s0;
    if (v0 == 0) goto loc_80029CE4;
    v0 = u32(i32(a1) >> 3);
    v1 = lw(s0 + 0x80);
    a1 = sfx_oof;
    sw(v0, v1 + 0x1C);
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_80029CE4:
    sw(0, s0 + 0x50);
loc_80029CE8:
    v0 = lw(s0 + 0x38);
    sw(v0, s0 + 0x8);
    goto loc_80029D14;
loc_80029CF4:
    v1 = lw(s0 + 0x50);
    v0 = 0xFFFE0000;                                    // Result = FFFE0000
    if (v1 != 0) goto loc_80029D0C;
    v0 = 0xFFFC0000;                                    // Result = FFFC0000
    goto loc_80029D10;
loc_80029D0C:
    v0 += v1;
loc_80029D10:
    sw(v0, s0 + 0x50);
loc_80029D14:
    v0 = lw(s0 + 0x8);
    a0 = lw(s0 + 0x44);
    v1 = lw(s0 + 0x3C);
    v0 += a0;
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80029D58;
    v0 = lw(s0 + 0x50);
    if (i32(v0) <= 0) goto loc_80029D44;
    sw(0, s0 + 0x50);
loc_80029D44:
    v0 = lw(s0 + 0x3C);
    v1 = lw(s0 + 0x44);
    v0 -= v1;
    sw(v0, s0 + 0x8);
loc_80029D58:
    v1 = lw(s0 + 0x5C);
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 == v0);
        v0 = v1 - 1;
        if (bJump) goto loc_80029DC0;
    }
    sw(v0, s0 + 0x5C);
    if (i32(v0) > 0) goto loc_80029DC0;
    v0 = lw(s0 + 0x60);
    v1 = lw(v0 + 0x10);
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
    sw(v0, s0 + 0x2C);
loc_80029DC0:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_BuildMove() noexcept {
loc_80029DD4:
    sp -= 0x18;
    v0 = *gPlayerNum;
    a1 = a0;
    sw(ra, sp + 0x10);
    v0 <<= 2;
    at = ptrToVmAddr(&gpPlayerCtrlBindings[0]);
    at += v0;
    t0 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    a2 = lw(at);
    v1 = lw(t0 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gOldTicButtons[0] (80078214)
    at += v0;
    a0 = lw(at);
    a3 = a2 & v1;
    v1 = a2 & 0x8000;
    a3 = (i32(a3) > 0);
    if (v1 == 0) goto loc_80029E3C;
    v0 = a0 & 0x8000;
    if (v0 != 0) goto loc_80029E50;
loc_80029E3C:
    v0 = a2 & 0x2000;
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 0x2000;
        if (bJump) goto loc_80029E64;
    }
    if (v0 == 0) goto loc_80029E64;
loc_80029E50:
    v0 = lw(a1 + 0x128);
    v0++;
    sw(v0, a1 + 0x128);
    goto loc_80029E68;
loc_80029E64:
    sw(0, a1 + 0x128);
loc_80029E68:
    v0 = lw(a1 + 0x128);
    v0 = (i32(v0) < 0xA);
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_80029E80;
    }
    sw(v0, a1 + 0x128);
loc_80029E80:
    sw(0, a1 + 0x10);
    sw(0, a1 + 0xC);
    sw(0, a1 + 0x8);
    v0 = lw(t0 + 0x10);
    v0 &= a2;
    v1 = a3 << 2;
    if (v0 == 0) goto loc_80029EDC;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7840;                                       // Result = SideMove[0] (80067840)
    at += v1;
    v1 = lw(at);
    v0 = -v0;
    mult(v0, v1);
    goto loc_80029F28;
loc_80029EDC:
    v0 = lw(t0 + 0x14);
    v0 &= a2;
    if (v0 == 0) goto loc_80029F40;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7840;                                       // Result = SideMove[0] (80067840)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
loc_80029F28:
    v0 = lo;
    if (i32(v0) >= 0) goto loc_80029F38;
    v0 += 3;
loc_80029F38:
    v0 = u32(i32(v0) >> 2);
    sw(v0, a1 + 0xC);
loc_80029F40:
    v0 = lw(t0 + 0x8);
    v0 &= a2;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x8000;
        if (bJump) goto loc_80029FF8;
    }
    v1 = a3 << 2;
    if (v0 == 0) goto loc_80029F98;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7840;                                       // Result = SideMove[0] (80067840)
    at += v1;
    v1 = lw(at);
    v0 = -v0;
    mult(v0, v1);
    goto loc_80029FDC;
loc_80029F98:
    v0 = a2 & 0x2000;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_8002A190;
    }
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7840;                                       // Result = SideMove[0] (80067840)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
loc_80029FDC:
    v0 = lo;
    if (i32(v0) >= 0) goto loc_80029FEC;
    v0 += 3;
loc_80029FEC:
    v0 = u32(i32(v0) >> 2);
    sw(v0, a1 + 0xC);
    goto loc_8002A18C;
loc_80029FF8:
    v0 = a2 & 0x5000;
    if (a3 == 0) goto loc_8002A0C8;
    {
        const bool bJump = (v0 != 0);
        v0 = a2 & 0x8000;
        if (bJump) goto loc_8002A0CC;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x2000;
        if (bJump) goto loc_8002A064;
    }
    v0 = *gPlayerNum;
    v1 = lw(a1 + 0x128);
    v0 <<= 2;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7870;                                       // Result = FastAngleTurn[0] (80067870)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A05C;
    v0 += 3;
loc_8002A05C:
    v0 = u32(i32(v0) >> 2);
    goto loc_8002A184;
loc_8002A064:
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_8002A190;
    }
    v0 = *gPlayerNum;
    v1 = lw(a1 + 0x128);
    v0 <<= 2;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7870;                                       // Result = FastAngleTurn[0] (80067870)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A0B8;
    v0 += 3;
loc_8002A0B8:
    v0 = u32(i32(v0) >> 2);
    v0 <<= 17;
    v0 = -v0;
    goto loc_8002A188;
loc_8002A0C8:
    v0 = a2 & 0x8000;
loc_8002A0CC:
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x2000;
        if (bJump) goto loc_8002A128;
    }
    v0 = *gPlayerNum;
    v1 = lw(a1 + 0x128);
    v0 <<= 2;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7848;                                       // Result = AngleTurn[0] (80067848)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A120;
    v0 += 3;
loc_8002A120:
    v0 = u32(i32(v0) >> 2);
    goto loc_8002A184;
loc_8002A128:
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_8002A190;
    }
    v0 = *gPlayerNum;
    v1 = lw(a1 + 0x128);
    v0 <<= 2;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7848;                                       // Result = AngleTurn[0] (80067848)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A17C;
    v0 += 3;
loc_8002A17C:
    v0 = u32(i32(v0) >> 2);
    v0 = -v0;
loc_8002A184:
    v0 <<= 17;
loc_8002A188:
    sw(v0, a1 + 0x10);
loc_8002A18C:
    v0 = a2 & 0x1000;
loc_8002A190:
    v1 = a3 << 2;
    if (v0 == 0) goto loc_8002A1D0;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7838;                                       // Result = ForwardMove[0] (80067838)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    goto loc_8002A214;
loc_8002A1D0:
    v0 = a2 & 0x4000;
    if (v0 == 0) goto loc_8002A22C;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7838;                                       // Result = ForwardMove[0] (80067838)
    at += v1;
    v1 = lw(at);
    v0 = -v0;
    mult(v0, v1);
loc_8002A214:
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A224;
    v0 += 3;
loc_8002A224:
    v0 = u32(i32(v0) >> 2);
    sw(v0, a1 + 0x8);
loc_8002A22C:
    a0 = lw(a1);
    v0 = lw(a0 + 0x48);
    if (v0 != 0) goto loc_8002A2A8;
    v0 = lw(a0 + 0x4C);
    if (v0 != 0) goto loc_8002A2A8;
    v0 = lw(a1 + 0x8);
    if (v0 != 0) goto loc_8002A2A8;
    v0 = lw(a1 + 0xC);
    if (v0 != 0) goto loc_8002A2A8;
    v1 = lw(a0 + 0x60);
    a1 = 0x80060000;                                    // Result = 80060000
    a1 -= 0x6180;                                       // Result = State_S_PLAY_RUN1[0] (80059E80)
    v0 = a1 + 0x1C;                                     // Result = State_S_PLAY_RUN2[0] (80059E9C)
    if (v1 == a1) goto loc_8002A2A0;
    {
        const bool bJump = (v1 == v0);
        v0 = a1 + 0x38;                                 // Result = State_S_PLAY_RUN3[0] (80059EB8)
        if (bJump) goto loc_8002A2A0;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = a1 + 0x54;                                 // Result = State_S_PLAY_RUN4[0] (80059ED4)
        if (bJump) goto loc_8002A2A0;
    }
    if (v1 != v0) goto loc_8002A2A8;
loc_8002A2A0:
    a1 = 0x9A;                                          // Result = 0000009A
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
loc_8002A2A8:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_Thrust() noexcept {
    a1 >>= 19;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a1 <<= 2;
    v0 += a1;
    v0 = lw(v0);
    a2 = u32(i32(a2) >> 8);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a3 = lw(a0);
    v1 = lw(a3 + 0x48);
    v0 = lo;
    v0 += v1;
    sw(v0, a3 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a1;
    v0 = lw(at);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a0);
    v1 = lw(a0 + 0x4C);
    v0 = lo;
    v0 += v1;
    sw(v0, a0 + 0x4C);
    return;
}

void P_CalcHeight() noexcept {
loc_8002A32C:
    a1 = a0;
    v0 = lw(a1);
    v0 = lw(v0 + 0x48);
    v0 = u32(i32(v0) >> 8);
    mult(v0, v0);
    v0 = lw(a1);
    v1 = lo;
    sw(v1, a1 + 0x20);
    v0 = lw(v0 + 0x4C);
    v0 = u32(i32(v0) >> 8);
    mult(v0, v0);
    v0 = lo;
    v0 += v1;
    v0 = u32(i32(v0) >> 4);
    v1 = 0x100000;                                      // Result = 00100000
    sw(v0, a1 + 0x20);
    v0 = (i32(v1) < i32(v0));
    if (v0 == 0) goto loc_8002A388;
    sw(v1, a1 + 0x20);
loc_8002A388:
    v0 = lw(gp + 0xBEC);                                // Load from: gbOnGround (800781CC)
    a0 = 0x290000;                                      // Result = 00290000
    if (v0 != 0) goto loc_8002A3D0;
    v0 = lw(a1);
    v1 = lw(v0 + 0x8);
    v0 = lw(a1);
    v1 += a0;
    sw(v1, a1 + 0x14);
    a0 = lw(v0 + 0x3C);
    v0 = 0xFFFC0000;                                    // Result = FFFC0000
    a0 += v0;
    v1 = (i32(a0) < i32(v1));
    if (v1 == 0) goto loc_8002A4E0;
    sw(a0, a1 + 0x14);
    goto loc_8002A4E0;
loc_8002A3D0:
    v1 = *gTicCon;
    v0 = v1 << 1;
    v0 += v1;
    v1 = v0 << 4;
    v0 += v1;
    v0 <<= 4;
    v0 &= 0x7FF0;
    v1 = lw(a1 + 0x20);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    v0 = lw(at);
    v1 = u32(i32(v1) >> 17);
    mult(v1, v0);
    v0 = lw(a1 + 0x4);
    a0 = lo;
    if (v0 != 0) goto loc_8002A4A4;
    v0 = lw(a1 + 0x18);
    v1 = lw(a1 + 0x1C);
    v0 += v1;
    v1 = 0x290000;                                      // Result = 00290000
    sw(v0, a1 + 0x18);
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 0x140000;                                  // Result = 00140000
        if (bJump) goto loc_8002A44C;
    }
    sw(v1, a1 + 0x18);
    sw(0, a1 + 0x1C);
loc_8002A44C:
    v1 = lw(a1 + 0x18);
    v0 |= 0x7FFF;                                       // Result = 00147FFF
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 0x140000;                                  // Result = 00140000
        if (bJump) goto loc_8002A478;
    }
    v1 = lw(a1 + 0x1C);
    v0 |= 0x8000;                                       // Result = 00148000
    sw(v0, a1 + 0x18);
    if (i32(v1) > 0) goto loc_8002A488;
    v0 = 1;                                             // Result = 00000001
    sw(v0, a1 + 0x1C);
loc_8002A478:
    v0 = lw(a1 + 0x1C);
    if (v0 == 0) goto loc_8002A4A4;
loc_8002A488:
    v0 = lw(a1 + 0x1C);
    v1 = 0x8000;                                        // Result = 00008000
    v0 += v1;
    sw(v0, a1 + 0x1C);
    if (v0 != 0) goto loc_8002A4A4;
    v0 = 1;                                             // Result = 00000001
    sw(v0, a1 + 0x1C);
loc_8002A4A4:
    v0 = lw(a1);
    v1 = lw(a1 + 0x18);
    v0 = lw(v0 + 0x8);
    v0 += v1;
    v1 = lw(a1);
    v0 += a0;
    sw(v0, a1 + 0x14);
    a0 = lw(v1 + 0x3C);
    v1 = 0xFFFC0000;                                    // Result = FFFC0000
    a0 += v1;
    v0 = (i32(a0) < i32(v0));
    if (v0 == 0) goto loc_8002A4E0;
    sw(a0, a1 + 0x14);
loc_8002A4E0:
    return;
}

void P_MovePlayer() noexcept {
loc_8002A4E8:
    sp -= 0x18;
    a3 = a0;
    sw(ra, sp + 0x10);
    a0 = lw(a3);
    v1 = lw(a3 + 0x10);
    v0 = lw(a0 + 0x24);
    v0 += v1;
    sw(v0, a0 + 0x24);
    v0 = lw(a3);
    v1 = lw(v0 + 0x8);
    v0 = lw(v0 + 0x38);
    a2 = lw(a3 + 0x8);
    v0 = (i32(v0) < i32(v1));
    v0 ^= 1;
    sw(v0, gp + 0xBEC);                                 // Store to: gbOnGround (800781CC)
    if (a2 == 0) goto loc_8002A5B0;
    a2 = u32(i32(a2) >> 8);
    if (v0 == 0) goto loc_8002A5B0;
    a1 = lw(a3);
    v1 = lw(a1 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 >>= 19;
    v1 <<= 2;
    v0 += v1;
    v0 = lw(v0);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a1 + 0x48);
    v0 = lo;
    v0 += a0;
    sw(v0, a1 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v1;
    v0 = lw(at);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a3);
    v1 = lw(a0 + 0x4C);
    v0 = lo;
    v0 += v1;
    sw(v0, a0 + 0x4C);
loc_8002A5B0:
    a2 = lw(a3 + 0xC);
    if (a2 == 0) goto loc_8002A64C;
    v0 = lw(gp + 0xBEC);                                // Load from: gbOnGround (800781CC)
    {
        const bool bJump = (v0 == 0);
        v0 = 0xC0000000;                                // Result = C0000000
        if (bJump) goto loc_8002A64C;
    }
    a1 = lw(a3);
    v1 = lw(a1 + 0x24);
    v1 += v0;
    v1 >>= 19;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 <<= 2;
    v0 += v1;
    v0 = lw(v0);
    a2 = u32(i32(a2) >> 8);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a1 + 0x48);
    v0 = lo;
    v0 += a0;
    sw(v0, a1 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v1;
    v0 = lw(at);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a3);
    v1 = lw(a0 + 0x4C);
    v0 = lo;
    v0 += v1;
    sw(v0, a0 + 0x4C);
loc_8002A64C:
    v0 = lw(a3 + 0x8);
    if (v0 != 0) goto loc_8002A66C;
    v0 = lw(a3 + 0xC);
    if (v0 == 0) goto loc_8002A690;
loc_8002A66C:
    a0 = lw(a3);
    v1 = lw(a0 + 0x60);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x619C;                                       // Result = State_S_PLAY[0] (80059E64)
    if (v1 != v0) goto loc_8002A690;
    a1 = 0x9B;                                          // Result = 0000009B
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
loc_8002A690:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_DeathThink() noexcept {
loc_8002A6A0:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    P_MovePsprites();
    v1 = lw(s0 + 0x18);
    v0 = 0x80000;                                       // Result = 00080000
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFFF0000;                                // Result = FFFF0000
        if (bJump) goto loc_8002A6D0;
    }
    v0 += v1;
    sw(v0, s0 + 0x18);
loc_8002A6D0:
    v0 = lw(s0);
    v1 = lw(v0 + 0x8);
    v0 = lw(v0 + 0x38);
    v0 = (i32(v0) < i32(v1));
    v0 ^= 1;
    sw(v0, gp + 0xBEC);                                 // Store to: gbOnGround (800781CC)
    a0 = s0;
    P_CalcHeight();
    v1 = lw(s0 + 0xE0);
    if (v1 == 0) goto loc_8002A78C;
    v0 = lw(s0);
    if (v1 == v0) goto loc_8002A78C;
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    _thunk_R_PointToAngle2();
    t0 = 0xFC710000;                                    // Result = FC710000
    t0 |= 0xC71D;                                       // Result = FC71C71D
    v1 = 0xF8E30000;                                    // Result = F8E30000
    a1 = lw(s0);
    v1 |= 0x8E3A;                                       // Result = F8E38E3A
    a0 = lw(a1 + 0x24);
    a3 = v0;
    a2 = v0 - a0;
    v0 = a2 + t0;
    v1 = (v1 < v0);
    if (v1 == 0) goto loc_8002A768;
    sw(a3, a1 + 0x24);
    goto loc_8002A78C;
loc_8002A768:
    v0 = 0x38E0000;                                     // Result = 038E0000
    if (i32(a2) < 0) goto loc_8002A780;
    v0 |= 0x38E3;                                       // Result = 038E38E3
    v0 += a0;
    sw(v0, a1 + 0x24);
    goto loc_8002A7A0;
loc_8002A780:
    v0 = a0 + t0;
    sw(v0, a1 + 0x24);
    goto loc_8002A7A0;
loc_8002A78C:
    v0 = lw(s0 + 0xD8);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002A7A0;
    }
    sw(v0, s0 + 0xD8);
loc_8002A7A0:
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    v0 = lbu(at);
    {
        const bool bJump = (v0 == 0);
        v0 = 0x80000;                                   // Result = 00080000
        if (bJump) goto loc_8002A7E4;
    }
    v1 = lw(s0 + 0x18);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8002A7E4;
    }
    sw(v0, s0 + 0x4);
loc_8002A7E4:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_PlayerThink() noexcept {
loc_8002A7F8:
    v0 = *gPlayerNum;
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v1 = lw(s0 + 0x4);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    s2 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gOldTicButtons[0] (80078214)
    at += v0;
    a1 = lw(at);
    at = ptrToVmAddr(&gpPlayerCtrlBindings[0]);
    at += v0;
    s1 = lw(at);
    v0 = 0xA;                                           // Result = 0000000A
    if (v1 != 0) goto loc_8002AA14;
    a0 = lw(s0 + 0x70);
    {
        const bool bJump = (a0 != v0);
        v0 = a0 << 2;
        if (bJump) goto loc_8002A874;
    }
    a0 = lw(s0 + 0x6C);
    v0 = a0 << 2;
loc_8002A874:
    at = 0x80070000;                                    // Result = 80070000
    at += 0x408C;                                       // Result = gWeaponMicroIndexes[0] (8007408C)
    at += v0;
    a0 = lw(at);
    v1 = lw(s1 + 0x18);
    v0 = s2 & v1;
    a2 = a0;
    if (v0 == 0) goto loc_8002A924;
    v0 = a1 & v1;
    if (v0 != 0) goto loc_8002A924;
    if (a0 != 0) goto loc_8002A8C4;
    v0 = lw(s0 + 0x94);
    if (v0 == 0) goto loc_8002A8C4;
    a2 = 1;                                             // Result = 00000001
    goto loc_8002A9E0;
loc_8002A8C4:
    if (i32(a0) <= 0) goto loc_8002A9E0;
    a0--;
    v0 = a0 << 2;
    v0 += s0;
    v0 = lw(v0 + 0x74);
    if (v0 != 0) goto loc_8002A9E0;
    if (i32(a0) <= 0) goto loc_8002A9E0;
    a0--;
    v0 = a0 << 2;
    v1 = v0 + s0;
    v0 = lw(v1 + 0x74);
loc_8002A900:
    if (v0 != 0) goto loc_8002A9E0;
    if (i32(a0) <= 0) goto loc_8002A9E0;
    v1 -= 4;
    v0 = lw(v1 + 0x74);
    a0--;
    goto loc_8002A900;
loc_8002A924:
    v1 = lw(s1 + 0x1C);
    v0 = s2 & v1;
    {
        const bool bJump = (v0 == 0);
        v0 = a1 & v1;
        if (bJump) goto loc_8002A9E0;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a0) < 8);
        if (bJump) goto loc_8002A9E0;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
    a0++;
    v0 = a0 << 2;
    v0 += s0;
    v0 = lw(v0 + 0x74);
    {
        const bool bJump = (v0 != 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
    v0 = (i32(a0) < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
    a0++;
    v0 = a0 << 2;
    v1 = v0 + s0;
    v0 = lw(v1 + 0x74);
    {
        const bool bJump = (v0 != 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
loc_8002A98C:
    v0 = (i32(a0) < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
    v1 += 4;
    v0 = lw(v1 + 0x74);
    a0++;
    if (v0 == 0) goto loc_8002A98C;
    v0 = 8;                                             // Result = 00000008
loc_8002A9B0:
    if (a0 != v0) goto loc_8002A9E0;
    v0 = lw(s0 + 0x90);
    a0 = 7;                                             // Result = 00000007
    if (v0 != 0) goto loc_8002A9E0;
    v1 = s0 + 0x1C;
loc_8002A9CC:
    v1 -= 4;
    v0 = lw(v1 + 0x74);
    a0--;
    if (v0 == 0) goto loc_8002A9CC;
loc_8002A9E0:
    if (a0 == a2) goto loc_8002AA14;
    v0 = 8;                                             // Result = 00000008
    if (a0 != 0) goto loc_8002AA10;
    v1 = lw(s0 + 0x6C);
    if (v1 == v0) goto loc_8002AA10;
    v0 = lw(s0 + 0x94);
    v0 = (v0 > 0);
    a0 = v0 << 3;
loc_8002AA10:
    sw(a0, s0 + 0x70);
loc_8002AA14:
    v0 = *gbGamePaused;
    if (v0 != 0) goto loc_8002ACCC;
    a0 = lw(s0);
    P_PlayerMobjThink();
    a0 = s0;
    P_BuildMove();
    v1 = lw(s0 + 0x4);
    v0 = 1;                                             // Result = 00000001
    if (v1 != v0) goto loc_8002AA5C;
    a0 = s0;
    P_DeathThink();
    goto loc_8002ACCC;
loc_8002AA5C:
    v0 = lw(s0);
    v0 = lw(v0 + 0x64);
    v0 &= 0x80;
    {
        const bool bJump = (v0 == 0);
        v0 = 0xC800;                                    // Result = 0000C800
        if (bJump) goto loc_8002AA98;
    }
    a0 = lw(s0);
    sw(0, s0 + 0x10);
    sw(v0, s0 + 0x8);
    sw(0, s0 + 0xC);
    v0 = lw(a0 + 0x64);
    v1 = -0x81;                                         // Result = FFFFFF7F
    v0 &= v1;
    sw(v0, a0 + 0x64);
loc_8002AA98:
    v1 = lw(s0);
    v0 = lw(v1 + 0x78);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002AAB8;
    }
    sw(v0, v1 + 0x78);
    goto loc_8002AAC0;
loc_8002AAB8:
    a0 = s0;
    P_MovePlayer();
loc_8002AAC0:
    a0 = s0;
    P_CalcHeight();
    v0 = lw(s0);
    v0 = lw(v0 + 0xC);
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    if (v0 == 0) goto loc_8002AB08;
    a0 = s0;
    P_PlayerInSpecialSector();
    v1 = lw(s0 + 0x4);
    v0 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_8002ACCC;
loc_8002AB08:
    v0 = lw(s1 + 0x4);
    v0 &= s2;
    if (v0 == 0) goto loc_8002AB40;
    v0 = lw(s0 + 0xBC);
    if (v0 != 0) goto loc_8002AB44;
    a0 = s0;
    P_UseLines();
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0xBC);
    goto loc_8002AB44;
loc_8002AB40:
    sw(0, s0 + 0xBC);
loc_8002AB44:
    v0 = lw(s1);
    v0 &= s2;
    if (v0 == 0) goto loc_8002ABC8;
    a0 = lw(s0);
    a1 = 0x9F;                                          // Result = 0000009F
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
    v0 = lw(s0 + 0xB8);
    v0++;
    sw(v0, s0 + 0xB8);
    v0 = (i32(v0) < 0x1F);
    if (v0 != 0) goto loc_8002ABCC;
    v1 = *gPlayerNum;
    v0 = *gCurPlayerIndex;
    {
        const bool bJump = (v1 != v0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_8002ABCC;
    }
    v1 = lw(s0 + 0x6C);
    {
        const bool bJump = (v1 == v0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_8002ABB4;
    }
    if (v1 != v0) goto loc_8002ABCC;
loc_8002ABB4:
    v0 = 7;                                             // Result = 00000007
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78E8);                                // Store to: gStatusBar[0] (80098718)
    goto loc_8002ABCC;
loc_8002ABC8:
    sw(0, s0 + 0xB8);
loc_8002ABCC:
    a0 = s0;
    P_MovePsprites();
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002ACCC;
    v0 = lw(s0 + 0x34);
    {
        const bool bJump = (v0 == 0);
        v0++;
        if (bJump) goto loc_8002AC08;
    }
    sw(v0, s0 + 0x34);
loc_8002AC08:
    v0 = lw(s0 + 0x30);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002AC1C;
    }
    sw(v0, s0 + 0x30);
loc_8002AC1C:
    v0 = lw(s0 + 0x38);
    v1 = v0 - 1;
    if (v0 == 0) goto loc_8002AC7C;
    sw(v1, s0 + 0x38);
    if (v1 != 0) goto loc_8002AC50;
    a0 = lw(s0);
    v1 = 0x8FFF0000;                                    // Result = 8FFF0000
    v0 = lw(a0 + 0x64);
    v1 |= 0xFFFF;                                       // Result = 8FFFFFFF
    v0 &= v1;
    sw(v0, a0 + 0x64);
    goto loc_8002AC7C;
loc_8002AC50:
    v0 = (i32(v1) < 0x3D);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 7;
        if (bJump) goto loc_8002AC7C;
    }
    a0 = 0x70000000;                                    // Result = 70000000
    if (v0 != 0) goto loc_8002AC7C;
    v0 = lw(s0);
    v1 = lw(v0 + 0x64);
    v1 ^= a0;
    sw(v1, v0 + 0x64);
loc_8002AC7C:
    v0 = lw(s0 + 0x44);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002AC90;
    }
    sw(v0, s0 + 0x44);
loc_8002AC90:
    v0 = lw(s0 + 0x3C);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002ACA4;
    }
    sw(v0, s0 + 0x3C);
loc_8002ACA4:
    v0 = lw(s0 + 0xD8);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002ACB8;
    }
    sw(v0, s0 + 0xD8);
loc_8002ACB8:
    v0 = lw(s0 + 0xDC);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002ACCC;
    }
    sw(v0, s0 + 0xDC);
loc_8002ACCC:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
