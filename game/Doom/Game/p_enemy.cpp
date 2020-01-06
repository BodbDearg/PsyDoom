#include "p_enemy.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Renderer/r_main.h"
#include "g_game.h"
#include "p_doors.h"
#include "p_floor.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_sight.h"
#include "p_switch.h"
#include "PsxVm/PsxVm.h"

void P_CheckMeleeRange() noexcept {
    sp -= 0x18;
    a1 = a0;
    sw(ra, sp + 0x10);
    v0 = lw(a1 + 0x64);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80015D0C;
    }
    v0 = lw(a1 + 0x74);
    if (v0 != 0) goto loc_80015CE0;
    v0 = 0;                                             // Result = 00000000
    goto loc_80015D0C;
loc_80015CE0:
    v1 = lw(v0);
    a0 = lw(a1);
    v0 = lw(v0 + 0x4);
    a1 = lw(a1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v0 = v1 ^ 1;
loc_80015D0C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_CheckMissileRange() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v1 = lw(s1 + 0x64);
    v0 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x40;
        if (bJump) goto loc_80015D6C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = -0x41;                                     // Result = FFFFFFBF
        if (bJump) goto loc_80015D5C;
    }
    v0 &= v1;
    sw(v0, s1 + 0x64);
    v0 = 1;                                             // Result = 00000001
    goto loc_80015DE8;
loc_80015D5C:
    v0 = lw(s1 + 0x78);
    if (v0 == 0) goto loc_80015D74;
loc_80015D6C:
    v0 = 0;                                             // Result = 00000000
    goto loc_80015DE8;
loc_80015D74:
    v0 = lw(s1 + 0x74);
    a2 = lw(s1);
    v1 = lw(s1 + 0x4);
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    a0 = a2 - a0;
    a1 = v1 - a1;
    P_AproxDistance();
    v1 = lw(s1 + 0x58);
    a0 = 0xFFC00000;                                    // Result = FFC00000
    v1 = lw(v1 + 0x28);
    s0 = v0 + a0;
    if (v1 != 0) goto loc_80015DB4;
    v0 = 0xFF800000;                                    // Result = FF800000
    s0 += v0;
loc_80015DB4:
    v1 = lw(s1 + 0x54);
    v0 = 0xE;                                           // Result = 0000000E
    s0 = u32(i32(s0) >> 16);
    if (v1 != v0) goto loc_80015DC8;
    s0 = u32(i32(s0) >> 1);
loc_80015DC8:
    v0 = (i32(s0) < 0xC9);
    if (v0 != 0) goto loc_80015DD8;
    s0 = 0xC8;                                          // Result = 000000C8
loc_80015DD8:
    _thunk_P_Random();
    v0 = (i32(v0) < i32(s0));
    v0 ^= 1;
loc_80015DE8:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_Move() noexcept {
loc_80015E00:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lw(s0 + 0x6C);
    v0 = 8;                                             // Result = 00000008
    {
        const bool bJump = (v1 == v0);
        v1 <<= 2;
        if (bJump) goto loc_80015F10;
    }
    v0 = lw(s0 + 0x58);
    a0 = lw(v0 + 0x3C);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7060;                                       // Result = MoveXSpeed[0] (80067060)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    a1 = lo;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7080;                                       // Result = MoveYSpeed[0] (80067080)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lw(s0);
    a0 = s0;
    a1 += v0;
    v0 = lw(s0 + 0x4);
    a2 = lo;
    a2 += v0;
    P_TryMove();
    {
        const bool bJump = (v0 != 0);
        v0 = 0xFFDF0000;                                // Result = FFDF0000
        if (bJump) goto loc_80015F2C;
    }
    v0 = lw(s0 + 0x64);
    v0 &= 0x4000;
    if (v0 == 0) goto loc_80015EEC;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F84);                               // Load from: gbFloatOk (8007807C)
    if (v0 == 0) goto loc_80015EEC;
    v1 = lw(s0 + 0x8);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E18);                               // Load from: gTmFloorZ (800781E8)
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 0x80000;                                   // Result = 00080000
        if (bJump) goto loc_80015ECC;
    }
    v0 = 0xFFF80000;                                    // Result = FFF80000
loc_80015ECC:
    v0 += v1;
    sw(v0, s0 + 0x8);
    v0 = 1;                                             // Result = 00000001
    v1 = lw(s0 + 0x64);
    a0 = 0x200000;                                      // Result = 00200000
    v1 |= a0;
    sw(v1, s0 + 0x64);
    goto loc_80015F54;
loc_80015EEC:
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7DB8);                               // Load from: gpBlockLine (80078248)
    v0 = 0;                                             // Result = 00000000
    if (a1 == 0) goto loc_80015F54;
    v0 = lw(a1 + 0x14);
    {
        const bool bJump = (v0 != 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_80015F18;
    }
loc_80015F10:
    v0 = 0;                                             // Result = 00000000
    goto loc_80015F54;
loc_80015F18:
    sw(v0, s0 + 0x6C);
    a0 = s0;
    P_UseSpecialLine();
    v0 = (v0 > 0);
    goto loc_80015F54;
loc_80015F2C:
    v1 = lw(s0 + 0x64);
    v0 |= 0xFFFF;                                       // Result = FFDFFFFF
    v0 &= v1;
    v1 &= 0x4000;
    sw(v0, s0 + 0x64);
    if (v1 != 0) goto loc_80015F50;
    v0 = lw(s0 + 0x38);
    sw(v0, s0 + 0x8);
loc_80015F50:
    v0 = 1;                                             // Result = 00000001
loc_80015F54:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_TryWalk() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    P_Move();
    if (v0 == 0) goto loc_80015F9C;
    _thunk_P_Random();
    v0 &= 0xF;
    sw(v0, s0 + 0x70);
    v0 = 1;                                             // Result = 00000001
    goto loc_80015FA0;
loc_80015F9C:
    v0 = 0;                                             // Result = 00000000
loc_80015FA0:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_NewChaseDir() noexcept {
loc_80015FB4:
    sp -= 0x38;
    sw(s1, sp + 0x24);
    s1 = a0;
    sw(ra, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s0, sp + 0x20);
    v0 = lw(s1 + 0x74);
    if (v0 != 0) goto loc_80015FF4;
    I_Error("P_NewChaseDir: called with no target");
loc_80015FF4:
    s4 = lw(s1 + 0x6C);
    v0 = lw(s1 + 0x74);
    v1 = lw(s1);
    a2 = s4 << 2;
    a1 = lw(v0);
    a0 = lw(v0 + 0x4);
    v0 = lw(s1 + 0x4);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70A0;                                       // Result = OppositeDir[0] (800670A0)
    at += a2;
    s2 = lw(at);
    s3 = a1 - v1;
    s0 = a0 - v0;
    v0 = 0xA0000;                                       // Result = 000A0000
    v0 = (i32(v0) < i32(s3));
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFF60000;                                // Result = FFF60000
        if (bJump) goto loc_80016040;
    }
    sw(0, sp + 0x14);
    goto loc_80016058;
loc_80016040:
    v0 = (i32(s3) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80016050;
    }
    v0 = 8;                                             // Result = 00000008
loc_80016050:
    sw(v0, sp + 0x14);
    v0 = 0xFFF60000;                                    // Result = FFF60000
loc_80016058:
    v0 = (i32(s0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_80016078;
    }
    v0 = 0xA0000;                                       // Result = 000A0000
    v0 = (i32(v0) < i32(s0));
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80016078;
    }
    v0 = 8;                                             // Result = 00000008
loc_80016078:
    sw(v0, sp + 0x18);
    v0 = lw(sp + 0x14);
    v1 = 8;                                             // Result = 00000008
    if (v0 == v1) goto loc_80016100;
    v0 = lw(sp + 0x18);
    {
        const bool bJump = (v0 == v1);
        v0 = u32(i32(s0) >> 31);
        if (bJump) goto loc_80016100;
    }
    v0 &= 2;
    a0 = 0x80060000;                                    // Result = 80060000
    a0 += 0x70C4;                                       // Result = DiagonalDirs[0] (800670C4)
    v1 = v0;
    if (i32(s3) <= 0) goto loc_800160BC;
    v0++;
    v0 <<= 2;
    goto loc_800160C0;
loc_800160BC:
    v0 = v1 << 2;
loc_800160C0:
    v0 += a0;
    v0 = lw(v0);
    sw(v0, s1 + 0x6C);
    if (v0 == s2) goto loc_80016100;
    a0 = s1;
    P_Move();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800160F8;
    }
    _thunk_P_Random();
    v0 &= 0xF;
    sw(v0, s1 + 0x70);
    v0 = 1;                                             // Result = 00000001
loc_800160F8:
    if (v0 != 0) goto loc_80016310;
loc_80016100:
    _thunk_P_Random();
    v0 = (i32(v0) < 0xC9);
    if (v0 == 0) goto loc_80016138;
    v1 = s0;
    if (i32(s0) >= 0) goto loc_80016120;
    v1 = -v1;
loc_80016120:
    v0 = s3;
    if (i32(s3) >= 0) goto loc_8001612C;
    v0 = -v0;
loc_8001612C:
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_80016148;
loc_80016138:
    v0 = lw(sp + 0x18);
    s0 = lw(sp + 0x14);
    sw(v0, sp + 0x14);
    sw(s0, sp + 0x18);
loc_80016148:
    v0 = lw(sp + 0x14);
    {
        const bool bJump = (v0 != s2);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8001615C;
    }
    sw(v0, sp + 0x14);
loc_8001615C:
    v0 = lw(sp + 0x18);
    {
        const bool bJump = (v0 != s2);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_80016170;
    }
    sw(v0, sp + 0x18);
loc_80016170:
    v1 = lw(sp + 0x14);
    v0 = 8;                                             // Result = 00000008
    if (v1 == v0) goto loc_800161B0;
    sw(v1, s1 + 0x6C);
    a0 = s1;
    P_Move();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800161A8;
    }
    _thunk_P_Random();
    v0 &= 0xF;
    sw(v0, s1 + 0x70);
    v0 = 1;                                             // Result = 00000001
loc_800161A8:
    if (v0 != 0) goto loc_80016310;
loc_800161B0:
    v1 = lw(sp + 0x18);
    v0 = 8;                                             // Result = 00000008
    if (v1 == v0) goto loc_800161F0;
    sw(v1, s1 + 0x6C);
    a0 = s1;
    P_Move();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800161E8;
    }
    _thunk_P_Random();
    v0 &= 0xF;
    sw(v0, s1 + 0x70);
    v0 = 1;                                             // Result = 00000001
loc_800161E8:
    {
        const bool bJump = (v0 != 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_80016310;
    }
loc_800161F0:
    if (s4 == v0) goto loc_80016228;
    sw(s4, s1 + 0x6C);
    a0 = s1;
    P_Move();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80016220;
    }
    _thunk_P_Random();
    v0 &= 0xF;
    sw(v0, s1 + 0x70);
    v0 = 1;                                             // Result = 00000001
loc_80016220:
    if (v0 != 0) goto loc_80016310;
loc_80016228:
    _thunk_P_Random();
    v0 &= 1;
    s0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8001628C;
loc_8001623C:
    if (s0 == s2) goto loc_80016274;
    sw(s0, s1 + 0x6C);
    a0 = s1;
    P_Move();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001626C;
    }
    _thunk_P_Random();
    v0 &= 0xF;
    sw(v0, s1 + 0x70);
    v0 = 1;                                             // Result = 00000001
loc_8001626C:
    if (v0 != 0) goto loc_80016310;
loc_80016274:
    s0++;
    v0 = (s0 < 8);
    {
        const bool bJump = (v0 != 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8001623C;
    }
    goto loc_800162D4;
loc_8001628C:
    s0 = 7;                                             // Result = 00000007
loc_80016290:
    if (s0 == s2) goto loc_800162C8;
    sw(s0, s1 + 0x6C);
    a0 = s1;
    P_Move();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800162C0;
    }
    _thunk_P_Random();
    v0 &= 0xF;
    sw(v0, s1 + 0x70);
    v0 = 1;                                             // Result = 00000001
loc_800162C0:
    if (v0 != 0) goto loc_80016310;
loc_800162C8:
    s0--;
    v0 = 8;                                             // Result = 00000008
    if (i32(s0) >= 0) goto loc_80016290;
loc_800162D4:
    {
        const bool bJump = (s2 == v0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8001630C;
    }
    sw(s2, s1 + 0x6C);
    a0 = s1;
    P_Move();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80016304;
    }
    _thunk_P_Random();
    v0 &= 0xF;
    sw(v0, s1 + 0x70);
    v0 = 1;                                             // Result = 00000001
loc_80016304:
    {
        const bool bJump = (v0 != 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_80016310;
    }
loc_8001630C:
    sw(v0, s1 + 0x6C);
loc_80016310:
    ra = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}

void P_LookForPlayers() noexcept {
loc_80016334:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x64);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    if (v0 != 0) goto loc_800163E0;
loc_8001635C:
    v0 = gbPlayerInGame[1];
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_800163B0;
    _thunk_P_Random();
    a0 = v0 & 1;
    v0 = a0 << 2;
    v0 += a0;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x77F0;                                       // Result = gPlayer1[9] (800A8810)
    at += v1;
    v0 = lw(at);
    {
        const bool bJump = (i32(v0) > 0);
        v0 = a0 << 2;
        if (bJump) goto loc_800163B4;
    }
    a0 ^= 1;
loc_800163B0:
    v0 = a0 << 2;
loc_800163B4:
    v0 += a0;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    at += v1;
    v1 = lw(at);
    v0 = 0;                                             // Result = 00000000
    sw(v1, s1 + 0x74);
    goto loc_8001649C;
loc_800163E0:
    s0 = lw(s1 + 0x74);
    if (s0 == 0) goto loc_8001635C;
    v0 = lw(s0 + 0x68);
    if (i32(v0) <= 0) goto loc_8001635C;
    v0 = lw(s1 + 0xC);
    v0 = lw(v0);
    v0 = lw(v0 + 0x20);
    if (v0 != s0) goto loc_80016424;
    a1 = 1;                                             // Result = 00000001
loc_80016424:
    v0 = 1;                                             // Result = 00000001
    if (a1 != 0) goto loc_8001649C;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    a2 = lw(s0);
    a3 = lw(s0 + 0x4);
    R_PointToAngle2();
    a1 = 0xBFFF0000;                                    // Result = BFFF0000
    a1 |= 0xFFFF;                                       // Result = BFFFFFFF
    v1 = 0x7FFF0000;                                    // Result = 7FFF0000
    a0 = lw(s1 + 0x24);
    v1 |= 0xFFFE;                                       // Result = 7FFFFFFE
    v0 -= a0;
    v0 += a1;
    v1 = (v1 < v0);
    v0 = 1;                                             // Result = 00000001
    if (v1 != 0) goto loc_8001649C;
    v1 = lw(s0);
    a0 = lw(s1);
    v0 = lw(s0 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = 0x460000;                                      // Result = 00460000
    v1 = (i32(v1) < i32(v0));
    v0 = 0;                                             // Result = 00000000
    if (v1 != 0) goto loc_8001649C;
    v0 = 1;                                             // Result = 00000001
loc_8001649C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_Look() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = 0;                                             // Result = 00000000
    P_LookForPlayers();
    if (v0 != 0) goto loc_80016520;
    v0 = lw(s0 + 0xC);
    sw(0, s0 + 0x7C);
    v0 = lw(v0);
    v1 = lw(v0 + 0x20);
    if (v1 == 0) goto loc_800165CC;
    v0 = lw(v1 + 0x64);
    v0 &= 4;
    if (v0 == 0) goto loc_800165CC;
    v0 = lw(s0 + 0x64);
    v0 &= 0x20;
    if (v0 != 0) goto loc_800165CC;
    sw(v1, s0 + 0x74);
loc_80016520:
    v0 = lw(s0 + 0x58);
    v1 = lw(v0 + 0x10);
    v0 = (i32(v1) < 0x24);
    if (v1 == 0) goto loc_800165B8;
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(v1) < 0x27);
        if (bJump) goto loc_80016580;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0x31);
        if (bJump) goto loc_8001655C;
    }
    _thunk_P_Random();
    v0 &= 1;
    a1 = v0 + 0x24;
    goto loc_8001658C;
loc_8001655C:
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0x2F);
        if (bJump) goto loc_80016580;
    }
    if (v0 != 0) goto loc_80016580;
    _thunk_P_Random();
    v0 &= 1;
    a1 = v0 + 0x2F;
    goto loc_8001658C;
loc_80016580:
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x10);
loc_8001658C:
    v1 = lw(s0 + 0x54);
    v0 = 0xF;                                           // Result = 0000000F
    {
        const bool bJump = (v1 == v0);
        v0 = 0x11;                                      // Result = 00000011
        if (bJump) goto loc_800165A4;
    }
    if (v1 != v0) goto loc_800165AC;
loc_800165A4:
    a0 = 0;                                             // Result = 00000000
    goto loc_800165B0;
loc_800165AC:
    a0 = s0;
loc_800165B0:
    S_StartSound();
loc_800165B8:
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0xC);
    a0 = s0;
    P_SetMObjState();
loc_800165CC:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_Chase() noexcept {
loc_800165E0:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x78);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_80016608;
    }
    sw(v0, s1 + 0x78);
loc_80016608:
    v0 = lw(s1 + 0x7C);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8001661C;
    }
    sw(v0, s1 + 0x7C);
loc_8001661C:
    v0 = lw(s1 + 0x6C);
    v0 = (i32(v0) < 8);
    a0 = 0xE0000000;                                    // Result = E0000000
    if (v0 == 0) goto loc_80016664;
    v1 = lw(s1 + 0x24);
    v0 = lw(s1 + 0x6C);
    v1 &= a0;
    v0 <<= 29;
    v0 = v1 - v0;
    sw(v1, s1 + 0x24);
    if (i32(v0) <= 0) goto loc_80016654;
    v0 = v1 + a0;
    goto loc_80016660;
loc_80016654:
    {
        const bool bJump = (i32(v0) >= 0);
        v0 = 0x20000000;                                // Result = 20000000
        if (bJump) goto loc_80016664;
    }
    v0 += v1;
loc_80016660:
    sw(v0, s1 + 0x24);
loc_80016664:
    a1 = lw(s1 + 0x74);
    a0 = s1;
    if (a1 == 0) goto loc_80016688;
    v0 = lw(a1 + 0x64);
    v0 &= 4;
    if (v0 != 0) goto loc_800166AC;
loc_80016688:
    a1 = 1;                                             // Result = 00000001
    P_LookForPlayers();
    if (v0 != 0) goto loc_80016910;
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0x4);
    goto loc_8001675C;
loc_800166AC:
    v1 = lw(s1 + 0x64);
    v0 = v1 & 0x80;
    {
        const bool bJump = (v0 == 0);
        v0 = -0x81;                                     // Result = FFFFFF7F
        if (bJump) goto loc_800166D8;
    }
    v0 &= v1;
    sw(v0, s1 + 0x64);
    a0 = s1;
    P_NewChaseDir();
    goto loc_80016910;
loc_800166D8:
    v0 = lw(s1 + 0x58);
    v0 = lw(v0 + 0x28);
    {
        const bool bJump = (v0 == 0);
        v0 = 0x4000000;                                 // Result = 04000000
        if (bJump) goto loc_8001676C;
    }
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80016728;
    v1 = lw(a1);
    a0 = lw(s1);
    v0 = lw(a1 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_80016728:
    if (v1 == 0) goto loc_8001676C;
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0x18);
    if (a1 == 0) goto loc_80016754;
    a0 = s1;
    S_StartSound();
    v0 = lw(s1 + 0x58);
loc_80016754:
    a1 = lw(v0 + 0x28);
loc_8001675C:
    a0 = s1;
    P_SetMObjState();
    goto loc_80016910;
loc_8001676C:
    v1 = *gGameSkill;
    v0 = 4;                                             // Result = 00000004
    if (v1 == v0) goto loc_80016790;
    v0 = lw(s1 + 0x70);
    {
        const bool bJump = (v0 != 0);
        v0--;
        if (bJump) goto loc_800168B0;
    }
loc_80016790:
    v0 = lw(s1 + 0x58);
    v0 = lw(v0 + 0x2C);
    {
        const bool bJump = (v0 == 0);
        v0 = 0x4000000;                                 // Result = 04000000
        if (bJump) goto loc_800168A4;
    }
    v1 = lw(s1 + 0x64);
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x40;
        if (bJump) goto loc_800167E4;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = -0x41;                                     // Result = FFFFFFBF
        if (bJump) goto loc_800167D4;
    }
    v0 &= v1;
    sw(v0, s1 + 0x64);
    v0 = 1;                                             // Result = 00000001
    goto loc_80016860;
loc_800167D4:
    v0 = lw(s1 + 0x78);
    if (v0 == 0) goto loc_800167EC;
loc_800167E4:
    v0 = 0;                                             // Result = 00000000
    goto loc_80016860;
loc_800167EC:
    v0 = lw(s1 + 0x74);
    a2 = lw(s1);
    v1 = lw(s1 + 0x4);
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    a0 = a2 - a0;
    a1 = v1 - a1;
    P_AproxDistance();
    v1 = lw(s1 + 0x58);
    a0 = 0xFFC00000;                                    // Result = FFC00000
    v1 = lw(v1 + 0x28);
    s0 = v0 + a0;
    if (v1 != 0) goto loc_8001682C;
    v0 = 0xFF800000;                                    // Result = FF800000
    s0 += v0;
loc_8001682C:
    v1 = lw(s1 + 0x54);
    v0 = 0xE;                                           // Result = 0000000E
    s0 = u32(i32(s0) >> 16);
    if (v1 != v0) goto loc_80016840;
    s0 = u32(i32(s0) >> 1);
loc_80016840:
    v0 = (i32(s0) < 0xC9);
    if (v0 != 0) goto loc_80016850;
    s0 = 0xC8;                                          // Result = 000000C8
loc_80016850:
    _thunk_P_Random();
    v0 = (i32(v0) < i32(s0));
    v0 ^= 1;
loc_80016860:
    if (v0 == 0) goto loc_800168A4;
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0x2C);
    a0 = s1;
    P_SetMObjState();
    v1 = *gGameSkill;
    v0 = 4;                                             // Result = 00000004
    if (v1 == v0) goto loc_80016910;
    v0 = lw(s1 + 0x64);
    v0 |= 0x80;
    sw(v0, s1 + 0x64);
    goto loc_80016910;
loc_800168A4:
    v0 = lw(s1 + 0x70);
    v0--;
loc_800168B0:
    sw(v0, s1 + 0x70);
    if (i32(v0) < 0) goto loc_800168C8;
    a0 = s1;
    P_Move();
    if (v0 != 0) goto loc_800168D0;
loc_800168C8:
    a0 = s1;
    P_NewChaseDir();
loc_800168D0:
    v0 = lw(s1 + 0x58);
    v0 = lw(v0 + 0x50);
    if (v0 == 0) goto loc_80016910;
    _thunk_P_Random();
    v0 = (i32(v0) < 3);
    if (v0 == 0) goto loc_80016910;
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0x50);
    a0 = s1;
    S_StartSound();
loc_80016910:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FaceTarget() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    v1 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_800169B4;
    v0 = lw(s1 + 0x64);
    a0 = lw(s1);
    v0 &= v1;
    sw(v0, s1 + 0x64);
    v0 = lw(s1 + 0x74);
    a1 = lw(s1 + 0x4);
    a2 = lw(v0);
    a3 = lw(v0 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_800169B4;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_800169B4:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_PosAttack() noexcept {
    sp -= 0x28;
    sw(s2, sp + 0x20);
    s2 = a0;
    sw(ra, sp + 0x24);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    v0 = lw(s2 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80016AB8;
    a0 = lw(s2);
    a1 = lw(s2 + 0x4);
    v0 = lw(s2 + 0x64);
    v1 = lw(s2 + 0x74);
    v0 &= a2;
    sw(v0, s2 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s2 + 0x74);
    sw(v0, s2 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s2;
    if (v0 == 0) goto loc_80016A60;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s2 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s2 + 0x24);
    a0 = s2;
loc_80016A60:
    s1 = lw(s2 + 0x24);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 20;
    s1 += s0;
    _thunk_P_Random();
    a0 = s2;
    a1 = s1;
    a2 = 0x8000000;                                     // Result = 08000000
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    v0 &= 7;
    v0++;
    v1 = v0 << 1;
    v1 += v0;
    sw(v1, sp + 0x10);
    P_LineAttack();
loc_80016AB8:
    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void A_SPosAttack() noexcept {
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_80016C04;
    a1 = 8;                                             // Result = 00000008
    S_StartSound();
    v0 = lw(s1 + 0x74);
    s2 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80016B84;
    a2 = -0x21;                                         // Result = FFFFFFDF
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80016B84;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80016B84:
    s3 = lw(s1 + 0x24);
loc_80016B88:
    s2++;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 20;
    s0 += s3;
    _thunk_P_Random();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(v0, v1);
    a0 = s1;
    a1 = s0;
    a2 = 0x8000000;                                     // Result = 08000000
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    v1 = u32(i32(v0) >> 31);
    t0 = hi;
    t0 = u32(i32(t0) >> 1);
    t0 -= v1;
    v1 = t0 << 2;
    v1 += t0;
    v0 -= v1;
    v0++;
    v1 = v0 << 1;
    v1 += v0;
    sw(v1, sp + 0x10);
    P_LineAttack();
    v0 = (i32(s2) < 3);
    if (v0 != 0) goto loc_80016B88;
loc_80016C04:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_CPosAttack() noexcept {
    sp -= 0x30;
    sw(s3, sp + 0x24);
    s3 = a0;
    sw(ra, sp + 0x28);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    v0 = lw(s3 + 0x74);
    if (v0 == 0) goto loc_80016D50;
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = lw(s3 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80016CD0;
    a0 = lw(s3);
    a1 = lw(s3 + 0x4);
    v0 = lw(s3 + 0x64);
    v1 = lw(s3 + 0x74);
    v0 &= a2;
    sw(v0, s3 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s3 + 0x74);
    sw(v0, s3 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s3;
    if (v0 == 0) goto loc_80016CD4;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s3 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s3 + 0x24);
loc_80016CD0:
    a0 = s3;
loc_80016CD4:
    s1 = lw(s3 + 0x24);
    a2 = 0x8000000;                                     // Result = 08000000
    a1 = s1;
    P_AimLineAttack();
    s2 = v0;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 20;
    s1 += s0;
    _thunk_P_Random();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(v0, v1);
    a0 = s3;
    a1 = s1;
    a2 = 0x8000000;                                     // Result = 08000000
    a3 = s2;
    v1 = u32(i32(v0) >> 31);
    t0 = hi;
    t0 = u32(i32(t0) >> 1);
    t0 -= v1;
    v1 = t0 << 2;
    v1 += t0;
    v0 -= v1;
    v0++;
    v1 = v0 << 1;
    v1 += v0;
    sw(v1, sp + 0x10);
    P_LineAttack();
loc_80016D50:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_CPosRefire() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80016DFC;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80016DFC;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80016DFC:
    _thunk_P_Random();
    v0 = (i32(v0) < 0x28);
    if (v0 != 0) goto loc_80016E54;
    a1 = lw(s1 + 0x74);
    if (a1 == 0) goto loc_80016E40;
    v0 = lw(a1 + 0x68);
    if (i32(v0) <= 0) goto loc_80016E40;
    a0 = s1;
    P_CheckSight();
    if (v0 != 0) goto loc_80016E54;
loc_80016E40:
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0xC);
    a0 = s1;
    P_SetMObjState();
loc_80016E54:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_SpidAttack() noexcept {
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_80016F9C;
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = lw(s1 + 0x74);
    s2 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80016F1C;
    a2 = -0x21;                                         // Result = FFFFFFDF
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80016F1C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80016F1C:
    s3 = lw(s1 + 0x24);
loc_80016F20:
    s2++;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 20;
    s0 += s3;
    _thunk_P_Random();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(v0, v1);
    a0 = s1;
    a1 = s0;
    a2 = 0x8000000;                                     // Result = 08000000
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    v1 = u32(i32(v0) >> 31);
    t0 = hi;
    t0 = u32(i32(t0) >> 1);
    t0 -= v1;
    v1 = t0 << 2;
    v1 += t0;
    v0 -= v1;
    v0++;
    v1 = v0 << 1;
    v1 += v0;
    sw(v1, sp + 0x10);
    P_LineAttack();
    v0 = (i32(s2) < 3);
    if (v0 != 0) goto loc_80016F20;
loc_80016F9C:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_SpidRefire() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017048;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80017048;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80017048:
    _thunk_P_Random();
    v0 = (i32(v0) < 0xA);
    if (v0 != 0) goto loc_800170A4;
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_80017090;
    v0 = lw(v0 + 0x68);
    v1 = 0x4000000;                                     // Result = 04000000
    if (i32(v0) <= 0) goto loc_80017090;
    v0 = lw(s1 + 0x64);
    v0 &= v1;
    if (v0 != 0) goto loc_800170A4;
loc_80017090:
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0xC);
    a0 = s1;
    P_SetMObjState();
loc_800170A4:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_BspiAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017158;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_8001714C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_8001714C:
    a1 = lw(a0 + 0x74);
    a2 = 0x1A;                                          // Result = 0000001A
    P_SpawnMissile();
loc_80017158:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_TroopAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017298;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_800171FC;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_800171FC:
    v0 = lw(s1 + 0x64);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8001724C;
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_8001724C;
    v1 = lw(v0);
    a0 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_8001724C:
    a0 = s1;
    if (v1 == 0) goto loc_8001728C;
    a1 = 0x2E;                                          // Result = 0000002E
    S_StartSound();
    _thunk_P_Random();
    a0 = lw(s1 + 0x74);
    a1 = s1;
    a2 = a1;
    v0 &= 7;
    v0++;
    a3 = v0 << 1;
    a3 += v0;
    P_DamageMObj();
    goto loc_80017298;
loc_8001728C:
    a1 = lw(a0 + 0x74);
    a2 = 0x14;                                          // Result = 00000014
    P_SpawnMissile();
loc_80017298:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_SargAttack() noexcept {
    sp -= 0x28;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x20);
    sw(s0, sp + 0x18);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017368;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_8001733C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_8001733C:
    _thunk_P_Random();
    a0 = s1;
    a2 = 0x460000;                                      // Result = 00460000
    v0 &= 7;
    v0++;
    v0 <<= 2;
    sw(v0, sp + 0x10);
    a1 = lw(a0 + 0x24);
    a3 = 0;                                             // Result = 00000000
    P_LineAttack();
loc_80017368:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void A_HeadAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_8001749C;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_8001740C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_8001740C:
    v0 = lw(s1 + 0x64);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8001745C;
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_8001745C;
    v1 = lw(v0);
    a0 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_8001745C:
    a0 = s1;
    if (v1 == 0) goto loc_80017490;
    _thunk_P_Random();
    a0 = lw(s1 + 0x74);
    a1 = s1;
    a2 = s1;
    v0 &= 7;
    v0++;
    a3 = v0 << 3;
    P_DamageMObj();
    goto loc_8001749C;
loc_80017490:
    a1 = lw(a0 + 0x74);
    a2 = 0x15;                                          // Result = 00000015
    P_SpawnMissile();
loc_8001749C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_CyberAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017550;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017544;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_80017544:
    a1 = lw(a0 + 0x74);
    a2 = 0x17;                                          // Result = 00000017
    P_SpawnMissile();
loc_80017550:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_BruisAttack() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = lw(s0 + 0x74);
    v1 = 0x4000000;                                     // Result = 04000000
    if (a1 == 0) goto loc_8001761C;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_800175C8;
    v1 = lw(a1);
    a0 = lw(s0);
    v0 = lw(a1 + 0x4);
    a1 = lw(s0 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_800175C8:
    a0 = s0;
    if (v1 == 0) goto loc_80017610;
    a1 = 0x2E;                                          // Result = 0000002E
    S_StartSound();
    _thunk_P_Random();
    a0 = lw(s0 + 0x74);
    a1 = s0;
    a2 = a1;
    v0 &= 7;
    v0++;
    a3 = v0 << 1;
    a3 += v0;
    a3 <<= 2;
    a3 -= v0;
    P_DamageMObj();
    goto loc_8001761C;
loc_80017610:
    a1 = lw(a0 + 0x74);
    a2 = 0x16;                                          // Result = 00000016
    P_SpawnMissile();
loc_8001761C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_SkelMissile() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017718;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_800176C0;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_800176C0:
    a2 = 4;                                             // Result = 00000004
    s0 = 0x100000;                                      // Result = 00100000
    v0 = lw(s1 + 0x8);
    a1 = lw(s1 + 0x74);
    v0 += s0;
    sw(v0, s1 + 0x8);
    P_SpawnMissile();
    v1 = lw(s1 + 0x8);
    v1 -= s0;
    sw(v1, s1 + 0x8);
    v1 = lw(v0);
    a1 = lw(v0 + 0x48);
    a0 = lw(v0 + 0x4);
    a2 = lw(v0 + 0x4C);
    v1 += a1;
    a0 += a2;
    sw(v1, v0);
    sw(a0, v0 + 0x4);
    v1 = lw(s1 + 0x74);
    sw(v1, v0 + 0x90);
loc_80017718:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_Tracer() noexcept {
    v0 = *gGameTic;
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    v0 &= 3;
    sw(s0, sp + 0x10);
    if (v0 != 0) goto loc_80017964;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    a2 = lw(s1 + 0x8);
    P_SpawnPuff();
    a3 = 5;                                             // Result = 00000005
    a2 = lw(s1 + 0x8);
    v1 = lw(s1);
    a0 = lw(s1 + 0x48);
    v0 = lw(s1 + 0x4);
    a1 = lw(s1 + 0x4C);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_SpawnMObj();
    s0 = v0;
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s0 + 0x50);
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    v0 &= 3;
    v1 -= v0;
    sw(v1, s0 + 0x5C);
    if (i32(v1) > 0) goto loc_800177BC;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x5C);
loc_800177BC:
    s2 = lw(s1 + 0x90);
    if (s2 == 0) goto loc_80017964;
    v0 = lw(s2 + 0x68);
    if (i32(v0) <= 0) goto loc_80017964;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    a2 = lw(s2);
    a3 = lw(s2 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x24);
    s0 = v0;
    v0 = s0 - v1;
    if (s0 == v1) goto loc_80017850;
    a0 = 0x80000000;                                    // Result = 80000000
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 == 0);
        v0 = 0xF4000000;                                // Result = F4000000
        if (bJump) goto loc_80017830;
    }
    v0 += v1;
    sw(v0, s1 + 0x24);
    v0 = s0 - v0;
    if (i32(v0) < 0) goto loc_80017850;
    sw(s0, s1 + 0x24);
    goto loc_80017850;
loc_80017830:
    v0 = 0xC000000;                                     // Result = 0C000000
    v0 += v1;
    sw(v0, s1 + 0x24);
    v0 = s0 - v0;
    v0 = (a0 < v0);
    if (v0 == 0) goto loc_80017850;
    sw(s0, s1 + 0x24);
loc_80017850:
    v0 = lw(s1 + 0x24);
    v1 = lw(s1 + 0x58);
    s0 = v0 >> 19;
    s0 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a0 = lw(v1 + 0x3C);
    v0 += s0;
    a1 = lw(v0);
    FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    FixedMul();
    sw(v0, s1 + 0x4C);
    v1 = lw(s2);
    a0 = lw(s1);
    v0 = lw(s2 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = lw(s1 + 0x58);
    v1 = lw(v1 + 0x3C);
    div(v0, v1);
    if (v1 != 0) goto loc_800178E0;
    _break(0x1C00);
loc_800178E0:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_800178F8;
    }
    if (v0 != at) goto loc_800178F8;
    tge(zero, zero, 0x5D);
loc_800178F8:
    a1 = lo;
    a0 = 0xFFD80000;                                    // Result = FFD80000
    if (i32(a1) > 0) goto loc_8001790C;
    a1 = 1;                                             // Result = 00000001
loc_8001790C:
    v1 = lw(s1 + 0x8);
    v0 = lw(s2 + 0x8);
    v1 += a0;
    v0 -= v1;
    div(v0, a1);
    if (a1 != 0) goto loc_8001792C;
    _break(0x1C00);
loc_8001792C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80017944;
    }
    if (v0 != at) goto loc_80017944;
    tge(zero, zero, 0x5D);
loc_80017944:
    v0 = lo;
    v1 = lw(s1 + 0x50);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = v1 - 0x2000;
        if (bJump) goto loc_80017960;
    }
    v0 = v1 + 0x2000;
loc_80017960:
    sw(v0, s1 + 0x50);
loc_80017964:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_SkelWhoosh() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017A18;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017A10;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_80017A10:
    a1 = 0x4F;                                          // Result = 0000004F
    S_StartSound();
loc_80017A18:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_SkelFist() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017B78;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80017ABC;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80017ABC:
    v0 = lw(s1 + 0x64);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80017B0C;
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_80017B0C;
    v1 = lw(v0);
    a0 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_80017B0C:
    if (v1 == 0) goto loc_80017B78;
    _thunk_P_Random();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(v0, v1);
    a0 = s1;
    a1 = 0x50;                                          // Result = 00000050
    v1 = u32(i32(v0) >> 31);
    a2 = hi;
    a2 = u32(i32(a2) >> 2);
    a2 -= v1;
    v1 = a2 << 2;
    v1 += a2;
    v1 <<= 1;
    v0 -= v1;
    v0++;
    s0 = v0 << 1;
    s0 += v0;
    s0 <<= 1;
    S_StartSound();
    a0 = lw(s1 + 0x74);
    a1 = s1;
    a2 = a1;
    a3 = s0;
    P_DamageMObj();
loc_80017B78:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FatRaise() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017C1C;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017C20;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80017C1C:
    a0 = s1;
loc_80017C20:
    a1 = 0x46;                                          // Result = 00000046
    S_StartSound();
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FatAttack1() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    s2 = 0x8000000;                                     // Result = 08000000
    if (v0 == 0) goto loc_80017CD4;
    a2 = -0x21;                                         // Result = FFFFFFDF
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017CD8;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80017CD4:
    a0 = s1;
loc_80017CD8:
    a2 = 7;                                             // Result = 00000007
    v0 = lw(s1 + 0x24);
    a1 = lw(s1 + 0x74);
    v0 += s2;
    sw(v0, s1 + 0x24);
    P_SpawnMissile();
    a0 = s1;
    a1 = lw(a0 + 0x74);
    a2 = 7;                                             // Result = 00000007
    P_SpawnMissile();
    s1 = v0;
    s0 = lw(s1 + 0x24);
    v0 = lw(s1 + 0x58);
    s0 += s2;
    sw(s0, s1 + 0x24);
    s0 >>= 19;
    a0 = lw(v0 + 0x3C);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    s0 <<= 2;
    v0 += s0;
    a1 = lw(v0);
    FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    FixedMul();
    sw(v0, s1 + 0x4C);
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FatAttack2() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a0 = s1;
    if (v0 == 0) goto loc_80017E10;
    a2 = -0x21;                                         // Result = FFFFFFDF
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017E10;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_80017E10:
    a2 = 7;                                             // Result = 00000007
    v1 = 0xF8000000;                                    // Result = F8000000
    v0 = lw(s1 + 0x24);
    a1 = lw(s1 + 0x74);
    v0 += v1;
    sw(v0, s1 + 0x24);
    P_SpawnMissile();
    a0 = s1;
    a1 = lw(a0 + 0x74);
    a2 = 7;                                             // Result = 00000007
    P_SpawnMissile();
    s1 = v0;
    v0 = 0xF0000000;                                    // Result = F0000000
    s0 = lw(s1 + 0x24);
    v1 = lw(s1 + 0x58);
    s0 += v0;
    sw(s0, s1 + 0x24);
    s0 >>= 19;
    s0 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a0 = lw(v1 + 0x3C);
    v0 += s0;
    a1 = lw(v0);
    FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    FixedMul();
    sw(v0, s1 + 0x4C);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FatAttack3() noexcept {
    sp -= 0x28;
    sw(s2, sp + 0x20);
    s2 = a0;
    sw(ra, sp + 0x24);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    v0 = lw(s2 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017F48;
    a0 = lw(s2);
    a1 = lw(s2 + 0x4);
    v0 = lw(s2 + 0x64);
    v1 = lw(s2 + 0x74);
    v0 &= a2;
    sw(v0, s2 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s2 + 0x74);
    sw(v0, s2 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s2;
    if (v0 == 0) goto loc_80017F4C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s2 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s2 + 0x24);
loc_80017F48:
    a0 = s2;
loc_80017F4C:
    a1 = lw(s2 + 0x74);
    a2 = 7;                                             // Result = 00000007
    P_SpawnMissile();
    s1 = v0;
    v0 = lw(s1 + 0x24);
    v1 = 0xFC000000;                                    // Result = FC000000
    v0 += v1;
    s0 = v0 >> 19;
    s0 <<= 2;
    sw(v0, s1 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = lw(s1 + 0x58);
    v0 += s0;
    a0 = lw(v1 + 0x3C);
    a1 = lw(v0);
    FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    FixedMul();
    a0 = s2;
    sw(v0, s1 + 0x4C);
    a1 = lw(a0 + 0x74);
    a2 = 7;                                             // Result = 00000007
    P_SpawnMissile();
    s1 = v0;
    v0 = lw(s1 + 0x24);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 += v1;
    s0 = v0 >> 19;
    s0 <<= 2;
    sw(v0, s1 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = lw(s1 + 0x58);
    v0 += s0;
    a0 = lw(v1 + 0x3C);
    a1 = lw(v0);
    FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    FixedMul();
    sw(v0, s1 + 0x4C);
    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void A_SkullAttack() noexcept {
loc_8001804C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    a2 = lw(s1 + 0x74);
    v1 = 0x1000000;                                     // Result = 01000000
    if (a2 == 0) goto loc_800181E0;
    v0 = lw(s1 + 0x64);
    a1 = lw(s1 + 0x58);
    v0 |= v1;
    sw(v0, s1 + 0x64);
    a1 = lw(a1 + 0x18);
    s2 = a2;
    S_StartSound();
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80018108;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80018108;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80018108:
    s0 = lw(s1 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    s0 >>= 19;
    s0 <<= 2;
    v0 += s0;
    a1 = lw(v0);
    a0 = 0x280000;                                      // Result = 00280000
    FixedMul();
    sw(v0, s1 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = 0x280000;                                      // Result = 00280000
    FixedMul();
    sw(v0, s1 + 0x4C);
    v1 = lw(s2);
    a0 = lw(s1);
    v0 = lw(s2 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    a1 = v0;
    v0 = 0x66660000;                                    // Result = 66660000
    v0 |= 0x6667;                                       // Result = 66666667
    mult(a1, v0);
    v1 = u32(i32(a1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 20);
    a1 = v0 - v1;
    if (i32(a1) > 0) goto loc_80018194;
    a1 = 1;                                             // Result = 00000001
loc_80018194:
    v0 = lw(s2 + 0x44);
    v1 = lw(s2 + 0x8);
    a0 = lw(s1 + 0x8);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    v0 -= a0;
    div(v0, a1);
    if (a1 != 0) goto loc_800181BC;
    _break(0x1C00);
loc_800181BC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_800181D4;
    }
    if (v0 != at) goto loc_800181D4;
    tge(zero, zero, 0x5D);
loc_800181D4:
    v0 = lo;
    sw(v0, s1 + 0x50);
loc_800181E0:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_PainShootSkull() noexcept {
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6554;                                       // Result = gThinkerCap[1] (80096554)
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v1 = lw(v0);                                        // Load from: gThinkerCap[1] (80096554)
    v0 -= 4;                                            // Result = gThinkerCap[0] (80096550)
    a0 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80018270;
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a3 = 0xE;                                           // Result = 0000000E
    a2 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_8001823C:
    v0 = lw(v1 + 0x8);
    if (v0 != t0) goto loc_80018260;
    v0 = lw(v1 + 0x54);
    if (v0 != a3) goto loc_80018260;
    a0++;                                               // Result = 00000001
loc_80018260:
    v1 = lw(v1 + 0x4);
    if (v1 != a2) goto loc_8001823C;
loc_80018270:
    v0 = (i32(a0) < 0x15);                              // Result = 00000001
    s1 = a1 >> 19;
    if (v0 == 0) goto loc_80018334;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    P_SpawnMObj();
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    P_TryMove();
    a0 = s0;
    if (v0 != 0) goto loc_80018328;
    a1 = s2;
    a2 = a1;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj();
    goto loc_80018334;
loc_80018328:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_80018334:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_PainAttack() noexcept {
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lw(s2 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80018504;
    a0 = lw(s2);
    a1 = lw(s2 + 0x4);
    v0 = lw(s2 + 0x64);
    v1 = lw(s2 + 0x74);
    v0 &= a2;
    sw(v0, s2 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s2 + 0x74);
    sw(v0, s2 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_800183E0;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s2 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s2 + 0x24);
loc_800183E0:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    s1 = lw(s2 + 0x24);
    a0 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80018440;
    a3 = 0x80010000;                                    // Result = 80010000
    a3 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a2 = 0xE;                                           // Result = 0000000E
    a1 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_8001840C:
    v0 = lw(v1 + 0x8);
    if (v0 != a3) goto loc_80018430;
    v0 = lw(v1 + 0x54);
    if (v0 != a2) goto loc_80018430;
    a0++;                                               // Result = 00000001
loc_80018430:
    v1 = lw(v1 + 0x4);
    if (v1 != a1) goto loc_8001840C;
loc_80018440:
    v0 = (i32(a0) < 0x15);                              // Result = 00000001
    s1 >>= 19;
    if (v0 == 0) goto loc_80018504;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    P_SpawnMObj();
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    P_TryMove();
    a0 = s0;
    if (v0 != 0) goto loc_800184F8;
    a1 = s2;
    a2 = a1;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj();
    goto loc_80018504;
loc_800184F8:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_80018504:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_PainDie() noexcept {
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    A_Fall();
    v0 = 0x40000000;                                    // Result = 40000000
    v1 = lw(s2 + 0x24);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    v1 += v0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    a1 = 0;                                             // Result = 00000000
    if (a0 == v0) goto loc_800185A4;
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a3 = 0xE;                                           // Result = 0000000E
    a2 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_80018570:
    v0 = lw(a0 + 0x8);
    if (v0 != t0) goto loc_80018594;
    v0 = lw(a0 + 0x54);
    if (v0 != a3) goto loc_80018594;
    a1++;                                               // Result = 00000001
loc_80018594:
    a0 = lw(a0 + 0x4);
    if (a0 != a2) goto loc_80018570;
loc_800185A4:
    v0 = (i32(a1) < 0x15);                              // Result = 00000001
    s1 = v1 >> 19;
    if (v0 == 0) goto loc_80018668;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    P_SpawnMObj();
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    P_TryMove();
    a0 = s0;
    if (v0 != 0) goto loc_8001865C;
    a1 = s2;
    a2 = s2;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj();
    v0 = 0x80000000;                                    // Result = 80000000
    goto loc_8001866C;
loc_8001865C:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_80018668:
    v0 = 0x80000000;                                    // Result = 80000000
loc_8001866C:
    v1 = lw(s2 + 0x24);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    v1 -= v0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    a1 = 0;                                             // Result = 00000000
    if (a0 == v0) goto loc_800186D0;
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a3 = 0xE;                                           // Result = 0000000E
    a2 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_8001869C:
    v0 = lw(a0 + 0x8);
    if (v0 != t0) goto loc_800186C0;
    v0 = lw(a0 + 0x54);
    if (v0 != a3) goto loc_800186C0;
    a1++;                                               // Result = 00000001
loc_800186C0:
    a0 = lw(a0 + 0x4);
    if (a0 != a2) goto loc_8001869C;
loc_800186D0:
    v0 = (i32(a1) < 0x15);                              // Result = 00000001
    s1 = v1 >> 19;
    if (v0 == 0) goto loc_80018794;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    P_SpawnMObj();
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    P_TryMove();
    a0 = s0;
    if (v0 != 0) goto loc_80018788;
    a1 = s2;
    a2 = s2;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj();
    v0 = 0xC0000000;                                    // Result = C0000000
    goto loc_80018798;
loc_80018788:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_80018794:
    v0 = 0xC0000000;                                    // Result = C0000000
loc_80018798:
    v1 = lw(s2 + 0x24);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    v1 += v0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    a1 = 0;                                             // Result = 00000000
    if (a0 == v0) goto loc_800187FC;
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a3 = 0xE;                                           // Result = 0000000E
    a2 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_800187C8:
    v0 = lw(a0 + 0x8);
    if (v0 != t0) goto loc_800187EC;
    v0 = lw(a0 + 0x54);
    if (v0 != a3) goto loc_800187EC;
    a1++;                                               // Result = 00000001
loc_800187EC:
    a0 = lw(a0 + 0x4);
    if (a0 != a2) goto loc_800187C8;
loc_800187FC:
    v0 = (i32(a1) < 0x15);                              // Result = 00000001
    s1 = v1 >> 19;
    if (v0 == 0) goto loc_800188C0;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    P_SpawnMObj();
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    P_TryMove();
    a0 = s0;
    if (v0 != 0) goto loc_800188B4;
    a1 = s2;
    a2 = a1;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj();
    goto loc_800188C0;
loc_800188B4:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_800188C0:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_Scream() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lw(s0 + 0x58);
    v1 = lw(v0 + 0x38);
    v0 = (v1 < 0x33);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8001894C;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x2C0;                                        // Result = JumpTable_A_Scream[0] (800102C0)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80018980: goto loc_80018980;
        case 0x8001894C: goto loc_8001894C;
        case 0x80018924: goto loc_80018924;
        case 0x80018938: goto loc_80018938;
        default: jump_table_err(); break;
    }
loc_80018924:
    _thunk_P_Random();
    v0 &= 1;
    a1 = v0 + 0x27;
    goto loc_80018958;
loc_80018938:
    _thunk_P_Random();
    v0 &= 1;
    a1 = v0 + 0x31;
    goto loc_80018958;
loc_8001894C:
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x38);
loc_80018958:
    v1 = lw(s0 + 0x54);
    v0 = 0xF;                                           // Result = 0000000F
    a0 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80018978;
    v0 = 0x11;                                          // Result = 00000011
    if (v1 == v0) goto loc_80018978;
    a0 = s0;
loc_80018978:
    S_StartSound();
loc_80018980:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_XScream() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = 0x23;                                          // Result = 00000023
    S_StartSound();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_Pain() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(a0 + 0x58);
    a1 = lw(v0 + 0x24);
    if (a1 == 0) goto loc_800189DC;
    S_StartSound();
loc_800189DC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_Fall() noexcept {
loc_800189EC:
    v0 = lw(a0 + 0x64);
    v1 = -3;                                            // Result = FFFFFFFD
    v0 &= v1;
    sw(v0, a0 + 0x64);
    return;
}

void A_Explode() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = lw(a0 + 0x74);
    a2 = 0x80;                                          // Result = 00000080
    P_RadiusAttack();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_BossDeath() noexcept {
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    sp -= 0x68;
    v0 = a1 & 1;
    sw(ra, sp + 0x60);
    if (v0 == 0) goto loc_80018A54;
    v0 = 0x29A;                                         // Result = 0000029A
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 6;                                             // Result = 00000006
    if (v1 == v0) goto loc_80018AF4;
loc_80018A54:
    v0 = a1 & 2;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29B;                                     // Result = 0000029B
        if (bJump) goto loc_80018A74;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0x10;                                          // Result = 00000010
    if (v1 == v0) goto loc_80018AF4;
loc_80018A74:
    v0 = a1 & 4;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29C;                                     // Result = 0000029C
        if (bJump) goto loc_80018A94;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0xF;                                           // Result = 0000000F
    if (v1 == v0) goto loc_80018AF4;
loc_80018A94:
    v0 = a1 & 8;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29D;                                     // Result = 0000029D
        if (bJump) goto loc_80018AB4;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0xD;                                           // Result = 0000000D
    if (v1 == v0) goto loc_80018AF4;
loc_80018AB4:
    v0 = a1 & 0x10;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29E;                                     // Result = 0000029E
        if (bJump) goto loc_80018AD4;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0x11;                                          // Result = 00000011
    if (v1 == v0) goto loc_80018AF4;
loc_80018AD4:
    v0 = a1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29F;                                     // Result = 0000029F
        if (bJump) goto loc_80018C34;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0xC;                                           // Result = 0000000C
    if (v1 != v0) goto loc_80018C34;
loc_80018AF4:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    a1 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    if (a1 == v0) goto loc_80018B4C;
    a2 = v0;                                            // Result = gMObjHead[0] (800A8E90)
loc_80018B10:
    if (a1 == a0) goto loc_80018B3C;
    v1 = lw(a1 + 0x54);
    v0 = lw(a0 + 0x54);
    if (v1 != v0) goto loc_80018B3C;
    v0 = lw(a1 + 0x68);
    if (i32(v0) > 0) goto loc_80018C34;
loc_80018B3C:
    a1 = lw(a1 + 0x14);
    if (a1 != a2) goto loc_80018B10;
loc_80018B4C:
    v0 = lw(sp + 0x28);
    v1 = v0 - 0x29A;
    v0 = (v1 < 6);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80018C34;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x390;                                        // Result = JumpTable_A_BossDeath[0] (80010390)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80018B80: goto loc_80018B80;
        case 0x80018B98: goto loc_80018B98;
        case 0x80018BB0: goto loc_80018BB0;
        case 0x80018BC8: goto loc_80018BC8;
        case 0x80018BE0: goto loc_80018BE0;
        case 0x80018C0C: goto loc_80018C0C;
        default: jump_table_err(); break;
    }
loc_80018B80:
    a0 = sp + 0x10;
    a1 = 1;                                             // Result = 00000001
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -2;                                            // Result = FFFFFFFE
    goto loc_80018C20;
loc_80018B98:
    a0 = sp + 0x10;
    a1 = 7;                                             // Result = 00000007
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -3;                                            // Result = FFFFFFFD
    goto loc_80018C20;
loc_80018BB0:
    a0 = sp + 0x10;
    a1 = 1;                                             // Result = 00000001
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -5;                                            // Result = FFFFFFFB
    goto loc_80018C20;
loc_80018BC8:
    a0 = sp + 0x10;
    a1 = 1;                                             // Result = 00000001
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -9;                                            // Result = FFFFFFF7
    goto loc_80018C20;
loc_80018BE0:
    a0 = sp + 0x10;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -0x11;                                         // Result = FFFFFFEF
    v0 &= v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7F88);                                // Store to: gMapBossSpecialFlags (80077F88)
    a1 = 3;                                             // Result = 00000003
    EV_DoDoor();
    goto loc_80018C34;
loc_80018C0C:
    a0 = sp + 0x10;
    a1 = 1;                                             // Result = 00000001
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -0x21;                                         // Result = FFFFFFDF
loc_80018C20:
    v0 &= v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7F88);                                // Store to: gMapBossSpecialFlags (80077F88)
    EV_DoFloor();
loc_80018C34:
    ra = lw(sp + 0x60);
    sp += 0x68;
    return;
}

void A_Hoof() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = 0x53;                                          // Result = 00000053
    S_StartSound();
    a0 = s0;
    A_Chase();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_Metal() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = 0x54;                                          // Result = 00000054
    S_StartSound();
    a0 = s0;
    A_Chase();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_BabyMetal() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = 0x45;                                          // Result = 00000045
    S_StartSound();
    a0 = s0;
    A_Chase();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void L_MissileHit() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    s1 = lw(s0 + 0x84);
    if (s1 == 0) goto loc_80018D34;
    _thunk_P_Random();
    v1 = lw(s0 + 0x58);
    v0 &= 7;
    v1 = lw(v1 + 0x4C);
    v0++;
    mult(v0, v1);
    a0 = s1;
    a2 = lw(s0 + 0x74);
    a3 = lo;
    a1 = s0;
    P_DamageMObj();
loc_80018D34:
    a0 = s0;
    P_ExplodeMissile();
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void L_SkullBash() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    s1 = lw(s0 + 0x84);
    v0 = 0xFEFF0000;                                    // Result = FEFF0000
    if (s1 == 0) goto loc_80018DAC;
    _thunk_P_Random();
    v1 = lw(s0 + 0x58);
    v0 &= 7;
    v1 = lw(v1 + 0x4C);
    v0++;
    mult(v0, v1);
    a0 = s1;
    a1 = s0;
    a3 = lo;
    a2 = s0;
    P_DamageMObj();
    v0 = 0xFEFF0000;                                    // Result = FEFF0000
loc_80018DAC:
    v1 = lw(s0 + 0x64);
    a0 = lw(s0 + 0x58);
    v0 |= 0xFFFF;                                       // Result = FEFFFFFF
    sw(0, s0 + 0x50);
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
    v1 &= v0;
    sw(v1, s0 + 0x64);
    a1 = lw(a0 + 0x4);
    a0 = s0;
    P_SetMObjState();
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
